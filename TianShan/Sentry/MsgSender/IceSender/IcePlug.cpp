// EventSender.cpp : Defines the entry point for the DLL application.
//
#include "IceSender.h"
#include <FileLog.h>

using namespace std;
using namespace ZQ::common;

IceSender* pIceSender = NULL;

void OnIceMessage(const MSGSTRUCT& msgStruct)
{
	if(pIceSender == NULL)
		return;

 	pIceSender->AddMessage(msgStruct);
}

void ExitSender()
{
	if(pIceSender != NULL)
	{
		pIceSender->Close();
		delete pIceSender;
		pIceSender = NULL;
		LOG(Log::L_INFO,"EventSender PlugIn exit");		
		return;
	}
	if(plog != NULL)
		LOG(Log::L_INFO,"PlugIn has exit");	
}

bool init(const char* pText)
{
	if(pText==NULL || strlen(pText)==0)
		return false;
	
	if(pIceSender == NULL)
		pIceSender = new IceSender(); 
	//init 
	if(pIceSender)
	{
		if(!pIceSender->GetParFromFile(pText))
		{
			if(plog != NULL)
			{
				LOG(Log::L_ERROR,"ICE sender get configuration error");
				delete plog;
				plog = NULL;
			}
			if(pEventSenderCfg != NULL)
			{
				delete pEventSenderCfg;
				pEventSenderCfg = NULL;
			}
			delete pIceSender;
			pIceSender = NULL;
			return false;		
		}
		if(!pIceSender->init())
		{
			if(plog != NULL)
			{
				LOG(Log::L_ERROR,"ICE sender init error");
				delete plog;
				plog = NULL;
			}
			if(pEventSenderCfg != NULL)
			{
				delete pEventSenderCfg;
				pEventSenderCfg = NULL;
			}
			
			delete pIceSender;
			pIceSender = NULL;
			return false;
		}
		
		if(plog != NULL)
			LOG(Log::L_INFO,"ICE Plugin init successful");
	}
	else
		return false;

	return true;
}
/*
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

        curSnapshot.swap(buf.str());
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
            SYSTEMTIME nowtm;
            GetSystemTime(&nowtm);
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

static bool initModWithConf(const char *pConfPath)
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
*/

extern "C"
{

bool InitModuleEntry( IMsgSender* pISender, const char* pType, const char* pText)
{
	if(pISender == NULL)
		return false;
	
	if(strcasecmp(pType,"ice") == 0)
	{	
		if(init(pText))
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
/*
    else if(stricmp(pType, "snapshot") == 0)
    {
            if(initModWithConf(pText))
            {
                return pISender->regist((OnNewMessage)OnSnapshotMessage,"snapshot");
            }
    }
*/
	if(plog != NULL)
		LOG(Log::L_ERROR,"PlugIn register %s failed",pType);
	return false;
}

void UninitModuleEntry( IMsgSender* pISender )
{
	if(pISender != NULL)
	{
		pISender->unregist((OnNewMessage)OnIceMessage,"ICE");
//        pISender->unregist((OnNewMessage)OnSnapshotMessage,"snapshot");
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




