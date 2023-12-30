// MagSender.cpp: implementation of the MagSender class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "MagSender.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
using namespace ZQ::common;

MagSender::MagSender()
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
	//Jms initialize
	else if(stricmp(pType,"jms") == 0)
	{
		if(_JMSSender.GetParFromFile(pText))	
			LOG(Log::L_INFO,"JMS sender get config info from file succeed");
		else
		{
			if(plog != NULL)
				LOG(Log::L_ERROR,"JMS sender get config info failed");
			return false;
		}
		if(_JMSSender.init())		
			LOG(Log::L_INFO,"JMS sender init succeed");
		else
		{
			LOG(Log::L_ERROR,"JMS sender [init] function return false,base info initialize failed");
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
	_JMSSender.Close();
	_TextWriter.Close();
}

void MagSender::iceMessage(const MSGSTRUCT &msgStruct)
{
	_ICESender.AddMessage(msgStruct);
}

void MagSender::jmsMessage(const MSGSTRUCT& msgStruct)
{
	_JMSSender.AddMessage(msgStruct);
}

void MagSender::textMessage(const MSGSTRUCT& msgStruct)
{
	_TextWriter.AddMessage(msgStruct);
}
