#ifndef _WEIWOO_SERVICE_PLUGIN_CONFIG_H__
#define _WEIWOO_SERVICE_PLUGIN_CONFIG_H__

#include <ConfigHelper.h>
struct PHOConfig
{
    char szPHOLogFileName[512];
    int32 lPHOLogLevel;
    int32 lPHOLogFileSize;
    int32 lPHOLogBufferSize;
    int32 lPHOLogWriteTimteout;
    
    PHOConfig()
    {
        strcpy(szPHOLogFileName,"pho_seachange.log");
    }
    
    static void structure(ZQ::common::Config::Holder<PHOConfig> &holder)
    {
        using namespace ZQ::common::Config;
        holder.addDetail("PHO/log", "level", &PHOConfig::lPHOLogLevel, NULL, optReadOnly);
        holder.addDetail("PHO/log", "size", &PHOConfig::lPHOLogFileSize, NULL, optReadOnly);
        holder.addDetail("PHO/log", "buffer", &PHOConfig::lPHOLogBufferSize, NULL, optReadOnly);
        holder.addDetail("PHO/log", "flushtimeout", &PHOConfig::lPHOLogWriteTimteout, NULL, optReadOnly);
    }
};
#endif//_WEIWOO_SERVICE_PLUGIN_CONFIG_H__

