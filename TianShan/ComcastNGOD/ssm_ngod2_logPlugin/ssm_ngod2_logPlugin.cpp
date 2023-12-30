//
#include <ConfigHelper.h>
#include "NGOD2mdbViewer.h"
#include "PluginConfig.h"

::ZQ::common::Config::Loader< NGOD2PlugInCfg > pConfig("");
::ZQ::common::FileLog _fileLog;
#define LOG _fileLog // global log

bool gInited;
static bool dbConnected; // used by checkDbConnection() and breakDbConnection()
static ZQ::common::Mutex lockConn; // the lock that protect the db connection
OSTRMDBViewer _mdbViewer; // the db communicator

#define SYSLOG_SLOT_MSG     "#syslog.msg"
#define SYSLOG_CATEGORY		"SSM_OSTR"
#define SYSLOG_SLOT_LEVEL   "#syslog.lvl"

#ifdef ZQ_OS_MSWIN
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                       )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		gInited = false;
        dbConnected = false;
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
#endif
// return true if the connection is ok, else false.
static bool checkDbConnection();
static void breakDbConnection();

static void OnSyslogMessage(const MSGSTRUCT& msg)
{
    using namespace ZQ::common;
	if (msg.category.compare(SYSLOG_CATEGORY) == 0)
	{
        if(checkDbConnection())
        {
            if(!_mdbViewer.ProcessLog(msg))
            {
                LOG(Log::L_WARNING, CLOGFMT(OSTRLogger,"Can't process message: eventName(%s), sourceNetId(%s), stamp(%s)"), msg.eventName.c_str(), msg.sourceNetId.c_str(), msg.timestamp.c_str());
            }
            // else not record the success here
        }
        else
            LOG(Log::L_WARNING, CLOGFMT(OSTRLogger,"Can't connect db, ignore message: eventName(%s), sourceNetId(%s), stamp(%s)"), msg.eventName.c_str(), msg.sourceNetId.c_str(), msg.timestamp.c_str());
	}	
}

#define SESSLOGNAME "SessLog"
extern "C"
{
	__EXPORT bool InitModuleEntry( IMsgSender* pISender, const char* pType, const char* pText)
	{
		if(gInited)
		{ // has been registered
			return false;
		}

		//pConfig.load();

		if(NULL == pISender || NULL == pType || NULL == pText)
			return false;

		if(stricmp(pType, SESSLOGNAME) == 0)
		{
			// init the config file and mdbViewer
			pConfig.load(pText);

			if (!pConfig._myLog.path.empty())
				_fileLog.open(pConfig._myLog.path.c_str(),
							  pConfig._myLog.level,
							  pConfig._myLog.logNum,
							  pConfig._myLog.size,
							  pConfig._myLog.flushTimeout);


            _mdbViewer.SetLogger(LOG);
            checkDbConnection(); // connect db once here but not care the result

			// the SessLog ok now
			if(pISender->regist((OnNewMessage)OnSyslogMessage, SESSLOGNAME))
			{
				gInited = true;
				return true;
			}
			return true;
		}
		return false;
	}

	__EXPORT void UninitModuleEntry( IMsgSender* pISender )
	{
		_fileLog(::ZQ::common::Log::L_INFO,CLOGFMT(OSTRLogger,"OSTRLogger uninitialize"));
		if(gInited && pISender)
		{
			pISender->unregist((OnNewMessage)OnSyslogMessage, SESSLOGNAME);
			gInited = false;
			_fileLog(::ZQ::common::Log::L_INFO,CLOGFMT(OSTRLogger,"OSTRLogger unregist %s"), SESSLOGNAME);
            breakDbConnection();
		}
	}


}//extern "c"


static bool checkDbConnection()
{
    if(!dbConnected)
    {
        ZQ::common::MutexGuard guard(lockConn);
        if(dbConnected) // double check
            return true;

        if(pConfig._dbPath.type == "mysql" || pConfig._dbPath.type == "mysql5")
        {
            dbConnected = _mdbViewer.InitDatabase(pConfig._dbPath.dsn, pConfig._dbPath.user, pConfig._dbPath.auth);
            if (!dbConnected)
            {
                LOG(::ZQ::common::Log::L_ERROR,CLOGFMT(OSTRLogger,"OSTRLogger initialize database with DSN(%s) failed"), pConfig._dbPath.dsn.c_str());
            }
            else
                LOG(::ZQ::common::Log::L_INFO,CLOGFMT(OSTRLogger,"OSTRLogger initialize database with DSN=%s;user=%s;auth=%s"), pConfig._dbPath.dsn.c_str(), pConfig._dbPath.user.c_str(), pConfig._dbPath.auth.c_str());
        }
        else if(pConfig._dbPath.type == "access")
        {
            dbConnected = _mdbViewer.InitDatabase(pConfig._dbPath.path, pConfig._dbPath.templatPath);
            if (!dbConnected)
            {
                LOG(::ZQ::common::Log::L_ERROR,CLOGFMT(OSTRLogger,"OSTRLogger initialize database with path(%s) failed, also could not copy template(%s)"), pConfig._dbPath.path.c_str(), pConfig._dbPath.templatPath.c_str());
            }
            else
                LOG(::ZQ::common::Log::L_INFO,CLOGFMT(OSTRLogger,"OSTRLogger initialize database with dbpath=%s"), pConfig._dbPath.path.c_str());
        }
        else
        {
            LOG(::ZQ::common::Log::L_WARNING,CLOGFMT(OSTRLogger,"db type(%s) not support"), pConfig._dbPath.type.c_str());
        }

        if(dbConnected)
        {
            uint32 iTimeOut = 60*60;
            if (pConfig._timeOut.iTime > 0)
                iTimeOut = pConfig._timeOut.iTime;
            LOG(::ZQ::common::Log::L_DEBUG,CLOGFMT(OSTRLogger,"OSTRLogger initialize data expire time=%d(seconds)"), iTimeOut*60);
            _mdbViewer.SetTimeOut(iTimeOut*60);

            _mdbViewer.start();
            LOG(::ZQ::common::Log::L_DEBUG,CLOGFMT(OSTRLogger,"OSTRLogger initialize success"));
        }
    }

    return dbConnected;
}
static void breakDbConnection()
{
    if(dbConnected)
    {
        ZQ::common::MutexGuard guard(lockConn);
        if(!dbConnected) // double check
            return;

        _mdbViewer.Uninit();
        dbConnected = false;
    }
}
