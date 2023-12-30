// ConfigParser.cpp: implementation of the CConfigParser class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable:4786)

#include "stdafx.h"
#include "ConfigParser.h"
#include "StreamSmithModule.h"

namespace ZQ{
	namespace StreamSmith {
		

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
const	char	STR_MODULE[]=("module");
const	char	STR_APPLICATION[]=("Application");	
const	char	STR_VIRTUALSITE[]=("virtualSite");
const	char	STR_PLUGIN[]=("plugin");
const	char	STR_FILE[]=("file");
const	char	STR_PATH[]=("path");
const	char	STR_HANDLER[]=("handler");
const	char	STR_STREAMSITE[]=("StreamSmithSite");
const	char	STR_SITENAME[]=("name");
const	char	STR_MODULE_CONFIGURATION[]=("configuration");

#ifdef _NEW_FEATURE_SUPPORT_QAM_
const	char	STR_DELIVERY_PATH[]=("DeliveryPath");
const	char	STR_SERVICE_GROUP[]=("ServiceGroup");
const	char	STR_QAM[]=("Qam");
#endif//_NEW_FEATURE_SUPPORT_QAM_

template<class classType>
class auto_free
{
public:
	auto_free(classType _p)
	{
		_ptr=_p;
	}
	~auto_free()
	{
		if(_ptr!=NULL)
		{
			_ptr->free();
			_ptr=NULL;
		}
	}	
public:
	classType& operator->()
	{
		return _ptr;
	}
	classType& operator=(const classType t)
	{
		if(_ptr!=NULL)
		{
			_ptr->free();
			_ptr=NULL;
		}
		_ptr=t;
		return _ptr;
	}
	classType& operator=(auto_free<classType>& t)
	{
		if(_ptr!=NULL)
		{
			_ptr->free();
			_ptr=NULL;
		}
		_ptr=t._ptr;
		t._ptr=NULL;
		return _ptr;
	}
	operator classType()
	{
		return _ptr;
	}
	bool operator==(classType& t)
	{
		if(t==_ptr)
			return true;
		else
			return false;
	}
	bool operator!()
	{
		return !_ptr;
	}
public:
	classType	_ptr;
};


using namespace ZQ::common;

CConfigParser::SiteMap CConfigParser::m_mapSite;

CConfigParser::CConfigParser()
{	
}
CConfigParser::CConfigParser(std::string ConfPath)
{
	m_strConfigurationPath=ConfPath;
}
CConfigParser::CConfigParser(char* pConfPath)
{
	m_strConfigurationPath=pConfPath;
}
void CConfigParser::SetConfigPath(std::string confPath)
{
	m_strConfigurationPath=confPath;
}
void CConfigParser::SetConfigPath(char* pConfPath)
{
	m_strConfigurationPath=pConfPath;
}

CConfigParser::~CConfigParser()
{
}
char* CConfigParser::GetText(XMLPreferenceEx* pPref,char* pSzItem)
{
	if(!pPref)
	{
		glog(Log::L_ERROR,"char* CConfigParser::GetText()## null pPref pass in");
		return NULL;
	}
	if(!pSzItem)
	{
		glog(Log::L_ERROR,"char* CConfigParser::GetText()## null pszItem pass in");
		return NULL;
	}
	ZeroMemory(m_szBuf,BUF_SIZE);
	pPref->get(pSzItem,m_szBuf,"",BUF_SIZE-1);
	return m_szBuf;
}
bool CConfigParser::ParsePlugin(XMLPreferenceEx* pPref)
{
	if(!pPref)
	{
		glog(Log::L_ERROR,"bool CConfigParser::ParsePlugin(IPreference* pPref)## null pPref pass in");
		return false;
	}
	auto_free<XMLPreferenceEx*> deft=NULL;
	try
	{
		char szBuf[256];
		for(deft=pPref->firstChild(STR_MODULE); deft; deft=pPref->nextChild() )
		{			
			ZeroMemory(szBuf,256);
			deft->name(szBuf,255);
			if(strncmp(szBuf,STR_MODULE,255)!=0)
			{//Invalid property
				glog(Log::L_ERROR,"bool CConfigParser::ParsePlugin(IPreference* pPref)##Invalid item");				
				continue;
			}
			//get file 
			PluginInfo info;
			char* pValue=GetText(deft,(char*)STR_FILE);
			if(!pValue)
			{
				glog(Log::L_ERROR,"bool CConfigParser::ParsePlugin(IPreference* pPref)##Get item value with %s fail",szBuf);
			}
			else
			{
				glog(Log::L_DEBUG,"bool CConfigParser::ParsePlugin(IPreference* pPref)##Get item %s and value is %s",szBuf,pValue);
				/*m_vecPluginPath.push_back(pValue);*/
				info._strFilePath=pValue;
			}

			pValue=GetText(deft,(char*)STR_MODULE_CONFIGURATION);
			if(!pValue)
			{
				glog(Log::L_DEBUG,"bool CConfigParser::ParsePlugin()##can't get configuaretion in node module");
			}
			else
			{
				glog(Log::L_DEBUG,"bool CConfigParser::ParsePlugin()##Get configuration %s",pValue);
				info._strConfiguration=pValue;
			}
			m_vecPluginPathAndInfo.push_back(info);
		}
		return true;
	}
	catch (Exception e)  	
	{		
		glog(Log::L_ERROR,"bool CConfigParser::ParsePlugin(IPreference* pPref)##Exception was threw out and error string is %s",e.getString());		
		return false;
	}
}
bool CConfigParser::ParseApplication(XMLPreferenceEx* pPref,MyVEC& HandlerToPathMap)
{
	if(!pPref)
	{
		glog(Log::L_ERROR,"bool CConfigParser::ParseApplication(IPreference* pPref)##null pPref pass in");
		return false;
	}
	HandlerToPathMap.clear();
	auto_free<XMLPreferenceEx*> deft=NULL;
	try
	{
		//I'll check if there is a default content handler here
	
		char szBuf[256];
		for(deft=pPref->firstChild(STR_APPLICATION);deft;deft=pPref->nextChild())
		{
			ZeroMemory(szBuf,256);
			deft->name(szBuf,255);
			if(strncmp(szBuf,STR_APPLICATION,255)!=0)
			{				
				//glog(Log::L_ERROR,"bool CConfigParser::ParseApplication(IPreference* pPref)##unrecognise item %s",szBuf);
				continue;
			}
			else
			{
				std::string strPath=GetText(deft,(char*)STR_PATH);
				if(strPath.size()<=0)
				{
					glog(Log::L_ERROR,"bool CConfigParser::ParseApplication(IPreference* pPref)## get path fail");					
					continue;
				}
				std::string strHandler=GetText(deft,(char*)STR_HANDLER);
				if(strHandler.size()<=0)
				{
					glog(Log::L_ERROR,"bool CConfigParser::ParseApplication(IPreference* pPref)##Get handler fail");					
					continue;
				}
				glog(Log::L_DEBUG,"bool CConfigParser::ParseApplication(IPreference* pPref)##Get pair value with handler=%s and path=%s",strHandler.c_str(),strPath.c_str());
				MyPair	p(strHandler,strPath);
				MyMap m;m.insert(p);
				HandlerToPathMap.push_back(m);
			}			
		}
		return true;
	}
	catch (Exception e)  
	{
		glog(Log::L_ERROR,"bool CConfigParser::ParseApplication(IPreference* pPref)## exception was threw out and error string = %s",e.getString());
		return false;
	}
}
bool CConfigParser::ParseConfig(std::vector<std::string>& path)
{
	try
	{
		//ComInitializer init;	
		XMLPreferenceDocumentEx root;
		if(!root.open(m_strConfigurationPath.c_str()))
		{
			glog(Log::L_ERROR,"bool CConfigParser::ParseConfig()##Load configuration fail %s fail",m_strConfigurationPath.c_str());
			return false;
		}
		{
			auto_free<XMLPreferenceEx*> deft=NULL;
			auto_free<XMLPreferenceEx*> pPref=NULL;		
			CConfigParser::m_mapSite.clear();
			pPref=root.getRootPreference();			
			auto_free<XMLPreferenceEx*> pStreamSmithSite=NULL;//pPref->firstChild(STR_STREAMSITE);
			while (path.size() > 0)
			{
				pStreamSmithSite=pPref->firstChild(path[0].c_str());
				if(!pStreamSmithSite)
				{
					glog(ZQ::common::Log::L_ERROR,"can't find node %s",path[0].c_str());
					return false;
				}
				pPref=pStreamSmithSite;
				path.erase(path.begin());
			}
			pStreamSmithSite=pPref;
			/*pPref->free();*/
			if(!pStreamSmithSite)
			{
				glog(Log::L_ERROR,"Can't get StreamSmithSite node");
				return false;
			}
			{
				
				//Get defaulSite Application
				MyVEC	DefaultSiteMap;
				char szDefaultContentHandler[256];
				ZeroMemory(szDefaultContentHandler,sizeof(szDefaultContentHandler));
				pStreamSmithSite->get("defaultHandler",szDefaultContentHandler,"",255);
				glog(ZQ::common::Log::L_INFO,"Get default ContentHandler;[%s]",szDefaultContentHandler);
				m_strDefaultContentHandler= szDefaultContentHandler;				
				if(!ParseApplication(pStreamSmithSite,DefaultSiteMap))
				{					
					glog(Log::L_ERROR,"bool CConfigParser::ParseConfig()##Get defaultSite information fail");				
				}	
				else
				{
					
					SitePair p(STR_SITE_DEFAULT,DefaultSiteMap);
					CConfigParser::m_mapSite.insert(p);
				}			
			}
			{
				MyVEC	VSMap;			
				for(deft=pStreamSmithSite->firstChild(STR_VIRTUALSITE); deft; deft=pStreamSmithSite->nextChild() )
				{
					char szBuf[256];
					ZeroMemory(szBuf,256);
					deft->name(szBuf,255);
					if(strncmp(szBuf,STR_VIRTUALSITE,255)==0)
					{
						//Get virtual site name
						ZeroMemory(szBuf,256);
						deft->get(STR_SITENAME,szBuf,"",255);
						
						if(!ParseApplication(deft,VSMap))
						{
							glog(Log::L_ERROR,"bool CConfigParser::ParseConfig()## Parse application under %s fail",szBuf);
						}
						else
						{
							SitePair p(szBuf,VSMap);
							CConfigParser::m_mapSite.insert(p);
						}
					}
					else if(strncmp(szBuf,STR_PLUGIN,255)==0)
					{
						if(!ParsePlugin(deft))
						{
							glog(Log::L_ERROR,"bool CConfigParser::ParseConfig()##Parse plugin fail");
						}
					}					
				}
							
				for( deft = pStreamSmithSite->firstChild(STR_PLUGIN); deft; deft=pStreamSmithSite->nextChild() )
				{
					char szBuf[256];
					ZeroMemory(szBuf,256);
					deft->name(szBuf,255);
					if( strncmp(szBuf,STR_VIRTUALSITE,255) == 0 )
					{
						//Get virtual site name
						ZeroMemory(szBuf,256);
						deft->get(STR_SITENAME,szBuf,"",255);
						
						if(!ParseApplication(deft,VSMap))
						{
							glog(Log::L_ERROR,"bool CConfigParser::ParseConfig()## Parse application under %s fail",szBuf);
						}
						else
						{
							SitePair p(szBuf,VSMap);
							CConfigParser::m_mapSite.insert(p);
						}
					}
					else if(strncmp(szBuf,STR_PLUGIN,255)==0)
					{
						if(!ParsePlugin(deft))
						{
							glog(Log::L_ERROR,"bool CConfigParser::ParseConfig()##Parse plugin fail");
						}
					}					
				}

			}		
			
#ifdef _NEW_FEATURE_SUPPORT_QAM_
			//parse qam configuration
			auto_free<IPreference*> pDeliveryPath=pPref->firstChild("DeliveryPath");
			if(!pDeliveryPath)
			{
				glog(Log::L_ERROR,"bool CConfigParser::ParseConfig()##Can't get node %s","DeliveryPath");
				return false;
			}
			ServiceGroup sg;
			ParseServiceGroup(pDeliveryPath,sg);
#endif			
		}

		return true;
	}
	catch (Exception e)  
	{	
		glog(Log::L_ERROR,"bool CConfigParser::ParseConfig()## unexpect exception");
		return false;
	}
	return true;
}

bool CConfigParser::GetVSitePathFromHandler(std::string siteName,std::string handler,std::vector<std::string>& path)
{
	if(m_mapSite.size()<=0)
	{
		glog(Log::L_ERROR,"bool CConfigParser::GetVSitePathFromHandler()## no site map record");
		return false;
	}	
	SiteMap::iterator iter=m_mapSite.find(siteName);
	if(m_mapSite.end()==iter)
	{
		glog(Log::L_ERROR,"bool CConfigParser::GetVSitePathFromHandler()## can't find the record with sitename=%s",siteName.c_str());
		return false;
	}
	MyVEC& tempVec=iter->second;
	MyVEC::iterator	vecIter=tempVec.begin();
	path.clear();
	for(;vecIter!=tempVec.end();vecIter++)
	{
		MyMap& tempMap=*vecIter;
		if(tempMap.find(handler)!=tempMap.end())
		{
			path.push_back(tempMap[handler]);
		}
		else
		{
		//	glog(Log::L_ERROR,"bool CConfigParser::GetVSitePathFromHandler()## can't find the path with sitename = %s and handler = %s",siteName.c_str(),handler.c_str());
		}	
	}
	//path=pathIter->second;
	return path.size()>0;
	
}
std::vector<std::string> CConfigParser::GetSiteName()
{
	SiteMap::iterator iter;
	std::vector<std::string>	SiteName;

	for(iter=m_mapSite.begin();iter!=m_mapSite.end();iter++)
	{
		SiteName.push_back(iter->first);
	}
	return SiteName;
}
void CConfigParser::GetPluginPathAndInfo(std::vector<PluginInfo>& pathInfo)
{
	pathInfo=m_vecPluginPathAndInfo;
}
#ifdef _NEW_FEATURE_SUPPORT_QAM_
bool CConfigParser::ParseServiceGroup(IPreference* pPref,ServiceGroup& vSG)
{
	glog(Log::L_DEBUG,"enter bool CConfigParser::ParseServiceGroup");
	if(!pPref)
	{
		glog(Log::L_ERROR,"bool CConfigParser::ParseServiceGroup()##NULL PPref pass in");
		return false;
	}
	ServiceNode	sn;
	auto_free<IPreference*> deft=NULL;
	for(deft=pPref->firstChild( ); deft; deft=pPref->nextChild() )
	{
		ParseServiceNode(deft,sn);
		vSG.push_back(sn);
	}
	glog(Log::L_DEBUG,"leave bool CConfigParser::ParseServiceGroup");
	return true;
}
bool CConfigParser::ParseServiceNode(IPreference* pPref,ServiceNode& sNode)
{
	glog(Log::L_DEBUG,"enter bool CConfigParser::ParseServiceNode");
	if(!pPref)
	{
		glog(Log::L_ERROR,"bool CConfigParser::ParseServiceNode()## NULL pPref pass IN");
		return false;
	}
	char	szBuf[1024];
	int		iBufSize=sizeof(szBuf);
	pPref->get("id",szBuf,"",iBufSize);
	sNode._groupID=atoi(szBuf);
	auto_free<IPreference*> deft=NULL;
	QamConfig	qc;
	for(deft=pPref->firstChild();deft;deft=pPref->nextChild())
	{
		ParseQamGroup(deft,qc);
		sNode._qamGroup.push_back(qc);
	}
	glog(Log::L_DEBUG,"Leave bool CConfigParser::ParseServiceNode");
	return true;
}
bool CConfigParser::ParseQamGroup(IPreference* pPref,QamConfig& qConfig)
{
	glog(Log::L_DEBUG,"Enter bool CConfigParser::ParseQamGroup");
	if(!pPref)
	{
		glog(Log::L_ERROR,"bool CConfigParser::ParseQamGroup()## NULL pPref pass IN");
		return false;
	}
	char	szBuf[1024];
	int		iBufSize=sizeof(szBuf);
	ZeroMemory(szBuf,iBufSize);

	//Get Qam IP
	pPref->get("IP",szBuf,"",iBufSize);
	glog(Log::L_DEBUG,"Get IP %s",szBuf);	
	qConfig._qamIP=szBuf;

	pPref->get("mode",szBuf,"",iBufSize);
	glog(Log::L_DEBUG,"get mode %s",szBuf);
	qConfig._qamMode=atoi(szBuf);

	pPref->get("symbolRate",szBuf,"",iBufSize);
	glog(Log::L_DEBUG,"get symbolRate %s",szBuf);
	qConfig._qamsymbolRate=atoi(szBuf);
	
	auto_free<IPreference*> deft=NULL;
	QamInfo				qi;
	for(deft=pPref->firstChild();deft;deft=pPref->nextChild())
	{
		deft->get("frequency",szBuf,"",iBufSize);
		glog(Log::L_DEBUG,"Get frequency %s",szBuf);
		qi._frequency=atoi(szBuf);

		deft->get("baseport",szBuf,"",iBufSize);
		glog(Log::L_DEBUG,"Get baseport %s",szBuf);
		qi._basePort=atoi(szBuf);

		deft->get("basePN",szBuf,"",iBufSize);
		glog(Log::L_DEBUG,"get basePN %s",szBuf);
		qi._baseProgramNumber=atoi(szBuf);

		deft->get("PNCount",szBuf,"",iBufSize);
		glog(Log::L_DEBUG,"get PNCount %s",szBuf);
		qi._programCount=atoi(szBuf);

		deft->get("bandwidth",szBuf,"",iBufSize);
		glog(Log::L_DEBUG,"get bandwidth %s",szBuf);
		qi._bandwidth=atoi(szBuf);

		qConfig._qamInfo.push_back(qi);
	}
	glog(Log::L_DEBUG,"Leave bool CConfigParser::ParseQamGroup");
	return true;
}
#endif
}
}
