#ifndef __ZQRESOURCE_H__
#define __ZQRESOURCE_H__

#define ZQ_PRODUCT_NAME			"XOR-Media Aqua ClientCLR"

#define __N2S2__(x) #x
#define __N2S__(x) __N2S2__(x)
// the following section will be replaced with the real value by the ZQAutoBuild process
#define ZQ_PRODUCT_VER_MAJOR		1
#define ZQ_PRODUCT_VER_MINOR		7
#define ZQ_PRODUCT_VER_PATCH		0
#define ZQ_PRODUCT_VER_BUILD		1

// the following section are static per-project
#define ZQ_FILE_DESCRIPTION        	ZQ_PRODUCT_NAME ": AquaClientCLR"
#ifdef _DEBUG
#define ZQ_INTERNAL_FILE_NAME      	"AquaClientCLR_d"
#else
#define ZQ_INTERNAL_FILE_NAME      	"AquaClientCLR"
#endif // _DEBUG
#define ZQ_FILE_NAME               	ZQ_INTERNAL_FILE_NAME ".dll"

// the following section are static per-project, but you can define many SDK involved
#define ZQ_PRODUCT_COMMENT          	ZQ_FILE_DESCRIPTION " " ZQ_PRODUCT_VER_STR3

#endif // __ZQRESOURCE_H__
