#ifndef __ZQRESOURCE_H__
#define __ZQRESOURCE_H__

#include "../mpfversion.h"

// the following section are static per-project
#define ZQ_FILE_DESCRIPTION        	"Library for Media Process Framework work node"
#ifdef _DEBUG
#define ZQ_FILE_NAME               	"WorkNode_d.dll"
#define ZQ_INTERNAL_FILE_NAME      	"WorkNode_d"
#else
#define ZQ_FILE_NAME               	"WorkNode.dll"
#define ZQ_INTERNAL_FILE_NAME      	"WorkNode"
#endif

// the following section are static per-project, but you can define many SDK involved
#define ZQ_PRODUCT_COMMENT          ZQ_PRODUCT_NAME " " ZQ_PRODUCT_VER_STR3 " "

#endif // __ZQRESOURCE_H__




