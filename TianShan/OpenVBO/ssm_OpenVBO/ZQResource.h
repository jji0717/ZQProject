
#ifndef __ZQRESOURCE_H__
#define __ZQRESOURCE_H__

#define __N2S2__(x) #x
#define __N2S__(x) __N2S2__(x)

#define ZQ_PRODUCT_NAME			"TianShan Architecture"


#include "../../../build/ZQSrcTreeVer.h" // defines the version number of the source tree


#define ZQ_PRODUCT_NAME_SHORT	"TianShan"
#define ZQ_FILE_DESCRIPTION     ZQ_PRODUCT_NAME_SHORT ": RTSP plugin for OpenVBO integration, support of eventIS VOD interface 5d"

#ifdef _DEBUG
#  define ZQ_INTERNAL_FILE_NAME   "ssm_OpenVBO_d"
#else
#  define ZQ_INTERNAL_FILE_NAME   "ssm_OpenVBO"
#endif // _DEBUG

#define ZQ_FILE_NAME            ZQ_INTERNAL_FILE_NAME ".dll"
#define ZQ_PRODUCT_COMMENT      ZQ_PRODUCT_NAME ZQ_PRODUCT_VER_STR3
#define ZQ_COMPONENT_NAME       ZQ_INTERNAL_FILE_NAME "/"  __N2S__(ZQ_PRODUCT_VER_MAJOR) "." __N2S__(ZQ_PRODUCT_VER_MINOR)



#endif // __ZQRESOURCE_H__


