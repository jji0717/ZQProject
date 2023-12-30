#ifndef __GetParamHandler_H__
#define __GetParamHandler_H__

#include "RequestHandler.h"

class GetParamHandler : public RequestHandler
{
public:
	GetParamHandler(NGODEnv& ssm, IStreamSmithSite* pSite, IClientRequestWriter* pReq);
	virtual ~GetParamHandler();
	virtual RequestProcessResult process();
	
private:
};

#endif // __GetParamHandler_H__