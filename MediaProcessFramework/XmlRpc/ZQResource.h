
#ifndef __ZQRESOURCE_H__
#define __ZQRESOURCE_H__

// the following section will be replaced with the real value by the ZQAutoBuild process
#define ZQ_PRODUCT_VER_MAJOR		0
#define ZQ_PRODUCT_VER_MINOR		7
#define ZQ_PRODUCT_VER_PATCH		0
#define ZQ_PRODUCT_VER_BUILD		1
#define ZQ_PRODUCT_VER_STR1		"0.7.0.1"
#define ZQ_PRODUCT_VER_STR2		"0,7,0,1"
#define ZQ_PRODUCT_VER_STR3		"V0.7.0 (build 1)"

// the following section are static per-project
#define ZQ_PRODUCT_NAME			"ZQXMLRPC"
#define ZQ_FILE_DESCRIPTION        	"ZQXMLRPC"
#ifdef _DEBUG
#define ZQ_FILE_NAME               	"XmlRpc.dll"
#define ZQ_INTERNAL_FILE_NAME      	"XMLRPC"
#else
#define ZQ_FILE_NAME               	"XmlRpc.dll"
#define ZQ_INTERNAL_FILE_NAME      	"XMLRPC"
#endif

// the following section are static per-project, but you can define many SDK involved
#define ZQ_PRODUCT_COMMENT          	ZQ_PRODUCT_NAME " " ZQ_PRODUCT_VER_STR3 

#endif // __ZQRESOURCE_H__
