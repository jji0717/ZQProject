#ifndef __ZQRESOURCE_H__
#define __ZQRESOURCE_H__


#define ZQ_PRODUCT_VER_STR1		"2.0.0.0"
#define ZQ_PRODUCT_VER_STR2		"2,0,0,0"
#define ZQ_PRODUCT_VER_STR3		"V1.0.0 (build 0)"
#define ZQ_PRODUCT_NAME			"ZQ PathSrv&WeiwooSrv Client TestAdminCtrlWtl"

#include "../../../build/ZQSrcTreeVer.h" // defines the version number of the source tree

// the following section are static per-project
#define ZQ_FILE_DESCRIPTION        	"ZQ PathSrv&WeiwooSrv Client TestAdminCtrlWtl"
#ifdef _DEBUG
#define ZQ_FILE_NAME               	"TestAdminCtrlWtl_d.exe"
#define ZQ_INTERNAL_FILE_NAME      	"TestAdminCtrlWtl_d"
#else
#define ZQ_FILE_NAME               	"TestAdminCtrlWtl.exe"
#define ZQ_INTERNAL_FILE_NAME      	"TestAdminCtrlWtl"
#endif

// the following section are static per-project, but you can define many SDK involved
#include "VODVersion.h"
#define ZQ_PRODUCT_COMMENT          	ZQ_PRODUCT_NAME " " ZQ_PRODUCT_VER_STR3 

#endif // __ZQRESOURCE_H__
