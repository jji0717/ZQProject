#ifndef __Playlist_H__
#define __Playlist_H__


#include <vector>
#include <StreamSmithModuleEx.h>
#include <Locks.h>
#include <Exception.h>

#include "StreamSmith.h"
#include "VstrmClass.h"

#include "VstrmSessionMonitor.h"

#include "PollingTimer.h"
#include "global.h"
#include "ResourceManager.h"
#include <vstrmver.h>

#ifdef _ICE_INTERFACE_SUPPORT
	#define  _WIN32_WINNT 0x0400
	#include <TianShanDefines.h>
	#include "FailStorePlInfo.h"
#if ICE_INT_VERSION / 100 < 303
	#include <ice\IdentityUtil.h>
#endif

	#include <playlistexi.h>
	#include <checkContent.h>
#endif//_ICE_INTERFACE_SUPPORT



#if ( VER_PRODUCTVERSION_MAJOR  == 6 && VER_PRODUCTVERSION_MINOR >= 3  ) || (VER_PRODUCTVERSION_MAJOR>6)

#define VSTRM_ONDEMANDE_SESSID_ON

#endif//VSTRMONDEMANDSESSID

namespace ZQ{
namespace StreamSmith {
	

#define INVALID_Item_STATE	(-1)
#define DUMMP_PORT_ID		5
#define DEFAULT_PL_EXPIRE   (60*1000*30) // playlist will expired if no actions for 30 min
//#define DEFAULT_PL_EXPIRE   (1000*10) // for debug

class PlaylistManager;
class StreamSmithSite;
class PlaylistFailStoreThread;

// -----------------------------
// class Playlist
// -----------------------------
const int64 PLISFlagNoPause = 0x0001; // (1<<0) forbid pause the item if this flag is on
const int64 PLISFlagNoFF    = 0x0002; // (1<<1) forbid fastforward the item if this flag is on
const int64 PLISFlagNoRew   = 0x0004; // (1<<2) forbid rewind the item if this flag is on
const int64 PLISFlagNoSeek  = 0x0008; // (1<<3) forbid seek the item if this flag is on
const int64 PLISFlagOnce	= 0x0010;
const int64 PLISFlagPlayTimes = 0x00F0; ///< PlaylistItemSetupInfo::flags(0xf<<4) count of item playtimes, would be removed from the playlist once the count is reached, ==0 means unlimited
//改片子能够播放的次数 int64 count = (it->itemflag&PLISFlagPlayTimes)>>4
const int64 PLISFlagSkipAtFF  = 0x0100; ///< PlaylistItemSetupInfo::flags(1<<8) item would be skipped if the current speed is a fast-forward
//it->itemflag & PLISFlagSkipAtFF
const int64 PLISFlagSkipAtRew = 0x0200; ///< PlaylistItemSetupInfo::flags(1<<9) item would be skipped if the current speed is a rewind    


typedef struct _VSTRMBANDWIDTHTICKETS 
{
	ULONG64			FileTicket; 
	ULONG64			EdgeTicket;
	_VSTRMBANDWIDTHTICKETS()
	{
		FileTicket = 0;
		EdgeTicket = 0;
	}
}VSTRMBANDWIDTHTICKETS ,*PVSTRMBANDWIDTHTICKETS ;


class Playlist : public IPlaylistEx, public VstrmSessSink, virtual public ZQ::common::PollingTimer

{
	friend class PlaylistFailStoreThread;
	friend class PlaylistManager;
	friend class TimerRequest;
	friend class vstrmCallBackRequest;
	//friend class PlaylistHelper;	
public:
	
	// global configuration
	static unsigned long	_cfgIdleToExpire;
	// end of configuration

	static const SPEED_IND	SPEED_NORMAL;
	static const SPEED_IND	SPEED_FF;
	static const SPEED_IND	SPEED_REW;
	
	union
	{
		IOCTL_STATUS_BUFFER			isb;					// grand-total status buffer
		IOCTL_STATUS_BUFFER_LONG	isbLong;
	}							_iostatus_u;	
	class LockPrimeNext
	{
	public:	
		LockPrimeNext(bool & bLock):_bLock(bLock)
		{
			bLock=true;
		}
		~LockPrimeNext()
		{
			_bLock=false;
		}
	private:
		bool& _bLock;
	};
	
	typedef struct _Item
	{
		// input parameters
		IOCTL_ASCIZ_NAME_LONG		objectName;
		ULONG						userCtrlNum;               // user defined id for sequence operations
		ULONG						intlCtrlNum;               // internal sequence number
		ULONG						subSeq;						
		
		ULONG						spliceIn			: 1;	//Splice using in  time offset
		ULONG						spliceOut			: 1;	//Splice using out time offset
		ULONG						forceNormalSpeed	: 1;	//Force this session to normal speed on startup
		//Add new flags above and decrement the reservedFlags bit count below. Total bits = 32.
		ULONG						reservedFlags		: 29;		
		
		ULONG						itemFlag;					//item flag
		ULONG						fileFlag;					//file flag

		TIME_OFFSET					inTimeOffset;				// in	time offset	(milliseconds)
		TIME_OFFSET					outTimeOffset;				// out	time offset	(milliseconds)
		
		// runtime parameters
		SESSION_ID					sessionId;
		ULONG						state;						// session state
		ULONG						bitrate;
		VVX_FILE_SPEED				speed;
		//ULONG						bufferAffinity;
		TIME_OFFSET					timeOffset;					//  current time offset	(milliseconds)		
		UQUADWORD					byteOffset;
		UQUADWORD					byteOffsetEOS;				// end of stream offset
		ULONG						pendingDataSize;

		TIME_OFFSET					lastProgressEventTimeoffset;
		
		time_t						criticalStart;
		
		// time stamps of execution life cycle
		time_t						stampLoad;
		time_t						stampLaunched;
		time_t						stampLastUpdate;
		time_t						stampUnload;
		
		bool						_sessionDone;
		char						_rawItemName[256];
		long						_itemPlaytime;
		long						_itemBitrate;
		long						_itemRealTotalTime;
		long                        _itemPlayTimesCount;   

		//for MOTOROLA pre-encryption
		bool						_bEnableEcm;
		ECM_COMMON					_encryptionData;
		int							_iEcmDataCount;

		bool						_bPauseLastUtilNext;		//如果后面的节目没有load起来就pause在最后一帧 缺省位 FALSE
		
		USHORT						_itemPID;
		bool						_bhasItemPID;
		
		char						_itemLibraryUrl[1024];
		std::vector<std::string>	_itemLibraryUrls;
		bool						_bEnableItemLibrary;
		long						_curUsedLibraryUrlIndex;	//This value will be initialize to 0 , that is use the first one

		std::string					_strProviderId;
		std::string					_strProviderAssetId;

		bool						_bPWE;
		_Item() 
		{
			memset( &objectName,0,sizeof(objectName) );
			
			fileFlag			=		0;
			userCtrlNum			=		0;
			intlCtrlNum			=		0;
			subSeq				=		0;

			spliceIn			=		0;
			spliceOut			=		0;
			forceNormalSpeed	=		0;			
			reservedFlags		=		0;

			itemFlag			=		0;

			inTimeOffset		=		0;
			outTimeOffset		=		0;

			sessionId			=		0;
			state				=		0;
			bitrate				=		0;
			speed.denominator	=		0;
			speed.denominator	=		0;
			
			timeOffset			=		0;
			byteOffset			=		0;
			byteOffsetEOS		=		0;
			pendingDataSize		=		0;

			lastProgressEventTimeoffset=0;

			criticalStart		=		0;

			stampLoad			=		0;
			stampLaunched		=		0;
			stampLastUpdate		=		0;
			stampUnload			=		0;

			_sessionDone		=		false;
			memset( _rawItemName,0,sizeof(_rawItemName) );
			_itemPlaytime		=		-1;
			_itemBitrate		=		0;
			_itemRealTotalTime	=		-1;

			//for MOTOROLA pre-encryption
			_bEnableEcm			=		false;
			memset(&_encryptionData , 0 ,sizeof(_encryptionData) );
			_iEcmDataCount		=		0;

			_bPauseLastUtilNext	=		false;
			
			_itemPID			=		0;
			_bhasItemPID		=		false;
			
			memset( _itemLibraryUrl , 0 , sizeof(_itemLibraryUrl) );
			_itemLibraryUrls.clear();
			_bEnableItemLibrary	=		false;
			_curUsedLibraryUrlIndex =	0;
			_itemPlayTimesCount       =   -1;

			_strProviderId		=		"";
			_strProviderAssetId	=		"";
			_bPWE				=		false;
		}
		
	} Item;

	typedef std::vector < Item >			List;
	typedef const List::iterator			const_iterator;
	typedef List::iterator					iterator;


public:
	// constructor
	Playlist(StreamSmithSite* pSite,PlaylistManager& mgr, 
			const ZQ::common::Guid& guid, bool bFailOver=false,
			const std::vector<int>& boardIDs=std::vector<int>(),const std::string& userSessId="" );
	//deconstructor
	virtual ~Playlist();
	
public:	
	///Set program number to playlist,this value will be used by vstrm,it's meaningless
	///it can't be over 4000 at now
	bool					setProgramNumber(IN USHORT ProgramNumber);
	
	/// Set stream's current bit rate and maximum bitrate.
	///@param MaxRate Maximum mux rate for all PID's in program.
	///@param NowRate Typical (current rate for CBR) mux rate
	///@param MinRate /Minimum mux rate
	bool					setMuxRate(IN ULONG NowRate, IN ULONG MaxRate, IN ULONG MinRate=0);
	
	/// set stream's destination
	///@param ipAddr ip address in string
	///@param udpPort destination udp port
	bool					setDestination(IN char* ipAddr, IN int udpPort);
	
	/// set destination mac address
	///@param macAddr mac addrss in string
	bool					setDestMac(IN char* macAddr);

	// hook the authorization owner
	bool					setAuth(IN const char* endpoint);

	///Get playlist information
	bool					getInfo(IN unsigned long mask,ZQ::common::Variant& var );	

	// list operations
	
	///insert a item into playlist
	///@return the new item's ctrlNum if success,INVALID_CTRL_NUM if failed
	///@param where the ctrlNum of where to insert before	
	///@param fileName new item 's name in string
	///@param userCtrlNum new item's ctrlNum
	///@param inTimeOffset cue in timeoffset in millisecond
	///@param outTimeOffset cue out timeoffset in millisecond
	///@param crticalStart item critical start GMT time,pass 0 if it 's not a crtical start item
	///@param spliceIn splice in
	///@param spliceOut splice out
	///@param forceNormal force normal speed when next item is streaming
	///@param flag,reserved for future use
	///@param newItem new item information
	const CtrlNum			insert(IN const CtrlNum where, IN const char* fileName, IN const CtrlNum userCtrlNum,
										 IN const uint32 inTimeOffset, IN const uint32 outTimeOffset, 
										 IN const time_t criticalStart=0, const bool spliceIn=false, 
										 IN const bool spliceOut=false,
										 IN const bool forceNormal=true, IN const uint32 flags=0);
	const CtrlNum			insert(IPlaylist::Item& newitem);
	
	const CtrlNum			insertEx(IN const CtrlNum where, IN const char* fileName, IN const CtrlNum userCtrlNum,
										 IN const uint32 inTimeOffset, IN const uint32 outTimeOffset, 
										 IN ZQ::common::Variant& var,
										 IN const time_t criticalStart=0, const bool spliceIn=false, 
										 IN const bool spliceOut=false,
										 IN const bool forceNormal=true, IN const uint32 flags=0);

	//playlist item list size,this is the logic item size
	///@return the playlist item size
	const int				size();
	
	///playlist item list logic begin position
	///@return the list begin position
	iterator				listBegin() ;
	
	///playlist iten list logic end position
	///@return the list end position	
	iterator				listEnd() ;
	
	///@playlist item list real start position
	///@param the list start position	
	iterator				listStart() ;
	
	///get current list position
	///@return current list position
	iterator				listCurrent() const;
	
	///detect current list is empty or not
	///@return true if list is empty,flase if not
	const bool				empty()	const;
	
	///get current item's ctrlNum
	///@return current item ctrlNum,INVALID_CTRL_NUM if failed
	const CtrlNum			current() const ;

	///erase specified item with it's ctrlNum,NOTE that ongoing item can't be erased
	///@return current item's ctrlNum
	///@param where the ctrlNum of which item is going to be erased
	const CtrlNum			erase(const CtrlNum where);
	
	///@push a new item into playlist item list
	const CtrlNum			push_back(IPlaylist::Item& newItem);
	const CtrlNum			push_back(const char* fileName, 
									const uint32 userCtrlNum, const uint32 inTimeOffset, 
									const uint32 outTimeOffset, const time_t criticalStart=0, 
									const bool spliceIn=false, const bool spliceOut=false, 
									const bool forceNormal=true, const uint32 flags=0);
									
	///flush expired item									
	const CtrlNum			flush_expired();
	
	const bool				clear_pending(const bool includeInitedNext=false);
	
	iterator				findUserCtrlNum(const CtrlNum userCtrlNum);
	
		
	bool					isReverseStreaming(){return _isReverseStreaming; }	

	std::vector<ULONG>		getItemSet();
	// playlist state checking
	const bool				isCompleted();
	
	const bool				isRunning() ;

	// search by userCtrlNum, default from the begin of the list
	iterator				findItem(const ULONG userCtrlNum, const_iterator from);
	const CtrlNum			findItem(const CtrlNum userCtrlNum,const CtrlNum from = INVALID_CTRLNUM);
	const bool				distance(OUT int* dist, IN CtrlNum to, IN CtrlNum from = INVALID_CTRLNUM);	
	const bool				getItemInfo(const_iterator, Item& elem);	
	
	//Seek to specify position of a specify item
	bool					SeekTo(CtrlNum to,long timeOffset,IPlaylist::SeekStartPos pos);	
	__int64					SeekStream(__int64 offset,IPlaylist::SeekStartPos pos);
	//////////////////////////////////////////////////////////////////////////
	//dvbc resource manager
	bool					allocateResource(int serviceGroupID, ZQ::common::Variant& varOut,int bandwidth=-1);
	bool					releaseResource(ZQ::common::Guid& uid);
	void					enableEoT(bool enable);
	//////////////////////////////////////////////////////////////////////////

	

	// stream control
	// --------------------	
	virtual bool			fastward(){return false;}
	virtual	bool			play();
	virtual bool			pause();
	virtual bool			resume();
	virtual bool			skipToItem(const_iterator it, bool bPlay =true);

	virtual bool			skipToItem(const CtrlNum where, bool bPlay =true);

	long					getSessDurLeft(); // in msec
	const int				left(){return static_cast<int>(listEnd()-_itCurrent);}
	
	void					printList(const char* hints=NULL);
	void					destroy();
	const char*				lastError(){return _lastError.c_str();}
	void					setLastError(std::string& lastErr){_lastError=lastErr;}
	ZQ::common::Guid&		getId() {return _guid;}
	
	
	IPlaylist::State		getCurrentState(){return (IPlaylist::State)_currentStatus;}
	
	void					setUserCtxIdx(const char* SessID);
	
	const char*				getUserCtxIdx(){return m_strClientSessionID.c_str();}
	
	bool					PlayListInitOK();	
	
	bool					setSpeed(const float newSpeed);

	const unsigned int		lastErrCode(){return _lastErrCode;}
	int32					lastExtErrorCode( ) const { return _lastExtErrCode;}
	
	void					setStreamServPort(unsigned short uPort);
	
	bool					setPokeHoleSessionID( const std::string& strPokeHoleSessID  ) ;

	void					setPlaylistAttributes( const TianShanIce::ValueMap& values ) ;

	bool					exPause( OUT StreamControlResultInfo& info );

	bool					exPlay( IN float newSpeed ,IN ULONG offset , IN short from , OUT StreamControlResultInfo& info );

	bool					exPlayItem( IN CtrlNum userCtrlNum , IN float newSpeed ,IN ULONG offset , IN short from , OUT StreamControlResultInfo& info  );

	void					setStreamPID(int pid);

	bool					commit( );
#ifdef _ICE_INTERFACE_SUPPORT
	public:
	void					copyList(List& _list);
	void					setPlaylistExProxy( const std::string&  playlistPrxStr , const std::string& ticketPrxStr);
	void					VstrmSessNotifyWhenStartup(const VstrmSessInfo& sessinfo);
	void					vstrmSessNotifyOverWhenStartup();

	
private:
	//TianShanIce::Streamer::InternalPlaylistExPrx	m_plExPrx;//This var is used to record playlistex proxy
	std::string				m_plePxStr;			/*<!-playlistex proxy string*/
	std::string				m_ticketPrx;

	bool					m_bCurrentSessionIsONLine; /*used for failover,check current vstrm session is on or not*/
		
#endif//_ICE_INTERFACE_SUPPORT

private:
	bool					checkContentAttribute( iterator it );
	bool					isLastItem( const iterator& it );
	bool					isItemPWE( const iterator& it ) const;
	void					convertVstrmErrToTianShanError( VSTATUS t );
	bool					iterValid( const iterator& it  ) const;
	iterator				lastItem( );
	std::string				convertItemFlagToStr( ULONG uFlag);
	std::string				convertItemFlagToStr( const_iterator it );
	bool					CanSetSpeed( const SPEED_IND newspeed );
	bool					checkVstrmRestrictionArea( const float& newSpeed , TIME_OFFSET& timeLeft );
	void					SetTimerEx(timeout64_t t,bool bGoodData=true);
	void					ERRLOG(int level, char* fmt,...) PRINTFLIKE(3, 4);
	void					DumpItemInfo(iterator it);
	void					DumpIOCTLCONTROLPARMSLONG(IOCTL_CONTROL_PARMS_LONG& para);
	void					DumpIOCTLLOADV2PARAMS(IOCTL_LOADV2_PARAMS& para);
	bool					LoadAndLaunch(iterator& it);
	void					DumpListInfo(bool bOnlyAvailableSessdId=false);
	void					ClearItemsState();
	void					FireStateChanged(IPlaylist::State plState , bool bFire=true);
	void					FireSpeedChanged( ULONG sessionId, SPEED_IND newSpeed, SPEED_IND oldSpeed );
	//bool					GetVvxInfo(std::string& strFile,VvxParser& parser);
	int						getNptFromItem( iterator it );
	int                     getNptPrimaryFromItem( iterator it );
	int					    getCurrentPlaylistNpt();
	int						getCurrentItemNpt();
	int64                   getTotalStreamDuration();
	int64                   getTotalVideoStreamDuration();


	bool					changeSequenceAfterChangespeed( TIME_OFFSET npt ,SPEED_IND originalSpeed , SPEED_IND returnSpeed );

	//check item restriction
	///@return true if no restriction on the item,vice versa
	bool					checkResitriction( const_iterator itTarget , const long mask );

	//update operation time
	//if stream event generation time is not great than last Op time , just ignore it
	//But this only affect StateChanged Event
	void					updateOpTime( );
public:
	int						m_iErrInfoCount;
	std::string				m_strDestinationMac;
	//reserve client session ID for EVENT SINK use
	std::string				m_strClientSessionID;
#ifdef VSTRM_ONDEMANDE_SESSID_ON
	VSTRM_GUID                              _userSessGuid;
#endif // LOAD_CODE_ONDEMAND_SESSION_ID
	
	StreamSmithSite*		m_pStreamSite;

	PlaylistManager&		_mgr;

	bool					_b1stPlayStarted;			// first play completion has been processed (T/F)	
	bool					_bLongFilename;				// long file names
	ZQ::common::Guid		_guid;
	std::string				_strGuid;

	std::string				_destIP;
	ULONG					_vstrmSessioIDForFailOver;

	VSTRMBANDWIDTHTICKETS		mVstrmBwTcikets;

protected:
	long volatile					_eventCseqNewSessDetected;
	long volatile					_eventCSeqBegginingOfStream;
	long volatile					_eventCSeqEndofStream;
	long volatile					_eventCSeqSpeedChanged;
	long volatile					_eventCSeqStateChanged;
	long volatile					_eventCSeqItemStepped;
	long volatile					_eventCSeqPlaylistExit;
	long volatile					_eventCSeqProgress;
	long volatile					_eventCSeqSessExpired;//session expired abnormal

	bool					_bEnableEOT;	//Force normal play if enabled,vice versa
	bool					_bDontPrimeNext;
	


	Variant					_varQamResource;
	bool					_bUpdateCurrentItemInfo;//是否跟新的是当前的Item信息
	bool					_isGoodItemInfoData;//用以表示当前的ItemInfo数据是否正确，如果不正确，需要更新 
	bool					_isReverseStreaming;
	bool					_bCleared;

	ULONG					_ulSetupWaitTime;
	ULONG					_ulPauseWaitTime;
	
	long					_plExistId;
	IPlaylist::State		_currentStatus;	

	int						_lastplaylistNpt;
	int                     _lastplaylistNptPrimary;
	int						_lastItemNpt;

	List					_list;
	iterator				_itCurrent;					// location of current playing item
	iterator				_itNext;					// location of last item that has committed to VstrmAPI
//	iterator				_itNextCriticalStart;		// location of next item that must started at a critical time
	
	
	bool					doPause( IN bool bChangeState ,  OUT StreamControlResultInfo& info );

	bool					doLoad( IN iterator it , IN float newSpeed , IN ULONG timeOffset , OUT StreamControlResultInfo& info , bool bSeek = true );

	bool					doReposition( IN short from  , IN LONG timeOffset , IN float newSpeed , OUT StreamControlResultInfo& info );

	bool					doResume( OUT StreamControlResultInfo& info ,bool bForce = false );

	bool					doChangeSpeed( IN float newSpeed , IN bool bForeceChangeSpeed , OUT StreamControlResultInfo& info , CtrlNum curItem = -2 ,bool bUpdateTimer = true );

	bool					doGetStreamResultInfo( IN  iterator it , OUT StreamControlResultInfo& info );

	bool					doPlay(  IN LONG timeOffset , IN float newSpeed , OUT StreamControlResultInfo& info );

	bool					doSeekStream( IN LONG timeOffset , IN short from , IN float newSpeed , OUT StreamControlResultInfo& info );
	bool					doSeekStreamCommand(  iterator it , IN LONG timeOffset ,  IN float newSpeed , OUT StreamControlResultInfo& info );

	///check effective session on vstrm port to make streamsmith deal with the correct session
	void					checkEffectiveSession( SESSION_ID currentSessioId );
	
	bool					repositionAfterLoad( iterator it, ULONG timeoffet , StreamControlResultInfo& info);
	// internal operations
//	bool					innerPause(bool bChangeState = true);
// 	bool					innerLoadEx(iterator it,bool bNormalSpeed=false,long timeOffset=0 );
// 	bool					loadEx( iterator it,bool bNormalSpeed=false,long timeOffset=0);
	bool					unloadEx(iterator it, bool bReset = true );
//	bool					launchEx(iterator it);
	bool					isLaunched(iterator it);

	void					DisplaySessionAttr(iterator it);
	
	bool					setSpeedEx(const SPEED_IND newspeed , bool bForceSet=false);

	iterator				findInternalCtrlNum(const ULONG intlCtrlNum);	
	bool					getVstrmSessInfo( iterator it, ESESSION_CHARACTERISTICS& valueOut);
	bool					getStrmSessAttribute(iterator it,int& cOffset,float& scale,int& totalOffset,bool canIgnoreTotalOffset=false ,bool bLocalCall = false);
	//////////////////////////////////////////////////////////////////////////
		
	// assume the following are per-playlist attributes
	DVB_SESSION_ATTR			_dvbAttrs;
	DVB_SESSION_ATTR			_dvbAttrResult;
	ULONG						_vstrmPortNum;
	SESSION_ID					_mastSessId;
	IOCTL_CONTROL_PARMS_LONG	_ioctrlparms;				// Load parameters
	VSTATUS						_status;
	NTSTATUS					_ntstatus;
	SPEED_IND					_crntSpeed;

	time_t						_stampCreated;
	time_t						_stampDone;

	std::string					_authEndpoint;

	std::string					_strClearReason;			//playlist exit reason
	int							_iExitCode;
	std::string					_lastError;
	unsigned int				_lastErrCode;
	int32						_lastExtErrCode;

	ULONG						_lastSessionID;
	ZQ::common::Guid			_ResourceGuid;

	int							_currentItemDist;			//as the internal userCtrl Num
	
	timeout64_t					_lastUpdateTimer64;

	std::string					_pokeHoleSessID;
	HANDLE						_hRemoteSessionNotifier;

	LONG						_perStreamPID;

	int64						_lastSessOpTime;
	DWORD                       _stampSpeedChangeReq;
	DWORD                       _stampStateChangeReq;
	bool						_bCommitted;

	int							_lastItemStepErrorCode; // this error may cause item stepped
	std::string					_lastItemStepItemName;
	std::string					_lastItemStepErrDesc;
	


protected:
	
	void						onPauseTimeout( );
	void						onItemReposition( iterator it, int oldTimeOffset , int newTimeOffset , StreamControlResultInfo* controlInfo = NULL);

	//I should change the function name OnItemDone into OnItemStepped
	virtual void				OnItemStepped(const iterator prevItem ,const iterator nextItem, int curTimeOffset = 0 , 
												const std::string& reason="" , int errCode = 0,	StreamControlResultInfo* controlInfo = NULL );
	virtual void				OnItemDone(iterator completedItem,iterator itNext);
	virtual void				OnItemDoneDummy(int PrevCtrlNum ,const std::string& strFileName ,iterator nextItemIter);
	virtual void OnPlaylistDone( const std::string& reason="");
	virtual void				OnTimer();	
	virtual void				OnItemAbnormal(SESSION_ID sessId);

	// overwritable event handles
	virtual VSTATUS				OnItemCompleted(PIOCTL_STATUS_BUFFER pStatusBlk, ULONG BlkLen);
	// callback
	static VSTATUS				cbItemCompleted(HANDLE classHandle, PVOID cbParam, PVOID bufP, ULONG bufLen);

	Playlist::const_iterator	updateItem( const VstrmSessInfo& sessionInfo , bool bUpdateTimer=false);
	void						updateTimer();
	void						stepList(ULONG doneSessionId,bool unloadCurrent =false,const std::string& reason="", int errorCode = 0 );
	void						primeNext();
	iterator					findNextCriticalStartItem(timeout_t& timeout);
	void						ClearAllResource();
	
	//calculate the past time
	ULONG						CalculatePastTime();
	ULONG                       CalculatePastTimeNPTPriamry();
	ULONG						CalculateCurrentTotalTime( );
	ULONG						CalculateFutureTime( );

	bool						jumpValidation( iterator itFrom , iterator itTo, ULONG fromNpt, ULONG toNpt);
	
#if _USE_NEW_SESSION_MON
	void						OnVstrmSessDetected( const VstrmSessInfo& sessionInfo,ZQ::common::Log& log ,  const int64& timeStamp );
	void						OnVstrmSessStateChanged( const VstrmSessInfo& sessionInfo, const ULONG curState , const ULONG PreviousState,ZQ::common::Log& log ,  const int64& timeStamp );
	void						OnVstrmSessSpeedChanged( const VstrmSessInfo& sessionInfo, const SPEED_IND curSpeed , const SPEED_IND PreviousSpeed,ZQ::common::Log& log ,  const int64& timeStamp );
	void						OnVstrmSessProgress(const VstrmSessInfo& sessionInfo, const TIME_OFFSET curTimeOffset ,const TIME_OFFSET PreviousTimeOffset,ZQ::common::Log& log ,  const int64& timeStamp );
	void						OnVstrmSessExpired(const ULONG sessionID,ZQ::common::Log& log , const int64& timeStamp ,const std::string& reason="" , int errorCode = 0 );
#else//_USE_NEW_SESSION_MON
	void						OnVstrmSessDetected(const ESESSION_CHARACTERISTICS* sessinfo);
	void						OnVstrmSessStateChanged(const ESESSION_CHARACTERISTICS* sessinfo, const ULONG PreviousState);
	void						OnVstrmSessSpeedChanged(const ESESSION_CHARACTERISTICS* sessinfo, const VVX_FILE_SPEED PreviousSpeed);
	void						OnVstrmSessProgress(const ESESSION_CHARACTERISTICS* sessinfo, const TIME_OFFSET PreviousTimeOffset);
#endif//_USE_NEW_SESSION_MON



//	timeout_t getTimeoutToNextCriticalStart(void); // in msec
};


//////////////////////////////////////////////////////////////////////////

class PlaylistSuicide:public ZQ::common::NativeThread
{
public:
	PlaylistSuicide();
	~PlaylistSuicide();

	void	PushPlaylist(Playlist* pl);
	void	SetquitFlag(bool bQuit)
	{
		if(bQuit)
		{
			SetEvent(m_hQuitHandle);
		}
	}	
	int		run();
	void	final(){}
	
	void	SetKillerWaitInterval(long interval){_KillerThreadInterval=interval;}
	HANDLE	GetQuittedHandle(){return m_hQuittedHanlde;}
	int		getPlCount()
	{
		ZQ::common::MutexGuard gd(m_ListLocker);
		return static_cast<int>(m_vecList.size());
	}
protected:
	
private:
	typedef struct _tagPLProperty
	{
		Playlist*	_pl;
		bool		_bCleanOver;
	}plPropery;
	typedef std::vector<plPropery>		VECSuicidePlayList;
	VECSuicidePlayList					m_vecList;
	HANDLE								m_hQuitHandle;
	ZQ::common::Mutex					m_ListLocker;
	StreamSmithSite*					m_pStreamSite;
	HANDLE								m_hQuittedHanlde;
	long								_KillerThreadInterval;
};


#ifdef _ICE_INTERFACE_SUPPORT
////////////////////////////////////////////////////////////////////////
class PlaylistFailStoreThread:public ZQ::common::NativeThread
{
public:
	PlaylistFailStoreThread(::ZQ::StreamSmith::PlaylistInfoStore& plinfoStore);
	~PlaylistFailStoreThread();
	void			Stop();
	int				run();
	void			final();
	void			UpdateFailStore(Playlist* pList,bool bplAttr,bool bpliAttr);
	void			AddClearPlaylistInfo(const std::string& uid,bool bplAttr,bool bpliAttr);

	void			ChangeItemToAttr(Playlist* pList,
									::ZQ::StreamSmith::VECPlaylistItemAttr& vecAttr);	

	void			ChangePlaylistToAttr(Playlist* pList , TianShanIce::Streamer::PlaylistAttr& attr);	

	void			ChangeAttrToItem(::ZQ::StreamSmith::VECPlaylistItemAttr& vecAttr,
									Playlist::List& itemList);	

	void			ChangeAttrToPlaylist(::TianShanIce::Streamer::PlaylistAttr& attr,Playlist* pList );

	void			AddNewPlaylistIntoFailOver(const std::string& strGuid, 	const PlaylistAttr&  attr);

	IPlaylistEx*	CreatePlaylist(ZQ::common::Guid& uid);
protected:
	typedef	struct _failStoreInfo 
	{
		std::string									uid;
		TianShanIce::Streamer::PlaylistAttr			plAttr;
		::ZQ::StreamSmith::VECPlaylistItemAttr		plivecAttr;
		Playlist*									pList;
		bool										bPlAttr;
		bool										bPliAttr;
	}FailStoreInfo;

	typedef std::vector<FailStoreInfo>		FailStoreList;
	
	typedef std::map<std::string,FailStoreInfo>	FailStoreMap;

	FailStoreMap							m_PlaylistFailStoreMap;

	FailStoreList							m_PlaylistFailStoreList;
	
	FailStoreList							m_PlaylistClearList;
	
	ZQ::common::Mutex						m_failStoreMutex;
	ZQ::common::Mutex						m_failClearMutex;

	HANDLE									m_hUpdatePlaylistFailStoreEvent;
	bool									m_bQuit;
	PlaylistInfoStore&						m_PlayListStore;
	ZQ::common::Mutex						m_addNewPlMutex;
};
#endif//_ICE_INTERFACE_SUPPORT



class spigotReplicaReport : public ZQ::common::NativeThread
{
public:
	spigotReplicaReport( PlaylistManager& plMan );
	~spigotReplicaReport( );
public:
	void set( int defaultInterval , const std::string& endpoint )
	{
		defaultUpdateInterval= defaultInterval;
		listenerEndpoint = endpoint;
		if(defaultUpdateInterval<10*1000)
			defaultUpdateInterval = 10 * 1000;
	}
	void			stop();
	virtual int		run( );
	virtual void	runTimerTask();
	void			setServiceState( bool serviceStatus );

private:
	std::string				listenerEndpoint;	
	int						defaultUpdateInterval;
	PlaylistManager&		_plManager;
	HANDLE					_hQuitEvent;
	bool					_bQuit;
	int						updateInterval;
	bool					bServiceUp;
};



// -----------------------------
// class PlaylistManager
// -----------------------------
#ifndef _ICE_INTERFACE_SUPPORT
	class PlaylistManager : public ZQ::common::ThreadRequest //, public VstrmClass
#else//_ICE_INTERFACE_SUPPORT
	class PlaylistManager : public ZQ::common::ThreadRequest ,public IPlaylistManager
#endif//_ICE_INTERFACE_SUPPORT
{
	friend class Playlist;
	friend class vstrmCallBackRequest;
	
public:
#ifndef _ICE_INTERFACE_SUPPORT
	PlaylistManager(VstrmClass& cls);
#else
	PlaylistManager(VstrmClass& cls,Ice::CommunicatorPtr& ic,const char* EndPoint);
#endif

	~PlaylistManager();
	
	void			ServiceDown();
	
	void			LoadSuperPlugin(const  char* strPath,
									const char* iceEventChannelEndpoint,
									char* strNetID,const std::vector<int>& spigotsID);
	void			ReleaseSuperPlugin( );

	Playlist*		find(const ZQ::common::Guid& uid);

	void			wakeup(void);

	VHANDLE			classHandle() const;
	
	ULONG			GetUnUsePort(int SpigotID){return  _cls.GetUnUsePort(-1,SpigotID);}
	void			FreePortUsage(ULONG port){_cls.FreePortUsage(port);}

	void			spigotStatusCallback( const VstrmClass::VstrmSpigotChar& spigotChar , const int spigotIndex );
	void			bandwidthUsageCallback( const BandwidthUsage& bwUsage );
	void			nodeStatusCallback( const std::string& nodeName , bool bUp = false );

	const char*		getErrorText(const VSTATUS status, char* textbuf, const int maxlen)
	{
		return _cls.getErrorText(status,textbuf,maxlen);
	}

	void			SetKillerThreadWaitInterval(long interval)
	{
		if(m_pKillPlayList)
		{
			if(interval<20*1000)
				interval=20*1000;
			m_pKillPlayList->SetKillerWaitInterval(interval);
		}
	}

	void			SetPlaylistTimeout(long timeout, bool bTimeoutOnPause)
	{
		glog(Log::L_INFO,"set playlist timeout=%d and timeoutOnPause=%d",timeout,bTimeoutOnPause);
		Playlist::_cfgIdleToExpire = timeout;
		_bTimeoutOnPause = bTimeoutOnPause;
	}

	void			StartSessionMonitor( char* spigotsID  );
	bool			initIceInterface( ZQ::common::Log* pLog, char* strSuperPluginPath,
											char* eventChannelEndPoint,char* netId, char* spigotsID );

	//this is only used for pauseTV
	void			SetForceNormalTimeOnPauseTV(long timeCount)//-1 if no force normal play
	{
//		if(timeCount>0)
//			glog(Log::L_DEBUG,"set force normal play before before last item end with time =%d",timeCount);
//		_forceNormalPlayTimeOnPauseTV=timeCount;
	}

//	long			GetForceNormalTimeOnPuaseTV(){return _forceNormalPlayTimeOnPauseTV;}
	bool			keepOnPause() { return !_bTimeoutOnPause; }
	////

	//////////////////////////////////////////////////////////////////////////
	bool			ParseResourceManager(char* confPath){return m_ResourceManager.ParseConfig(confPath);}
	bool			GetResource(int serviceGroupdID,long needBW, 
								std::string&	strIP,
								int&			Port,
								std::string&	strMac,
								int&			ProNum,
								int&			Frequency,
								int&			ChannelID,
								ZQ::common::Guid& uid,
								int& qamMode);
	
	bool			FreeResource(ZQ::common::Guid& uid);
	
	///retrive spigotsID using service group id and max bit rate
	///if success,the SpigotsIDs contain a set of spigotID,false if no spigot id is available.if all spigot is available,-1 is include in SpigotIDs
	bool			GetSpigotIDsFromResource(int serviceGroupID,int MaxBitRate,std::vector<int>& SpigotIDs);
	//////////////////////////////////////////////////////////////////////////
#ifdef WITH_ICESTORM
	void			registerEventSink(DWORD type,EventDispatchEvent eDispatch,void* pExtraData);
	bool			postEventSink(DWORD eventType,ZQ::common::Variant& params);
#else
	void			registerEventSink(DWORD type,EventDispatchEvent eDispatch,void* pExtraData){ }
#endif

	//for fail over
#ifdef _ICE_INTERFACE_SUPPORT
public:
	
	VstrmClass&		getVstrmClass() 
	{
		return	_cls;
	}
	static	void	plSetupCallback(const std::string& guidStr , 
									const TianShanIce::Streamer::PlaylistAttr& plAttr,void* pData);
	static	void	pliSetupCallback(const std::string& guidStr , 
									ZQ::StreamSmith::VECPlaylistItemAttr& pliAttr,void* pData);

	bool			setContentStoreProxy( Ice::ObjectPrx objPrx );

	void			PlaylistReConstruct(const std::string& guidStr ,
									const TianShanIce::Streamer::PlaylistAttr& plAttr);
	void			PlaylistItemReConstruct(const std::string& guidStr ,
									ZQ::StreamSmith::VECPlaylistItemAttr& pliAttr);

	void			UpdatePlaylistFailStore(Playlist* pList,bool bPlaylistAttr,bool bItemAttr);	

	void			ClearFailOverInfo(ZQ::common::Guid& uid);

	bool			StartFailOver(ZQ::common::Log* log);
	
	void			listPlaylistGuid(::std::vector<::std::string>& IDs);
	
	void			addNewPlaylistIntoFailOver(const std::string& strGuid ,	const PlaylistAttr& attr);
	//void			addNewPlaylistIntoFailOver(const std::string& strGuid ,const ::TianShanIce::Streamer::PlaylistAttr& attr);
	IPlaylistEx*	CreatePlaylist( const std::string& uid, const std::vector<int>& SpigotID, const std::string& userSessId );
	IPlaylistEx*	find(const std::string uidString);
	void			listStreamers(std::vector<int>& SpigotIDs);
	int				getIceEvitorPLaylistSize();
	
	int				getIceEvitorPLaylistItemSize();
	
	int				getPlaylistCount() ;
	
	int				getSuicidePlCount();

	void			getReplicaInfo( const std::string& category , const std::string& groupId , bool bLocalOnly , TianShanIce::Replicas& res ) ;

	void			getSpigotReplicaInfo( TianShanIce::Replicas& res );
	//Ice::CommunicatorPtr&	GetIceCommunicator(){return m_ic;}

#endif//_ICE_INTERFACE_SUPPORT

protected:

	virtual bool	init(void);
	virtual int		run(void);
	
	virtual void	final(int retcode =0, bool bCancelled =false);

	long			reg(Playlist& subscr);
	bool			unreg(Playlist& subscr);
	void			DeleteFailOverDbPath(std::string dbPath);	
	
	Playlist*		plExist(  long id );

public:
	ZQADAPTER_DECLTYPE 		m_Adapter;
	spigotReplicaReport		m_spigotReplicaReporter;
	ZQ::common::NativeThreadPool	mAmdThreadpool;
protected:
	bool											_bTimeoutOnPause;
	//long											_forceNormalPlayTimeOnPauseTV;
	typedef struct _PlaylistInfo
	{
		Playlist*	pl;
		long		id;
	}PlaylistInfo;
	typedef std::map<ZQ::common::Guid, PlaylistInfo > Map;
	//typedef std::map<Playlist*,int>					MapPLExist;
	typedef std::map<long,Playlist*>				MapPLExist;
	typedef	std::vector<ZQ::common::Guid>			GuidVec;
	Map												_map;
	MapPLExist										_existPl;
	ZQ::common::Mutex								_mapLocker;

	//this id can rollover
	int64											_staticPlaylistId;
//	typedef std::vector<Playlist* > Playlist_v;

//	Playlist_v        _playlists;
//	ZQ::common::Mutex _playlists_locker;

	VstrmSessMon		_sessmon;
#ifdef _ICE_INTERFACE_SUPPORT
	PlaylistFailStoreThread*	m_pFailStoreThread;
	PlaylistInfoStore*			m_pPlayListStore;
	Ice::CommunicatorPtr&		m_ic;	
	//添加一个额外的操作来renew ticket
	CheckContent				m_contentChecker;
#endif//_ICE_INTERFACE_SUPPORT

#ifdef WITH_ICESTORM
	EventDispatchEvent		_pEventDispatch;
	void*						_pEventDispatchExtraData;


#endif//WITH_STORM

	HANDLE							_hEvtWakeup;

	timeout64_t						_timeout;
	bool							_bQuit;

	VstrmClass&						_cls;

	PlaylistSuicide*				m_pKillPlayList;
	

	Ice::ObjectPrx					mContentStoreProxy;
	ZQ::IdxParser::IdxParserEnv		mIdxParserEnv;
	//resource manager////////////////////////////////////////////////////////
	ResourceMan						m_ResourceManager;
	//////////////////////////////////////////////////////////////////////////

	HMODULE							m_hSuperPlugin;

	std::vector<ULONG64>			ticketsInDb;

	bool							mbDBRecordValid;
	Ice::Long						mPendingRequestLastCheckTime;
	bool							mbServiceAvailable;

	//add a sys log to support 
	SysLog							mSysLogger;

};

}
}
#endif // __Playlist_H__
