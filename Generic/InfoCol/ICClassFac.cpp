
#pragma warning(disable:4786)

#include "InfoCollector.h"
#include "SCLogCol.h"
#include "TextFileWriter.h"
#include "BoostRegHandler.h"
#include "UnixTextLog.h"


BaseMessageReceiver* InfoCollector::createMessageReceiver(int channelID, const char* szType)
{
	if (!stricmp(szType, TextFileWriter::getTypeInfo()))
	{
		return new TextFileWriter(channelID);
	}
	else if (!stricmp(szType, UnixTextLog::getTypeInfo()))
	{
		return new UnixTextLog(channelID);
	}

	return NULL;
}

BaseInfoCol* InfoCollector::createInfoCol(const char* szType)
{
	if (!stricmp(szType, SCLogCol::getTypeInfo()))
	{
		return new SCLogCol();
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
