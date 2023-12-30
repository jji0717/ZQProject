// TextWriter.cpp: implementation of the TextWriter class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "TextWriter.h"
#include <FileLog.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
using namespace ZQ::common;

TextWriter::TextWriter():_hExit(false),_hMsgSem(0),_hFile(0)
{
}

TextWriter::~TextWriter()
{
	Close();
}

bool TextWriter::init()
{
	return start();
}

int TextWriter::run()
{
	while(true)
	{
		//wait for a message
		_hMsgSem.wait();
		if (_hExit) //exit
		{
			LOG(Log::L_DEBUG,"TextWrite wait a exit object");
			break;
		}
		
		while (_msgQue.size())
		{
			bool bGetMsg = false;
			MSGSTRUCT msg;
			{			
				ZQ::common::MutexGuard MG(_lock);
				msg = _msgQue.front();
				_msgQue.pop_front();
				bGetMsg = true;
			}
			if(!bGetMsg)
				continue;
			WriteMessage(msg);
		}
	}
	
	return 0;
}

void TextWriter::AddMessage(const MSGSTRUCT& msgStruct)
{
	{	
		ZQ::common::MutexGuard MG(_lock);
		_msgQue.push_back(msgStruct);	
	}
	_hMsgSem.post();
}

void TextWriter::Close()
{
	_hExit=true;
	_hMsgSem.post();

	_msgQue.clear();
		
	if(_hFile)
	{
		fclose(_hFile);
		_hFile = 0;
	}
}

bool TextWriter::GetParFromFile(const char *pFileName)
{
	//get configure information from file
	if(pFileName == NULL || strlen(pFileName) == 0)
	{
		if(plog != NULL)
			LOG(Log::L_ERROR,"TextWriter::GerParFromFile() configuration file path is NULL");
		return false;
	}
	//load config item form xml config file
	if(pEventSenderCfg == NULL)
	{
		pEventSenderCfg = new Config::Loader< EventSender >("");

		if(!pEventSenderCfg)
		{	
			if(plog != NULL)
				LOG(Log::L_ERROR,"TextWriter::GetParFromFile() Create PlugConfig object error");
			return false;
		}
		if(!pEventSenderCfg->load(pFileName))
		{
			if(plog != NULL)
				LOG(Log::L_ERROR,"TextWriter not load config item from xml file:%s",pFileName);
			return false;	
		}
		pEventSenderCfg->snmpRegister("");
	}

	try
	{
		if(plog == NULL)
		{
			plog = new ZQ::common::FileLog(pEventSenderCfg->logPath.c_str(),pEventSenderCfg->logLevel,5,pEventSenderCfg->logSize);
		}
	}
	catch(FileLogException& ex)
	{
#ifdef _DEBUG
		printf("TextWriter::GetParFromFile() Catch a file log exception: %s\n",ex.getString());
#endif	
		return false;			
	}
	catch(...)
	{
		return false;
	}

	_hFile = fopen(pEventSenderCfg->recvFilePath.c_str(), "w+");
	if(!_hFile)
	{
		LOG(Log::L_ERROR,"TextWriter create file %s failed",pEventSenderCfg->recvFilePath.c_str());
		return false;
	}

	return true;
}

bool TextWriter::WriteMessage(const MSGSTRUCT &msg)
{
	if(!_hFile)
	{
		LOG(Log::L_ERROR,"Save event to file error ,TextWriter file handle is invalid");
		return false;
	}

	fseek(_hFile, 0, SEEK_END);

	std::string strText = "";
	char chId[10] = {0};
	sprintf(chId,"%04d",msg.id);
	strText = "eventId:";
	strText += chId;
	strText += "  ";
	strText += "category:" + msg.category + "  ";
	strText += "stampUTC:" + msg.timestamp + "  ";
	strText += "eventName:" + msg.eventName + "  ";
	strText += "sourceNetId:" + msg.sourceNetId;
	std::map<std::string,std::string>::const_iterator it;
	for(it = msg.property.begin(); it != msg.property.end(); it++)
	{
		strText += "  " + it->first + ":" + it->second;
	}
	strText += "\r\n";

	size_t dwByte = fwrite(strText.c_str(), 1, strText.size(), _hFile);
	if(!dwByte)
	{
		LOG(Log::L_ERROR,"TextWriter write file error");
		return false;
	}

	return true;
}
