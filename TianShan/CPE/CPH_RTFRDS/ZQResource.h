#ifndef __ZQRESOURCE_H__
#define __ZQRESOURCE_H__

#define ZQ_PRODUCT_NAME			"SeaChange TianShan Architecture"

#include "../../../build/ZQSrcTreeVer.h" // defines the version number of the source tree

// the following section are static per-project
#define ZQ_FILE_DESCRIPTION        	ZQ_PRODUCT_NAME ": ContentProvisionHelper RDSvsVstrm module"
#ifdef _DEBUG
#define ZQ_INTERNAL_FILE_NAME      	"CPH_RDS_d"
#else
#define ZQ_INTERNAL_FILE_NAME      	"CPH_RDS"
#endif // _DEBUG
#define ZQ_FILE_NAME               	ZQ_INTERNAL_FILE_NAME ".exe"

// the following section are static per-project, but you can define many SDK involved
#include <TrickLibVersion.h>

#define __xN2S2__(x) #x
#define __xN2S__(x) __xN2S2__(x)
#define ZQ_PRODUCT_COMMENT          ZQ_FILE_DESCRIPTION " " ZQ_PRODUCT_VER_STR3 " w/ " __xN2S__(TrickLIB_TYPE) " " __xN2S__(TrickLIB_PRODUCTVERSION )

#endif // __ZQRESOURCE_H__
