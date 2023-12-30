#ifndef __PlayHandler_H__
#define __PlayHandler_H__

#include "RequestHandler.h"

class PlayHandler : public RequestHandler
{
public:
	PlayHandler(ssmNGODr2c1& ssm, IStreamSmithSite* pSite, IClientRequestWriter* pReq);
	virtual ~PlayHandler();
	virtual RequestProcessResult process();

private:
	std::string _reqRange;
	std::string _reqScale;
	std::string _retRange;
	std::string _retScale;
};

#endif // __PlayHandler_H__