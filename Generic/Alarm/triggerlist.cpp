
#include "triggerlist.h"
#include "loghandler.h"
	
TG_BEGIN
	
bool TriggerList::startTrigger()
{
	bool bRtn = false;
	size_t count = size();
	for (size_t i = 0; i < count; ++i)
	{
		if (at(i)->start())
		{
			bRtn = true;
		}
		else
		{
			Log(LogHandler::L_ERROR, "[TriggerList::startTrigger] can not start trigger thread");
		}
	}
	return bRtn;
}

bool TriggerList::stopTrigger()
{
	bool bRtn = false;
	size_t count = size();
	for (size_t i = 0; i < count; ++i)
	{
		if (at(i)->quit())
		{
			bRtn = true;
		}
		else
		{
			Log(LogHandler::L_ERROR, "[TriggerList::startTrigger] can not stop trigger thread");
		}
	}
	return bRtn;
}
	
TG_END
