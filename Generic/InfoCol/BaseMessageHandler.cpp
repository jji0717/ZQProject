// BaseMessageHandler.cpp: implementation of the BaseMessageHandler class.
//
//////////////////////////////////////////////////////////////////////

#include "BaseMessageHandler.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BaseMessageHandler::BaseMessageHandler()
{
	_channelID = 0;
	_channelQueue = NULL;
}

BaseMessageHandler::~BaseMessageHandler()
{

}

bool BaseMessageHandler::init(int channelID, const char* syntax, ChannelMessageQueue* channelQueue)
{
	_channelID = channelID;
	_channelQueue = channelQueue;
	_syntax = syntax;

	return true;
}

void BaseMessageHandler::close()
{
	_channelID = 0;
	_channelQueue = NULL;
}

void BaseMessageHandler::addOutputField(const char* key, const char* value)
{
	std::vector<struct OUTPUT_FIELD>::iterator it;
	for(it=_outputs.begin();it!=_outputs.end();it++)
	{
		if (!it->strKey.compare(key))
		{
			it->strValue = value;
			return;
		}
	}

	OUTPUT_FIELD of;
	of.strKey = key;
	of.strValue = value;
	_outputs.push_back(of);
}


