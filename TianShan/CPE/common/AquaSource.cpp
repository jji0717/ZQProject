#include "AquaSource.h"
#include "Log.h"
#include "urlstr.h"
#include "ErrorCode.h"
#include "AquaFileIoFactory.h"
#include "EncodeConvert.h"

#ifdef ZQ_OS_LINUX
extern "C"
{
#include <sys/mount.h>
}
#endif

using namespace ZQ::common;

#define MOLOG			(*_pLog)

namespace ZQTianShan {
	namespace ContentProvision {


AquaIOSource::AquaIOSource()
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
}

AquaIOSource::~AquaIOSource()
{
}

bool AquaIOSource::Init()
{
	std::string filename;
	MOLOG(Log::L_INFO, CLOGFMT(AquaIOSource, "[%s] Enter Init()"), _strLogHint.c_str());

	if (!_pFileIoFac)
	{
		MOLOG(Log::L_INFO, CLOGFMT(AquaIOSource, "[%s] IO factory handle is NULL"), _strLogHint.c_str());
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
		MOLOG(Log::L_ERROR, CLOGFMT(AquaIOSource, "[%s] Failed to create  file: %s with error: %s"),
			_strLogHint.c_str(), _strFilename.c_str(), strErr.c_str());

		return false;
	}
	MOLOG(Log::L_INFO, CLOGFMT(AquaIOSource, "[%s] open file [%s] successful"),_strLogHint.c_str(),_strFilename.c_str());

	// verify the free space of the content
	_fileSize = _pFileIoFac->getFileSize(filename.c_str());

	if (_fileSize)
	{
		MOLOG(Log::L_INFO, CLOGFMT(AquaIOSource, "[%s] file %s, filesize is ["FMT64"]"), _strLogHint.c_str(),_strFilename.c_str(),_fileSize);
		GetGraph()->setTotalBytes(_fileSize);
	}
	else
	{
		MOLOG(Log::L_WARNING, CLOGFMT(AquaIOSource, "[%s] file %s, failed to getfilesize"), _strLogHint.c_str(),_strFilename.c_str());
	}

	_bitrateCtrl.setBitrate(_nBandwidthBps);
	_bitrateCtrl.start();

	return true;
}

void AquaIOSource::Stop()
{
	_bStop = true;

	if (_pFileIo.get())
	{
		_pFileIo->closefile();
	}
	
}

void AquaIOSource::Close()
{
	_bStop = true;
	if (_pFileIo.get())
	{
		_pFileIo->closefile();
	}

}

void AquaIOSource::endOfStream()
{
	
}

const char* AquaIOSource::GetName()
{
	return SOURCE_TYPE_AQUA;
}

int64 AquaIOSource::getProcessBytes()
{
	return _llProcBytes;
}

MediaSample* AquaIOSource::GetData(int nOutputIndex)
{
	if (nOutputIndex != 0)
	{
		MOLOG(Log::L_INFO, CLOGFMT(AquaIOSource, "[%s] OutputIndex != 0"), _strLogHint.c_str());
		return NULL;
	}
	
	if (_bStop)
	{
		MOLOG(Log::L_INFO, CLOGFMT(AquaIOSource, "[%s] require to stop while GetData()"), _strLogHint.c_str());

		return NULL;
	}

	MediaSample* pSample = GetGraph()->allocMediaSample();
	if (!pSample)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(AquaIOSource, "[%s] failed to alloc meida sample"), _strLogHint.c_str());

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
			MOLOG(Log::L_DEBUG, CLOGFMT(AquaIOSource, "[%s] bitrate control waiting time %dms"), _strLogHint.c_str(), nRet);
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
				MOLOG(Log::L_ERROR, CLOGFMT(AquaIOSource, "[%s] Failed to read source file with error[%s]"), _strLogHint.c_str(), msg.c_str());
			}
			else
			{
				msg = "downloaded size is smaller than source content";
				MOLOG(Log::L_ERROR, CLOGFMT(AquaIOSource, "[%s] downloaded size[%lld] is smaller than source content[%lld]"), _strLogHint.c_str(), _llProcBytes, _fileSize);
			}

			SetLastError(msg, ERRCODE_READ_SOURCE);
		}

		GetGraph()->freeMediaSample(pSample);
		pSample = NULL;
	}

	return pSample;
}

void AquaIOSource::setMaxBandwidth(unsigned int nBandwidthBps)
{
	_nBandwidthBps = nBandwidthBps;
	
	MOLOG(Log::L_INFO, CLOGFMT(AquaIOSource, "[%s] set MaxBandwidth=%d bps"), _strLogHint.c_str(), nBandwidthBps);
}
bool AquaIOSource::readbuf(char* pBuf, unsigned int bufLen, unsigned int& rcvLen)
{
	if (!_pFileIo.get())
		return false;
     return _pFileIo->readfile(pBuf, bufLen, rcvLen);
}
bool AquaIOSource::seek(int64 offset, int pos)
{
	if (!_pFileIo.get())
		return false;
	return _pFileIo->seekfile(offset, (ZQTianShan::ContentProvision::FileIo::Position)pos);
}

}}

