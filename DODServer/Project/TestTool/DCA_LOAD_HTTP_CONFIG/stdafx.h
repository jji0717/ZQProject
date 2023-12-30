// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
/*
 
Revision History:

Rev			   Date        Who			Description
----		---------    ------		----------------------------------
V1,0,0,8	2006.02.28	zhenan_ji   1.create class(CCreateILP) for nav , pot , pos 
										2.modify ConfDefine_DOD.xml : add SendMsgDataType and SendMsgExpiredTime
										3.when nav update,the ILP file will create in the msg_channel_dir,
										  so the msg will update,too.
V1,0,0,8	2006.03.02	zhenan_ji   1.By jerroy request,if groupID is zero,the channel dateType verily is exclusive, So mark compare about GroupID
V1,0,0,9	2006.03.07	zhenan_ji   1.when DCA starting ,if current channel is msg channel,DCA can not send msg to the channel.
V1,0,1,0	2006.03.15	zhenan_ji   1.Modified ILP.Format "1"---change"0"
V1,0,1,1	2006.05.15	zhenan_ji   1.Modified RemoveDir fun(), add check and error log.
V1,0,1,3	2006.06.08	zhenan_ji   1.fix bug:if current channel type is localfile_format.It will don't process received message that is a same dataType.		
V1,0,1,4	2006.06.10	zhenan_ji   1.Modified channel dataType from int to string by ";" interver.Sample "18;19"; 
										For new feature: if message dataType has multiple datatype ,this message data will be put into a same subfolder.
V1,0,1,5	2006.06.11	zhenan_ji   1.fix bug: compare result about dataType;
V1,0,1,6	2006.06.19	zhenan_ji   1.fix bug: http://192.168.80.12/show_bug.cgi?id=3323 ; The same dataType was set in mutil_group ,
											put message data to a group about groupID come from message body,then put message to group that groupID is zero.
V1,0,1,6	2006.06.20	zhenan_ji   1.remove log:if groupid is zero ,this group was not config.this log should not write to logfile.
V1,0,1,7	2006.06.21	zhenan_ji   1.To SendmsgDataType in sharefolder, It's processing is the same as the message format.
V1,0,1,8	2006.06.22	zhenan_ji   1.Add updateMode is 6 for sharefolder,by xia.zhang 's request about navigation.
V1,0,1,9	2006.07.04	zhenan_ji   1.Old messageID of a message is integer. Now,it have changed to cstring. 20060606,hefei site occurs this overflow.
									2.Add new feature: "FirstDataTypeMsg" with DCA service start.
										In practice configuration, the same dataType of channel often was setting many DOD’s port (A DOD’s port is a node group). 
										In DOD configuration, we add a parameter; its name is ‘Data type with initial’. 
										This parameter shows which channel‘s data type needs send the below message to relative application. 
										If DCA service encounter the first data type in many DOD’s port, DCA will sends this message to Application by Command type
V1,0,2,0	2006.07.10	zhenan_ji   1.Add try and catch for push operation of Vector..

V1,0,2,1	2006.07.26	zhenan_ji   1.change port number limited about portinfo struct to list;
									2.print share folder log;
									3,print active log in a thread of each channel and a thread of portManager
									4,To "Out of msg " print this content of msg;									
									5,The latest jmscpp.dll don't applied this project.
	


*/										
#include <afx.h>
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#include <afxole.h>         // MFC OLE 类
#include <afxodlgs.h>       // MFC OLE 对话框类
#include <afxdisp.h>        // MFC 自动化类

#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include<list>
#import "msxml3.dll"
//#include <atlbase.h>
//#include <tchar.h>

#include"clog.h"
//#include <iostream>

#define LOG_NORECORD 0
#define LOG_ERROR	 1
#define LOG_RELEASE	 2
#define LOG_DEBUG	 3
#define LOG_WARNING	 4
#define LOG_INFO	 5
#define LOG_DETAIL	 6


#define DCADERRORCODE	0X30010
#define DODJMSINITIATIONERROR			DCADERRORCODE+1	// JMS 's initiation value is negative
#define DODRECEIVERDATAFOLDERERROR		DCADERRORCODE+2	// receiveddatafold, xml message error!
#define DODRECEIVERDATATCPERROR			DCADERRORCODE+3	// receiveddataTCP, xml message error!
#define DODRECEIVERPORTCONFIGMSGERROR	DCADERRORCODE+4	// receiveddataconfig,xml message error!
#define DODReceiverUpDataTCPRROR		DCADERRORCODE+5	// receiveupdateTCP, xml message error!
#define DODReceiverUpDataTCPERROR		DCADERRORCODE+6	// receiveupdatefold, xml message error! 
#define DODSendConfigRequestERROR		DCADERRORCODE+7	// sendconfigurationrequest, the operation is not respondence! 
#define DODsendgetfulldataERROR			DCADERRORCODE+8	// sendgetfull, the operation is not respondence! 
#define DODcreatetempdirERROR			DCADERRORCODE+9	// when receive configuration message, create tempdirectory error
#define DODcreatetempfileERROR			DCADERRORCODE+10// when receive configuration message, create tempfile error! 


// DOD send message_code

#define PORTCONFIGURATIONREQUEST	3001	//Port Configuration request
#define GETFULLDATA					3002	//GetFullData ,request send data

// DOD received message_code

#define PORTCONFIGURATION			3003	//Port Configuration
#define FULLDATAWITHSHAREDFOLDERS	3004	//FullData with shared Folders
#define FULLDATAWITHTCP				3005	//FullData with TCP
#define UPDATEINSHAREFOLDERMODE		3006	//Channel Data Update in Share-Folder mode
#define UPDATEINTCPMODE				3007	//Channel Data Update in TCP mode
#define FIRSTDATATYPEMESSAGE		3008	//FirstDataTypeMsg with DCA service start 
#define RETUENMESSAGEABOUT3008		3009	//Return message about MSG_3008 


// DOD send message_type
#define PORTCONFIGURATIONREQUEST   3001

#define REALPORTCONFIGURATIONREQUEST   1515


// TODO: reference additional headers your program requires here
