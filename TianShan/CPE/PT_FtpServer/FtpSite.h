// ===========================================================================
// Copyright (c) 2004 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
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
// ---------------------------------------------------------------------------
// ===========================================================================

#ifndef __RTENV_H__
#define __RTENV_H__

#include <time.h>
#include <string>
#include "Locks.h"
#include "Socket.h"
#include <vector>
#include "CECommon.h"
#include "NativeThreadPool.h"
#include <map>
#include "ZQResource.h"



#define _PROGNAME		"CPE FTPServer"
#define _PROGVERSION	""
#define _LOGINSTR "Connected to " _PROGNAME " "__STR1__(ZQ_PRODUCT_VER_MAJOR)"."__STR1__(ZQ_PRODUCT_VER_MINOR)"."__STR1__(ZQ_PRODUCT_VER_PATCH)"."__STR1__(ZQ_PRODUCT_VER_BUILD)"."
#define DEF_TCP_PORT 21
#define DEFAULT_FTP_BINDIP  _T("0.0.0.0")

#define RTENV_MAXPWLINE 64      //max size of a string parameter in the password (passwd) file

#define RTENV_ANONYMOUSGROUPNUM 99   //the group number used to specify anonymous (no Password)

//The default access permissions (applied to directories only)
//l = list directory contents
//v = recursive directory listing
//c = change current working directory (CWD)
//p = print working dir (PWD)
//s = system info (SYST and STAT)
//t = SITE command
//d = download files
//u = upload files
//o = remove files
//a = rename files/directories
//m = make directories
//n = remove directories
//r = "read" -> d
//w = "write" -> uoamn
//x = "execute" -> lvcpst
#define RTENV_DEFAULTPERMISSIONS "lvcpstduoamn"

//Formatting string for the log date field
//This uses the formatting for strftime() except the "%O"
//specifier was added to display the UTC offset in minutes
#define RTENV_DEFDATEFORMAT "%d/%m/%Y:%H:%M:%S"

//Formatting string for the program log file
// %D -> date (format is specified using setDateFormat())
// %S -> sub-system (where in the prog the log is coming from)
// %M -> message (message that is passed in)
#define RTENV_DEFPROGFORMAT "%D;%M"
//Default program log file name
//This uses the same formatting as RTENV_DEFDATEFORMAT
#define RTENV_DEFPROGLOG "ftpd.log"

//Formatting string for the access log file
// %d -> date (format is specified using setDateFormat())
// %s -> server IP
// %l -> server listening port
// %c -> client IP
// %p -> client port
// %u -> user name
// %m -> FTP command (method)
// %a -> argument to the command
// %r -> 3-digit response code from the server
// %t -> response text from the server
#define RTENV_DEFACCESSFORMAT "%d;%s;%l;%c;%p;%u;%m;%a;%r;%t"
//Default access log file name
//This uses the same formatting as RTENV_DEFDATEFORMAT




#define RDS_SERVICELOG						"tsRDS.log"
#define APPLICATION_NAME					"RDS"

#define DEFAULT_FTP_ALLOWANONYMOUS			1                // the default value
#define DEFAULT_FTP_SKIPAUTHENTICATION		1
#define DEFAULT_FTP_MAXCONNECTION			1000

#define	DEFAULT_LOGFILE_SIZE		10*1024*1024
#define DEFAULT_LOGFILE_LEVEL		ZQ::common::Log::L_DEBUG

namespace ZQTianShan{
	namespace ContentProvision{
		class IPushSource;
		class FileIoFactory;
	}
}

class FtpsPushXfer;
class FtpConnection;

using namespace ZQTianShan::ContentProvision;

typedef bool (* VALIDATEPUSH)(void* pCtx, const char* szNetId, const char* szVolume, const char* szContent);


class FtpSite : public NativeThread
{
protected:
	virtual int run(void);
public:
	FtpSite(NativeThreadPool& threadPool);
	virtual ~FtpSite();

	typedef struct
	{
		char username[RTENV_MAXPWLINE];      //client's username
		char passwordraw[RTENV_MAXPWLINE];   //raw password (maybe encrypted)
		char usernumber[16];                    //string version of the user number (int)
		char groupnumber[16];                   //string version of the group number (int)
		char dateadded[RTENV_MAXPWLINE];     //date the user was added
		char homedir[MAX_PATH];         //user's home directory
	} userPwd_t;

	bool	Initialize();
	void	Stop();
	bool	Uninit();


	void setListenPort(int nPort = 21);
	void setBindIP(const char* szIP = "0.0.0.0");
	void setHomeDir(const char* szHome);
// 	void setSocketTimeOut(int nMiniSeconds);

	// delete old trick files that above nSeconds, default is 1 day
	void DeleteOldFilesByTimes(const char* TargetDir, int nSeconds = 24*3600);

	//user functions
	virtual int checkUserProp(char *username);

	//password functions
	virtual bool getPwdInfo(userPwd_t& pwd, const char *username);

	//permissions functions
	virtual bool checkPermissions(const char *username, const char *path, const char *pflags);

	//path functions
// 	virtual bool buildPathRoot(std::string& path, const char *subdir, const char *filename=NULL);
// 	virtual bool buildPathHome(std::string& path, const char *subdir="", const char *filename = NULL);


	//FTP data connection functions
	virtual int setDataPortRange(unsigned short startport, unsigned short endport);
	virtual int dataPort(char *bindport, int maxportlen, int *sdptr, char *bindip);
	virtual int freetDataPort(int sd);

	//used to handle added (uploaded) files
	virtual bool addFile(char *filepath, char *username, int size = -1);

	//transfer rate functions
	virtual void setCurrentXferRate(int userid, long bytespersec);
	virtual int getMaxDLSpeed(char *username);
	virtual int getMaxULSpeed(char *username);

	//file transfer information functions
	virtual int updateDLStats(int userid, char *username, int64 bytes, int bytespersec);
	virtual int updateULStats(int userid, char *username, int64 bytes, int bytespersec);

	//credits functions
	virtual int addCredits(const char *filepath, const int userid, const char *username, const int64 bytes, const int flagupload);
	virtual int removeCredits(const char *filepath, const int userid, const char *username, const int64 bytes, const int flagupload);

	//checks if a user is allowed to use FXP
	//(use a different IP address for data than their control connection).
	virtual bool isAllowedFXP(char *username, int flagupload, int flagpasv);

	//time functions
	std::string timeString(const char *formatstring = NULL, time_t ctime =0, int flaggmt =0);
	struct tm *getTimeR(long ctime, struct tm *tmoutput, int flaggmt);

	//program information functions
	const char *progName();
	const char *progVersion();
	const char *coreVersion();

	//register client connection to connections vector
	void clientConnAdd(FtpConnection* pCon);

	//remove client connection from connections vector
	void clientConnRemove(FtpConnection* pCon);

	//
	// for PushTrigger
	//
	IPushSource* findPushXfer(const char* contentKey);
	bool addPushXfer(const char* contentKey, FtpsPushXfer* pXfer);
	bool removePushXfer(const char* contentKey);
	std::string makeContentKey(const char* contentStoreNetId, 
		const char* volume,
		const char* content);
		
	void setMaxConnection(int nMaxConnNum);

	void setValidatePushCB(VALIDATEPUSH cb, void* ctx){_validateCB=cb;_pCbCtx=ctx;}
	VALIDATEPUSH	_validateCB;
	void*			_pCbCtx;
protected:
	ZQ::common::Mutex		_lock;
	typedef std::map<std::string, FtpsPushXfer*>	FtpXferMap;
	FtpXferMap		_pushXfers;

public:
	//Global variable used to signal the program to exit
	//Used in FtpsXfer to limit the transfer speed for the site
	long			_maxDlSpeed;  //max download speed (bytes/sec)
	long			_maxUlSpeed;  //max upload speed (bytes/sec)
	long			_bytesSent;   //num bytes sent in the current transfer cycle (downloading)
	long			_bytesRecv;   //num bytes recv in the current transfer cycle (uploading)
	unsigned long	_sendTimer;	  //used for sending rate timing (downloading)
	unsigned long	_recvtimer;  //used for receiving rate timing (uploading)
	Mutex			_sendMutex;   //used for synchronization when limiting download speed
	Mutex			_recvMutex;   //used for synchronization when limiting upload speed
	
	static Mutex	 _mutexAccess;   //used for synchronization when limiting upload speed
	
	typedef vector<FtpConnection*> ftpConn_v;

	static ftpConn_v _ftpAliveConnetion;


	//Format strings for the log files
	char _dateformat[64];      //format string for log date field

private:
	void setFilename(std::string &PathAndName, const char *logfile, const char *basepath = NULL);
	bool	_isUninitialized;

public:
	// value got from isacontent
	uint32		_ftpAllowAnonymous;
	uint32		_ftpSkipAuthentication;

public:
	//The default root directory for the site
	std::string _defaultRoot;
	std::string _cwd;
	std::string	_strFtpBindIp;

	
	//Program Information
	char _progName[64];        //name of the program (Ex. RDS)
	char _progVersion[32];     //program version (as a string)
	char _coreVersion[32];     //core version (as a string)
	//This is the version of the base code.


public:
	static bool _bQuit; //stores if the user quit (!0 = exit the server)
	
	ZQ::common::Mutex _lockConCount;//the lock protected variable _nConnections
	static volatile long _nConnections;    //stores the number of current client connections

	static unsigned long timeval();
	static unsigned long timeDiff(const unsigned long start, const unsigned long end);
	static float timeDiff2Sec(const unsigned long start, const unsigned long end);

	static bool formatString(std::string& str, const char *format, const char *specifiers, const char *values[], int nparams, char escchar = '%');

	static wstring  StdString2WString(const std::string &src);

	static void		DeleteArchivedFiles(const char* TargetDir);
	// manutil call back
//	static MANSTATUS ConnectionInfoCallBack(WCHAR*, WCHAR**, DWORD*);
public:

	// config items
	uint32		_ftpListenPort;
	uint32		_maxConnection;

#ifdef ZQ_OS_MSWIN
	HANDLE		_stopEvent;
#else
	sem_t		_stopSem;
#endif
	NativeThreadPool&	_gThreadPool;
	SOCKET		_serverSd;    //socket desc of the client connection
	FileIoFactory*                          _pFileIoFactory;
};

#endif //__RTENV_H__
