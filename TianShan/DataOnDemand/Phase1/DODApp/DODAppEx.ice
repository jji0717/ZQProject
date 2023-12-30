#ifndef __ZQ_CHANNELONDEMAND_DODAPPEX_ICE__
#define __ZQ_CHANNELONDEMAND_DODAPPEX_ICE__

/*
$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I$(ZQProjsPath)/DataOnDemand/Ice $(InputPath)  
$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I$(ZQProjsPath)/DataOnDemand/Ice --index "DataOnDemand::ChannelTypeIndex,DataOnDemand::ChannelPublishPointEx,myType" ChannelTypeIndex $(InputPath)
*/

#include "DODApp.ice"
#include "DataStream.ice"

// 增加通过 groudId 查找 destination 的 index

/// namespace of DataOnDemand
module DataOnDemand {


class DestinationEx;
class DataPublisherEx;

// -----------------------------
// struct ChannelToDestAssoc
// -----------------------------
/// linkage from to find the destionation by channel
struct ChannelToDestAssoc
{
	string 			destName;
	DestinationEx*	dest;
	int				lastUpdate;	
};


/// linkage collection to find the destionation by channel
dictionary<string, ChannelToDestAssoc> DestLinks;

class FolderChannelEx extends FolderChannel {

	/// how can i get the cache of the ts package
	void getCacheInfo(out CacheType type, out string addres);
	DestLinks getDestLinks();

	void linkDest(string name, DestinationEx* dest, long lastUpdate);
	void unlinkDest(string name);
	void activate();
	
	long						myLastUpdate;
	ChannelInfo					myInfo;
	DestLinks					myDestLinks;
	DataPublisherEx*			myParent;
	TianShanIce::Properties		myProps;
	ChannelType					myType;
};

class MessageChannelEx extends MessageChannel {

	/// how can i get the cache of the ts package
	void getCacheInfo(out CacheType type, out string addres);
	DestLinks getDestLinks();

	void linkDest(string name, DestinationEx* dest, long lastUpdate);
	void unlinkDest(string name);
	void activate();
	
	long						myLastUpdate;
	ChannelInfo					myInfo;
	DestLinks					myDestLinks;
	DataPublisherEx*			myParent;
	TianShanIce::Properties		myProps;
	ChannelType					myType;
};

struct AttachedInfo {
	ChannelPublishPoint*	channel;
	int						minBitRate;
	int						repeatTime;
};

dictionary<string, AttachedInfo > AttachedInfoDict;
dictionary<string, FolderChannelEx* > FodChannelDict;
dictionary<string, MessageChannelEx* > MsgChannelDict;

class DestinationEx extends Destination {
	
	void removeChannel(string name);
	void activate();
	void getAttachedInfo(string chName, out AttachedInfo info);
	
	DestInfo					myInfo;
	DataPublisherEx*			myParent;
	AttachedInfoDict			myChannels;
	TianShanIce::Properties		myProps;
	DestState					myState;
};

dictionary<string, DestinationEx*> DestDict;

class DataPublisherEx extends DataPublisher {

	void removeDestination(string name);
	void removeChannel(string name);

	void activate();
	void reconnect();
	
	DestDict					myDests;
	MsgChannelDict				myMsgChannels;
	FodChannelDict				myShaFodChannels;
	FodChannelDict				myLocFodChannels;
	TianShanIce::Properties		myProps;
};

}; /// module DataOnDemand

#endif // __ZQ_CHANNELONDEMAND_DODAPP_ICE__
