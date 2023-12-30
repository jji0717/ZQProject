#ifndef _PROXY_INTERFACE_DEFINITION_FOR_INTERNAL_USE_HEADER_FILE_H__2007_12_17__
#define _PROXY_INTERFACE_DEFINITION_FOR_INTERNAL_USE_HEADER_FILE_H__2007_12_17__

#include "StreamSmithModule.h"
#include "Pointer.h"

class IConnectionInternal;

typedef  ZQ::common::Pointer<IConnectionInternal>  IConnectionInternalPtr;

class IClientSessionInternal : public IClientSession
{
public:
	virtual long					reference() = 0;

	virtual bool					isActive() = 0;

	virtual bool					checkTimeout(uint32 timeo) = 0;

	virtual	bool					close() = 0;
	
	virtual bool					onAccess(IConnectionInternalPtr conn) = 0;

	virtual IConnectionInternalPtr	getActiveConnection() = 0;
};

class IClientRequestWriterInternal : public IClientRequestWriter
{
public:
		virtual IConnectionInternalPtr getConnection() =0;
		virtual int64 getRequestInitTime() const = 0;
};

class ClientRequestGuard
{
public:
	ClientRequestGuard(IClientRequest* req)
		:mReq(req)
	{
		if(mReq)
		{
			mReq->addRef();
		}
	}
	~ClientRequestGuard()
	{
		if(mReq)
		{
			mReq->release();
		}
	}
private:
	ClientRequestGuard( const ClientRequestGuard& );
	ClientRequestGuard& operator=(const ClientRequestGuard&);
	IClientRequest*	mReq;
};

class IConnectionInternal : public ZQ::common::SharedObject
{
public:
	virtual ~IConnectionInternal(){};

	virtual bool operator != (const IConnectionInternal& other)
	{
		return this != &other;
	}

	virtual const char*			getRemoteIP(char* IP, const uint IPLen, uint16* port) =0;

	virtual const char*			getLocalIP(char* IP,const uint IPLen,uint16* port) =0;

	virtual uint32				recv(uint8* buf, const uint32 maxlen) =0;
	
	virtual uint32				send(const uint8* buf, const uint32 maxlen) =0;
	
	virtual bool				close()=0 ;

	virtual bool				isActive() =0;
	
	virtual void				updateLastSession(const char* sessID) =0;

	virtual	std::string			getLastSessionID() = 0;

#ifdef ZQ_OS_MSWIN
	virtual SOCKET				getSocket() = 0;
#else
	virtual int					getSocket() = 0;
#endif
	virtual	uint64	getConnectionIdentity() =0;
	
	virtual void				setPeerInfo  ( const std::string& strPeerIP , const std::string& strPeerPort ) = 0;
	
	virtual void				setLocalInfo ( const std::string& strLocalIP, const std::string& strLocalPort ) =0;

	enum
	{
		CONNECTION_PROTCOL_TYPE_RTSP,
		CONNECTION_PROTCOL_TYPE_LSCP
	};
	
	virtual void				setConnectionProtocol ( int protocolType ) = 0;

	virtual int					getConnectionProtocol ( ) = 0;
	
};

#endif//_PROXY_INTERFACE_DEFINITION_FOR_INTERNAL_USE_HEADER_FILE_H__2007_12_17__

