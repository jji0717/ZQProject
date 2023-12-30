#ifndef __ZQ_TianShanIce_StreamService_Playlist_ICE__
#define __ZQ_TianShanIce_StreamService_Playlist_ICE__

#include "StreamService.ICE"

module TianShanIce
{
module Streamer
{

///StreamingResource is a structure used to identify the required resource for streaming
///and this resource is read only after stream is set up
///
struct StreamingResource
{
	ValueMap		resource;
};

dictionary< int, long>	OperationProfileS;
///         id  , time cost
///OperationProfileS is used to record each operation time cost and only for debug and profile use

struct ItemSessionInfo
{	
	//setup information which is defined in tsStreamer.ice
	PlaylistItemSetupInfo		setupInfo; 
	
	//user ctrl num which is used to identify a item
	//this member should be unique in playlist wide
	///NOTE that -2 is reserved for Invalid user control number
	int							userCtrlNum;	
	
	/////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////
	//////																			/////
	//////																			/////
	//These attribute is only used at session run time
	
	//runtime session bitrate	
	int							muxBitrate;
	
	//how many times current item is loaded
	int							loadedCount;
	
	//loaded time 
	long						timeLoaded;
	
	//launched time
	long						timeLaunched;
	
	//unload time or expired time
	long						timeUnloaded;
	
	///runtime item time offset
	///This is the real running timeoffset
	long						runningOffset;
	
	//runtime session Id
	string						sessionId;
	
	///real play duration
	long						itemRealDuration;
	
	///item play total duration
	long						itemTotalDuration;
	
	///item session state
	StreamState					sessionState;	

	///item play times count
	long                        itemPlayTimesCount;
	/////																			/////
	/////																			/////
	/////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////
	
	
	/////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////
	/////																			/////
	/////																			/////
	///item real in time offset, this is the offset relative to the beginning of the item
	long						itemInTimeOffset;
	
	///item real out time offset, this is the offset relative to the beginning of the item
	/// <=0 for the actual end of item
	/// NOTE if the item is being ingested , the itemOutTimeOffset is changing when it's wanted if itemOutTimeOffset is valid
	long						itemOutTimeOffset;
	
	/// if itemOutTimeOffset > 0 , then the real item playduration is itemOutTimeOffset - itermInTimeOffset
	/// and real item playduration should <= itemTotalDuration
	
	/////																			//////
	/////																			//////
	//////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////
	
	///session flag is used at runtime to store some useful information
	long						sessFlag;
	
	ValueMap					privateData;
	
	///是否需要将profile数据存入到数据库呢？
	///实际上StreamService的数据库只是一个runtime的数据记录，一旦这个stream被destroy掉以后。
	///其数据就会被销毁，那么到时候这个profile数据也就不存在了
	OperationProfileS			itemProfiles;
};

sequence<ItemSessionInfo> ItemSessionInfoS;


///不再使用SpeedDef，转而使用Float。
///转换过程由porting层来完成
//struct SpeedDef
//{
//	short	numerator;
//	short	denominator;
//};

enum Timertype
{
	TIMERNULL,
	//renew PathManager Ticket if available
	//issue a destroy command if we get an ObjectNotExist Exception when renew ticket
	TIMERRENEWTICKET,		//to renew pathticket if available
		
	///destroy playlist command	
	TIMERPLAYLISTDESTROY,	//to destroy current playlist
		
	TIMERDPC				//defer precedure call
};

["freeze:write"] 
class SsPlaylist  extends  PlaylistEx
{
	//stream target ip address 
	//can be IPv4 or IPV6 address string format
	string						targetIpAddress;
	
	//stream target udp port
	int							targetIpPort;
	
	string						targetMac;
	
	string						sourceIpAddress;
	
	int							sourcePort;
	
	int							maxMuxRate;	
	
	int							nowMuxRate;
	
	int							minMuxRate;
	//record allocated streaming resources
	//these resoucres should be released when playlist is gone
	//StreamingResource			streamResource;

	string						streamerReplicaId;
	
	//speed	
	float						speed; //This value should always be positive
		
	///playlist state
	///
	StreamState					playlistState;
			
	//last stream operation time
	///这个数据用来标识最后一个主动的操作发生的时间
	long						lastOperationTime;
	
	///current item's control number 
	///maybe the item is not being pumped
	///这个是当前文件指针所指向的item的index
	///即便这个item没有被播放	
	int							currentItemCtrlNum;
	int							nextItemCtrlNum;
	
	///我不是很确定使用要记录last error string and error code
	///先把这个数据放在这里			
	string						lastErrorString;
	
	int							lastErrorCode;
	
	///item数据集	
	ItemSessionInfoS			items;
	
	ConditionalControl*			condControl;
	
	ConditionalControlMask		condCtrlMask;
	
	SRM::Session*				srmSession;
	
	Transport::PathTicket*		pathTicket;
	
	///是否启动EOT保护
	bool						bEnableEOT;
	
	//identify current playlist is alive or not
	bool						bAlive;
	bool						bCommited;
	
	bool						bFirstPlay;

	
	TianShanIce::SRM::ResourceMap	crResource; //client request resource map
		
	///playlist的私有属性
	///你可以在这里填入你自己想要填入的任何值
	///不过需要注意的是以 sys. 开头的属性是library保留的属性
	///请确保你不会覆盖和任意修改这些属性
	TianShanIce::Properties		property;

	//ValueMap					attributes;

	///和上面一样，我还不确定是否需要把这个放在数据库记录里面
	OperationProfileS			opProfiles;
	
	
//	["freeze:read","cpp:const"] 
//	StreamingResource			getStreamingResource( );
		
	///event generator inform playlist that some item session has new state
	void						onSessionStateChanged( StreamState newSessionState , string itemSessionId , long timeStamp ,TianShanIce::Properties props );	
	
	///event generator inform playlist that some item session has new speed
	void						onSessionSpeedChanged( float newSpeed , string itemSessionId ,long timeStamp, TianShanIce::Properties props  );
	
	///event generator inform playlist that some item session has been expired
	void						onSessionExpired( string itemSessionId ,long timeStamp ,TianShanIce::Properties props  );
	
	void						onSessionProgress( string itemSessionId , long curTimeOffset , long totalDuration ,TianShanIce::Properties props );
	
	///timer inform playlist that new timer event is coming
	void						onTimer( Timertype type );
	
	///renew path ticket if available
	bool						renewTicket( int interval );

	///restore session when recover from DB
	void						onRestore( );
	
	["freeze:read","cpp:const"] 
	bool						getRunningSession( out string currentSession , out string nextSession , out float speed, out StreamState state );
	
	["freeze:read","cpp:const"] 
	string						getAttribute( string key );
	
	["freeze:read","cpp:const"] 
	Transport::PathTicket*		getPathTicket( );
};

sequence<SsPlaylist*> SsPlaylistS;

interface StreamServiceAdmin extends StreamSmithAdmin
{
	///list all current running session
	SsPlaylistS		listSessions( );
};

};
};

#endif //__ZQ_TianShanIce_StreamService_Playlist_ICE__
