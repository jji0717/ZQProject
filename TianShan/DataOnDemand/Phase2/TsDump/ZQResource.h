#ifndef __ZQRESOURCE_H__
#define __ZQRESOURCE_H__

#include "../../../../build/ZQSrcTreeVer.h" // defines the version number of the source tree

#define ZQ_PRODUCT_VER_STR1			"2.10.0.1"
#define ZQ_PRODUCT_VER_STR2			"2,10,0,1"
#define ZQ_PRODUCT_VER_STR3			"V2.10.0 (build 1)"
#define ZQ_PRODUCT_NAME				"SeaChange DataOnDemand TsDump"

// the following section are static per-project
#define ZQ_FILE_DESCRIPTION        	"TsDump"
#ifdef _DEBUG
#define ZQ_INTERNAL_FILE_NAME      	"TsDump_d"
#else
#define ZQ_INTERNAL_FILE_NAME      	"TsDump"
#endif // _DEBUG
#define ZQ_FILE_NAME               	ZQ_INTERNAL_FILE_NAME ".exe"

#define ZQ_PRODUCT_COMMENT          	ZQ_PRODUCT_NAME " " ZQ_PRODUCT_VER_STR3

#endif // __ZQRESOURCE_H__
