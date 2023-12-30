#ifndef __XML_SESSCTXHANDLER__
#define __XML_SESSCTXHANDLER__

#include "XML_Handler.h"
#include "MacroHandler.h"

#define SESSCSEQ		"SessCSeq"
#define SESSCTXKEY		"key"
#define SESSCTXVALUE	"value"
#define SESSCTXCLS		"CLS"
#define SESSCTXTYPE		"type"

static const char *strGlobalType = "global";
static const char *strLocalType = "local";

typedef struct SessCSeq
{
	SessCSeq(uint16 uCSeq)
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
		_lock.WriteLock();
		uint16 i = _uCSeq++;
		if (i == 0)
			i = _uCSeq++;
		ss << i;
		//LeaveCriticalSection(&_CS);
		_lock.WriteUnlock();
		return ss.str();
	}

	uint16 ugetCSeq()
	{
		//EnterCriticalSection(&_CS);
		_lock.WriteLock();
		return _uCSeq++;
		//LeaveCriticalSection(&_CS);
		_lock.WriteUnlock();
	}
	uint16 _uCSeq;
	//CRITICAL_SECTION _CS;
	::ZQ::common::RWLock _lock;
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

	//add or modify SessCtx key in the map
	bool	parseSessCtx(::ZQ::common::XMLPreferenceEx *xmlNode, const char* type);
	bool	parseSessCtx(::ZQ::common::XMLUtil::XmlNode xmlNode, const char* type);

	bool	parseSessCtx(::ZQ::common::XMLPreferenceEx *xmlNode, ::std::string &inKey, MapValue &inValue, const char* type);
	bool	parseSessCtx(::ZQ::common::XMLUtil::XmlNode xmlNode, ::std::string &inKey, MapValue &inValue, const char* type);

	//find a SessCtx key in the map
	MapValue	findSessCtxKey(const ::std::string &key);
	bool		removeSessCtxKey(const ::std::string &key);

	bool		getGlobalSessCtxKey(XML_SessCtxHandler &xml_SessCtxHandler);
	bool		removeGlobalSessCtxKey(XML_SessCtxHandler &xml_SessCtxHandler);

	sessCtxMap _sessCtxMap;

	bool updateMacro(const ::std::string &strKey, const ::std::string &strValue);

	bool fixupMacro(::std::string &str);

	bool modifyGlobalMacro();

	HANDLE	_handle;
	bool	_status;

private:
	::ZQ::common::Preprocessor _pp;
	MacroHandler			_macroHandler;
	//CRITICAL_SECTION		_CS;
	::ZQ::common::RWLock	_lock;
};

typedef ::std::map<::std::string, XML_SessCtxHandler*> XML_SessCtxHandlerMap;

#endif __XML_SESSCTXHANDLER__