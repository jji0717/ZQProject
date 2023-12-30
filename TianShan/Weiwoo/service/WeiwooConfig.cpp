
#include "WeiwooConfig.h"
#include <Exception.h>

#define READCONF(x)	"[ReadConfiguration]:"##x

using namespace ZQ::common;
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


WeiwooServiceConfig::WeiwooServiceConfig()
{	
	_pLog=NULL;

	
	strcpy(szWeiwooSvcAdapterThreadPool,"5");
	strcpy(szIceStormEndpoint,"");
	strcpy(szWeiwooSvcEndpoint,"");
	strcpy(szphoPluginFolder,"");

	strcpy(szWeiwooLogFileName,"Weiwoo.log");
	lWeiwooLogLevel=7;
	lWeiwooLogFileSize=10240000;
	lWeiwooLogBufferSize=204800;
	lWeiwooLogWriteTimeout=2;

	strcpy(szPathManagerLogFileName,"PathManager.log");lPathManagerLogLevel=7;
	lPathManagerLogFileSize=10240000;
	lPathManagerLogBufferSize=204800;
	lPathManagerLogWriteTimeout=2;

	strcpy(szPHOLogFileName,"SCIPEdge.log");
	lPHOLogLevel=7;
	lPHOLogFileSize=10240000;
	lPHOLogBufferSize=204800;
	lPHOLogWriteTimteout=2;

	strcpy(szIceTraceLogFileName,"iceTrace.log");
	lIceTraceLogLevel=7;
	lIceTraceLogFileSize=10240000;
	lIceTraceLogBufferSize=204800;
	lIceTraceLogWriteTimteout=2;
}
WeiwooServiceConfig::~WeiwooServiceConfig()
{

}

/*
	lIceTraceLogBufferSize=204800;
	lIceTraceLogWriteTimteout=2;*/
WeiwooServiceConfig::ConfigProperty* WeiwooServiceConfig::GetConfigEntry()
{
	static ConfigProperty entry[]=
	{
		{"WeiwooSvcAdapterThreadPool", &szWeiwooSvcAdapterThreadPool,sizeof(szWeiwooSvcAdapterThreadPool),WeiwooServiceConfig::ConfigProperty::CONFTYPE_STRING},
		{"IceStormEndpoint", &szIceStormEndpoint,sizeof(szIceStormEndpoint),WeiwooServiceConfig::ConfigProperty::CONFTYPE_STRING},
		{"WeiwooSvcEndpoint", &szWeiwooSvcEndpoint,sizeof(szWeiwooSvcEndpoint),WeiwooServiceConfig::ConfigProperty::CONFTYPE_STRING},
		{"WeiwooLogFileName", &szWeiwooLogFileName,sizeof(szWeiwooLogFileName),WeiwooServiceConfig::ConfigProperty::CONFTYPE_STRING},
		{"WeiwooLogLevel", &lWeiwooLogLevel,sizeof(lWeiwooLogLevel),WeiwooServiceConfig::ConfigProperty::CONFTYPE_NUMBER},
		{"WeiwooLogFileSize", &lWeiwooLogFileSize,sizeof(lWeiwooLogFileSize),WeiwooServiceConfig::ConfigProperty::CONFTYPE_NUMBER},
		{"WeiwooLogBufferSize", &lWeiwooLogBufferSize,sizeof(lWeiwooLogBufferSize),WeiwooServiceConfig::ConfigProperty::CONFTYPE_NUMBER},
		{"WeiwooLogWriteTimeout", &lWeiwooLogWriteTimeout,sizeof(lWeiwooLogWriteTimeout),WeiwooServiceConfig::ConfigProperty::CONFTYPE_NUMBER},
		{"PathManagerLogFileName", &szPathManagerLogFileName,sizeof(szPathManagerLogFileName),WeiwooServiceConfig::ConfigProperty::CONFTYPE_STRING},
		{"PathManagerLogLevel", &lPathManagerLogLevel,sizeof(lPathManagerLogLevel),WeiwooServiceConfig::ConfigProperty::CONFTYPE_NUMBER},
		{"PathManagerLogFileSize", &lPathManagerLogFileSize,sizeof(lPathManagerLogFileSize),WeiwooServiceConfig::ConfigProperty::CONFTYPE_NUMBER},
		{"PathManagerLogBufferSize", &lPathManagerLogBufferSize,sizeof(lPathManagerLogBufferSize),WeiwooServiceConfig::ConfigProperty::CONFTYPE_NUMBER},
		{"PathManagerLogWriteTimeout", &lPathManagerLogWriteTimeout,sizeof(lPathManagerLogWriteTimeout),WeiwooServiceConfig::ConfigProperty::CONFTYPE_NUMBER},
		{"PHOLogFileName",&szPHOLogFileName,sizeof(szPHOLogFileName),WeiwooServiceConfig::ConfigProperty::CONFTYPE_STRING},
		{"PHOLogLevel", &lPHOLogLevel,sizeof(lPHOLogLevel),WeiwooServiceConfig::ConfigProperty::CONFTYPE_NUMBER},
		{"PHOLogFileSize", &lPHOLogFileSize,sizeof(lPHOLogFileSize),WeiwooServiceConfig::ConfigProperty::CONFTYPE_NUMBER},
		{"PHOLogBufferSize", &lPHOLogBufferSize,sizeof(lPHOLogBufferSize),WeiwooServiceConfig::ConfigProperty::CONFTYPE_NUMBER},
		{"PHOLogWriteTimteout", &lPHOLogWriteTimteout,sizeof(lPHOLogWriteTimteout),WeiwooServiceConfig::ConfigProperty::CONFTYPE_NUMBER},
		{"IceTraceLogFileName",&szIceTraceLogFileName,sizeof(szIceTraceLogFileName),WeiwooServiceConfig::ConfigProperty::CONFTYPE_STRING},
		{"IceTraceLogLevel", &lIceTraceLogLevel,sizeof(lIceTraceLogLevel),WeiwooServiceConfig::ConfigProperty::CONFTYPE_NUMBER},
		{"IceTraceLogFileSize", &lIceTraceLogFileSize,sizeof(lIceTraceLogFileSize),WeiwooServiceConfig::ConfigProperty::CONFTYPE_NUMBER},
		{"IceTraceLogBufferSize", &lIceTraceLogBufferSize,sizeof(lIceTraceLogBufferSize),WeiwooServiceConfig::ConfigProperty::CONFTYPE_NUMBER},
		{"IceTraceLogWriteTimteout", &lIceTraceLogWriteTimteout,sizeof(lIceTraceLogWriteTimteout),WeiwooServiceConfig::ConfigProperty::CONFTYPE_NUMBER},
		{"phoPluginFolder", &szphoPluginFolder,sizeof(szphoPluginFolder),WeiwooServiceConfig::ConfigProperty::CONFTYPE_STRING},

		{NULL,NULL,0,WeiwooServiceConfig::ConfigProperty::CONFTYPE_NUMBER}
	};
	return entry;
}
typedef auto_free<IPreference*>	PREF;
bool WeiwooServiceConfig::ParseConfig(char *szFilePath,ZQ::common::Log* pLog)
{
	_pLog=pLog;
	ComInitializer init;
	XMLPrefDoc root(init);
	PREF pref = NULL,  child = NULL;
	try
	{
		if(!root.open(szFilePath))
		{
			(*_pLog)(Log::L_ERROR,READCONF("Can't open WeiwooServiceConfig file %s"),szFilePath);
			return false;
		}
		pref=root.root();
		pref=pref->firstChild("Service");
		if(!pref)
		{
			(*_pLog)(Log::L_ERROR,READCONF("Can't loacte node 'Service',please check the WeiwooServiceConfig file"));
			return false;
		}
		ConfigProperty* p=GetConfigEntry();
		char	szBuf[1024];
		while (p->key!=NULL) 
		{
			child=pref->firstChild(p->key);
			if(!child)
			{
				(*_pLog)(Log::L_ERROR,READCONF("Can't find key=%s"),p->key);
				p++;
				continue;
			}
			ZeroMemory(szBuf,sizeof(szBuf));
			child->get("value",szBuf,"",sizeof(szBuf)-1);
			(*_pLog)(Log::L_DEBUG,READCONF("Get configuration '%s' with value = %s"),p->key,szBuf);
			switch(p->type)
			{
			case ConfigProperty::CONFTYPE_NUMBER:
				{
					long	iTemp=atol(szBuf);
					memcpy(p->value,&iTemp,p->valueLen);
				}
				break;
			case ConfigProperty::CONFTYPE_STRING:
				{
					memcpy(p->value,szBuf,((int)(strlen(szBuf)+1)>p->valueLen)?p->valueLen:(strlen(szBuf)+1) );
				}
				break;
			default:
				{
					(*_pLog)(Log::L_ERROR,READCONF("not support WeiwooServiceConfig type"));
				}
				break;
			}
			p++;
		}
		
	}
	catch (ZQ::common::Exception& e) 
	{
		(*_pLog)(Log::L_ERROR,READCONF("exception occured when parse WeiwooServiceConfig file %s"),szFilePath);
		(*_pLog)(Log::L_ERROR,READCONF("error description is %s"),e.what());
		return false;
	}
	return true;
}


