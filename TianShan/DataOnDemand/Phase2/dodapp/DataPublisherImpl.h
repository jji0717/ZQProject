#pragma once

#include <DODAppEx.h>
#include <Freeze/Freeze.h>
#include <TsSrm.h>
#include <TsStreamer.h>
#include "DODAppImpl.h"
#include <TsStorage.h>
#include "Locks.h"
#include "DataStream.h"
#include "DODAPPthread.h"
#include <map>
#include <string>
typedef std::map<std::string , SessionRenewThread *> SessionTrdMap;
namespace DataOnDemand {

class DataPublisherImpl : public DataPublisherEx , 
	public IceUtil::AbstractMutexI<IceUtil::RecMutex> {
public:

    virtual FolderChannelPrx createLocalFolderChannel(const ::std::string&,
								      const ChannelInfo&,
								      const ::std::string&,
								      const Ice::Current&);

    virtual FolderChannelPrx createShareFolderChannel(const ::std::string&,
								      const ChannelInfo&,
								      const Ice::Current&);

    virtual MessageChannelPrx createMsgChannel(const ::std::string&,
							       const ChannelInfo&,
							       const Ice::Current&);

    virtual ChannelPublishPointPrx getChannel(const ::std::string&,
							      const Ice::Current&);

    virtual DestinationPrx createDestination(const ::std::string&,
							     const DestInfo&,
							     const Ice::Current&);

    virtual DestinationPrx getDestination(const ::std::string&,
							  const Ice::Current&);

    virtual ::TianShanIce::StrValues listChannels(const Ice::Current&) const;

    virtual ::TianShanIce::StrValues listDestinations(const Ice::Current&) const;

    virtual void notifyFolderFullUpdate(::Ice::Int,
					::Ice::Int,
					const ::std::string&,
					bool,
					int,
					const Ice::Current&);

    virtual void notifyFolderPartlyUpdate(::Ice::Int,
					  ::Ice::Int,
					  const ::std::string&,
					  const ::std::string&,
					  int,
					  const Ice::Current&);

    virtual void notifyFolderDeleted(::Ice::Int,
				     ::Ice::Int,
				     const ::std::string&,
					 int,
				     const Ice::Current&);

    virtual void notifyFileAdded(::Ice::Int,
				 ::Ice::Int,
				 const ::std::string&,
				 const ::std::string&,
				 int,
				 const Ice::Current&);

    virtual void notifyFileModified(::Ice::Int,
				    ::Ice::Int,
				    const ::std::string&,
				    const ::std::string&,
					int,
				    const Ice::Current&);

    virtual void notifyFileDeleted(::Ice::Int,
				   ::Ice::Int,
				   const ::std::string&,
				   int,
				   const Ice::Current&);

    virtual void notifyMessageAdded(::Ice::Int,
				    ::Ice::Int,
				    const ::std::string&,
				    const ::std::string&,
				    const ::std::string&,
				    ::Ice::Long,
				    const Ice::Current&);

    virtual void notifyMessageDeleted(::Ice::Int,
				      ::Ice::Int,
				      const ::std::string&,
				      const Ice::Current&);

    virtual void removeDestination(const ::std::string&,
				   const Ice::Current&);

    virtual void removeChannel(const ::std::string&,
				   const Ice::Current&);

	virtual void activate(const ::Ice::Current& );
	virtual void reconnect(const ::Ice::Current& );

public:
	DataPublisherImpl();
	~DataPublisherImpl();
	bool init();
	static SessionRenewThread * getSessionTrd(std::string destname);

protected:
	inline DataPublisherExPrx getThisProxy(
		const Ice::ObjectAdapterPtr& adapter);
	bool checkSync(bool bReconnect = false);

	// Ice::IdentitySeq findChannelByType(ChannelType type);

public:
	static Ice::CommunicatorPtr		_ic;
	static Ice::ObjectAdapterPtr	_adapter;
	static ::Freeze::EvictorPtr		_evictor;
	// static ChannelTypeIndexPtr		_channelTypeIndex;
	static DODAppServicePtr			_dodappservice;
	static TianShanIce::SRM::SessionManagerPrx		_sessManager;
    static TianShanIce::Storage::ContentStorePrx    _contentStroe;
	static	SessionTrdMap			_SessionTrdMap;
	static  CreatDestionTrd*		_pCreatDestTrd;
protected:
	DataPublisherExPrx				_thisPrx;
};
}
