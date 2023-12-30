// SessionEventSinkImpl.h: interface for the SessionEventSinkImpl class.
//
//////////////////////////////////////////////////////////////////////

#ifndef  __SESSIONEVENTSINK_H
#define  __SESSIONEVENTSINK_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "ZQEventsCtrl.h"

class SessionEventSinkImpl : public TianShanIce::SRM::SessionEventSink
{
public:
	SessionEventSinkImpl();
	virtual ~SessionEventSinkImpl();
	SessionEventSinkImpl(RegEvent_Proc pFun);
	void SetFunction(RegEvent_Proc pFun);

	virtual void OnNewSession(const ::std::string& sessId, const ::std::string& proxy, const ::Ice::Current&  ic= ::Ice::Current());
	virtual void OnDestroySession(const ::std::string& sessId, const ::Ice::Current& ic  = ::Ice::Current());
	virtual void OnStateChanged(const ::std::string& sessId, const ::std::string& proxy, ::TianShanIce::State prevState, ::TianShanIce::State currentState, const ::Ice::Current& ic  = ::Ice::Current());
	virtual void ping(::Ice::Long timestamp, const ::Ice::Current& ic = ::Ice::Current());
private:
	RegEvent_Proc m_pFun;
};
#endif // !defined(AFX_SESSIONEVENTSINKIMPL_H__96EF9D13_E465_470F_9D66_D2823A47D5A5__INCLUDED_)
