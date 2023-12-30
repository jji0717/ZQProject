#ifndef __ZQRESOURCE_H__
#define __ZQRESOURCE_H__

#define ZQ_PRODUCT_VER_MAJOR  	1
#define ZQ_PRODUCT_VER_MINOR  	15
#define ZQ_PRODUCT_VER_PATCH  	0
#define ZQ_PRODUCT_VER_BUILD  	0
#define ZQ_PRODUCT_NAME			"ZQ TianShan"


#ifdef ZQ_PRODUCT_NAME
#  undef ZQ_PRODUCT_NAME
#endif

#define ZQ_PRODUCT_NAME			"ZQ TianShan"

// the following section are static per-project
#define ZQ_FILE_DESCRIPTION        	"ZQ TianShan CMEV2"

#ifdef _DEBUG
#  define ZQ_INTERNAL_FILE_NAME      	"CMEV2_d"
#else
#  define ZQ_INTERNAL_FILE_NAME      	"CMEV2"
#endif // _DEBUG
#define ZQ_FILE_NAME               	ZQ_INTERNAL_FILE_NAME ".exe"

#endif // __ZQRESOURCE_H__
