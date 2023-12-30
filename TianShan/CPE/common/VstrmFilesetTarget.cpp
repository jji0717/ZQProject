
#include "Log.h"
#include "vstrmuser.h"
#include "vsiolib.h"
#include "VstrmFilesetTarget.h"
#include "PacedIndex.h"
#include "vvx.h"
#include "ErrorCode.h"

extern "C" {
#include "CTFLib.h"
}


#define VsmFsIO			"VsmFsIO"

#pragma comment(lib, "VstrmDLLEx.lib")
#pragma comment(lib, "PacedIndex.lib")
#pragma comment(lib, "vsiolib.lib")

using namespace ZQ::common;

#define MOLOG (*_pLog)
#define SMOLOG (*(pThis->_pLog))


namespace ZQTianShan {
	namespace ContentProvision {

HANDLE VstrmFsTarget::_hVstrm = INVALID_HANDLE_VALUE;
bool VstrmFsTarget::_bPacingTrace = false;


VstrmFsTarget::VstrmFsTarget():_bTypeH264(false)
{
	_nOutputCount = 0;
	_nInputCount = 10;

	int i;
	for(i=0;i<_nInputCount;i++)
	{
		InputPin pin;
		pin.nPrevPin = 0;
		pin.pPrevFilter = 0;		
		_inputPin.push_back(pin);
	}

	_bDriverModule = false;

	for(i=0;i<10;i++)
	{
		SubFile& subFile = _subFiles[i];

		subFile.bIndexFile = false;
		subFile.objectId = 0;
		subFile.hOutputFile = 0;
		subFile.cacheCurLength = 0;
		subFile.llProcBytes = 0;		
		subFile.bwTicket = 0;
		subFile.reservedBW = 0;
		subFile.bwPercentageN = 0;   // numerator for calculating bandwidth
		subFile.bwPercentageD = 0;   // denominator for calculating bandwidth
		subFile.pacingIndexCtx = 0;	//context of pacing

		subFile.llProcOffset = 0;		
	}
	_ntfsCacheHandle = NULL;
	_bDeleteOnFail = true;
	_bPacing = false;
	_posCacheRead.QuadPart = 0;
	_vvxByteRead = 0;
	_bCleanup = false;
	_bwmgrClientId = 0;
	_disableBufDrvThrottle = true;
	_bEnableMD5 = false;
}

void VstrmFsTarget::InitPins()
{

	_nInputCount = 2 + _SpeedAndFileExt.size();

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

		subFile.bIndexFile = false;
		subFile.objectId = 0;
		subFile.hOutputFile = 0;
		subFile.cacheCurLength = 0;
		subFile.llProcBytes = 0;		
		subFile.bwTicket = 0;
		subFile.reservedBW = 0;
		subFile.bwPercentageN = 0;   // numerator for calculating bandwidth
		subFile.bwPercentageD = 0;   // denominator for calculating bandwidth
		subFile.pacingIndexCtx = 0;	//context of pacing

		subFile.llProcOffset = 0;		
	}
}

bool VstrmFsTarget::Init()
{
	// initialize the vstrm handle, this make only one handle in the whole filter life
	std::string _vstrmErrMsg;

	_subFiles[0].reservedBW = _dwBandwidth;
	_subFiles[0].strFilename = _strFilename;
	_subFiles[0].strPacename = _strFilename + std::string(".mpg");

	std::map<std::string, int>::iterator iter;

	if (!_bTypeH264)
	{
		int i = 1;
		_subFiles[i].bIndexFile = true;
		_subFiles[i].strFilename = _strFilename + ".vvx";
		_subFiles[i].reservedBW = _dwBandwidth/20;
		_subFiles[i].strPacename =  _strFilename + std::string(".mpg") + std::string(".vvx");


		for (iter = _SpeedAndFileExt.begin(); iter != _SpeedAndFileExt.end(); iter++)
		{
			i++;
			_subFiles[i].strFilename = _strFilename + (*iter).first;
			_subFiles[i].reservedBW = _dwBandwidth/(((*iter).second + 1)*6);
			_subFiles[i].strPacename =  _strFilename + std::string(".mpg") + (*iter).first;
		}
	}
	else
	{
		int i = 1;

		_subFiles[i].bIndexFile = true;
		_subFiles[i].strFilename = _strFilename + ".vv2";
		_subFiles[i].reservedBW = _dwBandwidth/20;
		_subFiles[i].strPacename =  _strFilename + std::string(".mpg") + std::string(".vv2");

		for (iter = _SpeedAndFileExt.begin(); iter != _SpeedAndFileExt.end(); iter++)
		{
			i++;
			_subFiles[i].strFilename = _strFilename + (*iter).first;
			_subFiles[i].reservedBW = _dwBandwidth/(((*iter).second + 1)*6);
			_subFiles[i].strPacename =  _strFilename + std::string(".mpg") + (*iter).first;
		}
	}

	MOLOG(Log::L_INFO, CLOGFMT(VsmFsIO, "[%s] subFile setting is successful"), _strLogHint.c_str());

	_bCleanup = false;

	for(int i=0;i<_nInputCount;i++)
	{
		SubFile& subFile = _subFiles[i];

//		MOLOG(Log::L_INFO, CLOGFMT(VsmFsIO, "The %d output file name is %s:%s."),i, _subFiles[i].strFilename.c_str(),subFile.strFilename.c_str() );
#ifdef FILE_FLAG_CACHED
		VSTATUS vstat = VsOpenEx( &subFile.hOutputFile,
			(char*)subFile.strFilename.c_str(), 
			GENERIC_WRITE,

			FILE_SHARE_READ | FILE_SHARE_WRITE,
			CREATE_ALWAYS,
			(subFile.bIndexFile)?0:FILE_FLAG_CACHED,		//disable FILE_FLAG_CACHED for index file, as Rick asked, 10/28 2008
			0,
			&subFile.objectId);
#else
		VSTATUS vstat = VsOpen( &subFile.hOutputFile,
			(char*)subFile.strFilename.c_str(), 
			GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			CREATE_ALWAYS,
			0,
			&subFile.objectId);
#endif
		if(!IS_VSTRM_SUCCESS(vstat))
		{
			std::string errstr;
			getVstrmErrorWithVStatus(_hVstrm, vstat, errstr);
			
			char msg[256];
			sprintf(msg, "VsOpenEx() failed with error: %s", errstr.c_str());
			int nErrorCode;
			if (vstat == VSTRM_DISK_FULL)
				nErrorCode = ERRCODE_VSTRM_DISK_FULL;
			else if (vstat == VSTRM_NETWORK_NOT_READY)
				nErrorCode = ERRCODE_VSTRM_NOT_READY;
			else
				nErrorCode = ERRCODE_VSTRM_API_ERROR;				

			SetLastError(msg, nErrorCode);
			
			MOLOG(Log::L_ERROR, CLOGFMT(VsmFsIO, "[%s] Failed to create output file: %s with error: %s"),
				_strLogHint.c_str(), subFile.strFilename.c_str(), errstr.c_str());

			return false;
		}
		MOLOG(Log::L_INFO, CLOGFMT(VsmFsIO, "open file [%s] on vstrm successful"),
			subFile.strFilename.c_str());

		// disable BufDrv throttling to have better Vstrm IO performance
		if(_disableBufDrvThrottle)
		{
			std::string errmsg;
			if(disableBufDrvThrottle(_hVstrm, subFile.hOutputFile, subFile.objectId, errmsg))
			{
				MOLOG(Log::L_DEBUG, CLOGFMT(VsmFsIO, "[%s] BufDrv throttling disabed"), _strLogHint.c_str());
			}
			else
			{
				MOLOG(Log::L_WARNING, CLOGFMT(VsmFsIO, "[%s] failed to disable BufDrv throttling with error: %s"), _strLogHint.c_str(), errmsg.c_str());
			}
		} // end of if(_disableBufDrvThrottle)

		subFile.llProcBytes = 0;
		subFile.pThis = this;
	}

	// reserve VStrm bandwidth
	std::string strVstrmErr;
	if(!reserveVstrmBW(strVstrmErr))
	{
		SetLastError(std::string("reserve vstrm bandwith failed with error: ") + strVstrmErr, ERRCODE_VSTRM_BANDWIDTH_EXCEEDED);
		return false;	
	}

	if(_bPacing)
	{
		//
		// generate pacing group
		//
		void* pacingGroup = this;

		DWORD indexFileNo = 1;
		std::string pacingType;
		if (!_bTypeH264)
			pacingType="vvx";
		else
			pacingType ="vv2";

		PacedIndexSetLogCbk(1, pacingAppLogCbk);
		DWORD paceresult = 0;
		for(DWORD i=0; i<(DWORD)_nInputCount; i++)
		{
			SubFile& subFile = _subFiles[i];

			// skip the vvx, make sure it was added last
			if(i == indexFileNo)
				continue;

			paceresult = PacedIndexAdd((void *)pacingGroup, pacingType.c_str(), subFile.strPacename.c_str(), 
				pacingAppWrite, pacingAppSeek, pacingAppSetEOF, pacingAppReportOffsets, 
				(void*)&subFile, &subFile.pacingIndexCtx);
			if(paceresult)
			{
				MOLOG(Log::L_ERROR, CLOGFMT(VsmFsIO, "[%s] PacedIndexAdd() on pacing group 0x%08x failed with error [%s]"), 
					subFile.strPacename.c_str(), (DWORD)(pacingGroup), DecodePacedIndexError(paceresult));
				SetLastError(std::string("PacedIndexAdd() failed with error ") + DecodePacedIndexError(paceresult), ERRCODE_PACING_ERROR);
				return false;	
			}
			MOLOG(Log::L_INFO, CLOGFMT(VsmFsIO, "[%s] PacedIndexAdd() on pacing group 0x%08x successful"), 
				subFile.strPacename.c_str(), (DWORD)(pacingGroup));
		}
		
		// add vvx last
		paceresult = PacedIndexAdd((void*)pacingGroup, pacingType.c_str(), _subFiles[indexFileNo].strPacename.c_str(), 
			pacingAppWrite, pacingAppSeek, pacingAppSetEOF, pacingAppReportOffsets, 
			(void*)&_subFiles[indexFileNo], &_subFiles[indexFileNo].pacingIndexCtx);
		if(paceresult)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(VsmFsIO, "[%s] PacedIndexAdd() on pacing group 0x%08x failed with error [%s]"), 
				_subFiles[indexFileNo].strPacename.c_str(), (DWORD)(pacingGroup), DecodePacedIndexError(paceresult));
			SetLastError(std::string("PacedIndexAdd() failed with error ") + DecodePacedIndexError(paceresult), ERRCODE_PACING_ERROR);
			return false;	
		}
		MOLOG(Log::L_INFO, CLOGFMT(VsmFsIO, "[%s] PacedIndexAdd() on pacing group 0x%08x successful"), 
			_subFiles[indexFileNo].strPacename.c_str(), (DWORD)(pacingGroup));
	}

#if defined(RTFLIB_SDK_VERSION) && (RTFLIB_SDK_VERSION <= 20)
	{
		std::string filePath = _cachePath + _subFiles[1].strFilename;
		MOLOG(Log::L_INFO, CLOGFMT(VsmFsIO, "[%s] Create output file %s on local disk"), _strLogHint.c_str(),
			filePath.c_str());
		
		_ntfsCacheHandle = CreateFileA(filePath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);		
		if (INVALID_HANDLE_VALUE == _ntfsCacheHandle)
		{
			std::string errmsg;
			getSystemErrorText(errmsg);
			SetLastError(errmsg, ERRCODE_NTFS_CREATEFILE);	
			
			MOLOG(Log::L_ERROR, CLOGFMT(VsmFsIO, "[%s] CreateFile failed with error: %s"), _strLogHint.c_str(), errmsg.c_str());
			return false;
		}
	}
#endif
	
	return true;
}

void VstrmFsTarget::Stop()
{
	
}

bool VstrmFsTarget::flushCacheBuffer()
{	
	for(int i=0;i<_nInputCount;i++)
	{
		if (i == 1)
			continue;

		SubFile& subFile = _subFiles[i];

		if(subFile.hOutputFile == INVALID_HANDLE_VALUE)
			continue;
		
		if(subFile.cacheCurLength > 0)
		{	
			DWORD amountWritten;
			if(!VsWrite(subFile.hOutputFile, subFile.cacheCurLength, subFile.cacheBuffer, &amountWritten, NULL))
			{
				std::string errstr;
				unsigned int vstat;
				getVstrmError(_hVstrm, vstat, errstr);
				int nErrorCode;
				if (vstat == VSTRM_DISK_FULL)
					nErrorCode = ERRCODE_VSTRM_DISK_FULL;
				else if (vstat == VSTRM_NETWORK_NOT_READY)
					nErrorCode = ERRCODE_VSTRM_NOT_READY;
				else if (vstat == VSTRM_NO_MEMORY)
					nErrorCode = ERRCODE_VSTRM_BUFFERQUEUE_FULL;
				else
					nErrorCode = ERRCODE_VSTRM_API_ERROR;				
				
				SetLastError(std::string("Vstrm failed to write file ") + subFile.strFilename + " with error:" + errstr, nErrorCode);
				MOLOG(Log::L_ERROR, CLOGFMT(VsmFsIO, "[%s] VsWrite() failed with error: %s)"), _strLogHint.c_str(), errstr.c_str());

				return false;
			}

			subFile.cacheCurLength = 0;
		}		
	}

	return true;
}

void VstrmFsTarget::Close()
{
	if (_bCleanup)
		return;

	flushCacheBuffer();

	if(!_bPacing)
	{
		int i;
		for(i=0;i<_nInputCount;i++)
		{
			if (i == 1)
				continue;

			SubFile& subFile = _subFiles[i];

			if(subFile.hOutputFile == INVALID_HANDLE_VALUE)
				continue;
			
			VsClose(subFile.hOutputFile, subFile.objectId);
			subFile.hOutputFile = INVALID_HANDLE_VALUE;
		}


		//
		// index file
#if defined(RTFLIB_SDK_VERSION) && (RTFLIB_SDK_VERSION <= 20)		
		if(_ntfsCacheHandle != NULL)
		{
			CloseHandle(_ntfsCacheHandle);
			_ntfsCacheHandle = NULL;
		}

		SubFile& subFile = _subFiles[1];
		std::string errmsg;
		std::string filePath = _cachePath + subFile.strFilename;
		if(!copyFileToVstrm(filePath, _hVstrm, subFile.hOutputFile, errmsg))
		{
			SetLastError(errmsg);
			MOLOG(Log::L_ERROR, CLOGFMT(VsmFsIO, "[%s] failed to copy file from NTFS to vstrm with error: %s"), 
				_strLogHint.c_str(), _subFiles[1].strFilename.c_str(), errmsg.c_str());
		}

		VsClose(subFile.hOutputFile, subFile.objectId);
		subFile.hOutputFile = INVALID_HANDLE_VALUE;
		
		MOLOG(Log::L_INFO, CLOGFMT(VsmFsIO, "[%s] delete local temp file %s"), 
			_strLogHint.c_str(), filePath.c_str());
		DeleteFileA(filePath.c_str());	
#else
		SubFile& subFile = _subFiles[1];
		VsClose(subFile.hOutputFile, subFile.objectId);
		subFile.hOutputFile = INVALID_HANDLE_VALUE;
#endif
	}	
	else
	{
		//
		// pacing
		//
		bool bEndFlag = false;
		for(int i=0;!bEndFlag;i++)
		{
			if (i == 1)
				continue;

			if (i >= _nInputCount) //PacedIndexRemove vvx file last
			{
				i = 1;
				bEndFlag = true;
			}

			SubFile& subFile = _subFiles[i];

			if (subFile.samples.size())
			{
				MOLOG(Log::L_INFO, CLOGFMT(VsmFsIO, "[%s][%d] sample queue size is %d while close"), 
					_strLogHint.c_str(), i, subFile.samples.size());
				
				std::vector<MediaSample*>::iterator it=subFile.samples.begin();
				for(;it!=subFile.samples.end();it++)
				{
					GetGraph()->freeMediaSample(*it);
				}
				subFile.samples.clear();
			}

			if(subFile.hOutputFile == INVALID_HANDLE_VALUE)
				continue;

			DWORD paceresult = 0;				
			//
			// Grab MD5 from the file
			//
			char md5[33] = "";
			if(subFile.pacingIndexCtx)
			{
				if (i == 1)	//only index file have this
				{
					paceresult = PacedIndexGetPacedFileMD5(subFile.pacingIndexCtx, md5);
					if (paceresult)
					{
						MOLOG(Log::L_WARNING, CLOGFMT(VsmFsIO, "[%s] PacedIndexGetPacedFileMD5() (%s) failed with error %s"), _strLogHint.c_str() 
							, subFile.strFilename.c_str(), DecodePacedIndexError(paceresult));
					}
					else
					{
						MOLOG(Log::L_INFO, CLOGFMT(VsmFsIO, "[%s] PacedIndexGetPacedFileMD5() (%s) return [%s]"), _strLogHint.c_str(), 
							subFile.strFilename.c_str(), md5);
					}
				}

				MOLOG(Log::L_INFO, CLOGFMT(VsmFsIO, "[%s] to PacedIndexRemove() (%s) file context"),
					_strLogHint.c_str(), subFile.strFilename.c_str());
				paceresult = PacedIndexRemove(subFile.pacingIndexCtx);
				if (paceresult)
				{
					MOLOG(Log::L_WARNING, CLOGFMT(VsmFsIO, "[%s] PacedIndexRemove() (%s) failed with error %s"), _strLogHint.c_str() 
						, subFile.strFilename.c_str(), DecodePacedIndexError(paceresult));
				}
				else
				{
					MOLOG(Log::L_INFO, CLOGFMT(VsmFsIO, "[%s] PacedIndexRemove (%s) file context successful"),
						_strLogHint.c_str(), subFile.strFilename.c_str());
				}
				subFile.pacingIndexCtx = NULL;				
			}

			MOLOG(Log::L_INFO, CLOGFMT(VsmFsIO, "[%s] to VsClose file (%s)"),
				_strLogHint.c_str(), subFile.strFilename.c_str());
			VsClose(subFile.hOutputFile, subFile.objectId);
			subFile.hOutputFile = INVALID_HANDLE_VALUE;
			MOLOG(Log::L_INFO, CLOGFMT(VsmFsIO, "[%s] VsClose file (%s) successful"),
				_strLogHint.c_str(), subFile.strFilename.c_str());
		}

#if defined(RTFLIB_SDK_VERSION) && (RTFLIB_SDK_VERSION <= 20)
		if(_ntfsCacheHandle != NULL)
		{
			CloseHandle(_ntfsCacheHandle);
			_ntfsCacheHandle = NULL;
		}

		std::string errmsg;
		std::string filePath = _cachePath + _subFiles[1].strFilename;
		MOLOG(Log::L_INFO, CLOGFMT(VsmFsIO, "[%s] delete local temp file %s"), 
			_strLogHint.c_str(), filePath.c_str());
		DeleteFileA(filePath.c_str());
#endif
	}

	// release the reserved VStrm bandwidth 
	releaseVstrmBW();
	
	// delete error files
	if(_bDeleteOnFail && GetGraph()->IsErrorOccurred())
	{
		delOutput();
	}

	_bCleanup = true;
	MOLOG(Log::L_INFO, CLOGFMT(VsmFsIO, "[%s] closed"), _strLogHint.c_str());
}

void VstrmFsTarget::endOfStream()
{
	GetGraph()->Finish();
}

const char* VstrmFsTarget::GetName()
{
	return TARGET_TYPE_VSTRMFS;
}

bool VstrmFsTarget::Receive(MediaSample* pSample, int nInputIndex)
{
	if (nInputIndex < 0 || nInputIndex >_nInputCount-1)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(VsmFsIO, "[%s] Receive() invalid input index: %d"), _strLogHint.c_str(), nInputIndex);
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
#if defined(RTFLIB_SDK_VERSION) && (RTFLIB_SDK_VERSION <= 20)
		if (!writeIndexData((char*)pointer, dwActualLen, uOffsetLow, uOffsetHigh))
			return false;			

		if (_bPacing)
		{
	         if (!pacingWriteIndex())
					return false;
		}
#else
	if (!pacingWriteData(nInputIndex, (char*)pointer, dwActualLen))
		return false;
#endif
			
	}
	else
	{
		if (_bPacing)
		{
			SubFile& subFile = _subFiles[nInputIndex];
//			samples
			
			LARGE_INTEGER llOffset;
			llOffset.LowPart = uOffsetLow;
			llOffset.HighPart = uOffsetHigh;
			if (subFile.llProcOffset == llOffset.QuadPart)
			{
				if (!pacingWriteData(nInputIndex, (char*)pointer, dwActualLen))
					return false;
				
				subFile.llProcOffset += dwActualLen;	
				
				//samples
				while(subFile.samples.size())
				{
					bool
						bFound = false;
					std::vector<MediaSample*>::iterator it=subFile.samples.begin();
					for(;it!=subFile.samples.end();it++)
					{
						MediaSample* pSam = *it;
						llOffset.LowPart = pSam->getOffset((unsigned int*)&llOffset.HighPart);
						if (subFile.llProcOffset == llOffset.QuadPart)
						{
							void* p = pSam->getPointer();
							DWORD dwDataLen = pSam->getDataLength();
							if (!pacingWriteData(nInputIndex, (char*)p, dwDataLen))
								return false;
							
							subFile.llProcOffset += dwDataLen;
							subFile.samples.erase(it);
							GetGraph()->freeMediaSample(pSam);
							bFound = true;
							MOLOG(Log::L_DEBUG, CLOGFMT(VsmFsIO, "[%s] sample queue size reduced to %d"), 
								subFile.strFilename.c_str(), subFile.samples.size());
							break;
						}
					}

					if (!bFound)
						break;					
				}
			}
			else
			{
				//make sure no error happen
				if (GetGraph()->IsErrorOccurred())
				{
					return false;
				}

#define MAX_QUEUED_SAMPLES	100
				if (subFile.samples.size()>=MAX_QUEUED_SAMPLES)
				{
					MOLOG(Log::L_ERROR, CLOGFMT(VsmFsIO, "[%s] exceeded max queue samples %d, seek too long, stop process"), 
						subFile.strFilename.c_str(), subFile.samples.size());
					SetLastError(std::string("exceeded max queue samples, seek too long"), ERRCODE_BUFFERQUEUE_FULL);
					return false;
				}
				else
				{
					subFile.samples.push_back(pSample);
					MOLOG(Log::L_DEBUG, CLOGFMT(VsmFsIO, "[%s] sample queue size increased to %d"), 
						subFile.strFilename.c_str(), subFile.samples.size());
					return true;
				}
			}
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

void VstrmFsTarget::getVstrmError(HANDLE hVstrm, unsigned int& status, std::string& strErr)
{
	if(hVstrm == INVALID_HANDLE_VALUE)
		return;
	
	char sErrorText[256]={0};
	
	status = VstrmGetLastError();
	
	VstrmClassGetErrorText(hVstrm, status, sErrorText, 255);
	
	char errcode[24];
	sprintf(errcode, "[0x%08x]", status);
	
    strErr = std::string(sErrorText)+ errcode;
}

void VstrmFsTarget::getVstrmErrorWithVStatus(HANDLE hVstrm, unsigned int status, std::string& strErr)
{
	if(hVstrm == INVALID_HANDLE_VALUE)
		return;
	
	char sErrorText[256]={0};
	VstrmClassGetErrorText(hVstrm, status, sErrorText, 255);
	
	char errcode[24];
	sprintf(errcode, "[0x%08x]", status);
	
    strErr = std::string(sErrorText)+ errcode;
}

bool VstrmFsTarget::disableBufDrvThrottle(HANDLE vstrmHandle, HANDLE fileHandle, OBJECT_ID objectId, std::string& errMsg)
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
		vstrmClass = vstrmHandle;
		
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

void VstrmFsTarget::uninitVstrm(HANDLE hvstrm)
{
	if(hvstrm != INVALID_HANDLE_VALUE)
	{
		// It is not recommended to invoke VstrmClassCloseEx function, but seems for streaming
		// but in case of !bRunningOnMC, must invoked
		VstrmClassCloseEx(hvstrm);
	}
}

bool VstrmFsTarget::initVstrm(ULONG bwClientId, std::string& errmsg)
{	
	VSTATUS vStatus;
	char szBuf[255] = {0};
	
	HANDLE hvstrm = INVALID_HANDLE_VALUE;
	vStatus = VstrmClassOpenEx(&hvstrm);
	if (vStatus != VSTRM_SUCCESS) 
	{
		VstrmClassGetErrorText(hvstrm, vStatus, szBuf, sizeof(szBuf));
		errmsg = szBuf;
		
		return false;
	} 
	_hVstrm = hvstrm;

	vStatus = VstrmClassReleaseAllBandwidth(hvstrm,bwClientId, 0);
	return true;
}

void VstrmFsTarget::uninitVstrm()
{
	uninitVstrm(_hVstrm);
}

/// set cache path
void VstrmFsTarget::setCacheDirectory(const std::string& path) 
{ 
	_cachePath = path; 
	if(_cachePath == "")
		return;
	
	int pos = _cachePath.length() - 1;
	if(_cachePath[pos] != '\\' && _cachePath[pos] != '/' )
	{
		_cachePath = _cachePath + "\\";
	}

	MOLOG(Log::L_INFO, CLOGFMT(VsmFsIO, "[%s] set CacheDirectory=%s"), _strLogHint.c_str(), _cachePath.c_str());
}

bool VstrmFsTarget::writeVstrmData(int nIndex, char* pBuf, unsigned int nLen)
{
	SubFile& subFile = _subFiles[nIndex];

	unsigned long amountWritten;
	if(subFile.cacheCurLength + nLen < VSTRM_IO_SIZE)
	{
		memcpy(&subFile.cacheBuffer[subFile.cacheCurLength], pBuf, nLen);
		subFile.cacheCurLength += nLen;		
	}
	else
	{
		DWORD dwLeftBytes = nLen;	
		int totalCopiedBytes = 0;

		if (subFile.cacheCurLength)
		{
			totalCopiedBytes = VSTRM_IO_SIZE - subFile.cacheCurLength;
			memcpy(&subFile.cacheBuffer[subFile.cacheCurLength], pBuf, totalCopiedBytes);
			dwLeftBytes -= totalCopiedBytes;

			//write
			if(!VsWrite(subFile.hOutputFile, VSTRM_IO_SIZE, subFile.cacheBuffer, &amountWritten, NULL))
			{
				std::string errstr;
				unsigned int vstat;
				getVstrmError(_hVstrm, vstat, errstr);
				int nErrorCode;
				if (vstat == VSTRM_DISK_FULL)
					nErrorCode = ERRCODE_VSTRM_DISK_FULL;
				else if (vstat == VSTRM_NETWORK_NOT_READY)
					nErrorCode = ERRCODE_VSTRM_NOT_READY;
				else if (vstat == VSTRM_NO_MEMORY)
					nErrorCode = ERRCODE_VSTRM_BUFFERQUEUE_FULL;
				else
					nErrorCode = ERRCODE_VSTRM_API_ERROR;				
				
				SetLastError(std::string("Vstrm failed to write file ") + subFile.strFilename + " with error:" + errstr, nErrorCode);
				MOLOG(Log::L_ERROR, CLOGFMT(VsmFsIO, "[%s] VsWrite() failed with error: %s)"), _strLogHint.c_str(), errstr.c_str());
				
				return false;
			}
			
			subFile.cacheCurLength = 0;
		}

		while (dwLeftBytes>=VSTRM_IO_SIZE)
		{
			//write directly
			if(!VsWrite(subFile.hOutputFile, VSTRM_IO_SIZE, pBuf+totalCopiedBytes, &amountWritten, NULL))
			{
				std::string errstr;
				unsigned int vstat;
				getVstrmError(_hVstrm, vstat, errstr);
				int nErrorCode;
				if (vstat == VSTRM_DISK_FULL)
					nErrorCode = ERRCODE_VSTRM_DISK_FULL;
				else if (vstat == VSTRM_NETWORK_NOT_READY)
					nErrorCode = ERRCODE_VSTRM_NOT_READY;
				else if (vstat == VSTRM_NO_MEMORY)
					nErrorCode = ERRCODE_VSTRM_BUFFERQUEUE_FULL;
				else
					nErrorCode = ERRCODE_VSTRM_API_ERROR;				
				
				SetLastError(std::string("Vstrm failed to write file ") + subFile.strFilename + " with error:" + errstr, nErrorCode);
				MOLOG(Log::L_ERROR, CLOGFMT(VsmFsIO, "[%s] VsWrite() failed with error: %s)"), _strLogHint.c_str(), errstr.c_str());
				
				return false;
			}

			dwLeftBytes -= VSTRM_IO_SIZE;
			totalCopiedBytes += VSTRM_IO_SIZE;
		}

		if (dwLeftBytes)
		{
			memcpy(subFile.cacheBuffer, pBuf+totalCopiedBytes, dwLeftBytes);
			subFile.cacheCurLength = dwLeftBytes;		
		}
	}

	if (!nIndex)
	{
		// only main file need the update the process bytes
		IncProcvBytes(nLen);
	}

	subFile.llProcBytes += nLen;
	return true;
}

bool VstrmFsTarget::writeIndexData(char* pBuf, unsigned int nLen, unsigned int uOffetLow, unsigned int uOffetHigh)
{
//	MOLOG(Log::L_DEBUG, CLOGFMT(VsmFsIO, "[%s] to writeIndexData to file[%s],pBuf[%p],len[%d],offset Low[%d], High[%d]"),
//		_strLogHint.c_str(),_subFiles[1].strFilename.c_str(),pBuf,nLen,uOffetLow, uOffetHigh);

	DWORD amountWritten = 0;
	DWORD dwPtr = SetFilePointer(_ntfsCacheHandle, uOffetLow, (long*)&uOffetHigh, FILE_BEGIN);
	if (dwPtr == INVALID_SET_FILE_POINTER) // Test for failure
	{ 
		// Obtain the error code. 
		std::string strErr;
		getSystemErrorText(strErr);
		MOLOG(Log::L_ERROR, CLOGFMT(VsmFsIO, "[%s] Failed to SetFilePointer to offset Low[%d], High[%d], error: %s"),
			_strLogHint.c_str(), uOffetLow, uOffetHigh, strErr.c_str());
		
		return false;
	}

	if(!WriteFile(_ntfsCacheHandle, pBuf, nLen, &amountWritten, NULL))
	{
		std::string errstr;
		getSystemErrorText(errstr);		
		SetLastError(std::string("failed to write file ") + _subFiles[1].strFilename + " on local with error:" + errstr, ERRCODE_NTFS_WRITEFILE);
		MOLOG(Log::L_ERROR, CLOGFMT(VsmFsIO, "[%s] Failed to create write file: %s with error: %s"),
			_strLogHint.c_str(), _subFiles[1].strFilename.c_str(), errstr.c_str());

		return false;
	}

	return true;
}

bool VstrmFsTarget::reserveVstrmBW(std::string& errmsg)
{
	if(0 == _bwmgrClientId)
	{
		return true;
	}

	for(int i=0;i<_nInputCount;i++)
	{		
		SubFile& subFile = _subFiles[i];

		// reserve VStrm bandwidth
		VSTATUS	statusTicket = ERROR_SUCCESS;
		VSTRM_BANDWIDTH_RESERVE_BLOCK   rbFile = {0};
		PVSTRM_BANDWIDTH_RESERVE_BLOCK	pRbFile=&rbFile;
		
		// The Bw Mgr considers bandwidth requests
		// to be from the perspective of the PCI Bus, not the disks. So, to get data
		// onto the disks they must READ from the PCI Bus, so ask for READ BW here,
		// even tho we are putting data onto the disks using writes. 
		rbFile.ClientId         = _bwmgrClientId;
		rbFile.Type				= kVSTRM_BANDWIDTH_TYPE_READ;
		rbFile.TargetType		= kVSTRM_BANDWIDTH_TARGETTYPE_FILE;
		
		rbFile.BwTarget         = (void*)(subFile.strFilename.c_str()); 
		
		rbFile.MaxBandwidth		= subFile.reservedBW;	// passed in with request
		rbFile.MinBandwidth		= subFile.reservedBW;	// passed in with request
		rbFile.ReservedBandwidth = NULL;
		
		statusTicket = VstrmClassReserveBandwidth(_hVstrm, pRbFile, &subFile.bwTicket);
		
		if (statusTicket != VSTRM_SUCCESS)
		{
			char szBuf[255] = {0};
			VstrmClassGetErrorText(_hVstrm, statusTicket, szBuf, sizeof(szBuf));
			
			MOLOG(Log::L_ERROR, CLOGFMT(VsmFsIO, "[%s] VstrmClassReserveBandwidth(BW - %dbps, ClientId - %d) failed with error %s"), 
				subFile.strFilename.c_str(), subFile.reservedBW, _bwmgrClientId, szBuf);		
			
			errmsg = szBuf;
			
			subFile.bwTicket = 0;
			
			return false;
		}

		MOLOG(Log::L_INFO, CLOGFMT(VsmFsIO, "[%s] VstrmClassReserveBandwidth(BW - %dbps, ClientId - %d) return ticket 0x%I64X"),  
			subFile.strFilename.c_str(), subFile.reservedBW, _bwmgrClientId, subFile.bwTicket);
	}
	
	return true;
}

bool VstrmFsTarget::releaseVstrmBW()
{	
	if(0 == _bwmgrClientId)
		return true;
	
	for(int i=0;i<_nInputCount;i++)
	{
		SubFile& subFile = _subFiles[i];

		if (0 == subFile.reservedBW || 0 == subFile.bwTicket)
			continue;

		VSTATUS	statusTicket = ERROR_SUCCESS;
		
		MOLOG(Log::L_INFO, CLOGFMT(VsmFsIO, "[%s] VstrmClassReleaseBandwidth(BW - %dbps) for ticket: 0x%I64X"), 
			_strLogHint.c_str(), subFile.reservedBW, subFile.bwTicket);
		statusTicket = VstrmClassReleaseBandwidth(_hVstrm, subFile.bwTicket);
		
		//  it always return failure: VSTRM_NOT_SUPPORTED even the bandwidth was released indeed
		if (statusTicket != VSTRM_SUCCESS)
		{
			char szBuf[255] = {0};
			VstrmClassGetErrorText(_hVstrm, statusTicket, szBuf, sizeof(szBuf));
			
			MOLOG(Log::L_WARNING, CLOGFMT(VsmFsIO, "[%s] VstrmClassReleaseBandwidth(%d pbs) for ticket 0x%I64X failed with error %s"), 
				_strLogHint.c_str(), subFile.reservedBW, subFile.bwTicket, szBuf);
			
			subFile.bwTicket = 0;
			
			continue;
		}
		
		subFile.bwTicket = 0;
		MOLOG(Log::L_INFO, CLOGFMT(VsmFsIO, "[%s] VstrmClassReleaseBandwidth(BW - %dbps) succeed"),  _strLogHint.c_str(), subFile.reservedBW);
	}
	
	return true;
}

void VstrmFsTarget::delOutput()
{
	for(int i=0;i<_nInputCount;i++)
	{
		SubFile& subFile = _subFiles[i];

		if(subFile.strFilename.empty())
			continue;
		
		// delete the file
		if(VstrmDeleteFile(_hVstrm, subFile.strFilename.c_str()))
		{
			MOLOG(Log::L_INFO, CLOGFMT(VsmFsIO, "[%s] file %s was successfully deleted"), _strLogHint.c_str(), subFile.strFilename.c_str());
			continue;
		}
		
		std::string strErr;
		unsigned int vstat;
		getVstrmError(_hVstrm, vstat, strErr);
		MOLOG(Log::L_WARNING, CLOGFMT(VsmFsIO, "[%s] failed to delete file %s with error: %s"), _strLogHint.c_str(), subFile.strFilename.c_str(),
			strErr.c_str());
	}
}

bool VstrmFsTarget::copyFileToVstrm(const std::string& sourceFile, HANDLE hVstrm, HANDLE vstrmFileHandle, std::string& errmsg)
{
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
				unsigned int vstat;
				getVstrmError(_hVstrm, vstat, errstr);

				errmsg = "VsWrite() failed with error:" + errstr;
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

	return succeed;
}

bool VstrmFsTarget::copyFileToVstrm(const std::string& sourceFile, const std::string& desFile, std::string& errmsg, bool bDisableBufDrvThrottle, HANDLE hVstrm)
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
				unsigned int vstat;
				getVstrmError(_hVstrm, vstat, errstr);
				
				errmsg = "VsWrite() failed with error:" + errstr;
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

void VstrmFsTarget::setFilename(const char* szFile)
{
	_strFilename = szFile;
}

VstrmFsTarget::~VstrmFsTarget()
{


}

void VstrmFsTarget::setBandwidth(unsigned int uBandwidthBps)
{
	_dwBandwidth = uBandwidthBps;
	MOLOG(Log::L_INFO, CLOGFMT(VsmFsIO, "[%s] set Bandwidth=%d"), _strLogHint.c_str(), uBandwidthBps);
}



void VstrmFsTarget::pacingAppReportOffsets(const void * const pCbParam, const LONGLONG offset1, const LONGLONG offset2)
{
	UNREFERENCED_PARAMETER(pCbParam);
	UNREFERENCED_PARAMETER(offset1);
	UNREFERENCED_PARAMETER(offset2);
}


#define XX(a,b) {a, b}
const char* VstrmFsTarget::DecodePacedIndexError(const unsigned long err)
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

void VstrmFsTarget::pacingAppLogCbk(const char * const pMsg)
{
	char				buf[1024];
	int					len;
	unsigned long		written = 0;
	
	if (_bPacingTrace)
	{
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
		
		//output	
		glog(ZQ::common::Log::L_DEBUG, buf);
	}
}

int VstrmFsTarget::pacingAppWrite(const void * const pCbParam, const int len, const char buf[])
{
	SubFile* subfile = (SubFile*) pCbParam;
	VstrmFsTarget* pThis = subfile->pThis;

	bool ret;
	DWORD amountWritten;
	//
	// pacing index module has make the buffer to 64k, just write it
	//
	if (subfile->bIndexFile)
	{
		//write
		ret = VsWrite(subfile->hOutputFile, len, (char*)buf,  &amountWritten, NULL);
		if (!ret)
		{
			std::string errstr;
			unsigned int vstat;
			getVstrmError(_hVstrm, vstat, errstr);
			int nErrorCode;
			if (vstat == VSTRM_DISK_FULL)
				nErrorCode = ERRCODE_VSTRM_DISK_FULL;
			else if (vstat == VSTRM_NETWORK_NOT_READY)
				nErrorCode = ERRCODE_VSTRM_NOT_READY;
			else if (vstat == VSTRM_NO_MEMORY)
				nErrorCode = ERRCODE_VSTRM_BUFFERQUEUE_FULL;
			else
				nErrorCode = ERRCODE_VSTRM_API_ERROR;				
			
			pThis->SetLastError(std::string("Vstrm failed to write file ") + subfile->strFilename + " with error:" + errstr, nErrorCode);
			SMOLOG(Log::L_ERROR, CLOGFMT(VsmFsIO, "[%s] VsWrite() failed with error: %s)"), pThis->_strLogHint.c_str(), errstr.c_str());
			
			return -1;
		}

#ifdef _DEBUG
		SMOLOG(Log::L_DEBUG, CLOGFMT(VsmFsIO, "[%s] pacingAppWrite() bytes %d"), subfile->strFilename.c_str(), len);
#endif
		return len;
	}

	if(subfile->cacheCurLength + len < VSTRM_IO_SIZE)
	{
		memcpy(&subfile->cacheBuffer[subfile->cacheCurLength], buf, len);
		subfile->cacheCurLength += len;		
	}
	else
	{
		DWORD dwLeftBytes = len;	
		int totalCopiedBytes = 0;
		
		if (subfile->cacheCurLength)
		{
			totalCopiedBytes = VSTRM_IO_SIZE - subfile->cacheCurLength;
			memcpy(&subfile->cacheBuffer[subfile->cacheCurLength], buf, totalCopiedBytes);
			dwLeftBytes -= totalCopiedBytes;
			
			//write
			ret = VsWrite(subfile->hOutputFile, VSTRM_IO_SIZE, subfile->cacheBuffer,  &amountWritten, NULL);
			if (!ret)
			{
				std::string errstr;
				unsigned int vstat;
				getVstrmError(_hVstrm, vstat, errstr);
				int nErrorCode;
				if (vstat == VSTRM_DISK_FULL)
					nErrorCode = ERRCODE_VSTRM_DISK_FULL;
				else if (vstat == VSTRM_NETWORK_NOT_READY)
					nErrorCode = ERRCODE_VSTRM_NOT_READY;
				else if (vstat == VSTRM_NO_MEMORY)
					nErrorCode = ERRCODE_VSTRM_BUFFERQUEUE_FULL;
				else
					nErrorCode = ERRCODE_VSTRM_API_ERROR;				
				
				pThis->SetLastError(std::string("Vstrm failed to write file ") + subfile->strFilename + " with error:" + errstr, nErrorCode);
				SMOLOG(Log::L_ERROR, CLOGFMT(VsmFsIO, "[%s] VsWrite() failed with error: %s)"), pThis->_strLogHint.c_str(), errstr.c_str());
				
				return -1;
			}

			subfile->cacheCurLength = 0;
		}
		
		while (dwLeftBytes>=VSTRM_IO_SIZE)
		{
			//write directly
			ret = VsWrite(subfile->hOutputFile, VSTRM_IO_SIZE, (char*)(buf+totalCopiedBytes), &amountWritten, NULL);
			if (!ret)
			{
				std::string errstr;
				unsigned int vstat;
				getVstrmError(_hVstrm, vstat, errstr);
				int nErrorCode;
				if (vstat == VSTRM_DISK_FULL)
					nErrorCode = ERRCODE_VSTRM_DISK_FULL;
				else if (vstat == VSTRM_NETWORK_NOT_READY)
					nErrorCode = ERRCODE_VSTRM_NOT_READY;
				else if (vstat == VSTRM_NO_MEMORY)
					nErrorCode = ERRCODE_VSTRM_BUFFERQUEUE_FULL;
				else
					nErrorCode = ERRCODE_VSTRM_API_ERROR;				
				
				pThis->SetLastError(std::string("Vstrm failed to write file ") + subfile->strFilename + " with error:" + errstr, nErrorCode);
				SMOLOG(Log::L_ERROR, CLOGFMT(VsmFsIO, "[%s] VsWrite() failed with error: %s)"), pThis->_strLogHint.c_str(), errstr.c_str());
				
				return -1;
			}
			
			dwLeftBytes -= VSTRM_IO_SIZE;
			totalCopiedBytes += VSTRM_IO_SIZE;
		}
		
		if (dwLeftBytes)
		{
			memcpy(subfile->cacheBuffer, buf+totalCopiedBytes, dwLeftBytes);
			subfile->cacheCurLength = dwLeftBytes;		
		}
	}
	
	if (subfile == pThis->_subFiles)
	{
		// only main file need the update the process bytes
		pThis->IncProcvBytes(len);

		if (pThis->_bEnableMD5)
		{
			pThis->_md5ChecksumUtil.checksum(buf, len);
		}
	}

	subfile->llProcBytes += len;
	return len;
}

int VstrmFsTarget::pacingAppSeek(const void * const pCbParam, const LONGLONG offset)
{
	SubFile* subfile = (SubFile*) pCbParam;
	VstrmFsTarget* pThis = subfile->pThis;
	
#ifdef _DEBUG
	SMOLOG(Log::L_DEBUG, CLOGFMT(VsmFsIO, "[%s] pacingAppSeek() offset %lld%"), subfile->strFilename.c_str(), offset);
#endif
	
	LARGE_INTEGER	tmp;
	tmp.QuadPart = offset;
	
	DWORD dwError = 0;
	DWORD dwPtrLow = 0;
	try
	{
		VSTATUS vStat = VsSeek(subfile->hOutputFile, subfile->objectId, &tmp, FILE_BEGIN);
		if (!IS_VSTRM_SUCCESS(vStat))
		{
			std::string errstr;
			getVstrmErrorWithVStatus(_hVstrm, vStat, errstr);
			
			pThis->SetLastError(std::string("Vstrm failed to write file ") + subfile->strFilename + " with error:" + errstr, ERRCODE_VSTRM_API_ERROR);
			SMOLOG(Log::L_ERROR, CLOGFMT(VsmFsIO, "[%s] VsSeek() to offset [%d] failed with error: %s)"), pThis->_strLogHint.c_str(),
				tmp.LowPart, errstr.c_str());
			return -1;
		}
	}
	catch (...) 
	{
		return -1;
	}

	return 0;
}

int VstrmFsTarget::pacingAppSetEOF(const void * const pCbParam, const LONGLONG offset)
{
	SubFile* subfile = (SubFile*) pCbParam;
	VstrmFsTarget* pThis = subfile->pThis;

	LARGE_INTEGER	tmp;
	tmp.QuadPart = offset;		
	
	SMOLOG(Log::L_DEBUG, CLOGFMT(VsmFsIO, "[%s] pacingAppSetEOF() offset %lld%"), subfile->strFilename.c_str(), offset);
	VSTATUS vstat = VsSetEndOfFile(subfile->hOutputFile, subfile->objectId, &tmp);
	if (!IS_VSTRM_SUCCESS(vstat))
	{
		std::string errstr;
		getVstrmErrorWithVStatus(_hVstrm, vstat, errstr);
		
		pThis->SetLastError(std::string("Vstrm failed to write file ") + subfile->strFilename + " with error:" + errstr, ERRCODE_VSTRM_API_ERROR);
		SMOLOG(Log::L_ERROR, CLOGFMT(VsmFsIO, "[%s] VsSetEndOfFile() offset [%d] failed with error: %s)"), pThis->_strLogHint.c_str(),
			tmp.LowPart, errstr.c_str());
		return -1;
	}

	return 0;
}

bool VstrmFsTarget::pacingWriteIndex()
{
	// move file point for reading
#ifdef _DEBUG
	MOLOG(Log::L_DEBUG, CLOGFMT(VsmFsIO, "[%s] Set index reader position: %lld"), _strLogHint.c_str(), _posCacheRead.QuadPart);
#endif	
	if(INVALID_SET_FILE_POINTER == SetFilePointer(_ntfsCacheHandle, _posCacheRead.LowPart, &_posCacheRead.HighPart, FILE_BEGIN))
	{
		return false;
	}
	
	DWORD byteRead = 0;
	if(!ReadFile(_ntfsCacheHandle, _tmpbuffer, VSTRM_IO_SIZE, &byteRead, NULL))
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
 	MOLOG(Log::L_DEBUG, CLOGFMT(VsmFsIO, "[%s] Save index reader position: %lld"), 
 		_strLogHint.c_str(), _posCacheRead.QuadPart);	
#endif
	
	if(0 == byteRead)
		return true;
	
	// log the last index read 
	VVX_V7_RECORD *pTmp = 
		(VVX_V7_RECORD *)(_tmpbuffer + byteRead - sizeof(VVX_V7_RECORD));
	
#ifdef _DEBUG
	MOLOG(Log::L_DEBUG, CLOGFMT(VsmFsIO, "[%s] last index read: %#I64x (%#I64x)(%#I64x), write bytes: %d"),
		_strLogHint.c_str(),
		pTmp->trick.frameByteOffset[0],
		pTmp->trick.frameByteOffset[1],
		pTmp->trick.frameByteOffset[2],
		byteRead);
#endif
	
	// pass the buffer to pacing
	DWORD paceresult = 0;
//	MOLOG(Log::L_DEBUG, CLOGFMT(VsmFsIO, "[%s] PacedIndexWrite() file [%s] len[%d] Buf[%p]"), 
//		_strLogHint.c_str(), _subFiles[1].strFilename.c_str(), byteRead, _tmpbuffer);
//	MOLOG.flush();

	try
	{
		if(PacedIndexWrite(_subFiles[1].pacingIndexCtx, byteRead, _tmpbuffer, &paceresult) < 0)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(VsmFsIO, "[%s] PacedIndexWrite(%s) failed with error"), 
				_strLogHint.c_str(), DecodePacedIndexError(paceresult));
			return false;
		}
	}
	catch(...)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(VsmFsIO, "[%s] PacedIndexWrite(%s) met unknown exception"), 
			_strLogHint.c_str());
		
		return false;
	}

	return true;
}

bool VstrmFsTarget::pacingWriteData(int nIndex, char* pBuf, unsigned int nLen)
{
	SubFile& subFile = _subFiles[nIndex];

	//write
	DWORD paceresult = 0;

//	MOLOG(Log::L_DEBUG, CLOGFMT(VsmFsIO, "[%s] PacedIndexWrite() file [%s] len[%d] Buf[%p]"), 
//		_strLogHint.c_str(), subFile.strFilename.c_str(), nLen, pBuf);

//	MOLOG.flush();

	try
	{
		if(PacedIndexWrite(subFile.pacingIndexCtx, nLen, pBuf, &paceresult) < 0)
		{
			std::string errstr = DecodePacedIndexError(paceresult);
			//			the error information has been set on vswrite failure
			//			SetLastError(std::string("PacedIndexWrite failed to write file ") + subFile.strFilename + " with error:" + errstr);
			MOLOG(Log::L_ERROR, CLOGFMT(VsmFsIO, "[%s] PacedIndexWrite() file %s with error: (%s)"), 
				_strLogHint.c_str(), subFile.strFilename.c_str(), errstr.c_str());
			
			return false;
		}
	}
	catch(...)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(VsmFsIO, "[%s] PacedIndexWrite() file [%s] len[%d] caught unknown exception"), 
			_strLogHint.c_str(), subFile.strFilename.c_str(), nLen);

		SetLastError(std::string("PacedIndexWrite() caught unknown exception"), ERRCODE_PACING_UNKNOWN_EXCEPTION);		
		return false;
	}	

	return true;
}

void VstrmFsTarget::enableMD5(bool bEnable)
{
	_bEnableMD5 = bEnable;
}

void VstrmFsTarget::getMD5(std::string& strMD5)
{
	strMD5 = _md5ChecksumUtil.lastChecksum();
}

void VstrmFsTarget::getSupportFileSize(LONGLONG& supportFilesize)
{	
	for (int i = 2; i < _nInputCount; i++)
	{
		DLL_FIND_DATA_LONG findData = {0};

		VHANDLE fileHandle =VstrmFindFirstFileEx(_hVstrm, (char*)(_subFiles[i].strFilename).c_str(), &findData);
		if(fileHandle != INVALID_HANDLE_VALUE)
		{
			VstrmFindClose(_hVstrm, fileHandle);
			LARGE_INTEGER fileSize = {findData.w.nFileSizeLow, findData.w.nFileSizeHigh};
			supportFilesize += fileSize.QuadPart;
		}
		else
		{
			std::string errstr;
			unsigned int stat;
			getVstrmError(_hVstrm,stat,errstr);

			MOLOG(Log::L_WARNING, CLOGFMT(VsmFsIO, "[%s] Can't get supportFilesize because opening file: %s with error: %s"),
				_strLogHint.c_str(), (_subFiles[i].strFilename).c_str(), errstr.c_str());
			return;
		}	
	}

	MOLOG(Log::L_INFO, CLOGFMT(VsmFsIO, "[%s] supportFilesize-[%lld]"),_strLogHint.c_str(),supportFilesize);
}

}}