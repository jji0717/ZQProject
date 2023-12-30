#ifndef __ZQRESOURCE_H__
#define __ZQRESOURCE_H__

#define ZQ_PRODUCT_VER_MAJOR		2
#define ZQ_PRODUCT_VER_MINOR		10
#define ZQ_PRODUCT_VER_PATCH		0
#define ZQ_PRODUCT_VER_BUILD		1
#define ZQ_PRODUCT_VER_STR1			"2.10.0.1"
#define ZQ_PRODUCT_VER_STR2			"2,10,0,1"
#define ZQ_PRODUCT_VER_STR3			"V2.10.0 (build 1)"
#define ZQ_PRODUCT_NAME				"SeaChange DOD DCA Service"


#ifdef ZQ_PRODUCT_NAME
#undef ZQ_PRODUCT_NAME
#endif

#define ZQ_PRODUCT_NAME			"SeaChange DOD DCA Service"

#define ISA_TARGET_ENV          	VOD_VERSION_DESC "; ITV " ITV_VERSION

// the following section are static per-project
#define ZQ_FILE_DESCRIPTION        	"SeaChange DOD DCA Service"
#ifdef _DEBUG
#define ZQ_INTERNAL_FILE_NAME      	"DCA_d"
#else
#define ZQ_INTERNAL_FILE_NAME      	"DCA"
#endif // _DEBUG
#define ZQ_FILE_NAME               	ZQ_INTERNAL_FILE_NAME ".exe"

#define ZQ_PRODUCT_COMMENT          	ZQ_PRODUCT_NAME " " ZQ_PRODUCT_VER_STR3

#endif // __ZQRESOURCE_H__
