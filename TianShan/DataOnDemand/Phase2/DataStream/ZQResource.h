#ifndef SEARESOURCE_H_
#define SEARESOURCE_H_

#define ZQ_PRODUCT_NAME				"DataStream"
#include "../../../../build/ZQSrcTreeVer.h" // defines the version number of the source tree

// the following section are static per-project
#define ZQ_FILE_DESCRIPTION        	"SeaChange DataOnDemand DataStream"
#ifdef _DEBUG
#define ZQ_INTERNAL_FILE_NAME      	"DataStream_d"
#else
#define ZQ_INTERNAL_FILE_NAME      	"DataStream"
#endif // _DEBUG
#define ZQ_FILE_NAME               	ZQ_INTERNAL_FILE_NAME ".exe"

#endif



