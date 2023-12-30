// StreamEventSinkI.h: interface for the StreamEventSinkI class.
//
//////////////////////////////////////////////////////////////////////
#ifndef __TIANSHAN_EVENTGW_STREAMEVENTSINKI_H__
#define __TIANSHAN_EVENTGW_STREAMEVENTSINKI_H__
#include <ZQ_common_conf.h>
#include "EventGwHelper.h"
#include <Locks.h>
#include "TsStreamer.h"
#include <set>
#include <list>
#include "SystemUtils.h"
#include "GenEventSinkI.h"
namespace EventGateway
{

	class GenHelpersOnEvent; 
	class BaseEventHelp
	{
	public:
		BaseEventHelp(ZQ::common::Log&);
		virtual ~BaseEventHelp();
	public:

		void add(IGenericEventHelper *pHelper);
		void remove(IGenericEventHelper *pHelper);
		size_t count();

	protected:
		void postStreamEvent(
			const ::std::string& category,
			::Ice::Int eventId,
			const ::std::string& eventName,
			const ::std::string& stampUTC,
			const ::std::string& sourceNetId,
			const ::TianShanIce::Properties& params) const;

	protected:
		ZQ::common::Log &_log;
		typedef std::set<IGenericEventHelper*> StreamEHelperCollection;
		StreamEHelperCollection _helpers;
		ZQ::common::Mutex   _lockHelpers;

		GenHelpersOnEvent  _thrHelpersOnEvent;
	};

	class StreamEventSinkImpl : public TianShanIce::Streamer::StreamEventSink, public BaseEventHelp
	{
	public:
		StreamEventSinkImpl(ZQ::common::Log&);
		virtual ~StreamEventSinkImpl();

		/// safe pointer to this type of objects
		typedef ::IceInternal::Handle<StreamEventSinkImpl> Ptr;

		virtual void OnEndOfStream(const ::std::string& proxy, const ::std::string& id, const TianShanIce::Properties& props, const ::Ice::Current& c) const;

		virtual void OnBeginningOfStream(const ::std::string& proxy, const ::std::string& id, const TianShanIce::Properties& props,
										const ::Ice::Current& c) const;
		virtual void OnExit(const ::std::string& proxy, const ::std::string& id, ::Ice::Int exitCode, const ::std::string& reason,
							const ::Ice::Current& c) const;

		virtual void OnSpeedChanged(const ::std::string& proxy, const ::std::string& id, ::Ice::Float prevSpeed, ::Ice::Float currentSpeed, 
									const ::TianShanIce::Properties& props, const ::Ice::Current& c) const;

		virtual void OnStateChanged(const ::std::string& proxy, const ::std::string& id, ::TianShanIce::Streamer::StreamState prevState, 
									::TianShanIce::Streamer::StreamState currentState, const ::TianShanIce::Properties& props, 
									const ::Ice::Current& c)const;

		virtual void ping(::Ice::Long timestamp, const ::Ice::Current& c);

		virtual void OnExit2(const ::std::string& proxy, const ::std::string& playlistId, ::Ice::Int exitCode, 
							const ::std::string& reason, const TianShanIce::Properties& props, const ::Ice::Current& c) const;
	private:
		ZQ::common::Log &_log;
	};

	class PlaylistEventSinkImpl : public  TianShanIce::Streamer::PlaylistEventSink, public BaseEventHelp
	{
	public:
		PlaylistEventSinkImpl(ZQ::common::Log&);
		virtual ~PlaylistEventSinkImpl();

		virtual void ping(::Ice::Long timestamp, const ::Ice::Current& c);

		void OnItemStepped(const ::std::string& proxy, const ::std::string& id, ::Ice::Int currentUserCtrlNum,
		::Ice::Int prevUserCtrlNum, const ::TianShanIce::Properties& ItemProps, const ::Ice::Current& c) const;
		/// safe pointer to this type of objects
		typedef ::IceInternal::Handle<PlaylistEventSinkImpl> Ptr;
	private:
		ZQ::common::Log &_log;
	};

}


#endif //__TIANSHAN_EVENTGW_STREAMEVENTSINKI_H__
