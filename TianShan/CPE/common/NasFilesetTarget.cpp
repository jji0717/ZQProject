
#include "Log.h"
#include "NasFilesetTarget.h"
#include "ErrorCode.h"
#include "PacedIndex.h"
#include "bitstrmlibrary.h"
#include "vvx.h"
#include "FileSystemOp.h"


#define NasFsIO			"NasFsIO"

using namespace ZQ::common;

#define MOLOG (*_pLog)
#define SMOLOG (*(pThis->_pLog))


#pragma comment(lib, "PacedIndex.lib")

namespace ZQTianShan {
	namespace ContentProvision {

bool NasFsTarget::_bPacingTrace = false;


NasFsTarget::NasFsTarget()
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
}

bool NasFsTarget::Init()
{
	_bCleanup = false;

	if(!FS::createDirectory(_filePath, true))
	{
		std::string err;
		getSystemErrorText(err);
		char msg[256];
		sprintf(msg, "CreateDirectory() failed with error: %s", err.c_str());
		SetLastError(err,ERRCODE_NTFS_CREATEFILE);

		MOLOG(Log::L_ERROR, CLOGFMT(NasFsIO, "[%s] Failed to create directory: %s with error: %s"),
			_strLogHint.c_str(), _filePath.c_str(), err.c_str());
		return false;
	}
	MOLOG(Log::L_INFO, CLOGFMT(NasFsIO, "[%s] CreateDirectory %s succeed."),
		_strLogHint.c_str(),_filePath.c_str());

	for(int i=0;i<4;i++)
	{

#ifdef FILE_FLAG_CACHED
		_subFiles[i].hOutputFile = CreateFile(
			(char*)(_filePath+_subFiles[i].strFilename).c_str(), 
			GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			0,
			CREATE_ALWAYS,			
			FILE_FLAG_CACHED,
			0);
#else
		_subFiles[i].hOutputFile = CreateFile(
			(char*)(_filePath+_subFiles[i].strFilename).c_str(), 
			GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			0,
			CREATE_ALWAYS,			
			0,
			0);
#endif
		if(_subFiles[i].hOutputFile==INVALID_HANDLE_VALUE)
		{
			std::string errstr;
			getSystemErrorText(errstr);

			char msg[256];
			sprintf(msg, "CreateFile() failed with error: %s", errstr.c_str());
			SetLastError(msg, ERRCODE_NTFS_CREATEFILE);

			MOLOG(Log::L_ERROR, CLOGFMT(NasFsIO, "[%s] Failed to create output file: %s with error: %s"),
				_strLogHint.c_str(), (_filePath+_subFiles[i].strFilename).c_str(), errstr.c_str());

			return false;
		}

		MOLOG(Log::L_INFO, CLOGFMT(NasFsIO, "open file [%s] on nas successful"),
			(_filePath+_subFiles[i].strFilename).c_str());

		_subFiles[i].llProcBytes = 0;
		_subFiles[i].pThis = this;

	}

	if(_bPacing)
	{
		//
		// generate pacing group
		//
		void* pacingGroup = this;

		int indexFileNo = 1;
		std::string pacingType="vvx";

		PacedIndexSetLogCbk(1, pacingAppLogCbk);
		DWORD paceresult = 0;
		int i;
		for(i=0; i<4; i++)
		{
			// skip the vvx, make sure it was added last
			if(i == indexFileNo)
				continue;

			paceresult = PacedIndexAdd((void *) pacingGroup, pacingType.c_str(), _subFiles[i].strFilename.c_str(), 
				pacingAppWrite, pacingAppSeek, pacingAppSetEOF, pacingAppReportOffsets, 
				(void*)&_subFiles[i], &_subFiles[i].pacingIndexCtx);
			if(paceresult)
			{
				MOLOG(Log::L_ERROR, CLOGFMT(NasFsIO, "[%s] PacedIndexAdd() on pacing group 0x%08x failed with error [%s]"), 
					_subFiles[i].strFilename.c_str(), (DWORD)(pacingGroup), DecodePacedIndexError(paceresult));
				SetLastError(std::string("PacedIndexAdd() failed with error ") + DecodePacedIndexError(paceresult), ERRCODE_PACING_ERROR);
				return false;	
			}
			MOLOG(Log::L_INFO, CLOGFMT(NasFsIO, "[%s] PacedIndexAdd() on pacing group 0x%08x successful"), 
				_subFiles[i].strFilename.c_str(), (DWORD)(pacingGroup));
		}

		// add vvx last
		paceresult = PacedIndexAdd((void*) pacingGroup, pacingType.c_str(), _subFiles[indexFileNo].strFilename.c_str(), 
			pacingAppWrite, pacingAppSeek, pacingAppSetEOF, pacingAppReportOffsets, 
			(void*)&_subFiles[indexFileNo], &_subFiles[indexFileNo].pacingIndexCtx);
		if(paceresult)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(NasFsIO, "[%s] PacedIndexAdd() on pacing group 0x%08x failed with error [%s]"), 
				_subFiles[i].strFilename.c_str(), (DWORD)(pacingGroup), DecodePacedIndexError(paceresult));
			SetLastError(std::string("PacedIndexAdd() failed with error ") + DecodePacedIndexError(paceresult), ERRCODE_PACING_ERROR);
			return false;	
		}
		MOLOG(Log::L_INFO, CLOGFMT(NasFsIO, "[%s] PacedIndexAdd() on pacing group 0x%08x successful"), 
			_subFiles[indexFileNo].strFilename.c_str(), (DWORD)(pacingGroup));
	}
			

	{
		_subFiles[1].bIndexFile = true;
		std::string filePath = _cachePath + _subFiles[1].strFilename;
		MOLOG(Log::L_INFO, CLOGFMT(NasFsIO, "[%s] Create output file %s on local disk"), _strLogHint.c_str(),
			filePath.c_str());

		_ntfsCacheHandle = CreateFileA(filePath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);		
		if (INVALID_HANDLE_VALUE == _ntfsCacheHandle)
		{
			std::string errmsg;
			getSystemErrorText(errmsg);
			SetLastError(errmsg, ERRCODE_NTFS_CREATEFILE);	

			MOLOG(Log::L_ERROR, CLOGFMT(NasFsIO, "[%s] CreateFile failed with error: %s"), _strLogHint.c_str(), errmsg.c_str());
			return false;
		}
	}

	_posCacheRead.QuadPart = 0;

	return true;
}

void NasFsTarget::Stop()
{
	
}

bool NasFsTarget::flushCacheBuffer()
{	
	for(int i=0;i<4;i++)
	{
		if (i == 1)
			continue;

		if(_subFiles[i].hOutputFile == INVALID_HANDLE_VALUE)
			continue;

		if(_subFiles[i].cacheCurLength > 0)
		{	
			DWORD amountWritten;
			if(!WriteFile(_subFiles[i].hOutputFile,_subFiles[i].cacheBuffer,  _subFiles[i].cacheCurLength, &amountWritten, NULL))
			{
				std::string errstr;
				getSystemErrorText(errstr);

				SetLastError(std::string("Nas failed to write file ") + _subFiles[i].strFilename + " with error:" + errstr);
				MOLOG(Log::L_ERROR, CLOGFMT(NasFsIO, "[%s] Write() failed with error: %s)"), _strLogHint.c_str(), errstr.c_str());

				return false;
			}

			_subFiles[i].cacheCurLength = 0;
		}		
	}

	return true;
}

void NasFsTarget::Close()
{
	if (_bCleanup)
		return;

	flushCacheBuffer();

	if(!_bPacing)
	{
		int i;
		for(i=0;i<4;i++)
		{
			if (i == 1)
				continue;

			if(_subFiles[i].hOutputFile == INVALID_HANDLE_VALUE)
				continue;

			CloseHandle(_subFiles[i].hOutputFile);
			_subFiles[i].hOutputFile = INVALID_HANDLE_VALUE;
		}

		//
		// index file
		//	
		i = 1;//index file
		if(_ntfsCacheHandle != NULL)
		{
			CloseHandle(_ntfsCacheHandle);
			_ntfsCacheHandle = NULL;
		}

		std::string errmsg;
		std::string filePath = _cachePath + _subFiles[i].strFilename;
		if(!copyFileToNas(filePath, _subFiles[i].strFilename, errmsg))
		{
			MOLOG(Log::L_ERROR, CLOGFMT(NasFsIO, "[%s] failed to copy file from NTFS to Nas with error: %s"), 
				_strLogHint.c_str(), _subFiles[i].strFilename.c_str(), errmsg.c_str());
		}
		
		MOLOG(Log::L_INFO, CLOGFMT(NasFsIO, "[%s] delete local temp file %s"), 
			_strLogHint.c_str(), filePath.c_str());
		DeleteFileA(filePath.c_str());		
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
				MOLOG(Log::L_INFO, CLOGFMT(NasFsIO, "[%s][%d] sample queue size is %d while close"), 
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
						MOLOG(Log::L_WARNING, CLOGFMT(NasFsIO, "[%s] PacedIndexGetPacedFileMD5() (%s) failed with error %s"), _strLogHint.c_str() 
							, subFile.strFilename.c_str(), DecodePacedIndexError(paceresult));
					}
					else
					{
						MOLOG(Log::L_INFO, CLOGFMT(NasFsIO, "[%s] PacedIndexGetPacedFileMD5() (%s) return [%s]"), _strLogHint.c_str(), 
							subFile.strFilename.c_str(), md5);
					}
				}

				MOLOG(Log::L_INFO, CLOGFMT(NasFsIO, "[%s] to PacedIndexRemove() (%s) file context"),
					_strLogHint.c_str(), subFile.strFilename.c_str());
				paceresult = PacedIndexRemove(subFile.pacingIndexCtx);
				if (paceresult)
				{
					MOLOG(Log::L_WARNING, CLOGFMT(NasFsIO, "[%s] PacedIndexRemove() (%s) failed with error %s"), _strLogHint.c_str() 
						, subFile.strFilename.c_str(), DecodePacedIndexError(paceresult));
				}
				else
				{
					MOLOG(Log::L_INFO, CLOGFMT(NasFsIO, "[%s] PacedIndexRemove (%s) file context successful"),
						_strLogHint.c_str(), subFile.strFilename.c_str());
				}
				subFile.pacingIndexCtx = NULL;				
			}

			MOLOG(Log::L_INFO, CLOGFMT(NasFsIO, "[%s] to Close file (%s)"),
				_strLogHint.c_str(), subFile.strFilename.c_str());
			CloseHandle(subFile.hOutputFile);
			subFile.hOutputFile = INVALID_HANDLE_VALUE;
			MOLOG(Log::L_INFO, CLOGFMT(NasFsIO, "[%s] Close file (%s) successful"),
				_strLogHint.c_str(), subFile.strFilename.c_str());
		}

		if(_ntfsCacheHandle != NULL)
		{
			CloseHandle(_ntfsCacheHandle);
			_ntfsCacheHandle = NULL;
		}

		std::string errmsg;
		std::string filePath = _cachePath + _subFiles[3].strFilename;
		MOLOG(Log::L_INFO, CLOGFMT(NasFsIO, "[%s] delete local temp file %s"), 
			_strLogHint.c_str(), filePath.c_str());
		DeleteFileA(filePath.c_str());
	}

	
	// delete error files
	if(_bDeleteOnFail && GetGraph()->IsErrorOccurred())
	{
		delOutput();
	}

	_bCleanup = true;
	MOLOG(Log::L_INFO, CLOGFMT(NasFsIO, "[%s] closed"), _strLogHint.c_str());
}

void NasFsTarget::endOfStream()
{
	GetGraph()->Finish();
}

const char* NasFsTarget::GetName()
{
	return TARGET_TYPE_NAS;
}

LONGLONG NasFsTarget::getProcessBytes()
{
	return _subFiles[0].llProcBytes;
}

bool NasFsTarget::Receive(MediaSample* pSample, int nInputIndex)
{
	if (nInputIndex < 0 || nInputIndex >3)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(NasFsIO, "[%s] Receive() invalid input index: %d"), _strLogHint.c_str(), nInputIndex);
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
			SubFile& subFile = _subFiles[nInputIndex];
			
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
					bool bFound = false;
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
							MOLOG(Log::L_DEBUG, CLOGFMT(NasFsIO, "[%s] sample queue size reduced to %d"), 
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
					MOLOG(Log::L_ERROR, CLOGFMT(NasFsIO, "[%s] exceeded max queue samples %d, seek too long, stop process"), 
						subFile.strFilename.c_str(), subFile.samples.size());
					SetLastError(std::string("exceeded max queue samples, seek too long"), ERRCODE_BUFFERQUEUE_FULL);
					return false;
				}
				else
				{
					subFile.samples.push_back(pSample);
					MOLOG(Log::L_DEBUG, CLOGFMT(NasFsIO, "[%s] sample queue size increased to %d"), 
						subFile.strFilename.c_str(), subFile.samples.size());
					return true;
				}
			}
		}
		else
		{
			if (!writeNasData(nInputIndex, (char*)pointer, dwActualLen))
				return false;
		}

	}

	GetGraph()->freeMediaSample(pSample);
	return true;
}



/// set cache path
void NasFsTarget::setCacheDirectory(const std::string& path) 
{ 
	_cachePath = path; 
	if(_cachePath == "")
		return;
	
	int pos = _cachePath.length() - 1;
	if(_cachePath[pos] != '\\' && _cachePath[pos] != '/' )
	{
		_cachePath = _cachePath + "\\";
	}

	MOLOG(Log::L_INFO, CLOGFMT(NasFsIO, "[%s] set CacheDirectory=%s"), _strLogHint.c_str(), _cachePath.c_str());
}

bool NasFsTarget::writeNasData(int nIndex, char* pBuf, unsigned int nLen)
{
	SubFile& subFile = _subFiles[nIndex];

	unsigned long amountWritten;
	if(subFile.cacheCurLength + nLen < NAS_IO_SIZE)
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
			totalCopiedBytes = NAS_IO_SIZE - subFile.cacheCurLength;
			memcpy(&subFile.cacheBuffer[subFile.cacheCurLength], pBuf, totalCopiedBytes);
			dwLeftBytes -= totalCopiedBytes;

			//write
			if(!WriteFile(_subFiles[nIndex].hOutputFile, _subFiles[nIndex].cacheBuffer, NAS_IO_SIZE, &amountWritten, NULL))
			{
				std::string errstr;
				getSystemErrorText(errstr);

				SetLastError(std::string("Nas failed to write file ") + _subFiles[nIndex].strFilename + " with error:" + errstr);
				MOLOG(Log::L_ERROR, CLOGFMT(NasFsIO, "[%s] Write() failed with error: %s)"), _strLogHint.c_str(), errstr.c_str());

				return false;
			}
			
			subFile.cacheCurLength = 0;
		}

		while (dwLeftBytes>=NAS_IO_SIZE)
		{
			//write directly
			if(!WriteFile(_subFiles[nIndex].hOutputFile, (char*)pBuf+totalCopiedBytes, NAS_IO_SIZE, &amountWritten, NULL))
			{
				std::string errstr;
				getSystemErrorText(errstr);

				SetLastError(std::string("Nas failed to write file ") + _subFiles[nIndex].strFilename + " with error:" + errstr);
				MOLOG(Log::L_ERROR, CLOGFMT(NasFsIO, "[%s] Write() failed with error: %s)"), _strLogHint.c_str(), errstr.c_str());

				return false;
			}


			dwLeftBytes -= NAS_IO_SIZE;
			totalCopiedBytes += NAS_IO_SIZE;
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

bool NasFsTarget::writeIndexData(char* pBuf, unsigned int nLen, unsigned int uOffetLow, unsigned int uOffetHigh)
{
	DWORD amountWritten = 0;
	DWORD dwPtr = SetFilePointer(_ntfsCacheHandle, uOffetLow, (long*)&uOffetHigh, FILE_BEGIN);
	if (dwPtr == INVALID_SET_FILE_POINTER) // Test for failure
	{ 
		// Obtain the error code. 
		std::string strErr;
		getSystemErrorText(strErr);

		SetLastError(strErr, ERRCODE_NTFS_WRITEFILE);
		MOLOG(Log::L_ERROR, CLOGFMT(NasFsIO, "[%s] Failed to SetFilePointer to offset Low[%d], High[%d], error: %s"),
			_strLogHint.c_str(), uOffetLow, uOffetHigh, strErr.c_str());
		
		return false;
	}

	if(!WriteFile(_ntfsCacheHandle, pBuf, nLen, &amountWritten, NULL))
	{
		std::string errstr;
		getSystemErrorText(errstr);		
		SetLastError(std::string("failed to write file ") + _subFiles[3].strFilename + " on local with error:" + errstr, ERRCODE_NTFS_WRITEFILE);
		MOLOG(Log::L_ERROR, CLOGFMT(NasFsIO, "[%s] Failed to create write file: %s with error: %s"),
			_strLogHint.c_str(), _subFiles[3].strFilename.c_str(), errstr.c_str());

		return false;
	}

	return true;
}



void NasFsTarget::delOutput()
{
	for(int i=0;i<4;i++)
	{

		if(_subFiles[i].strFilename.empty())
			continue;
		
		// delete the file
		if(DeleteFile((_filePath+_subFiles[i].strFilename).c_str()))
		{
			MOLOG(Log::L_INFO, CLOGFMT(NasFsIO, "[%s] file %s was successfully deleted"), _strLogHint.c_str(), (_filePath+_subFiles[i].strFilename).c_str());
			continue;
		}
		
		std::string strErr;
		getSystemErrorText(strErr);
		MOLOG(Log::L_WARNING, CLOGFMT(NasFsIO, "[%s] failed to delete file %s with error: %s"), _strLogHint.c_str(), (_filePath+_subFiles[i].strFilename).c_str(),
			strErr.c_str());
	}
}

bool NasFsTarget::copyFileToNas(std::string sourceFile, std::string desFile, std::string& errmsg)
{	
	HANDLE FileHandle;
	// create Vstrm file handle

	FileHandle = CreateFile(
		(char*)desFile.c_str(), 
		GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		0,
		CREATE_ALWAYS,			
		0,
		0);

	if(FileHandle==INVALID_HANDLE_VALUE)
	{
		std::string errstr;
		getSystemErrorText(errstr);

		char msg[256];
		sprintf(msg, "CreateFile() failed with error: %s", errstr.c_str());
		errmsg = msg;
		return false;
	}
	
	DWORD dwBytesRead = 0;
	DWORD amountWritten = 0;	
	char buffer[NAS_IO_SIZE];
	
	HANDLE ntfsFileHandle = ::CreateFileA(sourceFile.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	
	bool succeed = true;
	if(ntfsFileHandle != INVALID_HANDLE_VALUE)
	{
		while(ReadFile(ntfsFileHandle, (LPVOID)buffer, NAS_IO_SIZE, &dwBytesRead, NULL) && dwBytesRead > 0)
		{
			if(!WriteFile(FileHandle, buffer, dwBytesRead, &amountWritten, NULL))
			{
				std::string errstr;
				getSystemErrorText(errstr);

				errmsg = "Nas failed to write file %s " + desFile + " with error:" + errstr;
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
	
	CloseHandle(FileHandle);
	
	return succeed;
}

void NasFsTarget::setFilename(const char* szFile)
{
	_subFiles[0].strFilename = szFile;
	_subFiles[1].strFilename = std::string(szFile) + ".vvx";
	_subFiles[2].strFilename = std::string(szFile) + ".ff";
	_subFiles[3].strFilename = std::string(szFile) + ".fr";

	MOLOG(Log::L_INFO, CLOGFMT(NasFsIO, "[%s] setFilename %s"), _strLogHint.c_str(), (_subFiles[0].strFilename).c_str());
}

NasFsTarget::~NasFsTarget()
{


}

void NasFsTarget::setBandwidth(unsigned int uBandwidthBps)
{
	_subFiles[0].reservedBW = uBandwidthBps;
	_subFiles[1].reservedBW = uBandwidthBps/20;
	_subFiles[2].reservedBW = uBandwidthBps/6;
	_subFiles[3].reservedBW = uBandwidthBps/6;

	MOLOG(Log::L_INFO, CLOGFMT(NasFsIO, "[%s] set Bandwidth=%d"), _strLogHint.c_str(), uBandwidthBps);
}



void NasFsTarget::pacingAppReportOffsets(const void * const pCbParam, const LONGLONG offset1, const LONGLONG offset2)
{
	UNREFERENCED_PARAMETER(pCbParam);
	UNREFERENCED_PARAMETER(offset1);
	UNREFERENCED_PARAMETER(offset2);
}


#define XX(a,b) {a, b}
const char* NasFsTarget::DecodePacedIndexError(const unsigned long err)
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

void NasFsTarget::pacingAppLogCbk(const char * const pMsg)
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

int NasFsTarget::pacingAppWrite(const void * const pCbParam, const int len, const char buf[])
{
	SubFile* subfile = (SubFile*) pCbParam;
	NasFsTarget* pThis = subfile->pThis;

	bool ret;
	DWORD amountWritten;
	//
	// pacing index module has make the buffer to 64k, just write it
	//
	if (subfile->bIndexFile)
	{
		//write
		ret = WriteFile(subfile->hOutputFile, (char*)buf,  len, &amountWritten, NULL);
		if (!ret)
		{
			std::string errstr;
			getSystemErrorText(errstr);
				
			pThis->SetLastError(std::string("Nas failed to write file ") + subfile->strFilename + " with error:" + errstr);
			SMOLOG(Log::L_ERROR, CLOGFMT(NasFsIO, "[%s] Write() failed with error: %s)"), pThis->_strLogHint.c_str(), errstr.c_str());
			
			return -1;
		}

#ifdef _DEBUG
		SMOLOG(Log::L_DEBUG, CLOGFMT(NasFsIO, "[%s] pacingAppWrite() bytes %d"), subfile->strFilename.c_str(), len);
#endif
		return len;
	}

	if(subfile->cacheCurLength + len < NAS_IO_SIZE)
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
			totalCopiedBytes = NAS_IO_SIZE - subfile->cacheCurLength;
			memcpy(&subfile->cacheBuffer[subfile->cacheCurLength], buf, totalCopiedBytes);
			dwLeftBytes -= totalCopiedBytes;
			
			//write
			ret = WriteFile(subfile->hOutputFile, subfile->cacheBuffer, NAS_IO_SIZE, &amountWritten, NULL);
			if (!ret)
			{
				std::string errstr;
				getSystemErrorText(errstr);

				pThis->SetLastError(std::string("Nas failed to write file ") + subfile->strFilename + " with error:" + errstr);
				SMOLOG(Log::L_ERROR, CLOGFMT(NasFsIO, "[%s] Write() failed with error: %s)"), pThis->_strLogHint.c_str(), errstr.c_str());

				return -1;			
			
			}

			subfile->cacheCurLength = 0;
		}
		
		while (dwLeftBytes>=NAS_IO_SIZE)
		{
			//write directly
			ret = WriteFile(subfile->hOutputFile, (char*)(buf+totalCopiedBytes), NAS_IO_SIZE, &amountWritten, NULL);
			if (!ret)
			{
				std::string errstr;
				getSystemErrorText(errstr);

				pThis->SetLastError(std::string("Nas failed to write file ") + subfile->strFilename + " with error:" + errstr);
				SMOLOG(Log::L_ERROR, CLOGFMT(NasFsIO, "[%s] Write() failed with error: %s)"), pThis->_strLogHint.c_str(), errstr.c_str());

				return -1;	
			}
			
			dwLeftBytes -= NAS_IO_SIZE;
			totalCopiedBytes += NAS_IO_SIZE;
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

int NasFsTarget::pacingAppSeek(const void * const pCbParam, const LONGLONG offset)
{
	SubFile* subfile = (SubFile*) pCbParam;
	NasFsTarget* pThis = subfile->pThis;
	
#ifdef _DEBUG
	SMOLOG(Log::L_DEBUG, CLOGFMT(NasFsIO, "[%s] pacingAppSeek() offset %lld%"), subfile->strFilename.c_str(), offset);
#endif
	
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
			pThis->SetLastError(std::string("Failed to SetFilePointer to offset")+strErr, ERRCODE_PACING_ERROR);
			SMOLOG(Log::L_ERROR, CLOGFMT(NasFsIO, "[%s] Failed to SetFilePointer to offset Low[%d], High[%d], error: %s"),
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

int NasFsTarget::pacingAppSetEOF(const void * const pCbParam, const LONGLONG offset)
{
	SubFile* subfile = (SubFile*) pCbParam;
	NasFsTarget* pThis = subfile->pThis;

	LARGE_INTEGER	tmp;
	tmp.QuadPart = offset;		
	
	SMOLOG(Log::L_DEBUG, CLOGFMT(NasFsIO, "[%s] pacingAppSetEOF() offset %lld%"), subfile->strFilename.c_str(), offset);
	DWORD dwPtr = SetFilePointer(subfile->hOutputFile, tmp.LowPart, &tmp.HighPart, FILE_BEGIN);
	if (dwPtr == INVALID_SET_FILE_POINTER) // Test for failure
	{ 
		// Obtain the error code. 
		std::string strErr;
		getSystemErrorText(strErr);
		pThis->SetLastError(std::string("Failed to SetFilePointer to offset")+strErr, ERRCODE_PACING_ERROR);
		SMOLOG(Log::L_ERROR, CLOGFMT(NasFsIO, "[%s] Failed to SetFilePointer to offset Low[%d], High[%d], error: %s"),
			pThis->_strLogHint.c_str(), tmp.LowPart, tmp.HighPart, strErr.c_str());

		return -1;
	}

	if (!SetEndOfFile(subfile->hOutputFile))
		return -1;


	return 0;
}

bool NasFsTarget::pacingWriteIndex()
{
	// move file point for reading
#ifdef _DEBUG
	MOLOG(Log::L_DEBUG, CLOGFMT(NasFsIO, "[%s] Set index reader position: %lld"), _strLogHint.c_str(), _posCacheRead.QuadPart);
#endif	
	if(INVALID_SET_FILE_POINTER == SetFilePointer(_ntfsCacheHandle, _posCacheRead.LowPart, &_posCacheRead.HighPart, FILE_BEGIN))
	{
		return false;
	}
	
	DWORD byteRead = 0;
	if(!ReadFile(_ntfsCacheHandle, _tmpbuffer, NAS_IO_SIZE, &byteRead, NULL))
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
 	MOLOG(Log::L_DEBUG, CLOGFMT(NasFsIO, "[%s] Save index reader position: %lld"), 
 		_strLogHint.c_str(), _posCacheRead.QuadPart);	
#endif
	
	if(0 == byteRead)
		return true;
	
	// log the last index read
	VVX_V7_RECORD *pTmp = 
		(VVX_V7_RECORD *)(_tmpbuffer + byteRead - sizeof(VVX_V7_RECORD));
	
#ifdef _DEBUG
	MOLOG(Log::L_DEBUG, CLOGFMT(NasFsIO, "[%s] last index read: %#I64x (%#I64x)(%#I64x), write bytes: %d"),
		_strLogHint.c_str(),
		pTmp->trick.frameByteOffset[0],
		pTmp->trick.frameByteOffset[1],
		pTmp->trick.frameByteOffset[2],
		byteRead);
#endif
	
	// pass the buffer to pacing
	DWORD paceresult = 0;
	try
	{
		if(PacedIndexWrite(_subFiles[3].pacingIndexCtx, byteRead, _tmpbuffer, &paceresult) < 0)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(NasFsIO, "[%s] PacedIndexWrite(%s) failed with error"), 
				_strLogHint.c_str(), DecodePacedIndexError(paceresult));
			return false;
		}
	}
	catch(...)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(NasFsIO, "[%s] PacedIndexWrite(%s) met unknown exception"), 
			_strLogHint.c_str());
		
		return false;
	}

	return true;
}

bool NasFsTarget::pacingWriteData(int nIndex, char* pBuf, unsigned int nLen)
{
	SubFile& subFile = _subFiles[nIndex];

	//write
	DWORD paceresult = 0;

	try
	{
		if(PacedIndexWrite(subFile.pacingIndexCtx, nLen, pBuf, &paceresult) < 0)
		{
			std::string errstr = DecodePacedIndexError(paceresult);
			//			the error information has been set on vswrite failure
			//			SetLastError(std::string("PacedIndexWrite failed to write file ") + subFile.strFilename + " with error:" + errstr);
			MOLOG(Log::L_ERROR, CLOGFMT(NasFsIO, "[%s] PacedIndexWrite() file %s with error: (%s)"), 
				_strLogHint.c_str(), subFile.strFilename.c_str(), errstr.c_str());
			return false;
		}
	}
	catch(...)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(NasFsIO, "[%s] PacedIndexWrite() file [%s] len[%d] caught unknown exception"), 
			_strLogHint.c_str(), subFile.strFilename.c_str(), nLen);

		SetLastError(std::string("PacedIndexWrite() caught unknown exception"), ERRCODE_PACING_UNKNOWN_EXCEPTION);		
		return false;
	}	

	return true;
}

void NasFsTarget::setFilePath(const char* szFilePath)
{
	_filePath = szFilePath;
	if (szFilePath[strlen(szFilePath)-1] != '\\')
	{
		_filePath += "\\";
	}
	MOLOG(Log::L_INFO, CLOGFMT(NasFsIO, "[%s] file path is %s"), _strLogHint.c_str(),_filePath.c_str());
}

void NasFsTarget::enableMD5(bool bEnable)
{
	_bEnableMD5 = bEnable;
}

void NasFsTarget::getMD5(std::string& strMD5)
{
	strMD5 = _md5ChecksumUtil.lastChecksum();
}

void NasFsTarget::getSupportFileSize(LONGLONG& supportFilesize)
{
	LARGE_INTEGER integer;

	for (int i = 1; i < 3; i++)
	{
		if (_subFiles[i].hOutputFile == INVALID_HANDLE_VALUE)
		{
			_subFiles[i].hOutputFile = CreateFile(
				(char*)(_filePath+_subFiles[i].strFilename).c_str(), 
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

				MOLOG(Log::L_WARNING, CLOGFMT(NasFsIO, "[%s] Can't get supportFilesize because opening output file: %s with error: %s"),
					_strLogHint.c_str(), (_filePath+_subFiles[i].strFilename).c_str(), errstr.c_str());

				return;
			}


		}
		GetFileSizeEx(_subFiles[i].hOutputFile, &integer);

		supportFilesize += integer.QuadPart;
	}

	MOLOG(Log::L_INFO, CLOGFMT(NasFsIO, "[%s] supportFilesize-[%lld]"),_strLogHint.c_str(),supportFilesize);
	
}


}}