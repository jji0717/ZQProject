/********************************************************************************************
* MOD-NAME      : SharedMemory.h
* LONG-NAME     : 
*
* AUTHOR        : Tony
* DEPARTMENT    : 
* TELEPHONE     : 
* CREATION-DATE : 
* SP-NO         : 
* FUNCTION      : 
* 
*********************************************************************************************/
#ifndef __XMLFILEOPER_H__
#define __XMLFILEOPER_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include "DataStruct.h"
#define SERVICE_NAME_LEN    80
#define VAR_DATA_LENGTH     180    

// FOR THE XMLFILEOPER
#define SERVICENAME_LEN     SERVICE_NAME_LEN
#define FILENAME_LEN        256

#define XML_SNMP_BEGIN			"SnmpExtensionXML"
#define XML_SNMP_INFO			"info"

#define XML_SNMP_SUPPORTOID  	"Supported_OID"
#define XML_SNMP_VER            "Version"
#define XML_SNMP_COMPANYOID     "1.3.6.1.4.1.2006.6.2"
#define XML_SNMP_SERVICE        "service"
#define XML_SNMP_SERVICEDATA    "service_data"
#define XML_SNMP_NAME           "Name"
#define XML_SNMP_VAR            "variable"
#define XML_SNMP_OID            "OID"
#define XML_SNMP_TYPE           "Type"
#define XML_SNMP_ACCESS         "Access"
#define XML_SNMP_VALUE          "Value"


#include "ZQSNMPManPkg.h"
#include "XMLPreference.h"

class CXMLFileOper  
{
public:
	CXMLFileOper();
	virtual ~CXMLFileOper();
public:
	void initXMLDoc();
	void UninitXMLDoc();
	BOOL CreateXMLFile(TCHAR *szDirectory,TCHAR *szCompanyOID,TCHAR *szXMLFileName);
	BOOL SetItemDataToXML(TCHAR *szServiceName,TCHAR *szVarName,TCHAR *szNewValue,TCHAR *szOID = NULL,WORD wType= 10,int iReadOnly = 2);
	BOOL WriteToXML(TCHAR *szServiceName,TCHAR *szVarName,TCHAR *szOID, WORD wType,BOOL bReadOnly, TCHAR *szVarValue);
	BOOL RemoveFromXML(TCHAR *szServiceName,TCHAR *szVarName);
	ZQ::common::IPreference * FindItemIprefByVarName(TCHAR *szServiceName,TCHAR *szVarName);
	ZQ::common::IPreference*  FindItemIprefByServiceName(TCHAR *szServiceName);
protected:
	char m_szXMLFileName[FILENAME_LEN];
	ZQ::common::XMLPrefDoc* m_XMLDoc;
	ZQ::common::ComInitializer* m_comInit;
};

#endif 
