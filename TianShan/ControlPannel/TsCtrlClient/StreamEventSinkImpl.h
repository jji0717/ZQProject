// StreamEventSinkImpl.h: interface for the StreamEventSinkImpl class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __STREAMEVENTSINK_H 
#define __STREAMEVENTSINK_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "ZQEventsCtrl.h"

class StreamEventSinkImpl : public TianShanIce::Streamer::StreamEventSink  
{
public:
	StreamEventSinkImpl(RegEvent_Proc pFun);
	StreamEventSinkImpl();
	virtual ~StreamEventSinkImpl();
	void SetFunction(RegEvent_Proc pFun);
	
	virtual void OnEndOfStream(const ::std::string& proxy, const ::std::string& uid, const ::Ice::Current& ic= ::Ice::Current()) const ;
	virtual void OnBeginningOfStream(const ::std::string& proxy, const ::std::string& uid, const ::Ice::Current& ic = ::Ice::Current())const ;
	virtual void OnSpeedChanged(const ::std::string& proxy, const ::std::string& uid, ::Ice::Float prevSpeed, ::Ice::Float currentSpeed, const ::Ice::Current& ic = ::Ice::Current()) const ;
	virtual void OnStateChanged(const ::std::string& proxy, const ::std::string& uid, ::TianShanIce::State Prevstate, ::TianShanIce::State curState, const ::Ice::Current&  ic= ::Ice::Current()) const;
	virtual void OnExit(const ::std::string& proxy, const ::std::string& uid, ::Ice::Int exitCode, const ::std::string& reason, const ::Ice::Current& ic = ::Ice::Current()) const ;
	virtual void ping(::Ice::Long timestamp, const ::Ice::Current& ic = ::Ice::Current());
private:
	RegEvent_Proc m_pFun;
};

extern RegPlayListStateProc m_StateProc; // for the playliststate proc

#endif // !defined(AFX_STREAMEVENTSINKIMPL_H__0424C0A2_2CC9_44F3_BB3A_ACB3ECED54E2__INCLUDED_)
