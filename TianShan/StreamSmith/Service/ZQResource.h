#ifndef __ZQRESOURCE_H__
#define __ZQRESOURCE_H__

#ifndef _RTSP_PROXY

	#define ZQ_PRODUCT_NAME				"ZQ TianShan StreamSmith Service"

	#include "../../../build/ZQSrcTreeVer.h" // defines the version number of the source tree


	#ifdef ZQ_PRODUCT_NAME
		#undef ZQ_PRODUCT_NAME
	#endif

	#define ZQ_PRODUCT_NAME			"ZQ TianShan StreamSmith Service"

	#define ISA_TARGET_ENV          	VOD_VERSION_DESC "; ITV " ITV_VERSION

	// the following section are static per-project
	#define ZQ_FILE_DESCRIPTION        	"ZQ TianShan Components StreamSmith Service"
	#ifdef _DEBUG
		#define ZQ_INTERNAL_FILE_NAME      	"StreamSmith_d"
	#else
		#define ZQ_INTERNAL_FILE_NAME      	"StreamSmith"
	#endif // _DEBUG
	#define ZQ_FILE_NAME               	ZQ_INTERNAL_FILE_NAME ".exe"

	// the following section are static per-project, but you can define many SDK involved	
	#include <vstrmver.h>


	#define __xN2S2__(x) #x
	#define __xN2S__(x) __xN2S2__(x)

	#define ZQ_PRODUCT_COMMENT          	ZQ_PRODUCT_NAME " " ZQ_PRODUCT_VER_STR3 " for SeaChange VstreamKit " __xN2S__(VER_PRODUCTVERSION_MAJOR) "." __xN2S__(VER_PRODUCTVERSION_MINOR) "." __xN2S__(VER_PRODUCTVERSION_ECO) " ==> " __BASELEVEL__

#else // #ifndef _RTSP_PROXY

	#include "RtspProxyResource.h"

#endif // #ifndef _RTSP_PROXY

#endif // __ZQRESOURCE_H__
