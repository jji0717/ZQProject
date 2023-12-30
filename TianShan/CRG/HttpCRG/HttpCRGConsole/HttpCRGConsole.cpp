// HttpCRGConsole.cpp : 定义控制台应用程序的入口点。
//
//include std header
#include <sstream>

//include tianshan header
#include "TianShanDefines.h"

//include 
#include "HttpCRGConfig.h"
#include "FileLog.h"
#include "ClientRequestGw.h"

const char *DefaultConfigPath = "C:\\tianshan\\etc\\HttpCRG.xml";

ZQ::common::Config::Loader<::ZQTianShan::HttpCRG::HttpCRGConfig> gConfig(DefaultConfigPath);

static bool validatePath( const char *     szPath )
{
	if (-1 != ::GetFileAttributesA(szPath))
		return true;

	DWORD dwErr = ::GetLastError();
	if ( dwErr == ERROR_PATH_NOT_FOUND || dwErr == ERROR_FILE_NOT_FOUND )
	{
		if (!::CreateDirectoryA(szPath, NULL))
		{
			dwErr = ::GetLastError();
			if ( dwErr != ERROR_ALREADY_EXISTS)
			{
				return false;
			}
		}
	}
	else
	{
		return false;
	}

	return true;
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

static void fixupConfig(::ZQTianShan::HttpCRG::HttpCRGConfig &config)
{
	// get the program root
	std::string tsRoot = ZQTianShan::getProgramRoot();
	// fixup the dump path
	if(gConfig._crashDump.enabled && gConfig._crashDump.path.empty())
	{
		// use the Root/logs as default dump folder
		gConfig._crashDump.path = tsRoot + FNSEPS + "logs" + FNSEPS;
	}
	else
	{
		fixupDir(gConfig._crashDump.path);
	}

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
		// get the program root
		std::string tsRoot = ZQTianShan::getProgramRoot();
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

	// fixup EventChannel endpoint
	//if(gConfig._eventChannel.endPoint.empty())
	//{
	//	gConfig._eventChannel.endPoint = DEFAULT_ENDPOINT_TopicManager;
	//}
}

bool bQuit = false;

BOOL WINAPI ConsoleHandler(DWORD CEvent)
{
	switch(CEvent)
	{
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		bQuit = true;
		break;
	}
	return TRUE;
}

int main(int argc, char* argv[])
{
	if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE) ConsoleHandler, TRUE)==FALSE)
	{
		printf("Unable to install handler!                      \n");
		return -1;
	}


	gConfig.load(DefaultConfigPath);
	// step 1: fixup the config
	fixupConfig(gConfig);

	//create logger
	::ZQ::common::FileLog _fileLog(gConfig._logFile.path.c_str(), gConfig._logFile.level, gConfig._logFile.maxCount, gConfig._logFile.size, gConfig._logFile.bufferSize, gConfig._logFile.flushTimeout);

	//gConfig.snmpRegister("");

	//step 3. init HttpCRG
	CRG::CRGateway* _crg = new CRG::CRGateway(_fileLog);

	::std::stringstream ss;
	ss << gConfig._bind.port;
	_crg->setEndpoint(gConfig._bind.ip, ss.str());

	_crg->setModEnv(gConfig._pluginsConfig.configDir, gConfig._pluginsConfig.logDir);

	_crg->setCapacity(gConfig._threadPoolConfig.size);

	try{
		// load modules
		for (std::set<std::string>::iterator iter = gConfig._pluginsConfig.modules.begin();
			iter != gConfig._pluginsConfig.modules.end(); iter++)
		{
			_crg->addModule(iter->c_str());
		}
		for (::ZQTianShan::HttpCRG::PluginsConfig::ModuleConfigHolderVec::iterator iter= gConfig._pluginsConfig._modules.begin();
			iter != gConfig._pluginsConfig._modules.end(); iter++)
		{
			_crg->addModule(iter->image.c_str());
		}
		_crg->start();
	}
	catch (...)
	{
		return S_FALSE;
	}

	_fileLog(::ZQ::common::Log::L_INFO, CLOGFMT(CRG_HttpConsole, "CRG_Http Service created"));
	while (!bQuit)
	{
		static const char* chs="-\\|/";
		static int chi=0;
		chi = ++chi %4;
		printf("\rCRG_HttpConsole is now listening %c", chs[chi]);
		//printf("\nCRG_HttpConsole is now listening");
		Sleep(1000);
	}

	// uninit
	if(_crg)
	{
		try{
			_crg->stop();
			delete _crg;
		}catch(...)
		{
		}
	}

	return 0;
}

