#ifndef __ZQRESOURCE_H__
#define __ZQRESOURCE_H__

#define ZQ_PRODUCT_NAME				"ZQ TianShan"

#include "../../build/ZQSrcTreeVer.h" // defines the version number of the source tree

// the following section are static per-project
#define ZQ_FILE_DESCRIPTION        	"ZQ EventRuleEngine"
#ifdef _DEBUG
#define ZQ_INTERNAL_FILE_NAME      	"EventRuleEngine_d"
#else
#define ZQ_INTERNAL_FILE_NAME      	"EventRuleEngine"
#endif // _DEBUG
#define ZQ_FILE_NAME               	ZQ_INTERNAL_FILE_NAME ".exe"

#define ZQ_PRODUCT_COMMENT          	ZQ_PRODUCT_NAME " " ZQ_PRODUCT_VER_STR3

#endif // __ZQRESOURCE_H__
