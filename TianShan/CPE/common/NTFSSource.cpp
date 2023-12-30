

#include "NTFSSource.h"
#include "Log.h"
#include "ErrorCode.h"
#include "EncodeConvert.h"
#pragma comment(lib, "Lz32.lib")
#define NTFSSrc			"NTFSSrc"

using namespace ZQ::common;

#define MOLOG (*_pLog)

namespace ZQTianShan {
	namespace ContentProvision {


NTFSIOSource::NTFSIOSource()
{
	_nOutputCount = 1;
	_nInputCount = 0;

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
	_fileSize = 0;
	
	_hFile = INVALID_HANDLE_VALUE;

	_llReadOffset = 0;
}

bool NTFSIOSource::Init()
{
	if (_bUtf8Flag)
	{
		std::wstring unifilename;
		if (!EncodeConvert::utf8_to_unicode(_strFilename,unifilename))
			MOLOG(Log::L_INFO, CLOGFMT(NTFSSrc, "[%s] Failed to convert Utf-8 Url %s to unicode"), _strLogHint.c_str(),_strFilename.c_str());

		_hFile = CreateFileW(unifilename.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);	
		if (_hFile == INVALID_HANDLE_VALUE)
		{
			std::string errmsg;
			getSystemErrorText(errmsg);
			MOLOG(Log::L_ERROR, CLOGFMT(NTFSSrc, "[%s] failed to open file %S with error: %s"), _strLogHint.c_str(),unifilename.c_str(),errmsg.c_str());
			return false;
		}

	}
	else
	{
		_hFile = CreateFile(_strFilename.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);	
		if (_hFile == INVALID_HANDLE_VALUE)
		{
			std::string errmsg;
			getSystemErrorText(errmsg);
			MOLOG(Log::L_ERROR, CLOGFMT(NTFSSrc, "[%s] failed to open file %s with error: %s"), _strLogHint.c_str(),_strFilename.c_str(),errmsg.c_str());
			return false;
		}
	}

	LARGE_INTEGER filesize;
	GetFileSizeEx(_hFile,&filesize);
	if (filesize.QuadPart)
	{
		MOLOG(Log::L_INFO, CLOGFMT(NTFSSrc, "[%s] file %s, filesize is [%lld]"), _strLogHint.c_str(),_strFilename.c_str(),filesize.QuadPart);
		GetGraph()->setTotalBytes(filesize.QuadPart);
		_fileSize = filesize.QuadPart;
	}
	else
	{
		std::string errmsg;
		getSystemErrorText(errmsg);
		MOLOG(Log::L_WARNING, CLOGFMT(NTFSSrc, "[%s] file %s, failed to getfilesize with error: %s"), _strLogHint.c_str(),_strFilename.c_str(),errmsg.c_str());
	}
	
	_bitrateCtrl.setBitrate(_nBandwidthBps);
	_bitrateCtrl.start();
	return true;
}

void NTFSIOSource::Stop()
{
	_bStop = true;
	if (_hFile!=INVALID_HANDLE_VALUE)
	{
		CloseHandle(_hFile);
		_hFile = INVALID_HANDLE_VALUE;
	}
}

void NTFSIOSource::Close()
{
	if (_hFile!=INVALID_HANDLE_VALUE)
	{
		CloseHandle(_hFile);
		_hFile = INVALID_HANDLE_VALUE;
	}
}

void NTFSIOSource::endOfStream()
{
	
}

const char* NTFSIOSource::GetName()
{
	return SOURCE_TYPE_NTFSSRC;
}

LONGLONG NTFSIOSource::getProcessBytes()
{
	
	return _llProcBytes;
}

MediaSample* NTFSIOSource::GetData(int nOutputIndex)
{
	if (nOutputIndex != 0)
		return NULL;

	if (_bStop)
		return NULL;
		
	MediaSample* pSample = GetGraph()->allocMediaSample();
	if (!pSample)
	{
		return NULL;
	}

	DWORD nRead;
	BOOL bReadRet;
	bReadRet = ReadFile(_hFile, pSample->getPointer(), pSample->getBufSize(), &nRead, NULL);
	if (bReadRet && nRead)
	{
		// bandwidth control
		uint32 nRet = _bitrateCtrl.control(_llProcBytes);
		if (nRet > 1000)
		{
			MOLOG(Log::L_DEBUG, CLOGFMT(NTFSSrc, "[%s] bitrate control waiting time %dms"), _strLogHint.c_str(), nRet);
		}

		pSample->setDataLength(nRead);
		pSample->setOffset(_llProcBytes);
		_llProcBytes += nRead;
	}
	else
	{
		//////////////////////////////////////////////////////////////////////////
		// verify if the content length with proceeded
		//
		if (_fileSize && _llProcBytes < _fileSize)
		{
			std::string msg;
			if (!bReadRet)
			{
				getSystemErrorText(msg);
				MOLOG(Log::L_ERROR, CLOGFMT(NTFSSrc, "[%s] Failed to read source file with error[%s]"), _strLogHint.c_str(), msg.c_str());
			}
			else
			{
				msg = "downloaded size is smaller than source content";
				MOLOG(Log::L_ERROR, CLOGFMT(NTFSSrc, "[%s] downloaded size[%lld] is smaller than source content[%lld]"), _strLogHint.c_str(), _llProcBytes, _fileSize);
			}

			SetLastError(msg, ERRCODE_READ_SOURCE);
		}

		GetGraph()->freeMediaSample(pSample);
		pSample = NULL;
	}
	
	return pSample;
}

void NTFSIOSource::setMaxBandwidth(unsigned int nBandwidthBps)
{
	_nBandwidthBps = nBandwidthBps;
	
	MOLOG(Log::L_INFO, CLOGFMT(NTFSSrc, "[%s] set MaxBandwidth=%d bps"), _strLogHint.c_str(), nBandwidthBps);
}
bool NTFSIOSource::readbuf(char* pBuf, unsigned int bufLen, unsigned int& rcvLen)
{
	BOOL bReadRet;
	DWORD dwRead;
	bReadRet = ReadFile(_hFile, pBuf, bufLen, &dwRead, NULL);
	if (!bReadRet)
	{
		return false;
	}
	rcvLen = dwRead;
	return true;
}
bool NTFSIOSource::seek(int64 offset, int pos)
{
	long lSeek = LZSeek((INT)_hFile, offset, pos);

	if(lSeek < 0)
		return false;

	return true;
}
}}