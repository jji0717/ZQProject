
#ifndef __ZQRESOURCE_H__
#define __ZQRESOURCE_H__

// the following section will be replaced with the real value by the ZQAutoBuild process
//#define ZQ_PRODUCT_VER_MAJOR		1
//#define ZQ_PRODUCT_VER_MINOR		0
//#define ZQ_PRODUCT_VER_PATCH		1
//#define ZQ_PRODUCT_VER_BUILD		100
//#undef  ZQ_PRODUCT_VER_STR1
//#define ZQ_PRODUCT_VER_STR1			"1.0.1.0"
//#undef  ZQ_PRODUCT_VER_STR2
//#define ZQ_PRODUCT_VER_STR2			"1,0,1,0"
//#undef  ZQ_PRODUCT_VER_STR3
// #define ZQ_PRODUCT_VER_STR3			"V1.0.1 (build )"

#include "..\\..\\..\\ChannelOnDemand\\PauseTVVersion.h"

// #define ZQ_PRODUCT_NAME				"ssm_PauseTV_s1.dll"
#define ZQ_FILE_DESCRIPTION        	"A RtspProxy Plugin"
#ifdef _DEBUG
#define ZQ_FILE_NAME               	"ssm_PauseTV_s1.dll"
#define ZQ_INTERNAL_FILE_NAME      	"ssm_PauseTV_s1"
#else
#define ZQ_FILE_NAME               	"ssm_PauseTV_s1.dll"
#define ZQ_INTERNAL_FILE_NAME      	"ssm_PauseTV_s1"
#endif


#define		SSM_PAUSETV_PLUGIN_VER		"PauseTV 1.0"

// the following section are static per-project, but you can define many SDK involved
#include "VODVersion.h"
#define ZQ_PRODUCT_COMMENT          	ZQ_PRODUCT_NAME " " ZQ_PRODUCT_VER_STR3 " for: " VOD_VERSION_DESC

#endif // __ZQRESOURCE_H__


