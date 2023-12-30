
#ifndef __ZQRESOURCE_H__
#define __ZQRESOURCE_H__

#define __N2S2__(x) #x
#define __N2S__(x) __N2S2__(x)

#define ZQ_PRODUCT_NAME			"C2Locator Web Controller"
#define ZQ_PRODUCT_NAME_SHORT	"c2loc_web"


#include "../../../../build/ZQSrcTreeVer.h" // defines the version number of the source tree

#define ZQ_FILE_DESCRIPTION     "Web management module of C2 Locate service"
#define ZQ_INTERNAL_FILE_NAME   "c2loc_web"
#define ZQ_FILE_NAME            ZQ_INTERNAL_FILE_NAME ".dll"

// the following section are static per-project, but you can define many SDK involved
#define ZQ_PRODUCT_COMMENT          	ZQ_FILE_DESCRIPTION " " ZQ_PRODUCT_VER_STR3
#endif // __ZQRESOURCE_H__


