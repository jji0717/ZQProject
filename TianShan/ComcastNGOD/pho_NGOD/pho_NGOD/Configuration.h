#ifndef _TIANSHAN_NGOD_PHO_CONFIGURATION_HEADER_FILE_H__
#define _TIANSHAN_NGOD_PHO_CONFIGURATION_HEADER_FILE_H__

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
		strcpy(szPHOLogFileName,"pho_NGOD.log");
		lPHOLogLevel			= 7;
		lPHOLogFileSize			= 1024 * 1000 * 10;
		lPHOLogBufferSize		= 10240;
		lPHOLogWriteTimteout	= 2;
	}

	static void structure(ZQ::common::Config::Holder<PHOConfig> &holder)
	{
		using namespace ZQ::common::Config;
		holder.addDetail("PHO/log", "level", &PHOConfig::lPHOLogLevel, "7", optReadOnly);
		holder.addDetail("PHO/log", "size", &PHOConfig::lPHOLogFileSize, "10240000", optReadOnly);
		holder.addDetail("PHO/log", "buffer", &PHOConfig::lPHOLogBufferSize, "10240", optReadOnly);
		holder.addDetail("PHO/log", "flushtimeout", &PHOConfig::lPHOLogWriteTimteout, "2", optReadOnly);
	}
};

#endif //_TIANSHAN_NGOD_PHO_CONFIGURATION_HEADER_FILE_H__
