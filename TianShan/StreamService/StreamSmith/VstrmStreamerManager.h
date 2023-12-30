
#ifndef _zq_StreamService_StreamSmith_Streamer_manager_header_file_h__
#define _zq_StreamService_StreamSmith_Streamer_manager_header_file_h__



#include <map>
#include <vector>
#include <Locks.h>
#include <SsEnvironment.h>
#include <SsServiceImpl.h>

#ifdef offsetof
	#undef offsetof
#endif

extern "C"
{
#include <VstrmUser.h>
}

namespace ZQ
{
namespace StreamService
{

#define SPIGOT_LINK_UP			1
#define SPIGOT_LINK_DOWN		-1

#define VSTRM_PORT_IN_USE		1
#define VSTRM_PORT_FREE			0


static const std::string	VSTRM_PORT_KEY		=	"VstrmPort";

class RemoteSessionMonitor
{
public:
	RemoteSessionMonitor(  SsEnvironment* environment  );
	virtual ~RemoteSessionMonitor( );

public:
	
	void		registerSession( const std::string& sessionId );

	void		unregisterSession( const std::string& sessionId );

	bool		checkRemoteSession( const std::string& sessionId , uint32 timeInMilliSeconds );

	bool		hasSession( const std::string& sessionId ) ;

	void		updateSession( const char* sessionId , int32 status );

private:
	SsEnvironment* env;
	typedef std::map<std::string,ZQ::common::Cond*> RSMAP;
	RSMAP				mRsMap;
	ZQ::common::Mutex	mMutex;
};

class VstrmStreamerManager : public ZQ::common::NativeThread
{
public:
	VstrmStreamerManager( SsEnvironment* environment );
	virtual ~VstrmStreamerManager( );

public:


	bool					initialize( );

	void					uninitialize( );

	VHANDLE					getVstrmHandle( ) const;

	int32					getClusterId( ) const;

	std::string				getVstrmError( ULONG errCode ) const;

	int32					getTotalVstrmPortCount( ) ;

	RemoteSessionMonitor&	getRSMonitor( );

	void					attachServiceInstance( SsServiceImpl* s );

	bool					getStreamingSourceAddress(   const std::string& streamerId , const std::string& strPortId , 
														std::string& Ipv4, std::string& ipV6, int32& port);

	ULONG					getSpigotHandle( ULONG vstrmPort ) const;

public:

	bool					allocateStreamPort(	IN const std::string& streamerId ,
												IN const std::string& portId ,
												IN const TianShanIce::SRM::ResourceMap&	resource );

	bool					releaseStreamPort(	IN const std::string& streamerId ,
												IN const std::string& portId );

	bool					listAllReplicas( OUT SsReplicaInfoS& infos );
	
	bool					isVstrmNodeReady( );

protected:

	typedef struct _PortAttr
	{
		ULONG		portId;
		int32		portStatus;
		bool		portAvailableOnSpigot;
	}PortAttr;


	

	bool					enumStreamer( );

	void					innervstrmSpigotCallback(	PSPG_SPIGOT_CHARACTERISTICS		spigotChars,
														ULONG							spigotCharsLength,
														ULONG							spigotIndex,
														ULONG							spigotCount );
	VSTATUS					innervstrmPortCallback(	HANDLE					VstrmClassHandle,													
													PEPORT_CHARACTERISTICS	portChars,
													ULONG					portCharSize,
													ULONG					currentPort,
													ULONG					portCount);


	friend static void		vstrmSpigotCallback( void*,PSPG_SPIGOT_CHARACTERISTICS,ULONG,ULONG,ULONG);
	friend static VSTATUS	vstrmPortCallback(HANDLE,PVOID,PEPORT_CHARACTERISTICS,ULONG,ULONG,ULONG);
	static void				vstrmSpigotCallback( void*							pCallback,
													PSPG_SPIGOT_CHARACTERISTICS	spigotChars,
													ULONG							spigotCharsLength,
													ULONG							spigotIndex,
													ULONG							spigotCount);
	static	VSTATUS			vstrmPortCallback(	HANDLE					VstrmClassHandle,
												PVOID				    pCtx,
												PEPORT_CHARACTERISTICS	portChars,
												ULONG					portCharSize,
												ULONG					currentPort,
												ULONG					portCount);
											
	bool					getStreamerAddress( ULONG boardNum , ULONG spigotIdx , std::string& ipv4 , std::string& ipv6 , int32& basePort );

	int32					getSpigotLinkState( ULONG portId );	

	bool					getFreeVstrmPort( int spigotIdx ,ULONG& portId );

	void					updateSpigotStatus( ULONG spigotHandle , int32 status );

	int						run( );

private:
	
	SsEnvironment*		env;
	

	typedef std::vector<PortAttr>	PORTS;
	typedef struct _SpigotAttr 
	{
		PORTS				ports;
		ULONG				spigotId;
		std::string			spigotIpv4;
		std::string			spigotIpv6;
		int32				baseUdpPort;
		int32				portLowBound;
		int32				portHighBound;
		int64				lastUpdate;
		int64				firstUp;
		int32				status;
	}SpigotAttr ;

	typedef std::map<int,SpigotAttr> SPIGOTS;
	SPIGOTS					mSpigots;
	ZQ::common::Mutex		mSpigotsMutex;
	VHANDLE					mVstrmHandle;
	int32					mClusterId;

	int32					mSpigotIndex;
	
	//to record all vstrm ports so I can check 
	PORTS					mTmpPorts;

private:
	
	bool					mbThreadQuit;
	VHANDLE					mEdgeEventVstrmHandle;
	RemoteSessionMonitor	mRSMonitor;
	SsServiceImpl*			ss;
};


}}
#endif//_zq_StreamService_StreamSmith_Streamer_manager_header_file_h__
