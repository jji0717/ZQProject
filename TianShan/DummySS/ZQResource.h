#ifndef __ZQRESOURCE_H__
#define __ZQRESOURCE_H__


	#define ZQ_PRODUCT_NAME				"ZQ TianShan Dummy StreamSmith Service"
	#include "../../build/ZQSrcTreeVer.h" // defines the version number of the source tree	

	#ifdef ZQ_PRODUCT_NAME
		#undef ZQ_PRODUCT_NAME
	#endif

	#define ZQ_PRODUCT_NAME			"ZQ TianShan DummySS Service"

	#define ISA_TARGET_ENV          	VOD_VERSION_DESC "; ITV " ITV_VERSION

	// the following section are static per-project
	#define ZQ_FILE_DESCRIPTION        	"ZQ TianShan Components DummySS Service"
	#ifdef _DEBUG
		#define ZQ_INTERNAL_FILE_NAME      	"DummySS_d"
	#else
		#define ZQ_INTERNAL_FILE_NAME      	"DummySS"
	#endif // _DEBUG
	#define ZQ_FILE_NAME               	ZQ_INTERNAL_FILE_NAME ".exe"

	// the following section are static per-project, but you can define many SDK involved	



	#define __xN2S2__(x) #x
	#define __xN2S__(x) __xN2S2__(x)

	#define ZQ_PRODUCT_COMMENT          	"DummySS for testing"

#endif // __ZQRESOURCE_H__
