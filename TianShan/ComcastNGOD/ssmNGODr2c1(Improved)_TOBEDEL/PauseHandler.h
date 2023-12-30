#ifndef __PauseHandler_H__
#define __PauseHandler_H__

#include "./RequestHandler.h"

class PauseHandler : public RequestHandler
{
public:
	PauseHandler(ssmNGODr2c1& ssm, IStreamSmithSite* pSite, IClientRequestWriter* pReq);
	virtual ~PauseHandler();
	virtual RequestProcessResult process();

private:
	std::string _retRange;
	std::string _retScale;
};

#endif // __PauseHandler_H__