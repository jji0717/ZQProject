#ifndef __ZQRESOURCE_H__
#define __ZQRESOURCE_H__


#include "PauseTVVersion.h"


// the following section are static per-project
#define ZQ_FILE_DESCRIPTION        	"ZQ TianShan ChannelOnDemand Service"
#ifdef _DEBUG
#define ZQ_INTERNAL_FILE_NAME      	"chodsvc_d"
#else
#define ZQ_INTERNAL_FILE_NAME      	"chodsvc"
#endif // _DEBUG
#define ZQ_FILE_NAME               	ZQ_INTERNAL_FILE_NAME ".exe"

// the following section are static per-project, but you can define many SDK involved

#define ZQ_PRODUCT_COMMENT          	ZQ_PRODUCT_NAME " " ZQ_PRODUCT_VER_STR3

#endif // __ZQRESOURCE_H__
