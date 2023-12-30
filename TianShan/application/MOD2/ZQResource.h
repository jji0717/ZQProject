#ifndef __ZQTianShan_Application_MOD_RESOURCE_H__
#define __ZQTianShan_Application_MOD_RESOURCE_H__

#define ZQ_PRODUCT_NAME			"TianShan Architecture"

#include "../../../build/ZQSrcTreeVer.h" // defines the version number of the source tree


// the following section are static per-project
#define ZQ_FILE_DESCRIPTION        	ZQ_PRODUCT_NAME ": MOD application service"
#ifdef _DEBUG
#define ZQ_INTERNAL_FILE_NAME      	"ModSvc"
#else
#define ZQ_INTERNAL_FILE_NAME      	"ModSvc"
#endif // _DEBUG
#define ZQ_FILE_NAME               	ZQ_INTERNAL_FILE_NAME ".exe"

// the following section are static per-project, but you can define many SDK involved
#define ZQ_PRODUCT_COMMENT          	ZQ_FILE_DESCRIPTION " " ZQ_PRODUCT_VER_STR3

#endif // __ZQTianShan_Application_MOD_RESOURCE_H__

