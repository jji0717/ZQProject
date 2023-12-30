#ifndef __PingHandler_H__
#define __PingHandler_H__

#include "./RequestHandler.h"

class PingHandler : public RequestHandler
{
public:
	PingHandler(ssmNGODr2c1& ssm, IStreamSmithSite* pSite, IClientRequestWriter* pReq);
	virtual ~PingHandler();
	virtual RequestProcessResult process();
};

#endif // __PingHandler_H__