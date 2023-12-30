#ifndef __ZQRESOURCE_H__                                                                                 
#define __ZQRESOURCE_H__                                                                                 
                                                                                                         
// the following section will be replaced with the real value by the ZQAutoBuild process                 
#define ZQ_PRODUCT_VER_MAJOR  	3	// updated by autobuild process @ 2006-10-27 16:55
#define ZQ_PRODUCT_VER_MINOR  	6	// updated by autobuild process @ 2006-10-27 16:55
#define ZQ_PRODUCT_VER_PATCH  	5	// updated by autobuild process @ 2006-10-27 16:55
#define ZQ_PRODUCT_VER_BUILD  	5	// updated by autobuild process @ 2006-10-27 16:55
#define ZQ_PRODUCT_VER_STR1   	"3.6.5.5"	// updated by autobuild process @ 2006-10-27 16:55
#define ZQ_PRODUCT_VER_STR2   	"3,6,5,5"	// updated by autobuild process @ 2006-10-27 16:55

// please enable "_ENG_VERSION" when compiling english version
// notice that this preprocessor should be set both in 
// 1. [DBSyncServ_Eng]->[setting]->[C/C++]->[preprocessor]
// 2. [DBSync.rc]->[setting]->[resources]->[preprocessor]

//#if defined _ENG_VERSION
//#define ZQ_PRODUCT_VER_STR3		"V3.5.8 (build 1) English Version"                                             
//#else
#define ZQ_PRODUCT_VER_STR3   	"V3.6.5 (build 5)"	// updated by autobuild process @ 2006-10-27 16:55
//#endif                                                      

                                                                                                         
// the following section are static per-project                                                          
#define ZQ_PRODUCT_NAME			"DBSync"                                               
#define ZQ_FILE_DESCRIPTION        	"ZQ DBSync Service"                                
#ifdef _DEBUG                                                                                            
#define ZQ_FILE_NAME               	"DBSync_d.exe"                                                  
#define ZQ_INTERNAL_FILE_NAME      	"DBSync_d"                                                      
#else                                                                                                    
#define ZQ_FILE_NAME               	"DBSync.exe"                                                    
#define ZQ_INTERNAL_FILE_NAME      	"DBSync"                                                        
#endif                                                                                                   
                                                                                                         
// the following section are static per-project, but you can define many SDK involved                    
#include "VODVersion.h"                                                                                  
#define ZQ_PRODUCT_COMMENT          	ZQ_PRODUCT_NAME " " ZQ_PRODUCT_VER_STR3 " for: " VOD_VERSION_DESC
                                                                                                         
#endif // __ZQRESOURCE_H__                                                                               
