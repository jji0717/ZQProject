#ifndef __ZQRESOURCE_H__
#define __ZQRESOURCE_H__

#define ZQ_PRODUCT_VER_MAJOR		1
#define ZQ_PRODUCT_VER_MINOR		0
#define ZQ_PRODUCT_VER_PATCH		0
#define ZQ_PRODUCT_VER_BUILD		1
#define ZQ_PRODUCT_VER_STR1			"1.0.0.1"
#define ZQ_PRODUCT_VER_STR2			"1.0.0.1"
#define ZQ_PRODUCT_VER_STR3			"V1.0.0 (build 1)"
#define ZQ_PRODUCT_NAME				"ZQ TianShan"


// the following section are static per-project
#define ZQ_FILE_DESCRIPTION        	"ZQ A3Message"
#ifdef _DEBUG
#define ZQ_INTERNAL_FILE_NAME      	"A3Message_d"
#else
#define ZQ_INTERNAL_FILE_NAME      	"A3Message"
#endif // _DEBUG
#define ZQ_FILE_NAME               	ZQ_INTERNAL_FILE_NAME ".dll"

#define ZQ_PRODUCT_COMMENT          	ZQ_PRODUCT_NAME " " ZQ_PRODUCT_VER_STR3

#endif // __ZQRESOURCE_H__
