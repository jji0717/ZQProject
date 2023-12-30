// UrlParser.cpp: implementation of the UrlParser class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UrlParser.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UrlParser::UrlParser()
{

}

UrlParser::~UrlParser()
{

}

bool UrlParser::parse(LPCTSTR url)
{
	assert(url);
	_protocol.clear();
	_host.clear();
	_port.clear();
	_path.clear();
	_queryItems.clear();
	const TCHAR* c = url;
	c = parseProtocol(c);
	if (c == NULL)
		return false;
	c = parseHost(c);
	if (c == NULL)
		return false;
	c = parsePort(c);
	if (c == NULL)
		return false;
	c = parsePath(c);
	if (c == NULL)
		return false;
	c = parseQuery(c);
	if (c == NULL)
		return false;

	return true;
}

const std::string& UrlParser::getProtocol()
{
	return _protocol;
}

const std::string& UrlParser::getHost()
{
	return _host;
}

const std::string& UrlParser::getPort()
{
	return _port;
}

const std::string& UrlParser::getPath()
{
	return _path;
}

#if 0
const std::string& UrlParser::getQueryItem(int index) const
{
	static std::string MissValue;
	if (index >= (int )_queryItems.size())
		return MissValue;

	QueryItem::const_iterator itor = _queryItems.begin();
	if (index)
		itor += index;
	return itor->second;
}
#endif

const std::string& UrlParser::getQueryItem(std::string key)
{
	return _queryItems[key];
}

const TCHAR* UrlParser::parseProtocol(const TCHAR* c)
{
	if (c == NULL || *c == 0)
		return NULL;

	const TCHAR* p = c;
	
	while(*p) {
		if (_tcsncmp(p, _T("://"), 3) == 0) {
			_protocol = std::string(c, p - c);
			return p;
		}
		p ++;
	}

	return c;
}

const TCHAR* UrlParser::parseHost(const TCHAR* c)
{
	if (c == NULL || *c == 0)
		return NULL;


	if (_tcsncmp(c, _T("://"), 3) != 0) {
		return c;
	}

	const TCHAR* p = c += 3;
	while(*p) {
		if (*p == _T(':') || *p == _T('/')) {
			_host = std::string(c, p - c);
			return p;
		}
		p ++;
	}

	_host = std::string(c, p - c);
	return p;
}

const TCHAR* UrlParser::parsePort(const TCHAR* c)
{
	if (c == NULL || *c == 0)
		return NULL;

	if (*c != _T(':'))
		return c;

	const TCHAR* p = c += 1;
	while(*p) {
		if (*p == _T('/')) {
			_port = std::string(c, p - c);
			return p;
		}
		p ++;
	}

	_port = std::string(c, p - c);
	return p;
}

const TCHAR* UrlParser::parsePath(const TCHAR* c)
{
	if (c == NULL || *c == 0)
		return NULL;

	if (*c != _T('/'))
		return c;

	const TCHAR* p = c += 1;
	while(*p) {
		if (*p == _T('?')) {
			_path = std::string(c, p - c);
			return p;
		}
		p ++;
	}

	_path = std::string(c, p - c);
	return p;
}

const TCHAR* UrlParser::parseQuery(const TCHAR* c)
{
	if (c == NULL || *c == 0)
		return NULL;

	const TCHAR* p = c;
	while(*p) {
		p = parseQueryItem(p);
	}

	return p;
}

const TCHAR* UrlParser::parseQueryItem(const TCHAR* c)
{
	if (c == NULL || *c == 0)
		return NULL;

	if (*c != _T('?') && *c != _T('&'))
		return c;

	const TCHAR* p = c += 1;
	std::string name, value;
	while(*p) {
		if (*p == _T('=')) {
			name = std::string(c, p - c);
			break;
		}
		p ++;
	}

	if (*p == 0)
		return p;

	bool valueSet = false;
	p ++;
	const TCHAR* s = p;
	while(*p) {
		if (*p == _T('&')) {
			value = std::string(s, p - s);
			valueSet = true;
			break;
		}
		
		p ++;
	}

	if (!valueSet) {
		value = std::string(s, p - s);
	}

	_queryItems.insert(QueryItem::value_type(name, value));

	return p;	
}
