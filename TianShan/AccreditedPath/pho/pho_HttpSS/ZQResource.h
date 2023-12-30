#ifndef __ZQRESOURCE_H__
#define __ZQRESOURCE_H__

#define ZQ_PRODUCT_NAME			"XOR-media TianShan Architecture"

#include "../../../../build/ZQSrcTreeVer.h" // defines the version number of the source tree

// the following section are static per-project
#define ZQ_FILE_DESCRIPTION        	ZQ_PRODUCT_NAME ": Path Helper Object for C2Stream"
#ifdef _DEBUG
#define ZQ_INTERNAL_FILE_NAME      	"pho_HttpSS_d"
#else
#define ZQ_INTERNAL_FILE_NAME      	"pho_HttpSS"
#endif // _DEBUG
#define ZQ_FILE_NAME               	ZQ_INTERNAL_FILE_NAME ".dll"

// the following section are static per-project, but you can define many SDK involved
#define ZQ_PRODUCT_COMMENT          	ZQ_FILE_DESCRIPTION " " ZQ_PRODUCT_VER_STR3 "\n Supports HTTP C2Stream"

#endif // __ZQRESOURCE_H__
