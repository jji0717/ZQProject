#include "stdafx.h"
#include "OnDataEventCB.h"

extern ZQ::common::Log* _logger;
#define ONDATAEVENTCB "OnDataEventCB"
namespace TianShanIce{
namespace Application{
namespace DataOnDemand{
OnDataEventCB::OnDataEventCB(void)
{
}

OnDataEventCB::~OnDataEventCB(void)
{
}

void OnDataEventCB::ice_response()
{
	(*_logger)(ZQ::common::Log::L_INFO, CLOGFMT(ONDATAEVENTCB,
		"ice_response()"));
}
void 
OnDataEventCB::ice_exception(const ::Ice::Exception&ex)
{
	(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(ONDATAEVENTCB,
		"ice_exception() '%s'"), ex.ice_name().c_str());
}
void 
OnDataEventCB::ice_exception(const ::std::exception&ex)
{
	(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(ONDATAEVENTCB,
		"ice_exception() '%s'"), ex.what());
}
void 
OnDataEventCB::ice_exception()
{
	(*_logger)(ZQ::common::Log::L_ERROR, CLOGFMT(ONDATAEVENTCB,
		"ice_exception()"));
}
}}}