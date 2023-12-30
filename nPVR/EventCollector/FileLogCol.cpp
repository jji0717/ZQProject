// FileLogCol.cpp: implementation of the FileLogCol class.
//
//////////////////////////////////////////////////////////////////////

#include "FileLogCol.h"
#include "Log.h"


using namespace ZQ::common;

#define CLOG_FILE_HEAD_LENGTH	10
#define MAX_LOG_LENGTH			8192 // 8k for the buffer
#define ERROR_HANDLE(Handle) (((Handle)==NULL)||((Handle)==INVALID_HANDLE_VALUE))

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FileLogCol::FileLogCol()
{
	_pos = 0;
	_bQuit=true;
}

FileLogCol::~FileLogCol()
{
	close();
}

bool FileLogCol::init(InitInfo& initInfo, const char* szSessionName)
{
	if (!initInfo.getValue(KD_KN_FILENAME, _filename, true, true))
		return false;

    // convert any environment variables
    if (strstr(_filename.c_str(), "%") != NULL)
    {
        char szLogName[MAX_PATH];
        if (::ExpandEnvironmentStringsA(_filename.c_str(), szLogName, sizeof(szLogName)))
        {
			_filename = szLogName;
        }
    }

	glog(Log::L_INFO, "FileLogCol for file %s initializing", _filename.c_str());
	
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
	}
	else
	{
		::CloseHandle(hFile);
	}
	
	_pos = 0;
	_bQuit=false;		

	glog(Log::L_INFO, "FileLogCol for file %s initialized", _filename.c_str());

	return true;
}

void FileLogCol::close()
{
	if (!_bQuit)
	{
		_bQuit = true;
		waitHandle(INFINITE);
	}	
}

int FileLogCol::run()
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

		char logmsgbuf[MAX_LOG_LENGTH] = "";
		DWORD nbyte =0;
		
		DWORD dwFileSize = GetFileSize(hFile, 0);
		if (!dwFileSize||_pos == dwFileSize)
		{
			::CloseHandle(hFile);
			for (int i=0; i< 2 && !_bQuit; i++)
				::Sleep(500);
			continue;
		}

		if(_pos > dwFileSize)
		{
			_pos = 0; // file changed, reset to 0			
			::SetFilePointer(hFile, 0, 0, FILE_BEGIN);
		}

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
			
			_pos =::SetFilePointer(hFile, 0, 0, FILE_CURRENT);
		}			
		
		::CloseHandle(hFile);
		
	} // !bQuit
	
	return 0;
}

int FileLogCol::nextLogLine(char* buf, char** pline)
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
