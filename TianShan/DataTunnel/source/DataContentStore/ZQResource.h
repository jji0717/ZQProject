#ifndef __ZQRESOURCE_H__
#define __ZQRESOURCE_H__

// the following section will be replaced with the real value by the ZQAutoBuild process
#define ZQ_PRODUCT_VER_MAJOR		1
#define ZQ_PRODUCT_VER_MINOR		0
#define ZQ_PRODUCT_VER_PATCH		1
#define ZQ_PRODUCT_VER_BUILD		0
#define ZQ_PRODUCT_VER_STR1		"1.0.1.0"
#define ZQ_PRODUCT_VER_STR2		"1,0,1,0"
#define ZQ_PRODUCT_VER_STR3		"V1.0.1 (build 0)"

// the following section are static per-project
#define ZQ_PRODUCT_NAME			"SeaChange DataOnDemand ContentStore"
#define ZQ_FILE_DESCRIPTION        	"DODContentStore"
#ifdef _DEBUG
#define ZQ_FILE_NAME               	"DODContentStore_d.exe"
#define ZQ_INTERNAL_FILE_NAME      	"DODContentStore_d"
#else
#define ZQ_FILE_NAME               	"DODContentStore.exe"
#define ZQ_INTERNAL_FILE_NAME      	"DODContentStore"
#endif

// the following section are static per-project, but you can define many SDK involved
#define ZQ_PRODUCT_COMMENT          	ZQ_PRODUCT_NAME " " ZQ_PRODUCT_VER_STR3
#endif // __ZQRESOURCE_H__




