
#ifndef _zq_StreamService_Vstrm_Command_header_file_h__
#define _zq_StreamService_Vstrm_Command_header_file_h__

#include <SsStreamBase.h>
#include "StreamSmithEnv.h"
#include "VstrmStreamerManager.h"
#include "VstrmSessionScaner.h"
#include "SsServiceImpl.h"


extern "C"
{
#include <vstrmuser.h>
}

#define STREAMSMITH_FILE_FLAG_NPVR	1 << 0

static const std::string		ITEM_PARA_ECM_ENCRYPTION_ENABLE		=		"Tianshan-ecm-data:preEncryption-Enable";
static const std::string		ITEM_PARA_ECM_PROGRAMNUMBER			=		"Tianshan-ecm-data:programNumber";
static const std::string		ITEM_PARA_ECM_KEYOFFSETS			=		"Tianshan-ecm-data:keyoffsets";
static const std::string		ITEM_PARA_ECM_KEYS					=		"Tianshan-ecm-data:keys";
static const std::string		ITEM_PARA_ECM_CYCLE1				=		"Tianshan-ecm-data:Cycle_1";
static const std::string		ITEM_PARA_ECM_CYCLE2				=		"Tianshan-ecm-data:Cycle_2";
static const std::string		ITEM_PARA_ECM_FREQ1					=		"Tianshan-ecm-data:Freq_1";
static const std::string		ITEM_PARA_ECM_FREQ2					=		"Tianshan-ecm-data:Freq_2";

static const std::string		ITEM_LIBRARY_URL					=		"storageLibraryUrl";


namespace ZQ
{
namespace StreamService
{


class VstrmCommand 
{
public:
	VstrmCommand(  SsEnvironment* environment ,	
					VstrmStreamerManager& streamerManager ,
					VstrmSessionScaner& sessScaner ,
					SsServiceImpl&		service ,
					SsContext& ctx );

	virtual ~VstrmCommand( );


public:
	
	int32					doCommit( IN const PlaylistItemSetupInfos& items  ,
										IN const TianShanIce::SRM::ResourceMap& requestResource );
	
	int32					doLoad( const TianShanIce::Streamer::PlaylistItemSetupInfo& info ,									
									float& newSpeed , 
									Ice::Long timeOffset ,
									ULONG& sessId );

	int32					doUnload( ULONG sessId );

	int32					doReposition( ULONG sessId , const float& newSpeed ,Ice::Long timeOffset );

	int32					doResume( );

	int32					doPause( );

	int32					doChangeSpeed( float& newSpeed , bool bGetInfo = true );
	

	int32					doGetInfo( ULONG sessId , ESESSION_CHARACTERISTICS& sessInfo );	

	int32					doGetStreamAttr(	int32 mask,
												ULONG sessId , 
												const TianShanIce::Streamer::PlaylistItemSetupInfo& info );
	
	///every routine will update the IRP output parameter

	StreamParams			getStreamInfo( ) const;

	int32					checkSessions(	SessionRestoreInfos& sessInfos );

	void					doClearResource( );

public:
	typedef struct _VSTRMBANDWIDTHTICKETS 
	{
		ULONG64			FileTicket; 
		ULONG64			EdgeTicket;
		ULONG64			CdnTicket;
	}VSTRMBANDWIDTHTICKETS ,*PVSTRMBANDWIDTHTICKETS ;

	
	int32					prepareItemRunTimeInfo( const TianShanIce::Streamer::PlaylistItemSetupInfo& info  , 
													int64& playDuration , 
													TianShanIce::Storage::ContentState& curState );

	int32					prepareItemStaticInfo(  TianShanIce::Streamer::PlaylistItemSetupInfo& info );

protected:

	int32					prepareItemRunTimeInfoFromVstrm( const TianShanIce::Streamer::PlaylistItemSetupInfo& info  , 
																int64& playDuration , 
																TianShanIce::Storage::ContentState& curState );

	int32					prepareItemStaticInfoFromVstrm( TianShanIce::Streamer::PlaylistItemSetupInfo& info );

		
private:

	int32					getReservedTicket( );
	
	int32					reserveBandwidthTickets( const PlaylistItemSetupInfos& infos );
	
	int32					releaseBandwidthTickets( );


	int32					prepareLoadParam(	const TianShanIce::Streamer::PlaylistItemSetupInfo& info ,												
												Ice::Long timeOffset,
												IN IOCTL_LOAD_PARAMS&	loadParams,
												OUT IOCTL_CONTROL_PARMS_LONG& loadParam );

	bool					hasEcmData( const TianShanIce::Streamer::PlaylistItemSetupInfo& info );

	USHORT					getEcmPid(  const TianShanIce::Streamer::PlaylistItemSetupInfo& info  );

	ULONG					getVstrmPort( );

	int32					setDvbAttr( DVB_SESSION_ATTR& dvbAttr , Ice::Int& muxRate);

	int						setEcmData(  LOAD_PARAM* loadPara,   const TianShanIce::Streamer::PlaylistItemSetupInfo& info  , int paraCount );

	bool					itemHasUrl(  const TianShanIce::Streamer::PlaylistItemSetupInfo& info  );

	std::string				setUrlData(  LOAD_PARAM* loadPara,   const TianShanIce::Streamer::PlaylistItemSetupInfo& info  , int& paraCount  );

	SPEED_IND				convertSpeed( const float& speed );

	VHANDLE					vstrmHandle( );	

	std::string				getItemFullPathName( const TianShanIce::Streamer::PlaylistItemSetupInfo& info );

	uint32					getCallBackId( const std::string& contextKey );

	bool					checkSession( SsStreamBase::iter it  );

	std::string				getProperty( const TianShanIce::Properties& props , const std::string& key );

	std::string				getNextUrl(  const TianShanIce::Streamer::PlaylistItemSetupInfo& info );

	int32					applyPokeHoleSession( const std::string& pokeHoleSessId );

	ULONG					getSpigotHandle( ULONG vstrmPort );

	void					collectRunningBandWidthTickets( std::vector<ULONG64>& tickets ) ;

private:
	
	SsServiceImpl&				ss;

	std::string					mContextKey;
	int							mUrlIndex;

	StreamSmithEnv*				mEnv;

	VstrmStreamerManager&		mStreamerManager ;
	VstrmSessionScaner&			mSessScaner;
	
	ECM_COMMON					mEcmData;
	size_t						mEcmDataCount;

	char						mCurrentUrl[512];

	std::string					mStreamPort;
	int							miStreamPort;

	StreamParams				mStreamInfo;

	DVB_SESSION_ATTR			dvbAttrs;//use when loading a item 

private:
	BOOLEAN						bRealeaseBWTicketOnTerm;
	VSTRMBANDWIDTHTICKETS		vstrmTickets;
	bool						mbEdgeServer;
	SsContext&					mSsCtx;

};

class VsrtmSessCallbackReqeust : public ZQ::common::ThreadRequest
{
public:
	VsrtmSessCallbackReqeust( ZQ::common::NativeThreadPool& pool , 
								SsEnvironment* environment,
								SsServiceImpl&	service,
								uint32 id , 
								ULONG sessId,
								const std::string& strResult );
	virtual ~VsrtmSessCallbackReqeust( );

public:
	bool				init(void);

	int					run(void);

	void				final(int retcode /* =0 */, bool bCancelled /* =false */);

private:
	SsServiceImpl&			ss;
	uint32					mCallbackId;
	StreamSmithEnv*			mEnv;
	SsEnvironment*			env;
	std::string				mStrResult;
	Ice::Long				mCreateTime;
	ULONG					mVstrmSessionId;
};

#define	VSTRMPROPKEY(x)	"VSTRMProp."#x

#define VSTRMPROP_POKE_SESSION		VSTRMPROPKEY(PokeHoleSessionId)

}}

#endif//_zq_StreamService_Vstrm_Command_header_file_h__

