
#include "readline.h"

#include <stdio.h>

#define FILE_READ_ERROR -1
#define FILE_READ_END -2
#define FILE_READ_NONE 0
#define FILE_READ_RANGE -3
#define FILE_READ_OK 1


int ReadLine(HANDLE hFile, char* strLine, size_t zLen)
{
	memset(strLine, 0, zLen);
	DWORD dwReadCount = 0;
	if (FALSE == ReadFile(hFile, strLine, zLen-1, &dwReadCount, NULL))
	{
		return FILE_READ_ERROR;
	}
	if (0 == dwReadCount)
	{
		return FILE_READ_END;
	}
	for (DWORD i = 0; i < dwReadCount-1; ++i)
	{
		if (strLine[i] == char(13)&& strLine[i+1] == char(10))
		{
			strLine[i] = char(0);
			SetFilePointer(hFile, int(i)+2-int(dwReadCount), 0, FILE_CURRENT);
			break;
		}
	}
	if (i == 0)
		return 0;
	return int(i-1);
}

bool BeFileEnd(HANDLE hFile)
{
	DWORD dwCur = SetFilePointer(hFile, 0, 0, FILE_CURRENT);
	DWORD dwEnd = SetFilePointer(hFile, 0, 0, FILE_END);
	if (dwCur == dwEnd)
	{
		return true;
	}
	SetFilePointer(hFile, dwCur, 0, FILE_BEGIN);
	return false;
}

void GoFileBegin(HANDLE hFile)
{
	SetFilePointer(hFile, SKIP_HEAD_CHARS+2, 0, FILE_BEGIN);
}

int GetLogEnd(HANDLE hFile)
{
	DWORD dwCur = SetFilePointer(hFile, 0, 0, FILE_CURRENT);
	char szBuf[SKIP_HEAD_CHARS+1] = {0};
	DWORD dwNumBytesRead = 0;
	DWORD dwCircpos = 0;
	SetFilePointer(hFile, 0, 0, FILE_BEGIN);
	if (ReadFile(hFile,&szBuf,SKIP_HEAD_CHARS,&dwNumBytesRead,NULL))
	{
		SetFilePointer(hFile, dwCur, 0, FILE_BEGIN);
		sscanf(szBuf, "%ul",&dwCircpos);
		return dwCircpos;
	}
	return -1;
}

bool GoLogBegin(HANDLE hFile)
{
	int nLogEnd = GetLogEnd(hFile);
	if (-1 == nLogEnd)
	{
		return false;
	}
	SetFilePointer(hFile, nLogEnd, 0, FILE_BEGIN);
	return true;
}

bool BeLogEnd(HANDLE hFile)
{
	int nLogEnd = GetLogEnd(hFile);
	if (-1 == nLogEnd)
	{
		return true;
	}
	int nCurPoint = SetFilePointer(hFile, 0, 0, FILE_CURRENT);
	return nLogEnd == nCurPoint;
}

int ReadFileLine(HANDLE hFile, char* strLine, size_t zLen)
{
	if (BeFileEnd(hFile))
	{
		GoFileBegin(hFile);
	}
	return ReadLine(hFile, strLine, zLen);
}

int ReadLogLine(HANDLE hFile, char* strLine, size_t zLen, bool bStart)
{
	if (bStart)
		return ReadFileLine(hFile, strLine, zLen);
	if (BeLogEnd(hFile))
		return -1;
	return ReadFileLine(hFile, strLine, zLen);
}

bool GoFirstLogLine(HANDLE hFile)
{
	int nLogEnd = GetLogEnd(hFile);
	if (-1 == nLogEnd)
	{
		return false;
	}

	DWORD dwEnd = SetFilePointer(hFile, 0, 0, FILE_END);
	if((DWORD)nLogEnd < dwEnd)
	{
		DWORD dwEnd = SetFilePointer(hFile, nLogEnd, 0, FILE_BEGIN);
		char strLine[1024];
		int zLen = sizeof(strLine);
		ReadLine(hFile, strLine, zLen);		//skip several lines
		ReadLine(hFile, strLine, zLen);
		ReadLine(hFile, strLine, zLen);
	}
	else
	{
		GoFileBegin(hFile);		//from head
	}
	
	return false;
}
