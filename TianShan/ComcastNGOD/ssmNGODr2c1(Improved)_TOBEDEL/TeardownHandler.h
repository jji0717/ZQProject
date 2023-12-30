#ifndef __TeardownHandler_H__
#define __TeardownHandler_H__

#include "RequestHandler.h"

class TeardownHandler : public RequestHandler
{
public:

	TeardownHandler(ssmNGODr2c1& ssm, IStreamSmithSite* pSite, IClientRequestWriter* pReq);
	virtual ~TeardownHandler();

	virtual RequestProcessResult process();

private:
	std::string _retContent;

};

#endif // __TeardownHandler_H__