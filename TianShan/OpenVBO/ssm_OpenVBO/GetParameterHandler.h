// File Name : GetParameterHandler.h

#ifndef __EVENT_IS_VODI5_GETPARAMETER_HANDLER_H__
#define __EVENT_IS_VODI5_GETPARAMETER_HANDLER_H__

#include "RequestHandler.h"

namespace EventISVODI5
{

class GetParameterHandler : public RequestHandler
{
public:
	typedef IceUtil::Handle<GetParameterHandler> Ptr;

	GetParameterHandler(ZQ::common::Log& fileLog, Environment& env,
		IStreamSmithSite* pSite, IClientRequestWriter* pReq);

	~GetParameterHandler();

	virtual RequestProcessResult doContentHandler();
};

} // end EventISVODI5

#endif // end __EVENT_IS_VODI5_GETPARAMETER_HANDLER_H__
