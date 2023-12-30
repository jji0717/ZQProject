// PurchaseImpl.h: interface for the PurchaseImpl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PURCHASEIMPL_H__7DB27C05_C3E3_4ECF_9779_90D520B977A6__INCLUDED_)
#define AFX_PURCHASEIMPL_H__7DB27C05_C3E3_4ECF_9779_90D520B977A6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "ChODDefines.h"
#include "ChannelOnDemandEx.h"
#include "ChannelItemDict.h"
#include "ChODSvcEnv.h"

#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>

namespace ZQChannelOnDemand {

class PurchaseImpl  : public NS_PREFIX(ChannelOnDemand::ChannelPurchaseEx), public IceUtil::AbstractMutexI<IceUtil::RecMutex>
{
public:
	PurchaseImpl(ChODSvcEnv& env);
	virtual ~PurchaseImpl();

public: // functions defined in TianShanIce::Application::Purchase in file TsApplication.ICE.
	virtual ::TianShanIce::SRM::SessionPrx getSession(const ::Ice::Current& = ::Ice::Current()) const;
	virtual void provision(const ::Ice::Current& = ::Ice::Current());
	virtual void render(const ::TianShanIce::Streamer::StreamPrx&, const ::TianShanIce::SRM::SessionPrx&, const ::Ice::Current& = ::Ice::Current());	
	virtual void detach(const ::std::string&, const ::TianShanIce::Properties&, const ::Ice::Current& = ::Ice::Current());
	virtual void bookmark(const ::std::string&, const ::TianShanIce::SRM::SessionPrx&, const ::Ice::Current& = ::Ice::Current());
	virtual ::Ice::Int getParameters(const ::TianShanIce::StrValues&, const ::TianShanIce::ValueMap&, ::TianShanIce::ValueMap&, const ::Ice::Current& = ::Ice::Current()) const;
	virtual ::TianShanIce::Application::PlaylistInfo PurchaseImpl::getPlaylistInfo(const ::Ice::Current& c) const;

public: // functions defined in ChannelOnDemand::ChannelPurchase in file ChannelOnDemand.ICE.
	virtual ::TianShanIce::Streamer::PlaylistPrx getPlaylist(const ::Ice::Current& = ::Ice::Current());
    virtual ::std::string getPlaylistId(const ::Ice::Current& = ::Ice::Current());
	virtual ::std::string getChannelName(const ::Ice::Current& = ::Ice::Current());
	virtual bool getIfNeedSyncChannel(const ::Ice::Current& = ::Ice::Current());
	virtual bool getIfNeedAuthorize(const ::Ice::Current& = ::Ice::Current());
	virtual void destroy(const ::TianShanIce::Properties& params, const ::Ice::Current& = ::Ice::Current());
	virtual bool isInService(const ::Ice::Current& = ::Ice::Current());

public: // functions defined in ChannelOnDemand::ChannelPurchaseEx in file ChannelOnDemandEx.ICE.
	::std::string getClientSessionId(const ::Ice::Current& = ::Ice::Current());

	::std::string getServerSessionId(const ::Ice::Current& = ::Ice::Current());

	bool appendPlaylistItem(
		const NS_PREFIX(ChannelOnDemand::ChannelItemEx)& appendChnlItem, 
		const ::Ice::Current& = ::Ice::Current());

	bool insertPlaylistItem(
		const ::std::string& insertPosKey, 
		const NS_PREFIX(ChannelOnDemand::ChannelItemEx)& insertChnlItem, 
		const ::Ice::Current& = ::Ice::Current());

	bool removePlaylistItem(
		const ::std::string& removeItemKey, 
		const ::Ice::Current& = ::Ice::Current());

	bool replacePlaylistItem(
		const ::std::string& oldItemKey, 
		const NS_PREFIX(ChannelOnDemand::ChannelItemEx)& replaceChnlItem, 
		const ::Ice::Current& = ::Ice::Current());

	// this function is used to sync the playlist according to the channel list
	// usually, when receiving an event of End-of-Item, syncPlaylist will be called
	bool syncPlaylist(const ::Ice::Current& = ::Ice::Current());

	void setProperties(const std::string& key, const std::string& value);

	::TianShanIce::Properties getProperties();

private: 
	// convert given userctrlnum and offset to utc time
	void getUtcTime(const ::Ice::Int ctrlNum, const ::Ice::Int offSet, ::std::string& utcTime) const;

	// convert given utc time to userctrlnum and offset
	void getStreamPos(const ::std::string& utcTime, ::Ice::Int& ctrlNum, ::Ice::Int& offSet, ::Ice::Int& startPos) const;

	// do todas session teardown work
	void todasTeardown(const ::TianShanIce::Properties& params);

	// get stream's current play position. UserCtrlNum and Offset
	void getCurStreamPos(::Ice::Int& ctrlNum, ::Ice::Int& offset) const;

	void copyChannelItemToSetupInfo(const NS_PREFIX(ChannelOnDemand::ChannelItemEx)& chnlItemInfo, 
		TianShanIce::Streamer::PlaylistItemSetupInfo& setupInfo);

	// map the channel item to purchase item identity
	// channelItem[in], the channel item name
	// piIdent[out], the purchase item identity
	bool ChnlItem2PItemIdent(const ::std::string& channelItem, Ice::Identity& piIdent) const;

private: 
	ChODSvcEnv&		_env;
};

typedef ::IceInternal::Handle< PurchaseImpl> PurchaseImplPtr;

} // namespace ZQChannelOnDemand;

#endif // !defined(AFX_PURCHASEIMPL_H__7DB27C05_C3E3_4ECF_9779_90D520B977A6__INCLUDED_)
