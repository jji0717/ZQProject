#ifndef __PlayHandler_H__
#define __PlayHandler_H__

#include "RequestHandler.h"

class PlayHandler : public RequestHandler
{
public:
	
	typedef IceUtil::Handle<PlayHandler> Ptr;

	PlayHandler(NGODEnv& ssm, IStreamSmithSite* pSite, IClientRequestWriter* pReq);
	virtual ~PlayHandler();
	virtual RequestProcessResult process();

	void addPlayEvent(const std::string strSpeed, const std::string strNPT, const std::string streamResourceID);

private:
	std::string _reqRange;
	std::string _reqScale;
	std::string _retRange;
	std::string _retScale;
};

#endif // __PlayHandler_H__