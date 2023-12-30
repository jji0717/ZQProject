#ifndef __ZQTianShan_TSPUMP_RESOURCE_H__
#define __ZQTianShan_TSPUMP_RESOURCE_H__

#define ZQ_PRODUCT_NAME			"TianShan Architecture"

// the following section will be replaced with the real value by the ZQAutoBuild process
#define ZQ_PRODUCT_VER_MAJOR		1
#define ZQ_PRODUCT_VER_MINOR		0
#define ZQ_PRODUCT_VER_PATCH		0
#define ZQ_PRODUCT_VER_BUILD		1

// the following section are static per-project
#define ZQ_FILE_DESCRIPTION        	ZQ_PRODUCT_NAME ": TSPump application service"
#ifdef _DEBUG
#define ZQ_INTERNAL_FILE_NAME      	"TSPump"
#else
#define ZQ_INTERNAL_FILE_NAME      	"TSPump"
#endif // _DEBUG
#define ZQ_FILE_NAME               	ZQ_INTERNAL_FILE_NAME ".exe"

// the following section are static per-project, but you can define many SDK involved
#define ZQ_PRODUCT_COMMENT          	ZQ_FILE_DESCRIPTION " " ZQ_PRODUCT_VER_STR3

#endif // __ZQTianShan_TSPUMP_RESOURCE_H__

