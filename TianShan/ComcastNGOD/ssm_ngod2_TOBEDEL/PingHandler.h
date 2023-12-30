#ifndef __PingHandler_H__
#define __PingHandler_H__

#include "./RequestHandler.h"

class PingHandler : public RequestHandler
{
public:
	typedef IceUtil::Handle<PingHandler> Ptr;
	PingHandler(NGODEnv& ssm, IStreamSmithSite* pSite, IClientRequestWriter* pReq);
	virtual ~PingHandler();
	virtual RequestProcessResult process();
};

#endif // __PingHandler_H__