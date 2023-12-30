// StreamEventSinkI.h: interface for the StreamEventSinkI class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _NGOD_STREAMEVENTSINKI_H_
#define _NGOD_STREAMEVENTSINKI_H_

#include "TsStreamer.h"
#include <string>

using namespace std;

class ssmNGODr2c1;
class StreamEventSinkI : public TianShanIce::Streamer::StreamEventSink  
{
protected:
	ssmNGODr2c1*										_pSsmNGODr2c1;
public:
	virtual ~StreamEventSinkI();
	StreamEventSinkI(ssmNGODr2c1* pSsmNGODr2c1);
	void ping(::Ice::Long lv, const ::Ice::Current& ic = ::Ice::Current());
	void OnEndOfStream(const ::std::string& proxy, const ::std::string& uid, const ::Ice::Current& ic= ::Ice::Current()) const;
    void OnBeginningOfStream(const ::std::string& proxy, const ::std::string& uid, const ::Ice::Current& ic= ::Ice::Current()) const;
	void OnSpeedChanged(const ::std::string& proxy, const ::std::string& uid, ::Ice::Float prevSpeed, ::Ice::Float currentSpeed, const ::Ice::Current& ic = ::Ice::Current()) const;
	void OnStateChanged(const ::std::string& proxy, const ::std::string&, ::TianShanIce::Streamer::StreamState prevState, ::TianShanIce::Streamer::StreamState currentState, const ::Ice::Current& ic= ::Ice::Current()) const ;
    void OnExit(const ::std::string& proxy, const ::std::string&, ::Ice::Int nExitCode, const ::std::string& sReason, const ::Ice::Current& ic = ::Ice::Current()) const;
};

#endif // _NGOD_STREAMEVENTSINKI_H_