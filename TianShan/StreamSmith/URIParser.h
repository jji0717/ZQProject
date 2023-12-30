// URIParser.h: interface for the URIParser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_URIPARSER_H__FF703C80_C7F8_4FD5_A1D0_193F95EC97C3__INCLUDED_)
#define AFX_URIPARSER_H__FF703C80_C7F8_4FD5_A1D0_193F95EC97C3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#pragma warning(disable:4786)
#include <string>
#include <vector>
namespace ZQ{
	namespace StreamSmith {
		


class URIParser 
{
public:
	URIParser();
	URIParser(const char* uriStr);
	virtual ~URIParser();	
public:
	bool			HaskeyValue(){return m_KeyValue.size()>0;}
	bool			IsURIComplete();
	bool			parse(const char* uristr);
	const char*		FillUri(const char* site,const char* application=NULL,const char* mediacontent=NULL);
	const char*		FillUri();
	//////////////////////////////////////////////////////////////////////////
	void			SetDefaultSite(const char* site){m_strDefaultSite=site;}
	void			SetDefaultApplication(const char* application){m_strDefaultApplication=application;}
	void			SetDefaultMediaContent(const char* medaicontent){m_strDefaultMediaContent=medaicontent;}
	//////////////////////////////////////////////////////////////////////////
	const char*		GetProtocol();
	const char*		GetSite();
	const char*		GetPath();
	const char*		GetContent();
	const char*		GetValue(const std::string& Key);
	
	void			SetContent(const std::string& strContent);
	void			SetPath(const std::string& strPath);
	void			SetValue(const std::string& strKey,const std::string& strValue);
protected:	
private:
	void			emptyString(std::string& str);
	void			eatWhite(std::string& str);
private:
	std::string			m_strDefaultProtocol;
	std::string			m_strDefaultSite;
	std::string			m_strDefaultApplication;
	std::string			m_strDefaultMediaContent;

	std::string			m_strProtocol;
	std::string			m_strSite;	
	std::string			m_strPath;
	std::string			m_strContent;
	
	std::string			m_strOutput;
	typedef std::vector<std::pair<std::string,std::string> >	VecKeyValue;
	VecKeyValue			m_KeyValue;
	
};
}
}
#endif // !defined(AFX_URIPARSER_H__FF703C80_C7F8_4FD5_A1D0_193F95EC97C3__INCLUDED_)
