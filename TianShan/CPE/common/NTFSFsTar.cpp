
#include "Log.h"
#include "vstrmuser.h"
#include "vsiolib.h"
#include "NTFSFsTar.h"
#include "PacedIndex.h"
#include "vvx.h"
#include "errorcode.h"



#define NTFSFileset			"NTFSFileset"

#pragma comment(lib, "PacedIndex.lib")

using namespace ZQ::common;

#define MOLOG (*_pLog)
#define SMOLOG (*(pThis->_pLog))


namespace ZQTianShan {
	namespace ContentProvision {


HANDLE NtfsFsTarget::_hVstrm = INVALID_HANDLE_VALUE;

NtfsFsTarget::NtfsFsTarget():_bTypeH264(false)
{
	_nOutputCount = 0;
	_nInputCount = 4;

	int i;
	for(i=0;i<_nInputCount;i++)
	{
		InputPin pin;
		pin.nPrevPin = 0;
		pin.pPrevFilter = 0;		
		_inputPin.push_back(pin);
	}

	_bDriverModule = false;

	for(i=0;i<4;i++)
	{
		_subFiles[i].bIndexFile = false;
		_subFiles[i].objectId = 0;
		_subFiles[i].hOutputFile = 0;
		_subFiles[i].cacheCurLength = 0;
		_subFiles[i].llProcBytes = 0;		
		_subFiles[i].bwTicket = 0;
		_subFiles[i].reservedBW = 0;
	}
	_ntfsCacheHandle = NULL;
	_bDeleteOnFail = true;
	_bPacing = false;
	_posCacheRead.QuadPart = 0;
	_vvxByteRead = 0;
}

void NtfsFsTarget::InitPins()
{
	if (_bTypeH264)
	{
		_nInputCount = 3;
	}
	else
	{
		_nInputCount = 4;
	}

	_inputPin.clear();
	int i;
	for(i=0;i<_nInputCount;i++)
	{
		InputPin pin;
		pin.nPrevPin = 0;
		pin.pPrevFilter = 0;		
		_inputPin.push_back(pin);
	}

	for(i=0;i<_nInputCount;i++)
	{
		SubFile& subFile = _subFiles[i];

		_subFiles[i].bIndexFile = false;
		_subFiles[i].objectId = 0;
		_subFiles[i].hOutputFile = 0;
		_subFiles[i].cacheCurLength = 0;
		_subFiles[i].llProcBytes = 0;		
		_subFiles[i].bwTicket = 0;
		_subFiles[i].reservedBW = 0;
	
	}
}

bool NtfsFsTarget::Init()
{
	_subFiles[0].reservedBW = _dwBandwidth;
	_subFiles[0].strFilename = _strFilename;

	if (!_bTypeH264)
	{		
		_subFiles[1].strFilename = _strFilename + ".ff";
		_subFiles[2].strFilename = _strFilename + ".fr";
		_subFiles[3].strFilename = _strFilename + ".vvx";
		_subFiles[1].reservedBW = _dwBandwidth/6;
		_subFiles[2].reservedBW = _dwBandwidth/6;
		_subFiles[3].reservedBW = _dwBandwidth/20;
	}
	else
	{
		_subFiles[1].strFilename = _strFilename + ".ffr";
		_subFiles[2].strFilename = _strFilename + ".vv2";
		_subFiles[1].reservedBW = _dwBandwidth/6;
		_subFiles[2].reservedBW = _dwBandwidth/20;
	}

	for(int i=0;i<_nInputCount;i++)
	{
		_subFiles[i].hOutputFile = CreateFile(
			(char*)_subFiles[i].strFilename.c_str(), 
			GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			0,
			CREATE_ALWAYS,			
			0,
			0);
		
		if(_subFiles[i].hOutputFile==INVALID_HANDLE_VALUE)
		{
			std::string errstr;
			getSystemErrorText(errstr);
			
			char msg[256];
			sprintf(msg, "CreateFile() failed with error: %s", errstr.c_str());
			SetLastError(msg, ERRCODE_NTFS_CREATEFILE);
			
			MOLOG(Log::L_ERROR, CLOGFMT(NTFSFileset, "[%s] Failed to create output file: %s with error: %s"),
				_strLogHint.c_str(), _subFiles[i].strFilename.c_str(), errstr.c_str());

			return false;
		}

		_subFiles[i].llProcBytes = 0;
		_subFiles[i].pThis = this;
	}

	if(_bPacing)
	{
		DWORD pacingGroup = GetCurrentThreadId();
		int indexFileNo = 3;
		std::string pacingType="vvx";
		
		PacedIndexSetLogCbk(2, pacingAppLogCbk);
		DWORD paceresult = 0;
		int i;
		for(i=0; i<4; i++)
		{
			// skip the vvx, make sure it was added last
			if(i == indexFileNo)
				continue;
			
			paceresult = PacedIndexAdd((void *) &pacingGroup, pacingType.c_str(), _subFiles[i].strFilename.c_str(), 
				pacingAppWrite, pacingAppSeek, pacingAppSetEOF, pacingAppReportOffsets, 
				(void*)&_subFiles[i], &_subFiles[i].pacingIndexCtx);
			if(paceresult)
			{
				MOLOG(Log::L_ERROR, CLOGFMT(NTFSFileset, "[%s] PacedIndexAdd() failed with error %s"), 
					_subFiles[i].strFilename.c_str(), DecodePacedIndexError(paceresult));
				SetLastError(std::string("PacedIndexAdd() failed with error ") + DecodePacedIndexError(paceresult), ERRCODE_PACING_ERROR);
				return false;	
			}
		}
		
		// add vvx last
		paceresult = PacedIndexAdd((void*) &pacingGroup, pacingType.c_str(), _subFiles[indexFileNo].strFilename.c_str(), 
			pacingAppWrite, pacingAppSeek, pacingAppSetEOF, pacingAppReportOffsets, 
			(void*)&_subFiles[indexFileNo], &_subFiles[indexFileNo].pacingIndexCtx);
		if(paceresult)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(NTFSFileset, "[%s] PacedIndexAdd() failed with error %s"), 
				_subFiles[i].strFilename.c_str(), DecodePacedIndexError(paceresult));
			SetLastError(std::string("PacedIndexAdd() failed with error ") + DecodePacedIndexError(paceresult), ERRCODE_PACING_ERROR);
			return false;	
		}
	}

	{
		_subFiles[3].bIndexFile = true;
		std::string filePath = _cachePath + _subFiles[3].strFilename;
		MOLOG(Log::L_INFO, CLOGFMT(NTFSFileset, "[%s] Create output file %s on local disk"), _strLogHint.c_str(),
			filePath.c_str());
		
		_ntfsCacheHandle = CreateFileA(filePath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);		
		if (INVALID_HANDLE_VALUE == _ntfsCacheHandle)
		{
			std::string errmsg;
			getSystemErrorText(errmsg);
			SetLastError(errmsg, ERRCODE_NTFS_CREATEFILE);	
			
			MOLOG(Log::L_ERROR, CLOGFMT(NTFSFileset, "[%s] CreateFile failed with error: %s"), _strLogHint.c_str(), errmsg.c_str());
			return false;
		}
	}

	_posCacheRead.QuadPart = 0;
	
	return true;
}

void NtfsFsTarget::Stop()
{
	
	
}

bool NtfsFsTarget::flushCacheBuffer()
{	
	for(int i=0;i<3;i++)
	{
		if(_subFiles[i].hOutputFile == INVALID_HANDLE_VALUE)
			continue;
		
		if(_subFiles[i].cacheCurLength > 0)
		{	
			if (_bPacing)
			{
				// flush the last buff to disk
				DWORD paceresult = 0;
				if(PacedIndexWrite(_subFiles[i].pacingIndexCtx, _subFiles[i].cacheCurLength, (char*)_subFiles[i].cacheBuffer, &paceresult) < 0)
				{
					MOLOG(Log::L_ERROR, CLOGFMT(NTFSFileset, "[%s] PacedIndexWrite() (%s) failed with error %s"), 
						_strLogHint.c_str(), _subFiles[i].strFilename.c_str(), DecodePacedIndexError(paceresult));
					SetLastError(std::string("PacedIndexWrite() failed to write file ") + _subFiles[i].strFilename + " with error:" + DecodePacedIndexError(paceresult));
					return false;
				}
			}
			else
			{
				DWORD amountWritten;
				if(!WriteFile(_subFiles[i].hOutputFile,_subFiles[i].cacheBuffer,  _subFiles[i].cacheCurLength, &amountWritten, NULL))
				{
					std::string errstr;
					getSystemErrorText(errstr);
					
					SetLastError(std::string("Vstrm failed to write file ") + _subFiles[i].strFilename + " with error:" + errstr);
					MOLOG(Log::L_ERROR, CLOGFMT(NTFSFileset, "[%s] VsWrite() failed with error: %s)"), _strLogHint.c_str(), errstr.c_str());

					return false;
				}
			}

			_subFiles[i].cacheCurLength = 0;
		}		
	}

	return true;
}

void NtfsFsTarget::Close()
{
	flushCacheBuffer();

	if(!_bPacing)
	{
		int i;
		for(i=0;i<3;i++)
		{
			if(_subFiles[i].hOutputFile == INVALID_HANDLE_VALUE)
				continue;
			
			CloseHandle(_subFiles[i].hOutputFile);
			_subFiles[i].hOutputFile = INVALID_HANDLE_VALUE;
		}


		//
		// index file
		//		
		if(_ntfsCacheHandle != NULL)
		{
			CloseHandle(_ntfsCacheHandle);
			_ntfsCacheHandle = NULL;
		}

		std::string errmsg;
		std::string filePath = _cachePath + _subFiles[i].strFilename;
		if(!copyFileToVstrm(filePath, _subFiles[i].strFilename, errmsg, _disableBufDrvThrottle, _hVstrm))
		{
			MOLOG(Log::L_ERROR, CLOGFMT(NTFSFileset, "[%s] failed to copy file from NTFS to vstrm with error: %s"), 
				_strLogHint.c_str(), _subFiles[i].strFilename.c_str(), errmsg.c_str());
		}
		
		MOLOG(Log::L_INFO, CLOGFMT(NTFSFileset, "[%s] delete local temp file %s"), 
			_strLogHint.c_str(), filePath.c_str());
		DeleteFileA(filePath.c_str());		
	}	
	else
	{
		//
		// pacing
		//
		for(int i=0;i<4;i++)
		{
			if(_subFiles[i].hOutputFile == INVALID_HANDLE_VALUE)
				continue;
			
			DWORD paceresult = 0;				
			//
			// Grab MD5 from the file
			//
			char md5[33] = "";
			if (i == 3)	//only index file have this
			{
				paceresult = PacedIndexGetPacedFileMD5(_subFiles[i].pacingIndexCtx, md5);
				if (paceresult)
				{
					MOLOG(Log::L_WARNING, CLOGFMT(NTFSFileset, "[%s] PacedIndexGetPacedFileMD5() (%s) failed with error %s"), _strLogHint.c_str() 
						, _subFiles[i].strFilename.c_str(), DecodePacedIndexError(paceresult));
				}
				else
				{
					MOLOG(Log::L_INFO, CLOGFMT(NTFSFileset, "[%s] PacedIndexGetPacedFileMD5() (%s) return [%s]"), _strLogHint.c_str(), 
						_subFiles[i].strFilename.c_str(), md5);
				}
			}

			MOLOG(Log::L_INFO, CLOGFMT(NTFSFileset, "[%s] to PacedIndexRemove() (%s) file context"),
				_strLogHint.c_str(), _subFiles[i].strFilename.c_str());
			paceresult = PacedIndexRemove(_subFiles[i].pacingIndexCtx);
			if (paceresult)
			{
				MOLOG(Log::L_WARNING, CLOGFMT(NTFSFileset, "[%s] PacedIndexRemove() (%s) failed with error %s"), _strLogHint.c_str() 
					, _subFiles[i].strFilename.c_str(), DecodePacedIndexError(paceresult));
			}
			else
			{
				MOLOG(Log::L_INFO, CLOGFMT(NTFSFileset, "[%s] PacedIndexRemove (%s) file context successful"),
					_strLogHint.c_str(), _subFiles[i].strFilename.c_str());
			}
			_subFiles[i].pacingIndexCtx = NULL;				
			
			CloseHandle(_subFiles[i].hOutputFile);
			_subFiles[i].hOutputFile = INVALID_HANDLE_VALUE;
		}

		if(_ntfsCacheHandle != NULL)
		{
			CloseHandle(_ntfsCacheHandle);
			_ntfsCacheHandle = NULL;
		}

		std::string errmsg;
		std::string filePath = _cachePath + _subFiles[3].strFilename;
		MOLOG(Log::L_INFO, CLOGFMT(NTFSFileset, "[%s] delete local temp file %s"), 
			_strLogHint.c_str(), filePath.c_str());
		DeleteFileA(filePath.c_str());
	}

	// release the reserved VStrm bandwidth 
	releaseVstrmBW();
	
	// delete error files
	if(_bDeleteOnFail & IsFailed())
	{
		delOutput();
	}
}

void NtfsFsTarget::endOfStream()
{
	GetGraph()->Finish();
}

const char* NtfsFsTarget::GetName()
{
	return TARGET_TYPE_NTFSFS;
}

LONGLONG NtfsFsTarget::getProcessBytes()
{
	return _subFiles[0].llProcBytes;
}


bool NtfsFsTarget::Receive(MediaSample* pSample, int nInputIndex)
{
	if (nInputIndex < 0 || nInputIndex >3)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(NTFSFileset, "[%s] Receive() invalid input index: %d"), _strLogHint.c_str(), nInputIndex);
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
	if (_subFiles[nInputIndex].bIndexFile)
	{
		if (!writeIndexData((char*)pointer, dwActualLen, uOffsetLow, uOffsetHigh))
			return false;			

		if (_bPacing)
		{
			if (!pacingWriteIndex())
				return false;
		}		
	}
	else
	{
		if (_bPacing)
		{
			if (!pacingWriteData(nInputIndex, (char*)pointer, dwActualLen))
				return false;
		}
		else
		{
			if (!writeVstrmData(nInputIndex, (char*)pointer, dwActualLen))
				return false;
		}

	}

	GetGraph()->freeMediaSample(pSample);

	return true;
}

void NtfsFsTarget::getVstrmError(HANDLE hVstrm, std::string& strErr)
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

bool NtfsFsTarget::disableBufDrvThrottle(HANDLE vstrmHandle, HANDLE fileHandle, OBJECT_ID objectId, std::string& errMsg)
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

void NtfsFsTarget::uninitVstrm(HANDLE hvstrm)
{
	if(hvstrm != INVALID_HANDLE_VALUE)
	{
		// It is not recommended to invoke VstrmClassCloseEx function, but seems for streaming
		// but in case of !bRunningOnMC, must invoked
		VstrmClassCloseEx(hvstrm);
	}
}

HANDLE NtfsFsTarget::initVstrm(std::string& errmsg)
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

bool NtfsFsTarget::initMyVstrm(std::string& errmsg)
{
	if(_hVstrm != INVALID_HANDLE_VALUE)
	{
		return true;
	}

	_hVstrm = initVstrm(errmsg);
	if(INVALID_HANDLE_VALUE == _hVstrm)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(NTFSFileset, "initVstrm() failed with reason: %s"), errmsg.c_str());
	}
	return _hVstrm != INVALID_HANDLE_VALUE;
}

/// set cache path
void NtfsFsTarget::setCacheDirectory(std::string path) 
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

bool NtfsFsTarget::writeVstrmData(int nIndex, char* pBuf, unsigned int nLen)
{
	unsigned long amountWritten;
	if(0 == _subFiles[nIndex].cacheCurLength && NTFSFS_IO_SIZE == nLen)
	{
		if(!WriteFile(_subFiles[nIndex].hOutputFile, (char*)pBuf, nLen, &amountWritten, NULL))
		{
			std::string errstr;
			getSystemErrorText(errstr);
			
			SetLastError(std::string("Vstrm failed to write file ") + _subFiles[nIndex].strFilename + " with error:" + errstr);
			MOLOG(Log::L_ERROR, CLOGFMT(NTFSFileset, "[%s] VsWrite() failed with error: %s)"), _strLogHint.c_str(), errstr.c_str());
			
			return false;
		}
	}
	else if(_subFiles[nIndex].cacheCurLength + nLen < NTFSFS_IO_SIZE)
	{
		memcpy(&_subFiles[nIndex].cacheBuffer[_subFiles[nIndex].cacheCurLength], pBuf, nLen);
		_subFiles[nIndex].cacheCurLength += nLen;		
	}
	else // if(_cacheCurLength + nLen >= _cacheBufferSize)
	{
		// the _temp64Buff will reach 64K
		DWORD dwLeftBytes = nLen;
		DWORD dwCopyBytes = NTFSFS_IO_SIZE - _subFiles[nIndex].cacheCurLength;			
		
		int totalCopiedBytes = 0;
		do 
		{
			memcpy(&_subFiles[nIndex].cacheBuffer[_subFiles[nIndex].cacheCurLength], (pBuf+totalCopiedBytes), dwCopyBytes);
			
			totalCopiedBytes += dwCopyBytes;
			
			if(!WriteFile(_subFiles[nIndex].hOutputFile, _subFiles[nIndex].cacheBuffer, NTFSFS_IO_SIZE, &amountWritten, NULL))
			{
				std::string errstr;
				getSystemErrorText(errstr);
				
				SetLastError(std::string("Vstrm failed to write file ") + _subFiles[nIndex].strFilename + " with error:" + errstr);
				MOLOG(Log::L_ERROR, CLOGFMT(NTFSFileset, "[%s] VsWrite() failed with error: %s)"), _strLogHint.c_str(), errstr.c_str());
				
				return false;
			}
			
			dwLeftBytes -= dwCopyBytes;
			dwCopyBytes = dwLeftBytes < NTFSFS_IO_SIZE ? dwLeftBytes : NTFSFS_IO_SIZE;			
		}while(dwLeftBytes >= NTFSFS_IO_SIZE);
		
		// copy the left bytes in pBufferData to cache buffer for later flushing
		if (dwLeftBytes)
		{
			memcpy(&_subFiles[nIndex].cacheBuffer[0], (pBuf+totalCopiedBytes), dwLeftBytes);
			_subFiles[nIndex].cacheCurLength = dwLeftBytes;
		}		
	}

	_subFiles[nIndex].llProcBytes += nLen;
	return true;
}

bool NtfsFsTarget::writeIndexData(char* pBuf, unsigned int nLen, unsigned int uOffetLow, unsigned int uOffetHigh)
{
	DWORD amountWritten = 0;
	DWORD dwPtr = SetFilePointer(_ntfsCacheHandle, uOffetLow, (long*)&uOffetHigh, FILE_BEGIN);
	if (dwPtr == INVALID_SET_FILE_POINTER) // Test for failure
	{ 
		// Obtain the error code. 
		std::string strErr;
		getSystemErrorText(strErr);
		MOLOG(Log::L_ERROR, CLOGFMT(NTFSFileset, "[%s] Failed to SetFilePointer to offset Low[%d], High[%d], error: %s"),
			_strLogHint.c_str(), uOffetLow, uOffetHigh, strErr.c_str());
		
		return false;
	}

	if(!WriteFile(_ntfsCacheHandle, pBuf, nLen, &amountWritten, NULL))
	{
		std::string errstr;
		getSystemErrorText(errstr);		
		SetLastError(std::string("failed to write file ") + _subFiles[3].strFilename + " on local with error:" + errstr);
		MOLOG(Log::L_ERROR, CLOGFMT(NTFSFileset, "[%s] Failed to create write file: %s with error: %s"),
			_strLogHint.c_str(), _subFiles[3].strFilename.c_str(), errstr.c_str());

		return false;
	}

	return true;
}

bool NtfsFsTarget::releaseVstrmBW()
{	
// 	if(0 == _bwmgrClientId)
// 		return true;
// 	
// 	for(int i=0;i<4;i++)
// 	{
// 		if (0 == _subFiles[i].reservedBW || 0 == _subFiles[i].bwTicket)
// 			continue;
// 
// 		VSTATUS	statusTicket = ERROR_SUCCESS;
// 		
// 		MOLOG(Log::L_INFO, CLOGFMT(NTFSFileset, "[%s] VstrmClassReleaseBandwidth(BW - %dbps) for ticket: 0x%I64X"), 
// 			_strLogHint.c_str(), _subFiles[i].reservedBW, _subFiles[i].bwTicket);
// 		statusTicket = VstrmClassReleaseBandwidth(_hVstrm, _subFiles[i].bwTicket);
// 		
// 		//  it always return failure: VSTRM_NOT_SUPPORTED even the bandwidth was released indeed
// 		if (statusTicket != VSTRM_SUCCESS)
// 		{
// 			char szBuf[255] = {0};
// 			VstrmClassGetErrorText(_hVstrm, statusTicket, szBuf, sizeof(szBuf));
// 			
// 			MOLOG(Log::L_WARNING, CLOGFMT(NTFSFileset, "[%s] VstrmClassReleaseBandwidth(%d pbs) for ticket 0x%I64X failed with error %s"), 
// 				_strLogHint.c_str(), _subFiles[i].reservedBW, _subFiles[i].bwTicket, szBuf);
// 			
// 			_subFiles[i].bwTicket = 0;
// 			
// 			continue;
// 		}
// 		
// 		_subFiles[i].bwTicket = 0;
// 		MOLOG(Log::L_INFO, CLOGFMT(NTFSFileset, "[%s] VstrmClassReleaseBandwidth(BW - %dbps) succeed"),  _strLogHint.c_str(), _subFiles[i].reservedBW);
// 	}
	
	return true;
}

void NtfsFsTarget::delOutput()
{
	for(int i=0;i<4;i++)
	{
		if(_subFiles[i].strFilename.empty())
			continue;
		
		// delete the file
		if(DeleteFile(_subFiles[i].strFilename.c_str()))
		{
			MOLOG(Log::L_INFO, CLOGFMT(NTFSFileset, "[%s] file %s was successfully deleted"), _strLogHint.c_str(), _subFiles[i].strFilename.c_str());
			continue;
		}
		
		std::string strErr;
		getSystemErrorText(strErr);
		MOLOG(Log::L_WARNING, CLOGFMT(NTFSFileset, "[%s] failed to delete file %s with error: %s"), _strLogHint.c_str(), _subFiles[i].strFilename.c_str(),
			strErr.c_str());
	}
}

bool NtfsFsTarget::copyFileToVstrm(std::string sourceFile, std::string desFile, std::string& errmsg, bool bDisableBufDrvThrottle, HANDLE hVstrm)
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
	char buffer[NTFSFS_IO_SIZE];
	
	HANDLE ntfsFileHandle = ::CreateFileA(sourceFile.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	
	bool succeed = true;
	if(ntfsFileHandle != INVALID_HANDLE_VALUE)
	{
		while(ReadFile(ntfsFileHandle, (LPVOID)buffer, NTFSFS_IO_SIZE, &dwBytesRead, NULL) && dwBytesRead > 0)
		{
			if(!WriteFile(vstrmFileHandle, buffer, dwBytesRead, &amountWritten, NULL))
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
	
	CloseHandle(vstrmFileHandle);
	
	return succeed;
}

void NtfsFsTarget::setFilename(const char* szFile)
{
	_strFilename = szFile;
}

NtfsFsTarget::~NtfsFsTarget()
{


}

void NtfsFsTarget::setBandwidth(unsigned int uBandwidthBps)
{
	_dwBandwidth = uBandwidthBps;
	MOLOG(Log::L_INFO, CLOGFMT(NTFSFileset, "[%s] set Bandwidth=%d"), _strLogHint.c_str(), uBandwidthBps);
}



void NtfsFsTarget::pacingAppReportOffsets(const void * const pCbParam, const LONGLONG offset1, const LONGLONG offset2)
{
	UNREFERENCED_PARAMETER(pCbParam);
	UNREFERENCED_PARAMETER(offset1);
	UNREFERENCED_PARAMETER(offset2);
}


#define XX(a,b) {a, b}
const char* NtfsFsTarget::DecodePacedIndexError(const unsigned long err)
{
	const char *errString = "unknown error";
	
	static struct
	{
		unsigned long code;
		char *str;
	} 
	errors[] = 
	{
		PACED_INDEX_ERROR_TABLE
	};
	
	for (int i = 0; i < (sizeof(errors) / sizeof(errors[0])); i++)
	{
		if (err == errors[i].code)
		{
			errString = errors[i].str;
		}
	}
	
	return errString;
}

void NtfsFsTarget::pacingAppLogCbk(const char * const pMsg)
{
	char				buf[1024];
	int					len;
	unsigned long		written = 0;
	
	//
	// If a msg has arrived without a CRLF, give it one now
	//
	len = strlen(pMsg);
	
	if (len > 1024 - 2)
	{
		len = 1024 - 2;
	}
	
	memcpy(buf, pMsg, len);
	buf[len] = 0;
	
}

int NtfsFsTarget::pacingAppWrite(const void * const pCbParam, const int len, const char buf[])
{
	SubFile* subfile = (SubFile*) pCbParam;
	NtfsFsTarget* pThis = subfile->pThis;
	
	DWORD writelen = len;
// 	if(!subfile->pacingWrite(buf, writelen))
// 	{
// 		return -1;
// 	}
//	_graph.traceLog(GetCurrentThreadId(), "VstrmIORender: pacingWrite buffer size %d bytes for %s", len, _outputFileName.c_str());
	
	DWORD amountWritten;
	bool ret;
	try
	{
		ret = WriteFile(subfile->hOutputFile, (char*)buf,  len, &amountWritten, NULL);
		if (!ret)
		{
			//log
			return -1;
		}
	}
	catch(...)
	{
// 		_graph.writeLog(ZQ::common::Log::L_ERROR, GetCurrentThreadId(), "VstrmIORender: pacingWrite(%s) met unknown exception", 
// 			_outputFileName.c_str());
		return false;
	}
	
	return amountWritten;
}

int NtfsFsTarget::pacingAppSeek(const void * const pCbParam, const LONGLONG offset)
{
	SubFile* subfile = (SubFile*) pCbParam;
	NtfsFsTarget* pThis = subfile->pThis;
	
//	_graph.traceLog(GetCurrentThreadId(), "VstrmIORender: pacingSeek offset %lld bytes for %s", offset, _outputFileName.c_str());
	
	LARGE_INTEGER	tmp;
	tmp.QuadPart = offset;
	
	DWORD dwError = 0;
	DWORD dwPtrLow = 0;
	try
	{
		DWORD dwPtr = SetFilePointer(subfile->hOutputFile, tmp.LowPart, &tmp.HighPart, FILE_BEGIN);
		if (dwPtr == INVALID_SET_FILE_POINTER) // Test for failure
		{ 
			// Obtain the error code. 
			std::string strErr;
			getSystemErrorText(strErr);
			SMOLOG(Log::L_ERROR, CLOGFMT(NTFSFileset, "[%s] Failed to SetFilePointer to offset Low[%d], High[%d], error: %s"),
				pThis->_strLogHint.c_str(), tmp.LowPart, tmp.HighPart, strErr.c_str());
			
			return -1;
		}
	}
	catch (...) 
	{
		return -1;
	}

	return 0;
}

int NtfsFsTarget::pacingAppSetEOF(const void * const pCbParam, const LONGLONG offset)
{
	SubFile* subfile = (SubFile*) pCbParam;
	NtfsFsTarget* pThis = subfile->pThis;

	LARGE_INTEGER	tmp;
	tmp.QuadPart = offset;		

	DWORD dwPtr = SetFilePointer(subfile->hOutputFile, tmp.LowPart, &tmp.HighPart, FILE_BEGIN);
	if (dwPtr == INVALID_SET_FILE_POINTER) // Test for failure
	{ 
		// Obtain the error code. 
		std::string strErr;
		getSystemErrorText(strErr);
		SMOLOG(Log::L_ERROR, CLOGFMT(NTFSFileset, "[%s] Failed to SetFilePointer to offset Low[%d], High[%d], error: %s"),
			pThis->_strLogHint.c_str(), tmp.LowPart, tmp.HighPart, strErr.c_str());
		
		return -1;
	}

	if (!SetEndOfFile(subfile->hOutputFile))
		return -1;
// 	VSTATUS vstat = VsSetEndOfFile(subfile->hOutputFile, subfile->objectId, &tmp);
// 	if (!IS_VSTRM_SUCCESS(vstat))
// 	{
// //		_graph.writeLog(ZQ::common::Log::L_INFO, GetCurrentThreadId(), "VstrmIORender: pacingSetEOF offset %lld bytes for %s", offset, _outputFileName.c_str());
// 		return -1;
// 	}

	return 0;
}

bool NtfsFsTarget::pacingWriteIndex()
{
	// move file point for reading
#ifdef _DEBUG
	MOLOG(Log::L_DEBUG, CLOGFMT(NTFSFileset, "[%s] Set index reader position: %lld"), _strLogHint.c_str(), _posCacheRead.QuadPart);
#endif	
	if(INVALID_SET_FILE_POINTER == SetFilePointer(_ntfsCacheHandle, _posCacheRead.LowPart, &_posCacheRead.HighPart, FILE_BEGIN))
	{
		return false;
	}
	
	DWORD byteRead = 0;
	if(!ReadFile(_ntfsCacheHandle, _tmpbuffer, NTFSFS_IO_SIZE, &byteRead, NULL))
	{
		// log for fail
		return false;
	}
	
	// if we get a single terminator record, and the other writer is gone,
	// then return the terminator - the next read will be 0 bytes
	if(sizeof(VVX_V7_RECORD) == byteRead)
	{
		VVX_V7_RECORD *pRecord = (VVX_V7_RECORD *)(_tmpbuffer);
		
		if(TRICK_INDEX == pRecord->recordType && 0 == pRecord->trick.timeCode.time)
		{
			LARGE_INTEGER tmp;
			tmp.QuadPart = _vvxByteRead;
			if(0xFFFFFFFF == SetFilePointer(_ntfsCacheHandle, tmp.LowPart, &tmp.HighPart, FILE_BEGIN))
			{
				// log for fail
				return false;
			}
			else
			{
				// remember the position for reading
				// _posCacheRead = tmp; 
				return true;						
			}
		}
	}
	
	// not enough data found in first read
	if(0 == _vvxByteRead)
	{
		if(byteRead < sizeof(VVX_V7_INDEX_HEADER))
		{
			// initial read too shot
			return false;
		}
	}
	
	// there are two conditions where we'll adjust buffer length
	// and reset the NT file pointer
	// 1: last record is not a complete vvx index record
	// 2: last record is a terminator
	
	int iPartial;
	if(0 == _vvxByteRead)
	{
		// first read - offset is calculated from the frame data offset
		VVX_V7_INDEX_HEADER *pTmp = (VVX_V7_INDEX_HEADER *)_tmpbuffer;
		
		iPartial = (byteRead - pTmp->frameDataOffset) % sizeof(VVX_V7_RECORD);
	}
	else
	{
		// not the first read - offset is calculated from the beginning of
		// the buffer because we always reset the file pointer to the start
		// of a record
		iPartial = byteRead % sizeof(VVX_V7_RECORD);
	}
	
	if(iPartial)
	{
		// reduce buffer length
		byteRead -= iPartial;
		
		// set file pointer
		_vvxByteRead += byteRead;
		
		LARGE_INTEGER tmp;
		tmp.QuadPart = _vvxByteRead;
		if(0xFFFFFFFF == SetFilePointer(_ntfsCacheHandle, tmp.LowPart, &tmp.HighPart, FILE_BEGIN))
		{
			// log for fail
			return false;
		}
		
		// remember the position for reading
		_posCacheRead = tmp;				
	}
	else
	{
		// check if the last record in the buffer is a terminator
		VVX_V7_RECORD *pRecord = 
			(VVX_V7_RECORD *)(_tmpbuffer + byteRead - sizeof(VVX_V7_RECORD));
		
		if(TRICK_INDEX == pRecord->recordType && 
			0 == pRecord->trick.timeCode.time)
		{
			// gotcha
			byteRead -= sizeof(VVX_V7_RECORD);
			
			_vvxByteRead += byteRead;
			
			LARGE_INTEGER tmp;
			tmp.QuadPart = _vvxByteRead;
			if(0xFFFFFFFF == SetFilePointer(_ntfsCacheHandle, tmp.LowPart, &tmp.HighPart, FILE_BEGIN))
			{
				// log for fail
				return false;
			}
			// remember the position for reading
			_posCacheRead = tmp;
		}
		else
		{
			_vvxByteRead += byteRead;
			
			// remember the position for reading
			_posCacheRead.QuadPart = byteRead;
		}
	}

#ifdef _DEBUG
 	MOLOG(Log::L_DEBUG, CLOGFMT(NTFSFileset, "[%s] Save index reader position: %lld"), 
 		_strLogHint.c_str(), _posCacheRead.QuadPart);	
#endif
	
	if(0 == byteRead)
		return true;
	
	// log the last index read
	VVX_V7_RECORD *pTmp = 
		(VVX_V7_RECORD *)(_tmpbuffer + byteRead - sizeof(VVX_V7_RECORD));
	
#ifdef _DEBUG
	MOLOG(Log::L_DEBUG, CLOGFMT(NTFSFileset, "[%s] last index read: %#I64x (%#I64x)(%#I64x)"),
		_strLogHint.c_str(),
		pTmp->trick.frameByteOffset[0],
		pTmp->trick.frameByteOffset[1],
		pTmp->trick.frameByteOffset[2]);
#endif
	
	// pass the buffer to pacing
	DWORD paceresult = 0;
	try
	{
		if(PacedIndexWrite(_subFiles[3].pacingIndexCtx, byteRead, _tmpbuffer, &paceresult) < 0)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(NTFSFileset, "[%s] PacedIndexWrite(%s) failed with error"), 
				_strLogHint.c_str(), DecodePacedIndexError(paceresult));
			return false;
		}
	}
	catch(...)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(NTFSFileset, "[%s] PacedIndexWrite(%s) met unknown exception"), 
			_strLogHint.c_str());
		
		return false;
	}

	return true;
}




bool NtfsFsTarget::pacingWriteData(int nIndex, char* pBuf, unsigned int nLen)
{
	DWORD paceresult = 0;
	if(0 == _subFiles[nIndex].cacheCurLength && NTFSFS_IO_SIZE == nLen)
	{		
		if(PacedIndexWrite(_subFiles[nIndex].pacingIndexCtx, NTFSFS_IO_SIZE, (char*)pBuf, &paceresult) < 0)
		{
			std::string errstr = DecodePacedIndexError(paceresult);
			SetLastError(std::string("PacedIndexWrite failed to write file ") + _subFiles[nIndex].strFilename + " with error:" + errstr);
			MOLOG(Log::L_ERROR, CLOGFMT(NTFSFileset, "[%s] PacedIndexWrite() file %s with error: %s)"), 
				_strLogHint.c_str(), _subFiles[nIndex].strFilename.c_str(), errstr.c_str());
			
			return false;
		}
	}
	else if(_subFiles[nIndex].cacheCurLength + nLen < NTFSFS_IO_SIZE)
	{
		memcpy(&_subFiles[nIndex].cacheBuffer[_subFiles[nIndex].cacheCurLength], pBuf, nLen);
		_subFiles[nIndex].cacheCurLength += nLen;		
	}
	else // if(_cacheCurLength + nLen >= _cacheBufferSize)
	{
		// the _temp64Buff will reach 64K
		DWORD dwLeftBytes = nLen;
		DWORD dwCopyBytes = NTFSFS_IO_SIZE - _subFiles[nIndex].cacheCurLength;			
		
		int totalCopiedBytes = 0;
		do 
		{
			memcpy(&_subFiles[nIndex].cacheBuffer[_subFiles[nIndex].cacheCurLength], (pBuf+totalCopiedBytes), dwCopyBytes);
			
			totalCopiedBytes += dwCopyBytes;
		
			if(PacedIndexWrite(_subFiles[nIndex].pacingIndexCtx, NTFSFS_IO_SIZE, _subFiles[nIndex].cacheBuffer, &paceresult) < 0)
			{
				std::string errstr = DecodePacedIndexError(paceresult);
				SetLastError(std::string("PacedIndexWrite failed to write file ") + _subFiles[nIndex].strFilename + " with error:" + errstr);
				MOLOG(Log::L_ERROR, CLOGFMT(NTFSFileset, "[%s] PacedIndexWrite() file %s with error: %s)"), 
					_strLogHint.c_str(), _subFiles[nIndex].strFilename.c_str(), errstr.c_str());
				
				return false;
			}

			dwLeftBytes -= dwCopyBytes;
			dwCopyBytes = dwLeftBytes < NTFSFS_IO_SIZE ? dwLeftBytes : NTFSFS_IO_SIZE;			
		}while(dwLeftBytes >= NTFSFS_IO_SIZE);
		
		// copy the left bytes in pBufferData to cache buffer for later flushing
		if (dwLeftBytes)
		{
			memcpy(&_subFiles[nIndex].cacheBuffer[0], (pBuf+totalCopiedBytes), dwLeftBytes);
			_subFiles[nIndex].cacheCurLength = dwLeftBytes;
		}		
	}
	
	_subFiles[nIndex].llProcBytes += nLen;
	return true;
}


}}