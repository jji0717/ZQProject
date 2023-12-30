
#ifndef _ZQ_STREAMSMITH_VSTRM_CLASS_EX_HEADER_FILE_H__
#define _ZQ_STREAMSMITH_VSTRM_CLASS_EX_HEADER_FILE_H__

#include "StreamSmith.h"
#include "NativeThreadPool.h"
#include "Variant.h"
extern "C"
{
#include <VstrmUser.h>
#include <vstrmver.h>
}

namespace ZQ{
namespace StreamSmith {

#define DEFAULT_THREADPOOL_NUM  (10)
#include <map>
#include <string>

class VstrmClass;

typedef struct _tagBandwidthUsage 
{
	int64			cdnTotalBandwidth;
	int64			cdnUsedBandiwidth;
	int				cdnImportSessionCount;
	_tagBandwidthUsage( )
	{
		cdnUsedBandiwidth		= 0;
		cdnImportSessionCount	= 0;
		cdnTotalBandwidth		= 0;
	}

}BandwidthUsage;

class BandwidthUsageScanner : public ZQ::common::NativeThread
{
public:
	BandwidthUsageScanner( VstrmClass& c );
	~BandwidthUsageScanner( );

public:
	
	bool			init(void);
	int				run(void);
	void			final(void);
	void			stop( );
	void			notify( );

private:
	
	void			cdnBandwidthUsage( BandwidthUsage& usage );

private:
	VHANDLE			_hClass;
	VstrmClass&		_cls;
	bool			_bQuit;
	HANDLE			_hEvent;
};

class BandwidthEventScanner : public ZQ::common::NativeThread
{
public:
	BandwidthEventScanner( BandwidthUsageScanner& bandwidthScanner );
	virtual ~BandwidthEventScanner();

public:
	
	void				stop( );

protected:
	
	virtual int			run( );



private:
	//VstrmFindFirstBandwidthNotification
	BandwidthUsageScanner&			mScanner;
	VHANDLE							mHandleClass;
	bool							mbQuit;
};

class SpigotLinkScanner : public ZQ::common::NativeThread
{
public:
	SpigotLinkScanner( VstrmClass& c);
	virtual ~SpigotLinkScanner( );

public:
	
	int			run( );

	void		stop( );

private:
	ZQ::common::Cond		mCond;
	ZQ::common::Mutex		mMutex;
	bool					mbQuit;
	VstrmClass&				mVc;
};

class EdgeEventListener :public ZQ::common::NativeThread
{
public:
	EdgeEventListener( VstrmClass& c );
	~EdgeEventListener();

public:
	bool			init(void);
	int				run(void);
	void			final(void);
	
private:
	void			close();
private:
	VHANDLE			_hClass;
	VstrmClass&		_cls;
};



class VstrmClass 
{
public:
	VstrmClass(ZQ::common::NativeThreadPool& thpool = _gThreadPool);
	~VstrmClass( );

public:
	bool isNodeReady( );

	bool initVstrmClass( );
	
	void close();

	bool isValid();

	const VHANDLE handle() const;
	
	const char* getErrorText(const VSTATUS status, char* textbuf, const int maxlen);

public:
	friend class SpigotLinkScanner;

	#define		PORT_IN_USE		1
	#define		PORT_NORMAL		0

#define STATUS_READY			1
#define STATUS_OUTOFSERVICE		0

	typedef struct _tagPortChar
	{		
		int				portId;		//vstrm port index based on 0
		int				portInUse;	//port usage
		int				status;		//reserve for future use
		int				availOnSpigot;
		int64			lastReleasedTime;
		_tagPortChar()
		{
			portId				= 0;
			portInUse			= PORT_NORMAL;
			status				= STATUS_OUTOFSERVICE;
			availOnSpigot		= 0;
			lastReleasedTime	= 0;
		}
	}VstrmPortChar;

	typedef std::vector<VstrmPortChar> VSTRMPORTS;

	typedef struct _tagSpigotChar 
	{
		ULONG			spigotHandle;
		ULONG			spigotId;	//spigot id , not the spigot index
		int				status;		//spigot status		
		int				portLowPart;//the first port on this spigot
		int				portHighPart;//the last port on this spigot
		std::string		sourceIp;
		int				sourceBasePort;
		int64			firstUp;
		int64			lastUpdate;		
		std::map<std::string,std::string> props;
		
		_tagSpigotChar()
		{
			spigotHandle	= 0;
			spigotId		= 0;
			status			= STATUS_OUTOFSERVICE;
			portLowPart		= 0;
			portHighPart	= 0;
			sourceBasePort	= 0;
			firstUp			= 0;
			lastUpdate		= 0;			
		}

	}VstrmSpigotChar;	

	typedef std::map< int , VstrmSpigotChar >	VSTRMSPIGOTS;
	//			spigot index ; vstrm spigot character

	typedef void	(*SpigotStatusCallback)( const VstrmSpigotChar& spigotChar ,  const int spigotIndex,  void* pUserData );
	typedef void	(*bandwidthUsageCallBack)( const BandwidthUsage& bwUsage , void* pUserData );
	typedef void	(*NodeStatusCallback)( const std::string& nodeName , bool bUp, void* pUserData );

	void	getSpigotInfo( VSTRMSPIGOTS& spigots );

	void	updateSpigotStatus( ULONG spigotId , int status );

	void	registerSpigotStatusCallback( SpigotStatusCallback callback , void* pUserData  );
	
	void	registerBandwidthUsageCallback( bandwidthUsageCallBack callback ,  void* pUserData );

	void	registerNodeStatusCallback( NodeStatusCallback callback , void* pUserData );

	void	updateNodeStatus( int clusterId , int nodeId , bool bUp );

	ULONG	getSpigotIdFromPortId( ULONG portNum ) ;

	void	updateCdnBwUsage(  const BandwidthUsage& tmpUsage  );

	BandwidthUsage	getBwUsage( );

protected:

	void	createNodeStatusListener();

private:
	SpigotStatusCallback	_spigotStatusCallback;
	void*					_pSpigotCallbackUserData;

	bandwidthUsageCallBack	_bandwidthUsageCallback;
	void*					_pBandwidthUsageUserData;

	NodeStatusCallback		_nodeStatusCallback;
	void*					_pNodeStatusUserData;

	BandwidthUsage			_bwUsage;

	VSTRMSPIGOTS			m_spigotMap;
	VSTRMPORTS				m_vstrmPorts;

	ZQ::common::Mutex		m_portMapMutex;

	int						getPortFromVstrmPortsPool( int firstPort , int lastPort  );

	friend static VSTATUS	cbPortInfo(	HANDLE					vstrmClassHandle,
										PVOID				    pCtx,
										PEPORT_CHARACTERISTICS	portChars,
										ULONG					portCharSize,
										ULONG					currentPort,
										ULONG					portCount);


	friend static VOID		SpigotCallbackForEachPort(	void* pThis,
														PSPG_SPIGOT_CHARACTERISTICS	spigotChars,
														ULONG						spigotCharsLength,
														ULONG						spigotIndex,
														ULONG						spigotCount);
	
protected:

	///detect the spigot linkage status
	/// <0 for error
	/// 1 for link up 
	/// 0 for link down
	int						getSpigotLinkState( ULONG portId );
public:




	typedef struct _PORTCHARRACTER
	{
		std::string		sourceIp;
		int				sourcePort;
	}PORTCHARRACTER;
	

	int			refreshPortInfo();
	
	int			getPortCount();
	
	ZQ::common::NativeThreadPool& getThreadPool() { return _threadPool; }


	ULONG		GetUnUsePort( ULONG port=(ULONG)(-1) , int SpigotID=-1 );
	
	bool		GetPortProperty( ULONG port ,PORTCHARRACTER& prop );

	void		FreePortUsage(ULONG port);

	void		listStreamer(std::vector<int>& IDs);

	HANDLE		registerPokeSession( const std::string& strPokeSessionId );

	void		unregisterPokeSession( const std::string& strPokeSessionId );

	void		remoteSessionAvailableNotify( const char* strPokeSessionId );

protected:

	typedef struct _tagNatPokeInfo 
	{
		HANDLE					hNotifier;

	}NatPokeInfo;

	typedef std::map<std::string,NatPokeInfo>	NATPOKEINFOMAP;
	NATPOKEINFOMAP				pokeInfoMap;
	ZQ::common::Mutex			pokeInfoMapLoker;

public:
	bool							_bSpigotCallBackIsCalled;
private:
	
	SpigotLinkScanner				_SpigotStatusScanner;
	BandwidthUsageScanner			_bwUsageScanner;
	BandwidthEventScanner			_bwEventScaner;
	EdgeEventListener				_edgeEventListener;
	ZQ::common::NativeThreadPool&	_threadPool;

	VHANDLE _hVstrmClass;
	int     _outputPortCount;

	static VSTATUS	cbPortInfo(HANDLE					vstrmClassHandle,
								PVOID				    pCtx,
								PEPORT_CHARACTERISTICS	portChars,
								ULONG					portCharSize,
								ULONG					currentPort,
								ULONG					portCount);
		

	static VOID		SpigotCallbackForEachPort(void* pThis,
												PSPG_SPIGOT_CHARACTERISTICS	spigotChars,
												ULONG						spigotCharsLength,
												ULONG						spigotIndex,
												ULONG						spigotCount);

};

std::string getVstrmError( VHANDLE h,VSTATUS err );

}}//namespace ZQ::StreamSmith

#endif//_ZQ_STREAMSMITH_VSTRM_CLASS_EX_HEADER_FILE_H__
