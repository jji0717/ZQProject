#ifndef _WEIWOO_SERVICE_PLUGIN_CONFIG_H__
#define _WEIWOO_SERVICE_PLUGIN_CONFIG_H__

#include <configloader.h>
class Config  :public ZQ::common::ConfigLoader
{
public:
	virtual ConfigSchemaItem*		getSchema()
	{
		static ConfigSchemaItem entry[]=			
		{
			{"PHO/log",
				"level",	&lPHOLogLevel,
				sizeof(&lPHOLogLevel), false, ConfigSchemaItem::TYPE_INTEGER},
			{"PHO/log",
				"size",		&lPHOLogFileSize,
				sizeof(&lPHOLogFileSize), false, ConfigSchemaItem::TYPE_INTEGER},
			{"PHO/log",
				"buffer",	&lPHOLogBufferSize,
				sizeof(&lPHOLogBufferSize), false, ConfigSchemaItem::TYPE_INTEGER},
			{"PHO/log",
				"flushtimeout",	&lPHOLogWriteTimteout,
				sizeof(&lPHOLogWriteTimteout), false, ConfigSchemaItem::TYPE_INTEGER},
			{NULL,NULL,NULL,0,true,ConfigSchemaItem::TYPE_STRING}
		};
		return entry;
	}
	Config()
	{
		strcpy(szPHOLogFileName,"pho_NGOD_ss.log");
		lPHOLogLevel   = 7;
		lPHOLogFileSize = 50*1024*1024;
		lPHOLogBufferSize =10*1024;
		lPHOLogWriteTimteout = 2;
	}
public:	
	char						szPHOLogFileName[512];
	long						lPHOLogLevel;
	long						lPHOLogFileSize;
	long						lPHOLogBufferSize;
	long						lPHOLogWriteTimteout;
};
#endif//_WEIWOO_SERVICE_PLUGIN_CONFIG_H__