//
#include <ZQ_common_conf.h>
#include <strHelper.h>
#include <TimeUtil.h>
#include <MsgSenderInterface.h>
#include <ConfigHelper.h>
#include "RemoteSyslog.h"


IMsgSender* gSender = NULL;
#ifdef ZQ_OS_MSWIN
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
        gSender = NULL;
        break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}
#endif

RemoteSyslog gSyslog;
#define SYSLOG_SLOT_MSG     "#syslog.msg"
#define SYSLOG_SLOT_LEVEL   "#syslog.lvl"
static void OnSyslogMessage(const MSGSTRUCT& msg, const MessageIdentity& mid, void* ctx)
{
    // convert the structured message to plain text
    static int counter = 0; // the internal message counter

    // check the message body
    std::map<std::string, std::string>::const_iterator pContent;
    pContent = msg.property.find(SYSLOG_SLOT_MSG);
    if(msg.property.end() == pContent)
    { // no message need to be sent
        if(gSender) { // ack the message processed
            gSender->ack(mid, ctx);
        }
        return;
    }

    std::string msgContent;
    msgContent.reserve(pContent->second.size() * 2);
    msgContent = pContent->second;

    // fixup the message
    {
        try
        {
            ZQ::common::Preprocessor pp;

            char buf[16] = {0};
            itoa(++counter, buf, 10);
            pp.define("SEQ", buf);
            pp.fixup(msgContent);
        }
        catch(ZQ::common::PreprocessException &)
        { // Failed to fixup the counter? Shouldn't happened!
            return;
        }
        catch(...)
        {
            return;
        }
    }

    // convert time stamp
    using namespace ZQ::common;
	std::string strLocalTime;
	char buf[50] = {0};
	if(TimeUtil::Iso2Local(msg.timestamp.c_str(), buf, sizeof(buf)))
	{
		strLocalTime = buf;
	}
	else
	{
		char chlocal[50] = {0};
		int64 nT = now();
		TimeUtil::TimeToUTC(nT, buf, sizeof(buf));
		TimeUtil::Iso2Local(buf, chlocal, sizeof(chlocal));
		strLocalTime = chlocal;
	}

    // log level
    int level = -1;
    std::map<std::string, std::string>::const_iterator pLevel;
    pLevel = msg.property.find(SYSLOG_SLOT_LEVEL);
    if(msg.property.end() != pLevel)
    {
        level = RemoteSyslog::levelCode(pLevel->second.c_str());
    }
    level = (-1 == level) ? LOG_INFO : level;

    // convert net id
    InetAddress addr;
    if(addr.setAddress(msg.sourceNetId.c_str()) && addr.isInetAddress())
    { // use the source net id as the host name
        gSyslog.write(level, msgContent.c_str(), strLocalTime, msg.sourceNetId.c_str());
    }
    else
    {
        gSyslog.write(level, msgContent.c_str(), strLocalTime);
    }

    if(gSender) {
        gSender->ack(mid, ctx);
    }
}
typedef std::map<std::string, std::string> SimpleSetting;
// format of config text:
//      key1=value1; key2=value2;
// keys are case-insensitive
static void parseConfig(const char* pCfgText, SimpleSetting &cfg)
{
    cfg.clear();
    if(NULL == pCfgText)
        return;

    using namespace ZQ::common;
    stringHelper::STRINGVECTOR vec;
    stringHelper::SplitString(pCfgText, vec, ";");
    for(size_t i = 0; i < vec.size(); ++i)
    {
        stringHelper::STRINGVECTOR item;
        stringHelper::SplitString(vec[i], item, "=", "= ");
        if(item.size() != 2)
            continue; // discard bad configuration

        std::transform(item[0].begin(), item[0].end(), item[0].begin(), tolower);
        cfg[item[0]] = item[1];
    }
}
extern "C"
{
    __EXPORT bool InitModuleEntry( IMsgSender* pISender, const char* pType, const char* pText)
    {
        if(gSender)
        { // has been registered
            return false;
        }

        if(NULL == pISender || NULL == pType || NULL == pText)
            return false;

        if(stricmp(pType,"syslog") == 0)
        {
            // init the gSyslog with config in the pText
            SimpleSetting cfg;
            parseConfig(pText, cfg);

            std::string dest = cfg["destination"];
            std::string ident = cfg["ident"];
            std::string fac = cfg["facility"];

            if(!gSyslog.setup(dest.c_str()))
                return false;

            int nFac = RemoteSyslog::facilityCode(fac.c_str());
            if(nFac < 0)
                nFac = LOG_DAEMON;

            if(!gSyslog.open(ident.c_str(), nFac))
            {
                return false;
            }

            // the syslog ok now
            if(pISender->regist((OnNewMessage)OnSyslogMessage,"syslog"))
            {
                gSender = pISender;
                return true;
            }
        }
        return false;
    }

    __EXPORT void UninitModuleEntry( IMsgSender* pISender )
    {
        if(gSender && gSender == pISender)
        {
            pISender->unregist((OnNewMessage)OnSyslogMessage,"syslog");
            gSender = NULL;
        }
    }


}//extern "c"
