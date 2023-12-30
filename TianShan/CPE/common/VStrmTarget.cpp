
#include "Log.h"
#include "vstrmuser.h"
#include "vsiolib.h"
#include "VstrmTarget.h"


#define VstrmIO			"VstrmIO"

#pragma comment(lib, "VstrmDLLEx.lib")
//#pragma comment(lib, "PacedIndex.lib")
#pragma comment(lib, "vsiolib.lib")

using namespace ZQ::common;

#define MOLOG (*_pLog)


namespace ZQTianShan {
	namespace ContentProvision {


HANDLE VstrmTarget::_hVstrm = INVALID_HANDLE_VALUE;



VstrmTarget::VstrmTarget()
{
	_nOutputCount = 0;
	_nInputCount = 1;

	for(int i=0;i<_nInputCount;i++)
	{
		InputPin pin;
		pin.nPrevPin = 0;
		pin.pPrevFilter = 0;		
		_inputPin.push_back(pin);
	}

	_bDriverModule = false;

	_bIndexFile = false;
	_objectId = 0;
	_hOutputFile = 0;
	_cacheCurLength = 0;
	_ntfsCacheHandle = NULL;
	_llProcBytes = 0;
	_bDeleteOnFail = true;
	_bwTicket = 0;
	_reservedBW = 0;

}

bool VstrmTarget::Init()
{
	// initialize the vstrm handle, this make only one handle in the whole filter life
	std::string _vstrmErrMsg;
	if(!initMyVstrm(_vstrmErrMsg))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(VstrmIO, "initVstrm failed with reason: %s"), _vstrmErrMsg.c_str());
		SetLastError(std::string("initVstrm failed with reason: ") + _vstrmErrMsg);

		return false;
	}

	if(!_bIndexFile)
	{
#ifdef FILE_FLAG_CACHED
		VSTATUS vstat = VsOpenEx( &_hOutputFile,
			(char*)_strFilename.c_str(), 
			GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			CREATE_ALWAYS,
			FILE_FLAG_CACHED,
			0,
			&_objectId);
#else
		VSTATUS vstat = VsOpen( &_hOutputFile,
			(char*)_strFilename.c_str(), 
			GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			CREATE_ALWAYS,
			0,
			&_objectId);
#endif
		
		if(!IS_VSTRM_SUCCESS(vstat))
		{
			std::string errstr;
			getSystemErrorText(errstr);
			
			char msg[256];
			sprintf(msg, "VsOpenEx() failed with error: %s", errstr.c_str());
			SetLastError(msg);
			
			MOLOG(Log::L_ERROR, CLOGFMT(VstrmIO, "[%s] Failed to create output file: %s with error: %s"),
				_strLogHint.c_str(), _strFilename.c_str(), errstr.c_str());

			return false;
		}

		// disable BufDrv throttling to have better Vstrm IO performance
		if(_disableBufDrvThrottle)
		{
			std::string errmsg;
			if(disableBufDrvThrottle(_hVstrm, _hOutputFile, _objectId, errmsg))
			{
				MOLOG(Log::L_INFO, CLOGFMT(VstrmIO, "[%s] BufDrv throttling enabled"), _strLogHint.c_str());
			}
			else
			{
				MOLOG(Log::L_WARNING, CLOGFMT(VstrmIO, "[%s] failed to enable BufDrv throttling with error: %s"), _strLogHint.c_str(), errmsg.c_str());
			}
		} // end of if(_disableBufDrvThrottle)
	}
	else
	{
		std::string filePath = _cachePath + _strFilename;
		MOLOG(Log::L_INFO, CLOGFMT(VstrmIO, "[%s] Create output file %s on local disk"), _strLogHint.c_str(),
			filePath.c_str());
		
		_ntfsCacheHandle = CreateFileA(filePath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);		
		if (INVALID_HANDLE_VALUE == _ntfsCacheHandle)
		{
			std::string errmsg;
			getSystemErrorText(errmsg);
			SetLastError(errmsg);	
			
			MOLOG(Log::L_ERROR, CLOGFMT(VstrmIO, "[%s] CreateFile failed with error: %s"), _strLogHint.c_str(), errmsg.c_str());
			return false;
		}
	}
	
	_llProcBytes = 0;
	
	return true;
}

void VstrmTarget::Stop()
{
	
}

void VstrmTarget::Close()
{
	if(_bIndexFile)
	{
		if(_ntfsCacheHandle != NULL)
		{
			CloseHandle(_ntfsCacheHandle);
			_ntfsCacheHandle = NULL;
		}

		std::string errmsg;
		std::string filePath = _cachePath + _strFilename;
		if(!copyFileToVstrm(filePath, _strFilename, errmsg, _disableBufDrvThrottle, _hVstrm))
		{
			MOLOG(Log::L_ERROR, CLOGFMT(VstrmIO, "[%s] failed to copy file from NTFS to vstrm with error: %s"), 
				_strLogHint.c_str(), _strFilename.c_str(), errmsg.c_str());
		}
	
		MOLOG(Log::L_INFO, CLOGFMT(VstrmIO, "[%s] delete local temp file %s"), 
			_strLogHint.c_str(), filePath.c_str());
		DeleteFileA(filePath.c_str());			
	}
	else
	{
		if(_hOutputFile == INVALID_HANDLE_VALUE)
			return;
		
		VsClose(_hOutputFile, _objectId);
		_hOutputFile = INVALID_HANDLE_VALUE;

	}
	
	// release the reserved VStrm bandwidth 
	releaseVstrmBW();
	
	// delete error files
	if(_bDeleteOnFail & IsFailed())
	{
		delOutput();
	}
}

void VstrmTarget::endOfStream()
{
	GetGraph()->Finish();
}

const char* VstrmTarget::GetName()
{
	return TARGET_TYPE_VSTRM;
}

LONGLONG VstrmTarget::getProcessBytes()
{
	return _llProcBytes;
}


bool VstrmTarget::Receive(MediaSample* pSample, int nInputIndex)
{
	if (nInputIndex)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(VstrmIO, "[%s] Receive() invalid input index: %d"), _strLogHint.c_str(), nInputIndex);
		SetLastError("Receive(): invalid input index");
		return false; 
	}

	void* pointer = pSample->getPointer();
	unsigned int uOffsetLow, uOffsetHigh;
	uOffsetLow = pSample->getOffset(&uOffsetHigh);
	unsigned int dwActualLen = pSample->getDataLength();

	//
	//do not support seek, vvx write will be implement in another one
	//
	if (!_bIndexFile)
	{
		if (!writeData((char*)pointer, dwActualLen))
			return false;			
	}
	else
	{
		if (!writeIndexData((char*)pointer, dwActualLen, uOffsetLow, uOffsetHigh))
			return false;
	}

	GetGraph()->freeMediaSample(pSample);

	return true;
}

void VstrmTarget::getVstrmError(HANDLE hVstrm, std::string& strErr)
{
	if(hVstrm == INVALID_HANDLE_VALUE)
		return;
	
	char sErrorText[256]={0};
	
	DWORD lastError = VstrmGetLastError();
	
	VstrmClassGetErrorText(hVstrm, lastError, sErrorText, 255);
	
	char errcode[24];
	sprintf(errcode, "[0x%08x]", lastError);
	
    strErr = std::string(sErrorText)+ errcode;
}

bool VstrmTarget::disableBufDrvThrottle(HANDLE vstrmHandle, HANDLE fileHandle, OBJECT_ID objectId, std::string& errMsg)
{
	VSTATUS vstat;
	ULONG disableThrottle = 1;
	ATTRIBUTE_ARRAY attributes;
	
	attributes.setAllorFailAll = 1;
	attributes.attributeCount = 1;
	attributes.objectId = objectId;
	
	attributes.attributeArray[0].setExactly = 1;
	attributes.attributeArray[0].attributeCode = VSTRM_ATTR_GEN_OVERRIDE_IO_THROTTLE;
	attributes.attributeArray[0].attributeQualifier = 0;
	attributes.attributeArray[0].attributeValueP = &disableThrottle;
	attributes.attributeArray[0].attributeValueLength = sizeof(disableThrottle);
	
	vstat = VstrmClassSetSessionAttributesEx(fileHandle, objectId, &attributes);
	
	if (!IS_VSTRM_SUCCESS(vstat))
	{
		// check free space before returning an error - if there's 0 bytes available
		// then the set attributes request will return an INVALID_SESSION error, not a
		// disk space error even though it looks like SeaFile did the right thing
		
		// if Vstrm Class handle does not pass in, create it.
		HANDLE vstrmClass = INVALID_HANDLE_VALUE;
		if(INVALID_HANDLE_VALUE == vstrmHandle)
		{
			vstrmClass = initVstrm(errMsg);
			if(INVALID_HANDLE_VALUE == vstrmClass)
			{
				return false;
			}
		}
		else
		{
			vstrmClass = vstrmHandle;
		}
		
		LARGE_INTEGER	free, total;
		
		VSTATUS vStatus = VstrmClassGetStorageData(vstrmClass, &free, &total);
		if(IS_VSTRM_SUCCESS(vStatus))
		{
			if(0 == free.QuadPart)
			{
				errMsg = "VstrmClassSetSessionAttributesEx() set session attributes failed, no free space on disk";
			}
			else
			{
				char szBuf[255] = {0};
				VstrmClassGetErrorText(vstrmClass, vstat, szBuf, sizeof(szBuf));
				
				errMsg = "VstrmClassSetSessionAttributesEx() set session attributes failed with error " + std::string(szBuf);
			}
		}
		
		// release the create VstrmClass handle if it is created here
		if(INVALID_HANDLE_VALUE == vstrmHandle && INVALID_HANDLE_VALUE != vstrmClass)
		{
			uninitVstrm(vstrmClass);
		}
		
		return false;
	}
	
	return true;
}

void VstrmTarget::uninitVstrm(HANDLE hvstrm)
{
	if(hvstrm != INVALID_HANDLE_VALUE)
	{
		// It is not recommended to invoke VstrmClassCloseEx function, but seems for streaming
		// but in case of !bRunningOnMC, must invoked
		VstrmClassCloseEx(hvstrm);
	}
}

HANDLE VstrmTarget::initVstrm(std::string& errmsg)
{	
	VSTATUS vStatus;
	char szBuf[255] = {0};
	
	HANDLE hvstrm = INVALID_HANDLE_VALUE;
	vStatus = VstrmClassOpenEx(&hvstrm);
	if (vStatus != VSTRM_SUCCESS) 
	{
		VstrmClassGetErrorText(hvstrm, vStatus, szBuf, sizeof(szBuf));
		
		errmsg = szBuf;
		
		return INVALID_HANDLE_VALUE;
	} 
	
	return hvstrm;
}

bool VstrmTarget::initMyVstrm(std::string& errmsg)
{
	if(_hVstrm != INVALID_HANDLE_VALUE)
	{
		return true;
	}

	_hVstrm = initVstrm(errmsg);
	if(INVALID_HANDLE_VALUE == _hVstrm)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(VstrmIO, "initVstrm() failed with reason: %s"), errmsg.c_str());
	}
	return _hVstrm != INVALID_HANDLE_VALUE;
}

/// set cache path
void VstrmTarget::setCacheDirectory(std::string path) 
{ 
	_cachePath = path; 
	if(_cachePath == "")
		return;
	
	int pos = _cachePath.length() - 1;
	if(_cachePath[pos] != '\\' && _cachePath[pos] != '/' )
	{
		_cachePath = _cachePath + "\\";
	}
}

bool VstrmTarget::writeData(char* pBuf, unsigned int nLen)
{
	unsigned long amountWritten;
	if(0 == _cacheCurLength && VSTRM_IO_SIZE == nLen)
	{
		if(!VsWrite(_hOutputFile, nLen, (char*)pBuf, &amountWritten, NULL))
		{
			std::string errstr;
			getSystemErrorText(errstr);
			
			SetLastError(std::string("Vstrm failed to write file ") + _strFilename + " with error:" + errstr);
			MOLOG(Log::L_ERROR, CLOGFMT(VstrmIO, "[%s] VsWrite() failed with error: %s)"), _strLogHint.c_str(), errstr.c_str());
			
			return false;
		}
	}
	else if(_cacheCurLength + nLen < VSTRM_IO_SIZE)
	{
		memcpy(&_cacheBuffer[_cacheCurLength], pBuf, nLen);
		_cacheCurLength += nLen;		
	}
	else // if(_cacheCurLength + nLen >= _cacheBufferSize)
	{
		// the _temp64Buff will reach 64K
		DWORD dwLeftBytes = nLen;
		DWORD dwCopyBytes = VSTRM_IO_SIZE - _cacheCurLength;			
		
		int totalCopiedBytes = 0;
		do 
		{
			memcpy(&_cacheBuffer[_cacheCurLength], (pBuf+totalCopiedBytes), dwCopyBytes);
			
			totalCopiedBytes += dwCopyBytes;
			
			if(!VsWrite(_hOutputFile, VSTRM_IO_SIZE, _cacheBuffer, &amountWritten, NULL))
			{
				std::string errstr;
				getSystemErrorText(errstr);
				
				SetLastError(std::string("Vstrm failed to write file ") + _strFilename + " with error:" + errstr);
				MOLOG(Log::L_ERROR, CLOGFMT(VstrmIO, "[%s] VsWrite() failed with error: %s)"), _strLogHint.c_str(), errstr.c_str());
				
				return false;
			}
			
			dwLeftBytes -= dwCopyBytes;
			dwCopyBytes = dwLeftBytes < VSTRM_IO_SIZE ? dwLeftBytes : VSTRM_IO_SIZE;			
		}while(dwLeftBytes >= VSTRM_IO_SIZE);
		
		// copy the left bytes in pBufferData to cache buffer for later flushing
		if (dwLeftBytes)
		{
			memcpy(&_cacheBuffer[0], (pBuf+totalCopiedBytes), dwLeftBytes);
			_cacheCurLength = dwLeftBytes;
		}		
	}

	_llProcBytes += nLen;
	return true;
}

bool VstrmTarget::writeIndexData(char* pBuf, unsigned int nLen, unsigned int uOffetLow, unsigned int uOffetHigh)
{
	DWORD amountWritten = 0;
	SetFilePointer(_ntfsCacheHandle, uOffetLow, (long*)&uOffetHigh, FILE_BEGIN);
	if(!WriteFile(_ntfsCacheHandle, pBuf, nLen, &amountWritten, NULL))
	{
		std::string errstr;
		getSystemErrorText(errstr);		
		SetLastError(std::string("failed to write file ") + _strFilename + " on local with error:" + errstr);
		MOLOG(Log::L_ERROR, CLOGFMT(VstrmIO, "[%s] Failed to create write file: %s with error: %s"),
			_strLogHint.c_str(), _strFilename.c_str(), errstr.c_str());

		return false;
	}

	return true;
}

bool VstrmTarget::releaseVstrmBW()
{	
	if(0 == _bwmgrClientId || 0 == _reservedBW || 0 == _bwTicket)
		return true;
	
	VSTATUS	statusTicket = ERROR_SUCCESS;
	
	MOLOG(Log::L_INFO, CLOGFMT(VstrmIO, "[%s] VstrmClassReleaseBandwidth(BW - %dbps) for ticket: 0x%I64X"), 
		_strLogHint.c_str(), _reservedBW, _bwTicket);
	statusTicket = VstrmClassReleaseBandwidth(_hVstrm, _bwTicket);
	
	//  it always return failure: VSTRM_NOT_SUPPORTED even the bandwidth was released indeed
	if (statusTicket != VSTRM_SUCCESS)
	{
		char szBuf[255] = {0};
		VstrmClassGetErrorText(_hVstrm, statusTicket, szBuf, sizeof(szBuf));
		
		MOLOG(Log::L_WARNING, CLOGFMT(VstrmIO, "[%s] VstrmClassReleaseBandwidth(%d pbs) for ticket 0x%I64X failed with error %s"), 
			_strLogHint.c_str(), _reservedBW, _bwTicket, szBuf);
		
		_bwTicket = 0;
		
		return false;
	}
	
	_bwTicket = 0;
	MOLOG(Log::L_INFO, CLOGFMT(VstrmIO, "[%s] VstrmClassReleaseBandwidth(BW - %dbps) succeed"),  _strLogHint.c_str(), _reservedBW);
	
	return true;
}

bool VstrmTarget::delOutput()
{
	if(_strFilename.empty())
		return true;
	
	// delete the file
	if(VstrmDeleteFile(_hVstrm, _strFilename.c_str()))
	{
		MOLOG(Log::L_INFO, CLOGFMT(VstrmIO, "[%s] file %s was successfully deleted"), _strLogHint.c_str(), _strFilename.c_str());
		return true;
	}

	std::string strErr;
	getVstrmError(_hVstrm, strErr);
	MOLOG(Log::L_WARNING, CLOGFMT(VstrmIO, "[%s] failed to delete file %s with error: %s"), _strLogHint.c_str(), _strFilename.c_str(),
		strErr.c_str());
	return false;
}

bool VstrmTarget::copyFileToVstrm(std::string sourceFile, std::string desFile, std::string& errmsg, bool bDisableBufDrvThrottle, HANDLE hVstrm)
{	
	OBJECT_ID objectId;
	HANDLE vstrmFileHandle;
	// create Vstrm file handle
	VSTATUS vstat = VsOpen(&vstrmFileHandle, 
		(char*)desFile.c_str(), 
		GENERIC_WRITE, 
		FILE_SHARE_READ|FILE_SHARE_WRITE, 
		CREATE_ALWAYS, 
		0, 
		&objectId);
	
	if(!IS_VSTRM_SUCCESS(vstat))
	{
		errmsg = "Failed to create vstrm file handle for " + desFile;
		
		return false;
	}
	
	// disable BufDrv throttling to have better Vstrm IO performance
	if(bDisableBufDrvThrottle)
	{
		std::string errstr;
		disableBufDrvThrottle(hVstrm, vstrmFileHandle, objectId, errstr);
	}
	
	DWORD dwBytesRead = 0;
	DWORD amountWritten = 0;	
	char buffer[VSTRM_IO_SIZE];
	
	HANDLE ntfsFileHandle = ::CreateFileA(sourceFile.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	
	bool succeed = true;
	if(ntfsFileHandle != INVALID_HANDLE_VALUE)
	{
		while(ReadFile(ntfsFileHandle, (LPVOID)buffer, VSTRM_IO_SIZE, &dwBytesRead, NULL) && dwBytesRead > 0)
		{
			if(!VsWrite(vstrmFileHandle, dwBytesRead, buffer, &amountWritten, NULL))
			{
				std::string errstr;
				getSystemErrorText(errstr);
				
				errmsg = "Vstrm failed to write file %s " + desFile + " with error:" + errstr;
				succeed = false;
				break;
			}
		}
		CloseHandle(ntfsFileHandle);
		ntfsFileHandle = NULL;
	}
	else
	{
		errmsg = "Failed to open file %s " + sourceFile;
	}
	
	VsClose(vstrmFileHandle, objectId);
	
	return succeed;
}

}}