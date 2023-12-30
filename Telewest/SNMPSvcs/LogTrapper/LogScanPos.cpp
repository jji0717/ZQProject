// LogScanPos.cpp: implementation of the LogScanPos class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <string>
#include "LogScanPos.h"
#include "Log.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

LogScanPos::LogScanPos()
{
	_nRecordCount = 0;
	
	_nByteOffset = 0;
	_szLastLine[0] = '\0';
	_szFilename[0] = '\0';
}

LogScanPos::~LogScanPos()
{
	unInit();
}

bool LogScanPos::init(const char* szFilename, int nFlashCount)
{
	_nFlashCount = nFlashCount;
	_nRecordCount = 0;
	strncpy(_szFilename, szFilename, sizeof(_szFilename)-1);
	
	_nByteOffset = 0;
	_szLastLine[0] = '\0';

	//read safestore if exist
	FILE* f = fopen(_szFilename, "rb");

	if (!f)
	{
		_bSafeStoreRead=false;
	}
	else
	{
		do
		{
			int nRet = fread(&_nByteOffset, 4, 1, f);
			if (nRet <1)break;

			int nLen;
			nRet = fread(&nLen, 4, 1, f);
			if (nRet <1)break;

			if (nLen>=sizeof(_szLastLine)||!nLen) break;

			nRet = fread(_szLastLine, nLen, 1, f);
			if (nRet <1)break;

			_szLastLine[nLen] = '\0';

			//check if there is a timestamp on this line(11/22 11:15:15:296)
			int nMonth,nDay,nHour,nMinute,nSecond,nMilliSec;
			int nRead = sscanf(_szLastLine, "%2d/%2d %2d:%2d:%2d:%3d", &nMonth,&nDay,&nHour,&nMinute,&nSecond,&nMilliSec);
			if (nRead!=6)
			{
				nLen=0;
				_szLastLine[0]='\0';
				break;
			}

			_bSafeStoreRead = true;
		}while(0);
		fclose(f);
		f=NULL;

		if (!_bSafeStoreRead)
			DeleteFileA(_szFilename);
	}

	return true;
}

void LogScanPos::unInit()
{
	if (_nRecordCount)
	{
		//write to disk
		FILE* f=fopen(_szFilename, "wb");
		if (f)
		{
			fwrite(&_nByteOffset, 4, 1, f);
			int nLen = strlen(_szLastLine);
			fwrite(&nLen, 4, 1, f);
			fwrite(_szLastLine, nLen, 1, f);
			fclose(f);
			f=NULL;
		}
		else
		{
			//log
			glog(ZQ::common::Log::L_ERROR, "Cann't write safestore file %s", _szFilename);
		}

		_nRecordCount = 0;
	}
}

bool LogScanPos::getSafeStoreScanPos(int& nByteOffset, std::string& strLastLine)
{
	if (!_bSafeStoreRead)
		return false;

	if (!_szLastLine[0])
		return false;

	nByteOffset = _nByteOffset;
	strLastLine = _szLastLine;

	return true;
}

void LogScanPos::setScanPos(int nByteOffset, const char* strLine, bool bForceFlash)
{
	_nRecordCount++;

	_nByteOffset = nByteOffset;
	strncpy(_szLastLine, strLine, sizeof(_szLastLine)-1);

	if (bForceFlash || _nRecordCount>= _nFlashCount)
	{
		//write to disk
		FILE* f=fopen(_szFilename, "wb");
		if (f)
		{
			fwrite(&nByteOffset, 4, 1, f);
			int nLen = strlen(_szLastLine);
			fwrite(&nLen, 4, 1, f);
			fwrite(_szLastLine, nLen, 1, f);
			fclose(f);
			f=NULL;
		}
		else
		{
			//log
			glog(ZQ::common::Log::L_ERROR, "Cann't write safestore file %s", _szFilename);
		}

		_nRecordCount = 0;
	}
}
//
//int main(int argc, char* argv[])
//{
//	LogScanPos  xx;
//	bool bRet = xx.init("c:\\11.dat", 3);
//
//	int nByteOffet;
//	std::string strLastLine;
//	bRet = xx.getSafeStoreScanPos(nByteOffet, strLastLine);
//
//	xx.setScanPos(5, "11111");
//	xx.setScanPos(6, "12111");
//	xx.setScanPos(7, "12411");
//	xx.setScanPos(8, "12151");
//	xx.setScanPos(9, "12116");
//	xx.setScanPos(10, "12118");
//	xx.setScanPos(12, "12158");
//
//	return 0;
//}

