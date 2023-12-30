#ifndef __GetParamHandler_H__
#define __GetParamHandler_H__

#include "RequestHandler.h"

class GetParamHandler : public RequestHandler
{
public:
	GetParamHandler(ssmNGODr2c1& ssm, IStreamSmithSite* pSite, IClientRequestWriter* pReq);
	virtual ~GetParamHandler();
	virtual RequestProcessResult process();
	
private:
	std::vector<std::string> _reqParams;
	std::vector<std::string> _streamParams;
	std::vector<std::string> _appParams;
	std::map<std::string, std::string> _outMap;
	std::string _returnContent;
};

#endif // __GetParamHandler_H__