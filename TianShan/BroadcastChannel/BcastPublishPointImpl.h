#ifndef __BcastPublishPointImpl_h__
#define __BcastPublishPointImpl_h__

#include <BcastChannelEx.h>
#include <Freeze/Freeze.h>
#include <IceUtil/IceUtil.h>
#include "soapsoapMRTProxy.h"
#include <TianShanDefines.h>
namespace ZQBroadCastChannel
{ 
class BroadCastChannelEnv;

class BcastPublishPointImpl : public TianShanIce::Application::Broadcast::BcastPublishPointEx,
				            //public IceUtil::AbstractMutexWriteI<IceUtil::RWRecMutex>
				            public ICEAbstractMutexWLock
{
  public:
	  typedef ::IceInternal::Handle< BcastPublishPointImpl> Ptr;
	  BcastPublishPointImpl(BroadCastChannelEnv& bcastChenv);
	  ~BcastPublishPointImpl();
 public:
	 ///implement publishpoint
	virtual ::std::string getType(const Ice::Current&)const;

	virtual ::std::string getName(const Ice::Current&)const;

	virtual ::std::string getDesc(const Ice::Current&)const;

	virtual ::Ice::Int getMaxBitrate(const Ice::Current&)const;

	virtual void setMaxBitrate(::Ice::Int,
		const Ice::Current&);

	virtual void setProperties(const TianShanIce::Properties&,
		const Ice::Current&);

	virtual void setDesc(const ::std::string&,
		const Ice::Current&);

	virtual TianShanIce::Properties getProperties(const Ice::Current&)const;

	virtual void destroy(const Ice::Current&);

	virtual void restrictReplica(const TianShanIce::StrValues&,
		const Ice::Current&);

	virtual TianShanIce::StrValues listReplica(const Ice::Current&)const;

	///implement broadcast publishpoint

	virtual ::Ice::Long requireResource(TianShanIce::SRM::ResourceType,
		const TianShanIce::SRM::Resource&,
		const Ice::Current&);

	virtual TianShanIce::SRM::ResourceMap getResourceRequirement(const Ice::Current&)const;

	virtual void withdrawResourceRequirement(TianShanIce::SRM::ResourceType,
		const Ice::Current&);

	virtual void setup(const Ice::Current&);

	virtual TianShanIce::SRM::SessionPrx getSession(const Ice::Current&);

	virtual void start(const Ice::Current&);

	virtual void stop(const Ice::Current&);

	virtual ::Ice::Long getUpTime(const Ice::Current&);

	virtual void renew(Ice::Long TTL, const Ice::Current &);
	 
	virtual ::Ice::Long getExpiration(const Ice::Current &);

	virtual bool isPersistent(const Ice::Current &);
	///implement channelpublishpoint
	virtual TianShanIce::StrValues getItemSequence(const Ice::Current&)const;

	virtual TianShanIce::Application::ChannelItem findItem(const ::std::string&,
		const Ice::Current&)const;

	virtual void appendItem(const TianShanIce::Application::ChannelItem&,
		const Ice::Current&);

	virtual void appendItemAs(const TianShanIce::Application::ChannelItem&,
		const ::std::string&,
		const Ice::Current&);

	virtual void insertItem(const ::std::string&,
		const TianShanIce::Application::ChannelItem&,
		const Ice::Current&);

	virtual void insertItemAs(const ::std::string&,
		const TianShanIce::Application::ChannelItem&,
		const ::std::string&,
		const Ice::Current&);

	virtual void replaceItem(const ::std::string&,
		const TianShanIce::Application::ChannelItem&,
		const Ice::Current&);

	virtual void removeItem(const ::std::string&,
		const Ice::Current&);

protected:
	bool setupPlaylist();
	bool destroyChannelItem();
	void copyChannelItemToSetupInfo(const TianShanIce::Application::Broadcast::ChannelItemEx& chnlItemInfo, TianShanIce::Streamer::PlaylistItemSetupInfo& setupInfo);
	bool ChnlItem2ChItemAssocIdent(const ::std::string& channelItemKey, std::vector<Ice::Identity>& chItemassociIdents) const;
	void resetchannel();
	bool checkChannelItem(TianShanIce::Application::ChannelItem& channelItem);
	bool destroyChannelItemAssoc();
	bool removeChItemAssocByCtrlNumber(int removeCtrlNum);
	bool getRandomFilterItem(TianShanIce::Application::Broadcast::ChannelItemEx& filterItem);
	bool addFilterItemToPlaylist();
	bool rebuildPlaylist(TianShanIce::StrValues& useChannelItems, int needAddItemCount,Ice::Long previousBcastStart);

public:
	virtual void OnEndOfStream(const ::std::string&,const Ice::Current&);

	virtual void OnStreamExit(const ::std::string&,const Ice::Current&);

	virtual void OnEndOfItem(Ice::Int,const Ice::Current&);

	virtual bool appendPlaylistItem(const TianShanIce::Application::Broadcast::ChannelItemEx&,
		const Ice::Current&);

	virtual bool insertPlaylistItem(const ::std::string&,
		const TianShanIce::Application::Broadcast::ChannelItemEx&,
		const Ice::Current&);

	virtual bool removePlaylistItem(const ::std::string&,
		const Ice::Current&);

	virtual bool replacePlaylistItem(const ::std::string&,
		const TianShanIce::Application::Broadcast::ChannelItemEx&,
		const Ice::Current&);

	// this function is used to sync the playlist according to the channel list
	// usually, when receiving an event of End-of-Item, syncPlaylist will be called
	virtual bool syncPlaylist(const Ice::Current&);

	virtual bool isNOVDMainCh(const Ice::Current&);
	virtual bool isNOVDSuppCh(const Ice::Current&);
	virtual bool isInService(const ::Ice::Current& = ::Ice::Current());
	virtual bool activate(const ::Ice::Current& = ::Ice::Current());
	virtual void pingStream(const ::Ice::Current& = ::Ice::Current());

	TianShanIce::Application::Broadcast::BcastPublishPointExPrx getBcastPublishPointEx()const;

protected:
	 BroadCastChannelEnv& _env;
private:
	bool setupMRTStream();
	bool tearDownMRTStream();
	bool getStatusMRTStream();
	void logSoapErrorMsg(const soapMRT& soapMRTClient);

};
}
#endif ///end define  __BcastPublishPointImpl_h__
