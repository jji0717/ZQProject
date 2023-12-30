#ifndef _STREAMSMITH_MODULE_EX_H__
#define _STREAMSMITH_MODULE_EX_H__

#include <StreamSmithModule.h>
#include <Ice/Ice.h>
#include <TianShanDefines.h>

enum StreamResultMask
{
	
	GET_NPT_CURRENTPOS		= 1 << 0,
	GET_NPT_TOTALPOS		= 1 << 1,

	GET_ITEM_CURRENTPOS		= 1 << 2,
	GET_ITEM_TOTALPOS		= 1 << 3,

	GET_SPEED				= 1 << 4,
	GET_STATE				= 1 << 5,
	GET_USERCTRLNUM			= 1 << 6
};

typedef struct _tagStreamControlResultInfo 
{
	ULONG				flag;
	ULONG				userCtrlNum;
	LONG				timeOffset;
	LONG				totalOffset;
	LONG				itemTimeOffset;
	LONG				itemTotalOffset;
	IPlaylist::State	plState;
	float				speed;
	std::map<std::string , std::string> extraProperties;
	_tagStreamControlResultInfo ( )
	{
		flag	=	0;//means do not get timeOffset and totalOffset
	}
}StreamControlResultInfo ;

typedef struct _tagSpigotInfo 
{
	Ice::Long	stampFirstUp;
	Ice::Long	stampUpdate;
	bool		bDisable;
	int			spigotIndex;	
}SpigotInfo;
typedef std::vector<SpigotInfo>	SpigotsInfo;

class IPlaylistEx:public IPlaylist
{
public:
	virtual int32	lastExtErrorCode( ) const = 0;
	///set last error to playlist
	///@param lastErr last error description string to be set
	virtual void	setLastError(std::string& lastErr) = 0;

	///set stream service port
	///@param uPort the service port of a specify stream,0 if any
	virtual void	setStreamServPort(unsigned short uPort) = 0;

	//set poke hole session ID
	virtual bool	setPokeHoleSessionID( const std::string& strPokeHoleSessID  ) = 0;

	virtual void	setPlaylistAttributes( const TianShanIce::ValueMap& values ) = 0;

	virtual bool	exPause( OUT StreamControlResultInfo& info ) = 0;

	virtual bool	exPlay( IN float newSpeed ,IN ULONG offset , IN short from , OUT StreamControlResultInfo& info ) = 0;	

	virtual bool	exPlayItem( IN CtrlNum userCtrlNum , IN float newSpeed ,IN ULONG offset , IN short from , OUT StreamControlResultInfo& info  ) = 0;

	virtual void	setStreamPID(int pid) = 0;

	virtual bool	commit( ) = 0;
};

struct PlaylistAttr
{
    ::std::string		Guid;
    ::std::string		StreamSmithSiteName;
    ::std::string		ResourceGuid;
    ::std::string		ClientSessionID;
    ::std::string		endPoint;
    long				MaxRate;
    long				MinRate;
    long				NowRate;
    ::std::string		destIP;
	long				destPort;
    ::std::string		destMac;
    long				vstrmPort;
    long				programNumber;
    int					playlistState;
    long				currentCtrlNum;
    long				vstrmSessID;
	long				streamPID;
};

typedef bool	(*EventDispatchEvent)(DWORD eventType,ZQ::common::Variant& params,void* pExtraData);

class IPlaylistManager
{
public:
	///list all available playlist with it's guid(in string)
	///@param [INOUT] IDs all playlist guid string will be included in this var
	virtual void	listPlaylistGuid(::std::vector<::std::string>& IDs)=0;

	///add a new playlist into failover database
	///@param [IN] new playlist's guid in string
	///@param [IN] new playlist attribute
	virtual void	addNewPlaylistIntoFailOver(const std::string& strGuid ,	const PlaylistAttr& attr)=0;

	///create a new playlist with guid 
	///@return a pointer to IPlaylistEx
	///@param [IN] uid new guid 
	virtual	IPlaylistEx* CreatePlaylist(const std::string& uid,const std::vector<int>& SpigotID , const std::string& clientSessId )=0;

	///find a playlistex with guid
	virtual IPlaylistEx* find(const std::string uidString)=0;

	///register a event sink with a eventsink ice proxy string
	///@param [IN] type the event type for register
	///@param [IN] eDispatch event dispatch function
	///@param [IN] pExtraData user define extra data
	virtual	void		registerEventSink(DWORD type,EventDispatchEvent eDispatch,void* pExtraData)=0;

	///list current node streamers
	virtual void		listStreamers(std::vector<int >& SpigotIDs)=0;

	virtual bool		GetSpigotIDsFromResource(int serviceGroupID,int MaxBitRate,std::vector<int>& SpigotsIdOut)=0;

	virtual int			getIceEvitorPLaylistSize() =0;
	
	virtual int			getIceEvitorPLaylistItemSize() =0;

	virtual int			getPlaylistCount() =0;
	virtual int			getSuicidePlCount() =0;

	virtual void		getReplicaInfo(const std::string& category , const std::string& groupId , bool bLocalOnly , TianShanIce::Replicas& res ) = 0;
	
};


////define  entry for super plug-in
///Entry name	initializeSuper
typedef	bool	(*initializeSuper)(IPlaylistManager* plMan,ZQADAPTER_DECLTYPE adapter , std::string,char*, ZQ::common::Log*,const std::vector<int>&);
//IPlaylistManager* ,::Ice::ObjectAdapterPtr ,std::string,char*,ZQ::common::Log*,const std::vector<int>&
///Entry name	UninitializeSuper
typedef	bool	(*UninitializeSuper)(ZQADAPTER_DECLTYPE adapter);


#define	PLI_CATALOG		"ItemCatalog"
#define PL_CATALOG		"PlayList"




#endif//_STREAMSMITH_MODULE_EX_H__