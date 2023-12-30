
#include "HttpCRGSvc.h"
#include "HttpCRGConfig.h"
#include "TianShanDefines.h"
#include <sstream>

ZQTianShan::HttpCRG::HttpCRGSvc g_HttpCRGSvc;
ZQ::common::ZQDaemon  *Application = &g_HttpCRGSvc;

ZQ::common::Config::Loader< ZQTianShan::HttpCRG::HttpCRGConfig > gConfig("HttpCRG.xml");
ZQ::common::Config::ILoader *configLoader = &gConfig;


//////////////////////////////////
//// class HttpCRGSvc
/////////////////////////////////
namespace ZQTianShan{
namespace HttpCRG{

HttpCRGSvc::HttpCRGSvc()
:_pcrg(NULL)
{
}

HttpCRGSvc::~HttpCRGSvc()
{
}

static void fixupDir(std::string &path)
{
    if(path.empty())
    {
        return;
    }
    else
    {
        if(path[path.size() - 1] != FNSEPC)
            path.push_back(FNSEPC);
    }
}

static void fixupConfig(HttpCRGConfig &config)
{
    // get the program root
	std::string tsRoot = ZQTianShan::getProgramRoot();

    // fixup the plug-in's config path
    if(gConfig._pluginsConfig.configDir.empty())
    {
        // use the Root/etc as default config folder
        gConfig._pluginsConfig.configDir = tsRoot + FNSEPS + "etc" + FNSEPS;
    }
    else
    {
        fixupDir(gConfig._pluginsConfig.configDir);
    }

    // fixup the plug-in's log path
    if(gConfig._pluginsConfig.logDir.empty())
    {
        // use the Root/logs as default log folder
        gConfig._pluginsConfig.logDir = tsRoot + FNSEPS + "logs" + FNSEPS;
    }
    else
    {
        fixupDir(gConfig._pluginsConfig.logDir);
    }

    // expand the plug-in's populate path
    if(!gConfig._pluginsConfig.populatePath.empty())
    {
        gConfig._pluginsConfig.populate(gConfig._pluginsConfig.populatePath);
    }
}

bool HttpCRGSvc::OnInit(void)
{
	ZQ::common::setGlogger(_logger);
    // step 1: fixup the config
    fixupConfig(gConfig);

    // step 2: crash dump

//  gConfig.snmpRegister("HTTPCRG");

    //step 3. init HttpCRG
    _pcrg = new CRG::CRGateway(*_logger);

    ::std::stringstream ss;
    ss << gConfig._bind.port;
    _pcrg->setEndpoint(gConfig._bind.ip, ss.str());

    _pcrg->setModEnv(gConfig._pluginsConfig.configDir, gConfig._pluginsConfig.logDir);

    _pcrg->setCapacity(gConfig._threadPoolConfig.size);


	return ZQDaemon::OnInit();
}

bool HttpCRGSvc::OnStart(void)
{
 	try
	{
        for (std::set<std::string>::iterator iter = gConfig._pluginsConfig.modules.begin();
            iter != gConfig._pluginsConfig.modules.end(); iter++)
        {
            _pcrg->addModule(iter->c_str());
        }
        for (std::vector<PluginsConfig::ModuleConfigHolder>::iterator iter= gConfig._pluginsConfig._modules.begin();
            iter != gConfig._pluginsConfig._modules.end(); iter++)
        {
            _pcrg->addModule(iter->image.c_str());
        }
        _pcrg->start();
    }
    catch (...)
    {
        return false;
    }


	return ZQDaemon::OnStart();
}

void HttpCRGSvc::OnStop(void)
{
	try
	{
        if (_pcrg)
            _pcrg->stop();
    }catch(...){}

	ZQDaemon::OnStop();
}

void HttpCRGSvc::OnUnInit(void)
{
 	if(_pcrg)
    {
        try
		{
            delete _pcrg;
			_pcrg = NULL;
        }catch(...){}
    }

	ZQDaemon::OnUnInit();
}

}
}//namespace





