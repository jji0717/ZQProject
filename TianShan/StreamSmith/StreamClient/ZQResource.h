#ifndef __ZQRESOURCE_H__
#define __ZQRESOURCE_H__

#define ZQ_PRODUCT_NAME			"SeaChange TianShan Architecture"

#include "../../../build/ZQSrcTreeVer.h" // defines the version number of the source tree


// the following section are static per-project
#define ZQ_FILE_DESCRIPTION        	ZQ_PRODUCT_NAME ": StreamSmith Client"
#ifdef _DEBUG
#define ZQ_INTERNAL_FILE_NAME      	"StreamClient_d"
#else
#define ZQ_INTERNAL_FILE_NAME      	"StreamClient"
#endif // _DEBUG
#define ZQ_FILE_NAME               	ZQ_INTERNAL_FILE_NAME ".exe"

// the following section are static per-project, but you can define many SDK involved
#define ZQ_PRODUCT_COMMENT          	ZQ_FILE_DESCRIPTION " " ZQ_PRODUCT_VER_STR3

#endif // __ZQRESOURCE_H__
