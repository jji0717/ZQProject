#ifndef __ZQRESOURCE_H__
#define __ZQRESOURCE_H__

// the following section will be replaced with the real value by the ZQAutoBuild process
#define ZQ_PRODUCT_VER_MAJOR		0
#define ZQ_PRODUCT_VER_MINOR		4
#define ZQ_PRODUCT_VER_PATCH		7
#define ZQ_PRODUCT_VER_BUILD		1
#define ZQ_PRODUCT_VER_STR1		"0.4.7.1"
#define ZQ_PRODUCT_VER_STR2		"0,4,7,1"
#define ZQ_PRODUCT_VER_STR3		"V0.4.7 (build 1)"

// the following section are static per-project
#define ZQ_PRODUCT_NAME			"Scheduled TV"
#define ZQ_FILE_DESCRIPTION        	"Service executable for ITVPlayback"
#ifdef _DEBUG
#define ZQ_FILE_NAME               	"ITVPlayback_d.exe"
#define ZQ_INTERNAL_FILE_NAME      	"ITVPlayback_d"
#else
#define ZQ_FILE_NAME               	"ITVPlayback.exe"
#define ZQ_INTERNAL_FILE_NAME      	"ITVPlayback"
#endif

// the following section are static per-project, but you can define many SDK involved
#include "VODVersion.h"
#define ZQ_PRODUCT_COMMENT          	ZQ_PRODUCT_NAME " " ZQ_PRODUCT_VER_STR3 " for: " VOD_VERSION_DESC

#endif // __ZQRESOURCE_H__




