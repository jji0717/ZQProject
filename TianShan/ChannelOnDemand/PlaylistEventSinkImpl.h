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


using namespace std;

namespace ZQChannelOnDemand {

class ChODSvcEnv;

class StreamEventSinkImpl : public ::TianShanIce::Streamer::StreamEventSink
{
public:
	StreamEventSinkImpl(ChODSvcEnv& env);
	virtual ~StreamEventSinkImpl();

	/// safe pointer to this type of objects
	typedef ::IceInternal::Handle<StreamEventSinkImpl> Ptr;

    virtual void OnEndOfStream(const ::std::string&, const ::std::string&, const ::TianShanIce::Properties&, const ::Ice::Current& = ::Ice::Current()) const;
    virtual void OnBeginningOfStream(const ::std::string&, const ::std::string&, const ::TianShanIce::Properties&, const ::Ice::Current& = ::Ice::Current()) const;
    virtual void OnExit(const ::std::string&, const ::std::string&, ::Ice::Int, const ::std::string&, const ::Ice::Current& = ::Ice::Current()) const;

	virtual void OnSpeedChanged(const ::std::string&, const ::std::string&, ::Ice::Float, ::Ice::Float, const ::TianShanIce::Properties&, const ::Ice::Current& = ::Ice::Current()) const;
    
	virtual void OnStateChanged(const ::std::string&, const ::std::string&, ::TianShanIce::Streamer::StreamState, ::TianShanIce::Streamer::StreamState, const ::TianShanIce::Properties&, const ::Ice::Current& = ::Ice::Current()) const;
    
	virtual void ping(::Ice::Long, const ::Ice::Current& = ::Ice::Current());

    virtual void OnExit2(const ::std::string&, const ::std::string&, ::Ice::Int, const ::std::string&, const ::TianShanIce::Properties&, const ::Ice::Current& = ::Ice::Current()) const;

protected:
	ChODSvcEnv& _env;
};

class PlaylistEventSinkImpl : public ::TianShanIce::Streamer::PlaylistEventSink  
{
public:
	PlaylistEventSinkImpl(ChODSvcEnv& env);
	virtual ~PlaylistEventSinkImpl();

	/// safe pointer to this type of objects
	typedef ::IceInternal::Handle<PlaylistEventSinkImpl> Ptr;

	virtual void OnItemStepped(const ::std::string&, const ::std::string&, ::Ice::Int,::Ice::Int , const ::TianShanIce::Properties&, const ::Ice::Current& = ::Ice::Current()) const;

	virtual void ping(::Ice::Long, const ::Ice::Current& = ::Ice::Current());
protected:
	ChODSvcEnv& _env;
};

}


#endif // !defined(AFX_STREAMEVENTSINKI_H__6BCA84CE_9168_4614_9860_8D9813969E03__INCLUDED_)
