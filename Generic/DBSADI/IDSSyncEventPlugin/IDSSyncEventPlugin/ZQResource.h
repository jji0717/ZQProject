#ifndef __ZQRESOURCE_H__
#define __ZQRESOURCE_H__

// the following section will be replaced with the real value by the ZQAutoBuild process
#define ZQ_PRODUCT_VER_MAJOR  	3	// updated by autobuild process @ 2006-10-27 16:55
#define ZQ_PRODUCT_VER_MINOR  	3	// updated by autobuild process @ 2006-10-27 16:55
#define ZQ_PRODUCT_VER_PATCH  	2	// updated by autobuild process @ 2006-10-27 16:55
#define ZQ_PRODUCT_VER_BUILD  	56	// updated by autobuild process @ 2006-10-27 16:55
#define ZQ_PRODUCT_VER_STR1   	"3.3.2.56"	// updated by autobuild process @ 2006-10-27 16:55
#define ZQ_PRODUCT_VER_STR2   	"3,3,2,56"	// updated by autobuild process @ 2006-10-27 16:55
#define ZQ_PRODUCT_VER_STR3   	"V3.3.2 (build 56)"	// updated by autobuild process @ 2006-10-27 16:55
#define ZQ_PRODUCT_NAME				"Itv"

// the following section are static per-project
#define ZQ_FILE_DESCRIPTION        	"Jms DBSync add-in"
#ifdef _DEBUG
#define ZQ_INTERNAL_FILE_NAME      	"JMS_DBSyncAddIn_d"
#else
#define ZQ_INTERNAL_FILE_NAME        	"JMS_DBSyncAddIn"
#endif
#define ZQ_FILE_NAME               	ZQ_INTERNAL_FILE_NAME ".dll"

// the following section are static per-project, but you can define many SDK involved
#include "VODVersion.h"
#define ZQ_PRODUCT_COMMENT          	ZQ_PRODUCT_NAME " " ZQ_PRODUCT_VER_STR3 " for: " VOD_VERSION_DESC

#endif // __ZQRESOURCE_H__
