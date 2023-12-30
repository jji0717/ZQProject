// RtspSession.h: interface for the RtspSession class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RTSPSESSION_H__9208F068_04C3_4779_91C3_48D592AE2225__INCLUDED_)
#define AFX_RTSPSESSION_H__9208F068_04C3_4779_91C3_48D592AE2225__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <map>
#include <string>

#include "StreamSmithModule.h"
#include "RtspSessionMgr.h"
//#include "rwlock.h"
#include "DataPostHouseService.h"
#include <Locks.h>

#include "proxydefinition.h"

typedef ZQ::common::Variant	ZQVariant;
#define MAX_ATTRNAME	60

#ifdef _DEBUG
#define		CHECK_THIS() do {assert(this != NULL);assert(!_deleted); }while(0)
#else
#define		CHECK_THIS()
#endif

#ifndef _NO_NAMESPACE
namespace ZQ {
	namespace StreamSmith {
#endif // #ifndef _NO_NAMESPACE

using namespace common;

class RtspSession : public IClientSessionInternal, public ZQ::common::Mutex
{
	friend class RtspSessionMgr;
	friend class SessionClearThread;
public:
	RtspSession(RtspSessionMgr* mgr, const char* sessionID, SessType type = LocalSession);
	virtual ~RtspSession();

	///query the type of the client session
	///@return type of the session
	virtual SessType getType();

	///retrieve the Session ID
	///@return id of the session
	virtual const char* getSessionID();

	//user context methods
	///get the access to the pre-set user's per-session context
	///@return pointer to the user context
	virtual void* getUserContext();

	///set the user's per-session context
	///@param pContext the pointer to user's per-session context in plugin space,\n
	///                NULL is used to reset the association in session in the server core space
	///@return pointer to the user context
	virtual void* setUserContext(void* pContext);
	
	///get a session attribute
	///@param attributeName the attribute name about to query, ASCII NULL-terminate string allowed
	///@return the value of the attribute, Variant::Nil if the attribute doesn't exist
	virtual const ZQVariant get(const char* attributeName);

	//set a session attribute
	///@param attributeName the attribute name about to set, ASCII NULL-terminate string allowed
	///@param value the new value of the attribute about to set, Variant::Nil and overwrite=ture will remove the attribute
	///@param overwrite true if need to overwrite the exist one, otherwise will return false if the same attribute has existed
	///@return true if the attibute has been successfully updated
	virtual bool set(const char* attributeName, const ZQVariant& value, bool overwrite = true);
	
	bool isActive()
	{
		return _active;
	}

	bool close()
	{
		_active = false;
		return true;
	}

	virtual std::string getProp( const std::string& key ) const;

	virtual  bool		setProp( const std::string& key, const std::string& value );
public:
	//////////////////////////////////////////////////////////////////////////
	// for support retrieving a active connection of a session
	bool							onAccess(IConnectionInternalPtr conn);
	IConnectionInternalPtr			getActiveConnection();
//	RtspConnection*					getConnection(const std::string& strConnIdent);
	long							reference();
	long							release();
//	void							connectionDownNotify(const std::string& connIdent);
//	void							sessionDown();

protected:
	void							updateTimestamp();
	bool							checkTimeout(uint32 timeo);

	RtspSessionMgr* _sessionMgr;
	typedef std::map<std::string, ZQVariant> AttrMap;
	
	typedef std::map<std::string, std::string> PropMap;
	PropMap			mProps;

	void*			_userContext;
	AttrMap			_attrMap;
	const SessType	_type;
	std::string		_sessionID;

	IConnectionInternalPtr	_activeConn;
	ZQ::common::Mutex		_refLock;	
	volatile long			_ref;
	bool					_active;
	int64					_timestamp;

//	typedef std::map<unsigned __int64,RtspConnection*> CONNSESSMAP;
//	CONNSESSMAP		_connSessMap;

#ifdef _DEBUG
	bool			_deleted;
#endif

};

#ifndef _NO_NAMESPACE
	} // namespace StreamSmith {
} // namespace ZQ {
#endif // #ifndef _NO_NAMESPACE

#endif // !defined(AFX_RTSPSESSION_H__9208F068_04C3_4779_91C3_48D592AE2225__INCLUDED_)
