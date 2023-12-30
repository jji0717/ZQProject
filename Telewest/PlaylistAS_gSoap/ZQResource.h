#ifndef __ZQRESOURCE_H__
#define __ZQRESOURCE_H__

// the following section will be replaced with the real value by the ZQAutoBuild process
#define ZQ_PRODUCT_VER_MAJOR		1
#define ZQ_PRODUCT_VER_MINOR		0
#define ZQ_PRODUCT_VER_PATCH		8
#define ZQ_PRODUCT_VER_BUILD		2
#define ZQ_PRODUCT_VER_STR1		"1.0.8.2"
#define ZQ_PRODUCT_VER_STR2		"1,0,8,2"
#define ZQ_PRODUCT_VER_STR3		"V1.0.8 (build 2)"

// the following section are static per-project
#define ZQ_PRODUCT_NAME			"Telewest VOD"
#define ZQ_FILE_DESCRIPTION        	"Service executable for PlaylistAS"
#ifdef _DEBUG
#define ZQ_FILE_NAME               	"PlaylistAS_d.exe"
#define ZQ_INTERNAL_FILE_NAME      	"PlaylistAS_d"
#else
#define ZQ_FILE_NAME               	"PlaylistAS.exe"
#define ZQ_INTERNAL_FILE_NAME      	"PlaylistAS"
#endif

// the following section are static per-project, but you can define many SDK involved
#include "VODVersion.h"
#include "gSoap_version.h"
#define ZQ_PRODUCT_COMMENT          	ZQ_PRODUCT_NAME " " ZQ_PRODUCT_VER_STR3 " for: " VOD_VERSION_DESC " and " GSOAP_VERSION_DESC

#endif // __ZQRESOURCE_H__




