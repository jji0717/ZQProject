
#include "Log.h"
#include "mpeg.h"
#include "Guid.h"
#ifdef ABS
#undef ABS
#endif

#include "RTFProc.h"
#include "urlstr.h"
#include "ErrorCode.h"
#include <set>
#include <math.h>

#ifdef ZQ_OS_LINUX
#include "CDNFileSetTarget.h"
#else
#include "FilesetTarget.h"
#endif


#define TrickGeneration			"TrickGeneration"

#pragma comment(lib, "CommonTrickFiles.lib")

using namespace ZQ::common;

#define MOLOG (*_pLog)
#define SMOLOG (*(This->_pLog))
#define SRTFLOG (glog)



namespace ZQTianShan {
	namespace ContentProvision {


#define DEF_MAX_QUEUE_DEPTH     100
// Log* RTFProcess::_rtfLogger = NULL;
bool RTFProcess::_trickKeyFrame = false;

bool RTFProcess::_rtfInited = false;
int RTFProcess::_ctfLibVerMajor = 0;
int RTFProcess::_ctfLibVerMinor = 0;

CTF_WARNING_TOLERANCE RTFProcess::_rtfTolerance = CTF_WARNING_TOLERANCE_RELAXED;

ZQ::common::Mutex RTFProcess::	_lock;
 
static char* VIDEO_CODEC_TYPE_STR[] = {"Invalid", "Mpeg2", "H264", "VC1"};
#define SUB_CONTNT_TYPE_VVX      "VVX"
#define SUB_CONTNT_TYPE_VV2      "VV2"
#define SUB_CONTNT_TYPE_VVC      "VVC"

void ridOfNewLineChar(const char* pstring, std::string& outstr)
{
	const char* p = pstring;
	while(*p && *p!=0x0a)
		p++;
	int nLen = p - pstring;

	outstr.assign(pstring,nLen);
}

RTFProcess::RTFProcess()
{
	//
	// base filter property
	//	
	_nOutputCount = 4;		//default output count
	_nInputCount = 1;
	
	_bDriverModule = true;	
	
	//
 	// set basic trick configuration
 	//
 	_sessionHandle = NULL;

	_bFirstReleaseSrc = true;
 
#ifdef ZQ_OS_MSWIN
	_bfrontToBackFR = false;
	_indexType = CTF_INDEX_TYPE_VVX;
	_indexOption.vvx = (true == _trickKeyFrame) ? CTF_INDEX_OPTION_VVX_7_3 : CTF_INDEX_OPTION_VVX_7_2; 
#else
	_bfrontToBackFR = true;
	_indexType = CTF_INDEX_TYPE_VVC;
#endif
	// default video type 
	_vcdType = CTF_VIDEO_CODEC_TYPE_MPEG2;

	// set index mode as REAL TIME as default
	_indexMode = CTF_INDEX_MODE_NOSEEK;
	

#ifndef ZQ_OS_MSWIN
	//now the linux does not support PWE, so use offline mode
	_indexMode = CTF_INDEX_MODE_OFFLINE;
#endif
 	
	// set output file count
	_unifiedTrickFile = true;
	_eosSubFileCount = 0;
	_newTrickSpeedNumerator[0] = 15; 
	memset(_newTrickSpeedNumeratorHD, 0, sizeof(_newTrickSpeedNumeratorHD));
	_bAudioOnly = false;

	_augmentationPidCount = 0;
	_augmentedBitRate = 0;
	_originalBitRate = 0;
	memset((void *)_augmentationPid, 0 , sizeof(_augmentationPid) );
	_bPreEncryptFile  = false;

	_retryCount = 0;
	_alreadyRetryCount = 0;
	_bRetry = false;
	_outputFileCount = 0;

	_bTestForCsico = false;
	_mainFileExt = "";

	_lingerAtFailed = 32; // linger for 32 buffers
}

RTFProcess::~RTFProcess(void)
{
	Close();
}

void RTFProcess::InitPins()
{
	// set output file count
	_eosSubFileCount = 0;

	//
	// Initialize output file information
	//
	initOutputFileInfo();

	_nOutputCount = _outputFileCount;
	int i;
	_inputPin.clear();
	_outputPin.clear();
	for(i=0;i<_nInputCount;i++)
	{
		InputPin pin;
		pin.nPrevPin = 0;
		pin.pPrevFilter = 0;		
		_inputPin.push_back(pin);
	}
	for(i=0;i<_nOutputCount;i++)
	{
		OutputPin pin;
		pin.nNextPin = 0;
		pin.pNextFilter = 0;		
		_outputPin.push_back(pin);
	}
}

bool RTFProcess::Init()
{
	_bFirstReleaseSrc = true;
	_eosSubFileCount = 0;
	_sessClosed = false;

	return true;
}

bool RTFProcess::Start()
{
	if(!openSession())
	{
		MOLOG(Log::L_ERROR, CLOGFMT(TrickGeneration, "[%s] openSession() failed for vcdtype: %s"), _strLogHint.c_str(), VIDEO_CODEC_TYPE_STR[_vcdType]);
		return false;
	}	

	_eosSubFileCount = 0;
	_sessClosed = false;

	return true;
}

bool RTFProcess::Run()
{
	MOLOG(Log::L_INFO, CLOGFMT(TrickGeneration, "[%s] Run() enter"), _strLogHint.c_str());
	
	bool bRet = true;
	_alreadyRetryCount = 0;

	do
	{
		if( _augmentationPidCount > 0 )
		{
			if( augmentationScan() != 0 )
			{
				_bPreEncryptFile = false;
				//				break;
			}
			else
			{
				if(_originalBitRate != _augmentedBitRate)
					_bPreEncryptFile = true;
				else
					_bPreEncryptFile = false;
			}
		}

		InputPin&  pin = _inputPin[0];		
		while(!_bStop)
		{
			// step 1. read from the input
			MediaSample* pSample = pin.pPrevFilter->GetData(pin.nPrevPin);
			if (!pSample)
			{
				MOLOG(Log::L_INFO, CLOGFMT(TrickGeneration, "[%s] GetData return 0, stop the process"), _strLogHint.c_str());
				bRet = !GetGraph()->IsErrorOccurred();				
				break;
			}

			pSample->addRef();

			// step 2. pass the data to the output main file first
			if (!_bFailed && _lingerAtFailed >0)
			{
				OutputPin& pin = _outputPin[0];	
				if (!pin.pNextFilter->Receive(pSample, pin.nNextPin))
				{
					MOLOG(Log::L_ERROR, CLOGFMT(TrickGeneration, "[%s] Next filter[%s] failed to Receive data, stop process"), _strLogHint.c_str(), pin.pNextFilter->GetName());

					pSample->relRef();
					//
					//release the sample
					GetGraph()->freeMediaSample(pSample);

					bRet = false;
					_bStop = true;		
					break;
				}
			}

			if (_bFailed)
			{
				if (_lingerAtFailed-- >0)
					continue;
				else
					break;
			}

			// step 3. fill the data into trick-generation lib, the latter will output to callback trickFilePutOutputBuffer()
			void* buf = pSample->getPointer();
			int nLen  = pSample->getDataLength();

			if (ctfProcessInput(_sessionHandle, 
				this, NULL, pSample, 
				(unsigned char*)buf, (unsigned long)nLen))
			{
				// succeed case
				continue;
			}

			// step 4. handling of ctfProcessInput() failure
			if (!_bRetry || _alreadyRetryCount >= _retryCount) // _bRetry would be reset to true by a succ ctf callback
			{
				MOLOG(Log::L_ERROR, CLOGFMT(TrickGeneration, "[%s] ctfProcessInput() failured r[%d], stop the process"), _strLogHint.c_str(), _retryCount);
				SetLastError("ctfProcessInput() return failure", ERRCODE_RTFLIB_PROCESSBUFF_FAIL);
				bRet = false;
				break;
			}

			//	MOLOG(Log::L_INFO, CLOGFMT(TrickGeneration, "[%s] retry [%d]"), _strLogHint.c_str(), _alreadyRetryCount + 1);
			closeSession();

			if (!openSession())
			{
				MOLOG(Log::L_ERROR, CLOGFMT(TrickGeneration, "[%s] openSession() failed for vcdtype: %s"), _strLogHint.c_str(), VIDEO_CODEC_TYPE_STR[_vcdType]);
				bRet = false;
				break;
			}

			bRet = true;
			_alreadyRetryCount++;
			_eosSubFileCount = 0;
			_sessClosed = false;

			_bFirstReleaseSrc = true;
			_bRetry = false;  // _bRetry would be reset to true by a succ ctf callback
			//	i++;
			continue;

		} // while (!_bStop)

		if (bRet)
		{
			bRet = !GetGraph()->IsErrorOccurred();
		}

		closeSession();

	} while(0);
	
	if (bRet && _bFailed)
		bRet = false; 

	std::set<BaseFilter*> nextFilters;
	for (int i=0; i<_nOutputCount; i++)
	{
		nextFilters.insert(_outputPin[i].pNextFilter);
	}
	
	std::set<BaseFilter*>::iterator it=nextFilters.begin();
	while(it!=nextFilters.end())	
	{
		(*it)->endOfStream();
		it++;
	}

	//再次确保没有错误发生，bug22569
	if (bRet)
		bRet = !GetGraph()->IsErrorOccurred();
	
	MOLOG(Log::L_INFO, CLOGFMT(TrickGeneration, "[%s] Run() left"), _strLogHint.c_str());
	return bRet;
}

void RTFProcess::Close()
{
	closeSession();
}

double RTFProcess::getFrameRateByCode(uint16 framecode)
{
	switch(framecode)
	{
	case VMPEG_FRAME_RATE_23_976:
		return (double) 23.976;

	case VMPEG_FRAME_RATE_24:
		return (double) 24;

	case VMPEG_FRAME_RATE_25:
		return (double) 25;

	case VMPEG_FRAME_RATE_29_97:
		return (double) 29.97;

	case VMPEG_FRAME_RATE_30:
		return (double) 30;

	case VMPEG_FRAME_RATE_50:
		return (double) 50;
		
	case VMPEG_FRAME_RATE_59_94:
		return (double) 59.94;

	case VMPEG_FRAME_RATE_60:
		return (double) 60;

	default:
		return (double) 0;
	}
	return 0;
}

void RTFProcess::appLog(void *hAppSession, const char *pShortString, char *pLongString)
{
	std::string longStr;

	if(!pShortString)
		pShortString = "";

	if (!pLongString)
	{
		pLongString = "";
	}
    
    ridOfNewLineChar(pLongString,longStr);

	RTFProcess* This = (RTFProcess*)hAppSession;
	if (This)
	{
		SMOLOG(Log::L_DEBUG, CLOGFMT(TrickGeneration, "[%s] Library Log: [%s][%s]"), 
			This->_strLogHint.c_str(), pShortString, longStr.c_str());
	}
	else
	{
		SRTFLOG(Log::L_DEBUG, CLOGFMT(TrickGeneration, "Library Log: [%s][%s]"), 
			pShortString, longStr.c_str());
	}
}

void RTFProcess::rtfLibErrorLog(char *pMessage)
{
	if(pMessage)
	{
		std::string longStr;
		ridOfNewLineChar(pMessage,longStr);

		SRTFLOG(Log::L_ERROR, CLOGFMT(TrickGeneration, "Library Log: [Library Error][%s]"), longStr.c_str());
	}
}

void* RTFProcess::appAlloc(void* hAppHeap, int32_t bytes)
{
	return malloc( bytes );
}

void RTFProcess::appFree(void* hAppHeap, void *ptr)
{
	free( ptr );
}

void RTFProcess::sessionErrorNotifier(void* hAppSession, char *pMessage)
{
	std::string longStr;
	if (!pMessage)
		return;

	ridOfNewLineChar(pMessage, longStr);

	RTFProcess* This = (RTFProcess*)hAppSession;
	if(This)
	{
		SMOLOG(Log::L_ERROR, CLOGFMT(TrickGeneration, "[%s] Library Log: [Session Error][%s]"), 
			This->_strLogHint.c_str(), longStr.c_str());
		int nErrorCode;
			nErrorCode = ERRCODE_RTFLIB_ERROR;

		if(!This->GetGraph()->IsErrorOccurred())
			This->SetLastError(longStr, nErrorCode);
	}
	else
	{
		SRTFLOG(Log::L_ERROR, CLOGFMT(TrickGeneration, "Library Log: [Session Error][%s]"), longStr.c_str());
	}
}

void RTFProcess::getUnifiedTrickExt(int speedNo, char* ext)
{
	if(0 == speedNo)
	{
		sprintf(ext, ".FFR");
	}
	else
	{
		sprintf(ext, ".FFR%d", speedNo);
	}		
}

void RTFProcess::getTrickExt(int speedNo, char* ext1, char* ext2)
{
	if(0 == speedNo)
	{
		sprintf(ext1, ".FF");
		sprintf(ext2, ".FR");
	}
	else
	{
		sprintf(ext1, ".FF%d", speedNo);
		sprintf(ext2, ".FR%d", speedNo);
	}		
}

void RTFProcess::setTrickGenParam(CTF_INDEX_TYPE indexType, CTF_VIDEO_CODEC_TYPE codecType)
{	
	// index Type
	switch(indexType)
	{
	case CTF_INDEX_TYPE_VV2:
		_indexType = CTF_INDEX_TYPE_VV2;
		_indexOption.vv2 = CTF_INDEX_OPTION_VV2_TSOIP;
		_indexOption.vvx = CTF_INDEX_OPTION_VVX_7_2;
//		_unifiedTrickFile = true;
		_indexMode = CTF_INDEX_MODE_OFFLINE;
		break;
#ifndef ZQ_OS_MSWIN
	case CTF_INDEX_TYPE_VVC:
		_indexType = CTF_INDEX_TYPE_VVC;
		_bfrontToBackFR = TRUE;
		_unifiedTrickFile = false;
		_indexMode = CTF_INDEX_MODE_OFFLINE;
		break;
#endif
	case CTF_INDEX_TYPE_VVX:
	default:
#ifdef ZQ_OS_MSWIN
		_indexType = CTF_INDEX_TYPE_VVX;
		_indexOption.vvx = (true == _trickKeyFrame) ? CTF_INDEX_OPTION_VVX_7_3 : CTF_INDEX_OPTION_VVX_7_2; 
#else
		_bfrontToBackFR = TRUE;
		_indexType = CTF_INDEX_TYPE_VVC;
		_indexMode = CTF_INDEX_MODE_OFFLINE;
#endif
		_unifiedTrickFile = false;
		break;
	}

	// set codec type
	switch(codecType)
	{
	case CTF_VIDEO_CODEC_TYPE_H264:
		_vcdType = CTF_VIDEO_CODEC_TYPE_H264;
		break;
	case CTF_VIDEO_CODEC_TYPE_H265:
		_vcdType = CTF_VIDEO_CODEC_TYPE_H265;
		break;
	case CTF_VIDEO_CODEC_TYPE_MPEG2:
	default:
		_vcdType = CTF_VIDEO_CODEC_TYPE_MPEG2;
		break;
	}
}

void RTFProcess::initOutputFileInfo()
{
	int outputIndex = 0;
	//
	// Attention: only set output file info for the trick file and index, without main file
	//            to the main file, if do this way, the output main file size by RTFLib is smaller than original one
	//            so the main file is output to file in releaseInputBuffer

	// main file
	_outputInfo[outputIndex].fileNo = outputIndex;
	_outputInfo[outputIndex].fileType = OPTFT_MAIN;
	_outputInfo[outputIndex].speedNo = 0;
#ifndef ZQ_OS_MSWIN
	if(CTF_INDEX_TYPE_VVC == _indexType) {
 		if(!_bTestForCsico)
			strcpy(_outputInfo[outputIndex].extension, ".0X0000");
		else
			strcpy(_outputInfo[outputIndex].extension, _mainFileExt.c_str());
	}
#else
	_outputInfo[outputIndex].extension[0]='\0';
#endif
	_outputInfo[outputIndex].filesize = 0;
	_outputInfo[outputIndex].fileOffset = 0;

	_outputInfo[outputIndex].speedDirection = 1;
	_outputInfo[outputIndex].speedNumerator = 1;
	_outputInfo[outputIndex].speedDenominator = 1;
	_outputInfo[outputIndex].speedNumeratorHD = 1;
	_outputInfo[outputIndex].speedDenominatorHD = 1;
	_outputInfo[outputIndex].begOffset = 0;
	_outputInfo[outputIndex].endOffset = 0;

	outputIndex++;


	// index file
	if(CTF_INDEX_TYPE_VV2 == _indexType)
	{
		_outputInfo[outputIndex].fileNo = outputIndex;
		_outputInfo[outputIndex].fileType = OPTFT_VV2;
		_outputInfo[outputIndex].speedNo = 0;
		strcpy(_outputInfo[outputIndex].extension, ".VV2");
		_outputInfo[outputIndex].filesize = 0;
		_outputInfo[outputIndex].fileOffset = 0;

		_outputInfo[outputIndex].speedDirection = 0;
		_outputInfo[outputIndex].speedNumerator = 0;
		_outputInfo[outputIndex].speedDenominator = 1;
		_outputInfo[outputIndex].speedNumeratorHD = 0;
		_outputInfo[outputIndex].speedDenominatorHD = 1;
		_outputInfo[outputIndex].begOffset = 0;
		_outputInfo[outputIndex].endOffset = 0;
	}
#ifndef ZQ_OS_MSWIN
	else if (CTF_INDEX_TYPE_VVC == _indexType)
	{
		_outputInfo[outputIndex].fileNo = outputIndex;
		_outputInfo[outputIndex].fileType = OPTFT_VVC;
		_outputInfo[outputIndex].speedNo = 0;
		strcpy(_outputInfo[outputIndex].extension, ".index");
		_outputInfo[outputIndex].filesize = 0;
		_outputInfo[outputIndex].fileOffset = 0;

		_outputInfo[outputIndex].speedDirection = 0;
		_outputInfo[outputIndex].speedNumerator = 0;
		_outputInfo[outputIndex].speedDenominator = 1;
		_outputInfo[outputIndex].speedNumeratorHD = 0;
		_outputInfo[outputIndex].speedDenominatorHD = 1;
		_outputInfo[outputIndex].begOffset = 0;
		_outputInfo[outputIndex].endOffset = 0;
        _unifiedTrickFile = false;
	}
#endif	
	else
	{
		_outputInfo[outputIndex].fileNo = outputIndex;
		_outputInfo[outputIndex].fileType = OPTFT_VVX;
		_outputInfo[outputIndex].speedNo = 0;
		strcpy(_outputInfo[outputIndex].extension, ".VVX");
		_outputInfo[outputIndex].filesize = 0;
		_outputInfo[outputIndex].fileOffset = 0;

		_outputInfo[outputIndex].speedDirection = 0;
		_outputInfo[outputIndex].speedNumerator = 0;
		_outputInfo[outputIndex].speedDenominator = 1;
		_outputInfo[outputIndex].speedNumeratorHD = 0;
		_outputInfo[outputIndex].speedDenominatorHD = 1;
		_outputInfo[outputIndex].begOffset = 0;
		_outputInfo[outputIndex].endOffset = 0;
		_unifiedTrickFile = false;
	}
	outputIndex++;
	
	// set the trick file output setting
	FileExtensions::iterator iter;
	int i= 0;
	for (iter = _trickSpeedAndFileExt.begin(); iter != _trickSpeedAndFileExt.end(); iter++)
	{
		std::string strExt = iter->second.ext;
		std::string strCiscoFileExt = iter->second.extForCisco;
		int   position = iter->second.position;

		if(_unifiedTrickFile)
		{
			char ext[16];
			getUnifiedTrickExt(i++, ext);

			// set FFR
			_outputInfo[outputIndex].fileNo = outputIndex;
			if (stricmp((*iter).second.ext.c_str(),".ffr1") == 0)
				_outputInfo[outputIndex].fileType = OPTFT_FFR1;
			else if (stricmp((*iter).second.ext.c_str(),".ffr2") == 0)
				_outputInfo[outputIndex].fileType = OPTFT_FFR2;
			else if (stricmp((*iter).second.ext.c_str(),".ffr3"))
				_outputInfo[outputIndex].fileType = OPTFT_FFR3;
			else
				_outputInfo[outputIndex].fileType = OPTFT_FFR;
			_outputInfo[outputIndex].speedNo = (*iter).second.position;

#ifndef ZQ_OS_MSWIN
			if(!_bTestForCsico || CTF_INDEX_TYPE_VVC != _indexType)
				strcpy(_outputInfo[outputIndex].extension, ext);
			else
			{
				strcpy(_outputInfo[outputIndex].extension, strCiscoFileExt.c_str());
			}
#else
			strcpy(_outputInfo[outputIndex].extension, ext);
#endif			
			_outputInfo[outputIndex].filesize = 0;
			_outputInfo[outputIndex].fileOffset = 0;
			_outputInfo[outputIndex].begOffset = 0;
			_outputInfo[outputIndex].endOffset = 0;

			_outputInfo[outputIndex].speedDirection = 0;
			_outputInfo[outputIndex].speedNumerator = _newTrickSpeedNumerator[position];
			_outputInfo[outputIndex].speedDenominator = 2;
			_outputInfo[outputIndex].speedNumeratorHD = _newTrickSpeedNumeratorHD[position];
			_outputInfo[outputIndex].speedDenominatorHD = 2;

			_indexExMap.insert(std::make_pair(_outputInfo[outputIndex].fileType,outputIndex));
			outputIndex++;
		}
		else
		{
			char extFF[16];
			char extFR[16];

			if (i ==(int)_trickSpeedAndFileExt.size()/2)
				i = 0;
			getTrickExt(i, extFF, extFR);
		
			// set FF
			_outputInfo[outputIndex].fileNo = outputIndex;

			if (strstr((*iter).second.ext.c_str(),"FF") == NULL)
			{
				if (stricmp((*iter).second.ext.c_str(),".fr1") == 0)
					_outputInfo[outputIndex].fileType = OPTFT_FR1;
				else if (stricmp((*iter).second.ext.c_str(),".fr2") == 0)
					_outputInfo[outputIndex].fileType = OPTFT_FR2;
				else if (stricmp((*iter).second.ext.c_str(),".fr3") == 0)
					_outputInfo[outputIndex].fileType = OPTFT_FR3;
				else
					_outputInfo[outputIndex].fileType = OPTFT_FR;
#ifndef ZQ_OS_MSWIN
				if(!_bTestForCsico || CTF_INDEX_TYPE_VVC != _indexType)
					strcpy(_outputInfo[outputIndex].extension, extFR);
				else
				{
					strcpy(_outputInfo[outputIndex].extension, strCiscoFileExt.c_str());
				}
#else
				strcpy(_outputInfo[outputIndex].extension, extFR);
#endif
				_outputInfo[outputIndex].speedDirection = -1;

				_indexExMap.insert(std::make_pair(_outputInfo[outputIndex].fileType,outputIndex));
			}
			else
			{
				if (stricmp((*iter).second.ext.c_str(),".ff1") == 0)
					_outputInfo[outputIndex].fileType = OPTFT_FF1;
				else if (stricmp((*iter).second.ext.c_str(),".ff2") == 0)
					_outputInfo[outputIndex].fileType = OPTFT_FF2;
				else if (stricmp((*iter).second.ext.c_str(),".ff3") == 0)
					_outputInfo[outputIndex].fileType = OPTFT_FF3;
				else
					_outputInfo[outputIndex].fileType = OPTFT_FF;
#ifndef ZQ_OS_MSWIN
				if(!_bTestForCsico || CTF_INDEX_TYPE_VVC != _indexType)
					strcpy(_outputInfo[outputIndex].extension, extFF);
				else
				{
					strcpy(_outputInfo[outputIndex].extension, strCiscoFileExt.c_str());
				}
#else
				strcpy(_outputInfo[outputIndex].extension, extFF);
#endif
				_outputInfo[outputIndex].speedDirection = 1;

				_indexExMap.insert(std::make_pair(_outputInfo[outputIndex].fileType,outputIndex));
			}	
			_outputInfo[outputIndex].speedNo =(*iter).second.position;
			_outputInfo[outputIndex].speedNumerator = _newTrickSpeedNumerator[position];
			_outputInfo[outputIndex].speedDenominator = 2;
			_outputInfo[outputIndex].speedNumeratorHD = _newTrickSpeedNumeratorHD[position];
			_outputInfo[outputIndex].speedDenominatorHD = 2;
			_outputInfo[outputIndex].filesize = 0;
			_outputInfo[outputIndex].fileOffset = 0;
			_outputInfo[outputIndex].begOffset = 0;
			_outputInfo[outputIndex].endOffset = 0;

			outputIndex++;
			i++;
			//// set FR
			//_outputInfo[outputIndex].fileNo = outputIndex;
			//if ((*iter).second == 1)
			//	_outputInfo[outputIndex].fileType = OPTFT_FR1;
			//else if ((*iter).second == 2)
			//	_outputInfo[outputIndex].fileType = OPTFT_FR2;
			//else if ((*iter).second == 3)
			//	_outputInfo[outputIndex].fileType = OPTFT_FR3;
			//else
			//	_outputInfo[outputIndex].fileType = OPTFT_FR;
			//_outputInfo[outputIndex].speedNo = (*iter).second;
			//strcpy(_outputInfo[outputIndex].extension, extFR);
			//_outputInfo[outputIndex].filesize = 0;
			//_outputInfo[outputIndex].fileOffset = 0;

			//_outputInfo[outputIndex].speedDirection = -1;
			//_outputInfo[outputIndex].speedNumerator = trickSpeedNumerator[(*iter).second];
			//_outputInfo[outputIndex].speedDenominator = trickSpeedDenominator[(*iter).second];

			//outputIndex++;
			//iter++;
		}
	}
	// set total file count
	_outputFileCount = outputIndex;	
}

//
//
//
#ifdef ZQ_OS_MSWIN
bool RTFProcess::isRTFLibLoaded()
{
	char szTmp[128];
	sprintf(szTmp, "CPE_CTFLib_PID_%08x", GetCurrentProcessId());
	HANDLE hHandle = OpenEvent(EVENT_ALL_ACCESS, NULL, szTmp);
	if (hHandle==INVALID_HANDLE_VALUE||hHandle==NULL)
	{
		return false;
	}
	if (WaitForSingleObject(hHandle, 0) != WAIT_OBJECT_0)
	{
		CloseHandle(hHandle);
		return false;
	}
	CloseHandle(hHandle);
	return true;
}

//
//
//
void RTFProcess::setRTFLibLoaded(bool bLoaded)
{
	char szTmp[128];
	sprintf(szTmp, "CPE_CTFLib_PID_%08x", GetCurrentProcessId());
	HANDLE hHandle = OpenEvent(EVENT_ALL_ACCESS, NULL, szTmp);
	if (hHandle==INVALID_HANDLE_VALUE||hHandle==NULL)
	{
		if (bLoaded)
		{
			hHandle = CreateEvent(NULL, TRUE,TRUE, szTmp);
			if (hHandle == INVALID_HANDLE_VALUE||hHandle==NULL)
			{
				printf("failed to create event[%s]\n", szTmp);
			}
		}
	}
	else
	{
		if (!bLoaded)
		{
			ResetEvent(hHandle);
		}
		else
		{
			SetEvent(hHandle);
		}
		CloseHandle(hHandle);
	}
}
#else

bool RTFProcess::isRTFLibLoaded()
{
	char szTmp[128] = {0};
	sprintf(szTmp, "CPE_CTFLib_PID_%08x", getpid());
	sem_t* psem = sem_open(szTmp, O_EXCL);
	if (psem == SEM_FAILED)
	{
		return false;
	}
	
	sem_close(psem);
	return true;
}

void RTFProcess::setRTFLibLoaded(bool bLoaded)
{
	char szTmp[128] = {0};
	sprintf(szTmp, "CPE_CTFLib_PID_%08x", getpid());
	if(!bLoaded)
	{
		sem_unlink(szTmp);
		return;
	}

	sem_t* psem = sem_open(szTmp, O_CREAT, 0774, 0);
	if (psem == SEM_FAILED)
		printf("failed to sem_open semaphore [%s] code [%d] string [%s]\n",szTmp,errno,strerror(errno));
	else
		sem_close(psem);
}
#endif


bool RTFProcess::initRTFLib(uint32 maxSession, Log* pLog, uint32 inputBufferSize, uint32 maxInputBuffersPerSession, uint32 sessionFailThreshold, bool trickKeyFrame, bool rtfWarningTolerance)
{
	unsigned long poolBytes;

	ZQ::common::setGlogger(pLog);

	int32_t pMajor,  pMinor,  pPatch,  pBuild, pReleaseCandidate,  pBuildType;
	ctfGetVersion(&pMajor, &pMinor,  &pPatch,  &pBuild, &pReleaseCandidate,  &pBuildType);
	_ctfLibVerMajor = pMajor;
	_ctfLibVerMinor = pMinor;  

	if (isRTFLibLoaded())
	{
		SRTFLOG(Log::L_INFO, CLOGFMT(TrickGeneration, "initTrickLib() - already loaded in current process, skip initialize here"));
		return true;
	}

	_trickKeyFrame = trickKeyFrame;

	if(rtfWarningTolerance)
		_rtfTolerance = CTF_WARNING_TOLERANCE_RELAXED;
	else
		_rtfTolerance = CTF_WARNING_TOLERANCE_STRICT;

	{
		char* rtfVersion = NULL;
		ctfGetVersionString(&rtfVersion);

		SRTFLOG(Log::L_INFO, CLOGFMT(TrickGeneration, "initTrickLib() - Initializing trickFile library, version %s"), rtfVersion);
	}

	// 
	poolBytes = ctfGetLibraryStorageRequirement(maxSession,
		maxInputBuffersPerSession, 
		MAX_GROUPS_PER_SEQUENCE,
		MAX_PICTURES_PER_GROUP,
		inputBufferSize);
	SRTFLOG(Log::L_INFO, CLOGFMT(TrickGeneration, "initTrickLib() - ctfGetLibraryStorageRequirement() returned %d bytes"), poolBytes);

	SRTFLOG(Log::L_INFO, CLOGFMT(TrickGeneration, "to ctfInitializeLibrary() - maxSession=%d,maxInputBufferBytes=%d,maxInputBuffersPerSession=%d,sessionFailThreshold=%d"),
		maxSession, inputBufferSize, maxInputBuffersPerSession, sessionFailThreshold);	
	if(!ctfInitializeLibrary(NULL,
		appAlloc, 
		appFree, 
		appLog,
		maxSession, 
		maxInputBuffersPerSession,
		MAX_GROUPS_PER_SEQUENCE, 
		MAX_PICTURES_PER_GROUP,
		inputBufferSize, 
		sessionFailThreshold,
		rtfLibErrorLog) ) 
	{
		SRTFLOG(Log::L_ERROR, CLOGFMT(TrickGeneration, "initTrickLib() - ctfInitializeLibrary() failed"));		
		return false;
	}

	SRTFLOG(Log::L_INFO, CLOGFMT(TrickGeneration, "initTrickLib() - TrickFile library initialization completed"));
	setRTFLibLoaded(true);
	_rtfInited = true;
	return true;
}

void RTFProcess::uninitRTFLib()
{
	if(!_rtfInited)
		return;

	if (!isRTFLibLoaded())
	{
		SRTFLOG(Log::L_INFO, CLOGFMT(TrickGeneration, "uninitTrickLib() - already shut down"));
		return;
	}

	SRTFLOG(Log::L_INFO, CLOGFMT(TrickGeneration, "uninitTrickLib() - shutting down trick library"));
	try
	{
		if(!ctfShutdownLibrary())
		{
			SRTFLOG(Log::L_INFO, CLOGFMT(TrickGeneration, "uninitTrickLib() - ctfShutdownLibrary() failed"));
		}
	}
	catch(...)
	{
		SRTFLOG(Log::L_INFO, CLOGFMT(TrickGeneration, "uninitTrickLib() - ctfShutdownLibrary() failed with unknown exception"));
	}

	setRTFLibLoaded(false);
	SRTFLOG(Log::L_INFO, CLOGFMT(TrickGeneration, "uninitTrickLib() - Trick library was shut down"));	
}

bool RTFProcess::openSession()
{
	if (_sessionHandle)
		return true;

	bool bRet = false;
	int settingIndex = 0;

      std::string provdExt = "";
#ifndef ZQ_OS_MSWIN
	if (_indexType == CTF_INDEX_TYPE_VVC)
		if(!_bTestForCsico)
		  provdExt = _strprovId + std::string(".0X0000");
		else
          provdExt = _strprovId + _mainFileExt;
#endif
        
        Guard<Mutex> op(_lock);
		bRet = ctfOpenSession(&_sessionHandle, 
			this, 
			NULL,
			(char*)provdExt.c_str(),
			 0,
			_vcdType,
			_indexMode,
			_indexType,
			_indexOption,
			_rtfTolerance,
			sessionErrorNotifier,
			releaseInputBuffer);

		if(!bRet)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(TrickGeneration, "[%s] ctfOpenSession() failed"), _strLogHint.c_str());
			SetLastError("ctfOpenSession failed", ERRCODE_RTFLIB_OPENSESS_EXCEPTION);
			return false;
		}

	// set the trick file output setting
	// Attention: The index of i from 1, does not include main file, since RTFLib output main file
	//            size may be NOT same as source content. 
	//            The source content buffer comes from receive() 
	for(int i=1; i<_outputFileCount; i++)
	{
//		OutputFileType fileType = _outputInfo[i].fileType;

		try
		{
			BOOL nRet = false;
			if(_outputInfo[i].speedNumeratorHD !=0  && _outputInfo[i].speedDenominatorHD != 0)
			{
				nRet = ctfAddOutputFileEx(_sessionHandle,
					(void *)&_outputInfo[i],
					_outputInfo[i].speedDirection, 
					_outputInfo[i].speedNumerator, 
					_outputInfo[i].speedDenominator, 
					_outputInfo[i].speedNumeratorHD, 
					_outputInfo[i].speedDenominatorHD,
					_outputInfo[i].extension,
					trickFileGetOutputBuffer,
					trickFilePutOutputBuffer,
					trickFileCloseOutput);
			}
			else
			{
				nRet = ctfAddOutputFile(_sessionHandle,
				(void *)&_outputInfo[i],
				_outputInfo[i].speedDirection, 
				_outputInfo[i].speedNumerator, 
				_outputInfo[i].speedDenominator, 
				_outputInfo[i].extension,
				trickFileGetOutputBuffer,
				trickFilePutOutputBuffer,
				trickFileCloseOutput);
			}
            
			if(!nRet)
			{
				MOLOG(Log::L_ERROR, CLOGFMT(TrickGeneration, "[%s] ctfAddOutputFile() failed for index %d"), 
					_strLogHint.c_str(), i);
				SetLastError("ctfAddOutputFile() failed", ERRCODE_RTFLIB_OPENSESS_EXCEPTION);
				return false;
			}
		}
		catch(...)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(TrickGeneration, "[%s] ctfInitializeOutputSettings() failed for index %d with unknown exception"), 
				_strLogHint.c_str(), i);
			SetLastError("ctfInitializeOutputSettings() failed", ERRCODE_RTFLIB_OPENSESS_EXCEPTION);
			return false;
		}	

		MOLOG(Log::L_DEBUG, CLOGFMT(TrickGeneration, "[%s] ctfInitializeOutputSettings() successful for index %d"),
			_strLogHint.c_str(), i);

		settingIndex++;
	}

//	rtfSetOutputSetting( _sessionHandle, "timeStampCorrection", "0" );    // formerly 'restampPTSDTS'
    setLegacyAudioOnly();
setProviderId();
#ifndef ZQ_OS_MSWIN
	ctfSetOutputSetting(_sessionHandle, "frontToBackFR", _bfrontToBackFR ? "1" : "0" );
#endif

	if( ctfStartSession(_sessionHandle) != TRUE )
	{
		MOLOG(Log::L_ERROR, CLOGFMT(TrickGeneration, "[%s] ctfStartSession failed"), _strLogHint.c_str());
		return false;
	}

	MOLOG(Log::L_INFO, CLOGFMT(TrickGeneration, "[%s] ctfOpenSession() succeed with returned session handle 0x%08X"), _strLogHint.c_str(), _sessionHandle);
	return true;
}

bool RTFProcess::closeSession()
{
	bool bRet = true;
	if(_sessionHandle) 
	{
		if (!_sessClosed)
		{
			MOLOG(Log::L_DEBUG, CLOGFMT(TrickGeneration, "[%s] to ctfCloseSession()"), _strLogHint.c_str());
			try
			{
				Guard<Mutex> op(_lock);
				bRet = ctfCloseSession(_sessionHandle);			
				if(bRet)
				{
					MOLOG(Log::L_INFO, CLOGFMT(TrickGeneration, "[%s] ctfCloseSession() succeed"), _strLogHint.c_str());
				}
				else
				{
					MOLOG(Log::L_WARNING, CLOGFMT(TrickGeneration, "[%s] ctfCloseSession() failed"), _strLogHint.c_str());
				}
			}
			catch(...)
			{
				MOLOG(Log::L_WARNING, CLOGFMT(TrickGeneration, "[%s] ctfCloseSession() met unknown exception"), _strLogHint.c_str());
			}
		}
		else
		{
			MOLOG(Log::L_INFO, CLOGFMT(TrickGeneration, "[%s] ctfCloseSession(): already closed by Trick Libary"), _strLogHint.c_str());
		}

		_sessionHandle = NULL;
	}

	return bRet;
}

int32_t RTFProcess::releaseInputBuffer(void *hAppSession, void *hAppFile, void *hAppBuffer, unsigned char *pBuffer)
{
	//	static int count = 0;

	RTFProcess* This = (RTFProcess*)hAppSession;
	MediaSample* pSample = (MediaSample*)hAppBuffer;

	//	SMOLOG(Log::L_INFO, CLOGFMT(RTFProcess, "[%s]releaseInputBuffer [%d] "), This->_strLogHint.c_str(), count++);

	This->GetGraph()->freeMediaSample(pSample);

	if (This->GetGraph()->IsErrorOccurred())
	{
		if(This->_retryCount > 0)
			SMOLOG(Log::L_WARNING, CLOGFMT(RTFProcess, "[%s]get graph errorOccurred[%d][alreadyretry=%d][retrycount=%d]"), 
			This->_strLogHint.c_str(), This->_bFirstReleaseSrc ,This->_alreadyRetryCount , This->_retryCount);
		if(This->_bFirstReleaseSrc && This->_alreadyRetryCount < This->_retryCount)
		{
			OutputPin& pin = This->_outputPin[0];
			bool bret = false;
#ifdef ZQ_OS_LINUX
			ZQTianShan::ContentProvision::CDNFilesetTarget* fileset = (ZQTianShan::ContentProvision::CDNFilesetTarget*)pin.pNextFilter;
			bret = fileset->resetMainfile();
#else
			ZQTianShan::ContentProvision::FilesetTarget* fileset = (ZQTianShan::ContentProvision::FilesetTarget*)pin.pNextFilter;
			bret = fileset->resetMainfile();
#endif
			if(!bret)
			{
				This->_bStop = true;
				return 0;
			}
			This->_bRetry = true;  
			This->GetGraph()->RetSetError(false);
		}
		else
		{
			This->_bStop = true;
		}
		return 0;
	}


	if (This->_bFirstReleaseSrc)
	{		
		CTFVIDEOINFORMATION vi;
		vi.length = sizeof(vi);
		static int nReqFlag = CTF_VI_BITRATE | CTF_VI_RESOLUTION | CTF_VI_FRAMETIME90 | CTF_VI_VIDEOCODEC;		
		vi.flags = nReqFlag;
		ctfGetVideoInformation(This->_sessionHandle, &vi);

		// see if all info requested was returned
		if (vi.flags == nReqFlag)
		{
			//
			// fire the OnMediaInfo event
			//
			MediaInfo info;
			info.bitrate = vi.transportBitrate;
			info.videoResolutionH = vi.videoResolution.height;
			info.videoResolutionV = vi.videoResolution.width;
			info.framerate = (vi.videoFrameTime == 0) ? 1. : 90000. / (float)vi.videoFrameTime;

			SMOLOG(Log::L_INFO, CLOGFMT(TrickGeneration, "[%s] MediaInfo: bitrate[%d],video_bitrate[%d],video_height[%d],video_width[%d],frame_rate[%.2f]"),
				This->_strLogHint.c_str(), info.bitrate,info.videoBitrate,info.videoResolutionH,info.videoResolutionV,
				info.framerate);

			This->GetGraph()->OnMediaInfoParsed(info);			
			This->_bFirstReleaseSrc = false;
		}
		else
		{
// 			SMOLOG(Log::L_WARNING, CLOGFMT(TrickGeneration, "[%s] failed on ctfSesGetStreamProfile, OnMediaInfoParsed() retry"), This->_strLogHint.c_str());
 			This->_bRetry = true;
		}
	}	

	return 0;
}

int32_t RTFProcess::trickFileCloseOutput( void *hAppSession, void *hAppFile )
{
	RTFProcess* This = (RTFProcess*)hAppSession;
	POutputFileInfo pOutputFile = (POutputFileInfo)hAppFile;

	SMOLOG(Log::L_INFO, CLOGFMT(TrickGeneration, "[%s] trickFileCloseOutput() callback entering for No.%d %s file"), This->_strLogHint.c_str(),
		pOutputFile->fileNo, pOutputFile->extension);

	This->_eosSubFileCount++;
	if (This->_eosSubFileCount>=This->_outputFileCount-1)
		This->_sessClosed = true;

	return 0;
}

#ifdef ZQ_OS_MSWIN
int32_t RTFProcess::trickFilePutOutputBuffer(void *hAppSession, void *hAppFile, void *hAppBuffer,unsigned char *pBuffer, unsigned long occupancy, CTF_BUFSEEK bufSeek, INT64 bufSeekOffset)
	{
		return RTFProcess::trickFilePutOutputBuffer_unite(hAppSession, hAppFile, hAppBuffer, pBuffer, occupancy, bufSeek, bufSeekOffset);
	}

#else
	int32_t RTFProcess::trickFilePutOutputBuffer(void *hAppSession, void *hAppFile, void *hAppBuffer,uint8 *pBuffer, uint32_t occupancy, CTF_BUFSEEK bufSeek, int64_t bufSeekOffset)
	{
		return RTFProcess::trickFilePutOutputBuffer_unite(hAppSession, hAppFile, hAppBuffer, pBuffer, occupancy, bufSeek, bufSeekOffset);
	}
#endif


int RTFProcess::trickFilePutOutputBuffer_unite(void *hAppSession, void *hAppFile, void *hAppBuffer,uint8 *pBuffer, uint32 occupancy, CTF_BUFSEEK bufSeek, int64 bufSeekOffset)
{
	RTFProcess* This = (RTFProcess*)hAppSession;
	POutputFileInfo pOutputFile = (POutputFileInfo)hAppFile;
	MediaSample* pSample = (MediaSample*)hAppBuffer;

	if (!pSample)
	{
		SMOLOG(Log::L_INFO, CLOGFMT(TrickGeneration, "[%s] trickFilePutOutputBuffer pSample is null"), This->_strLogHint.c_str());
		return 0;
	}

	if (!occupancy)
	{
		SMOLOG(Log::L_INFO, CLOGFMT(TrickGeneration, "[%s] trickFilePutOutputBuffer output 0 bytes"), This->_strLogHint.c_str());

		//
		//release the sample
		This->GetGraph()->freeMediaSample(pSample);
		return 0;
	}

	if (This->GetGraph()->IsErrorOccurred())
	{
		//skip the output buffer is error occurred, release the sample
		This->GetGraph()->freeMediaSample(pSample);
		This->_bStop = true;		
		return 0;//we could not return -1, which may cause the rtflib unstable
	}

	int nIndex;

	std::map<int, int>::iterator indexIter = This->_indexExMap.find(pOutputFile->fileType);
	if (indexIter != This->_indexExMap.end())
		nIndex = (*indexIter).second;
	else
	{
		switch (pOutputFile->fileType)
		{
		case OPTFT_VV2:
		case OPTFT_VVX:
		case OPTFT_VVC:
			nIndex = 1;
			break;
		default:
			nIndex = 0;
			break;
		}
	}

	pSample->setDataLength(occupancy);		

	{
		switch( bufSeek )
		{
		case CTF_BUFSEEK_NONE:
			break;

		case CTF_BUFSEEK_SET:
			pOutputFile->fileOffset = bufSeekOffset;
			break;

		case CTF_BUFSEEK_CUR:
			pOutputFile->fileOffset += bufSeekOffset;
			break;

		case CTF_BUFSEEK_END:
			pOutputFile->fileOffset = pOutputFile->filesize;
			break;
		default:
			SMOLOG(Log::L_WARNING, CLOGFMT(TrickGeneration, "[%s] trickFilePutOutputBuffer() callback, unknown seek type %d for [%d][%s]"), 
				This->_strLogHint.c_str(), bufSeek, pOutputFile->fileNo, pOutputFile->extension);

			This->GetGraph()->freeMediaSample(pSample);
			return 0;
		}

		pSample->setOffset(pOutputFile->fileOffset);
	}

	pOutputFile->fileOffset += occupancy;
	if (pOutputFile->fileOffset > pOutputFile->filesize)
		pOutputFile->filesize = pOutputFile->fileOffset;
	if (pOutputFile->fileOffset > pOutputFile->endOffset)
		pOutputFile->endOffset = pOutputFile->fileOffset;
	if (pOutputFile->fileOffset < pOutputFile->begOffset || pOutputFile->fileOffset<pOutputFile->endOffset)
		pOutputFile->begOffset = pOutputFile->fileOffset;

	OutputPin& pin = This->_outputPin[nIndex];
	if (!pin.pNextFilter->Receive(pSample, pin.nNextPin))
	{
		SMOLOG(Log::L_ERROR, CLOGFMT(TrickGeneration, "[%s] Next filter[%s] failed to Receive data, stop process"), This->_strLogHint.c_str(), pin.pNextFilter->GetName());
        This->SetLastError("Next filter failed to Receive data", ERRCODE_RTFLIB_ERROR);
		//
		//release the sample
		This->GetGraph()->freeMediaSample(pSample);
		This->_bStop = true;		
		return 0;//we could not return -1, which may cause the rtflib unstable
	}		

	return 0;
}

#ifdef ZQ_OS_MSWIN
int32_t RTFProcess::trickFileGetOutputBuffer( void *hAppSession, void *hAppFile, void **phAppBuffer,unsigned char **ppBuffer, unsigned long *pCapacity)
{
	return RTFProcess::trickFileGetOutputBuffer_unite(hAppSession, hAppFile, phAppBuffer, ppBuffer, (uint32*)pCapacity);
}
#else
int32_t RTFProcess::trickFileGetOutputBuffer( void *hAppSession, void *hAppFile, void **phAppBuffer,uint8_t **ppBuffer, uint32_t *pCapacity)
{
	return RTFProcess::trickFileGetOutputBuffer_unite(hAppSession, hAppFile, phAppBuffer, ppBuffer, pCapacity);;
}
#endif

int RTFProcess::trickFileGetOutputBuffer_unite(void *hAppSession, void *hAppFile, void **phAppBuffer, uint8** ppBuffer, uint32 *pCapacity)
{
	RTFProcess* This = (RTFProcess*)hAppSession;
	//POutputFileInfo pOutputFile = (POutputFileInfo)hAppFile;

	//
	// Allocate a data buffer
	//
	MediaSample* pSample = This->GetGraph()->allocMediaSample();
	if (!pSample)
	{
		//
		SMOLOG(Log::L_ERROR, CLOGFMT(TrickGeneration, "[%s] failed to alloc meida sample, stop process"), This->_strLogHint.c_str());
        This->SetLastError("failed to alloc meida sample", ERRCODE_BUFFERQUEUE_FULL);
		*phAppBuffer = 0;
		*pCapacity = 0;
		*ppBuffer = 0;

		This->_bStop = true;
		return 0;
	}

	*phAppBuffer = pSample;
	*pCapacity = pSample->getBufSize();
	*ppBuffer = (unsigned char*)pSample->getPointer();

	return 0;
}

void RTFProcess::settrickSpeedNumerator( std::list<float>& temp )
{
	int index = 0;
	for(std::list<float>::iterator iter = temp.begin();iter != temp.end()&& index < sizeof(_newTrickSpeedNumerator);iter++,index++)
	{
		_newTrickSpeedNumerator[index] = int(ceil((*iter)*2));
	}
}
void RTFProcess::settrickSpeedNumeratorHD( std::list<float>& temp)
{
	int index = 0;
	for(std::list<float>::iterator iter = temp.begin();iter != temp.end() && index < sizeof(_newTrickSpeedNumeratorHD);iter++,index++)
	{
		_newTrickSpeedNumeratorHD[index] = int(ceil((*iter)*2));
	}
}
void RTFProcess::setLegacyAudioOnly()
{
	ctfSetOutputSetting( _sessionHandle, "legacyAudioOnly", _bAudioOnly ? "1" : "0" );
}

void RTFProcess::getOutputFileExtCol(std::string& ext)
{
	ext.clear();
	std::string tempext;
	for (int i = 0;i < _outputFileCount;i++)
	{
		tempext = _outputInfo[i].extension;
		if (tempext[0] == '.')
			tempext = tempext.substr(1);
		ext += tempext;
		if (i != _outputFileCount-1)
			ext += std::string(";");
	}
}

void RTFProcess::getOutputFileInfo(std::map<std::string, std::string>& fileMap )
{
	char tmp[40];char tmp1[40];	
    for (int i = 0;i < _outputFileCount;i++)
	{
		std::string tempext = _outputInfo[i].extension;
		if (tempext[0] == '.')
			tempext = tempext.substr(1);

		sprintf(tmp, FMT64, _outputInfo[i].begOffset);
		sprintf(tmp1, FMT64,_outputInfo[i].endOffset);
             std::string strrange = std::string(tmp)+std::string("-")+std::string(tmp1);
		fileMap.insert(std::make_pair(tempext,strrange));
	}
}

void RTFProcess::getIndexType( std::string& type )
{
	if(_indexType == CTF_INDEX_TYPE_VV2)
		type = "VV2";
#ifndef ZQ_OS_MSWIN
	else if (_indexType == CTF_INDEX_TYPE_VVC)
		type = "index";
#endif
	else
		type = "VVX";
}

void RTFProcess::setAssetInfo(const std::string& providerId, const std::string& provAssetId )
{
	_strprovId = providerId;
	_strprovAssetId = provAssetId;
}

void RTFProcess::setProviderId()
{
#ifndef ZQ_OS_MSWIN
	if (!_strprovId.empty())
		ctfSetOutputSetting(_sessionHandle, "providerID",_strprovId.c_str());
	if (!_strprovAssetId.empty())
		ctfSetOutputSetting(_sessionHandle, "assetID",_strprovAssetId.c_str());
#endif
}
void RTFProcess::setAugmentationPids(uint16* pIDs, int pidcount)
{
  _augmentationPidCount = pidcount;
  for(int i = 0; i < pidcount && i < MAX_AUGMENTATION_PIDS; ++i)
  {
	  _augmentationPid[i] = pIDs[i];
  }
}
// read input file callback function (only used for augmentation pre-scan) **************
#ifdef ZQ_OS_MSWIN
int32_t RTFProcess::readInputStream( void* hAppSession,unsigned char *pBuffer, unsigned long bufSize,
					unsigned long *pBufOccupancy,CTF_BUFSEEK bufSeek, INT64 bufSeekOffset )
#else
int32_t RTFProcess::readInputStream( void* hAppSession, uint8_t* pBuffer, uint32_t bufSize,
								 uint32_t* pBufOccupancy, CTF_BUFSEEK bufSeek, int64_t bufSeekOffset )
#endif
{
	RTFProcess* This = (RTFProcess*)hAppSession;
	unsigned int iResult = 0;
	int doSeek = 0, seekType = 0;

	// is a seek being requested?
	switch( bufSeek )
	{
	case CTF_BUFSEEK_NONE:
		break;
	case CTF_BUFSEEK_SET:
		doSeek = 1;
		seekType = SEEK_SET;
		break;
	case CTF_BUFSEEK_CUR:
		doSeek = 1;
		seekType = SEEK_CUR;
		break;
	case CTF_BUFSEEK_END:
		doSeek = 1;
		seekType = SEEK_END;
		break;
	default:
// 		PRINTF( "Illegal SEEK option in read input file callback (%d)\n", bufSeek );
		return -1;
	}
	InputPin&  pin = This->_inputPin[0];
	BaseSource* pBaseSource = (BaseSource*)(pin.pPrevFilter);

	// preform the seek, if requested
	if( doSeek != 0 )
	{
	  pBaseSource->seek(bufSeekOffset, seekType);
	}
	// perform the read, if requested
	if( bufSize > 0 )
	{
		pBaseSource->readbuf((char*)pBuffer, bufSize, iResult);
		// check for error
		if( iResult < 0 )
		{
// 			PRINTF( "Error reading input stream\n" );
			*pBufOccupancy = 0;
			return -1;
		}
		// return the number of bytes actually read
		*pBufOccupancy = iResult;
	}

	return 0;
}

int RTFProcess::augmentationScan()
{
	int result = 0;

	if( ctfGetAugmentationInfo( _sessionHandle,
		_augmentationPidCount,
		_augmentationPid,
		readInputStream,
		&_augmentedBitRate,
		&_originalBitRate) != TRUE )
	{
//		PRINTF( "ctfGetAugmentationInfo failed on file %s\n", pInputPath[ sessionNumber ] );
		result = -1;
	}
	return result;
}
void RTFProcess::getPreEncryptBitRate(	int& augmentedBitRate, int& originalBitRate)
{
	augmentedBitRate = _augmentedBitRate;
	originalBitRate  = _originalBitRate;
}
void RTFProcess::getAugmentationPids(std::string& augmentionpids)
{
	char temp[32] = "";
	augmentionpids = "";
	for(int i = 0; i < _augmentationPidCount && i < MAX_AUGMENTATION_PIDS; ++i)
	{
		sprintf(temp, "%d;",_augmentationPid[i]);
		augmentionpids += temp;
		memset(temp, 0, 32);
	}
}
void RTFProcess::setRetryCount(int retryCount)
{
	_retryCount = retryCount;
}
void RTFProcess::setTrickFile(std::map<std::string, int>& temp)
{
	std::map<std::string, int>::iterator itor = temp.begin();
	while(itor != temp.end())
	{
		FileExtension fileExt;
		fileExt.ext = itor->first;
		fileExt.extForCisco = "";
		fileExt.position = itor->second;
//		_trickSpeedAndFileExt.push_back(fileExt);
		_trickSpeedAndFileExt.insert(std::make_pair(itor->first, fileExt));
		itor++;
	}
}
void RTFProcess::getCTFLibVersion(int& major, int& Minor)
{
	major = _ctfLibVerMajor;
	Minor = _ctfLibVerMinor;
}
}}

