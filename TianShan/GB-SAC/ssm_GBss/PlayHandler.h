// File Name : PlayHandler.h

#ifndef __EVENT_IS_VODI5_PLAY_HANDLER_H__
#define __EVENT_IS_VODI5_PLAY_HANDLER_H__

#include "RequestHandler.h"

namespace GBss
{

class PlayHandler : public RequestHandler
{
public:
	typedef IceUtil::Handle<PlayHandler> Ptr;

	PlayHandler(ZQ::common::Log& fileLog, Environment& env,
		IStreamSmithSite* pSite, IClientRequestWriter* pReq);

	~PlayHandler();

	virtual RequestProcessResult doContentHandler();

private:
	std::string _reqRange;
	std::string _reqScale;
};

} // end GBss

#endif // end __EVENT_IS_VODI5_PLAY_HANDLER_H__
