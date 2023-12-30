// URIParser.cpp: implementation of the URIParser class.
//
//////////////////////////////////////////////////////////////////////


#include "URIParser.h"
#include <log.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
using namespace ZQ::common;
namespace ZQ{
	namespace StreamSmith {
		

URIParser::URIParser()
{
	m_strDefaultProtocol="RTSP";
	m_strPath="";
	m_strProtocol="";
	m_strSite="";
	m_strContent="";
}
URIParser::URIParser(const char* uriStr)
{
	m_strDefaultProtocol="RTSP";
	parse(uriStr);
}
URIParser::~URIParser()
{

}
void URIParser::emptyString(std::string& str)
{
	str.erase(str.begin(),str.end());
}

#define	POSOK(x) (x!=std::string::npos)
bool URIParser::parse(const char* uristr)
{
	if(!uristr)
	{
		glog(Log::L_ERROR,"bool URIParser::parse()## null ruistr pass in");
		return false;
	}
	m_KeyValue.clear();
	std::string		RqURI=uristr;
	
	int sToken,sToken2,pToken,cToken;
	sToken=RqURI.find("://");
	if(!POSOK(sToken))
	{
		sToken=RqURI.find("//");
		sToken2=sToken+2;
		
	}
	else
	{
		sToken2=sToken+3;
		
	}
	
	if(POSOK(sToken))
	{
		pToken=RqURI.find("/",sToken2);
	}
	else
	{
		pToken=RqURI.find("/");
	}
	cToken=RqURI.find("?");

	if(POSOK(sToken))
	{
		//protocol
		m_strProtocol=RqURI.substr(0,sToken);
		if(POSOK(pToken))
			m_strSite=RqURI.substr(sToken2,pToken-sToken2);
		else
			m_strSite=RqURI.substr(sToken2);
	}
	else
	{
		emptyString(m_strProtocol);
		if(POSOK(pToken))
			m_strSite=RqURI.substr(0,pToken);
		else
			m_strSite=RqURI;
	}
	if(POSOK(pToken))
	{
		if(POSOK(cToken))
		{
			m_strPath=RqURI.substr(pToken+1,cToken-pToken-1);
			m_strContent=RqURI.substr(cToken+1);
		}
		else
		{
			m_strPath=RqURI.substr(pToken+1);
			emptyString(m_strContent);
		}
	}
	else
	{
		emptyString(m_strPath);
		emptyString(m_strContent);
		return false;
	}	
	eatWhite(m_strContent);
	eatWhite(m_strProtocol);
	eatWhite(m_strSite);
	eatWhite(m_strPath);

	int iTempPos=m_strSite.find(":");
	if(POSOK(iTempPos))
	{
		m_strSite=m_strSite.substr(0,iTempPos);
	}

	//Parse key value
	bool bOK=true;
	std::string	strTemp=m_strContent;
	std::string	strKey;
	std::string	strValue;

	while (bOK)
	{
		int iPos=strTemp.find_first_of('=');
		if(iPos==std::string::npos)
		{
			bOK=false;
			break;
		}
		strKey=strTemp.substr(0,iPos);
		strTemp=strTemp.substr(iPos+1);
		iPos=strTemp.find_first_of('&');
		if(iPos==std::string::npos)
		{
			strValue=strTemp;
			bOK=false;
		}
		else
		{
			strValue=strTemp.substr(0,iPos);
			strTemp=strTemp.substr(iPos+1);
		}
		std::pair<std::string,std::string> p(strKey,strValue);
		m_KeyValue.push_back(p);
	}
	return true;
}
void URIParser::eatWhite(std::string& str)
{
	int iPos=str.find_first_not_of(' ');
	if(!POSOK(iPos))
	{
		str="";
		return;
	}	
	str=str.substr(iPos);

	iPos=str.find_last_not_of(' ');
	if(!POSOK(iPos))
	{
		str="";
		return;
	}
	str=str.substr(0,iPos+1);
	if(str.size()>1 && str.at(str.size()-1)=='/')
		str=str.substr(0,str.size()-1);
}

#undef POSOK
const char* URIParser::GetValue(const std::string& Key)
{
	VecKeyValue::iterator	it;
	for(it=m_KeyValue.begin();it!=m_KeyValue.end();it++)
	{
		if(stricmp(it->first.c_str(),Key.c_str())==0)
		{
			return it->second.c_str();
		}
	}
	return NULL;
}
bool URIParser::IsURIComplete()
{
	return (!m_strContent.empty() && !m_strPath.empty() && !m_strProtocol.empty() && !m_strSite.empty());
}
const char*	URIParser::GetProtocol()
{
	return m_strProtocol.c_str();
}
const char*		URIParser::GetSite()
{
	return m_strSite.c_str();
}
const char*	 URIParser::GetPath()
{
	return m_strPath.c_str();
}
const char*	 URIParser::GetContent()
{
	return m_strContent.c_str();
}
const char*	 URIParser::FillUri(const char* site,const char* application/* =NULL */,const char* mediacontent/* =NULL */)
{
	emptyString(m_strOutput);
	if (m_strProtocol.empty()) 
	{
		m_strOutput=m_strDefaultProtocol;
	}
	else
	{
		m_strOutput=m_strProtocol;
	}

	m_strOutput+="://";

	if(m_strSite.empty())
	{
		if(!application)
			m_strOutput+=m_strDefaultSite;
		else
			m_strOutput+=site;
	}
	else
	{
		m_strOutput+=m_strSite;
	}


	m_strOutput+="/";
	
	if(m_strPath.empty())
	{
		if(!application)
			m_strOutput+=m_strDefaultApplication;
		else
			m_strOutput+=application;
	}
	else
	{
		m_strOutput+=m_strPath;
	}

	m_strOutput+="?";

	if(m_strContent.empty())
	{
		if(!mediacontent)
			m_strOutput+=m_strDefaultMediaContent;
		else
			m_strOutput+=mediacontent;
	}
	else
	{
		m_strOutput+=m_strContent;
	}
	parse(m_strOutput.c_str());
	return m_strOutput.c_str();
}
const char*	 URIParser::FillUri()
{
	return FillUri(m_strDefaultSite.c_str(),m_strDefaultApplication.c_str(),m_strDefaultMediaContent.c_str());
}
void URIParser::SetContent(const std::string& strContent)
{
	m_strContent=strContent;
}
void URIParser::SetPath(const std::string& strPath)
{
	m_strPath=strPath;
}
void URIParser::SetValue(const std::string& strKey , const std::string& strValue)
{
	if(m_strContent.find('=')!=std::string::npos)
		m_strContent+="&";
	m_strContent+=strKey;
	m_strContent+="=";
	m_strContent+=strValue;
}
}
}