// LscDialogImpl.h: interface for the LscDialogImpl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LSCDIALOGIMPL_H__5BBDD4E3_B3EB_4E20_BBD9_2E455EDEDDC3__INCLUDED_)
#define AFX_LSCDIALOGIMPL_H__5BBDD4E3_B3EB_4E20_BBD9_2E455EDEDDC3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <string>
#include <map>

#include "DataPostHouseService.h"
#include "RtspSessionMgr.h"
#include "proxydefinition.h"
#include <lsc_parser.h>

using namespace ZQ::DataPostHouse;

#ifndef _NO_NAMESPACE
namespace ZQ {
	namespace StreamSmith {
#endif // #ifndef _NO_NAMESPACE

#define LSC_HEADER_SESSION		"Session"
#define LSC_HEADER_CSEQ			"CSeq"
#define LSC_HEADER_SCALE		"Scale"
#define LSC_HEADER_RANGE		"Range"
#define LSC_HEADER_METHOD		"Method"
#define LSC_HEADER_STATUS		"Status"
#define LSC_HEADER_SERVERMODE	"ServerMode"

enum
{
	LSC_KEY_SCALE,
	LSC_KEY_CURRENTNPT,
	LSC_KEY_STARTNPT,
	LSC_KEY_STOPNPT,
	LSC_KEY_CSEQ,
	LSC_KEY_SESSIONID,
	
	LSC_KEY_INVALID	//invalid key
};

class LscConnection;

typedef  ZQ::common::Pointer<LscConnection>  LscConnectionPtr;

class LscDialog : public IDataDialog
{
public:
	LscDialog( RtspSessionMgr& sessionMgr );
	virtual ~LscDialog( );
public:
	virtual void onCommunicatorSetup(IDataCommunicatorPtr communicator);
	virtual	void onCommunicatorDestroyed(IDataCommunicatorPtr communicator);
	virtual	bool onRead(const int8* buffer, size_t bufSize);
	virtual	void onWritten(size_t bufSize);
	virtual	void onError();

protected:
	
	lsc::lscMessage*			_reservedMsg;

	RtspSessionMgr&				_sessionMgr;
	LscConnectionPtr            _conn;
	std::string					_strSavedmsg;
	std::string					_strPeerIP,_strPeerPort,_strServerIP,_strServerPort;
};

class LscConnection : public IConnectionInternal , public ZQ::common::Mutex
{
	friend class LscDialog;
	friend class RtspSessionMgr;
	
public:
	
	LscConnection(IDataCommunicatorPtr conn , int connType);
	~LscConnection()
	{
		ZQ::common::MutexGuard guard(*this);
		//it's useless if connection type is UDP
		_conn->close();
	}
	
	
public:
	virtual const char* getRemoteIP(char* IP, const uint IPLen, uint16* port)
	{		
		//ZQ::common::MutexGuard guard(*this);
		strncpy(IP,_strPeerIP.c_str(),IPLen);
		*port = (uint16)_iPeerPort;		
		return IP;
	}
	virtual const char* getLocalIP(char* IP,const uint IPLen,uint16* port)
	{
		strncpy(IP,_strLocalIP.c_str(),IPLen);
		*port = (uint16)_iLocalPort;		
		return IP;
	}
	
	virtual uint32 recv(uint8* buf, const uint32 maxlen);

	virtual uint32 send(const uint8* buf, const uint32 maxlen);
	
	virtual bool close();	
	
	virtual bool isActive()
	{
		ZQ::common::MutexGuard guard(*this);
		return _conn->isValid();
	}
	
	void	updateLastSession(const char* sessID)
	{
		ZQ::common::MutexGuard guard(*this);
		_lastSessID = sessID;
	}
	
	
	std::string getLastSessionID()
	{
		ZQ::common::MutexGuard guard(*this);
		
		return _lastSessID;
	}
	
	
	SOCKET getSocket()
	{
		return 0;
	}
	uint64	getConnectionIdentity()
	{
		return _conn->getCommunicatorId();
	}	
	
	
	void	setPeerInfo  ( const std::string& strPeerIP , const std::string& strPeerPort );
	
	void	setLocalInfo ( const std::string& strLocalIP, const std::string& strLocalPort );
	
	void	setPeerInfo (const CommAddr& peer );
	
	virtual void				setConnectionProtocol ( int protocolType )
	{
		_connProtocol = protocolType;
	}
	
	virtual int					getConnectionProtocol ( )
	{
		return _connProtocol;
	}

private:
	int					_connProtocol;
	IDataCommunicatorPtr _conn;
	
	ZQ::common::Mutex	_refLock;
	volatile long		_ref;
	
	std::string			_lastSessID;
	
	std::string			_strPeerIP;
	int					_iPeerPort;
	
	std::string			_strLocalIP;
	int					_iLocalPort;
	
	CommAddr			_peerInfo;	//Used for UDP
	
	int					_iConnType;//connection type TCP or UDP
};

class LscServerRequest: public IServerRequest 
{
public:
	LscServerRequest(IConnectionInternalPtr conn, const char* clientSessID);
	
	virtual ~LscServerRequest();
	
	virtual int			printCmdLine(const char* startline);
	
	virtual int			printHeader(char* header, char* value);
	
	virtual int			printMsgBody(char* msg);
	
	virtual int			post();	
	
	virtual int			closeConnection();
	
	virtual void		release();

	virtual const char* getClientSessionID();	

	void				reference();
	
protected:
	
	IConnectionInternalPtr		_conn;
	lsc::LSCMESSAGE				_msg;//lsc message	
	std::string					_strClientSessionID;

	ZQ::common::Mutex			_refLock;
	volatile long				_ref;
};

class LscClientRequest;

class LscResponse : public IServerResponse
{
public:
	LscResponse(LscClientRequest& request , int64 dwReceivedTime);

	virtual ~LscResponse();

public:
	virtual void printf_preheader(const char* content) ;

	virtual void printf_postheader(const char* content) ;

	virtual void setHeader(const char* key, const char* value) ;

	virtual const char* get_preheader(IN OUT char* value , IN OUT uint16* valueLen ) ;

	virtual const char* get_postheader(IN OUT char* value , IN OUT uint16* valueLen ) ;

	virtual const char* getHeader(IN const char* key ,IN OUT char *value,IN OUT uint16* valueLen) ;

	virtual uint32 post() ;

	void			setLscMsg(const lsc::LSCMESSAGE& msg);

protected:

private:	
	lsc::LSCMESSAGE		_msg;
	LscClientRequest&	_request;
	int64		_dwRecvTime;
};


class LscClientRequest : public IClientRequestWriterInternal
{
	friend class LscResponse;
public:
	
	LscClientRequest( lsc::lscMessage* lscMsg, LscConnectionPtr conn, int64 dwRecvTime );

	virtual ~LscClientRequest( );
	
	virtual void setContext(const char* key  , const char* value);

	virtual const char* getContext( const char* key );
	
	virtual void addRef( );

	virtual void release( );
	
	virtual IClientRequest::ProcessPhase getPhase() ;
	
	virtual REQUEST_VerbCode getVerb() ;
	
	virtual const char* getStartline( char *buf , int bufLen ) ;
	
	virtual const char* getUri(char* uri, int len) ;
	
	virtual const char* getProtocol(char* protocol, int len) ;
	
	virtual const char* getHeader(IN const char* key, OUT char* value, IN OUT uint16* maxLen) ;
	
	virtual bool getTransportParam(IN const char* subkey, OUT char* value, 	IN OUT uint16* maxLen) ;
	
	virtual const char* getContent(unsigned char* buf, uint32* maxLen) ;
	
	virtual IServerResponse* getResponse() ;
	
	virtual IStreamSmithSite* getSite() ;
	
	virtual const char* getClientSessionId() ;
	
	virtual bool getRemoteInfo(RemoteInfo& info) ;
	
	virtual bool getLocalInfo(LocalInfo& info) ;
	
	virtual bool  setArgument(REQUEST_VerbCode verb, const char* uri, const char* protocol) ;
	
	virtual void setHeader(const char* key, char* value) ;
	
	virtual void setContent(const unsigned char* content, const uint32 len) ;
	
	virtual const char* read(const uint32 offset, const unsigned char* content, const uint32* plen) ;
	
	virtual const char* write(const unsigned char* reqcontent, 	const uint32* plen) ;
	
	virtual bool setSite(IStreamSmithSite* pSite) ;
	
	virtual void setUserCtxIdx(const char* userCtxIdx);
	
	virtual bool setPhase(ProcessPhase phase);

	virtual bool checkConnection()
	{
		return _lscConn->isActive();
	}

	virtual int64 getRequestInitTime() const 
	{
		return _requestInitTime;
	}

	bool						ParseLscIntoRtspHeader( );
	
	IConnectionInternalPtr      getConnection();

protected:
	int							GetKey(const char* keyStr);
	

	
	
private:
	bool						AddSessionField(unsigned int value);
	bool						AddHeaderField(const char* key ,unsigned int value);
	bool						AddRangeField(const char* key ,unsigned int value ,unsigned int value2);
	bool						AddScaleField(short numerator , unsigned short denominator);

protected:
	lsc::lscMessage*					_lscMessage;
	//It's the pointer to lsc message
	//Delete it when client request is gone

	IConnectionInternalPtr				_lscConn;
	ProcessPhase						_phase;
	IStreamSmithSite*					_site;

	LscResponse							_response;
	void*								_userContxt;
	REQUEST_VerbCode					_verb;

	struct icmp
	{
		bool operator()(const std::string& str1 , const std::string& str2) const
		{
		#ifdef ZQ_OS_MSWIN
			return stricmp( str1.c_str() , str2.c_str() ) < 0;
		#else
			return strcasecmp( str1.c_str() , str2.c_str() ) < 0;
		#endif
		}
	};
	typedef std::map<std::string , std::string , icmp >	KEYVALUEPIR;
	KEYVALUEPIR							_header;	//perform like a rtsp header and no content body is available
	
	std::map<std::string,std::string>	_contextMap;

	ZQ::common::Mutex					_refLock;
	long volatile						_ref;
	int64								_requestInitTime;
};


#ifndef _NO_NAMESPACE
	} // namespace StreamSmith {
} // namespace ZQ {
#endif // #ifndef _NO_NAMESPACE


#endif // !defined(AFX_LSCDIALOGIMPL_H__5BBDD4E3_B3EB_4E20_BBD9_2E455EDEDDC3__INCLUDED_)
