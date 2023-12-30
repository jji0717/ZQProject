#ifndef __ZQRESOURCE_H__
#define __ZQRESOURCE_H__

#define __N2S2__(x) #x
#define __N2S__(x) __N2S2__(x)

//the following section will be replaced with the real value by the ZQAutoBuild process
#define ZQ_PRODUCT_VER_MAJOR		1
#define ZQ_PRODUCT_VER_MINOR		0
#define ZQ_PRODUCT_VER_PATCH		0
#define ZQ_PRODUCT_VER_BUILD		0
#define ZQ_PRODUCT_NAME			"CRM_3dServer.dll" " " __N2S__(ZQ_PRODUCT_VER_MAJOR) "." __N2S__(ZQ_PRODUCT_VER_MINOR) "." __N2S__(ZQ_PRODUCT_VER_PATCH) "." __N2S__(ZQ_PRODUCT_VER_BUILD)
#define ZQ_PRODUCT_NAME_SHORT	"CRM_3dServer" " " __N2S__(ZQ_PRODUCT_VER_MAJOR) "." __N2S__(ZQ_PRODUCT_VER_MINOR)
#define ZQ_TIANSHAN_VERSION		"TianShan " __N2S__(ZQ_PRODUCT_VER_MAJOR) "." __N2S__(ZQ_PRODUCT_VER_MINOR)
#define ZQ_FILE_DESCRIPTION     "CRM_3dServer.dll"
#define ZQ_INTERNAL_FILE_NAME   "CRM_3dServer"
#define ZQ_FILE_NAME            "CRM_3dServer.dll"
#define ZQ_PRODUCT_COMMENT      ZQ_PRODUCT_NAME

#endif // __ZQRESOURCE_H__


