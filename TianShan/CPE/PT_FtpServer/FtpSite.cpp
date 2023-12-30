// ===========================================================================
// Copyright (c) 2004 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this _edia is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// ===========================================================================
#include "CECommon.h"
#include "FtpSite.h"
#include "CPECfg.h"
#include "FtpSock.h"

#include "utils.h"
#include "FtpConnection.h"
#include "IPushTrigger.h"
#include "FtpPushSess.h"
#include "TianShanDefines.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>     //for va_list, etc.
#include <assert.h>
#ifdef ZQ_OS_MSWIN
#include <objbase.h>
#include "NtfsFileIoFactory.h"
#include "VstrmFileIoFactory.h"
#else
#include "CStdFileIoFactory.h"
#endif
extern "C"
{
#include <sys/types.h>  //for ftime (struct timeb)
#include <sys/timeb.h>  //for ftime (struct timeb)
#ifdef ZQ_OS_LINUX
#include <sys/time.h>
#endif
}

#define _MAINLOOPFREQ 1     //number of seconds the main loop runs at
#define _USERLOOPFREQ 1     //the freq of the loop in the user thread


#define FTPSite			"FTPSite"

Mutex FtpSite::_mutexAccess;   //used for synchronization when limiting upload speed
FtpSite::ftpConn_v FtpSite::_ftpAliveConnetion;


#define MOLOG				glog

// because in the itvmessages.h, it define ZQ to a value, cause the namespace ZQ error
#ifdef ZQ
#undef ZQ
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
class RealTimeMulticast;

// default parameters to init thread pool
#define	RDS_THREADPOOL_THREADS		10
#define RDS_THREADPOOL_GROWBY			5
#define	RDS_THREADPOOL_MAX			100
#define	RDS_THREADPOOL_IDLETIME		5000

FtpSite::FtpSite(NativeThreadPool& threadPool) 
:  _gThreadPool(threadPool) , _serverSd(INVALID_SOCKET)
{
	//Initialize the program information
	strcpy(_progName, _PROGNAME);
	strcpy(_progVersion, _PROGVERSION);
	strcpy(_coreVersion, FTPS_COREVERSION);

	strcpy(_dateformat, RTENV_DEFDATEFORMAT);

	//Initialize the variables used to limit the site transfer speed
	_maxDlSpeed					= 0;  //max download speed (bytes/sec)
	_maxUlSpeed					= 0;  //max upload speed (bytes/sec)
	_bytesSent					= 0;   //num bytes sent in the current transfer cycle
	_bytesRecv					= 0;   //num bytes recv in the current transfer cycle
	_sendTimer					= 0;   //used for sending rate timing (downloading)
	_recvtimer					= 0;   //used for receiving rate timing (uploading)
	_isUninitialized			= false;  // indicate if the resource has been deleted
	
	// for SeaChange service
#ifdef ZQ_OS_MSWIN
	_stopEvent					= NULL;
#endif
	_maxConnection				= 1000;
	_validateCB					= NULL;
	_pFileIoFactory             = NULL;
}

FtpSite::~FtpSite()
{
	if (!_isUninitialized)
	{
		Uninit();
	}
}

//////////////////////////////////////////////////////////////////////
// static variables
//////////////////////////////////////////////////////////////////////
volatile long FtpSite::_nConnections	  = 0;
bool FtpSite::_bQuit		  = false;

//////////////////////////////////////////////////////////////////////
// Public Methods
//////////////////////////////////////////////////////////////////////


bool FtpSite::Initialize()
{
	if (!_defaultRoot.empty())
		FSUtils::checkSlashEnd(_defaultRoot);
#ifdef ZQ_OS_MSWIN	
	_stopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
#else
	sem_init(&_stopSem,0,0);
#endif
//	if (_gCPECfg._dwEnableFtpOverVstrm)
//	{
//#ifdef ZQ_OS_MSWIN
//		BaseIOI::setDefaultIO(BaseIOI::IO_VSTRM);
//		VStrmIO::setClientId(_gCPECfg._dwVstrmBwClientId);
//#else//linux now no vstream
//		BaseIOI::setDefaultIO(BaseIOI::IO_NTFS);		
//#endif
//
//		BaseIOI::Init();		
//	}

#ifdef ZQ_OS_MSWIN
	if (!_gCPECfg._dwEnableFtpOverVstrm)
	{
		NtfsFileIoFactory* pFactory = new NtfsFileIoFactory();
		pFactory->setRootDir(_gCPECfg._homeDir);
		_pFileIoFactory = pFactory;
	}
	else
	{
		VstrmFileIoFactory* pfactory = new VstrmFileIoFactory();
		pfactory->setBandwidthManageClientId(_gCPECfg._dwVstrmBwClientId);
		pfactory->setDisableBufDrvThrottle(true);
		_pFileIoFactory = pfactory;
	}
#else
	CStdFileIoFactory* pFactory = new CStdFileIoFactory();
	//pFactory->setRootDir(_gCPECfg._homeDir);
	_pFileIoFactory = pFactory;
#endif

	_pFileIoFactory->setLog(&glog);
	if (!_pFileIoFactory->initialize())
		return false;

	start();
	return true;
}

int FtpSite::run()
{
	MOLOG(Log::L_DEBUG, CLOGFMT(FTPSite, "work thread enter"));

	uint32 dwRet = 0;
	_bQuit = false;

	FtpSock sock;	

	_serverSd = sock.OpenServer((unsigned short)_ftpListenPort, _strFtpBindIp.c_str(), 5, &dwRet);
	if (_serverSd == SOCK_INVALID)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(FTPSite, "Failed to start ftp server on ip(%s), port(%d), error(%s) code(%d)"), 
			_strFtpBindIp.c_str(), _ftpListenPort, getErrMsg(dwRet).c_str(), dwRet);
		return -1;
	}

	MOLOG(Log::L_INFO, CLOGFMT(FTPSite, "*****************************************************************"));
	MOLOG(Log::L_INFO, CLOGFMT(FTPSite, "\tStarting server on ip %s, port %d..."), _strFtpBindIp.c_str(), _ftpListenPort);
	MOLOG(Log::L_INFO, CLOGFMT(FTPSite, "*****************************************************************"));

	//loop until the user quits
	bool bContinue = true;
	while ( bContinue )
	{
#ifdef ZQ_OS_MSWIN
		DWORD dwStatus = WaitForSingleObject(
			_stopEvent, 
			0     // time-out immediately
		);
		
		switch (dwStatus)
		{
			
		case WAIT_OBJECT_0:
			// we are about to exit
			//
			bContinue = false;
			break;
		case WAIT_TIMEOUT:

			// timer fires to start process 
			//
			// fall through
			//
		//break;
#else
		struct timeval tmval;
		gettimeofday(&tmval,(struct timezone*)NULL);
		struct timespec ts;
		ts.tv_sec = tmval.tv_sec;
		ts.tv_nsec = tmval.tv_usec *1000LL;
		int ret = sem_timedwait(&_stopSem, &ts);
		
		switch (ret)
		{
		
		case 0:
			bContinue = false;
			break;
		case ETIMEDOUT://time out			
#endif
		default:
			//loop every _MAINLOOPFREQ sec
			if (sock.CheckStatus(_MAINLOOPFREQ) > 0) 
			{
				FtpConnection *pFtpConn = NULL;
				SOCKET clientsd = sock.Accept();
				
				if (clientsd != SOCK_INVALID)
				{
					//Create the FTP server class to handle the client connection
					if ((pFtpConn = 
						new FtpConnection(clientsd, *this, _maxConnection, _gThreadPool)) != NULL)
					{
						MOLOG(Log::L_INFO, CLOGFMT(FTPSite, "Connection established from %s:%s -> %s (%u)"),
							pFtpConn->clientIP(),pFtpConn->clientPort(),
							pFtpConn->clientName(),clientsd);

						pFtpConn->start();
					}
				}
			} // end of sock.CheckStatus

		} // end switch
	}

	//close the listening socket
	sock.Close();

	MOLOG(Log::L_INFO, CLOGFMT(FTPSite, "Ftp server is stopping, waiting for client connection exit"));

	//set quit flag to notify client
	_bQuit = true;

	{
		Guard<Mutex> AutoLock(_mutexAccess);

		for(unsigned int i=0;i<_ftpAliveConnetion.size();i++)
		{
			_ftpAliveConnetion[i]->Close();
		}		
	}

	//wait for all the client connection threads to exit
	int64 dwT1 = ZQTianShan::now();
	while (_ftpAliveConnetion.size() > 0 && ZQTianShan::now() - dwT1 < 2000)
#ifdef ZQ_OS_MSWIN
		Sleep(300);
#else
		usleep(200000);
#endif
	if (_ftpAliveConnetion.size() > 0)
		MOLOG(Log::L_ERROR, CLOGFMT(FTPSite, "There are %d clients connection when Ftp site have been shut down"), _ftpAliveConnetion.size());
	else
		MOLOG(Log::L_INFO, CLOGFMT(FTPSite, "Ftp server stopped"));

	MOLOG(Log::L_DEBUG, CLOGFMT(FTPSite, "work thread exit"));		

	return 0;
}


void FtpSite::Stop()
{
#ifdef ZQ_OS_MSWIN
	if (_stopEvent && _stopEvent!=INVALID_HANDLE_VALUE)
	{
		MOLOG(Log::L_INFO, CLOGFMT(FTPSite, "shutting down ftp server"));
		::SetEvent(_stopEvent);
	}
#else
	int nval;	
	if(sem_getvalue(&_stopSem,&nval) == 0)
	{
		MOLOG(Log::L_INFO, CLOGFMT(FTPSite, "shutting down ftp server"));
		sem_post(&_stopSem);
	}
#endif
}

bool FtpSite::Uninit()
{
#ifdef ZQ_OS_MSWIN
	if (_stopEvent != NULL)
	{
		waitHandle(2000);
		CloseHandle(_stopEvent);
		_stopEvent = NULL;
	}
#else
	int nval;
	if(sem_getvalue(&_stopSem, &nval) == 0)
	{
		waitHandle(2000);
		sem_destroy(&_stopSem);
	}
#endif
	_isUninitialized = true;


	//if (_gCPECfg._dwEnableFtpOverVstrm)
	//{
	//	BaseIOI::Uninit();
	//}

	if (_pFileIoFactory)
	{
		_pFileIoFactory->uninitialize();
		delete _pFileIoFactory;
		_pFileIoFactory = NULL;
	}

	return true;
}



////////////////////////////////////////
// Functions used load/read the user
// properties files
////////////////////////////////////////
int FtpSite::checkUserProp(char *username)
{
	return true;  //allow all users by default
}

////////////////////////////////////////
// Functions used to access the password
// (passwd) file.
////////////////////////////////////////
bool FtpSite::getPwdInfo(userPwd_t& pwd, const char *username)
{
	//set the group to anonymous (no password required)
	sprintf(pwd.groupnumber,"%d",RTENV_ANONYMOUSGROUPNUM);

	//set the home directory
	if (!_defaultRoot.empty())
		snprintf(pwd.homedir,sizeof(pwd.homedir),"%s",_defaultRoot.c_str());

	return true;
}

////////////////////////////////////////
// Functions used to check the user's permissions.
//@return	- 1 if any of the "pflags" are present in the user's permissions
bool FtpSite::checkPermissions(const char *username, const char *path, const char *pflags)
{
	if (pflags == NULL)
		return false;

	for (const char *ptr = pflags; *ptr != '\0'; ptr++)
	{
		if (strchr(RTENV_DEFAULTPERMISSIONS,*ptr) != NULL)
			return true;
	}

	return false;
}

////////////////////////////////////////
// Create various server paths
////////////////////////////////////////

//"subdir" is the sub-directory of the root path of the site
//that should be added to the path.
//"filename" is used to specify a filename within the directory.
//Example root path: /usr/ftpserver/
//bool FtpSite::buildPathRoot(std::string& path, const char *subdir, const char *filename /*=NULL*/)
//{
//	path = "";
//
//	if (subdir == NULL)
//		return false;
//
//	path = _defaultRoot + subdir;
//
//	FSUtils::checkSlash(path);           //make sure the slashes are correct
//	FSUtils::checkSlashEnd(path);  //add ending '/' if necessary
//	if (filename != NULL)
//		path += filename;
//
//	return true;
//}

//builds a path in the default home directory for the site
//Example home path: /usr/ftpserver/site/
//bool FtpSite::buildPathHome(std::string& path, const char *subdir, const char *filename/*=NULL*/)
//{
//	path = _defaultRoot;
//
//	if (subdir == NULL)
//		return false;
//
//	//use the default root dir as the default home dir
//	path += subdir;
//	FSUtils::checkSlash(path);           //make sure the slashes are correct
//	FSUtils::checkSlashEnd(path);  //add ending '/' if necessary
//	if (filename != NULL)
//		path += filename;
//
//	return true;
//}


////////////////////////////////////////
// Functions used for logging
////////////////////////////////////////





//////////////////////////////////////////////////////////////////////
// Set the name of the current log file.
// "logfile" is the name of the file and "basepath" is the directory
// the file will be written in.
// 
// [in] PathAndName : The log file path and name
// [in] logfile     : Name of the log file.
// [in] basepath    : Path to write the file into (default is local dir)
//
// Return : VOID
//
// Ex. logfile = access.log, basepath = /var/log/
//     The PathAndName will be "/var/log/access.log"
//
// NOTE: This function assumes "logfile" and "basepath" are valid.
//       "basepath" must end in a "/" (UNIX) or "\" (WINDOWS).
//
void 
FtpSite::setFilename(std::string &PathAndName,const char *logfile, const char *basepath /*=NULL*/)
{

	if (logfile == NULL)
		return;

	size_t len = strlen(logfile) + 1;
	if (basepath != NULL)
		len += strlen(basepath);
	char *tmpname = NULL;
	try {
		tmpname = new char[len];
	}
	catch (std::bad_alloc&) { assert(0); }

	if (tmpname == NULL)
		return;

	if (basepath != NULL)
	{
		strcpy(tmpname,basepath);
		strcat(tmpname,logfile);
	}
	else
		strcpy(tmpname,logfile);

	PathAndName = tmpname;    //set the new log file name
	

	delete[] tmpname;  //log file name did not change
}


////////////////////////////////////////
// Functions used to specify/limit the
// range of ports used in data
// connections (PASV)
////////////////////////////////////////

//Returns -1 to indicate the function is unimplemented
int FtpSite::setDataPortRange(unsigned short startport, unsigned short endport)
{
	return(-1);  //by default this feature is not implemented
}

//Returns -1 to indicate the function is unimplemented
int FtpSite::dataPort(char *bindport, int maxportlen, int *sdptr, char *bindip)
{
	return(-1);  //by default this feature is not implemented
}

//Returns -1 to indicate the function is unimplemented
int FtpSite::freetDataPort(int sd)
{
	return(-1);  //by default this feature is not implemented
}

////////////////////////////////////////
// Functions used to handle adding new
// files to the server.
////////////////////////////////////////

//This function is called in FtpsXfer to add files to the server
//(called during upload).
//If size = -1, the file size will be set by getting the file stats from the OS.
bool FtpSite::addFile(char *filepath, char *username, int size /*=-1*/)
{
	return true;  //by default do nothing
}


////////////////////////////////////////
// Functions used to control/monitor
// transfer rates.
////////////////////////////////////////

//This function is called in FtpsXfer to periodically update
//the user's current transfer rate.  The rate at which this 
//function is called will be set in FtpsXfer.h.
void FtpSite::setCurrentXferRate(int userid, long bytespersec)
{
	//override to implement
	return;
}

//Returns the maximum allowable download speed for a user.
//This function is called in FtpsXfer to regulate a
//user's download speed.
int FtpSite::getMaxDLSpeed(char *username)
{
	return 0;  //0 -> no speed limit
}

//Returns the maximum allowable upload speed for a user.
//This function is called in FtpsXfer to regulate a
//user's upload speed.
int FtpSite::getMaxULSpeed(char *username)
{
	return 0;  //0 -> no speed limit
}

////////////////////////////////////////
// Functions used to update the file
// transfer information/statistics
////////////////////////////////////////

//update download statistics
int FtpSite::updateDLStats(int userid, char *username, int64 bytes, int bytespersec)
{
	return -1;  //by default this feature is not implemented
}

//update upload statistics
int FtpSite::updateULStats(int userid, char *username, int64 bytes, int bytespersec)
{
	return -1;  //by default this feature is not implemented
}

////////////////////////////////////////
// Functions used to update the user's
// credits
////////////////////////////////////////

//Add credits to a user's account.
//flagupload != 0 -> Add after upload, flagupload == 0 -> Add after download
int FtpSite::addCredits(const char *filepath, const int userid, const char *username, const int64 bytes, const int flagupload)
{
	return -1;  //by default this feature is not implemented
}

//Remove credits from a user's account
//flagupload != 0 -> Remove after upload, flagupload == 0 -> Remove after download
int FtpSite::removeCredits(const char *filepath, const int userid, const char *username, const int64 bytes, const int flagupload)
{
	return -1;  //by default this feature is not implemented
}

////////////////////////////////////////
// Functions used to check if FXP is
// allowed
////////////////////////////////////////

bool FtpSite::isAllowedFXP(char *username, int flagupload, int flagpasv)
{
	return true;  //allow FXP by default
}

////////////////////////////////////////
// Time Functions
////////////////////////////////////////

//Loads "buffer" with the string version of the time (returns a pointer to buffer).
//"formatstring" is the string that is passed to strftime().
//If ctime = 0, the current time is used.  If flaggmt != 0 GMT time will be used (else localtime).
std::string FtpSite::timeString(const char *formatstring/*=NULL*/, time_t ctime /*=0*/, int flaggmt /*=0*/)
{
	struct tm *ltimeptr, ltime;
	struct timeb timebuf;
	long tmptime;
	char *tmpformatstring = NULL, utcoffset[16];

	char buffer[MAX_PATH] = ""; //initialize to an "" string
	std::string timestr;

	if (formatstring == NULL)
		formatstring = _dateformat;

	ftime(&timebuf);     //get the current time

	for (const char *ptr = formatstring; *ptr != '\0'; ptr++)
	{
		if (*ptr == '%')
		{
			ptr++;  //move to the next character
			if (*ptr == '\0')
				break;
			if (*ptr == 'O')
			{
				size_t len = strlen(formatstring) + 16;    //add 16 for UTC offset string
				//allocate the temp format string buffer
				if ((tmpformatstring = new char[len]) == NULL)
					return(buffer);
				//copy over the first part of the format string
				strncpy(tmpformatstring,formatstring,ptr-formatstring-1);
				tmpformatstring[ptr-formatstring-1] = '\0';
				//build the UTC offset string
				if (timebuf.timezone > 0)
					sprintf(utcoffset,"-%04d",timebuf.timezone); //add "-" western time zones
				else 
					sprintf(utcoffset,"+%04d",(-1)*(timebuf.timezone));

				//append the UTC offset string
				strcat(tmpformatstring,utcoffset);
				//append the rest of the format string
				strcat(tmpformatstring,ptr+1);
				break;
			}
		}
	}

	//if a time was not specified, use the current time
	tmptime = (long)((ctime != 0) ? ctime : timebuf.time);  ///////

	//If "tmptime" is an invalid time, try to use 0 instead
	if ((ltimeptr = getTimeR(tmptime,&ltime,flaggmt)) == NULL)
	{
		tmptime = 0;
		ltimeptr = getTimeR(tmptime,&ltime,flaggmt);
	}

	if (ltimeptr != NULL)
	{
		if (tmpformatstring != NULL)
			strftime(buffer,MAX_PATH-1,tmpformatstring,&ltime);
		else
			strftime(buffer,MAX_PATH-1,formatstring,&ltime);
		buffer[MAX_PATH-1] = '\0';
	}

	if (tmpformatstring != NULL)
		delete[] tmpformatstring;

	timestr = buffer;
	return timestr;
}

//A thread safe version of the C standard function localtime()/gmtime()
struct tm *FtpSite::getTimeR(long ctime, struct tm *tmoutput, int flaggmt)
{
	struct tm *ltimeptr;

	if (tmoutput == NULL)
		return NULL;

	memset(tmoutput,0,sizeof(struct tm));

#ifdef WIN32
	FILETIME filetime;
	LONGLONG ll;
	SYSTEMTIME systemtime;
	//converts from the standard C time (sec since Jan 1970) to FILETIME.
	ll = Int32x32To64(ctime,10000000) + 116444736000000000;
	filetime.dwLowDateTime = (DWORD)(ll);
	filetime.dwHighDateTime = (DWORD)(ll >> 32);
	//load the tmoutput struct
	if (flaggmt == 0)
		FileTimeToLocalFileTime(&filetime,&filetime);
	if (FileTimeToSystemTime(&filetime,&systemtime) != 0)
	{
		tmoutput->tm_hour = systemtime.wHour;
		tmoutput->tm_mday = systemtime.wDay;
		tmoutput->tm_min = systemtime.wMinute;
		tmoutput->tm_mon = systemtime.wMonth - 1;
		tmoutput->tm_sec = systemtime.wSecond;
		tmoutput->tm_wday = systemtime.wDayOfWeek;
		tmoutput->tm_year = systemtime.wYear - 1900;
		ltimeptr = tmoutput;
	}
	else
		ltimeptr = NULL;
#else
	if (flaggmt == 0)
		ltimeptr = localtime_r(&ctime,tmoutput);
	else
		ltimeptr = gmtime_r(&ctime,tmoutput);
#endif

	return(ltimeptr);
}


////////////////////////////////////////
// Functions used to get Site state
////////////////////////////////////////

const char *FtpSite::progName()
{
	return _progName;
}

const char *FtpSite::progVersion()
{
	return _progVersion;
}

const char *FtpSite::coreVersion()
{
	return _coreVersion;
}

unsigned long FtpSite::timeval()
{
	unsigned long rettime = 1;

#ifdef ZQ_OS_MSWIN
	FILETIME systemtimeasfiletime;
	LARGE_INTEGER litime;

	GetSystemTimeAsFileTime(&systemtimeasfiletime);
	memcpy(&litime,&systemtimeasfiletime,sizeof(LARGE_INTEGER));
	litime.QuadPart /= 10000;  //convert to milliseconds
	litime.QuadPart &= 0xFFFFFFFF;    //keep only the low part
	rettime = (unsigned long)(litime.QuadPart);
#else
	struct timeval tv;
	struct timezone tz;
	unsigned long t1, t2;

	gettimeofday(&tv,&tz);
	t1 = (tv.tv_sec & 0x003FFFFF) * 1000; //convert sec to milliseconds
	t2 = (tv.tv_usec / 1000) % 1000;      //convert usec to milliseconds
	rettime = t1 + t2;
#endif

	return(rettime);
}

unsigned long FtpSite::timeDiff(const unsigned long start, const unsigned long end)
{
	unsigned long timediff = 0;

	if (start > end)
		timediff = ((long)(-1) - start) + end + 1;    //if overflow
	else
		timediff = end - start;

	return(timediff);
}

float FtpSite::timeDiff2Sec(const unsigned long start, const unsigned long end)
{
	unsigned long timediff = timeDiff(start,end);
	float timesec = timediff / (float)1000.00;

	//always return at least 0.01 sec;
	if (timesec < 0.01)
		timesec = (float)0.01;

	return(timesec);
}

//////////////////////////////////////////////////////////////////////
// Builds a buffer based on a formatting string.
// This function is very similar to the time function strftime().
// It fills in all the occurrences of the specifiers, from the
// "specifiers" array, in the "format" string with the corresponding
// values in the "values" array.  "nparams" is the number of elements
// in the "specifiers" and "values" arrays.  "escchar" is the
// character used to start the specifier (Ex. '%').
//
// [out] str       : The output string
// [in] format     : Format string.
// [in] specifiers : String containing the specifier characters.
// [in] values     : Array of strings used to substitute for the
//                   corresponding specifiers.
// [in] nparams    : Number of elements in "specifiers" or "values"
// [in] escchar    : Escape character used to indicate a specifier.
//
// Return : A dynamically allocated buffer containing the format
//          string with all the expanded specifiers.
//
// Ex. format = "User is %u and command is %m",
//     specifiers = "um", values = {"root","RETR"}, escchar = '%'
//     --> returned buffer = "User is root and command is RETR"
bool FtpSite::formatString(std::string& str, const char *format, const char *specifiers, const char *values[], int nparams, char escchar /* ='%' */)
{
	if (format == NULL || specifiers == NULL || values == NULL || nparams == 0)
		return false;

	str = "";
	for (const char* ptr = (char *)format; *ptr != '\0'; ptr++)
	{
		if (*ptr != escchar)
		{
			str += *ptr;
			continue;
		}

		if (*ptr++ == '\0')
			break;  //reached the end of the string

		if (*ptr == escchar)
		{
			str += *ptr;
			continue;
		}

		//lookup the value for the specifier
		for (int i = 0; i < nparams; i++)
		{
			if (*ptr == specifiers[i])
				if (values[i] != NULL)
				str += values[i];
		}
	}

	return true;
}

wstring FtpSite::StdString2WString(const std::string &src)
{
	size_t string_len = src.length();
	wstring ret = L"";
	ret.resize(string_len);

	for (size_t i = 0; i < string_len; i++)
		ret[i] = static_cast<unsigned char>(src[i]);

	return ret;

}


//register client connection to connections vector
void FtpSite::clientConnAdd(FtpConnection* pCon)
{
	Guard<Mutex> AutoLock(_mutexAccess);
	
	_ftpAliveConnetion.push_back(pCon);	
}

//remove client connection from connections vector
void FtpSite::clientConnRemove(FtpConnection* pCon)
{
	Guard<Mutex> AutoLock(_mutexAccess);

	for(ftpConn_v::iterator it= _ftpAliveConnetion.begin();
		it != _ftpAliveConnetion.end();
		it ++)
	{
		if (*it == pCon)
		{
			_ftpAliveConnetion.erase(it);
			break;
		}
	}		
}

void FtpSite::setListenPort(int nPort)
{
	_ftpListenPort = nPort;
}

void FtpSite::setBindIP(const char* szIP)
{
	_strFtpBindIp = szIP;
}

void FtpSite::setHomeDir(const char* szHome)
{
	_defaultRoot = szHome;
	validatePath(szHome);
}

IPushSource* FtpSite::findPushXfer(const char* contentKey)
{
	Guard<Mutex> op(_lock);
	FtpXferMap::iterator it = _pushXfers.find(contentKey);
	if (it==_pushXfers.end())
	{
		MOLOG(Log::L_WARNING, CLOGFMT(FTPSite, "Failed to find Pushsess for contentKey(%s)"), contentKey);
		return 0;
	}
	
	return it->second;
}

bool FtpSite::addPushXfer(const char* contentKey, FtpsPushXfer* pXfer)
{
	if (!pXfer)
		return false;
	
	Guard<Mutex> op(_lock);
	FtpXferMap::iterator it = _pushXfers.find(contentKey);
	if (it!=_pushXfers.end())
		return false;
	_pushXfers[contentKey] = pXfer;
	return true;
}

bool FtpSite::removePushXfer(const char* contentKey)
{
	Guard<Mutex> op(_lock);
	FtpXferMap::iterator it = _pushXfers.find(contentKey);
	if (it==_pushXfers.end())
	{
		MOLOG(Log::L_WARNING, CLOGFMT(FTPSite, "Failed to find Pushsess for contentKey(%s)"), contentKey);
		return false;
	}
	
	_pushXfers.erase(it);
	return true;
}

std::string FtpSite::makeContentKey(const char* contentStoreNetId, 
				const char* volume,
		const char* content)
{
	return std::string(content) + volume + contentStoreNetId;
}
void FtpSite::setMaxConnection( int nMaxConnNum )
{
	_maxConnection = nMaxConnNum;
	MOLOG(Log::L_DEBUG, CLOGFMT(FTPSite, "set MaxConnection=%d"), _maxConnection);
}