#pragma once

#include <DataAppEx.h>
#include <Freeze/Freeze.h>
#include <TsSrm.h>
#include <TsStreamer.h>
#include "DataAppImpl.h"
#include <TsStorage.h>
#include "Locks.h"
#include "DataStream.h"
#include "DataAPPthread.h"
#include <map>
#include <string>
#include "messagemanage.h"
typedef std::map<std::string , SessionRenewThread *> SessionTrdMap;
namespace TianShanIce {
namespace Application {
namespace DataOnDemand {

	class DataPointPublisherImpl : public TianShanIce::Application::DataOnDemand::DataPointPublisherEx , 
	public IceUtil::AbstractMutexI<IceUtil::RecMutex> {
public:
	//impl PublishPoint

	virtual ::TianShanIce::Application::PublishPointPrx publish(const ::std::string&,
		::Ice::Int,
		const ::std::string&,
		const Ice::Current&);

	virtual ::TianShanIce::Application::PublishPointPrx open(const ::std::string&,
		const Ice::Current&);

	virtual ::TianShanIce::StrValues list(const Ice::Current&);

	virtual void listPublishPointInfo_async(const ::TianShanIce::Application::AMD_PointPublisher_listPublishPointInfoPtr&,
		const ::TianShanIce::StrValues&,
		const Ice::Current&) const;

   /// impl DataPublishPoint
	virtual FolderPrx createLocalFolderPublishPoint(const ::std::string&,
		const DataPublishPointInfo&,
		const ::std::string&,
		const ::std::string&,
		const Ice::Current&);

	virtual FolderPrx createShareFolderPublishPoint(const ::std::string&,
		const DataPublishPointInfo&,
		const ::std::string&,
		const Ice::Current&);

	virtual MessageQueuePrx createMessageQueue(const ::std::string&,
		const DataPublishPointInfo&,
		const ::std::string&,
		const Ice::Current&);

	virtual DataPublishPointPrx openDataPublishPoint(const ::std::string&,
		const Ice::Current&);

	virtual DataStreamPrx broadcast(const ::std::string&,
		const ::TianShanIce::SRM::ResourceMap&,
		const ::TianShanIce::Properties&,
		const ::std::string&,
		const Ice::Current&);

	virtual DataStreamPrx openDataStream(const ::std::string&,
		const Ice::Current&);

	virtual DataPublishPointInfos listDataPublishPoints(const ::std::string&,
		const Ice::Current&);

	virtual DataStreamInfos listDataStreams(const ::std::string&,const Ice::Current&);

	virtual void OnDataEvent_async(const ::TianShanIce::Application::DataOnDemand::AMD_DataPointPublisher_OnDataEventPtr&,
		::TianShanIce::Application::DataOnDemand::DataEvent,
		const ::TianShanIce::Properties&,
		const Ice::Current&);

    virtual void removeDataStream(const ::std::string&,
				   const Ice::Current&);

    virtual void removeDataPublishPoint(const ::std::string&,
				   const Ice::Current&);

	virtual void activate(const ::Ice::Current& );
	virtual void reconnect(const ::Ice::Current& );

public:
	DataPointPublisherImpl();
	~DataPointPublisherImpl();
	bool init();
	static SessionRenewThread * getSessionTrd(std::string destname);

protected:
	inline DataPointPublisherExPrx getThisProxy(
		const Ice::ObjectAdapterPtr& adapter);
	bool checkSync(bool bReconnect = false);

	virtual void onFolderFullUpdate(const ::TianShanIce::Properties&);

	virtual void onFolderPartlyUpdate(const ::TianShanIce::Properties&);

	virtual void onFolderDeleted(const ::TianShanIce::Properties&);

	virtual void onFileAdded(const ::TianShanIce::Properties&);

	virtual void onFileModified(const ::TianShanIce::Properties&);

	virtual void onFileDeleted(const ::TianShanIce::Properties&);

	virtual void onMessageAdded(const ::TianShanIce::Properties&);

	virtual void onMessageDeleted(const ::TianShanIce::Properties&);

public:
	static Ice::CommunicatorPtr		_ic;
	static Ice::ObjectAdapterPtr	_adapter;
	static ::Freeze::EvictorPtr		_evictor;
	static DataTunnelServicePtr			_dodappservice;
	static TianShanIce::SRM::SessionManagerPrx		_sessManager;
    static TianShanIce::Storage::ContentStorePrx    _contentStroe;
	static	SessionTrdMap			_SessionTrdMap;
	static MessageManage*			_messagemanage;
protected:
	DataPointPublisherExPrx				_thisPrx;
};
} // END DataOnDemand
} // END Application
} // END TianshanICE