
#ifndef _ZQ_TRIGGERLIST_H_
#define _ZQ_TRIGGERLIST_H_
	
#include "trigger.h"
#include <vector>
	
TG_BEGIN
	
class TriggerList : public std::vector<Trigger*>
{
public:
	bool startTrigger();
	bool stopTrigger();
};
	
TG_END
	
#endif//_ZQ_TRIGGERLIST_H_
