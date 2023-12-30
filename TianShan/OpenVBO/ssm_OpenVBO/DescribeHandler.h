// File Name : DescribeHandler.h

#ifndef __EVENT_IS_VODI5_DESCRIBE_HANDLER_H__
#define __EVENT_IS_VODI5_DESCRIBE_HANDLER_H__

#include "RequestHandler.h"

namespace EventISVODI5
{

class DescribeHandler : public RequestHandler
{
public:
	typedef IceUtil::Handle<DescribeHandler> Ptr;

	DescribeHandler(ZQ::common::Log& fileLog, Environment& env, 
		IStreamSmithSite* pSite, IClientRequestWriter* pReq);

	~DescribeHandler();

	virtual RequestProcessResult doContentHandler();
};

} // end EventISVODI5

#endif // end __EVENT_IS_VODI5_DESCRIBE_HANDLER_H__
