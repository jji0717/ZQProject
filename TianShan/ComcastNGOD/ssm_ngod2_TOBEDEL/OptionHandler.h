#ifndef __OptionHandler_H__
#define __OptionHandler_H__

#include "./RequestHandler.h"

class OptionHandler : public RequestHandler
{
public: 
	typedef IceUtil::Handle<OptionHandler> Ptr;
	OptionHandler(NGODEnv& ssm, IStreamSmithSite* pSite, IClientRequestWriter* pReq);
	virtual ~OptionHandler();
	virtual RequestProcessResult process();

protected:
	const char* getMonthStr(WORD month);
	const char* getDayOfWeekStr(WORD dayOfWeek);
	void getGMTTime(char* buff, uint16 len);
};

#endif // __OptionHandler_H__