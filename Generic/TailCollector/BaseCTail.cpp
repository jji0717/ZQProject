// ============================================================================================
// Copyright (c) 1997, 1998 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved. Unpublished rights reserved under the copyright laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the confidential
// technology of ZQ Interactive, Inc. Possession, use, duplication or dissemination of the
// software and media is authorized only pursuant to a valid written license from ZQ Interactive,
// Inc.
// This source was copied from shcxx, shcxx's copyright is belong to Hui Shao
//
// This software is furnished under a  license  and  may  be used and copied only in accordance
// with the terms of  such license and with the inclusion of the above copyright notice.  This
// software or any other copies thereof may not be provided or otherwise made available to any
// other person.  No title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and should not be
// construed as a commitment by ZQ Interactive, Inc.
// --------------------------------------------------------------------------------------------
// Author: Hui Shao
// Desc  : ctail implement for clog messages
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/Generic/TailCollector/BaseCTail.cpp 1     10-11-12 15:59 Admin $
// $Log: /ZQProjs/Generic/TailCollector/BaseCTail.cpp $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:31 Admin
// Created.
// 
// 2     8/23/05 1:38p Hui.shao
// 
// 1     6/24/05 2:34p Hui.shao
// ============================================================================================

#include "BaseCTail.h"

#define CLOG_FILE_HEAD_LENGTH	10
#define MAX_LOG_LENGTH			2048 // 2k for the buffer
#define ERROR_HANDLE(Handle) (((Handle)==NULL)||((Handle)==INVALID_HANDLE_VALUE))

// -----------------------------
// class BaseCTail
// -----------------------------
BaseCTail::BaseCTail(const char* filename, const bool checkIfFileExist)
: _bQuit(false), _pos(-1), _bCheckIfExist(checkIfFileExist)
{
	_filename = filename? filename :"";
}

void BaseCTail::setFilename(const char* filename)
{
	if (filename != NULL && strlen(filename)>0)
		_filename = filename;
}

const char* BaseCTail::getFilename()
{
	return _filename.c_str();
}

bool BaseCTail::init(void)
{
	// check if the file could be opened
	if (!_bCheckIfExist)
		return true;

	HANDLE hFile = CreateFileA(_filename.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	
	if (!ERROR_HANDLE(hFile))
	{
		_pos = getLogPos(hFile);
		::CloseHandle(hFile);
		return true;
	}
	
	return false;
}

int BaseCTail::run(void)
{
	while (!_bQuit)
	{
		HANDLE hFile = CreateFileA(_filename.c_str(),	GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE,	NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		
		if (ERROR_HANDLE(hFile))
		{
			// failed to open the specified log file
			for (int i=0; i< 6 && !_bQuit; i++)
				::Sleep(500);
			continue;
		}

		DWORD logpos = getLogPos(hFile);
		if (_pos <=0)
			_pos = logpos;
		
		int diff = logpos - _pos;
		diff = diff <0 ? -diff: diff;
		
		if (diff <= 2 || logpos <=CLOG_FILE_HEAD_LENGTH)
		{
			// no more messages logged since last scan
			::CloseHandle(hFile);
			for (int i=0; i< 2 && !_bQuit; i++)
				::Sleep(500);
			continue;
		}
		
		char logmsgbuf[MAX_LOG_LENGTH] = "";
		DWORD nbyte =0;
		
		if (_pos > logpos)
		{
			// log has been rolled since last read
			::SetFilePointer(hFile, _pos, 0, FILE_BEGIN);
			
			while (!_bQuit && ReadFile(hFile, logmsgbuf, MAX_LOG_LENGTH-1, &nbyte, NULL) && nbyte>0)
			{
				logmsgbuf[nbyte] = '\0';
				
				char* line = NULL;
				char* buf  = logmsgbuf;
				int nByteStepped = 0;
				
				while ((nByteStepped = nextLogLine(buf, &line)) >0)
				{
					buf += nByteStepped;
					_pos += nByteStepped;
					
					if (_bQuit || NULL == line)
						break;
					
					OnNewMessage(line);
				}
			}
			
			_pos = CLOG_FILE_HEAD_LENGTH; // start from the beginning
		}
		
		::SetFilePointer(hFile, _pos, 0, FILE_BEGIN);
		while (!_bQuit && (_pos < logpos) && ReadFile(hFile, logmsgbuf, MAX_LOG_LENGTH-1, &nbyte, NULL) && nbyte>0)
		{
			logmsgbuf[nbyte] = '\0';
			
			char* line = NULL;
			char* buf  = logmsgbuf;
			int nByteStepped = 0;
			
			while ((_pos < logpos) && (nByteStepped = nextLogLine(buf, &line)) >0)
			{
				buf += nByteStepped;
				_pos += nByteStepped;
				
				if (_bQuit || NULL == line)
					break;
				
				OnNewMessage(line);
			}
			::SetFilePointer(hFile, _pos, 0, FILE_BEGIN);
		}
		
		::CloseHandle(hFile);
		
	} // !bQuit
	
	return 0;
}

DWORD BaseCTail::getLogPos(HANDLE hFile)
{
	if (ERROR_HANDLE(hFile))
		return 0;
	
	DWORD crnt_pos = ::SetFilePointer(hFile, 0, 0, FILE_CURRENT);
	char buf[CLOG_FILE_HEAD_LENGTH+10] = "";
	DWORD nbyte =0;
	
	::SetFilePointer(hFile, 0, 0, FILE_BEGIN);
	
	DWORD pos = 0;
	if (::ReadFile(hFile,&buf,CLOG_FILE_HEAD_LENGTH,&nbyte,NULL) &&  nbyte==CLOG_FILE_HEAD_LENGTH)
		pos = ::atol(buf);
	
	::SetFilePointer(hFile, crnt_pos, 0, FILE_BEGIN);
	
	return pos;
}

int BaseCTail::nextLogLine(char* buf, char** pline)
{
	if (buf == NULL)
		return NULL;
	
	*pline = buf;
	bool bValidLine = false;
	
	while (**pline == '\r' || **pline =='\n')
		(*pline)++;
	
	char* q = *pline;
	while (*q != '\r' && *q!= '\n' && *q!='\0')
		q++;
	
	while (*q == '\r' || *q == '\n')
	{
		*q++ = '\0';
		bValidLine = true;
	}
	
	if (bValidLine)
		return (q - buf); // found a valid line
	
	// this is a incompleted line
	int stepped = *pline - buf;
	*pline = NULL;
	return stepped;
}
