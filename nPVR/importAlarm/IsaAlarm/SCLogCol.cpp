// SCLogCol.cpp: implementation of the SCLogCol class.
//
//////////////////////////////////////////////////////////////////////

#include "SCLogCol.h"
#include "KeyDefine.h"
#include "Log.h"


using namespace ZQ::common;

#define CLOG_FILE_HEAD_LENGTH	10
#define MAX_LOG_LENGTH			4096 // 4k for the buffer
#define ERROR_HANDLE(Handle) (((Handle)==NULL)||((Handle)==INVALID_HANDLE_VALUE))


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SCLogCol::SCLogCol()
{
	_pos = 0;
	_bQuit=true;
}

SCLogCol::~SCLogCol()
{
	close();
}

bool SCLogCol::init(InitInfo& initInfo, const char* szSessionName)
{
	if (!initInfo.getValue(KD_KN_FILENAME, _filename, true, true))
		return false;
	
	HANDLE hFile = CreateFileA(_filename.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	
	if (ERROR_HANDLE(hFile))
	{
		glog(Log::L_ERROR, "Fail to open file %s", _filename.c_str());
		return false;
	}
	
	_pos = getLogPos(hFile);
	::CloseHandle(hFile);

	_bQuit=false;		

	return true;
}

void SCLogCol::close()
{
	if (!_bQuit)
	{
		_bQuit = true;
		waitHandle(INFINITE);
	}	
}

int SCLogCol::run()
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

DWORD SCLogCol::getLogPos(HANDLE hFile)
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

int SCLogCol::nextLogLine(char* buf, char** pline)
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
