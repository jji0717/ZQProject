// TextWriter.cpp: implementation of the TextWriter class.
//
//////////////////////////////////////////////////////////////////////
#include "TextWriter.h"
#include <FileLog.h>
#include <FileSystemOp.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
using namespace ZQ::common;

class WriterCmd : public ZQ::common::ThreadRequest
{
public:
	WriterCmd(ZQ::common::NativeThreadPool& thpool, TextWriter& writer, const MSGSTRUCT& msgStruct, const MessageIdentity& mid, void* ctx)
		:ThreadRequest(thpool), _textWriter(writer), _msgStruct(msgStruct), 
		_mid(mid), _ctx(ctx)
	{
	}
protected:
	int run()
	{
		if(_textWriter._bQuit)
			return 0;

		if( _textWriter.WriteMessage(_msgStruct) )
		{
			if(g_pIMsgSender)
				g_pIMsgSender->ack(_mid, _ctx);
		}

		return 0;
	}

	void final(int retcode =0, bool bCancelled =false)
	{
		delete this;
	}

private:
	TextWriter&		_textWriter;
	MSGSTRUCT		_msgStruct;
	MessageIdentity	_mid;
	void*			_ctx;
};

//////////////////
//class TextWriter
//////////////////
TextWriter::TextWriter(int poolSize)
	:_thPool(poolSize), _hFile(NULL), _bQuit(false)
{
	if(poolSize<MSGSENDER_POOLSIZE)
		_thPool.resize(MSGSENDER_POOLSIZE);

}

TextWriter::~TextWriter()
{
	Close();
}

bool TextWriter::init()
{	
	return true;
}

void TextWriter::AddMessage(const MSGSTRUCT& msgStruct, const MessageIdentity& mid, void* ctx)
{
	(new WriterCmd(_thPool, *this, msgStruct, mid, ctx))->start();
}

void TextWriter::Close()
{	
	if( !_bQuit )
	{
		_bQuit = true;
		_thPool.stop();
	}
	if(_hFile != NULL)
	{
		fflush(_hFile);
		fclose(_hFile);
		_hFile = NULL;
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
			plog = new ZQ::common::FileLog(pEventSenderCfg->logPath.c_str(), pEventSenderCfg->logLevel, pEventSenderCfg->logNumber, pEventSenderCfg->logSize);
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

	size_t nIndex = pEventSenderCfg->recvFilePath.rfind(FNSEPS);
	if(nIndex != std::string::npos)
		FS::createDirectory(pEventSenderCfg->recvFilePath.substr(0, nIndex), true);

	_hFile = fopen(pEventSenderCfg->recvFilePath.c_str(), "w+");
	if(_hFile == NULL)
	{
		LOG(Log::L_ERROR,"TextWriter create file %s failed",pEventSenderCfg->recvFilePath.c_str());
		return false;
	}

	return true;
}

bool TextWriter::WriteMessage(const MSGSTRUCT &msg)
{
	ZQ::common::MutexGuard MG(_lock);
	if(_hFile == NULL)
	{
		LOG(Log::L_ERROR,"Save event to file error ,TextWriter file handle is invalid");
		return false;
	}

	fseek(_hFile,0,SEEK_END);

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

	if( fwrite((void*)strText.c_str(), 1, strText.size(), _hFile) < strText.size())
	{
		LOG(Log::L_ERROR,"TextWriter write file error");
		return false;
	}

	return true;
}
