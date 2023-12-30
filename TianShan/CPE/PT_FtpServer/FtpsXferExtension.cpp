// FtpsXferExtension.cpp: implementation of the CFtpsXferExtension class.
//
//////////////////////////////////////////////////////////////////////
#define _START_PUSHCONTENT_AFTER_CONNECTION_

#include "stdafx.h"
#include "FtpsXferExtension.h"
#include "Ftp_Svr.h"


extern PushSessIMgr* gpMgr;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DWORD _dwSocketReadTimeoutSecs = 30;

FtpsXferExtension::FtpsXferExtension(FtpConnection& ftps, FtpSite& site, FtpSock* pasv_sock, FtpSock *port_sock, NativeThreadPool& Pool, context_t* pContext)
:FtpsXfer(ftps, site, pasv_sock, port_sock, Pool, pContext), _Pool(Pool)
{
	 _dwBufferBlockSize = DEF_BUFFERBLOCK_SIZE;
}

FtpsXferExtension::~FtpsXferExtension()
{

}

bool FtpsXferExtension::recvFile()
{
		// argument for send response to client
	char cmd[] ="STOR";
	const char *args[2] = {cmd, NULL};
	char msg[MAX_PATH];	
	char filepath[MAX_PATH];
	DWORD dwTime1, dwTime2;
	char netId[MAX_PATH];
	
	ULONGLONG ulFileSize = 0;	
	wchar_t wszUploadFileName[256];
	std::string  strFilename;
	DWORD dwFtpMaxTransferRate = 0;

	{
		const char* pBegin, * pEnd, *pTmp;
		pBegin = _context.path;
		pEnd = pBegin + strlen(_context.path);
		pTmp = pEnd - 1;

		while(*pTmp != '.' && *pTmp != '/' && pBegin < pTmp)
			pTmp--;

		if (*pTmp == '.')
			pEnd = pTmp;

		while(*pTmp != '/' && pBegin < pTmp)
			pTmp--;

		if (*pTmp == '/')
			pBegin = pTmp + 1;

		strFilename = pBegin;

		pEnd = pTmp--;
		pBegin = _context.path;
		while (*pTmp != '/' && pBegin < pTmp)
			pTmp--;
		if (*pTmp == '/')
			pBegin = pTmp + 1;
        memset(netId,0, MAX_PATH);
		strncpy(netId,pBegin,(pEnd-pBegin));

		swprintf(_sLogHeaderW, L"%S", strFilename.c_str());
	}

	//set for the log header
	_conn._strFilename = strFilename;

	LogMsg(Log::L_DEBUG, L"start uploading AE, Current Thread ID: [%04x]", GetCurrentThreadId());

	PushSessI* pPushSess = gpMgr->find(netId, strFilename.c_str());
	if (pPushSess == NULL)
	{
	//	sprintf(msg, "Session %s already exist, duplicate session id", strFilename.c_str());
		_conn.eventHandler(0, 0,msg,"425",1,1,0);
		LogMsg(Log::L_ERROR, L"There is no Session which path is %s and filename is %s", netId, strFilename.c_str());
		return false;
	}

	args[1] = strFilename.c_str();
	//allocate memory/build the full directory/file path
	FSUtils::buildPath(filepath, MAX_PATH, _context.userroot, _context.cwd, args[1]);

	//make argu
	{
		if (_context.mode == 1)
		{
			//if STOU was used (mode = 1), make sure the filename is unique
			strcpy(cmd,"STOU");
			int extnum =getUniqueExtNum(filepath);
			if (extnum != 0)
			{
				std::string fp = filepath;
				//build the new display path
				sprintf(filepath,"%s.%u",fp.c_str(), extnum);
				args[1] = filepath;
			}
		}
		else if (_context.mode == 2)
		{
			FSUtils::fileInfo_t fileinfo;
			//if APPE was used (mode = 2), set the reset offset to the end of the file
			strcpy(cmd,"APPE");
			if (FSUtils::getFileStats(filepath, &fileinfo) != 0)
				_context.restoffset = fileinfo.size;
		}
	}
	MultiByteToWideChar(CP_ACP, 0, filepath, -1, wszUploadFileName, sizeof(wszUploadFileName)/sizeof(WCHAR));



#ifdef _START_PUSHCONTENT_AFTER_CONNECTION_
	
	_context.type = 'I';// binary mode
	
	//create the data connection
	if (_context.flagpasv != 0)
	{
		//wait for FTPS_STDSOCKTIMEOUT seconds to see if
		//_sock.Accept() will block.
		if (_pasv_sock->CheckStatus(_dwSocketReadTimeoutSecs) <= 0)
		{
			_conn.eventHandler(0, 0,"Can't open data connection","425",1,1,0);
			LogMsg(Log::L_ERROR, L"Can't open data connection 1");
			
			_conn.decDataThreads();
			return false;
		}
		//accept the connection
		if ((_dataSd = _pasv_sock->Accept()) == SOCK_INVALID)
		{
			_conn.eventHandler(2,args,"Can't open data connection","425",1,1,0);
			LogMsg(Log::L_ERROR, L"Can't open data connection 2");
			
			_conn.decDataThreads();
			return false;
		}
	}
	else
	{
		//open a connection (as a client)
		if ((_dataSd = _port_sock->OpenClient(_context.portaddr,_context.portport,_dwSocketReadTimeoutSecs)) == SOCK_INVALID)
		{
			_conn.eventHandler(0, 0, "Can't open data connection","425",1,1,0);
			
			LogMsg(Log::L_ERROR, L"Can't open data connection 3");
			
			_conn.decDataThreads();
			return false;
		}
	}
	
	//
	// check client if is valid(just check the data conection client ip if eque control connection client ip)
	//
	if (checkFXP(1) == 0)
	{
		_conn.eventHandler(2,args,"Can't open data connection","426",1,1,0);
		LogMsg(Log::L_ERROR, L"Can't open data connection, client ip is diffrent");
		
		_conn.decDataThreads();
		return false;
	}
	
#endif

	//
//	pPushSess->setbuf(pMedia->getPointer()) ;
	do
	{
	
#ifndef _START_PUSHCONTENT_AFTER_CONNECTION_
		_context.type = 'I';// binary mode
		
		//create the data connection
		if (_context.flagpasv != 0)
		{
			//wait for FTPS_STDSOCKTIMEOUT seconds to see if
			//_sock.Accept() will block.
			if (_pasv_sock->CheckStatus(_dwSocketReadTimeoutSecs) <= 0)
			{
				_conn.eventHandler(0, 0,"Can't open data connection","425",1,1,0);
				LogMsg(Log::L_ERROR, L"Can't open data connection 1");
				
				break;
			}
			//accept the connection
			if ((_dataSd = _pasv_sock->Accept()) == SOCK_INVALID)
			{
				_conn.eventHandler(2,args,"Can't open data connection","425",1,1,0);
				LogMsg(Log::L_ERROR, L"Can't open data connection 2");
				
				break;
			}
		}
		else
		{
			//open a connection (as a client)
			if ((_dataSd = _port_sock->OpenClient(_context.portaddr,_context.portport,_dwSocketReadTimeoutSecs)) == SOCK_INVALID)
			{
				_conn.eventHandler(0, 0, "Can't open data connection","425",1,1,0);
				
				LogMsg(Log::L_ERROR, L"Can't open data connection 3");
				
				break;
			}
		}
		
		//
		// check client if is valid(just check the data conection client ip if eque control connection client ip)
		//
		if (checkFXP(1) == 0)
		{
			_conn.eventHandler(2,args,"Can't open data connection","426",1,1,0);
			LogMsg(Log::L_ERROR, L"Can't open data connection, client ip is diffrent");
			
			break; //FXP is not allowed
		}
#endif
		
		sprintf(msg, "Opening BINARY mode data connection for %s", _context.path);
		_conn.eventHandler(2,args,msg,"150",1,1,0);

		//tell pushsess to start
		pPushSess->start();

		//initialize the transfer rate parameters
		//
		_bytesSinceRateUpdt = 0;
		_bytesXfered		= 0;
		SetRcvBufSize(_dwBufferBlockSize);
		
		
		
		//
		//time how long it takes to receive the data
		//
		dwTime1   = GetTickCount();  // for count upload time

		void* buf;
		int nLen;
		bool	bMaxSpeedSet = false;
		while(!_conn.isAbor())
		{
			//if the data stops being sent
			int nRet = 0;
			for(unsigned int j=0;j<_dwSocketReadTimeoutSecs;j++)
			{
				nRet = FtpSock::CheckStatus(_dataSd, 1, 0);
				if (nRet == 0)
				{
					//
					// time out
					//
					if (!_conn.isAbor())
						continue;
					else
						break;
				}
				else if (nRet > 0)
				{
					break;
				}
				else
				{
					LogMsg(Log::L_DEBUG, L"data sending stopped, upload finish");
					break;
				}
			}

			if (j >= _dwSocketReadTimeoutSecs)
			{
				LogMsg(Log::L_DEBUG, L"read data timeout in %d seconds, upload stopped", _dwSocketReadTimeoutSecs);
				break;
			}

			if (nRet < 0)
			{
				break;
			}

			if (_conn.isAbor())
				break;

           
			MediaSample* pMedia = pPushSess->alloc();
 			if (!pMedia)
			{
				LogMsg(Log::L_DEBUG, L"Allocing memory failed."); 	
				break;
 			}

            buf = (void*)(pMedia->getPointer());
			nLen = (int)(pMedia->getBufSize());

			nLen = recvData((char*)buf, nLen);
			if (nLen<=0)
			{
				pPushSess->free(pMedia);
				LogMsg(Log::L_DEBUG, L"data sending stopped, upload finish");
				break;
			};
			ulFileSize += nLen;
         
			//add to receiver
			pMedia->setDataLength(nLen);
			pMedia->setOffset((unsigned int)ulFileSize, (unsigned int)(ulFileSize>>32));
			pPushSess->addData(pMedia);

			if (!bMaxSpeedSet)
			{
				dwFtpMaxTransferRate = pPushSess->getTransferBitrate();
				if (dwFtpMaxTransferRate)
				{
					//set bitrate
					_maxUlSpeed = dwFtpMaxTransferRate/8;
					bMaxSpeedSet = true;
					LogMsg(Log::L_DEBUG, L"max ftp speed set to %d bps", dwFtpMaxTransferRate);
				}
			}
		}
		
		
		_conn.eventHandler(0,NULL,"Upload finished successfully","226",1,1,0);
		
		if (_conn.isAbor())
		{
			LogMsg(Log::L_INFO, L"User cancelled uploading");
		}
		
	}while(0);

	dwTime2   = GetTickCount();  // for count upload time	
	DWORD dwActualFtpBitrate = 0;
	if (dwTime2 > dwTime1)
	{
		dwActualFtpBitrate = (DWORD)(ulFileSize*1000*8/(dwTime2-dwTime1));
	}

	pPushSess->endOfData();

	LogMsg(Log::L_INFO, L"Uploaded file size (%lld)bytes, ftpmaxspeed(%dbps), ftpactualspeed(%dbps), upload spent time(%dms)", 
		ulFileSize, dwFtpMaxTransferRate, dwActualFtpBitrate, dwTime2-dwTime1);

	_conn.decDataThreads();

	pPushSess->abort();
    
	return true;
}

void FtpsXferExtension::LogMsg(DWORD dwTraceLevel, const wchar_t* lpszFmt, ...)
{
    va_list    marker;
    WCHAR    wszMsg[4096];
	
    // Initialize access to variable arglist
    va_start(marker, lpszFmt);
	
    // Expand message
    _vsnwprintf(wszMsg, 4095, (WCHAR*)lpszFmt, marker);
    wszMsg[4095] = 0;
	
	glog(dwTraceLevel, L"%s: %s", _sLogHeaderW, wszMsg);
}

void FtpsXferExtension::SetRcvBufSize(int size)
{
	if (_dataSd == INVALID_SOCKET)
		return;
	
	int status = setsockopt(_dataSd, SOL_SOCKET, SO_RCVBUF, (char *)&size, sizeof(int));
	if (status == SOCKET_ERROR)
	{
		status = WSAGetLastError();
	}
}

