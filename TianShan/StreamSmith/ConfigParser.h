// ConfigParser.h: interface for the CConfigParser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONFIGPARSER_H__7F17A779_0326_499C_BB34_9114DD676B94__INCLUDED_)
#define AFX_CONFIGPARSER_H__7F17A779_0326_499C_BB34_9114DD676B94__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#pragma warning(disable:4503)
#include <string>
#include <vector>
#include <map>
//#include "IPreference.h"
#include "XMLPreferenceEx.h"

using namespace ZQ::common;

#define		BUF_SIZE	2048

namespace ZQ{
namespace StreamSmith {

typedef struct _tagQamInfo
{	
	long		_basePort;
	long		_frequency;
	long		_baseProgramNumber;
	long		_programCount;
	long		_bandwidth;
}QamInfo;

typedef std::vector<QamInfo>		vecQamInfo;

typedef struct  _tagQamGroup
{
	std::string		_qamIP;
	long			_qamMode;
	long			_qamsymbolRate;
	vecQamInfo		_qamInfo;
}QamConfig;

typedef	std::vector<QamConfig>		QamGroup;

typedef struct _tagServiceNode 
{
	std::string		_groupID;
	QamGroup		_qamGroup;
}ServiceNode;

typedef std::vector<ServiceNode>	ServiceGroup;



class CConfigParser  
{
public:
	CConfigParser();
	CConfigParser(char* pConfPath);
	CConfigParser(std::string ConfPath);
	virtual ~CConfigParser();

	typedef	std::map<std::string,std::string>	MyMap;
	typedef	std::pair<std::string,std::string>	MyPair;
	typedef std::vector<MyMap>					MyVEC;

	typedef	std::pair<std::string,MyVEC>		SitePair;	
	typedef	std::map<std::string,MyVEC>			SiteMap;

	typedef struct _tagPluginInfo
	{
		std::string		_strFilePath;
		std::string		_strConfiguration;
	}PluginInfo;
public:
	bool			ParseConfig(std::vector<std::string>& path);
	void			SetConfigPath(char* pConfPath);
	void			SetConfigPath(std::string confPath);
	std::vector<std::string>	GetSiteName();
	void			GetPluginPathAndInfo(std::vector<PluginInfo>& info);
	bool			GetVSitePathFromHandler(std::string siteName,std::string handler,std::vector<std::string>& path);
	std::string		GetDefaultContenHandler()
	{
		return m_strDefaultContentHandler;
	}
protected:
	bool			ParsePlugin(XMLPreferenceEx* pPref);
	bool			ParseVirtualSite(XMLPreferenceEx* pPref);
	bool			ParseApplication(XMLPreferenceEx* pPref,MyVEC& HandlerToPathMap);
	bool			ParseServiceConfiguration(XMLPreferenceEx* pPref);	
#ifdef _NEW_FEATURE_SUPPORT_QAM_	
//	bool			ParseResourceManager(IPreference* pPref,ServiceGroup& vServiceGourpInfo);
//	bool			ParseServerGroup(IPreference* pPref,ServiceGroup& vServiceGourpInfo);
//	bool			ParseQamIPAndFrequency(IPreference* pPref,ServiceGroup& vServiceGourpInfo);
//	void			GetQamInfo(vecQamToIPConfig& vServiceGourpInfo);
	
	
	bool			ParseQamGroup(IPreference* pPref,QamConfig& qConfig);
	bool			ParseServiceNode(IPreference* pPref,ServiceNode& sNode);
	bool			ParseServiceGroup(IPreference* pPref,ServiceGroup& vSG);
#endif
private:
	char*			GetText(XMLPreferenceEx* pPref,char* pSzItem);
private:
	char						m_szBuf[BUF_SIZE];
	std::string					m_strConfigurationPath;
	std::vector<PluginInfo>		m_vecPluginPathAndInfo;	
	static SiteMap				m_mapSite;
	std::string					m_strDefaultContentHandler;

};
}
}
#endif // !defined(AFX_CONFIGPARSER_H__7F17A779_0326_499C_BB34_9114DD676B94__INCLUDED_)
