// EventSender.cpp : Defines the entry point for the DLL application.
//
#pragma warning(disable:4503)
#include "StdAfx.h"
#include "MagSender.h"
#include <fstream>
#include <algorithm>
#include <TimeUtil.h>
#include <FileLog.h>

using namespace std;
using namespace ZQ::common;


#ifdef ZQ_OS_MSWIN
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}
#endif
 
MagSender* pMagSender = NULL;

void OnIceMessage(const MSGSTRUCT& msgStruct)
{
	if(pMagSender == NULL)
		return;

 	pMagSender->iceMessage(msgStruct);
}

void OnJmsMessage(const MSGSTRUCT& msgStruct)
{
	if(pMagSender == NULL)
		return;

 	pMagSender->jmsMessage(msgStruct);
}

void OnTextMessage(const MSGSTRUCT& msgStruct)
{
	if(pMagSender == NULL)
		return;

 	pMagSender->textMessage(msgStruct);
}

void ExitSender()
{
	if(pMagSender != NULL)
	{
		pMagSender->uninit();
		delete pMagSender;
		pMagSender = NULL;
		LOG(Log::L_INFO,"EventSender PlugIn exit");		
		return;
	}
	if(plog != NULL)
		LOG(Log::L_INFO,"PlugIn has exit");	
}

bool init(const char* pText,const char* pType)
{
	if(pText==NULL || strlen(pText)==0 || pType==NULL || strlen(pType)==0)
		return false;
	
	if(pMagSender == NULL)
		pMagSender = new MagSender(); 
	//init 
	if(pMagSender)
	{
		if(!pMagSender->init(pText,pType))
		{
			if(plog != NULL)
			{
				LOG(Log::L_ERROR,"Plugin init %s error",pType);
				delete plog;
				plog = NULL;
			}

			if(pEventSenderCfg != NULL)
			{
				delete pEventSenderCfg;
				pEventSenderCfg = NULL;
			}

			delete pMagSender;
			pMagSender = NULL;
			return false;
		}
		if(plog != NULL)
			LOG(Log::L_INFO,"Plugin init %s successful",pType);
	}
	else
		return false;

	return true;
}

typedef std::map<std::string, std::string> SimpleSetting;
// format of config text:
//      key1=value1; key2=value2;
// keys are case-sensitive
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

        cfg[item[0]] = item[1];
    }
}

// system status snapshot
static 
void OnSnapshotMessage(const MSGSTRUCT& msgStruct)
{
    // check the message's properties for the snapshot instruction
    typedef std::map<std::string, std::string> StringMap;
    StringMap::const_iterator itTarget = msgStruct.property.find("#snapshot.target"); // the target file path
    if(msgStruct.property.end() == itTarget)
    {
        return;
    }

    StringMap::const_iterator itTemplate = msgStruct.property.find("#snapshot.template"); // the template file path
    if(msgStruct.property.end() == itTemplate)
    {
        LOG(ZQ::common::Log::L_WARNING, CLOGFMT(OnSnapshotMessage, "snapshot reqest received but no template file supplied. target=%s"), itTarget->second.c_str());
        return;
    }

    // load the template file and create the target file
    // The template file is always a text file and shouldn't be very large.
    std::ofstream target;
    target.open(itTarget->second.c_str());
    if(!target.good())
    { // can't open the target file
        LOG(ZQ::common::Log::L_WARNING, CLOGFMT(OnSnapshotMessage, "snapshot file not writable. target=%s, template=%s"), itTarget->second.c_str(), itTemplate->second.c_str());
        return;
    }

    std::string curSnapshot;
    // load the template into memory
    { 
        std::ifstream tmpl;
        tmpl.open(itTemplate->second.c_str());
        if(!tmpl.good())
        { // failed to open template file
            LOG(ZQ::common::Log::L_WARNING, CLOGFMT(OnSnapshotMessage, "template file not readable. target=%s, template=%s"), itTarget->second.c_str(), itTemplate->second.c_str());
            return;
        }
        std::ostringstream buf;
        char ch;
        while(tmpl.get(ch))
        {
            buf << ch;
        }

	std::string tmp = buf.str();
        curSnapshot.swap(tmp);
    }

    // fixup the template with this message
    try
    {
        
        ZQ::common::Preprocessor ctx;
        // predefined variable
        // NOW
        if(!msgStruct.timestamp.empty())
        { // use message stamp as 'now'
            ctx.define("NOW", msgStruct.timestamp);
        }
        else
        { // the real 'now'
	    time_t nowtm = time(0);
            char utcbuf[40] = {0};
            ZQ::common::TimeUtil::Time2Iso(nowtm, utcbuf, sizeof(utcbuf));
            ctx.define("NOW", utcbuf);
        }
        
        // setup the environment
        StringMap::const_iterator itEnvironment = msgStruct.property.find("#snapshot.env"); // the environment
        if(msgStruct.property.end() != itEnvironment)
        {
            // parse the environment string
            SimpleSetting env;
            parseConfig(itEnvironment->second.c_str(), env);
            for(SimpleSetting::iterator it = env.begin(); it != env.end(); ++it)
            {
                ctx.define(it->first, it->second);
            }
        }

        ctx.fixup(curSnapshot);
    }
    catch (ZQ::common::PreprocessException &e)
    {
        LOG(ZQ::common::Log::L_WARNING, CLOGFMT(OnSnapshotMessage, "PreprocessException (%s) during generating the snapshot. target=%s, template=%s"), e.getString(), itTarget->second.c_str(), itTemplate->second.c_str());
        return;
    }
    catch(...)
    {
        LOG(ZQ::common::Log::L_WARNING, CLOGFMT(OnSnapshotMessage, "Unexpect exception during generating the snapshot. target=%s, template=%s"), itTarget->second.c_str(), itTemplate->second.c_str());
        return;
    }

    std::copy(curSnapshot.begin(), curSnapshot.end(), std::ostream_iterator<char>(target));
    LOG(ZQ::common::Log::L_DEBUG, CLOGFMT(OnSnapshotMessage, "snapshot file generated at %s. template=%s"), itTarget->second.c_str(), itTemplate->second.c_str());
}
static bool initModWithConf(const char *pConfPath);
extern "C"
{

__EXPORT bool InitModuleEntry( IMsgSender* pISender, const char* pType, const char* pText)
{
	if(pISender == NULL)
		return false;
	if(stricmp(pType,"jms") == 0)
	{	
		if(init(pText,pType))
		{
			if(pISender->regist((OnNewMessage)OnJmsMessage,pType))
			{
				LOG(Log::L_INFO,"PlugIn register JMS successful");
				return true;
			}
			LOG(Log::L_ERROR,"JMS register failed");
			ExitSender();//release plug resource
		}
	}
	else if(stricmp(pType,"ice") == 0)
	{	
		if(init(pText,pType))
		{
			if(pISender->regist((OnNewMessage)OnIceMessage,pType))
			{
				LOG(Log::L_INFO,"PlugIn register ICE successful");
				return true;	
			}
			LOG(Log::L_ERROR,"ICE register failed");
			ExitSender();//release plug resource
		}
	}
	else if(stricmp(pType,"text") == 0)
	{	
		if(init(pText,pType))
		{
			if(pISender->regist((OnNewMessage)OnTextMessage,pType))
			{
				LOG(Log::L_INFO,"PlugIn register TextWriter successful");
				return true;
			}
			LOG(Log::L_ERROR,"TextWriter regist failed");
			ExitSender();//release plug resource
		}
	}
    else if(stricmp(pType, "snapshot") == 0)
    {
            if(initModWithConf(pText))
            {
                return pISender->regist((OnNewMessage)OnSnapshotMessage,"snapshot");
            }
    }

	if(plog != NULL)
		LOG(Log::L_ERROR,"PlugIn register %s failed",pType);
	return false;
}

__EXPORT void UninitModuleEntry( IMsgSender* pISender )
{
	if(pISender != NULL)
	{
		pISender->unregist((OnNewMessage)OnIceMessage,"ICE");
		pISender->unregist((OnNewMessage)OnJmsMessage,"JMS");
        pISender->unregist((OnNewMessage)OnTextMessage,"TEXT");
        pISender->unregist((OnNewMessage)OnSnapshotMessage,"snapshot");
	}
	
	ExitSender();

	if(pEventSenderCfg != NULL)
	{
		delete pEventSenderCfg;
		pEventSenderCfg = NULL;
	}

	if(plog != NULL)
	{
		LOG(Log::L_INFO,"PlugIn unregister");
		delete plog;
		plog = NULL;
	}

}
}//extern "c"


bool initModWithConf(const char *pConfPath)
{ //load config item form xml config file	
    if(pEventSenderCfg == NULL)
    {
        pEventSenderCfg = new Config::Loader< EventSender >("");

        if(!pEventSenderCfg)
        { // out of memory?
            return false;
        }
        if(!pEventSenderCfg->load(pConfPath))
        { // bad conf file
            return false;
        }
        pEventSenderCfg->snmpRegister("");
    }

    if(plog == NULL)
    {
        try
        {
            plog = new ZQ::common::FileLog(pEventSenderCfg->logPath.c_str(),pEventSenderCfg->logLevel,5,pEventSenderCfg->logSize);
        }
        catch(FileLogException&)
        {
            return false;
        }
        catch(...)
        {
            return false;
        }
    }

    return true;
}
