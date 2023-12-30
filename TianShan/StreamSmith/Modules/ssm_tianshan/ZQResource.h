#ifndef __ZQRESOURCE_H__
#define __ZQRESOURCE_H__

#ifndef _RTSP_PROXY

#include "../../../../build/ZQSrcTreeVer.h" // defines the version number of the source tree

#define ZQ_PRODUCT_VER_STR1			"1.0.1.0"
#define ZQ_PRODUCT_VER_STR2			"1,0,1,0"
#define ZQ_PRODUCT_VER_STR3			"V1.0.1 (build )"
#define ZQ_PRODUCT_NAME				"StreamSmith Service Tianshan edge plugin"


	#ifdef ZQ_PRODUCT_NAME
		#undef ZQ_PRODUCT_NAME
	#endif

#define ZQ_PRODUCT_NAME			"StreamSmith Service plugin"

// the following section are static per-project
#define ZQ_FILE_DESCRIPTION        	"SeaChange ITV Components StreamSmith Service plugin"
	#ifdef _DEBUG
		#define ZQ_INTERNAL_FILE_NAME      	"ssm_tianshan_d"
	#else
		#define ZQ_INTERNAL_FILE_NAME      	"ssm_tianshan"
	#endif // _DEBUG

#define ZQ_FILE_NAME               	ZQ_INTERNAL_FILE_NAME ".dll"

#define ISA_TARGET_ENV          	VOD_VERSION_DESC "; ITV " ITV_VERSION
// the following section are static per-project, but you can define many SDK involved


#define ZQ_PRODUCT_COMMENT          	ZQ_PRODUCT_NAME " " ZQ_PRODUCT_VER_STR3 

#else // #ifndef _RTSP_PROXY
	#include "RtspProxyResource.h"
#endif // #ifndef _RTSP_PROXY

#endif // __ZQRESOURCE_H__
