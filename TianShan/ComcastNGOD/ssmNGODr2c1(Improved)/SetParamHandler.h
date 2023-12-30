#ifndef __SetParamHandler_H__
#define __SetParamHandler_H__

#include "./RequestHandler.h"

class SetParamHandler : public RequestHandler
{
public:
	SetParamHandler(ssmNGODr2c1& ssm, IStreamSmithSite* pSite, IClientRequestWriter* pReq);
	virtual ~SetParamHandler();
	virtual RequestProcessResult process();
private:
	std::string _connectionID;
	std::string _returnContent;
};

#endif // __SetParamHandler_H__