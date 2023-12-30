// UrlParser.h: interface for the UrlParser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_URLPARSER_H__9DD67D6E_A214_458A_B87C_EBBFE0F9ACDE__INCLUDED_)
#define AFX_URLPARSER_H__9DD67D6E_A214_458A_B87C_EBBFE0F9ACDE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class UrlParser  
{
public:
	UrlParser();
	virtual ~UrlParser();
	bool parse(LPCTSTR url);

	const std::string& getProtocol();
	const std::string& getHost();
	const std::string& getPort();
	const std::string& getPath();
#if 0	
	const std::string& getQueryItem(int index) const;
#endif
	const std::string& getQueryItem(std::string key);

protected:
	const TCHAR* parseProtocol(const TCHAR* c);
	const TCHAR* parseHost(const TCHAR* c);
	const TCHAR* parsePort(const TCHAR* c);
	const TCHAR* parsePath(const TCHAR* c);
	const TCHAR* parseQuery(const TCHAR* c);
	const TCHAR* parseQueryItem(const TCHAR* c);

protected:
	std::string	_protocol;
	std::string	_host;
	std::string _port;
	std::string _path;
	typedef std::map<std::string, std::string> QueryItem;
	QueryItem	_queryItems;

};

#endif // !defined(AFX_URLPARSER_H__9DD67D6E_A214_458A_B87C_EBBFE0F9ACDE__INCLUDED_)
