#ifndef __XML_SESSCTXHANDLER__
#define __XML_SESSCTXHANDLER__

#include "XML_Handler.h"
#include "MacroHandler.h"
#include "limits.h"

#define SESSCSEQ		"SessCSeq"
#define SESSCTXKEY		"key"
#define SESSCTXVALUE	"value"
#define SESSCTXCLS		"CLS"
#define SESSCTXTYPE		"type"

static const char *strGlobalType = "global";
static const char *strLocalType = "local";

typedef struct SessCSeq
{
	SessCSeq(DWORD uCSeq)
	{
		_uCSeq = uCSeq;
		//InitializeCriticalSection(&_CS);
	}
	~SessCSeq()
	{
		//DeleteCriticalSection(&_CS);
	}

	::std::string getCSeq()
	{
		stringstream ss;
		//EnterCriticalSection(&_CS);
		//_lock.WriteLock();
		ZQ::common::MutexGuard mutexGuard(_lock);
		DWORD i = _uCSeq;
		ss << i;
		if (_uCSeq == ULONG_MAX)
		{
			_uCSeq = 1;
		}
		else
		{
			_uCSeq++;
		}
		//LeaveCriticalSection(&_CS);
		//_lock.WriteUnlock();
		return ss.str();
	}

	DWORD ugetCSeq()
	{
		//EnterCriticalSection(&_CS);
		//_lock.WriteLock();
		ZQ::common::MutexGuard mutexGuard(_lock);
		DWORD i = _uCSeq;
		if (_uCSeq == ULONG_MAX)
		{
			_uCSeq = 1;
		}
		else
		{
			_uCSeq++;
		}
		//LeaveCriticalSection(&_CS);
		//_lock.WriteUnlock();
		return i;
	}
	DWORD _uCSeq;
	//CRITICAL_SECTION _CS;
	//::ZQ::common::RWLock _lock;
	::ZQ::common::Mutex _lock;
}SessCSeq;

typedef struct  
{
	::std::string	value;
	::std::string	type;
}MapValue;

typedef map<::std::string, MapValue> sessCtxMap;

class XML_SessCtxHandler : public XML_Handler
{
public:
	XML_SessCtxHandler();
	~XML_SessCtxHandler();

	XML_SessCtxHandler &operator=(XML_SessCtxHandler &xml_SessCtxHandler)
	{
		this->_sessCtxMap = xml_SessCtxHandler._sessCtxMap;
		this->_pp = xml_SessCtxHandler._pp;
		return *this;
	}

	void setLogger(::ZQ::common::Log *pLog){XML_Handler::setLogger(pLog);}

	void setTailorProp(std::string strTailorType, int tailorRange);

	//add or modify SessCtx key in the map
	bool	parseSessCtx(::ZQ::common::XMLPreferenceEx *xmlNode, const char* type);
	bool	parseSessCtx(::ZQ::common::XMLUtil::XmlNode xmlNode, const char* type);

	bool	parseSessCtx(::ZQ::common::XMLPreferenceEx *xmlNode, ::std::string &inKey, MapValue &inValue, const char* type);
	bool	parseSessCtx(::ZQ::common::XMLUtil::XmlNode xmlNode, ::std::string &inKey, MapValue &inValue, const char* type);

	//find a SessCtx key in the map
	MapValue	findSessCtxKey(const ::std::string &key);
	bool		removeSessCtxKey(const ::std::string &key);

	// @function : global ctx value such as INC(1) -> 1
	bool		getGlobalSessCtxKey(XML_SessCtxHandler &xml_SessCtxHandler);

	// remove global SessCtx
	bool		removeGlobalSessCtxKey(XML_SessCtxHandler &xml_SessCtxHandler);

	// add or update ctx value
	bool updateMacro(const ::std::string &strKey, const ::std::string &strValue);

    //@function : ${URL} -> "rtsp://./60010000" and INC(1)->1
	bool fixupMacro(::std::string &str);

	// 
	bool modifyGlobalMacro();

public:
	sessCtxMap _sessCtxMap;
	bool	_status;

private:
	::ZQ::common::Preprocessor _pp;
	MacroHandler			_macroHandler;
	//::ZQ::common::RWLock	_lock;
	ZQ::common::Mutex _lock;
};

typedef ::std::map<::std::string, XML_SessCtxHandler*> XML_SessCtxHandlerMap;

#endif __XML_SESSCTXHANDLER__