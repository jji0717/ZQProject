// ProvisionStateChangeImpl.h: interface for the ProvisionStateChangeImpl class.
//
//////////////////////////////////////////////////////////////////////

#ifndef  __PROVISIONSTATECHANGE_H
#define  __PROVISIONSTATECHANGE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "ZQEventsCtrl.h"

class ProvisionStateChangeImpl : public TianShanIce::Storage::ProvisionStateChangeSink  
{
public:
	ProvisionStateChangeImpl();
	virtual ~ProvisionStateChangeImpl();
	ProvisionStateChangeImpl(RegEvent_Proc pFun);
	void SetFunction(RegEvent_Proc pFun);

	virtual void OnStateChange(const ::std::string& netId, const ::std::string& contentname, const ::std::string& triggerTimeUTC, ::TianShanIce::Storage::ContentProvisionStatus previousStatus, ::TianShanIce::Storage::ContentProvisionStatus currentStatus, ::Ice::Int errorCode, const ::std::string& errorMsg, const ::Ice::Current& ic= ::Ice::Current());	
	virtual void ping(::Ice::Long timestamp, const ::Ice::Current& ic = ::Ice::Current());

private:
	RegEvent_Proc m_pFun;
};

#endif // !defined(AFX_PROVISIONSTATECHANGEIMPL_H__F2F54EB8_3F12_410D_8508_F9CFA4E23F52__INCLUDED_)
