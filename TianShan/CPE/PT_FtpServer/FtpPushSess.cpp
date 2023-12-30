

#include "FtpPushSess.h"
#include "FtpSite.h"
#include <TianShanDefines.h>

#define DEF_PUSH_VOLUME		""

#define MOLOG	glog

#define FtpPushSess			"FtpPushSess"

uint32 _dwSocketReadTimeoutSecs = 10;


FtpsPushXfer::FtpsPushXfer(FtpConnection& ftps, FtpSite& site, FtpSock* pasv_sock, FtpSock *port_sock, NativeThreadPool& Pool, context_t* pContext)
:FtpsXfer(ftps, site, pasv_sock, port_sock, Pool, pContext)
{
	_ulFileSize = 0;
}

FtpsPushXfer::~FtpsPushXfer()
{
	if (_dataSd!=SOCK_INVALID)
		FtpSock::Close(_dataSd);
}

bool FtpsPushXfer::recvFile()
{
	// argument for send response to client
	char cmd[] ="STOR";
	const char *args[2] = {cmd, NULL};
	char msg[MAX_PATH];	
	char filepath[MAX_PATH];
	char netId[MAX_PATH];
	std::string  strFilename;

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

		_strLogHint = strFilename;
	}

	//set for the log header
	_conn._strFilename = strFilename;

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
	
	_context.type = 'I';// binary mode
	
	//create the data connection
	if (_context.flagpasv != 0)
	{
		//wait for FTPS_STDSOCKTIMEOUT seconds to see if
		//_sock.Accept() will block.
		if (_pasv_sock->CheckStatus(_dwSocketReadTimeoutSecs) <= 0)
		{
			_conn.eventHandler(0, 0,"Can't open data connection","425",1,1,0);
			MOLOG(Log::L_ERROR, CLOGFMT(FtpPushSess, "Can't open pasv data connection"));
			
			return false;
		}
		//accept the connection
		if ((_dataSd = _pasv_sock->Accept()) == SOCK_INVALID)
		{
			_conn.eventHandler(2,args,"Can't open data connection","425",1,1,0);
			MOLOG(Log::L_ERROR, CLOGFMT(FtpPushSess, "Can't open pasv data connection"));

			return false;
		}
	}
	else
	{
		//open a connection (as a client)
		if ((_dataSd = _port_sock->OpenClient(_context.portaddr,_context.portport,_dwSocketReadTimeoutSecs)) == SOCK_INVALID)
		{
			_conn.eventHandler(0, 0, "Can't open data connection","425",1,1,0);
			MOLOG(Log::L_ERROR, CLOGFMT(FtpPushSess, "Can't open data connection"));
			
			return false;
		}
	}
	
	//
	// check client if is valid(just check the data conection client ip if eque control connection client ip)
	//
	if (checkFXP(1) == 0)
	{
		_conn.eventHandler(2,args,"Can't open data connection","426",1,1,0);
		MOLOG(Log::L_ERROR, CLOGFMT(FtpPushSess, "Can't open data connection, client ip is diffrent with control connection"));
		return false;
	}

	MOLOG(Log::L_INFO, CLOGFMT(FtpPushSess, "start uploading, validating ticket for file: %s"), strFilename.c_str());
	
	//
	// make the contentkey, and add the to 
	//
	_strContentKey = _site.makeContentKey(netId,DEF_PUSH_VOLUME, strFilename.c_str());
	if(!_site.addPushXfer(_strContentKey.c_str(), this))
	{
		sprintf(msg, "content %s already exists",_strContentKey.c_str());
		_conn.eventHandler(0, 0,msg,"425",1,1,0);
		MOLOG(Log::L_ERROR, CLOGFMT(FtpPushSess, "[%s] content %s already exists"), _strLogHint.c_str(), _strContentKey.c_str());
		return false;
	}
	//
	// validate the session
	//
	if (!_site._validateCB||!_site._validateCB(_site._pCbCtx, netId,DEF_PUSH_VOLUME, strFilename.c_str()))
	{
		sprintf(msg, "No ticket information for ingest %s", _context.path);
		_conn.eventHandler(0, 0,msg,"425",1,1,0);
		MOLOG(Log::L_ERROR, CLOGFMT(FtpPushSess, "[%s] No ticket information for ingest %s"), _strLogHint.c_str(), _context.path);
		_site.removePushXfer(_strContentKey.c_str());
		return false;
	}
	
	sprintf(msg, "Opening BINARY mode data connection for %s", _context.path);
	_conn.eventHandler(2,args,msg,"150",1,1,0);

	//initialize the transfer rate parameters
	//
	_bytesSinceRateUpdt = 0;
	_bytesXfered		= 0;

	_dwStart = ZQTianShan::now();
	MOLOG(Log::L_INFO, CLOGFMT(FtpPushSess, "Data connection established, ready to ingest file: %s"), strFilename.c_str());

//	MOLOG(Log::L_DEBUG, CLOGFMT(FtpPushSess, "[%s]recvFile() data connection count(increase)[%lld]"), strFilename.c_str(), _site._nConnections);
//	_conn.incDataThreads();
	return true;
}

unsigned int FtpsPushXfer::read(void* pBuf, unsigned int nReadBytes)
{
	int nRet = 0;
	unsigned int j;
	for(j=0;(!_conn.isAbor() && j<_dwSocketReadTimeoutSecs);j++)
	{
		nRet = FtpSock::CheckStatus(_dataSd, 1, 0);
		if (nRet > 0)
		{
			break;
		}
		else if (nRet<0)
		{
			MOLOG(Log::L_INFO, CLOGFMT(FtpPushSess, "[%s] data sending stopped, upload finished"), _strLogHint.c_str());
			return 0;
		}
	}
		
	if (j >= _dwSocketReadTimeoutSecs)
	{
		MOLOG(Log::L_INFO, CLOGFMT(FtpPushSess, "[%s] Read data timeout in %d seconds, ingest stopped"), _strLogHint.c_str(), _dwSocketReadTimeoutSecs);
		return 0;
	}
		
	if (_conn.isAbor())
	{
		MOLOG(Log::L_INFO, CLOGFMT(FtpPushSess, "[%s] Control connection aborted the transfer, ingest stopped"), _strLogHint.c_str());
		return 0;
	}

	int nLen = recvData((char*)pBuf, nReadBytes);
	if (nLen<=0)
	{
		MOLOG(Log::L_INFO, CLOGFMT(FtpPushSess, "[%s] data sending stopped, upload finished"), _strLogHint.c_str());
		return 0;
	};

	_ulFileSize+=nLen;

	return nLen;
}

void FtpsPushXfer::close(bool succ, const char* szErr)
{
	if(succ)
		_conn.eventHandler(0,NULL,"Ingest finished successfully","226",1,1,0);
	else
		_conn.eventHandler(0,NULL,"Ingest stopped by error","226",1,1,0);
	
	int64 dwEnd = ZQTianShan::now();  // for count upload time	
	int64 dwActualFtpBitrate = 0;
	if (dwEnd > _dwStart)
	{
		dwActualFtpBitrate = (_ulFileSize*1000*8/(dwEnd-_dwStart));
	}

	MOLOG(Log::L_INFO, CLOGFMT(FtpPushSess, "[%s] uploaded file size (%lld)bytes, ftpactualspeed(%dbps), upload spent time(%dms)"), 
	   _strLogHint.c_str(), _ulFileSize, dwActualFtpBitrate, dwEnd-_dwStart);

	//reduce the data connection count
//	MOLOG(Log::L_DEBUG, CLOGFMT(FtpPushSess, "[%s]colse() data connection count(reduce)[%lld]"), _strLogHint.c_str(), _site._nConnections);
//	_conn.decDataThreads();

	//remove this from the map
	MOLOG(Log::L_INFO, CLOGFMT(FtpPushSess, "[%s] to release PushSession"), _strLogHint.c_str());
	_site.removePushXfer(_strContentKey.c_str());

	std::string strLogHint = _strLogHint;
	
	try
	{
		delete this;

		MOLOG(Log::L_INFO, CLOGFMT(FtpPushSess, "[%s] PushSession released"), strLogHint.c_str());
	}
	catch (...)
	{
		MOLOG(Log::L_WARNING, CLOGFMT(FtpPushSess, "[%s] unknown exception caught while delete FtpsPushXfer object, this[0x%08x]"),
			strLogHint.c_str(), this);
	}	
}


