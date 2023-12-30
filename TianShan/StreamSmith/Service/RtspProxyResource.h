#ifndef __RTSPPROXYRESOURCE_H__
#define __RTSPPROXYRESOURCE_H__

#define ZQ_PRODUCT_NAME				"TianShan Component RtspProxy Service"

#include "../../../build/ZQSrcTreeVer.h" // defines the version number of the source tree

#ifdef ZQ_PRODUCT_NAME
#undef ZQ_PRODUCT_NAME
#endif

#define ZQ_PRODUCT_NAME			"TianShan Component RtspProxy Service"

#define ISA_TARGET_ENV          	VOD_VERSION_DESC "; ITV " ITV_VERSION

// the following section are static per-project
#define ZQ_FILE_DESCRIPTION        	" RtspProxy Service"
#ifdef _DEBUG
#define ZQ_INTERNAL_FILE_NAME      	"RtspProxy_d"
#else
#define ZQ_INTERNAL_FILE_NAME      	"RtspProxy"
#endif // _DEBUG
#define ZQ_FILE_NAME               	ZQ_INTERNAL_FILE_NAME ".exe"

#define ZQ_PRODUCT_COMMENT          	ZQ_PRODUCT_NAME " " ZQ_PRODUCT_VER_STR3

#endif // __RTSPPROXYRESOURCE_H__
