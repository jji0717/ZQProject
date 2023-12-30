// StreamEventSinkI.h: interface for the StreamEventSinkI class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STREAMEVENTSINKI_H__6BCA84CE_9168_4614_9860_8D9813969E03__INCLUDED_)
#define AFX_STREAMEVENTSINKI_H__6BCA84CE_9168_4614_9860_8D9813969E03__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TsStreamer.h"
#include <ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <IceStorm/IceStorm.h>
#include <EventChannel.h>
//#include "ScLog.h"
#include "FileLog.h"
#include "ssm_PauseTV_s1.h"

using namespace std;

class StreamEventSinkI : public TianShanIce::Streamer::StreamEventSink  
{
protected:
	//static ZQ::common::ScLog *		pLog;
	static ZQ::common::FileLog *		pLog;
public:
	//StreamEventSinkI(ZQ::common::ScLog * log);
	StreamEventSinkI(ZQ::common::FileLog * log);
	virtual ~StreamEventSinkI();
	void ping(::Ice::Long lv, const ::Ice::Current& ic = ::Ice::Current());
	void OnEndOfStream(const ::std::string& proxy, const ::std::string& uid, const ::Ice::Current& ic= ::Ice::Current()) const;
    void OnBeginningOfStream(const ::std::string& proxy, const ::std::string& uid, const ::Ice::Current& ic= ::Ice::Current()) const;
	void OnSpeedChanged(const ::std::string& proxy, const ::std::string& uid, ::Ice::Float prevSpeed, ::Ice::Float currentSpeed, const ::Ice::Current& ic = ::Ice::Current()) const;
	void OnStateChanged(const ::std::string& proxy, const ::std::string& uid, ::TianShanIce::Streamer::StreamState prevState, ::TianShanIce::Streamer::StreamState currentState, const ::Ice::Current& ic = ::Ice::Current()) const ;
    void OnExit(const ::std::string& proxy, const ::std::string&, ::Ice::Int nExitCode, const ::std::string& sReason, const ::Ice::Current& ic = ::Ice::Current()) const;
};

#endif // !defined(AFX_STREAMEVENTSINKI_H__6BCA84CE_9168_4614_9860_8D9813969E03__INCLUDED_)
