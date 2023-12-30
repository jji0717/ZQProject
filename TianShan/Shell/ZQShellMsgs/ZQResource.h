#ifndef SEARESOURCE_H_
#define SEARESOURCE_H_

#define RES_FILE_VERSION_NUMERIC    1,0,0,0
#define RES_FILE_DESCRIPTION        "ZQ Service Shell Messages\0"
#define RES_FILE_VERSION            "V1.0\0"
#define RES_FILE_NAME               "ZQShellMsgs.dll\0"
#define RES_INTERNAL_NAME           "ZQShellMsgs\0"

#endif

#ifndef __ZQRESOURCE_H__
#define __ZQRESOURCE_H__

#define ZQ_PRODUCT_NAME			"TianShan Architecture"

#define __N2S2__(x) #x
#define __N2S__(x) __N2S2__(x)

#include "../../../build/ZQSrcTreeVer.h" // defines the version number of the source tree

// the following section are static per-project
#define ZQ_FILE_DESCRIPTION        	ZQ_PRODUCT_NAME ": ZQShellMsgs"
#ifdef _DEBUG
#define ZQ_INTERNAL_FILE_NAME      	"ZQShellMsgs"
#else
#define ZQ_INTERNAL_FILE_NAME      	"ZQShellMsgs"
#endif // _DEBUG
#define ZQ_FILE_NAME               	ZQ_INTERNAL_FILE_NAME ".dll"

// the following section are static per-project, but you can define many SDK involved
#define ZQ_PRODUCT_COMMENT          	ZQ_FILE_DESCRIPTION " " ZQ_PRODUCT_VER_STR3

#endif // __ZQRESOURCE_H__


