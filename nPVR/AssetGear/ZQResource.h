#ifndef __ZQRESOURCE_H__
#define __ZQRESOURCE_H__

#include "nPVRVersion.h"

// the following section are static per-project
#define ZQ_FILE_DESCRIPTION        	"SeaChange AssetGear Service"
#ifdef _DEBUG
#define ZQ_INTERNAL_FILE_NAME      	"AssetGear_d"
#else
#define ZQ_INTERNAL_FILE_NAME      	"AssetGear"
#endif // _DEBUG
#define ZQ_FILE_NAME               	ZQ_INTERNAL_FILE_NAME ".exe"

// the following section are static per-project, but you can define many SDK involved
#include "VODVersion.h"


#define ZQ_PRODUCT_COMMENT          	ZQ_PRODUCT_NAME " " ZQ_PRODUCT_VER_STR3 " for " VOD_VERSION_DESC

#endif // __ZQRESOURCE_H__
