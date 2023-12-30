#ifndef __ZQRESOURCE_H__
#define __ZQRESOURCE_H__

#define ZQ_PRODUCT_NAME			"ZQ TianShan"

#include "../../../build/ZQSrcTreeVer.h" // defines the version number of the source tree

#ifdef ZQ_PRODUCT_NAME
#  undef ZQ_PRODUCT_NAME
#endif

#define ZQ_PRODUCT_NAME			"ZQ TianShan"

// the following section are static per-project
#define ZQ_FILE_DESCRIPTION        	"ZQ TianShan SNMP Extension Agent"

#ifdef _DEBUG
#  define ZQ_INTERNAL_FILE_NAME      	"ZQSNMPAgent_d"
#else
#  define ZQ_INTERNAL_FILE_NAME      	"ZQSNMPAgent"
#endif // _DEBUG
#define ZQ_FILE_NAME               	ZQ_INTERNAL_FILE_NAME ".dll"

#endif // __ZQRESOURCE_H__
