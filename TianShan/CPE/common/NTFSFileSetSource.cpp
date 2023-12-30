#include "NTFSFileSetSource.h"
#include "Log.h"

#include "vstrmuser.h"
#include "PacedIndex.h"
#include "vvx.h"
#include "errorcode.h"



#define NTFSFileSetSrc			"NTFSFileSetSrc"

using namespace ZQ::common;

#define MOLOG (*_pLog)

namespace ZQTianShan {
	namespace ContentProvision {
		
		
		NTFSFileSetSource::NTFSFileSetSource()
		{
			_nOutputCount = 4;
			_nInputCount = 1;
			
			int i;
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
			
			_llProFF = 0;
			_llProFR = 0;
			_llProVVX = 0;
			_llProRegin = 0;
			
			_llProcBytes = 0;
			_bDriverModule = true;
			_nBandwidthBps = 0;
			
			_nBwCtlBytes = 0;
			_nBwCtlMilliSec = 500;		//500ms
			
			_failNum = 0;
			_hFileOrign = NULL;
			_hFileff = NULL;
			_hFilefr = NULL;
			_hFilevvx = NULL;
			_ntfsCacheHandle = NULL;
			_bStop = false;
			
			_posCacheRead.QuadPart = 0;
			_vvxByteRead = 0;
			_bEndOfVvx = false;
			_bEndOfFF = false;
			_bEndOfFR = false;
			_bEndOfRegin = false;
			_filesize = 0;
		}
		
		NTFSFileSetSource::~NTFSFileSetSource()
		{
			DeleteFile((_cachePath+ _subFiles[3].strFilename).c_str());
		}
		void NTFSFileSetSource::setSourceDirectory(const char* szSourceDir) 
		{ 	
			_sourcePath = szSourceDir;
			if (szSourceDir[strlen(szSourceDir)-1] != '\\')
			{
				_sourcePath += "\\";
			}
			
		}
		void NTFSFileSetSource::setCachDir(const char* cachpath) 
		{ 	
			_cachePath = cachpath;
			if (cachpath[strlen(cachpath)-1] != '\\')
			{
				_cachePath += "\\";
			}

		}

		void NTFSFileSetSource::setWaitTime(int waittime)
		{
			if (waittime == 0)
				_waittime = 1000;
			else
				_waittime = waittime*1000;
		}
		
		bool NTFSFileSetSource::Init()
		{
			_nBwCtlBytes = int(((float)_nBandwidthBps)*_nBwCtlMilliSec/8000);
			MOLOG(Log::L_INFO, CLOGFMT(NTFSFileSetSrc, "[%s] NTFSFileSetSource::initialized"), _strLogHint.c_str());
			LONGLONG regionFilesize;
			LONGLONG ffFilesize;
			
			if (!_hFileOrign)
			{
				_hFileOrign = CreateFileA((_sourcePath + _subFiles[0].strFilename).c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,0, OPEN_EXISTING,0,0);
				if (INVALID_HANDLE_VALUE == _hFileOrign)
				{
					std::string errmsg;
					getSystemErrorText(errmsg);
					SetLastError(errmsg, ERRCODE_NTFS_CREATEFILE);	
					
					MOLOG(Log::L_ERROR, CLOGFMT(NTFSFileSetSrc, "[%s] OpenFile %s failed with error: %s"), _strLogHint.c_str(),_subFiles[0].strFilename.c_str(), errmsg.c_str());
					return false;
				}
				regionFilesize = GetFileSize(_hFileOrign,NULL); 
			}
			
			if (!_hFileff)
			{
				_hFileff = CreateFileA((_sourcePath + _subFiles[1].strFilename).c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,0, OPEN_EXISTING,0,0);
				if (INVALID_HANDLE_VALUE == _hFileff)
				{
					std::string errmsg;
					getSystemErrorText(errmsg);
					SetLastError(errmsg, ERRCODE_NTFS_CREATEFILE);	
					
					MOLOG(Log::L_ERROR, CLOGFMT(NTFSFileSetSrc, "[%s] OpenFile %s failed with error: %s"), _strLogHint.c_str(), _subFiles[1].strFilename.c_str(),errmsg.c_str());
					return false;
				}
				ffFilesize = GetFileSize(_hFileff,NULL);
			}
			
			//calculate the 
			_fColRate = (float)regionFilesize/(float)ffFilesize;
			if (!_hFilefr)
			{
				_hFilefr = CreateFileA((_sourcePath + _subFiles[2].strFilename).c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,0, OPEN_EXISTING,0,0);
				if (INVALID_HANDLE_VALUE == _hFilefr)
				{
					std::string errmsg;
					getSystemErrorText(errmsg);
					SetLastError(errmsg, ERRCODE_NTFS_CREATEFILE);	
					
					MOLOG(Log::L_ERROR, CLOGFMT(NTFSFileSetSrc, "[%s] OpenFile %s failed with error: %s"), _strLogHint.c_str(),_subFiles[2].strFilename.c_str(), errmsg.c_str());
					return false;
				}
			}
			if (!_hFilevvx)
			{
				_hFilevvx = CreateFileA((_sourcePath + _subFiles[3].strFilename).c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,0, OPEN_EXISTING,0,0);
				if (INVALID_HANDLE_VALUE == _hFilevvx)
				{
					std::string errmsg;
					getSystemErrorText(errmsg);
					SetLastError(errmsg, ERRCODE_NTFS_CREATEFILE);	
					
					MOLOG(Log::L_ERROR, CLOGFMT(NTFSFileSetSrc, "[%s] OpenFile %s failed with error: %s"), _strLogHint.c_str(), _subFiles[3].strFilename.c_str(),errmsg.c_str());
					return false;
				}
			}
			if (_ntfsCacheHandle == NULL)
			{
				
				std::string filePath = _cachePath+ _subFiles[3].strFilename;
				MOLOG(Log::L_INFO, CLOGFMT(NTFSFileSetSrc, "[%s] Create output file %s on local disk"), _strLogHint.c_str(),
					filePath.c_str());
				
				_ntfsCacheHandle = CreateFileA(filePath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);		
				if (INVALID_HANDLE_VALUE == _ntfsCacheHandle)
				{
					std::string errmsg;
					getSystemErrorText(errmsg);
					SetLastError(errmsg, ERRCODE_NTFS_CREATEFILE);	
					
					MOLOG(Log::L_ERROR, CLOGFMT(NTFSFileSetSrc, "[%s] CreateFile failed with error: %s"), _strLogHint.c_str(), errmsg.c_str());
					return false;
				}
			}
			return true;
		}
		
		void NTFSFileSetSource::Stop()
		{
			BaseFilter::Stop();
			
			
			MOLOG(Log::L_INFO, CLOGFMT(NTFSFileSetSrc, "[%s] Stop() called"), _strLogHint.c_str());
		}
		
		void NTFSFileSetSource::Close()
		{
			
		}
		
		void NTFSFileSetSource::endOfStream()
		{
			
		}
		
		const char* NTFSFileSetSource::GetName()
		{
			return SOURCE_TYPE_NTFS_FILESET;
		}
		
		LONGLONG NTFSFileSetSource::getProcessBytes()
		{
			return _llProcBytes;
		}
		
		void NTFSFileSetSource::setFilename(const char* szFile)
		{
			_subFiles[0].strFilename = szFile;
			_subFiles[1].strFilename = std::string(szFile) + ".ff";
			_subFiles[2].strFilename = std::string(szFile) + ".fr";
			_subFiles[3].strFilename = std::string(szFile) + ".vvx";
		}
		
		MediaSample* NTFSFileSetSource::GetData(int nOutputIndex)
		{
			if (_bStop)
			{
				MOLOG(Log::L_INFO, CLOGFMT(NTFSFileSetSrc, "[%s] require to stop while GetData()"), _strLogHint.c_str());
				
				// set the total bytes to processed bytes
				GetGraph()->setTotalBytes(_llProcBytes);
				return NULL;
			}
			
			MediaSample* pSample = GetGraph()->allocMediaSample();
			if (!pSample)
			{
				MOLOG(Log::L_ERROR, CLOGFMT(NTFSFileSetSrc, "[%s] failed to alloc meida sample"), _strLogHint.c_str());
				
				// set the total bytes to processed bytes
				GetGraph()->setTotalBytes(_llProcBytes);
				_failNum++;
				if (_failNum > 3)
					_bStop = true;
				return NULL;
			}
			_nBufSize = pSample->getBufSize();
			
			if (!_nLastTime)
			{
				_nLastTime = GetTickCount();
				_nBytesPro = 0;
			}
			
			
			unsigned long nRead;
			
			if(nOutputIndex == 0)
			{
				if (!_bEndOfRegin)
				{	
					bool bRetryAvoided = false;
					bool bDoingFinalRead = false;
					int i= 0;
again:		
					bool result = ReadFile(_hFileOrign,pSample->getPointer(),_nBufSize,&nRead,NULL);
					if (!result)
					{
						std::string error = GetLastError();
					}
					if(TRUE == result && 0 == nRead)
					{
						if (i > 4)
						{
							GetGraph()->freeMediaSample(pSample);
							return NULL;
						}
						if(false == bRetryAvoided)
						{
							// possible EOF
							i++;
							Sleep(_waittime);
							bRetryAvoided = true;
							
							goto again;
						}
						else
						{
							// if we've been here already, and found that the
							// writer's gone, this is set to true and we're in
							// the process of a final read	
							HANDLE tmp = CreateFile((_sourcePath + _subFiles[nOutputIndex].strFilename).c_str(), GENERIC_READ,
								FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
							
							if(INVALID_HANDLE_VALUE == tmp)
							{
								i++;
								Sleep(_waittime);
								
								goto again;
							}
							else
							{
								CloseHandle(tmp);
									
									if(!bDoingFinalRead)
									{
										i++;
										bDoingFinalRead = true;
										goto again;
									}
									else
									{
										_bEndOfRegin = true;
										GetGraph()->freeMediaSample(pSample);
										
										GetGraph()->setTotalBytes(_llProcBytes);
										return NULL;
									}
							}
						}
					}
					if (!nRead)
					{
						//no more data
						GetGraph()->freeMediaSample(pSample);
						MOLOG(Log::L_INFO, CLOGFMT(NTFSFileSetSrc, "[%s] read return 0 bytes, no more data available"), _strLogHint.c_str());
						
						// set the total bytes to processed bytes
						GetGraph()->setTotalBytes(_llProcBytes);
						return NULL;
					}
					pSample->setDataLength(nRead);
					pSample->setOffset(_llProRegin);
					_llProRegin += nRead;
					_llProcBytes = _llProRegin;
				}
				else
				{
					GetGraph()->freeMediaSample(pSample);
					return NULL;
				}
			}
			else if (nOutputIndex == 1 || nOutputIndex == 2)
			{
				unsigned int bufsize = (unsigned int)((float)_nBufSize/_fColRate);
				
				if (nOutputIndex == 1)
				{
					if(!_bEndOfFF)
					{
						bool bRetryAvoided = false;
						bool bDoingFinalRead = false;
						int i= 0;
ff:		
						bool result =ReadFile(_hFileff,pSample->getPointer(),bufsize,&nRead,NULL);
						
						if(TRUE == result && 0 == nRead)
						{
							if (i > 4)
							{
								GetGraph()->freeMediaSample(pSample);
								return NULL;
							}
							if(false == bRetryAvoided)
							{
								// possible EOF
								i++;
								Sleep(_waittime);
								bRetryAvoided = true;
								
								goto ff;
							}
							else
							{
								// if we've been here already, and found that the
								// writer's gone, this is set to true and we're in
								// the process of a final read	
								HANDLE tmp = CreateFile((_sourcePath + _subFiles[nOutputIndex].strFilename).c_str(), GENERIC_READ,
									FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
								
								if(INVALID_HANDLE_VALUE == tmp)
								{
									i++;
									Sleep(_waittime);
									
									goto ff;
								}
								else
								{
									CloseHandle(tmp);
										
										if(!bDoingFinalRead)
										{
											i++;
											bDoingFinalRead = true;
											goto ff;
										}
										else
										{
											_bEndOfFF = true;
											GetGraph()->freeMediaSample(pSample);
											
											GetGraph()->setTotalBytes(_llProcBytes);
											return NULL;
										}
								}
							}
						}
						if (!nRead)
						{
							
							//no more data
							GetGraph()->freeMediaSample(pSample);
							MOLOG(Log::L_INFO, CLOGFMT(NTFSFileSetSrc, "[%s] read return 0 bytes, no more data available"), _strLogHint.c_str());
							
							// set the total bytes to processed bytes
							GetGraph()->setTotalBytes(_llProcBytes);
							return NULL;
						}
						pSample->setDataLength(nRead);
						pSample->setOffset(_llProFF);
						_llProFF += nRead;
						_llProcBytes = _llProFF;
					}
					else
					{
						GetGraph()->freeMediaSample(pSample);
						return NULL;
					}
				}
				else
				{
					if (!_bEndOfFR)
					{
						bool bRetryAvoided = false;
						bool bDoingFinalRead = false;
						int i= 0;
fr:		
						bool result = ReadFile(_hFilefr,pSample->getPointer(),bufsize,&nRead,NULL);
						if(TRUE == result && 0 == nRead)
						{
							if (i > 4)
							{
								GetGraph()->freeMediaSample(pSample);
								return NULL;
							}
							if(false == bRetryAvoided)
							{
								i++;
								// possible EOF
								Sleep(_waittime);
								bRetryAvoided = true;
								
								goto fr;
							}
							else
							{
								// if we've been here already, and found that the
								// writer's gone, this is set to true and we're in
								// the process of a final read	
								HANDLE tmp = CreateFile((_sourcePath + _subFiles[nOutputIndex].strFilename).c_str(), GENERIC_READ,
									FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
								
								if(INVALID_HANDLE_VALUE == tmp)
								{
									i++;
									Sleep(_waittime);
									
									goto fr;
								}
								else
								{
									CloseHandle(tmp);
										
										if(!bDoingFinalRead)
										{
											i++;
											bDoingFinalRead = true;
											goto fr;
										}
										else
										{
											_bEndOfFR = true;
											GetGraph()->freeMediaSample(pSample);
											
											GetGraph()->setTotalBytes(_llProcBytes);
											return NULL;
										}
								}
							}
						}
						if (!nRead)
						{
							
							//no more data
							GetGraph()->freeMediaSample(pSample);
							MOLOG(Log::L_INFO, CLOGFMT(NTFSFileSetSrc, "[%s] read return 0 bytes, no more data available"), _strLogHint.c_str());
							
							// set the total bytes to processed bytes
							GetGraph()->setTotalBytes(_llProcBytes);
							return NULL;
						}
						pSample->setDataLength(nRead);
						pSample->setOffset(_llProFR);
						_llProFR += nRead;
						_llProcBytes = _llProFR;
					}
					else
					{
						GetGraph()->freeMediaSample(pSample);
						return NULL;
					}
				}
				
				
	}
	else if (nOutputIndex == 3)
	{
		if (!_bEndOfVvx)
		{	
			bool result = WriteIndexFile(nRead);
			
			if (!result)
			{
				GetGraph()->freeMediaSample(pSample);
				return NULL;
			}
			
			if (!nRead)
			{
				//no more data
				GetGraph()->freeMediaSample(pSample);
				MOLOG(Log::L_INFO, CLOGFMT(NTFSFileSetSrc, "[%s] read return 0 bytes, no more data available."), _strLogHint.c_str());
				
				// set the total bytes to processed bytes
				GetGraph()->setTotalBytes(_llProcBytes);
				
				return NULL;
			}
			pSample->init((void*)_tmpbuffer, nRead);
			pSample->setDataLength(nRead);
			pSample->setOffset(_llProVVX);
			_llProVVX += nRead;
			_llProcBytes = _llProVVX;
		}
		else
		{
			GetGraph()->freeMediaSample(pSample);
			return NULL;
		}
	}
	
	// bandwidth control
	//
	if (_nBandwidthBps && nOutputIndex == 1)
	{
		_nBytesPro += nRead;
		
		int nShouldSpent = int(((LONGLONG)_nBytesPro)*8000/_nBandwidthBps); //ms
		int nSpent = GetTickCount()-_nLastTime;
		
		if (nSpent>=0 && nShouldSpent > nSpent + 16)
		{
			int nSleep = nShouldSpent - nSpent;
			if (nSleep<100000)		//impossible
				Sleep(nShouldSpent - nSpent);
		}
		
		if (_nBytesPro > _nBwCtlBytes)
		{
			_nBytesPro = 0;
			_nLastTime = GetTickCount();
		}
	}
	
	return pSample;
}


void NTFSFileSetSource::setMaxBandwidth(unsigned int nBandwidthBps)
{
	_nBandwidthBps = nBandwidthBps;
	_nBwCtlBytes = int(((float)_nBandwidthBps)*_nBwCtlMilliSec/8000);
	
	MOLOG(Log::L_INFO, CLOGFMT(NTFSFileSetSrc, "[%s] set MaxBandwidth=%d bps"), _strLogHint.c_str(), nBandwidthBps);
}

void NTFSFileSetSource::setBandwidthCtrlInterval(int nIntervalMs)
{
	_nBwCtlMilliSec = nIntervalMs;
	MOLOG(Log::L_INFO, CLOGFMT(NTFSFileSetSrc, "[%s] set BandwidthControlInterval=%d ms"), _strLogHint.c_str(), nIntervalMs);
}

bool NTFSFileSetSource::Run()
{
	MOLOG(Log::L_INFO, CLOGFMT(NTFSFileSetSrc, "[%s] enter NTFSFileSetSource::run()"), _strLogHint.c_str());
	
	while (!_bStop)
	{
		MediaSample* pSample = NULL;
		for (int i= 0; i < 4; i++)
		{
			if (i== 3 && _bEndOfVvx)
				continue;
			if (i == 3)
				Sleep(600);
			
			pSample = GetData(i);
			OutputPin& pin = this->_outputPin[i];
			if (pSample && pin.pNextFilter)
			{
				bool result = pin.pNextFilter->Receive(pSample,pin.nNextPin);
				if (!result)
				{
					MOLOG(Log::L_ERROR, CLOGFMT(NTFSFileSetSrc, "[%s] failed to receive data for file which index is %d."), _strLogHint.c_str(),i);
				}
			}
			else
			{
				if (!pSample)
				{			
					if(_bEndOfRegin && _bEndOfFF && _bEndOfFR && _bEndOfVvx)
						_bStop = true;
				//	MOLOG(Log::L_INFO, CLOGFMT(NTFSFileSetSrc, "[%s] File index=%d, which get empty media."), _strLogHint.c_str(),i);
					
				}
				else
				{
					MOLOG(Log::L_INFO, CLOGFMT(NTFSFileSetSrc, "[%s] Wrong information of outputPin"), _strLogHint.c_str());
					//return false;
				}
			}
		}
		
	}
	
	for (int i= 0; i < 4; i++)
	{
		OutputPin& pin = this->_outputPin[i];
		pin.pNextFilter->Close();
	}
	
	CloseHandle(_hFileOrign);
	CloseHandle(_hFileff);
	CloseHandle(_hFilefr);
    CloseHandle(_hFilevvx);
	CloseHandle(_ntfsCacheHandle);
	
	return true;
}


bool NTFSFileSetSource::WriteIndexFile(unsigned long& readbytes)
{
	// move file point for reading
#ifdef _DEBUG
	MOLOG(Log::L_DEBUG, CLOGFMT(NTFSFileSetSrc, "[%s] Set index reader position: %lld"), _strLogHint.c_str(), _posCacheRead.QuadPart);
#endif	
	if(INVALID_SET_FILE_POINTER == SetFilePointer(_hFilevvx, _posCacheRead.LowPart, &_posCacheRead.HighPart, FILE_BEGIN))
	{
		return false;
	}
	
	DWORD byteRead = 0;
	
vvx:	
	if(!ReadFile(_hFilevvx, _tmpbuffer, NTFSFS_IO_SIZE, &byteRead, NULL))
	{
		// log for fail
		return false;
	}
	Sleep(20);
	_filesize = GetFileSize(_hFilevvx,NULL);
	
	//	MOLOG(Log::L_INFO, CLOGFMT(NTFSFileSetSrc, "[%s] read bytes is %d."), _strLogHint.c_str(),byteRead);
	// if we get a single terminator record, and the other writer is gone,
	// then return the terminator - the next read will be 0 bytes
	if(sizeof(VVX_V7_RECORD) == byteRead)
	{
		VVX_V7_RECORD *pRecord = (VVX_V7_RECORD *)(_tmpbuffer);
		
		if(TRICK_INDEX == pRecord->recordType && 0 == pRecord->trick.timeCode.time)
		{
			LARGE_INTEGER tmp;
			tmp.QuadPart = _vvxByteRead;
			if(0xFFFFFFFF == SetFilePointer(_hFilevvx, tmp.LowPart, &tmp.HighPart, FILE_BEGIN))
			{
				// log for fail
				return false;
			}
			else
			{
				HANDLE tmp = CreateFile((_sourcePath + _subFiles[3].strFilename).c_str(), GENERIC_READ,
					FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
				
				if(INVALID_HANDLE_VALUE == tmp)
				{
					Sleep(_waittime);
					
					goto vvx;
				}
				CloseHandle(tmp);
				// remember the position for reading
				// _posCacheRead = tmp; 
				readbytes = byteRead;
				_bEndOfVvx = true;
				MOLOG(Log::L_INFO, CLOGFMT(NTFSFileSetSrc, "[%s] vvx file has reached the end."), _strLogHint.c_str());
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
		if(0xFFFFFFFF == SetFilePointer(_hFilevvx, tmp.LowPart, &tmp.HighPart, FILE_BEGIN))
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
			if(0xFFFFFFFF == SetFilePointer(_hFilevvx, tmp.LowPart, &tmp.HighPart, FILE_BEGIN))
			{
				// log for fail
				return false;
			}
			// remember the position for reading
			_posCacheRead = tmp;//_bEndOfVvx=TRUE;
		}
		else
		{
			_vvxByteRead += byteRead;
			
			// remember the position for reading
			_posCacheRead.QuadPart = byteRead;
		}
	}
	
	
#ifdef _DEBUG
	MOLOG(Log::L_DEBUG, CLOGFMT(NTFSFileSetSrc, "[%s] Save index reader position: %lld"), 
		_strLogHint.c_str(), _posCacheRead.QuadPart);	
#endif
	
	if(0 == byteRead)
		return true;
	
	// log the last index read
	VVX_V7_RECORD *pTmp = 
		(VVX_V7_RECORD *)(_tmpbuffer + byteRead - sizeof(VVX_V7_RECORD));
	
#ifdef _DEBUG
	MOLOG(Log::L_DEBUG, CLOGFMT(NTFSFileSetSrc, "[%s] last index read: %#I64x (%#I64x)(%#I64x)"),
		_strLogHint.c_str(),
		pTmp->trick.frameByteOffset[0],
		pTmp->trick.frameByteOffset[1],
		pTmp->trick.frameByteOffset[2]);
#endif
	   readbytes = byteRead;
	   return true;
}
bool NTFSFileSetSource::readbuf(char* pBuf, unsigned int bufLen, unsigned int& rcvLen)
{
	return false;
}
bool NTFSFileSetSource::seek(int64 offset, int pos)
{
	return false;
}
}}