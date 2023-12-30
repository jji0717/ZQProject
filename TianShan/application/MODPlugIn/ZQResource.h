#ifndef __ZQRESOURCE_H__
#define __ZQRESOURCE_H__

#define ZQ_PRODUCT_NAME			"SeaChange TianShan Architecture"

#include "../../../build/ZQSrcTreeVer.h" // defines the version number of the source tree

// the following section are static per-project
#define ZQ_FILE_DESCRIPTION        	ZQ_PRODUCT_NAME ": MOD Helper Object for SeaChange Topologic"
#ifdef _DEBUG
#define ZQ_INTERNAL_FILE_NAME      	"mho_plugin_d"
#else
#define ZQ_INTERNAL_FILE_NAME      	"mho_plugin"
#endif // _DEBUG
#define ZQ_FILE_NAME               	ZQ_INTERNAL_FILE_NAME ".dll"

// the following section are static per-project, but you can define many SDK involved
#define ZQ_PRODUCT_COMMENT          	ZQ_FILE_DESCRIPTION " " ZQ_PRODUCT_VER_STR3 "\n Supports playlist query and authorization"

#endif // __ZQRESOURCE_H__
