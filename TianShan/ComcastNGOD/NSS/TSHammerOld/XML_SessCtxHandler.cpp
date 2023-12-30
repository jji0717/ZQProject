#include "XML_SessCtxHandler.h"

XML_SessCtxHandler::XML_SessCtxHandler()
{
	_pp.define(SESSCTXCLS, string("\r\n"));
	//_pp.define(SESSCSEQ, string("1"));
	//_sessCtxMap[SESSCSEQ] = string("1");
	_handle = CreateEvent(NULL, true, true, NULL);
	_status = false;

	//InitializeCriticalSection(&_CS);
}

XML_SessCtxHandler::~XML_SessCtxHandler()
{
	_sessCtxMap.clear();
	CloseHandle(_handle);
	//DeleteCriticalSection(&_CS);
}

bool XML_SessCtxHandler::parseSessCtx(::ZQ::common::XMLUtil::XmlNode xmlNode, const char* type)
{
	if (NULL == xmlNode)
		return false;

	string strKey;
	MapValue strValue;

	if (parseSessCtx(xmlNode, strKey, strValue, type))
	{
		MapValue findValue = findSessCtxKey(strKey);
		if (findValue.value.length())
			XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_SessCtxHandler,"find <SessCtx> key(%s):value(%s) and modify value to %s"), strKey.c_str(), findValue.value.c_str(), strValue.value.c_str());
		_sessCtxMap[strKey] = strValue;

		//define the macro
		updateMacro(strKey, strValue.value);

		return true;
	}
	else
		return false;
}

extern uint16 g_uCSeq;

bool XML_SessCtxHandler::parseSessCtx(::ZQ::common::XMLPreferenceEx *xmlNode, const char* type)
{
	if (NULL == xmlNode)
		return false;

	string strKey;
	MapValue strValue;

	if (parseSessCtx(xmlNode, strKey, strValue, type))
	{
		MapValue findValue = findSessCtxKey(strKey);
		if (findValue.value.length())
			XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_SessCtxHandler,"find <SessCtx> key(%s):value(%s) and modify value to %s"), strKey.c_str(), findValue.value.c_str(), strValue.value.c_str());
		_sessCtxMap[strKey] = strValue;

		//define the macro
		updateMacro(strKey, strValue.value);

		return true;
	}
	else
		return false;
}

bool XML_SessCtxHandler::parseSessCtx(::ZQ::common::XMLPreferenceEx *xmlNode, ::std::string &inKey, MapValue &inValue, const char* type)
{
	if (NULL == xmlNode)
		return false;

	//get node attribute
	char ctype[iKeyLen];
	char key[iKeyLen];
	char value[iValueLen];

	if (xmlNode->getAttributeValue(SESSCTXTYPE, ctype, iKeyLen))
	{
		if (strcmp(type, ctype) != 0)
			return false;
		else
			inValue.type = ctype;
	}

	if (xmlNode->getAttributeValue(SESSCTXKEY, key, iKeyLen))
	{
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_SessCtxHandler,"get <SessCtx> attribute %s=\"%s\""),SESSCTXKEY, key);
		inKey = key;
	}
	else
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_SessCtxHandler,"fail get <SessCtx> attribute %s"),SESSCTXKEY);

	if (xmlNode->getAttributeValue(SESSCTXVALUE, value, iValueLen))
	{
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_SessCtxHandler,"get <SessCtx> attribute %s=\"%s\""),SESSCTXVALUE, value);
		inValue.value = value;
	}
	else
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_SessCtxHandler,"fail get <SessCtx> attribute %s"),SESSCTXVALUE);

	if (inKey.length() && inValue.type.length() && inValue.value.length())
	{
		return true;
	}
	else
		return false;

}

bool XML_SessCtxHandler::parseSessCtx(::ZQ::common::XMLUtil::XmlNode xmlNode, ::std::string &inKey, MapValue &inValue, const char* type)
{
	if (NULL == xmlNode)
		return false;

	//get node attribute
	char ctype[iKeyLen];
	char key[iKeyLen];
	char value[iValueLen];

	if (xmlNode->getAttributeValue(SESSCTXTYPE, ctype, iKeyLen))
	{
		if (strcmp(type, ctype) != 0)
			return false;
		else
			inValue.type = ctype;
	}

	if (xmlNode->getAttributeValue(SESSCTXKEY, key, iKeyLen))
	{
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_SessCtxHandler,"get <SessCtx> attribute %s=\"%s\""),SESSCTXKEY, key);
		inKey = key;
	}
	else
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_SessCtxHandler,"fail get <SessCtx> attribute %s"),SESSCTXKEY);

	if (xmlNode->getAttributeValue(SESSCTXVALUE, value, iValueLen))
	{
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_SessCtxHandler,"get <SessCtx> attribute %s=\"%s\""),SESSCTXVALUE, value);
		inValue.value = value;
	}
	else
		XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_SessCtxHandler,"fail get <SessCtx> attribute %s"),SESSCTXVALUE);

	if (inKey.length() && inValue.type.length() && inValue.value.length())
	{
		return true;
	}
	else
		return false;
}

MapValue XML_SessCtxHandler::findSessCtxKey(const ::std::string &key)
{
	MapValue ret;
	ret.type ="";
	ret.value = "";
	sessCtxMap::iterator iter = _sessCtxMap.find(key);
	if (iter == _sessCtxMap.end())
		return ret;
	else
		return (*iter).second;
}

bool XML_SessCtxHandler::removeSessCtxKey(const ::std::string &key)
{
	sessCtxMap::iterator iter = _sessCtxMap.find(key);
	if (iter == _sessCtxMap.end())
		return false;
	else
	{
		_sessCtxMap.erase(iter);
		return true;
	}
}

bool XML_SessCtxHandler::getGlobalSessCtxKey(XML_SessCtxHandler &xml_SessCtxHandler)
{
	for (sessCtxMap::iterator iter = xml_SessCtxHandler._sessCtxMap.begin(); iter != xml_SessCtxHandler._sessCtxMap.end(); iter++)
	{
		if ((*iter).second.type.compare(strGlobalType) == 0)
		{
			::std::string strValue = (*iter).second.value;
			_macroHandler.fixupMacro(strValue);
			updateMacro((*iter).first, strValue);
		}
	}
	return true;
}

bool XML_SessCtxHandler::removeGlobalSessCtxKey(XML_SessCtxHandler &xml_SessCtxHandler)
{
	for (sessCtxMap::iterator iter = xml_SessCtxHandler._sessCtxMap.begin(); iter != xml_SessCtxHandler._sessCtxMap.end(); iter++)
	{
		if ((*iter).second.type.compare(strGlobalType) == 0)
			removeSessCtxKey((*iter).first);
	}
	return true;
}

bool XML_SessCtxHandler::updateMacro(const ::std::string &strKey, const ::std::string &strValue)
{
	_sessCtxMap[strKey].value = strValue;
	return _pp.define(strKey, strValue);
}

bool XML_SessCtxHandler::fixupMacro(::std::string &str)
{
	_lock.ReadLock();
	bool b = _pp.fixup(str);
	if (b)
		b = _macroHandler.fixupMacro(str);
	else
		b = false;
	_lock.ReadUnlock();
	return b;
}

bool XML_SessCtxHandler::modifyGlobalMacro()
{
	//EnterCriticalSection(&_CS);
	_lock.WriteLock();
	for (sessCtxMap::iterator iter = _sessCtxMap.begin(); iter != _sessCtxMap.end(); iter++)
	{
		MapValue value = (*iter).second;
		if (value.type.compare(strGlobalType) == 0)
		{
			_macroHandler.fixupMacro(value.value);
			_pp.define((*iter).first, value.value);
			_macroHandler.updateMacro((*iter).second.value);
		}
	}
	//LeaveCriticalSection(&_CS);
	_lock.WriteUnlock();
	return true;
}