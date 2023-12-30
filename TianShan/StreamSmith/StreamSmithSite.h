#ifndef __STREAM_SMITH_SITE_H__
#define	__STREAM_SMITH_SITE_H__

#pragma warning(disable:4786)
#include "StreamSmithModule.h"
#include "NativeThreadPool.h"
#include "urlstr.h"
#include "SystemUtils.h"
#include "DynSharedObj.h"

#ifndef _RTSP_PROXY	
	#include "StreamSmithConfig.h"
    #include "VstrmClass.h"
	#include "Playlist.h"
	#pragma message(__MSGLOC__"Add StreamSmith Configuration here")
#else //_RTSP_PROXY
	#include "rtspProxyConfig.h"
#endif //_RTSP_PROXY


#ifndef TEST
#include "RtspSessionMgr.h"
#endif

#ifdef WITH_ICESTORM
	#include <EventChannel.h>
#endif

#include <vector>
#include <map>
#include <list>

namespace ZQ{
namespace StreamSmith {

extern ZQ::common::NativeThreadPool&			_gThreadPool;
// -----------------------------
// class StreamSmithSite
// -----------------------------
/// Definition of a vitural site. DefaultSite is a special instance of this class
class StreamSmithSite : public IStreamSmithSite, public SessionEvent
{	
public:
	StreamSmithSite(const char* pSitename);
	StreamSmithSite();
	~StreamSmithSite();
	// per site stacks:
	//  AuthUser        FixupRequest     ContentMap        FixupResponse
	// +------------+  +-------------+  +--------------+  +--------------+
	// | auh1       |  | freq1       |  |path->handler |
	//
	// the default site;
	static StreamSmithSite* _pDefaultSite;
	
	static std::string		_strIDsServerAddr;

	static std::string		_strApplicationLogFolder;

public: // IStreamSmithSite exports
	//Set IDS server address
	void		SetIdsServerAddr(std::string& strAddr);
	
	const char*	getAssetDictoryUrl();

	const char* getSiteName() { return _siteName.c_str(); }
	
	virtual const char*		getApplicationLogFolder( ) 
	{
		return _strApplicationLogFolder.c_str() ;
	}
	/// allow a request to be re-queued in the process stack
	///@param pReq the request or a sub-request to be queued
	///@param @phase specify the process stack to insert in, forward to the default site if the phase is before PARSE_HEADER
	///@return true if successful
	bool		postRequest(IClientRequestWriter* pReq, const IClientRequest::ProcessPhase phase);
	
	///find a virtual site by site name
	///@param sitename a string name of the virtual site mounted as. NULL means to get access to the default site
	///@return get the access to the virtual site
	static StreamSmithSite* findSite(const char* sitename);

	
	///open an existing playlist instance or create a new playlist
	///@param playlistGuid the Guid used to identify a playlist
	///@param bCreateIfNotExist true if create a new playlist with the give guid if it does not exists
	///@return pointer to the playlist instance
	IPlaylist* openPlaylist(const ZQ::common::Guid& playlistGuid,  const bool bCreateIfNotExist,
							const char* pCLientSessionID=NULL);


	
	///open an exist playlist instance or create a new playlist with resource
	///@param playlistGuid the Guid used to identify a playlist
	///@param bCreateIfNotExist true if create a new playlist with the give guid if it does not exists
	///@param serviceGroupID the target service group id
	///@param maxBitRate bandwith the playlist need	
	///@return pointer to the playlist instance
	IPlaylist* openPlaylist(const ZQ::common::Guid& playlistGuid,const bool bCreateIfNotExist,
							const int serviceGroupID,const int maxBitRate,
							const char* pCLientSessionID=NULL);
	

	
	static  IStreamSmithSite* getDefaultSite(){	return (IStreamSmithSite*)StreamSmithSite::_pDefaultSite;}

//	IClientSession* openClientSession(const char* sessId, 
//		const bool bCreateIfNotExist,
//		IConnection* conn, 
//		const IClientSession::SessType sessType = IClientSession::LocalSession);
//	
	IClientSession* createClientSession(const char* sessId, const char* uri,
								const IClientSession::SessType sessType = IClientSession::LocalSession) ;
	
	/// find the ClientSession with session ID return NULL if can't find it!
	IClientSession* findClientSession(const char* sessId, 
								const IClientSession::SessType sessType = IClientSession::LocalSession) ;

	bool		destroyClientSession(const char* sessionID);

	IServerRequest*	newServerRequest(const char* sessionID,const std::string& ConnectionID="");

	void			PostServerRequest(IServerRequest* pReq) ;

	virtual ZQ::common::Variant getInfo( int32 infoType ) ;

protected:
	
	/// find a SSMH_ContentHandle by the mounted path name
	///@param path the mounted path name, for example, "MovieOnDemand" from a RTSP Uri: "rtsp://mysite/MovieOnDemand"
	///@return the associated content handler entry, NULL if not found
	SSMH_ContentHandle findContentHandler(const char*path);
	
protected:// sub classes
	// -----------------------------
	// subclass AuthUserHandler
	// -----------------------------
	class AuthUserHandler : public ZQ::common::ThreadRequest
	{
		friend class StreamSmithSite;
	public:
		
		typedef std::vector < SSMH_AuthUser > Stack;
		
	protected:
		AuthUserHandler(ZQ::common::NativeThreadPool& pool, Stack& stack, IClientRequestWriter* pRequest);
		
		///destructor
		virtual ~AuthUserHandler();
		
		Stack& _stack;
		IClientRequestWriter* _pReq;
		
		bool init(void);
		
		int run(void);
		
		void final(int retcode =0, bool bCancelled =false);
		
	public:
		
		const uint8 getPriority(void) { return 50; }
	};
	
	// -----------------------------
	// subclass FixupRequestHandler
	// -----------------------------
	class FixupRequestHandler : public ZQ::common::ThreadRequest
	{
		friend class StreamSmithSite;
	public:
		
		typedef std::vector < SSMH_FixupRequest > Stack;
		
	protected:
		FixupRequestHandler(ZQ::common::NativeThreadPool& pool, Stack& stack, IClientRequestWriter* pRequest,int priority = -1);
		
		///destructor
		virtual ~FixupRequestHandler();
		
		Stack& _stack;
		IClientRequestWriter* _pReq;
		
		bool init(void);
		
		int run(void);
		
		void final(int retcode =0, bool bCancelled =false);	
	};
	
	class FixupServerRequestHandler : public ZQ::common::ThreadRequest
	{
		friend class StreamSmithSite;
	protected:
		typedef std::vector < SSMH_FixupServerRequest > Stack;
		FixupServerRequestHandler(ZQ::common::NativeThreadPool& pool,Stack& s,IServerRequest* pReq);
		virtual ~FixupServerRequestHandler();
		Stack&				_stack;
		IServerRequest*		_pReq;

		bool	init();
		int		run();
		void	final(int retcode =0, bool bCancelled =false);	
	};

	// -----------------------------
	// subclass ContentHandler
	// -----------------------------
	class ContentHandler : public ZQ::common::ThreadRequest
	{
		friend class StreamSmithSite;
	public:
		
	protected:
		ContentHandler(ZQ::common::NativeThreadPool& pool,SSMH_ContentHandle funcHandle, IClientRequestWriter* pRequest, int priority = -1 );
		
		///destructor
		virtual ~ContentHandler();
		
		SSMH_ContentHandle _func;
		IClientRequestWriter* _pReq;
		
		bool init(void);
		
		int run(void);
		
		void final(int retcode =0, bool bCancelled =false);
	protected:
		void	FillURIIntoClientSession();
	};
	
	// -----------------------------
	// subclass FixupResponseHandler
	// -----------------------------
	class FixupResponseHandler : public ZQ::common::ThreadRequest
	{
		friend class StreamSmithSite;
	public:
		
		typedef std::vector < SSMH_FixupResponse > Stack;
		
	protected:
		FixupResponseHandler(ZQ::common::NativeThreadPool& pool, Stack& stack, IClientRequestWriter* pRequest , int priority = 30);
		
		///destructor
		virtual ~FixupResponseHandler() ;
		
		Stack& _stack;
		IClientRequestWriter* _pReq;
		
		bool init(void);
		
		int run(void);
		
		void final(int retcode =0, bool bCancelled =false);
	};

	class EventFire:public ZQ::common::ThreadRequest
	{
		friend class EventSinkManager;
	public:
		EventFire(ZQ::common::NativeThreadPool& pool);
		~EventFire();
		
		//////////////////////////////////////////////////////////////////////////		
		typedef struct _tagEventData
		{
			int					_msgIndex;	//for debug
			int64				_ttl;
			uint32				_eventCode;//Event type code;
			uint32				_priority;
			uint32				_weighted;
			ZQ::common::Variant	_var;
			std::string			_strGuid;
			bool				_isValid;
			StreamSmithSite*	_pSite;
		}EventData;
		typedef	std::list<EventData>			EVENTDATALST;
		typedef EVENTDATALST::iterator			iterator;
		typedef EVENTDATALST::const_iterator	const_iterator;	
	public:		
		int		run(void);
		void	final(int retcode /* =0 */, bool bCancelled /* =false */);
		void	addNewEvent(uint32& dwEventCode,ZQ::common::Variant& EventData,
							StreamSmithSite* pSite,const std::string& strGuid,
							bool bNew=true);
		void	SetQuit();
	private:			

		EVENTDATALST				m_listData;		
		
		ZQ::common::Mutex			m_mutex;
		SYS::SingleObject    		m_hNewSinkData;
		bool						_bQuit;
		int							m_SleepTime;
		//int							m_msgTTL;
	};
	class EventSinkManager:public ZQ::common::ThreadRequest
	{
		friend class StreamSmithSite;		
	public:
		EventSinkManager(ZQ::common::NativeThreadPool& pool,StreamSmithSite* pSite,uint32 dwEventCode,ZQ::common::Variant& var,EventFire& ef,const std::string& strGuid);
		int run(void);
		void final(int retcode /* =0 */, bool bCancelled /* =false */);
	private:
		ZQ::common::Variant		_var;
		uint32					_eventCode;
		StreamSmithSite*		_pSite;
		EventFire&				_EventFire;
		std::string				_strGuid;
	};


	
public:
	void	RegisterFixupRequest(const SSMH_FixupRequest& pRequest)
	{
		using namespace ZQ::common;
		FixupRequestHandler::Stack::iterator iter;
		for(iter=StreamSmithSite::_stackFixupRequest.begin();iter!=StreamSmithSite::_stackFixupRequest.end();iter++)
		{
			if(*iter==pRequest)
			{
				glog(Log::L_DEBUG,"RegisterFixupRequest()## the FixupRequest %X is already in the stack",pRequest);
				return;
			}
		}
		glog(Log::L_DEBUG,"RegisterFixupRequest()##register a FixupRequest  %X",pRequest);
		StreamSmithSite::_stackFixupRequest.push_back(pRequest);
	}
	void	RegisterFixupResponse(const SSMH_FixupResponse& pResponse)
	{
		using namespace ZQ::common;
		FixupResponseHandler::Stack::iterator	iter;
		for(iter=StreamSmithSite::_stackFixupResponse.begin();iter!=StreamSmithSite::_stackFixupResponse.end();iter++)
		{
			if(*iter==pResponse)
			{
				glog(Log::L_DEBUG,"RegisterFixupResponse()## the FixupResponse %X is already in the stack",pResponse);
				return;
			}
		}
		glog(Log::L_DEBUG,"RegisterFixupResponse()##Register a FixupResponse %X",pResponse);
		StreamSmithSite::_stackFixupResponse.push_back(pResponse);
	}
	
	void	RegisterFixeupServerRequest(const SSMH_FixupServerRequest& enFixupServerRequest)
	{
		using namespace ZQ::common;
		//_stackFixupServerRequest
		FixupServerRequestHandler::Stack::iterator it;
		for(it=StreamSmithSite::_stackFixupServerRequest.begin();
			it!=StreamSmithSite::_stackFixupServerRequest.end();
			it++)
		{
			if(*it == enFixupServerRequest)
			{
				glog(ZQ::common::Log::L_DEBUG,
						"RegisterFixeupServerRequest()## the FixupServerRequest %X is already in the stack",
						enFixupServerRequest);
				return;
			}
		}
		glog(ZQ::common::Log::L_DEBUG,
				"RegisterFixeupServerRequest()## REgister FixupServerRequest %X",
				enFixupServerRequest);
		StreamSmithSite::_stackFixupServerRequest.push_back(enFixupServerRequest);
	}

	void	RegisterContentHandle(const char* str,const SSMH_ContentHandle& ctntHandle)
	{
		using namespace ZQ::common;
		if (!(str&&strlen(str)>0)) 
		{
			glog(ZQ::common::Log::L_ERROR,"RegisterContentHandle() invalid handler name passed in");
			return;
		}
		
		std::string defaulthandler= m_pRequestHandler->GetDefaultContenHandler();
		if ( !defaulthandler.empty() && defaulthandler ==  std::string(str) ) 
		{
			_defaultContentHandler =  ctntHandle;
			glog(ZQ::common::Log::L_INFO,"RegisterContentHandle() register a default contentHanlder with [%s]",str);
			return;
		}
		else
		{
			//glog(ZQ::common::Log::L_ERROR,"RegisterContentHandle() no matched path is found,return without registered");
			//return;
		}
		
		//This function must depend on the configuration!!!!!
		std::vector<std::string>		vecStrpath;
		if( !m_pRequestHandler->GetVSitePathFromHandler(getSiteName(),str,vecStrpath))
		{
			glog(Log::L_ERROR,"RegisterContentHandle()## can't get path with sitename=%s and handler=%s",getSiteName(),str);			
			return;
		}	
		if(vecStrpath.size()<=0)
		{
			glog(Log::L_ERROR,"RegisterContentHandle()## can't get path with sitename=%s and handler=%s",getSiteName(),str);
			return;
		}
		for(int i=0;i<(int)vecStrpath.size();i++)
		{
			glog(Log::L_DEBUG,"RegisterContentHandle()## register Contenthandler with hanlder=%s and path=%s and proc address=%X",str,vecStrpath[i].c_str(),ctntHandle);
			std::pair<std::string, SSMH_ContentHandle> p(vecStrpath[i],ctntHandle);
			_mapContentHandle.insert(p);
		}
	}


	void	CoreLog	(ZQ::common::Log::loglevel_t level,char* fmt,...) PRINTFLIKE(3, 4)
	{
		using namespace ZQ::common;
		if(!fmt)
		{
			glog(Log::L_ERROR,"ThrowLogToMainModule()## null msg pass in from dll module");
			return;
		}	
		char msg[2048];
		memset(msg, 0, 2048);
		va_list args;
		
		va_start(args, fmt);
		vsprintf(msg, fmt, args);
		va_end(args);
		
		//glog(level,msg);		
		(*_PluginLog)(level,msg);
	}
	void SinkEvent(const uint32 dwEventType,const SSMH_EventSink& EventHandler)
	{
		using namespace ZQ::common;
		EventSinkStack::iterator iter;
		for(iter=_stackEventSink.begin();iter!=_stackEventSink.end();iter++)
		{
			if(EventHandler==iter->second)
			{
				glog(Log::L_DEBUG,"void RegisterEventSink()## The EventHanlder %X has been registered!",EventHandler);
				return;
			}
		}
		std::pair<uint32,SSMH_EventSink>	p(dwEventType,EventHandler);
		_stackEventSink.push_back(p);
	}

	//////////////////////////////////////////////////////////////////////////
	// added by Cary
public:
	virtual	void RegisterSessionDrop(SSMH_SessionDrop sessionDropHandle);

protected:

	bool checkRequestWithSessionPriority( IClientRequestWriter * req , const char* sessId , int& priority );

	typedef std::vector<SSMH_SessionDrop> SessionDropHandlers;
	SessionDropHandlers			_sessionDropHandlers;
protected:
	virtual void onSessionRemoved(const std::string& sessionId);

	// added by Cary
	//////////////////////////////////////////////////////////////////////////
	
public:
	void		setSitename(const char* pSitename){_siteName=pSitename;}
	void		setSitename(std::string& sitename){_siteName=sitename;}
	void		setLogInstance(ZQ::common::Log* pLog){_PluginLog=pLog;}
	void		setDefaultSpigotsBoards(const std::vector<int>& bdIDs);
	
	static void	StartService(IClientRequest* pReq);
	//Setup all site,include default Site and virtual site
	static bool	SetupStreamSmithSite(const char *pConfPath,std::vector<std::string>& configPath);
	//Destroy all site,exclude default site.The default site will destroy it itself!
	static void	DestroyStreamSmithSite();

public:	
	void		PostEventSink(uint32 dwEventCode,ZQ::common::Variant& EventData,const std::string& strGuid);	
public:

#ifndef _RTSP_PROXY
    static PlaylistManager*								m_pPlayListManager;
	static VstrmClass*									m_pvstrmClass;
#endif


	static	SSMH_ContentHandle							_defaultContentHandler;

	

#ifndef TEST
	static RtspSessionMgr*								m_pSessionMgr;
#endif
	//Global sink eventSink stack
	typedef std::vector<std::pair<uint32,SSMH_EventSink> >	EventSinkStack;
	static EventSinkStack								_stackEventSink;
public:

	RtspRequestHandler*									m_pRequestHandler;
	

protected:
	typedef std::map <std::string, SSMH_ContentHandle>	ContentHandlerMap;
	typedef	ContentHandlerMap::iterator					IterContentHanldermap;

//	typedef std::map<std::string,IClientSession*>		ClientSessionMap;
//	typedef	std::map<std::string,IClientSession*>		IterClientSessionmap;
	

	std::string											_siteName;	
	

	static EventFire*									m_pEventFire;
	// per-site definition
	ContentHandlerMap									_mapContentHandle;

	//map of client session
//	ClientSessionMap									_mapClientSession;

	//
	static	ZQ::common::Log*							_PluginLog;	

	// stacks shared among multiple sites

	//These two are global
	static FixupRequestHandler::Stack					_stackFixupRequest;
	static FixupResponseHandler::Stack					_stackFixupResponse;	
	static FixupServerRequestHandler::Stack				_stackFixupServerRequest;
	//

	static std::vector< StreamSmithSite* >				_stackStreamSmithSite;
	static std::vector<ZQ::common::DynSharedObj*>		_stackPluginModuleHandle;
	static std::vector<int>								_defaultSpigotsBoardIDs;
	
};
extern StreamSmithSite									defaultSite;
}//namespace StreamSmith
}//namespace ZQ
#endif
