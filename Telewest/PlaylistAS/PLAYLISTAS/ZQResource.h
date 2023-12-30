#ifndef __ZQRESOURCE_H__
#define __ZQRESOURCE_H__

// the following section will be replaced with the real value by the ZQAutoBuild process
#define ZQ_PRODUCT_VER_MAJOR  	1	// updated by autobuild process @ 2006-10-31 11:25
#define ZQ_PRODUCT_VER_MINOR  	0	// updated by autobuild process @ 2006-10-31 11:25
#define ZQ_PRODUCT_VER_PATCH  	8	// updated by autobuild process @ 2006-10-31 11:25
#define ZQ_PRODUCT_VER_BUILD  	4	// updated by autobuild process @ 2006-10-31 11:25
#define ZQ_PRODUCT_VER_STR1   	"1.0.8.4"	// updated by autobuild process @ 2006-10-31 11:25
#define ZQ_PRODUCT_VER_STR2   	"1,0,8,4"	// updated by autobuild process @ 2006-10-31 11:25
#define ZQ_PRODUCT_VER_STR3   	"V1.0.8 (build 4)"	// updated by autobuild process @ 2006-10-31 11:25

// the following section are static per-project
#define ZQ_PRODUCT_NAME			"Telewest VOD"
#define ZQ_FILE_DESCRIPTION        	"Service executable for PlaylistAS"
#ifdef _DEBUG
#define ZQ_FILE_NAME               	"PlaylistAS_d.exe"
#define ZQ_INTERNAL_FILE_NAME      	"PlaylistAS_d"
#else
#define ZQ_FILE_NAME               	"PlaylistAS.exe"
#define ZQ_INTERNAL_FILE_NAME      	"PlaylistAS"
#endif

// the following section are static per-project, but you can define many SDK involved
#include "VODVersion.h"
#define ZQ_PRODUCT_COMMENT          	ZQ_PRODUCT_NAME " " ZQ_PRODUCT_VER_STR3 " for: " VOD_VERSION_DESC

#endif // __ZQRESOURCE_H__




