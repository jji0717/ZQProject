/* */
// DVBSuite.h : 
//
/*****************************************************************************
File Name:		DVBSuite.h
Author:			Simin.Xue (Interactive ZQ) 
Security:		Confidential
Description:	This is a header file which define app data struct,const data,etc
Function Inventory:  	
Modification Log:
When		Version     Who			What
2004/7/15	1.0			Simin.Xue	Original Program
*****************************************************************************/

#ifndef DVBSUITE_H
#define DVBSUITE_H

//#include <atlcomtime.h>
//#include "ConnectManager.h"
//#include "clog.h"
#include <vector>
#include "clog.h"

//define log level for tracking
#define LOGLEVEL_INFO	 0
#define LOGLEVEL_ERROR	 1
#define LOGLEVEL_DEBUG	 2

//define log level for tracking
#define LOG_NORECORD 0
#define LOG_ERROR	 1
#define LOG_RELEASE	 2
#define LOG_DEBUG	 3

#define LOGFILE_SIZE	0x400000

//define APPTYPE is used for network communication between apps. 
#define APPTYPE_MANAGER	 			0               //Scheduling Manger
#define APPTYPE_PLAYBACKSERVER  	1				//Playback Server
#define APPTYPE_INGESTSERVER	  	2				//Ingest Server
#define APPTYPE_ROUTERSERVER	  	3				//Router Server
#define APPTYPE_IPCONTROLLER		4				//IP Streaming Controller
#define APPTYPE_VDCPCONTROLLER		5				//VDCP Controller
#define APPTYPE_ROUTERCONTROLLER	6				//Signal Router Server
#define APPTYPE_DEVICEINFO			7				//Device Info Server
#define APPTYPE_CONFIGURATION 		8				//Configuration Manager

//define PORTTYPE is used for related device port, every port has its own 
#define PORTTYPE_BMS_INGEST			0     //BMS channel for ingest
#define PORTTYPE_BMS_PLAYBACK		1     //BMS	channel for playback
#define PORTTYPE_IP_PUMP			2     //IP channel
#define PORTTYPE_RX_SEND			3     //RX card for sender

//define SCHEDULETYPE is used for schedule list
#define SCHEDULETYPE_STV			0
#define SCHEDULETYPE_NVOD			1
#define SCHEDULETYPE_INGEST			2

//define event type, phase I  for 0-2
#define EVENTTYPE_ONTIME		0            // Play independent
#define EVENTTYPE_DEPENDANT		1  	 // Play in turn
#define EVENTTYPE_UNCERTAIN		2  	// play uncertain


#define PACKET_VERSION (0x00000909)

#define ELEMENT_MESSAGEHEAD		"DVBSuite Header"
#define ELEMENT_SCHEDULEINFORMATION   "ScheduleInformation"
#define ELEMENT_CURDATE   "CurDate"
#define ELEMENT_DEVICEID   "DeviceID"
#define ELEMENT_PORTID   "PortID"



#define ELEMENT_MESSAGETIME		"MessageTime"
#define ELEMENT_MESSAGEFLAG		"MessageFlag"
#define ELEMENT_MESSAGEINDEX	"MessageIndex"
#define ELEMENT_MESSAGESOURCE	"MessageSource"
#define ELEMENT_MESSAGECODE		"MessageCode"
//define message flag
#define MESSAGEFLAG_REQUEST		0
#define MESSAGEFLAG_REPLY			1

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//---from ConnectManager.h-------
#define SC_RECEIVER_THREAD_OK		(0x00)
#define SC_RECEIVER_THREAD_ERROR	(0x01)

#define SC_SENDER_THREAD_OK		(0x00)
#define SC_SENDER_THREAD_ERROR	(0x01)

// --from ListenManager.h--
#define MAX_CONNECTION    10         //Listener can be connected by maxminum conector
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------liqing----------
#define TIMELENGTH		24
#define ASSERTCOUNT		500
#define ASSERTCOUNTALL	2000

#define NOTIFYCHANGE_FILE_NEW					1
#define NOTIFYCHANGE_FILE_CREATED				2
#define NOTIFYCHANGE_FILE_E_PRIVATE_DATA_CHANGE	3
#define NOTIFYCHANGE_FILE_RENAMED				4
#define NOTIFYCHANGE_FILE_DELETE_PENDING		5
#define NOTIFYCHANGE_FILE_DELETED				6


// Message code definition from playback server.
//#define MESSAGECODE_OPENPORT		(4000)
//#define MESSAGECODE_CLOSEPORT		(4002)
//#define MESSAGECODE_PLAYCUE			(5006)
//#define MESSAGECODE_PLAYCUEWITHDATA	(5007)
//#define MESSAGECODE_PLAY			(5009)
//#define MESSAGECODE_STOP			(5011)
//#define MESSAGECODE_PORTSTATUS		(5012)
#define MESSAGECODE_PORTERROR		(5032)
#define MESSAGECODE_PORTREPORT		(5015)
#define MESSAGECODE_HANDSHAKE		(5030)
#define MESSAGECODE_CONTROLLERERROR	(5031)

#define MEDIAPUMP_NOERROR               0x0000		///< No Error

// app type as source flag when it is sent to playback server.
#define IP_CONTROLLER_APP_TYPE		(0x04)

//define error code
//#define ERRORCODE_NORMAL				0x0000
#define ERRORCODE_NOERROR				0000
//#define error code in DeviceInfo Server from 7000-8000
#define ERRORCODE_DEVICEINFO_CANNOTOPENVSTRM	7000
#define ERRORCODE_DEVICEINFO_CANNOTCONTECTAM	7001
#define ERRORCODE_DEVICEINFO_NOFILEFINDED		7002
#define ERRORCODE_BINDFAIL						7003
//-------------liqing----------

typedef struct tagDeviceInfo{	// ListItem struct
	int  nIcon;				// relate icon
	LONG lDeviceID;
	LONG lTransportID;		//
	LONG lPortID;
	WORD wStatus;
	LONG lID;				//MAKEDEVICEID( lDeviceID, lTransportID, lPortID);
} DeviceInfo;

typedef std::vector<DeviceInfo> DeviceVector;
extern  DeviceVector vcDeviceItem;


extern TCHAR g_szRichText[];
extern TCHAR g_szErrorText[];
extern TCHAR g_szControllerStatusText[];

//-----------------------------
#endif