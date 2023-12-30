#ifndef __TsStreamerI_h__
#define __TsStreamerI_h__

namespace ZQ {
	namespace common {
		class NativeThread;
	}
}

namespace TianShanIce {
namespace Streamer {

class IceExporter {
public:
	IceExporter(Ice::CommunicatorPtr& ic, u_short port);
	bool init();
	Ice::ObjectPrx export(Ice::ObjectPtr obj, const std::string& objName);
	Ice::ObjectPtr remove(const Ice::Identity& id);
	
protected:
	Ice::CommunicatorPtr	_ic;
	Ice::ObjectAdapterPtr	_adapter;
	u_short					_icePort;
};

class WMSStreamerServiceImpl : virtual public WMSStreamerService
{
public:

	WMSStreamerServiceImpl(IceExporter& exporter, std::string appName);
	virtual ~WMSStreamerServiceImpl();

	bool init();

    virtual ::std::string getAdminUri(const ::Ice::Current& );

    virtual State getState(const ::Ice::Current& );

    virtual Streamer::StreamPrx createStream(const ::TianShanIce::Weiwoo::SessionPrx&,
							    const Ice::Current&);

	virtual Streamer::StreamerDescriptors listStreamers(const ::Ice::Current&);

	virtual ::std::string getNetId(const ::Ice::Current&) const;

    virtual Streamer::PlaylistPrx createPublishingPoint(const ::std::string&,
								      const ::TianShanIce::PlaylistItems&,
								      ::TianShanIce::PublishingPointType,
									  const std::string& destination, 
									  std::string& reloc, 
								      const Ice::Current&);

    virtual Streamer::PlaylistPrx openPublishingPoint(const ::std::string&,
								     const Ice::Current&);

    virtual bool deletePublishingPoint(const ::std::string&,
				       const Ice::Current&);

    virtual PublishingPointNames listPublishingPoints(const Ice::Current&);

    virtual bool getAllowClinetsToConnect(const Ice::Current&);

    virtual bool setAllowClinetsToConnect(bool,
					  const Ice::Current&);
protected:
	bool createTemporaryPlaylist(const PlaylistItems& items, 
		_bstr_t& fileName);

	bool limitPublishingPoint(IWMSPublishingPointPtr pubPt, 
		PublishingPointType type, 
		const std::string& destination);
protected:
	ZQ::common::NativeThread*	_scanThread;	
	IWMSServerPtr			_wmsServer;
	IStreamPtr				_wmsMarshalStrm;
	IMarshalPtr				_wmsMarshal;
	IceExporter&			_exporter;
	std::string				_appName;
};

//////////////////////////////////////////////////////////////////////////

class StreamEventSinkImpl : virtual public StreamEventSink
{
public:
};

class PlaylistEventSinkImpl : virtual public PlaylistEventSink,
			   virtual public ::TianShanIce::Streamer::StreamEventSinkImpl
{
public:
};

//////////////////////////////////////////////////////////////////////////

class PlaylistImpl : virtual public Playlist
{
public:

	PlaylistImpl(PublishingPointType pubPtType = OnDemandPublishingPoint);
	virtual ~PlaylistImpl();

    virtual void allotAccreditPathTicket(
		const ::TianShanIce::AccreditedPath::PathTicketPrx&,
		const Ice::Current&);

    virtual void destroy(const Ice::Current&);

    virtual ::std::string lastError(const Ice::Current&) const;

    virtual ::Ice::Identity getIdent(const Ice::Current&) const;

    virtual bool play(const Ice::Current&);

    virtual bool setSpeed(::Ice::Float,
			  const Ice::Current&);

    virtual bool pause(const Ice::Current&);

    virtual bool resume(const Ice::Current&);

    virtual ::TianShanIce::Streamer::StreamState getCurrentState(const Ice::Current&) const;

    virtual ::TianShanIce::Weiwoo::SessionPrx getSession(const Ice::Current&);


    virtual ::std::string getId(const Ice::Current&) const;

    virtual ::Ice::Int insert(::Ice::Int,
			      const ::TianShanIce::Streamer::PlaylistItemSetupInfo&,
			      ::Ice::Int,
			      const Ice::Current&);

    virtual ::Ice::Int pushBack(::Ice::Int,
				const ::TianShanIce::Streamer::PlaylistItemSetupInfo&,
				const Ice::Current&);

    virtual ::Ice::Int size(const Ice::Current&) const;

    virtual ::Ice::Int left(const Ice::Current&) const;

    virtual bool empty(const Ice::Current&);

    virtual ::Ice::Int current(const Ice::Current&) const;

    virtual void erase(::Ice::Int,
		       const Ice::Current&) const;

    virtual ::Ice::Int flushExpired(const Ice::Current&);

    virtual bool clearPending(bool,
			      const Ice::Current&);

    virtual bool isCompleted(const Ice::Current&);

    virtual ::Ice::Int findItem(::Ice::Int,
				::Ice::Int,
				const Ice::Current&);

    virtual bool distance(::Ice::Int,
			  ::Ice::Int,
			  ::Ice::Int&,
			  const Ice::Current&);

    virtual bool skipToItem(::Ice::Int,
			    bool,
			    const Ice::Current&);

public:
	bool init(const Ice::Identity& objId, IWMSPublishingPointPtr pubPt);
	void setSession(Weiwoo::SessionPrx session);
	void setPlaylistId(const std::string& playlistId);

protected:
	std::string				_lastError;
	std::string				_playlistId;
	PublishingPointType		_pubPtType;
	Weiwoo::SessionPrx		_session;

	// IWMSServerPtr		_wmsServer;
	IWMSPublishingPointPtr	_pubPt;
	IWMSPlaylistPtr			_wmsPlaylist;

	IWMSOnDemandPublishingPointPtr	_onDemondPubPt;
	IWMSBroadcastPublishingPointPtr	_broadPubPt;
};

} // namespace Streamer {
} // namespace TianShanIce {

#endif
