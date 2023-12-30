#ifndef __ZQRESOURCE_H__
#define __ZQRESOURCE_H__

// the following section will be replaced with the real value by the ZQAutoBuild process
#define ZQ_PRODUCT_VER_MAJOR		1
#define ZQ_PRODUCT_VER_MINOR		2
#define ZQ_PRODUCT_VER_PATCH		1
#define ZQ_PRODUCT_VER_BUILD		1
#define ZQ_PRODUCT_VER_STR1		"1.2.1"
#define ZQ_PRODUCT_VER_STR2		"1,2,1,1"
#define ZQ_PRODUCT_VER_STR3		"NSSync V1.2.1 (build 1) Multi-Language Version"

// the following section are static per-project
#define ZQ_PRODUCT_NAME			"SeaChange Multiverse - Navigation Service (NSSync)"
#define ZQ_FILE_DESCRIPTION        	"Service Executable for NSSync"
#ifdef _DEBUG
#define ZQ_FILE_NAME               	"NSSync_d.exe"
#define ZQ_INTERNAL_FILE_NAME      	"NSSync_d"
#else
#define ZQ_FILE_NAME               	"NSSync.exe"
#define ZQ_INTERNAL_FILE_NAME      	"NSSync"
#endif

// the following section are static per-project, but you can define many SDK involved
#include "VODVersion.h"
#define ZQ_PRODUCT_COMMENT          	ZQ_PRODUCT_NAME " " ZQ_PRODUCT_VER_STR3 " for: " VOD_VERSION_DESC

#endif // __ZQRESOURCE_H__




