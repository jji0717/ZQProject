// File Name : TeardownHandler.cpp

#ifndef __EVENT_IS_VODI5_TEARDOWN_HANDLER_H__
#define __EVENT_IS_VODI5_TEARDOWN_HANDLER_H__

#include "RequestHandler.h"

namespace GBss
{

class TeardownHandler : public RequestHandler
{
public:
	typedef IceUtil::Handle<TeardownHandler> Ptr;

	TeardownHandler(ZQ::common::Log& fileLog, Environment& env, 
		IStreamSmithSite* pSite, IClientRequestWriter* pReq);

	~TeardownHandler();

	virtual RequestProcessResult doContentHandler();
};

} // end GBss

#endif // end __EVENT_IS_VODI5_TEARDOWN_HANDLER_H__
