#ifndef __ZQ_DATATUNNEL_DATAAPPEX_ICE__
#define __ZQ_DATATUNNEL_DATAAPPEX_ICE__

/*
$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I$(ZQProjsPath)/DataTunnel/Ice $(InputPath)  
$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I$(ZQProjsPath)/DataTunnel/Ice --index "DataOnDemand::ChannelTypeIndex,DataOnDemand::ChannelPublishPointEx,myType" ChannelTypeIndex $(InputPath)
*/

#include "TsAppDOD.ICE"
#include "DataStream.ice"
[["cpp:include:list"]]
// 增加通过 groudId 查找 destination 的 index

module TianShanIce
{

// -----------------------------
// namespace Application
// -----------------------------
/// Application represents the basic entry definition of a business application to TianShan architecture
module Application
{

/// namespace of DataOnDemand
module DataOnDemand {


class DataStreamEx;
class DataPointPublisherEx;

// -----------------------------
// struct ChannelToDestAssoc
// -----------------------------
/// linkage from to find the destionation by channel
struct DataPPToDataStreamAssoc
{
	string 			destName;
	DataStreamEx*	dest;
	int				lastUpdate;	
};

/// linkage collection to find the destionation by channel
dictionary<string, DataPPToDataStreamAssoc> DataStreamLinks;
dictionary<int   , string>GroupIdContentName;
["freeze:write"]
class FolderEx extends Folder {

	/// how can i get the cache of the ts package
	void getCacheInfo(out TianShanIce::Streamer::DataOnDemand::CacheType type, out string addres);
	DataStreamLinks getDataStreamLinks();

	void linkDataStream(string name, DataStreamEx* dest, long lastUpdate);
	void unlinkDataStream(string name);
	string getContentName(int groupId);
	void   setContentName(int groupId, string contentname);

	void activate();
	
	long						myLastUpdate;
	DataPublishPointInfo		myInfo;
	DataStreamLinks			    myDataStreamLinks;
	DataPointPublisherEx*		myParent;
    GroupIdContentName			myGroupCName;	
};
["freeze:write"]
class MessageQueueEx extends MessageQueue {

	/// how can i get the cache of the ts package
	void getCacheInfo(out TianShanIce::Streamer::DataOnDemand::CacheType type, out string addres);
	DataStreamLinks getDataStreamLinks();

	void linkDataStream(string name, DataStreamEx* dest, long lastUpdate);
	void unlinkDataStream(string name);
	void activate();
	
	long						myLastUpdate;
	DataPublishPointInfo		myInfo;
	DataStreamLinks				myDataStreamLinks;
	DataPointPublisherEx*		myParent;
};

dictionary<string, DataAttachInfo > AttachedInfoDict;
dictionary<string, FolderEx* > FolderDataDict;
dictionary<string, MessageQueueEx* > MsgQueueDict;
["freeze:write"]
class DataStreamEx extends DataStream {	
	void removeDataPublishPoint(string name);
	void activate();
	DataStreamInfo getInfo();
	
	/// weiwoo, stores the server session's proxy
	TianShanIce::SRM::Session*	   weiwooSession;		
	DataStreamInfo				   myInfo;
	DataPointPublisherEx*		   myParent;
	AttachedInfoDict			   myDataPublishPoints;
	TianShanIce::Streamer::StreamState          myState;
};

dictionary<string, DataStreamEx*> DataStreamDict;

["freeze:write"]
class DataPointPublisherEx extends DataPointPublisher {

	void removeDataStream(string name);
	void removeDataPublishPoint(string name);

	void activate();
	void reconnect();
	
	DataStreamDict				dataStreams;
	MsgQueueDict				msgQueues;
	FolderDataDict				shareFolderDatas;
	FolderDataDict				localFolderDatas;
};

struct messageinfo
{
  	///It denote a order number in the queue.
	///bForever=TRUE,means= forever;else has duration
	bool bForever;	
   //each message deleteTime;
   	long  deleteTime;	
	// messageid and nDataType identify message
	string messageID;	
	string messageBody;
	int GroupId;
	string contentfilename;	
};
["cpp:type:std::list<messageinfo>"]
sequence <messageinfo>MessageInfos;

};/// module DataOnDemand
};/// module Application
};/// module TianShanIce

#endif // __ZQ_DATATUNNEL_DATAAPPEX_ICE__
