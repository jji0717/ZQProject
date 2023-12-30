// FtpXferEx.cpp: implementation of the FtpXferEx class.
//
//////////////////////////////////////////////////////////////////////
#include "CECommon.h"
#include "FtpXferEx.h"
#include "FtpSite.h"
#include "IOInterface.h"
#include "CPECfg.h"
#include <sstream>
#include <iomanip>
#include "FileIo.h"

#define XferEx			"XferEx"
#define MOLOG					glog

using namespace ZQ::common;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FtpXferEx::FtpXferEx(FtpConnection& ftps, FtpSite& site, FtpSock* pasv_sock, FtpSock *port_sock, ZQ::common::NativeThreadPool& Pool, context_t* pContext)
:FtpsXfer(ftps, site, pasv_sock, port_sock, Pool, pContext)
{
	_pFileIoFactory = site._pFileIoFactory;
	_pIOInterface = new IOInterface(_pFileIoFactory);
}

FtpXferEx::~FtpXferEx()
{
	if (_pIOInterface)
	{
		delete _pIOInterface;
		_pIOInterface = NULL;
	}
}

bool FtpXferEx::init()
{
	if (!_pIOInterface)
		return false;
	return true;
}
bool FtpXferEx::sendFile()
{
	const char *args[2], cmd[] = "RETR";

	bool bRet = false;
	do
	{
		int64 lFileSize=0;
		char buffer[MAX_PATH], *filepath=buffer;
		unsigned long ltime1, ltime2;
		int64 lBytes = 0;    //number of bytes sent
		
		args[0] = cmd; args[1] = _context.path;

		//allocate memory/build the full directory/file path
		if (!FSUtils::buildPath(buffer, MAX_PATH, _context.userroot, _context.cwd, _context.path))
		{
			_conn.eventHandler(2,args,"ERROR: out of memory.","425",1,1,1);
			break;
		}

		if (!_pIOInterface)
		{
			_conn.eventHandler(2,args,"ERROR: internal server error, vstream not ready.","425",1,1,1);
			MOLOG(Log::L_ERROR, CLOGFMT(XferEx, "User[%s]: failed to download file because the vstream not ready."), 
				_conn.getLogin());
			break;
		}

		lFileSize = _pIOInterface->GetFileSize(filepath);
		if (!lFileSize)
		{
			MOLOG(Log::L_WARNING, CLOGFMT(XferEx, "User[%s] failed to find file[%s]"), 
				_conn.getLogin(), filepath);
			
			_conn.eventHandler(2,args,"ERROR: file not exist.","425",1,1,1);
			break;
		}

		MOLOG(Log::L_INFO, CLOGFMT(XferEx, "File [%s] size is [%lld] bytes"), 
			filepath, lFileSize);

		if (!_pIOInterface->Open(filepath, FileIo::ACCESS_READ))
		{
				MOLOG(Log::L_WARNING, CLOGFMT(XferEx, "User[%s] failed to open file[%s]"), 
				_conn.getLogin(), filepath);

			_conn.eventHandler(2,args,"Failed to open file.","550",1,1,0);
			break;
		}

		if (_context.flagpasv != 0)
		{
			//wait for FTPS_STDSOCKTIMEOUT seconds to see if
			//_sock.Accept() will block.
			if (_pasv_sock->CheckStatus(FTPS_STDSOCKTIMEOUT) <= 0)
			{
				MOLOG(Log::L_WARNING, CLOGFMT(XferEx, "User[%s] CheckStatus::Can't open data connection for file[%s]"), 
				_conn.getLogin(), filepath);
				_site.addCredits(filepath,_conn.getUserID(),_conn.getLogin(),(int)(lFileSize),0); //restore the credits
				_conn.eventHandler(2,args,"Can't open data connection","425",1,1,0);	
				_pIOInterface->Close();
				break;
			}
			//accept the connection
			if ((_dataSd = _pasv_sock->Accept()) == SOCK_INVALID)
			{
				MOLOG(Log::L_WARNING, CLOGFMT(XferEx, "User[%s] Accept::Can't open data connection for file[%s]"), 
					_conn.getLogin(), filepath);
				_site.addCredits(filepath,_conn.getUserID(),_conn.getLogin(),(int)(lFileSize),0); //restore the credits
				_conn.eventHandler(2,args,"Can't open data connection","425",1,1,0);
				_pIOInterface->Close();
				break;
			}
		}
		else
		{
			//open a connection (as a client)
			if ((_dataSd = _port_sock->OpenClient(_context.portaddr,_context.portport,FTPS_STDSOCKTIMEOUT)) == SOCK_INVALID)
			{
				MOLOG(Log::L_WARNING, CLOGFMT(XferEx, "User[%s] OpenClient::Can't open data connection for file[%s]"), 
				_conn.getLogin(), filepath);
					
				_site.addCredits(filepath,_conn.getUserID(),_conn.getLogin(),(int)(lFileSize),0); //restore the credits
				_conn.eventHandler(2,args,"Can't open data connection","425",1,1,0);
				_pIOInterface->Close();
				break;
			}
		}

		if (checkFXP(0) == 0)
		{
			MOLOG(Log::L_WARNING, CLOGFMT(XferEx, "User[%s] checkFXP::data connection close for file[%s]"), 
				_conn.getLogin(), filepath);
				
			_site.addCredits(filepath,_conn.getUserID(),_conn.getLogin(),(int)(lFileSize),0); //restore the credits
			_conn.eventHandler(2,args,"Can't open data connection","426",1,1,0);
			_pIOInterface->Close();
			FtpSock::Close(_dataSd);
			break;  //FXP is not allowed
		}

		_context.type = 'I';
		//display which mode of transfer is being used.
		if (_context.type == 'A')
		{
			sprintf(buffer,"Opening ASCII mode data connection for %s ("FMT64" bytes).",_context.path, lFileSize);
			_conn.eventHandler(2,args,buffer,"150",1,1,0);
		}
		else
		{
			sprintf(buffer,"Opening BINARY mode data connection for %s ("FMT64" bytes).",_context.path,lFileSize);
			_conn.eventHandler(2,args,buffer,"150",1,1,0);
		}

		ltime1 = _site.timeval();   //time how long it takes to send the data
		lBytes = sendFileData();
		ltime2 = _site.timeval();   //stop timing

		//close the data connection and file descriptor
		FtpSock::Close(_dataSd);	
		_pIOInterface->Close();

		//reset the current transfer rate to 0
		_site.setCurrentXferRate(_conn.getUserID(),0);

		_conn._lastActive = time(NULL); //update the last active time

		//if the transfer was successful
		if ((lBytes + _context.restoffset) >= lFileSize)
		{
			//update the download statistics
			_site.updateDLStats(_conn.getUserID(),_conn.getLogin(),(int)(lFileSize),(int)(lBytes/_site.timeDiff2Sec(ltime1,ltime2)));
		}
		else
		{
			//restore the credits
			_site.addCredits(filepath,_conn.getUserID(),_conn.getLogin(),(int)(lFileSize),0);
		}

		//TODO: GENERALIZE THIS
		snprintf(buffer,79,"[Bytes: "FMT64"][Time: %.2f s][Speed: %.2f K/s]",lBytes,_site.timeDiff2Sec(ltime1,ltime2),(lBytes/1024.0)/_site.timeDiff2Sec(ltime1,ltime2));
		buffer[79] = '\0';
		_conn.eventHandler(2,args,buffer,"226",1,1,0);

		bRet = true;
	}while(0);

	//decrement the data transfer thread counter
	if (_conn._nDataThreads > 0)
		_conn._nDataThreads --;

	return bRet;
}

int64 FtpXferEx::sendFileData()
{
	MOLOG(Log::L_INFO, CLOGFMT(XferEx, "User[%s] set max download bitrate to %d Bps"), 
		_conn.getLogin(), _maxDlSpeed);

	int nMaxBitrate = _maxDlSpeed*8;

	if (!_pIOInterface->ReserveBandwidth(nMaxBitrate))
	{
		return 0;
	}

	int packetsize = _pIOInterface->GetRecommendedIOSize();
	char* packet = new char[packetsize];

	long nsent;
	int64 lBytes = 0;
	int packetlen = 0;
//	int tmppacketlen = 0;

	//move the file pointer to the proper restart position if REST was specified
	if (_context.restoffset > 0)
		_pIOInterface->Seek(_context.restoffset,FileIo::POS_BEGIN);// BaseIOI::FP_BEGIN);

	//initialize the transfer rate parameters
	_site.setCurrentXferRate(_conn.getUserID(),0);
	_bytesSinceRateUpdt = 0;
	_lastXferRateUpdt = _site.timeval();
	_bytesXfered = 0;
	_xferTimer = _site.timeval();

	do
	{
		if (_conn.isAbor())
		{
			MOLOG(Log::L_WARNING, CLOGFMT(XferEx, "User[%s] abort transfer, stop sending file"), 
				_conn.getLogin());
			break;
		}

		packetlen = _pIOInterface->Read(packet, packetsize);
		if ((nsent = sendData(packet,packetlen)) < 0)
		{
			MOLOG(Log::L_WARNING, CLOGFMT(XferEx, "User[%s] abort transfer, stop sending file"), 
				_conn.getLogin());
			_pIOInterface->Close();
			break;
		}
		else
		{
			lBytes += nsent;
		}
	} while(packetlen == packetsize);

	if (packet)
		delete packet;

	_pIOInterface->ReleaseBandwidth();

	return(lBytes);
}

int64 FtpXferEx::GetFileSize(const char* szFile)
{
	if (!_pIOInterface)
		return 0;

	return _pIOInterface->GetFileSize(szFile);
}

bool FtpXferEx::sendList() 
{
	char buffer[MAX_PATH];
	unsigned long ltime1, ltime2;
	size_t nbytes = 0;    //number of bytes sent in the dir listing

	const char* args[2] = {"LIST", NULL};
	args[1] = _context.path;

	bool bRet = false;
	do
	{
		//create the data connection
		if (_context.flagpasv != 0) {

			//wait for FTPS_STDSOCKTIMEOUT seconds to see if
			//_sock.Accept() will block.

			if (_pasv_sock->CheckStatus(FTPS_STDSOCKTIMEOUT) <= 0) {
				_conn.eventHandler(2,args,"Can't open data connection.","425",1,1,0);
				break;
			}

			//accept the connection
			if ((_dataSd = _pasv_sock->Accept()) == SOCK_INVALID) {
				_conn.eventHandler(2,args,"Can't open data connection.","425",1,1,0);
				break;
			}
		}
		else {
			//open a connection (as a client)

			if ((_dataSd = _port_sock->OpenClient(_context.portaddr,_context.portport,FTPS_STDSOCKTIMEOUT)) == SOCK_INVALID) {
				_conn.eventHandler(2,args,"Can't open data connection","425",1,1,0);
				break;
			}
		}

		if (checkFXP(0) == 0) {
			_conn.eventHandler(2,args,"Can't open data connection","426",1,1,0);
			FtpSock::Close(_dataSd);
			break;  //FXP is not allowed for the user
		}

		_conn.eventHandler(2,args,"Opening ASCII mode data connection for directory listing.","150",1,1,0);

		ltime1 = _site.timeval();   //time how long it takes to send the listing
		sendListData(_context, nbytes);
		ltime2 = _site.timeval();   //stop timing

		//close the data connection
		FtpSock::Close(_dataSd);

		_conn._lastActive = time(NULL); //update the last active time

		//TODO: GENERALIZE THIS
		snprintf(buffer,79,"[Bytes: %ld][Time: %.2f s][Speed: %.2f K/s]",nbytes,_site.timeDiff2Sec(ltime1,ltime2),(nbytes/1024.0)/_site.timeDiff2Sec(ltime1,ltime2));
		buffer[79] = '\0';
		_conn.eventHandler(2,args,buffer,"226",1,1,0);

		bRet = true;
	}while(0);

//	MOLOG(Log::L_DEBUG, CLOGFMT(XferEx, "sendList() data connection count(reduce)[%lld]"), _site._nConnections);
	//decrement the list transfer thread counter
//	_conn.decDataThreads();

	return bRet;
}

//#ifdef ZQ_OS_MSWIN
//void FtpXferEx::sendListData(context_t& ctxt,size_t& nbytes) 
//{
//	
// 	std::string listline;
//
//	// List all the files
//	WIN32_FIND_DATA data;
//
//	VHANDLE handle = _pIOInterface->FindFirstFile("*", data);
//	if(handle == INVALID_HANDLE_VALUE) {
//		return;
//	}
//
//	char packet[FTPSXFER_MAXPACKETSIZE];
//	size_t packetlen = 0;
//	do {
//		if (_conn.isAbor()) {  //check for ABOR
//			return;
//		}
//		
//		buildListLine(listline, data);
//		nbytes += addToSendBuffer(packet, FTPSXFER_MAXPACKETSIZE, packetlen, listline.c_str(), listline.length(), ctxt.bencdata);
//
//	} while (_pIOInterface->FindNextFile(handle, data));
//
//	_pIOInterface->FindClose(handle);
//
//	nbytes += addToSendBuffer(packet,FTPSXFER_MAXPACKETSIZE, packetlen,"",0,ctxt.bencdata,1);
//}
//
//
//bool FtpXferEx::buildListLine(std::string& line, WIN32_FIND_DATA& data) 
//{
//	line.clear();
//
//	if (strchr(_context.options,'N') != NULL) {
//		line = data.cFileName;
//		line +="\r\n";        
//
//		return true;
//	}
//
//	LARGE_INTEGER fileSize = {data.nFileSizeLow, data.nFileSizeHigh};
//
//	SYSTEMTIME time;
//	FileTimeToSystemTime(&data.ftCreationTime, &time);
//
//	std::ostringstream oss;
//
//	oss << std::setw(2) << std::setfill('0')
//		<< time.wDay << '-' 
//		<< std::setw(2) << std::setfill('0')
//		<< time.wMonth << '-'
//		<< std::setw(4)
//		<< time.wYear << "  "
//		<< std::setw(2) << std::setfill('0')
//		<< time.wHour << ':'
//		<< std::setw(2) << std::setfill('0')
//		<< time.wMinute << ':'
//		<< std::setw(2) << std::setfill('0')
//		<< time.wSecond << '\t'
//		<< std::setw(10) << std::right << std::setfill(' ')
//		<< fileSize.QuadPart << "  "
//		<< std::setw(50) << std::left
//		<< data.cFileName << "\r\n";
//		
//	line = oss.str();
//	
//	return true;
//}
//
//#else
//void FtpXferEx::sendListData(context_t& ctxt,size_t& nbytes)//not complete
//{		
// 	return;
//}
//
//
//#endif
//
//bool FtpXferEx::buildListLine(
//							 std::string& line, 
//							 const char *fullpath, 
//							 const char *filename, 
//							 const int flagdir, 
//							 const char *thisyear) 
//{
//
//	line = "";
//
////	FSUtils::fileInfo_t finfo;
//	char filepath[MAX_PATH];
//
//	//do not display the "." and ".." entries
//	if (strcmp(filename,".") == 0 || strcmp(filename,"..") == 0)
//		return false;
//
////	if (strchr(fullpath,'*') != NULL)   //check if wildcard
////		FSUtils::getDirPath(filepath,strlen(fullpath)+1,fullpath); //get only the directory name
////	else
////		strcpy(filepath,fullpath);
//
////	FSUtils::checkSlashEnd(filepath,strlen(fullpath)+strlen(filename)+2);
////	strcat(filepath,filename);
////	FSUtils::checkSlash(filepath);
//
////	if (_pBaseIO->getFileStats(filepath,&finfo) == 0)
////		return false;
//
//	//	if (finfo.mode & S_IFDIR) {
//	//		if ((flagdir & 1) == 0)
//	//			return false;   //only file lines should be returned
//	//	}
//	//	else if ((flagdir & 2) == 0) {
//	//		return false;   //only directory lines should be returned
//	//	}
//
//	if (strchr(_context.options,'N') != NULL)
//		line = filename;
//	else {
//		// build a full LIST line
//		char linebuf[MAX_PATH] ="", filename[MAX_PATH];
//		FSUtils::fileInfo_t finfo;
//
//		char *ptr, permissions[10], user[9], group[9], filetype;
//
//		if (_pIOInterface->getFileStats(filepath,&finfo) == 0)
//			return false;
//
//		FSUtils::getFileName(filename, strlen(filepath)+1, filepath);
//
//		filetype = (finfo.mode & S_IFDIR) ? 'd' : '-';
//
//		char timebuf[64];
//		strcpy(timebuf, _site.timeString("%Y %b %d %H:%M", finfo.timecreate).c_str());
//		if ((ptr = strrchr(timebuf,' ')) != NULL)
//		{
//			ptr++;
//			if (thisyear != NULL)
//			{
//				if (strncmp(timebuf,thisyear,4) != 0)
//				{
//					*ptr = ' ';
//					memmove(ptr+1,timebuf,4);     //overwrite the time of day with the year
//				}
//			}
//			ptr = timebuf + 5;  //move to the start of the month name
//			FSUtils::getPrmString(finfo.mode,permissions,sizeof(permissions));
//			FSUtils::getUsrName(finfo.userid,user,sizeof(user));
//			FSUtils::getGrpName(finfo.groupid,group,sizeof(group));
//			snprintf(linebuf, MAX_PATH,"%c%9s %3ld %-8s %-8s %10lld %s ",filetype,permissions,finfo.nlinks,user,group,finfo.size,ptr);
//
//			line = linebuf;
//			line += filename;
//		}
//	}
//
//	line +="\r\n";        //end the line
//
//	return true;
//}
