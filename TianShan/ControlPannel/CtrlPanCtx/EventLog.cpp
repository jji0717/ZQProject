
//////////////////////////////////////////////////////////////////////////
// EventLog.cpp: implementation of the FileLog class.
// Author: copyright (c) Han Guan
//////////////////////////////////////////////////////////////////////

#include "EventLog.h"
#include <iostream>
#include <time.h>
#include <vector>
#include <string>
#include <fstream>
#include <new>

#pragma warning(disable: 4307)
#pragma warning(disable: 4018)

#define	SYSTEMLOG (*m_pSysLog)



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


	
/// -----------------------------
/// class FileLogException
/// -----------------------------
CFileLogException::CFileLogException(const std::string &what_arg) throw()
            :IOException(what_arg)
{
}

CFileLogException::~CFileLogException() throw()
{
}

/// -----------------------------
/// class FileLogNest
/// -----------------------------
class CFileLogNest {
friend class CEventFileLog;
std::ofstream	m_FileStream;			//log�ļ���
};


// ���conString���ұߵ�һ��vChar������ַ�������û��vChar�򷵻ؿ��ַ�����
void GetStringRightAtChar(const ::std::string& contString,char vChar,::std::string &strTemp)
{
	int strLen = contString.size();
	int cur;
	for(cur = strLen-1;cur >= 0;cur--)
	{
		if(contString[cur] == vChar)
			break;
	}
	if(cur < 0)
	{
		strTemp = "";
		return;
	}
	else
	{
		cur++;
		while(cur<strLen)
		{
			strTemp.push_back(contString[cur]);
			cur++;
		}
	}
}


// ���contString���ڵ�һ��vChar��ߵ��ַ���, ��û��vChar�򷵻ؿ��ַ���.
void GetStringLeftAtChar(const ::std::string& contString,char vChar,::std::string &strTemp)
{
	int strLen = contString.size();
	int cur;
	for(cur = strLen-1;cur >= 0;cur--)
	{
		if(contString[cur] == vChar)
			break;
	}
	if(cur < 0)
	{
		strTemp = "";
		return;
	}
	else
	{
		int tempCur=0;
		while(tempCur<cur)
		{
			strTemp.push_back(contString[tempCur]);
			tempCur++;
		}
	}
}

//������;
void GetPathString(const ::std::string& fullName,::std::string &pathName)
{
	int strLen = fullName.size();
	int cur;
	for(cur = strLen-1;cur >= 0; cur--)
	{
		if(fullName[cur] == '\\' || fullName[cur] == '/')
			break;
	}
	if(cur < 0)
	{
		pathName = "";
		return;
	}
	else
	{
		int tempCur=0;
		while(tempCur<=cur)
		{
			pathName.push_back(fullName[tempCur]);
			tempCur++;
		}
	}
}


bool StringIsNumber(::std::string str)
{
	int strLen = str.size();
	if (strlen == 0)
		return false;

	for(int i=0;i<strLen;i++)
	{
		if(str[i]<'\0'||str[i]>'9')
			return false;
	}
	return true;
}

bool hasInvalidChar(std::string str)
{
	int strlen = str.size();
	for(int i=0;i<strlen;i++)
	{
		if(str[i]=='*' || str[i]=='?' || str[i]=='\"' || str[i]=='<' || str[i]=='>' || str[i]=='|')
			return true;
	}
	return false;
}

void makeDirectory(std::string name)
{
	std::vector<std::string> tmpArray;
	std::vector<std::string>::iterator t_itor;
	std::string tmpStr;
	GetPathString(name,tmpStr);
	while (tmpStr.size())
	{
		tmpArray.push_back(tmpStr);
		tmpStr[tmpStr.size()-1] = '\0';
		std::string temp = tmpStr;
		tmpStr = "";
		GetPathString(temp,tmpStr);
	};
	
	int i=0;
	int count = tmpArray.size();
	for (i = count-1; i >= 0; i--)
	{
		CreateDirectory(tmpArray[i].c_str(),NULL);
	}
}

CEventFileLog::CEventFileLog(const char* filename, const int verbosity /*=L_ERROR*/, int logFileNum /*= ZQLOG_DEFAULT_FILENUM*/, int fileSize /*=ZQLOG_DEFAULT_FILESIZE*/, int buffersize /*=ZQLOG_DEFAULT_BUFSIZE*/, int flushInterval /*=ZQLOG_DEFAULT_FLUSHINTERVAL*/)
: Log(verbosity), _pNest(NULL)
{	
	try{
		m_pSysLog = new ZQ::common::SysLog("FileLog");
	}
	catch(...)
	{
		SYSTEMLOG(Log::L_ERROR,"Alloc memory fail!");
		throw CFileLogException("Alloc memory fail!");
	}

	try{
	_pNest = new CFileLogNest();
	}
	catch(...)
	{
		SYSTEMLOG(Log::L_ERROR,"Alloc memory fail!");
		throw CFileLogException("Alloc memory fail!");
	}
	
	_event = NULL;
	
	SYSTEMTIME time;
	GetLocalTime(&time);
	m_currentDay = time.wDay;								//���õ�ǰ���ڣ��Ա㷢�����ڸı�
	
	strcpy(m_FileName,filename);							//��������õ��ļ���
	
	m_nMaxFileSize			= fileSize;						//����log�ļ�size, ��СΪ1MB
	if(m_nMaxFileSize<1024*1024*1)
		m_nMaxFileSize = 1024*1024*1;
	
	m_nMaxBuffSize			= buffersize;					//���û�����size, ��СΪ4KB
	if(m_nMaxBuffSize<1024*4)
		m_nMaxBuffSize = 1024*4;
	
	m_nMaxLogfileNum		= logFileNum;					//����log�ļ�����Ŀ, ����Ϊ1�������Ϊ1000��
	if(m_nMaxLogfileNum>1000)
	{
		m_nMaxLogfileNum = 1000;
	}
	if (m_nMaxLogfileNum<1)
	{
		m_nMaxLogfileNum = 1;
	}
	
	m_nFlushInterval		= flushInterval;				//���ö�ʱ���������е�����д��log�ļ���ʱ����
	if(m_nFlushInterval<1)
		m_nFlushInterval = 1;
	
	m_nCurrentFileSize		= 0;							//���õ�ǰ��log�ļ���С
	m_nCurrentBuffSize		= 0;							//���õ�ǰ��buffer size
	
	m_Buff = new char[m_nMaxBuffSize+1];					//���仺����
	memset(m_Buff,0,m_nMaxBuffSize+1);
	
	std::string pathName;
	GetPathString(filename,pathName);						//���ַ����л��·����
	if(hasInvalidChar(pathName))							//����·����ʱ��Ϸ�
	{
		throw CFileLogException("Invalid directory name");
	}
	
	makeDirectory(filename);

	if(IsFileExsit(filename,m_nCurrentFileSize))
	{
		//����ļ��Ѿ����ڣ����ļ�����ʱ���ļ���С�Ѿ������m_nCurrentFileSize��
		_pNest->m_FileStream.open(filename,std::ios::out||std::ios::app||std::ios::binary);
		if(!_pNest->m_FileStream.is_open())
		{
			SYSTEMLOG(Log::L_ERROR,"Open file fail!");
			throw CFileLogException("Open file fail!");
		}
	}
	else
	{
		//�ļ������ڣ������ļ�
		_pNest->m_FileStream.open(filename,std::ios::out||std::ios::binary);
		if(!_pNest->m_FileStream.is_open())
		{
			SYSTEMLOG(Log::L_ERROR,"Create file fail!");
			throw CFileLogException("Create file fail!");
		}
		m_nCurrentFileSize = 0;
	}

	//�����߳�
	start();
}

CEventFileLog::~CEventFileLog()
{
	stop();
	//���ٻ�����
	if (m_Buff)
		delete []m_Buff;
	if (_pNest)
		delete _pNest;
	if (m_pSysLog)
		delete m_pSysLog;
}

//��buffer ��д�����ݣ�Ҫ��buffer���м���
void CEventFileLog::writeMessage(const char *msg, int level)
{
	//TODO: ����msgΪָ���Ľṹ������ʱ��ͻس����з�
	int nMsgSize = strlen(msg);
	if(nMsgSize == 0)//��Ϣ����Ϊ0�򷵻�
		return;
	SYSTEMTIME time;
	GetLocalTime(&time);

	//������ڷ����ı�
	if(time.wDay != m_currentDay)
	{
		m_currentDay = time.wDay;
		{
			ZQ::common::MutexGuard lk(m_buffMtx);
			flushData();
			RenameAndCreateFile();
		}
	}
	char line[ZQLOG_DEFAULT_MAXLINESIZE];

//	int nCount = _snprintf(line,ZQLOG_DEFAULT_MAXLINESIZE-2,"%02d-%02d %02d:%02d:%02d.%03d [ %5s ] %s",time.wMonth,time.wDay,time.wHour,time.wMinute,time.wSecond,time.wMilliseconds,getVerbosityStr(level),msg);
	int nCount = _snprintf(line,ZQLOG_DEFAULT_MAXLINESIZE-2,"%s",msg);
	if(nCount < 0)
	{
		line[ZQLOG_DEFAULT_MAXLINESIZE-2] = '\n';
		line[ZQLOG_DEFAULT_MAXLINESIZE-1] = '\0';
	}
	else
	{
		line[nCount] = '\n';
		line[nCount+1] = '\0';
	}

	int nLineSize = strlen(line);

	{
		ZQ::common::MutexGuard lk(m_buffMtx);
		int totalBuffSize = m_nCurrentBuffSize + nLineSize;
		if(totalBuffSize > m_nMaxBuffSize)
		{
			//���������������ɣ��򽫻�������Ϣflush���ļ�����
			flushData();
			strcpy(m_Buff,line);
			m_nCurrentBuffSize = nLineSize;
		}
		else
		{
			strcat(m_Buff,line);
			m_nCurrentBuffSize += nLineSize;
		}
	}

	return;
}

void CEventFileLog::writeMessage(const wchar_t *msg, int level)
{
	char temp[ZQLOG_DEFAULT_MAXLINESIZE];
	sprintf(temp,"%S",msg);
	writeMessage(temp,level);
}

//�ڽ�buffer�е�����д���ļ�ʱ�������ټ�����ӦΪ�ú���ָ��ͨ��writeMessage�������ã�����writeMessage���Ѿ���buffer���м���
void CEventFileLog::flushData()
{
	if(m_nCurrentBuffSize == 0)
		return;
	int totalFileSize = m_nCurrentFileSize + m_nCurrentBuffSize;
	if(totalFileSize > m_nMaxFileSize)
	{
		//�������ļ��ܴ�С���򴴽����ļ������޸��ļ�����
		RenameAndCreateFile();
		//���������е�����д���ļ��������ļ��Ĵ�СΪ�����������ݵĴ�С
		_pNest->m_FileStream.write(m_Buff,m_nCurrentBuffSize);
		_pNest->m_FileStream.flush();
		m_nCurrentFileSize += m_nCurrentBuffSize;
		//memset����������m_nCurrentBuffSize=0
		memset(m_Buff,0,m_nMaxBuffSize+1);
		m_nCurrentBuffSize=0;
		return;
	}
	else
	{
		//���������ļ���maxsize����д���ļ�
		_pNest->m_FileStream.write(m_Buff,m_nCurrentBuffSize);
		_pNest->m_FileStream.flush();
		m_nCurrentFileSize = totalFileSize;
		//memset����������m_nCurrentBuffSize=0
		memset(m_Buff,0,m_nMaxBuffSize+1);
		m_nCurrentBuffSize=0;
		return;
	}
}

bool CEventFileLog::IsFileExsit(const char* filename,int& retFileSize)
{
	WIN32_FIND_DATA finddata;
	HANDLE hHandle;
	hHandle = ::FindFirstFile(filename,&finddata);

	//filename������ͨ���*?
	if(strstr(filename,"*") != NULL && strstr(filename,"?") != NULL)
	{
		throw CFileLogException("Invalid file name!");
		return false;
	}

	//�Ҳ����ļ�
	if(hHandle == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	//�ҵ����ļ�Ϊ�ļ���
	if((finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
	{
		FindClose(hHandle);
		return false;
	}

	//�õ��ļ���С
	retFileSize = (finddata.nFileSizeHigh * (MAXDWORD + 1)) + finddata.nFileSizeLow;
	FindClose(hHandle);
	return true;
}

void CEventFileLog::RenameAndCreateFile()
{
	//�ٶ����õ��ļ�Ϊc:\tianshan.log
	//����������c:\tianshan_*.logƥ����ļ���*Ϊ������Ŀ���ַ�
	//��c:\tianshan_32.log��c:\tianshan_13.log��c:\tianshan_cf.log��
	::std::vector<::std::string> tempArray;
	::std::vector<::std::string> fileArray;
	WIN32_FIND_DATA findData;
	HANDLE hHandle;
	::std::string tmpName,tmpExt;
	::std::string pathName;
	GetStringLeftAtChar(m_FileName,'.',tmpName);
	GetStringRightAtChar(m_FileName,'.',tmpExt);
	GetPathString(m_FileName,pathName);
	char tmpFind[MAX_PATH];
	sprintf(tmpFind,"%s_*.%s",tmpName.c_str(),tmpExt.c_str());
	hHandle = ::FindFirstFile(tmpFind,&findData);
	if(hHandle != INVALID_HANDLE_VALUE)
	{
		do {
			::std::string fullName = pathName + findData.cFileName;
			tempArray.push_back(fullName);
		} while(::FindNextFile(hHandle,&findData));
	}
	FindClose(hHandle);

	int i=0;
	for(i=0;i<tempArray.size();i++)
	{
		::std::string tmpMiddle;
		::std::string tmpName;
		GetStringLeftAtChar(tempArray[i],'.',tmpName);
		GetStringRightAtChar(tmpName,'_',tmpMiddle);
		if(StringIsNumber(tmpMiddle))
		{
			if(atoi(tmpMiddle.c_str())>=0 && atoi(tmpMiddle.c_str())<=9 && fileArray.size()<m_nMaxLogfileNum)
				fileArray.push_back(tempArray[i]);
		}
	}
	
	
	for(i=0;i<tempArray.size();i++)
	{
		::std::string tmpMiddle;
		::std::string tmpName;
		GetStringLeftAtChar(tempArray[i],'.',tmpName);
		GetStringRightAtChar(tmpName,'_',tmpMiddle);
		if(StringIsNumber(tmpMiddle))
		{
			if(atoi(tmpMiddle.c_str())>=10 && atoi(tmpMiddle.c_str())<=99 && fileArray.size()<m_nMaxLogfileNum)
				fileArray.push_back(tempArray[i]);
		}
	}


	//TODO: �����û�б����ļ�����c:\tianshan_number.log��numberΪ����
	if(fileArray.size() == 0)
	{
		//�رյ�ǰ���ڶ�Щ���ļ�c:\tianshan.log�����������Ϊc:\tianshan_0.log
		if(_pNest->m_FileStream.is_open())
			_pNest->m_FileStream.close();
		char newName[MAX_PATH];
		::std::string tmpName,tmpExt;
		GetStringLeftAtChar(m_FileName,'.',tmpName);
		GetStringRightAtChar(m_FileName,'.',tmpExt);
		sprintf(newName,"%s_0.%s",tmpName.c_str(),tmpExt.c_str());
		if (rename(m_FileName,newName) != 0)
		{
			SYSTEMLOG(Log::L_ERROR,"Can't rename %s to %s",m_FileName,newName);
		}

		//����һ���µ��ļ�������Ϊssm_tianshan_s1.log
		_pNest->m_FileStream.open(m_FileName,std::ios::out||std::ios::binary);
		if(!_pNest->m_FileStream.is_open())
		{
			SYSTEMLOG(Log::L_ERROR,"Create file fail!");
			throw CFileLogException("Create file fail!");
		}
		m_nCurrentFileSize = 0;
		return;
	}


	//TODO: ������������c:\tianshan_number.log�ļ���numberֵ��1
	if(fileArray.size() == m_nMaxLogfileNum)
	{
		//����ļ����Ѿ��ﵽ���ֻ��ɾ�����ϵ��ļ����ڴ���һ�����ļ�
		//ɾ�����ϵ��ļ�fileArray[fileArray.size()-1];
		DeleteFile(fileArray[fileArray.size()-1].c_str());
		for(int i=fileArray.size()-2; i>=0; i--)
		{
			char newName[MAX_PATH];
			::std::string tmpPre,tmpNumber,tmpExt,tmpName;
			GetStringLeftAtChar(fileArray[i],'.',tmpName);
			GetStringRightAtChar(fileArray[i],'.',tmpExt);
			GetStringLeftAtChar(tmpName,'_',tmpPre);
			GetStringRightAtChar(tmpName,'_',tmpNumber);
			int nFileNumber = atoi(tmpNumber.c_str()) + 1;
			sprintf(newName,"%s_%d.%s",tmpPre.c_str(),nFileNumber,tmpExt.c_str());
			if (rename(fileArray[i].c_str(),newName) != 0)
			{
				SYSTEMLOG(Log::L_ERROR,"Can't rename %s to %s",fileArray[i].c_str(),newName);
			}
		}

		//�رյ�ǰ���ڶ�Щ���ļ�c:\tianshan.log�����������Ϊc:\tianshan_0.log
		if(_pNest->m_FileStream.is_open())
			_pNest->m_FileStream.close();
		char newName[MAX_PATH];
		::std::string tmpName,tmpExt;
		GetStringLeftAtChar(m_FileName,'.',tmpName);
		GetStringRightAtChar(m_FileName,'.',tmpExt);
		sprintf(newName,"%s_0.%s",tmpName.c_str(),tmpExt.c_str());
		if (rename(m_FileName,newName) != 0)
		{
			SYSTEMLOG(Log::L_ERROR,"Can't rename %s to %s",m_FileName,newName);
		}

		//����һ���µ��ļ�������Ϊssm_tianshan_s1.log
		_pNest->m_FileStream.open(m_FileName,std::ios::out||std::ios::binary);
		if(!_pNest->m_FileStream.is_open())
		{
			SYSTEMLOG(Log::L_ERROR,"Create file fail!");
			throw CFileLogException("Create file fail!");
		}
		m_nCurrentFileSize = 0;
	}
	else
	{
		for(int i=fileArray.size()-1; i>=0; i--)
		{
			char newName[MAX_PATH];
			::std::string tmpPre,tmpNumber,tmpExt,tmpName;
			GetStringLeftAtChar(fileArray[i],'.',tmpName);
			GetStringRightAtChar(fileArray[i],'.',tmpExt);
			GetStringLeftAtChar(tmpName,'_',tmpPre);
			GetStringRightAtChar(tmpName,'_',tmpNumber);
			int nFileNumber = atoi(tmpNumber.c_str()) + 1;
			sprintf(newName,"%s_%d.%s",tmpPre.c_str(),nFileNumber,tmpExt.c_str());
			if (rename(fileArray[i].c_str(),newName) != 0)
			{
				SYSTEMLOG(Log::L_ERROR,"Can't rename %s to %s",fileArray[i].c_str(),newName);
			}
		}

		//�رյ�ǰ���ڶ�Щ���ļ�c:\tianshan.log�����������Ϊc:\tianshan_0.log
		if(_pNest->m_FileStream.is_open())
			_pNest->m_FileStream.close();
		char newName[MAX_PATH];
		::std::string tmpName,tmpExt;
		GetStringLeftAtChar(m_FileName,'.',tmpName);
		GetStringRightAtChar(m_FileName,'.',tmpExt);
		sprintf(newName,"%s_0.%s",tmpName.c_str(),tmpExt.c_str());
		if (rename(m_FileName,newName) != 0)
		{
			SYSTEMLOG(Log::L_ERROR,"Can't rename %s to %s",m_FileName,newName);
		}

		//����һ���µ��ļ�������Ϊssm_tianshan_s1.log
		_pNest->m_FileStream.open(m_FileName,std::ios::out||std::ios::binary);
		if(!_pNest->m_FileStream.is_open())
		{
			SYSTEMLOG(Log::L_ERROR,"Create file fail!");
			throw CFileLogException("Create file fail!");
		}
		m_nCurrentFileSize = 0;
	}
}

void CEventFileLog::stop()
{
	flush();
	waitHandle(2000);

	if (_event)
	{
		CloseHandle(_event);
		_event = NULL;
	}
}

bool CEventFileLog::init(void)
{
	_event =  ::CreateEvent(NULL,TRUE,FALSE,NULL);
	return true;
}

void CEventFileLog::final(void)
{
	delete(this);
}

int CEventFileLog::run()
{
	while(1)
	{
 		WaitForSingleObject(_event,m_nFlushInterval*1000);
		{
			flush();
 		}
	}

	return 0;
}

void CEventFileLog::flush()
{
	ZQ::common::MutexGuard lk(m_buffMtx);
	flushData();
}

