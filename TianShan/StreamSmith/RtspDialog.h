// RtspDialog.h: interface for the RtspDialog class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RTSPDIALOG_H__D3FB2C3C_A761_4CD6_9C49_CFC5329601E3__INCLUDED_)
#define AFX_RTSPDIALOG_H__D3FB2C3C_A761_4CD6_9C49_CFC5329601E3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "StringData.hxx"
#include "RtspMsg.hxx"
#include "RtspRequest.hxx"
#include "RtspResponse.hxx"
#include "RtspMsgParser.hxx"

#include "DataPostHouseService.h"
#include "RtspSessionMgr.h"
#include <Locks.h>
#include <deque>

#include "TimeUtil.h"
#include "proxydefinition.h"
using namespace ZQ::common;

#ifndef _NO_NAMESPACE
namespace ZQ {
	namespace StreamSmith {
#endif // #ifndef _NO_NAMESPACE

#define CRLF	"\r\n"

using namespace ZQ::common;
using namespace ZQ::DataPostHouse;

class RtspConnection;

typedef  ZQ::common::Pointer<RtspConnection>   RtspConnectionPtr;

class RtspDialog : public IDataDialog
{
protected:
	void fireTeardownForConnectionLostRequest();
	void dummyPing();

	RtspSessionMgr&		_sessionMgr;
	RtspConnectionPtr   _conn;
	std::string			_strSavedMsg;	

public:
	RtspDialog(RtspSessionMgr& sessionMgr);
	virtual ~RtspDialog();

public:
	virtual void onCommunicatorSetup(IDataCommunicatorPtr communicator);
	virtual	void onCommunicatorDestroyed(IDataCommunicatorPtr communicator);
	virtual	bool onRead(const int8* buffer, size_t bufSize);
	virtual	void onWritten(size_t bufSize);
	virtual	void onError();

	RtspConnectionPtr getConnection()
	{
		return _conn;
	}
};



class RtspConnection : public IConnectionInternal, public ZQ::common::Mutex 
{
	friend class RtspDialog;
	friend class RtspSessionMgr;	
public:
	RtspConnection(IDataCommunicatorPtr communicator);

	virtual ~RtspConnection();
	

	virtual const char* getRemoteIP(char* IP, const uint IPLen, 
		uint16* port)
	{
		//当然现在取得的Remote IP port都不是实时的!但是这个东西应该不会随时更改的
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

	virtual uint32 recv(uint8* buf, const uint32 maxlen)
	{
		return (uint32 )_mainConn->read(reinterpret_cast<int8*>(buf), (int )maxlen);
	}

	virtual uint32 send(const uint8* buf, const uint32 maxlen)
	{
		return (uint32 )_mainConn->write(reinterpret_cast<const int8*>(buf), (int )maxlen, 10000);
	}

	virtual bool close();	
	virtual bool isActive()
	{
		ZQ::common::MutexGuard guard(*this);
		return _mainConn->isValid();
	}
	
	void	updateLastSession(const char* sessID)
	{
		ZQ::common::MutexGuard guard(*this);
		_lastSessionId = sessID;
	}
	bool	assocSession(RtspSession* sess);

	std::string getLastSessionID()
	{
		ZQ::common::MutexGuard guard(*this);
		return _lastSessionId;
	}
	
	// for debug.
	SOCKET getSocket()
	{
		return 0;
	}
	uint64	getConnectionIdentity()
	{
		return _mainConn->getCommunicatorId();
	}
	virtual void				setPeerInfo  ( const std::string& strPeerIP , const std::string& strPeerPort )
	{
	}
	
	virtual void				setLocalInfo ( const std::string& strLocalIP, const std::string& strLocalPort )
	{
	}

	virtual void				setConnectionProtocol ( int protocolType )
	{
		_ConnectionPrtocol= protocolType;
	}
	
	virtual int					getConnectionProtocol ( ) 
	{
		return _ConnectionPrtocol;
	}

	void sessiondownNotify(const std::string& strSessID);
	void connectionDown();

	const char* getConnIdent(){return _szConnIdent;}
	const char* getLocalIp(){return _strLocalIP.c_str();}
	const char* getLocalPort(){return _strLocalPort.c_str();}
	const char* getLogHint(){return _szLogHint;}
protected:
	int					_ConnectionPrtocol ;
	IDataCommunicatorPtr _mainConn;

	ZQ::common::Mutex	_refLock;
	volatile long		_ref;

	std::string			_lastSessionId;

	std::string			_strLocalIP;
	std::string			_strLocalPort;
	std::string			_strPeerIP;
	std::string			_strPeerPort;
	char				_szConnIdent[32];
	char				_szLogHint[128];
	
	int					_iPeerPort;
	int					_iLocalPort;
};

struct RTSPBUFFER
{
	char* buf;
	int   dataLen;
	int   size;
};

class ParseProcThrd : public ZQ::common::NativeThread
{
public:
	static void pushReq(RTSPBUFFER& pReq, RtspConnectionPtr pConn, int64 dwRecvTime)
	{
		{
			ZQ::common::MutexGuard gd(ParseProcThrd::_lock);	
			PARSEREQUEST parseReq;
			parseReq.dwRecvTime = dwRecvTime;
			parseReq.pConn = pConn;		
			parseReq.pReq = pReq;
			_req_que.push_back(parseReq);
		}
#ifdef ZQ_OS_MSWIN
		ReleaseSemaphore(_hEvent, 1, NULL);
#else
		sem_post(&_hEvent);	
#endif
	};
	
	static void pushReq(const std::vector<RTSPBUFFER>& pReqs, RtspConnectionPtr pConn, int64 dwRecvTime)
	{
		{	
			ZQ::common::MutexGuard gd(ParseProcThrd::_lock);	
			std::vector<RTSPBUFFER>::const_iterator it;
			for(it=pReqs.begin();it!=pReqs.end();it++)
			{
				PARSEREQUEST parseReq;
				parseReq.dwRecvTime = dwRecvTime;
				parseReq.pConn = pConn;		
				parseReq.pReq = *it;
				_req_que.push_back(parseReq);
			}
		}
#ifdef ZQ_OS_MSWIN
		ReleaseSemaphore(_hEvent, pReqs.size(), NULL);
#else
		for(size_t ns = 0; ns < pReqs.size(); ns++)
			sem_post(&_hEvent);
#endif
	}
#ifdef ZQ_OS_MSWIN	
	static bool create(int nThreadCount, int nThreadPriority=THREAD_PRIORITY_NORMAL);
#else
	static bool create(int nThreadCount, int nThreadPriority=0);
#endif

	static void close();

	static void getRtspBuf(int nSize, RTSPBUFFER& buf)
	{
		_buflock.enter();
		if (nSize<MINI_RTSPBUF_SIZE)
		{
			if (_rtsp_buf_que.empty())
			{
				buf.buf= new char[MINI_RTSPBUF_SIZE];
				buf.size = MINI_RTSPBUF_SIZE;
			}
			else
			{
				buf=_rtsp_buf_que.front();
				_rtsp_buf_que.pop_front();
			}
		}
		else
		{
			if (_rtsp_bigbuf_que.empty())
			{
				buf.buf= new char[nSize + MINI_RTSPBUF_SIZE];
				buf.size = nSize + MINI_RTSPBUF_SIZE;
			}
			else
			{
				buf=_rtsp_bigbuf_que.front();
				if (buf.size >= nSize )
					_rtsp_bigbuf_que.pop_front();
				else
				{
					buf.buf= new char[nSize + MINI_RTSPBUF_SIZE];
					buf.size = nSize + MINI_RTSPBUF_SIZE;
				}
			}
		}
		_buflock.leave();		
	}

	static void freeRtspBuf(RTSPBUFFER& buf)
	{
		_buflock.enter();
		if (buf.size > MINI_RTSPBUF_SIZE)
		{
			_rtsp_bigbuf_que.push_front(buf);
		}
		else
		{
			_rtsp_buf_que.push_front(buf);
		}
		_buflock.leave();
	}

protected:
	ParseProcThrd(){};
	struct PARSEREQUEST
	{
		int64 dwRecvTime;
		RtspConnectionPtr pConn;
		RTSPBUFFER pReq;
	};

	
	virtual int run(void);

	bool processReq(PARSEREQUEST& parseReq);

	enum {MINI_RTSPBUF_SIZE=2048};
	
	static	ZQ::common::Mutex _lock;
	static	std::deque<PARSEREQUEST>	_req_que;

	static	ZQ::common::Mutex _buflock;
	static	std::deque<RTSPBUFFER>	_rtsp_buf_que;
	static	std::deque<RTSPBUFFER>	_rtsp_bigbuf_que;
	static	bool		_bQuit;
	static  std::vector<ParseProcThrd*>	_parse_thd;
#ifdef ZQ_OS_MSWIN
	static  HANDLE		_hEvent;
#else
	static sem_t		_hEvent;	
#endif
};

class RtspServerRequest: public IServerRequest {

public:
	RtspServerRequest(IConnectionInternalPtr conn,const char* clientSessID);
	virtual ~RtspServerRequest();
	/// output start line of request
	virtual int printCmdLine(const char* startline);
	/// output headers of request
	virtual int printHeader(char* header, char* value);
	/// output message body of request
	virtual int printMsgBody(char* msg);
	/// post the message to a client
	virtual int post();
  
	/// close current connection
	virtual int closeConnection();
	/// release itself
	virtual void release();
	virtual const char* getClientSessionID()
	{
		return _strClientSessionID.c_str();
	}
	void		reference();

protected:
	IConnectionInternalPtr		_conn;
	RtspRequest			_rtspRequest;
	std::string			_strClientSessionID;
	ZQ::common::Mutex	_refLock;
	volatile long		_ref;
};

class StreamSmithUtilityImpl {
public:
	virtual ~StreamSmithUtilityImpl(){}
	
	virtual bool getVerbString(RTSP_VerbCode verb, char buf[], 
		uint32 size);

	// virtual bool getHeaderString(RTSP_VerbCode header, char buf[], 
	//	uint32 size);
};

//////////////////////////////////////////////////////////////////////////
class RtspClientRequest;

class RtspServerResponse: public IServerResponse
{
public:

	RtspServerResponse(RtspClientRequest& req, int64 dwRecvTime) : 
	  _rtspReq(req)
	{
		  _recvTime = dwRecvTime;
		  _constructTime = now();
		  _rtspResponse.setHeaders(0);
	}

#ifdef _DEBUG
	virtual ~RtspServerResponse()
	{
		_rtspResponse.setHeaders(0);
	}
#endif

	/// print some content before the SDP response header fields
	///@param content the string to print before the header fields, for example: "200 OK\r\n" for http
	///@return the start offset of the printed string
	virtual void printf_preheader(const char* content)
	{
		if (content && content[0] ) 
		{
			_rtspResponse.setStartLine(Data(content));
		}
	}

	/// print some content after the SDP response header fields
	///@param content the string to print after the header fields, for example: "\r\n<html></html>" for http
	///@return the start offset of the printed string
	virtual void printf_postheader(const char* content)
	{
		if ( content && content[0] ) 
		{
			_rtspResponse.setMsgBody(Data(content));			
		}
	}

	/// set a header field
	///@param key the field name
	///@param value the field value
	///@return pointer to the key if succes
	virtual void setHeader(const char* key, const char* value)
	{
		if ( key&&key[0]&&value) 
		{
			_rtspResponse.setAddedHdrBodyData(key, value);
		}
	}
	///get the start line
	///@return the start line ,NULL if no start line
	///@param value the buffer to hold the start line content
	///@param valueLen the buffer size in byte
	virtual const char* get_preheader(IN OUT char* value , IN OUT uint16* valueLen )
	{
		const char * p = _rtspResponse.getStartLine().getData();		
		if (p) 
		{
			uint16 iLen = strlen(p);			
			strncpy(value,p,*valueLen);
			if(*valueLen<=iLen)
			{
				value[*valueLen - 1] = '\0';
				*valueLen -= 1;
			}
			else
			{
				*valueLen = iLen;
			}
			
			return value;
		}
		else
		{
			return NULL;
		}
	}

	///get the content body
	///@return the content body,NULL if there is no content body
	///@param value the buffer to hold the body content
	///@param valueLen the buffer size in byte
	virtual const char* get_postheader(IN OUT char* value , IN OUT uint16* valueLen )
	{
		const char* p = _rtspResponse.getMsgBody().getData();
		if (p) 
		{
			uint16 iLen = strlen(p);			
			strncpy(value,p,*valueLen);
			if(*valueLen<=iLen)
			{
				value[*valueLen - 1] = '\0';
				*valueLen -= 1;
			}
			else
			{
				*valueLen = iLen;
			}
			
			return value;
		}
		else
		{
			return NULL;
		}
	}

	///get header field value through its key
	///@return the value ,NUll if no value is associated with the key
	///@param key the field key
	///@param value the buffer to hold the field value
	///@param valueLen the buffer size in byte
	virtual const char* getHeader(IN const char* key ,IN OUT char *value,IN OUT uint16* valueLen)
	{
//		const char *p = _rtspResponse.getAddedHdrBodyData(key).getData();
		Data res = _rtspResponse.getAddedHdrBodyData(key);
		const char *p = res.getData();
		if (p) 
		{
			uint16 iLen = strlen(p);			
			strncpy(value,p,*valueLen);
			if(*valueLen<=iLen)
			{
				value[*valueLen - 1] = '\0';
				*valueLen -= 1;
			}
			else
			{
				*valueLen = iLen;
			}
			
			return value;
		}
		else
		{
			return NULL;
		}
	}


	/// post a server response on the given connection
	///@param pConn the connection that the response want to be sent via
	///@return count of the byte sent
	virtual uint32 post();

protected:
	/*
	This time can be treat as the start time of the client request
	*/
	int64				_constructTime;
	int64				_recvTime;

	RtspResponse		_rtspResponse;
	RtspClientRequest&	_rtspReq;
};

//////////////////////////////////////////////////////////////////////////

class RtspClientRequest : public IClientRequestWriterInternal
{
	friend class RtspDialog;
public:

	RtspClientRequest(RtspRequest* req, RtspConnectionPtr conn, int64 dwRecvTime);

	virtual ~RtspClientRequest();

	virtual void setContext(const char* key  , const char* value) 
	{
		if( key && key[0]!=0 )
		{
			if(value)
			{
				_contextMap[key]= value;
			}
			else
			{
				_contextMap.erase(key);
			}
		}
	}
	virtual bool checkConnection()
	{
		return _conn->isActive();
	}

	virtual const char* getContext( const char* key ) 
	{
		if( ! ( key && key[0] != 0 ))
		{
			return NULL;
		}
		std::map<std::string,std::string>::const_iterator it = _contextMap.find(key);
		if( it != _contextMap.end() )
		{
			return it->second.c_str();
		}
		else
		{
			return NULL;
		}
	}

	virtual void addRef( )
	{
		ZQ::common::MutexGuard gd(_refMutex);
		++_ref;
	}

	 /// release this request object
	virtual void release()
	{
		int iRef = 0;
		{
			ZQ::common::MutexGuard gd(_refMutex);
			iRef = --_ref;
		}
		if( iRef == 0 )
			delete this;		
	}

	/// get the process phase
	///@return the current process phase of the request
	virtual ProcessPhase getPhase()
	{
		return _phase;
	}

	/// get the request verb, available after phase ASSOC_URI
	///@return the verb string
	virtual RTSP_VerbCode getVerb()
	{
		return (RTSP_VerbCode )_rtspReq->getMethod();
	}

	/// get the request content uri, available after phase ASSOC_URI
	///@return the uri string
	virtual const char* getUri(char* uri, int len)
	{
		strcpy(uri, _rtspReq->getHost().getData());
		strcat(uri, "/");
		strcat(uri, _rtspReq->getFilePath().getData());
		return uri;
	}
	virtual const char* getStartline( char *buf , int bufLen )
	{
		if(!buf || bufLen<=0)
			return NULL;
		const char* p = _rtspReq->getStartLine().getDataBuf();
		//int iLen = (int) _rtspReq->getStartLine().getData()->length();
		strncpy(buf,p,bufLen);
		return buf;
	}

	/// get the protocol, available after phase ASSOC_URI
	///@return the protocol string
	virtual const char* getProtocol(char* protocol, int len)
	{
		strcpy(protocol, _rtspReq->getProtocol().getData());
		return protocol;
	}

	/// get a header field
	///@param key the name of the header field
	///@param value buffer to receive the value as a string
	///@param maxLen specify the max buffer size in byte that the value can receive
	///@return pointer to the value if successful
	virtual const char* getHeader(IN const char* key, OUT char* value, 
		IN OUT uint16* maxLen)
	{
		if (NULL ==value || *maxLen <= 1)
			return NULL;		

		std::string keyName(key);
		Data result = _rtspReq->getHdrBodyData(keyName, true);
		
		const char* str = result.getData();
		if ( !str || strlen(str) <= 0) {
			result = _rtspReq->getAddedHdrBodyData(keyName);
			str = result.getData();
			if ( !str || strlen(str) <= 0)
				return NULL;
		}
		
		int len = result.length() + 1;
		if (*maxLen >0 && (uint32)len > *maxLen) // adjust the len if maxLen is valid and the value exceeded the limitation
			len = *maxLen;
		
		if (len <=0) // empty result
			return NULL;

		strncpy( value , result.getData(), len );
		
		value[*maxLen-1] = '\0';

		trimString(value);
		*maxLen = strlen(value);  // need to adjust output len after trim
		
		return value;
	}

	/// get the content body of the request
	///@param buf buffer to receive the value as a string
	///@param maxLen specify the max buffer size in byte that the content buffer can receive
	///@return pointer to the content if successful
	virtual const char* getContent(unsigned char* buf, uint32* maxLen)
	{
		uint32 contLen = (uint32 )_rtspReq->getContentLength();
		contLen = *maxLen > contLen ? contLen : *maxLen-1;
		strncpy((char* )buf, _rtspReq->getMsgBody().getData(), contLen);
		buf[contLen] = 0;
		*maxLen = contLen;

		return (char *)buf;
	}

	virtual bool getTransportParam(IN const char* subkey, OUT char* value, 
		IN OUT uint16* maxLen)
	{
		// ZeroMemory((char*)value, *maxLen);
		// const Data& headers = _rtspReq->getHeaders();
		glog(Log::L_DEBUG, "getTransportParam():\t subKey = %s", subkey);
		u_int32_t method = _rtspReq->getMethod();

		switch( method ) {

		case RTSP_SETUP_MTHD:
			{
				Sptr<RtspTransportSpec> trans = 
					_rtspReq->getTransport();
				if (trans == NULL) {
					*maxLen = 0;
					*value = 0;
					glog(Log::L_DEBUG, 
						"getTransportParam():\t subKey = %s not found", 
						subkey);
					return false;
				}
				
				if (strcmp(subkey, KEY_TRANS_DEST) == 0) {
					strcpy(value, trans->myDestination.getDataBuf());
				} else if (strcmp(subkey, KEY_TRANS_CPORTA) == 0) {
					sprintf(value, "%d", trans->myClientPortA);
				} else if (strcmp(subkey, KEY_TRANS_CPORTB) == 0) {
					sprintf(value, "%d", trans->myClientPortB);
				}

				*maxLen = strlen(value);
				return true;				
				break;
			}
			break;
		}

		*maxLen = 0;
		*value = 0;
		glog(Log::L_DEBUG, "getTransportParam():\t subKey = %s not found", 
			subkey);
		
		return false;
	}

	///get the associated connection
	///@return pointer to the connection associated to this request
	IConnectionInternalPtr getConnection()
	{
		// EnterCriticalSection(&_connCritSec);
		if (!_conn)
			return NULL;

		// if connection is invalid, release the object
		if (!_conn->isActive()) {
			return NULL;
			// if (_conn->release() <= 0)
			// 	_conn = NULL;
		}

		// LeaveCriticalSection(&_connCritSec);
		return _conn;
	}

	///get the associated response
	///@return pointer to the response associated to this request
	virtual IServerResponse* getResponse()
	{
		return &_response;
	}

	///get the associated site
	///@return pointer to the site associated to this request
	virtual IStreamSmithSite* getSite()
	{
		return _site;
	}

	///get the associated client session record
	///@return pointer to the client session record associated to this request
	virtual const char* getClientSessionId()
	{
		return _rtspReq->getSessionId();
	}

	virtual bool setSite(IStreamSmithSite* site)
	{
		_site = site;
		return true;
	}

	virtual bool setPhase(ProcessPhase phase)
	{
		_phase = phase;
		return true;
	}
	
	virtual bool setClientSessionId(const char* sessionID)
	{
		return _rtspReq->setSessionId(sessionID);
	}

	virtual void setUserCtxIdx(const char* userCtxIdx)
	{
		assert(false);
	}
	
	virtual bool getRemoteInfo(IClientRequest::RemoteInfo& info)
	{
		if (info.size != sizeof(RemoteInfo))
			return false;
		IConnectionInternalPtr conn = getConnection();
		if (!conn)
			return false;
		if (!conn->isActive())
			return false;
		bool result = conn->getRemoteIP(info.ipaddr, info.addrlen, 
			&info.port) != NULL;
		conn = NULL;
		return result;
	}
	virtual bool getLocalInfo(IClientRequest::LocalInfo& info)
	{
		if (info.size != sizeof(RemoteInfo))
			return false;
		IConnectionInternalPtr conn = getConnection();
		if (!conn)
			return false;
		if (!conn->isActive())
			return false;
		bool result = conn->getLocalIP(info.ipaddr, info.addrlen, 	&info.port) != NULL;
		conn = NULL;
		return result;
	}

public:

	///set the associated site, only available at phase
	///@return pointer to the site associated to this request
	virtual bool  setArgument(RTSP_VerbCode verb, const char* uri, 
		const char* protocol)
	{
		const CharData& verbData = 
			RtspUtil::getMethodInString((u_int32_t )verb);

		std::string tempBuf;
		tempBuf = tempBuf + verbData.getPtr();
		tempBuf = tempBuf + " ";
		tempBuf = tempBuf + uri;
		tempBuf = tempBuf + " ";
		tempBuf = tempBuf + protocol;
		
		//char buf[512];
		//sprintf(buf, "%s %s %s", verbData.getPtr(), uri, protocol);
		// Data header(verbData.getPtr(), verbData.getLen());
		_rtspReq->setStartLine(tempBuf.c_str());
		return _rtspReq->parse() != RTSP_NULL_MTHD;
	}

	virtual void setHeader(const char* key, char* value)
	{
		u_int32_t code = RtspUtil::getHeaderInNumber(CharData(key));
		if (code == RTSP_UNKNOWN_HDR) {
			_rtspReq->setAddedHdrBodyData(std::string(key), Data(value));
		} else {
			_rtspReq->setHdrBodyData((RtspHeadersType )code, Data(value));
		}
		_rtspReq->getHdrBodyData(std::string(key), true).getDataBuf();	
	}

	virtual void setContent(const unsigned char* content, 
		const uint32 len)
	{
		_rtspReq->setMsgBody(Data((char* )content, len));
	}

	// read a data directly from the received request byte stream
	virtual const char* read(const uint32 offset, 
		const unsigned char* content, const uint32* plen)
	{
		// have no implementation
		assert(false);
		return NULL;
	}

	// overwrite the entire raw request, available only at phase PostRequestRead
	virtual const char* write(const unsigned char* reqcontent, 
		const uint32* plen)
	{
		// have no implementation
		assert(false);
		return NULL;
	}

	virtual int64 getRequestInitTime() const 
	{
		return _requestInitTime;
	}

protected:
	void trimString(char* str);

protected:
	RtspRequest*						_rtspReq;
	IConnectionInternalPtr              _conn;
	RtspServerResponse					_response;
	ProcessPhase						_phase;
	IStreamSmithSite*					_site;
	
	std::map<std::string,std::string>	_contextMap;
	ZQ::common::Mutex					_refMutex;
	long volatile						_ref;
	int64								_requestInitTime;
};

#ifndef _NO_NAMESPACE
	} // namespace StreamSmith {
} // namespace ZQ {
#endif // #ifndef _NO_NAMESPACE

#endif // !defined(AFX_RTSPDIALOG_H__D3FB2C3C_A761_4CD6_9C49_CFC5329601E3__INCLUDED_)
