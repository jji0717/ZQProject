// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently

#if !defined(AFX_STDAFX_H__93972832_0F86_4698_B450_C3BF3FD08350__INCLUDED_)
#define AFX_STDAFX_H__93972832_0F86_4698_B450_C3BF3FD08350__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#include "ZQ_common_conf.h"
#include "XMLPreference.h"

#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;
#include <atlcom.h>
#include <atlctl.h>
#include <atlapp.h>

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlw.h>
#include <atlmisc.h>
#include <atlsplit.h>
#include <atlctrlx.h>
#include <atlcrack.h>
#include <atlwin.h>
#include <atlhost.h>

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <IceStorm/IceStorm.h>
#include <Freeze/Freeze.h>

#include <string>
using std::string;

#define XMLNAME_LEN  60
#define XMLDATA_LEN  200

#define SERVICENAME "ServiceSNMP"
#define TABNAME     "Tab"
#define VARNAME     "Name"
#define SERVICEOID  "OID"
#define CONFIG      "Config"
#define SNMPIP      "SnmpIp"
#define GRIDNAME    "Grid"
#define COLUMNNAME  "Column"
#define FEEDER      "Feeder"
#define DATASOURCE  "DataSource"
#define DSODATA     "DSO"
#define EVENTSOURCE "EventSource"
#define ICESTORM    "IceStorm"
#define SERVICE     "Service"
#define ENDPOINT    "EndPoint"
#define EVENTLOG    "EventLog"
#define FILENAME    "FileName"

typedef struct XMLFileData
{
	string  strServiceName;  //  Service Name's value
	string  strServiceOID;   //  Service oid's value
	string  strSnmpIp;       //  SnmpIP's value
//	string  strTabname;      //  Grid Tabname's value
	string  strGridFunc;     //  Feeder Func's value
	string  strFuncDso;      //  Feeder DSO's value
	string  strEventsFeeder; //  Events Feeder's value
	string  strEventsDso;    //  Events DSO's value
	string  strIceStormEndPoint;     //  IceStrom EndPoint
	string  strServcieEndPoint;  //  Service's endpoint
	string  strEventLogName; //  Event Log Filename's value
}XMLFILEDATA,*PXMLFILEDATA;
extern XMLFILEDATA  gXMLFileData;

extern HINSTANCE m_hLib;


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__93972832_0F86_4698_B450_C3BF3FD08350__INCLUDED)
