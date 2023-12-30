
#include "sclogtrigger.h"
#include "loghandler.h"
	
TG_BEGIN

inline bool ERROR_HANDLE(HANDLE handle)
{
	return ((INVALID_HANDLE_VALUE == handle)||(NULL == handle));
}
	
bool SCLogTrigger::OnTail(char* logline, size_t length)
{
	HANDLE	hLogFile = NULL;
	if (m_strLogFile.empty())
	{
		Log(LogHandler::L_ERROR, "[SCLogTrigger::OnTail] file name is empty");
		return false;
	}
	
	hLogFile = ::CreateFileA(m_strLogFile.c_str(),
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
	
	if (ERROR_HANDLE(hLogFile))
	{
		Log(LogHandler::L_ERROR, "[SCLogTrigger::OnTail] can not open file, maybe this is opened by other application");
		return false;
	}
	
	if (0 == m_zFilePointer)
	{
		size_t logLength = getLogLength(hLogFile);
		if (0xffffffff == logLength)
			logLength = 0;

		m_zFilePointer = logLength+1;
		::CloseHandle(hLogFile);
		Log(LogHandler::L_INFO, "[SCLogTrigger::OnTail] first read log file");
		return false;//this is not error
	}
	
	::SetFilePointer(hLogFile, m_zFilePointer, 0, FILE_BEGIN);
	memset(logline, 0, length);
	
	int nReadCount = readLogFileLine(hLogFile, logline, length-1);
	if (nReadCount <= 0)
	{
		Log(LogHandler::L_ERROR, "[SCLogTrigger::OnTail] can not read data from file");
		::CloseHandle(hLogFile);
		return false;
	}
		
	m_zFilePointer += (nReadCount+strlen(NEXT_LINE_SIGN));
	::CloseHandle(hLogFile);
	return true;
}

SCLogTrigger::SCLogTrigger(const char* logfile, SyntaxParser& parser, size_t sourceid)
	:Trigger(logfile, parser, sourceid), m_zFilePointer(0)
{
}

unsigned long SCLogTrigger::getLogLength(HANDLE hLogFile)
{
	unsigned long ulOld = ::SetFilePointer(hLogFile, 0, 0, FILE_CURRENT);
	::SetFilePointer(hLogFile, 0, 0, FILE_BEGIN);
	if (0 == ulOld)
	{
		Log(LogHandler::L_WARNING, "[SCLogTrigger::getLogLength] current log position is 0");
	}
	char szBuf[SKIP_HEAD_CHARS+1] = {0};
	DWORD dwNumBytesRead = 0;
	DWORD dwCircpos = 0;
	if (::ReadFile(hLogFile,&szBuf,SKIP_HEAD_CHARS,&dwNumBytesRead,NULL))
	{
		sscanf(szBuf, "%ul",&dwCircpos);
		::SetFilePointer(hLogFile, ulOld, 0, FILE_BEGIN);
		return dwCircpos-1;
	}
	::SetFilePointer(hLogFile, ulOld, 0, FILE_BEGIN);
	return 0xffffffff;
}

int SCLogTrigger::readLine(HANDLE hLogFile, char* strLine, size_t zLen, const char* sign)
{
	memset(strLine, 0, zLen);
	DWORD dwReadCount = 0;
	if (FALSE == ::ReadFile(hLogFile, strLine, zLen-1, &dwReadCount, NULL))
		return -1;
	if (0 == dwReadCount)
		return 0;

	char* nextline = strstr(strLine, sign);
	if (NULL == nextline)
	{
		strLine[0] = 0;
		return dwReadCount;
	}
	nextline[0] = 0;

	int leftstringlen = dwReadCount - (int(nextline-strLine)+strlen(sign));
	::SetFilePointer(hLogFile, -leftstringlen, 0, FILE_CURRENT);

	return int(nextline-strLine);
}

int SCLogTrigger::readLogFileLine(HANDLE hLogFile, char* strLine, size_t zLen)
{
	unsigned long loglen	= getLogLength(hLogFile);
	unsigned long curpos	= ::SetFilePointer(hLogFile, 0, 0, FILE_CURRENT);
	
	if (0xffffffff == loglen)
		return -1;

	if ((loglen == curpos)||(loglen+1 == curpos))
		return 0;
	
	int nRead = readLine(hLogFile, strLine, zLen);
	if (0 == nRead)
	{
		::SetFilePointer(hLogFile, SKIP_HEAD_CHARS+strlen(NEXT_LINE_SIGN), 0, FILE_BEGIN);
		nRead = readLine(hLogFile, strLine, zLen);
	}
	
	return nRead;
}
	
TG_END
	