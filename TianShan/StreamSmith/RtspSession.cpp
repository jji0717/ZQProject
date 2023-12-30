// RtspSession.cpp: implementation of the RtspSession class.
//
//////////////////////////////////////////////////////////////////////

#pragma warning(disable:4786)

#include "global.h"
#include "RtspSession.h"
#include "RtspDialog.h"

#ifndef _NO_NAMESPACE
namespace ZQ {
	namespace StreamSmith {
#endif // #ifndef _NO_NAMESPACE

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

RtspSession::RtspSession(RtspSessionMgr* mgr, const char* sessionID, 
						 SessType type /* = LocalSession */) :
	_sessionMgr(mgr), _type(type)
{
		
#ifdef _DEBUG
	_deleted = false;
#endif
	_sessionID = sessionID!=NULL?sessionID:"";
	_activeConn = NULL;
	_ref = 0;
	_active = true;
	_timestamp = ZQ::common::now();
}

RtspSession::~RtspSession()
{
	// when it was locked, you can't delete it.
	ZQ::common::MutexGuard guard(*this);

	if (_activeConn)
		_activeConn = NULL;
	
#ifdef _DEBUG
	CHECK_THIS();
	_attrMap.clear();
	_activeConn = NULL;
	_sessionID.erase();
	_userContext = NULL;
	_deleted = true;
#endif
}

///query the type of the client session
///@return type of the session
/* virtual */  inline RtspSession::SessType RtspSession::getType()
{
	CHECK_THIS();
	ZQ::common::MutexGuard guard( *this );
	return _type;
}

///retrieve the Session ID
///@return id of the session
/* virtual */ const char* RtspSession::getSessionID()
{
	CHECK_THIS();
	ZQ::common::MutexGuard guard(*this /*, RWLockGuard::LOCK_READ*/);
	if (!_active)
		return NULL;

	return _sessionID.c_str();
}

//user context methods
///get the access to the pre-set user's per-session context
///@return pointer to the user context
/* virtual */ inline void* RtspSession::getUserContext()
{
	CHECK_THIS();
	//ZQ::common::MutexGuard guard(/**this, RWLockGuard::LOCK_READ*/);
	if (!_active)
		return NULL;

	return _userContext;
}

///set the user's per-session context
///@param pContext the pointer to user's per-session context in plugin space,\n
///                NULL is used to reset the association in session in the server core space
///@return pointer to the user context
/* virtual */ inline void* RtspSession::setUserContext(void* pContext)
{
	CHECK_THIS();
	ZQ::common::MutexGuard guard(*this);
	if (!_active)
		return NULL;

	return _userContext = pContext;
}


std::string RtspSession::getProp( const std::string& key ) const {
	ZQ::common::MutexGuard guard(*this/*, RWLockGuard::LOCK_READ*/);
	PropMap::const_iterator it = mProps.find(key);
	if(it != mProps.end())
		return it->second;
	return std::string("");
}


bool RtspSession::setProp( const std::string& key, const std::string& value ) {
	ZQ::common::MutexGuard guard(*this/*, RWLockGuard::LOCK_READ*/);
	mProps[key] = value;
	return true;
}

///get a session attribute
///@param attributeName the attribute name about to query, ASCII NULL-terminate string allowed
///@return the value of the attribute, Variant::Nil if the attribute doesn't exist
/* virtual */ const ZQVariant RtspSession::get(const char* attributeName)
{
	CHECK_THIS();
	ZQ::common::MutexGuard guard(*this/*, RWLockGuard::LOCK_READ*/);
	if (!_active)
		return ZQVariant();

	AttrMap::iterator itor = _attrMap.find( attributeName );
	if (itor == _attrMap.end())
		return ZQ::common::Variant();
	return itor->second;
}

//set a session attribute
///@param attributeName the attribute name about to set, ASCII NULL-terminate string allowed
///@param value the new value of the attribute about to set, Variant::Nil and overwrite=ture will remove the attribute
///@param overwrite true if need to overwrite the exist one, otherwise will return false if the same attribute has existed
///@return true if the attibute has been successfully updated
/* virtual */  bool RtspSession::set(const char* attributeName, 
						const ZQVariant& value, 
						bool overwrite /* = true */)
{
	CHECK_THIS();
	ZQ::common::MutexGuard guard(*this);
	if (!_active)
		return false;

	AttrMap::iterator itor = _attrMap.find( attributeName );
	if (itor == _attrMap.end()) {
		typedef std::pair<std::string, ZQVariant> AttrPair;
		_attrMap.insert(AttrPair(attributeName, value));
		return true;
	}

	if (!overwrite)
		return false;
	
	itor->second = value;
	return true;
}

bool RtspSession::onAccess(IConnectionInternalPtr conn)
{
	CHECK_THIS();
	ZQ::common::MutexGuard guard(*this);
	if (!_active)
		return false;

	if (conn == NULL) 
	{
		assert(false);
		return false;
	}

	if (!conn->isActive()) 
	{//connection destroy 
		if (_activeConn == conn) 
		{
			_activeConn = NULL;
		}
		return true;
	}

	if (_activeConn != conn)
	{		
		if (_activeConn)
			_activeConn = NULL;
		_activeConn = conn;
	}	

	updateTimestamp();
	return true;
}

//RtspConnection*	RtspSession::getConnection(const std::string& strConnIdent)
//{
//	return NULL;
//}

IConnectionInternalPtr RtspSession::getActiveConnection()
{
	CHECK_THIS();
	ZQ::common::MutexGuard guard(*this);
	if (!_active)
	{
		glog(ZQ::common::Log::L_INFO,"RtspSession::getActiveConnection() current session is not active, return NULL");
		return NULL;
	}
	if (_activeConn) 
	{
		if (!_activeConn->isActive()) 
		{
			glog(ZQ::common::Log::L_INFO,
				"RtspSession::getActiveConnection() Connection [%llu] is not active in session [%s]",
				_activeConn->getConnectionIdentity(),_sessionID.c_str()	);
			_activeConn = NULL;
		}
		else
		{
			glog(ZQ::common::Log::L_DEBUG,
				"RtspSession::getActiveConnection() get active connection [%llu] in session [%s]",
				_activeConn->getConnectionIdentity() , _sessionID.c_str() );
		}
	}	
	return _activeConn;
}
//void	RtspSession::connectionDownNotify(const std::string& connIdent)
//{
//}

//void	RtspSession::sessionDown()
//{
//}


long RtspSession::reference()
{
	long re = 0;
	{
		ZQ::common::MutexGuard gd(_refLock);
		re = ++_ref;
	}
	DEBUG_DETAIL(DEBUG_DETAIL_LEVEL1)
	{
		glog(Log::L_DEBUG, "RtspSession::reference():\t"
			"_ref = %d, this = %s", re, getSessionID());
	}

	return re;
}

long RtspSession::release()
{
	long r = 0;
	{
		ZQ::common::MutexGuard gd(_refLock);
		r = --_ref;
	}
	assert(r >= 0);
	DEBUG_DETAIL(DEBUG_DETAIL_LEVEL1) {
		glog(Log::L_DEBUG, "RtspSession::release():\t "
			"release this(%s), _ref = %d", _sessionID.c_str(), r);
	}
	//release all conn associated with current session
	if (r == 0)
	{
		{
			ZQ::common::MutexGuard guard(*this);
			glog(Log::L_DEBUG, "RtspSession::release():\t Session %p with sessionID = %s is deleted", 
				this, _sessionID.c_str());
			// _sessionMgr->_eraseSession(_sessionID);
		}
		delete this;
	}
	return r;
}

void RtspSession::updateTimestamp()
{
	_timestamp = ZQ::common::now();
}

bool RtspSession::checkTimeout(uint32 timeo)
{
	ZQ::common::MutexGuard guard(*this);

	// 超时, 并且没有可用的连接时返回 true
	if ( ZQ::common::now() - _timestamp < timeo)
		return false;
	if (!_activeConn) {
		DEBUG_DETAIL(DEBUG_DETAIL_LEVEL2) {
			glog(Log::L_DEBUG, "RtspSession::checkTimeout():\t "
				"RtspSession::checkTimeout this(%p), _activeConn == NULL, "
				"sessionID = %s", this, getSessionID());
		}

		return true;
	}
	
	if (!_activeConn->isActive()) {
		DEBUG_DETAIL(DEBUG_DETAIL_LEVEL2) {
			glog(Log::L_DEBUG, "RtspSession::checkTimeout():\t "
				"RtspSession::checkTimeout this(%p), _activeConn(%p)"
				"_activeConn->isActive() == false, sessionID = %s",	
				this, _activeConn.get(), getSessionID());
		}

		return true;
	}
	
	return false;
}

#ifndef _NO_NAMESPACE
	} // namespace StreamSmith {
} // namespace ZQ {
#endif // #ifndef _NO_NAMESPACE
