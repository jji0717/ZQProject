

#include "FTPSource.h"
#include "Log.h"
#include "urlstr.h"
#include "ErrorCode.h"
#include "FtpClient.h"
#include "FTPMSClient.h"
#include "EncodeConvert.h"


#define FTPSrc			"FTPSrc"


using namespace ZQ::common;

#define MOLOG			(*_pLog)

namespace ZQTianShan {
	namespace ContentProvision {


FTPIOSource::FTPIOSource()
	:_pFTPDownloader(new FTPMSClient())
{
	_nOutputCount = 1;
	_nInputCount = 0;
	_fileSize = 0;

	for(int i=0;i<_nOutputCount;i++)
	{
		OutputPin pin;
		pin.nNextPin = 0;
		pin.pNextFilter = 0;		
		_outputPin.push_back(pin);
	}
	_bDriverModule = false;

	_llProcBytes = 0;
	_nBandwidthBps = 0;
	
	_connectionInterval = 30000;//ms
	_bStop = false;
	_bPassiveMode = false;

	_llReadOffset = 0;
	_bSourceUrlUTF8 = false;
}

FTPIOSource::~FTPIOSource()
{
	if (_pFTPDownloader.get())
	{
		_pFTPDownloader->closeFile();
		_pFTPDownloader->close();
		_pFTPDownloader.reset(0);
	}
}

bool FTPIOSource::Init()
{
	MOLOG(Log::L_INFO, CLOGFMT(FTPSrc, "[%s] Enter Init()"), _strLogHint.c_str());

	ZQ::common::URLStr srcUrl(_srcURL.c_str());
	std::string strServer = srcUrl.getHost();
	std::string strOrgFile = srcUrl.getPath();
	std::string strUserName = srcUrl.getUserName();
	std::string strPassword = srcUrl.getPwd();
	int nPort = srcUrl.getPort();

	if (_bDecodeSourceURL)
	{
		char strTemp[512] = "";
		memset(strTemp, 0, 512);

		URLStr::decode(strOrgFile.c_str(), strTemp, 512);
		strOrgFile = strTemp;
	}

/*	Andy said, only cifs need to convert, 
	if (_bSourceUrlUTF8)
	{
		// convert source filename from utf8 to gb2312
		std::string strFileGB;
		if (EncodeConvert::utf8_to_ansi(strOrgFile, strFileGB))
			strOrgFile = strFileGB;
	}
*/
	MOLOG(Log::L_INFO, CLOGFMT(FTPSrc, "[%s] url=%s, Host=%s, File=%s, User=%s, Password=%s, Port=%d"),
		_strLogHint.c_str(), _srcURL.c_str(), strServer.c_str(), strOrgFile.c_str(), strUserName.c_str(), strPassword.c_str(), nPort);
    _strOrgFile = strOrgFile;

	FTPClient::FTPMode mode;
	if (_bPassiveMode)
		mode = FTPClient::passive;
	else
		mode = FTPClient::active;

	// add by zjm
	_pFTPDownloader->setLog(_pLog);
	_pFTPDownloader->setTransmitMode(FTPClient::binary);
	_pFTPDownloader->setFTPMode(mode);
	_pFTPDownloader->setIoTimeout(_connectionInterval);

	if (!_pFTPDownloader->open(strServer, nPort, strUserName, strPassword, _strLocalNetIf)) // open fail
	{
		std::string strErr = "failed to connect to url " + _srcURL + " with error: " + _pFTPDownloader->getLastError();

		MOLOG(Log::L_ERROR, CLOGFMT(FTPSrc, "[%s] %s"), _strLogHint.c_str(), strErr.c_str());
        SetLastError(strErr, ERRCODE_INVALID_SRCURL);

		return false;
	}
	MOLOG(Log::L_INFO, CLOGFMT(FTPSrc, "[%s] ftp server connected"), _strLogHint.c_str());
	
	_fileSize = _pFTPDownloader->getFileSize(strOrgFile);
	
	GetGraph()->setTotalBytes(_fileSize);
	MOLOG(Log::L_INFO, CLOGFMT(FTPSrc, "[%s] the source file size is %lld bytes"), _strLogHint.c_str(),_fileSize);

	if (!_pFTPDownloader->openFile(strOrgFile)) // open file fail
	{
		std::string strErr = "openfile " + strOrgFile + " failed with error " + _pFTPDownloader->getLastError();

		MOLOG(Log::L_ERROR, CLOGFMT(FTPSrc, "[%s] %s"), _strLogHint.c_str(), strErr.c_str());

		SetLastError(strErr, ERRCODE_INVALID_SRCURL);

		_pFTPDownloader->close();
		return false;
	}
	
	MOLOG(Log::L_INFO, CLOGFMT(FTPSrc, "[%s] file %s open succeed"), _strLogHint.c_str(), strOrgFile.c_str());

	_bitrateCtrl.setBitrate(_nBandwidthBps);
	_bitrateCtrl.start();

	return true;
}

void FTPIOSource::Stop()
{
	_bStop = true;
}

void FTPIOSource::Close()
{
	_bStop = true;
	if (_pFTPDownloader.get())
	{
		_pFTPDownloader->closeFile();
		_pFTPDownloader->close();
		_pFTPDownloader.reset(0);
	}
}

void FTPIOSource::endOfStream()
{
	
}

const char* FTPIOSource::GetName()
{
	return SOURCE_TYPE_FTP;
}

int64 FTPIOSource::getProcessBytes()
{
	return _llProcBytes;
}

MediaSample* FTPIOSource::GetData(int nOutputIndex)
{
	if (nOutputIndex != 0)
	{
		MOLOG(Log::L_INFO, CLOGFMT(FTPSrc, "[%s] OutputIndex != 0"), _strLogHint.c_str());
		return NULL;
	}
	
	if (_bStop)
	{
		MOLOG(Log::L_INFO, CLOGFMT(FTPSrc, "[%s] require to stop while GetData()"), _strLogHint.c_str());

		// set the total bytes to processed bytes
		//GetGraph()->setTotalBytes(_llProcBytes);
		return NULL;
	}

	MediaSample* pSample = GetGraph()->allocMediaSample();
	if (!pSample)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(FTPSrc, "[%s] failed to alloc meida sample"), _strLogHint.c_str());

		// set the total bytes to processed bytes
		//GetGraph()->setTotalBytes(_llProcBytes);
		return NULL;
	}

	int dwBytesRead = 0;
	if (!_pFTPDownloader->readFile((char*)pSample->getPointer(), pSample->getBufSize(), dwBytesRead)) // read fail
	{
/*
		// TODO: log here
		if (_fileSize && _llProcBytes < _fileSize) //not get the total bytes
		{
			std::string strErr = "GetData()  read file have a error: " + _pFTPDownloader->getLastError();
			MOLOG(Log::L_ERROR, CLOGFMT(FTPSrc, "[%s] %s"), _strLogHint.c_str(), strErr.c_str());
			SetLastError(std::string(strErr), ERRCODE_READ_SOURCE);
		}

		GetGraph()->freeMediaSample(pSample);
		return NULL;
*/
	}

	//bandwidth control
	uint32 nRet = _bitrateCtrl.control(_llProcBytes);
	if (nRet > 1000)
	{
		MOLOG(Log::L_DEBUG, CLOGFMT(FTPSrc, "[%s] bitrate control waiting time %dms"), _strLogHint.c_str(), nRet);
	}

	if (dwBytesRead)
	{
		pSample->setDataLength(dwBytesRead);
		pSample->setOffset(_llProcBytes);

		_llProcBytes += dwBytesRead;

		return pSample;
	}
	else
	{
		MOLOG(Log::L_INFO, CLOGFMT(FTPSrc, "[%s] downloaded size[%lld] total size [%lld]"), _strLogHint.c_str(), _llProcBytes, _fileSize);
		if (_fileSize && _llProcBytes < _fileSize)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(FTPSrc, "[%s] downloaded size[%lld] is smaller than source content[%lld]"), _strLogHint.c_str(), _llProcBytes, _fileSize);
			std::string msg = "downloaded size is smaller than source content";
			SetLastError(std::string(msg), ERRCODE_READ_SOURCE);
		}

		GetGraph()->freeMediaSample(pSample);
		return NULL;
	}
}

void FTPIOSource::setMaxBandwidth(unsigned int nBandwidthBps)
{
	_nBandwidthBps = nBandwidthBps;
	
	MOLOG(Log::L_INFO, CLOGFMT(FTPSrc, "[%s] set MaxBandwidth=%d bps"), _strLogHint.c_str(), nBandwidthBps);
}

void FTPIOSource::setLocalNetworkInterface( const char* szLocalNetIf )
{
	_strLocalNetIf = szLocalNetIf;
}
bool FTPIOSource::readbuf(char* pBuf, unsigned int bufLen, unsigned int& rcvLen)
{
	if (!_pFTPDownloader->readFile((char*)pBuf, bufLen, (int&)rcvLen)) // read fail
	{
		return false;
	}
	_llReadOffset += rcvLen;
  return true;
}
bool FTPIOSource::seek(int64 offset, int pos)
{
	if(pos == 0)
	{
		_llReadOffset =  offset;
	}
	else if(pos == 1)
	{
		_llReadOffset += offset;
	}
	else
	{
		_llReadOffset = offset ;
	}

	if (!_pFTPDownloader->openFile(_strOrgFile, ZQTianShan::ContentProvision::FTPClient::toRead, _llReadOffset)) // open file fail
	{
		return false;
	}
	return true;
}
void FTPIOSource::setDecodeSourceURL(bool bDecodeSourceURL)
{
   _bDecodeSourceURL = bDecodeSourceURL;
}

void FTPIOSource::setSourceUrlUTF8( bool bIsUTF8 )
{
	_bSourceUrlUTF8 = bIsUTF8;
}
}}

