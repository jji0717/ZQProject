
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

#define LOG_NORECORD 0
#define LOG_ERR		 1
#define LOG_RELEASE	 2
#define LOG_DEBUG	 3

#define LOGFILE_SIZE	(64*1024*1024)

#define PACKET_VERSION (0x00000909)

//define message flag
#define MESSAGEFLAG_REQUEST		0
#define MESSAGEFLAG_REPLY			1

///////////////////////////////////////////////////////////////////////////////////////////////////

#define MAX_CONNECTION		10

#define MESSAGECODE_PORTERROR		(5032)
#define MESSAGECODE_PORTREPORT		(5015)
#define MESSAGECODE_HANDSHAKE		(5030)
#define MESSAGECODE_CONTROLLERERROR	(5031)

#define MEDIAPUMP_NOERROR               0x0000		///< No Error

// app type as source flag when it is sent to playback server.
#define IP_CONTROLLER_APP_TYPE		(0x04)

//define error code
//#define ERRORCODE_NORMAL				0x0000
//#define ERRORCODE_NOERROR				0000
//#define error code in DeviceInfo Server from 7000-8000
#define ERRORCODE_DEVICEINFO_CANNOTOPENVSTRM	7000
#define ERRORCODE_DEVICEINFO_CANNOTCONTECTAM	7001
#define ERRORCODE_DEVICEINFO_NOFILEFINDED		7002
#define ERRORCODE_BINDFAIL						7003

#define ERRORCODE_NOERROR				0x10000
#define ERRORCODE_DISCONNECT			0x10001
#define ERRORCODE_HANDSHAKE				0x10002
#define ERRORCODE_PORTERROR				0x10004
#define ERRORCODE_CONTROLLERERROR		0x10008
//-------------liqing----------

extern TCHAR g_szRichText[];
extern TCHAR g_szErrorText[];
extern TCHAR g_szControllerStatusText[];

//-----------------------------
#endif