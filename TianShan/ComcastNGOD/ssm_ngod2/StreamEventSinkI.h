// StreamEventSinkI.h: interface for the StreamEventSinkI class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _NGOD_STREAMEVENTSINKI_H_
#define _NGOD_STREAMEVENTSINKI_H_

#include "TsStreamer.h"
#include <string>

using namespace std;

class NGODEnv;
class StreamEventSinkI : public TianShanIce::Streamer::StreamEventSink  
{
protected:
	NGODEnv*										_pSsmNGODr2c1;
public:
	virtual ~StreamEventSinkI();
	StreamEventSinkI(NGODEnv* pSsmNGODr2c1);
	void ping(::Ice::Long lv, const ::Ice::Current& ic = ::Ice::Current());
	void OnEndOfStream(const ::std::string& proxy, const ::std::string& uid, const ::Ice::Current& ic= ::Ice::Current()) const;
    void OnBeginningOfStream(const ::std::string& proxy, const ::std::string& uid, const ::Ice::Current& ic= ::Ice::Current()) const;
	void OnSpeedChanged(const ::std::string& proxy, const ::std::string& uid, ::Ice::Float prevSpeed, ::Ice::Float currentSpeed, const ::Ice::Current& ic = ::Ice::Current()) const;
	void OnStateChanged(const ::std::string&, const ::std::string&, ::TianShanIce::Streamer::StreamState, ::TianShanIce::Streamer::StreamState, const ::Ice::Current& = ::Ice::Current()) const ;
    void OnExit(const ::std::string& proxy, const ::std::string&, ::Ice::Int nExitCode, const ::std::string& sReason, const ::Ice::Current& ic = ::Ice::Current()) const;
	void OnExit2(const ::std::string& proxy, const ::std::string&, ::Ice::Int nExitCode, const ::std::string& sReason, const TianShanIce::Properties&, const ::Ice::Current& ic = ::Ice::Current()) const;
};

class PlayListEventSinkI : public TianShanIce::Streamer::PlaylistEventSink
{
protected:
	NGODEnv* _pSsmNGODr2c1;
public:
	virtual ~PlayListEventSinkI();
	PlayListEventSinkI(NGODEnv* pSsmNGODr2c1);
	void ping(::Ice::Long lv, const ::Ice::Current& ic = ::Ice::Current());
	virtual void OnItemStepped(const ::std::string& proxy, const ::std::string& playlistId, 
		::Ice::Int currentUserCtrlNum, ::Ice::Int prevUserCtrlNum, 
		const ::TianShanIce::Properties& ItemProps, const ::Ice::Current& ic = ::Ice::Current()) const;

};

#endif // _NGOD_STREAMEVENTSINKI_H_