#ifndef __ZQRESOURCE_H__
#define __ZQRESOURCE_H__

// the following section will be replaced with the real value by the ZQAutoBuild process
#define ZQ_PRODUCT_VER_MAJOR		1
#define ZQ_PRODUCT_VER_MINOR		0
#define ZQ_PRODUCT_VER_PATCH		0
#define ZQ_PRODUCT_VER_BUILD		177
#define ZQ_PRODUCT_VER_STR1		"1.0.0.177"
#define ZQ_PRODUCT_VER_STR2		"1,0,0,177"
#define ZQ_PRODUCT_VER_STR3		"V1.0.0 (build 177)"
#define ZQ_PRODUCT_NAME			"Project Template"

// the following section are static per-project
#define ZQ_FILE_DESCRIPTION        	"Simple Console Program Template"
#ifdef _DEBUG
#define ZQ_FILE_NAME               	"console_d.exe"
#define ZQ_INTERNAL_FILE_NAME      	"console_d"
#else
#define ZQ_FILE_NAME               	"console.exe"
#define ZQ_INTERNAL_FILE_NAME      	"console"
#endif

// the following section are static per-project, but you can define many SDK involved
#include "VODVersion.h"
#define ZQ_PRODUCT_COMMENT          	ZQ_PRODUCT_NAME " " ZQ_PRODUCT_VER_STR3 " for: " VOD_VERSION_DESC

#endif // __ZQRESOURCE_H__
