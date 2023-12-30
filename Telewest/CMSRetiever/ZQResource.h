#ifndef __ZQRESOURCE_H__                                                                                  
#define __ZQRESOURCE_H__                                                                                  
                                                                                                          
// the following section will be replaced with the real value by the ZQAutoBuild process                  
#define ZQ_PRODUCT_VER_MAJOR		1                                                                 
#define ZQ_PRODUCT_VER_MINOR		0                                                                 
#define ZQ_PRODUCT_VER_PATCH		1                                                                 
#define ZQ_PRODUCT_VER_BUILD		0                                                               
#define ZQ_PRODUCT_VER_STR1		"1.0.1.0"                                                       
#define ZQ_PRODUCT_VER_STR2		"1,0,1,0"                                                       
#define ZQ_PRODUCT_VER_STR3		"V1.0.1 (build 1)"                                              
                                                                                                          
// the following section are static per-project                                                           
#define ZQ_PRODUCT_NAME			"SrvLoad"                                                
#define ZQ_FILE_DESCRIPTION        	"ZQ SrvLoad Service"                                 
#ifdef _DEBUG                                                                                             
#define ZQ_FILE_NAME               	"SrvLoad_d.exe"                                                   
#define ZQ_INTERNAL_FILE_NAME      	"SrvLoad_d"                                                       
#else                                                                                                     
#define ZQ_FILE_NAME               	"SrvLoad.exe"                                                     
#define ZQ_INTERNAL_FILE_NAME      	"SrvLoad"                                                         
#endif                                                                                                    
                                                                                                          
// the following section are static per-project, but you can define many SDK involved                     
#include "VODVersion.h"                                                                                   
#define ZQ_PRODUCT_COMMENT          	ZQ_PRODUCT_NAME " " ZQ_PRODUCT_VER_STR3 " for: " VOD_VERSION_DESC 
                                                                                                          
#endif // __ZQRESOURCE_H__                                                                                
