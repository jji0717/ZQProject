// OTEForTeardownCB.h: interface for the OTEForTeardownCB class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ONDATAEVENTCB_H__03CFADFD_9E5E_41BD_B579_387E1426D706__INCLUDED_)
#define AFX_ONDATAEVENTCB_H__03CFADFD_9E5E_41BD_B579_387E1426D706__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TSappdod.h"
#include <IceUtil/IceUtil.h>
namespace TianShanIce{
namespace Application{
namespace DataOnDemand{
	class OnDataEventCB: public AMI_DataPointPublisher_OnDataEvent
	{
	public:
		OnDataEventCB();
		~OnDataEventCB();
		virtual void ice_response();
		virtual void ice_exception(const ::Ice::Exception&);
		virtual void ice_exception(const ::std::exception&);
		virtual void ice_exception();
	};

	typedef ::IceUtil::Handle<OnDataEventCB> OnDataEventCBPtr;

#endif // !defined(AFX_ONDATAEVENTCB_H__03CFADFD_9E5E_41BD_B579_387E1426D706__INCLUDED_)

}}}//end namespace