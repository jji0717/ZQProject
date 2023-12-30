// TextFileWriter.cpp: implementation of the TextFileWriter class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable:4786)

#include "stdafx.h"
#include "TextFileWriter.h"
#include "ChannelMessageQueue.h"
#include "KeyDefine.h"
#include "Log.h"

using namespace ZQ::common;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

const char*	TextFileWriter::_requiredFields[]={
	KD_KN_OUTPUT,
};
int	TextFileWriter::_nRequiredField = sizeof(TextFileWriter::_requiredFields)/sizeof(const char*);

TextFileWriter::TextFileWriter(int channelID):BaseMessageReceiver(channelID)
{
	_hFile = NULL;
}

TextFileWriter::~TextFileWriter()
{
	close();
}

void TextFileWriter::requireFields(std::vector<std::string>& fields)
{
	fields.clear();

	for(int i=0;i<_nRequiredField;i++)
	{
		fields.push_back(_requiredFields[i]);
	}
}

bool TextFileWriter::init(InitInfo& initInfo, const char* szSessionName)
{
	initInfo.setCurrent(szSessionName);
	
	std::string filename;
	if(!initInfo.getValue(KD_KN_FILENAME, filename, true, true))
		return false;

	_hFile = ::CreateFileA( filename.c_str(), GENERIC_WRITE, FILE_SHARE_READ,
          NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if (_hFile == INVALID_HANDLE_VALUE)
	{
		glog(Log::L_ERROR, KD_KEY_ERROR_FORMAT, KD_KN_FILENAME, szSessionName, filename.c_str());
		return false;
	}

	// seek to file end, append mode
	LONG lHigh = 0;
	SetFilePointer(_hFile, 0, &lHigh, FILE_END);

	glog(Log::L_DEBUG, "TextFileWriter init success, filename %s", filename.c_str());
	return true;
}

void TextFileWriter::close()
{
	if (_hFile && _hFile!=INVALID_HANDLE_VALUE)
	{
		CloseHandle(_hFile);
		_hFile = NULL;
	}
}

void TextFileWriter::OnMessage(int nMessageID, MessageFields* pMessage)
{
	if (!_hFile || _hFile==INVALID_HANDLE_VALUE)
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

			//if log the output?

			bFound = true;
			break;
		}
	}	

	if (!bFound)
	{
		glog(Log::L_ERROR, "TextFileWriter OnMessage, required field %s not found", KD_KN_OUTPUT);
	}
}

