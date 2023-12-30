// UnixTextLog.cpp: implementation of the UnixTextLog class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable:4786)

#include "stdafx.h"
#include "UnixTextLog.h"
#include "ChannelMessageQueue.h"
#include "KeyDefine.h"
#include "Log.h"

using namespace ZQ::common;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define		DEF_MAX_FILE_SIZE		10*1024*1024
#define		DEF_MAX_FILE_NUMBER		5



const char*	UnixTextLog::_requiredFields[]={
	KD_KN_OUTPUT,
};
int	UnixTextLog::_nRequiredField = sizeof(UnixTextLog::_requiredFields)/sizeof(const char*);

UnixTextLog::UnixTextLog(int channelID):BaseMessageReceiver(channelID)
{
	_hFile = NULL;
	_dwFileSize = 0;
}

UnixTextLog::~UnixTextLog()
{
	close();
}

void UnixTextLog::requireFields(std::vector<std::string>& fields)
{
	fields.clear();

	for(int i=0;i<_nRequiredField;i++)
	{
		fields.push_back(_requiredFields[i]);
	}
}

bool UnixTextLog::init(InitInfo& initInfo, const char* szSessionName)
{
	initInfo.setCurrent(szSessionName);
	
	if(!initInfo.getValue(KD_KN_FILENAME, _filename, true, true))
		return false;

	if(!initInfo.getValue(KD_KN_MAXFILESIZE, _nMaxFileSize))
	{
		_nMaxFileSize = DEF_MAX_FILE_SIZE;
	}
	
	if(!initInfo.getValue(KD_KN_MAXFILENUMBER, _nMaxFileNumber))
	{
		_nMaxFileNumber = DEF_MAX_FILE_NUMBER;
	}

	_hFile = ::CreateFileA( _filename.c_str(), GENERIC_WRITE, FILE_SHARE_READ,
          NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if (_hFile == INVALID_HANDLE_VALUE)
	{
		glog(Log::L_ERROR, KD_KEY_ERROR_FORMAT, KD_KN_FILENAME, szSessionName, _filename.c_str());
		return false;
	}

	_dwFileSize = GetFileSize(_hFile, NULL);

	// seek to file end, append mode
	LONG lHigh = 0;
	SetFilePointer(_hFile, 0, &lHigh, FILE_END);

	glog(Log::L_DEBUG, "UnixTextLog init success, _filename %s", _filename.c_str());
	return true;
}

void UnixTextLog::close()
{
	if (_hFile && _hFile!=INVALID_HANDLE_VALUE)
	{
		CloseHandle(_hFile);
		_hFile = NULL;
	}
}

void UnixTextLog::OnMessage(int nMessageID, MessageFields* pMessage)
{
	if (!checkLog())
		return;
	
	//
	// only care KD_KN_OUTPUT
	//
	MessageFields::iterator it;
	bool bFound = false;
	for(it=pMessage->begin();it!=pMessage->end();it++)
	{
		if(!stricmp(it->key.c_str(), KD_KN_OUTPUT))
		{
			char tmp[1024*10];
			int len = sprintf(tmp, "%s\r\n", it->value.c_str());
			DWORD dwWrite;
			WriteFile(_hFile, tmp, len, &dwWrite, NULL);

			_dwFileSize += len;

			//if log the output?

			bFound = true;
			break;
		}
	}	

	if (!bFound)
	{
		glog(Log::L_ERROR, "UnixTextLog OnMessage, required field %s not found", KD_KN_OUTPUT);
	}
}

bool UnixTextLog::checkLog()
{
	if (!_hFile || _hFile==INVALID_HANDLE_VALUE)
		return false;

	if (_dwFileSize < _nMaxFileSize)
		return true;

	DWORD dw1 = GetTickCount();
	CloseHandle(_hFile);	
	
	//delete the oldest file(if the file number is bigger than max file number)
	char szFilename[256];
	sprintf(szFilename, "%s.%d", _filename.c_str(), _nMaxFileNumber);
	DeleteFileA(szFilename);

	char szFile2[256];
	for(int i=_nMaxFileNumber;i>1;i--)
	{
		sprintf(szFilename, "%s.%d", _filename.c_str(), i-1);
		sprintf(szFile2, "%s.%d", _filename.c_str(), i);

		MoveFileA(szFilename, szFile2);		
	}

	//rename current file
	if (!MoveFileA(_filename.c_str(), szFilename))
	{
		glog(Log::L_ERROR, "Fail to rename file %s to %s", _filename.c_str(), szFilename);
	}

	_hFile = ::CreateFileA( _filename.c_str(), GENERIC_WRITE, FILE_SHARE_READ,
          NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if (_hFile == INVALID_HANDLE_VALUE)
	{
		glog(Log::L_ERROR, "Fail to open file %s", _filename.c_str());
		return false;
	}

	_dwFileSize = 0;
	glog(Log::L_DEBUG, "New log file created, spent %d ms", GetTickCount()-dw1);
	
	return true;
}