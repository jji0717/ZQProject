#ifndef __TeardownHandler_H__
#define __TeardownHandler_H__

#include "RequestHandler.h"

class TeardownHandler : public RequestHandler
{
public:
	typedef IceUtil::Handle<TeardownHandler> Ptr;

	TeardownHandler(NGODEnv& ssm, IStreamSmithSite* pSite, IClientRequestWriter* pReq);
	virtual ~TeardownHandler();

	virtual RequestProcessResult process();

private:

	std::string _retContent;

};

#endif // __TeardownHandler_H__