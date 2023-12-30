// MagSender.cpp: implementation of the MagSender class.
//
//////////////////////////////////////////////////////////////////////

#include "MagSender.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
using namespace ZQ::common;

MagSender::MagSender()
	:_ICESender(5), _TextWriter(5)
{

}

MagSender::~MagSender()
{

}

bool MagSender::init(const char* pText,const char* pType)
{
	//ICE initialize
	if(stricmp(pType,"ice") == 0)
	{	
		if(_ICESender.GetParFromFile(pText))
			LOG(Log::L_INFO,"ICE sender get config info from file succeed");
		else
		{
			if(plog != NULL)
				LOG(Log::L_ERROR,"ICE sender get config info from file failed");
			return false;
		}
		if(_ICESender.init())		
			LOG(Log::L_INFO,"ICE sender [init] function return true,base info initialize succeed");
		else
		{
			LOG(Log::L_ERROR,"ICE sender init failed");			
			return false;
		}
	}

	//TEXT initialize
	else if(stricmp(pType,"text") == 0)
	{
		if(_TextWriter.GetParFromFile(pText))
			LOG(Log::L_INFO,"TextWriter get config info from file success");
		else
		{
			if(plog != NULL)
				LOG(Log::L_ERROR,"TextWriter get config info failed");
			return false;
		}
		if(_TextWriter.init())		
			LOG(Log::L_INFO,"TextWriter init succeed");
		else
		{
			LOG(Log::L_ERROR,"TextWriter init failed");
			return false;
		}
	}
	else
		return false;

	return true;
}

void MagSender::uninit()
{
	_ICESender.Close();
	_TextWriter.Close();
}

void MagSender::iceMessage(const MSGSTRUCT &msgStruct, const MessageIdentity& mid, void* ctx)
{
	_ICESender.AddMessage(msgStruct, mid, ctx);
}

void MagSender::textMessage(const MSGSTRUCT& msgStruct, const MessageIdentity& mid, void* ctx)
{
	_TextWriter.AddMessage(msgStruct, mid, ctx);
}

