#ifndef __SetParamHandler_H__
#define __SetParamHandler_H__

#include "./RequestHandler.h"

class SetParamHandler : public RequestHandler
{
public:
	
	typedef IceUtil::Handle<SetParamHandler> Ptr;

	SetParamHandler(NGODEnv& ssm, IStreamSmithSite* pSite, IClientRequestWriter* pReq);
	virtual ~SetParamHandler();
	virtual RequestProcessResult process();
private:
	std::string _connectionID;
	std::string _returnContent;
};

#endif // __SetParamHandler_H__