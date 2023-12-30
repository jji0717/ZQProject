// ProvisionProgressImpl.h: interface for the ProvisionProgressImpl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROVISIONPROGRESSIMPL_H__F351E54C_014C_4B73_94F9_9BACA1427FD4__INCLUDED_)
#define AFX_PROVISIONPROGRESSIMPL_H__F351E54C_014C_4B73_94F9_9BACA1427FD4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "ZQEventsCtrl.h"

class ProvisionProgressImpl : public TianShanIce::Storage::ProvisionProgressSink  
{
public:
	ProvisionProgressImpl();
	virtual ~ProvisionProgressImpl();
	ProvisionProgressImpl(RegEvent_Proc pFun);
	void SetFunction(RegEvent_Proc pFun);

	virtual void OnProgress(const ::std::string& netId, const ::std::string& contentname, const ::std::string& triggerTimeUTC, ::Ice::Long processed, ::Ice::Long total, ::Ice::Int stepNo, ::Ice::Int totalSteps, const ::Ice::Current& ic= ::Ice::Current());
	virtual void ping(::Ice::Long timestamp, const ::Ice::Current& ic = ::Ice::Current());
		
private:
	RegEvent_Proc m_pFun;
};

#endif // !defined(AFX_PROVISIONPROGRESSIMPL_H__F351E54C_014C_4B73_94F9_9BACA1427FD4__INCLUDED_)
