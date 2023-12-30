
#pragma warning(disable:4786)

#include "InfoCollector.h"
#include "SCLogCol.h"
#include "FileLogCol.h"
#include "TextFileWriter.h"
#include "BoostRegHandler.h"
#include "CORBAEventReceiver.h"
#include "UnixTextLog.h"
#include "JmsMsgSender.h"

BaseMessageReceiver* InfoCollector::createMessageReceiver(int channelID, const char* szType)
{
	if (!stricmp(szType, TextFileWriter::getTypeInfo()))
	{
		return new TextFileWriter(channelID);
	}
	else if (!stricmp(szType, CORBAEventReceiver::getTypeInfo()))
	{
		return new CORBAEventReceiver(channelID);
	}
	else if (!stricmp(szType, UnixTextLog::getTypeInfo()))
	{
		return new UnixTextLog(channelID);
	}
	else if(!stricmp(szType,JmsMsgSender::getTypeInfo()))
	{
		return new JmsMsgSender(channelID);
	}
	return NULL;
}

BaseInfoCol* InfoCollector::createInfoCol(const char* szType)
{
	if (!stricmp(szType, SCLogCol::getTypeInfo()))
	{
		return new SCLogCol();
	}
	else if ( stricmp(szType,FileLogCol::getTypeInfo()) == 0 )
	{
		return new FileLogCol();
	}

	return NULL;
}

BaseMessageHandler* InfoCollector::createMessageHandler(const char* szType)
{
	if (!stricmp(szType, BoostRegHandler::getTypeInfo()))
	{
		return new BoostRegHandler();
	}

	return NULL;
}
