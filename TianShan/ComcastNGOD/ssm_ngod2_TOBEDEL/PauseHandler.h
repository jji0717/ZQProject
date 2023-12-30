#ifndef __PauseHandler_H__
#define __PauseHandler_H__

#include "./RequestHandler.h"

class PauseHandler : public RequestHandler
{
public:
	typedef IceUtil::Handle<PauseHandler> Ptr;
	PauseHandler(NGODEnv& ssm, IStreamSmithSite* pSite, IClientRequestWriter* pReq);
	virtual ~PauseHandler();
	virtual RequestProcessResult process();

	void addPauseEvent(const std::string strSpeed, const std::string strNPT, const std::string streamResourceID);
private:
	std::string _retRange;
	std::string _retScale;
};

#endif // __PauseHandler_H__