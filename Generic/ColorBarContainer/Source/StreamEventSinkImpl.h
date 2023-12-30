// StreamEventSinkImpl.h: interface for the StreamEventSinkImpl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STREAMEVENTSINKIMPL_H__F9AD8D6D_8713_454A_99AD_D89020A636C4__INCLUDED_)
#define AFX_STREAMEVENTSINKIMPL_H__F9AD8D6D_8713_454A_99AD_D89020A636C4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <IceStorm/IceStorm.h>
#include "TsStreamer.h"

class CColorBarContainerControl;
class StreamEventSinkImpl : public ::TianShanIce::Streamer::StreamEventSink
{
public:
	StreamEventSinkImpl();
	StreamEventSinkImpl(CColorBarContainerControl  *pCBContainControl);
	virtual ~StreamEventSinkImpl();
	virtual void OnEndOfStream(const ::std::string& proxy, const ::std::string& uid, const ::Ice::Current& ic= ::Ice::Current()) const ;
	virtual void OnBeginningOfStream(const ::std::string& proxy, const ::std::string& uid, const ::Ice::Current& ic = ::Ice::Current())const ;
	virtual void OnSpeedChanged(const ::std::string& proxy, const ::std::string& uid, ::Ice::Float prevSpeed, ::Ice::Float currentSpeed, const ::Ice::Current& ic = ::Ice::Current()) const ;
	virtual void OnStateChanged(const ::std::string& proxy, const ::std::string& uid, ::TianShanIce::State Prevstate, ::TianShanIce::State curState, const ::Ice::Current&  ic= ::Ice::Current()) const;
	virtual void OnExit(const ::std::string& proxy, const ::std::string& uid, ::Ice::Int exitCode, const ::std::string& reason, const ::Ice::Current& ic = ::Ice::Current()) const ;
	virtual void ping(::Ice::Long timestamp, const ::Ice::Current& ic = ::Ice::Current());

private:
	CColorBarContainerControl  *m_pCBContainControl;

};
#endif // !defined(AFX_STREAMEVENTSINKIMPL_H__F9AD8D6D_8713_454A_99AD_D89020A636C4__INCLUDED_)
