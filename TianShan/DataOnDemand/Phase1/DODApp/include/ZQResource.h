#ifndef __ZQRESOURCE_H__                                                                                  
#define __ZQRESOURCE_H__                                                                                  
                                                                                                          
// the following section will be replaced with the real value by the ZQAutoBuild process                  
#define ZQ_PRODUCT_VER_MAJOR		0                                                                 
#define ZQ_PRODUCT_VER_MINOR		1                                                                 
#define ZQ_PRODUCT_VER_PATCH		0                                                                 
#define ZQ_PRODUCT_VER_BUILD		0                                                               
#define ZQ_PRODUCT_VER_STR1		"0.1.0.0"                                                       
#define ZQ_PRODUCT_VER_STR2		"0,1,0,0"                                                       
#define ZQ_PRODUCT_VER_STR3		"V0.1.0 (build 0)"                                              
                                                                                                          
// the following section are static per-project                                                           
#define ZQ_PRODUCT_NAME			"JMSCpp"                                                
#define ZQ_FILE_DESCRIPTION        	"JMS C++ lib"                                 
#ifdef _DEBUG                                                                                             
#define ZQ_FILE_NAME               	"JMSCpp_d.lib"                                                   
#define ZQ_INTERNAL_FILE_NAME      	"JMSCpp"                                                       
#else                                                                                                     
#define ZQ_FILE_NAME               	"JMSCpp.lib"                                                     
#define ZQ_INTERNAL_FILE_NAME      	"JMSCpp"                                                         
#endif                                                                                                    
                                                                                                          
// the following section are static per-project, but you can define many SDK involved                     
#include "VODVersion.h"                                                                                   
#define ZQ_PRODUCT_COMMENT          	ZQ_PRODUCT_NAME " " ZQ_PRODUCT_VER_STR3 " for: " VOD_VERSION_DESC 
                                                                                                          
#endif // __ZQRESOURCE_H__                                                                                
