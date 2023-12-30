

#include "CIFSSource.h"
#include "Log.h"
#include "urlstr.h"
#include "ErrorCode.h"
#include "CStdFileIoFactory.h"
#include "EncodeConvert.h"

#ifdef ZQ_OS_LINUX
extern "C"
{
#include <sys/mount.h>
}
#endif

#define CIFSSrc			"CIFSSrc"

using namespace ZQ::common;

#define MOLOG			(*_pLog)

namespace ZQTianShan {
	namespace ContentProvision {


CIFSIOSource::CIFSIOSource()
{
	_nOutputCount = 1;
	_nInputCount = 0;
	_fileSize = 0;

	//_pFileIoFac = pFileIoFac;

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
	
	_bStop = false;
	_szMountPoint = "/mnt/";
	_strSysType = "cifs";
	_szOpt = "username=,password=";
	_localSourceFlag = false;
	_bSourceUrlUTF8 = false;
}

CIFSIOSource::~CIFSIOSource()
{
#ifdef ZQ_OS_LINUX
//	umount(_szMountPoint.c_str());
//	rmdir(_szMountPoint.c_str());
#endif
}

bool CIFSIOSource::Init()
{
	std::string filename;
	MOLOG(Log::L_INFO, CLOGFMT(CIFSSrc, "[%s] Enter Init()"), _strLogHint.c_str());

	if (!_pFileIoFac)
	{
		MOLOG(Log::L_INFO, CLOGFMT(CIFSSrc, "[%s] IO factory handle is NULL"), _strLogHint.c_str());
		return false;
	}

	//
	// decode %
	//
	{
		char strTemp[512] = "";
		memset(strTemp, 0, 512);

		URLStr::decode(_strFilename.c_str(), strTemp, 512);
		_strFilename = strTemp;
	}

	if (_bSourceUrlUTF8)
	{
#ifdef ZQ_OS_LINUX
		// do nothing, because if mounted, the chinese directory name already converted to utf8, so utf8 is fine
#else
		// convert source filename from utf8 to gb2312
		std::string strFileGB;
		if (EncodeConvert::utf8_to_ansi(_strFilename, strFileGB))
			_strFilename = strFileGB;
#endif
	}

    if (!_localSourceFlag)
    {  
#ifdef ZQ_OS_MSWIN
        std::string::size_type bpos = _strLogHint.find_last_of(LOGIC_FNSEPS);
		if (bpos!=std::string::npos)
			_szMountPoint += _strLogHint.substr(bpos+1);
		else
			_szMountPoint += _strLogHint;
#endif
		CStdFileIoFactory::createDirectory(_szMountPoint);
	    MOLOG(Log::L_INFO, CLOGFMT(CIFSSrc, "[%s] Mount path[%s],Share path[%s]"), _strLogHint.c_str(),_szMountPoint.c_str(), _szSharePath.c_str());
#ifdef ZQ_OS_LINUX
/*		int res = mount(_szSharePath.c_str(),_szMountPoint.c_str(),_strSysType.c_str(),MS_RDONLY,(void*)_szOpt.c_str());
		if (res < 0)
		{
			SetLastError(strerror(errno),ERRCODE_NTFS_CREATEFILE);
			MOLOG(Log::L_ERROR, CLOGFMT(CIFSSrc, "[%s] Failed to mount[%s] to target[%s] with option[%s], errStr:[%s]"),
				_strLogHint.c_str(),_szSharePath.c_str(),_szMountPoint.c_str(),_szOpt.c_str(),strerror(errno));
			return false;
		}*/
                if(_szMountPoint[_szMountPoint.size()-1] != FNSEPC)
                    _szMountPoint += FNSEPS;
        filename = _szMountPoint + _strFilename;
#endif
    }
	else
		filename = _strFilename;

	_pFileIo.reset(_pFileIoFac->create());
	if (!_pFileIo->openfile(filename.c_str(), 
		FileIo::ACCESS_READ,
		(FileIo::ShareMode)(FileIo::SHARE_READ | FileIo::SHARE_WRITE),
		FileIo::WAY_OPEN_EXISTING,
		(FileIo::FileAttrib)(FileIo::ATTRIB_NONE)))
	{
		std::string strErr;
		int nErrorCode;
		_pFileIo->getLastError(strErr, nErrorCode);

		SetLastError(strErr, nErrorCode);			
		MOLOG(Log::L_ERROR, CLOGFMT(CIFSSrc, "[%s] Failed to create  file: %s with error: %s"),
			_strLogHint.c_str(), _strFilename.c_str(), strErr.c_str());

		return false;
	}
	MOLOG(Log::L_INFO, CLOGFMT(CIFSSrc, "[%s] open file [%s] successful"),_strLogHint.c_str(),_strFilename.c_str());

	// verify the free space of the content
	_fileSize = _pFileIoFac->getFileSize(filename.c_str());

	if (_fileSize)
	{
		MOLOG(Log::L_INFO, CLOGFMT(CIFSSrc, "[%s] file %s, filesize is ["FMT64"]"), _strLogHint.c_str(),_strFilename.c_str(),_fileSize);
		GetGraph()->setTotalBytes(_fileSize);
	}
	else
	{
		MOLOG(Log::L_WARNING, CLOGFMT(CIFSSrc, "[%s] file %s, failed to getfilesize"), _strLogHint.c_str(),_strFilename.c_str());
	}

	_bitrateCtrl.setBitrate(_nBandwidthBps);
	_bitrateCtrl.start();

	return true;
}

void CIFSIOSource::Stop()
{
	_bStop = true;
#ifdef ZQ_OS_LINUX
        umount(_szMountPoint.c_str());
	rmdir(_szMountPoint.c_str());
#endif
	if (_pFileIo.get())
	{
		_pFileIo->closefile();
	}
	
}

void CIFSIOSource::Close()
{
	_bStop = true;
	if (_pFileIo.get())
	{
		_pFileIo->closefile();
	}

}

void CIFSIOSource::endOfStream()
{
	
}

const char* CIFSIOSource::GetName()
{
	return SOURCE_TYPE_CIFS;
}

int64 CIFSIOSource::getProcessBytes()
{
	return _llProcBytes;
}

MediaSample* CIFSIOSource::GetData(int nOutputIndex)
{
	if (nOutputIndex != 0)
	{
		MOLOG(Log::L_INFO, CLOGFMT(CIFSSrc, "[%s] OutputIndex != 0"), _strLogHint.c_str());
		return NULL;
	}
	
	if (_bStop)
	{
		MOLOG(Log::L_INFO, CLOGFMT(CIFSSrc, "[%s] require to stop while GetData()"), _strLogHint.c_str());

		return NULL;
	}

	MediaSample* pSample = GetGraph()->allocMediaSample();
	if (!pSample)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CIFSSrc, "[%s] failed to alloc meida sample"), _strLogHint.c_str());

		return NULL;
	}

	unsigned int dwBytesRead = 0;
	bool bReadRet;
	bReadRet = _pFileIo->readfile((char*)pSample->getPointer(), pSample->getBufSize(), dwBytesRead); // read fail
	if (bReadRet && dwBytesRead)
	{
		// bandwidth control
		uint32 nRet = _bitrateCtrl.control(_llProcBytes);
		if (nRet > 1000)
		{
			MOLOG(Log::L_DEBUG, CLOGFMT(CIFSSrc, "[%s] bitrate control waiting time %dms"), _strLogHint.c_str(), nRet);
		}

		pSample->setDataLength(dwBytesRead);
		pSample->setOffset(_llProcBytes);
		_llProcBytes += dwBytesRead;
	}
	else
	{
		//////////////////////////////////////////////////////////////////////////
		// verify if the content length with proceeded
		//
		if (_fileSize && _llProcBytes < _fileSize)
		{
			std::string msg;
			int errcode;
			if (!bReadRet)
			{
				_pFileIo->getLastError(msg,errcode);
				MOLOG(Log::L_ERROR, CLOGFMT(CIFSSrc, "[%s] Failed to read source file with error[%s]"), _strLogHint.c_str(), msg.c_str());
			}
			else
			{
				msg = "downloaded size is smaller than source content";
				MOLOG(Log::L_ERROR, CLOGFMT(CIFSSrc, "[%s] downloaded size[%lld] is smaller than source content[%lld]"), _strLogHint.c_str(), _llProcBytes, _fileSize);
			}

			SetLastError(msg, ERRCODE_READ_SOURCE);
		}

		GetGraph()->freeMediaSample(pSample);
		pSample = NULL;
	}

	return pSample;
}

void CIFSIOSource::setMaxBandwidth(unsigned int nBandwidthBps)
{
	_nBandwidthBps = nBandwidthBps;
	
	MOLOG(Log::L_INFO, CLOGFMT(CIFSSrc, "[%s] set MaxBandwidth=%d bps"), _strLogHint.c_str(), nBandwidthBps);
}
bool CIFSIOSource::readbuf(char* pBuf, unsigned int bufLen, unsigned int& rcvLen)
{
	if (!_pFileIo.get())
		return false;
     return _pFileIo->readfile(pBuf, bufLen, rcvLen);
}
bool CIFSIOSource::seek(int64 offset, int pos)
{
	if (!_pFileIo.get())
		return false;
	return _pFileIo->seekfile(offset, (ZQTianShan::ContentProvision::FileIo::Position)pos);
}

void CIFSIOSource::setSourceUrlUTF8( bool bIsUTF8 )
{
	_bSourceUrlUTF8 = bIsUTF8;
}

}}

