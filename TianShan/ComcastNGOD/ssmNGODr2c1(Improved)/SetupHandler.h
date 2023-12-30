#ifndef __SetupHandler_H__
#define __SetupHandler_H__

#include "RequestHandler.h"

class SetupHandler : public RequestHandler
{
public:
	SetupHandler(ssmNGODr2c1& ssm, IStreamSmithSite* pSite, IClientRequestWriter* pReq);
	virtual ~SetupHandler();
	virtual RequestProcessResult process();
};

#endif // __SetupHandler_H__