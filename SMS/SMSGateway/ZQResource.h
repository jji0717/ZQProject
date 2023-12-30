#ifndef __ZQRESOURCE_H__
#define __ZQRESOURCE_H__

// the following section will be replaced with the real value by the ZQAutoBuild process
#define ZQ_PRODUCT_VER_MAJOR		1
#define ZQ_PRODUCT_VER_MINOR		0
#define ZQ_PRODUCT_VER_PATCH		8
#define ZQ_PRODUCT_VER_BUILD		4
#define ZQ_PRODUCT_VER_STR1		"1.0.8.4"
#define ZQ_PRODUCT_VER_STR2		"1.0.8.4"
#define ZQ_PRODUCT_VER_STR3		"V0.1.a (build 0)"
#define ZQ_PRODUCT_NAME			"SeaChange NPVR Components"

#include "VODVersion.h"

#define ISA_TARGET_ENV          	
//VOD_VERSION_DESC "; ISA " ISA_VERSION


// the following section are static per-project
#define ZQ_FILE_DESCRIPTION        	"SeaChange NPVR Components SMS Application"
#ifdef _DEBUG
#define ZQ_INTERNAL_FILE_NAME      	"SMSGateway_d"
#else
#define ZQ_INTERNAL_FILE_NAME      	"SMSGateway"
#endif // _DEBUG
#define ZQ_FILE_NAME               	ZQ_INTERNAL_FILE_NAME ".exe"

// the following section are static per-project, but you can define many SDK involved
#include "VODVersion.h"


#define ZQ_PRODUCT_COMMENT          	ZQ_PRODUCT_NAME " " ZQ_PRODUCT_VER_STR3

#endif // __ZQRESOURCE_H__
