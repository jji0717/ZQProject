#ifndef __ZQRESOURCE_H__
#define __ZQRESOURCE_H__

#define ZQ_PRODUCT_NAME			    "CdmiFuse"
#include "../../../build/ZQSrcTreeVer.h" // defines the version number of the source tree

// the following section are static per-project
#define ZQ_FILE_DESCRIPTION        	"CDMI FUSE for Windows"
#ifdef _DEBUG
#define ZQ_FILE_NAME               	"CdmiFuse_d.exe"
#define ZQ_INTERNAL_FILE_NAME      	"CdmiFuse_d"
#else
#define ZQ_FILE_NAME               	"CdmiFuse.exe"
#define ZQ_INTERNAL_FILE_NAME      	"CdmiFuse"
#endif

// the following section are static per-project, but you can define many SDK involved
#define ZQ_PRODUCT_COMMENT          	ZQ_PRODUCT_NAME " " ZQ_PRODUCT_VER_STR3

#endif // __ZQRESOURCE_H__
