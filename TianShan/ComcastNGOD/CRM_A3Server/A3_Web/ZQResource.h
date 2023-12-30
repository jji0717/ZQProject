#ifndef __ZQTianShan_BroadcastChannel_RESOURCE_H__
#define __ZQTianShan_BroadcastChannel_RESOURCE_H__

#define ZQ_PRODUCT_NAME			"TianShan Architecture"

// the following section will be replaced with the real value by the ZQAutoBuild process
#define ZQ_PRODUCT_VER_MAJOR  	1	// updated by autobuild process @ 2009-12-23 21:09
#define ZQ_PRODUCT_VER_MINOR  	15	// updated by autobuild process @ 2009-12-23 21:09
#define ZQ_PRODUCT_VER_PATCH  	0	// updated by autobuild process @ 2009-12-23 21:09
#define ZQ_PRODUCT_VER_BUILD  	1	// updated by autobuild process @ 2009-12-23 21:09
#define ZQ_PRODUCT_BUILDTIME   	""	// updated by autobuild process @ 2009-12-23 21:09

// the following section are static per-project
#define ZQ_FILE_DESCRIPTION         "ZQ TianShan Asset Propagation Web"
#ifdef _DEBUG
#define ZQ_INTERNAL_FILE_NAME      	"APM_Web_d"
#else
#define ZQ_INTERNAL_FILE_NAME      	"APM_Web"
#endif // _DEBUG
#define ZQ_FILE_NAME               	ZQ_INTERNAL_FILE_NAME ".dll"

// the following section are static per-project, but you can define many SDK involved
#define ZQ_PRODUCT_COMMENT          	ZQ_FILE_DESCRIPTION " " ZQ_PRODUCT_VER_STR3

#endif // __ZQTianShan_BroadcastChannel_RESOURCE_H__

