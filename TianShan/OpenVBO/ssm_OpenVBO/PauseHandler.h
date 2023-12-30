// File Name : PauseHandler.h

#ifndef __EVENT_IS_VODI5_PAUSE_HANDLER_H__
#define __EVENT_IS_VODI5_PAUSE_HANDLER_H__

#include "RequestHandler.h"

namespace EventISVODI5
{

class PauseHandler : public RequestHandler
{
public:
	typedef IceUtil::Handle<PauseHandler> Ptr;

	PauseHandler(ZQ::common::Log& fileLog, Environment& env,
		IStreamSmithSite* pSite, IClientRequestWriter* pReq);

	~PauseHandler();

	virtual RequestProcessResult doContentHandler();
};

} // end EventISVODI5

#endif // end __EVENT_IS_VODI5_PAUSE_HANDLER_H__