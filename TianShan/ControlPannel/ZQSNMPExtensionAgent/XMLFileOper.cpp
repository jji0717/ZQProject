// XMLFileOper.cpp: implementation of the CXMLFileOper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XMLFileOper.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void CXMLFileOper::initXMLDoc()
{
	m_comInit = new ZQ::common::ComInitializer();
	m_XMLDoc = new ZQ::common::XMLPrefDoc (*m_comInit);
	memset(m_szXMLFileName,0,sizeof(m_szXMLFileName));
}

void CXMLFileOper::UninitXMLDoc()
{
	if(m_comInit != NULL)
		delete m_comInit;
	m_comInit = NULL;

	//if(m_XMLDoc != NULL)
	//	delete m_XMLDoc;
	m_XMLDoc = NULL;
	
	// uninitialize com interface
	
	memset(m_szXMLFileName,0,sizeof(m_szXMLFileName));
}

CXMLFileOper::CXMLFileOper()
{
	initXMLDoc();
}

CXMLFileOper::~CXMLFileOper()
{
	UninitXMLDoc();
}

BOOL CXMLFileOper::CreateXMLFile(TCHAR *szDirectory,TCHAR *szCompanyOID,TCHAR *szXMLFileName)
{
	if ( szDirectory == NULL || szCompanyOID == NULL || szXMLFileName == NULL )
		return FALSE;
	
	TCHAR szFilePath[FILENAME_LEN];
	int iLen = _tcslen(szDirectory);
	if ( szDirectory[iLen-1]  !='\\' )
	{
		_tcscat(szDirectory,_T("\\"));
	}
	_stprintf(szFilePath,_T("%s%s%s"),szDirectory,szXMLFileName,_T(".xml"));

	char strFile[FILENAME_LEN] ={0};
	char strServiceName[SERVICENAME_LEN]={0};
	char strCompanyOID[VAR_DATA_LENGTH] ={0};

#if defined _UNICODE || defined UNICODE
	WideCharToMultiByte(CP_ACP,NULL,szFilePath,-1,strFile,sizeof(strFile),NULL,NULL);
	WideCharToMultiByte(CP_ACP,NULL,szXMLFileName,-1,strServiceName,sizeof(strServiceName),NULL,NULL);
	WideCharToMultiByte(CP_ACP,NULL,szCompanyOID,-1,strCompanyOID,sizeof(strCompanyOID),NULL,NULL);
#else
	sprintf(strFile,"%s",szFilePath);
	sprintf(strServiceName,"%s",szXMLFileName);
	sprintf(strCompanyOID,"%s",szCompanyOID);
#endif
	sprintf(m_szXMLFileName,"%s",strFile);

	BOOL bXmlDoc = FALSE;
	BOOL bCreate = TRUE;
	ZQ::common::IPreference* rootIpref;

	WIN32_FIND_DATA findData;
	memset(&findData, 0x0, sizeof(WIN32_FIND_DATA));
	HANDLE hFind = FindFirstFile((LPCTSTR)szFilePath, &findData);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);
		// open exist xml file
		try
		{
			bXmlDoc = m_XMLDoc->open(strFile);
		}
		catch(...)
		{
			bXmlDoc = FALSE;
		}

		if(bXmlDoc)
		{
			bCreate = FALSE;
		}
		else
		{	// open failed then delete the bad file
			if(!DeleteFile((LPCTSTR)szFilePath))
			{
				UninitXMLDoc();
			}
			bCreate = TRUE;
		}
	}
	if ( bCreate )
	{
		// open xml file with creation
		bXmlDoc = m_XMLDoc->open(strFile, XMLDOC_CREATE);
		rootIpref = m_XMLDoc->newElement(XML_SNMP_BEGIN);
		m_XMLDoc->set_root(rootIpref);

		ZQ::common::IPreference* itemIpref = NULL;
		ZQ::common::IPreference* itemIpref1 = NULL;
		itemIpref =m_XMLDoc->newElement(XML_SNMP_INFO);
		
		itemIpref->set(XML_SNMP_SUPPORTOID,strCompanyOID);
		
		itemIpref->set(XML_SNMP_VER,"1.0");
		rootIpref->addNextChild(itemIpref);

		itemIpref =m_XMLDoc->newElement(XML_SNMP_SERVICE);

		itemIpref1 =m_XMLDoc->newElement(XML_SNMP_SERVICEDATA);
		itemIpref1->set(XML_SNMP_NAME,strServiceName);
		itemIpref1->set(XML_SNMP_OID, "1");
		itemIpref->addNextChild(itemIpref1);

		rootIpref->addNextChild(itemIpref);

		itemIpref1->free();
		itemIpref->free();
		rootIpref->free();
		m_XMLDoc->save(strFile);
	}
	return TRUE;
}

BOOL CXMLFileOper::RemoveFromXML(TCHAR *szServiceName,TCHAR *szVarName)
{
	if (m_XMLDoc == NULL || szServiceName == NULL || szVarName == NULL )
		return FALSE;
	ZQ::common::IPreference* rootIpref = m_XMLDoc->root();
	ZQ::common::IPreference* itemIpref = NULL;
	ZQ::common::IPreference* itemIpref1 = NULL;
	BOOL bFind = FALSE;
	
	itemIpref = rootIpref->firstChild(); 
	while(itemIpref != NULL)
	{
		char szNodeName[SERVICENAME_LEN]={0};
		itemIpref->name(szNodeName);
		
		if ( _stricmp(szNodeName,XML_SNMP_SERVICE) == 0 )
		{
			itemIpref1 = itemIpref->firstChild();
			while(itemIpref1 != NULL)
			{

				memset(szNodeName,0,sizeof(szNodeName));
				itemIpref1->name(szNodeName);

				TCHAR szTmp[SERVICENAME_LEN] ={0};
				if ( _stricmp(szNodeName,XML_SNMP_SERVICEDATA) == 0 )
				{
					memset(szNodeName,0,sizeof(szNodeName));
					itemIpref1->get(XML_SNMP_NAME,szNodeName);
					
	#if defined _UNICODE || defined UNICODE
					MultiByteToWideChar(
					 CP_ACP,         // code page
					 0,              // character-type options
					 szNodeName,        // address of string to map
					strlen(szNodeName),      // number of bytes in string
					szTmp,       // address of wide-character buffer
					SERVICENAME_LEN);             // size of buffer);

					if ( _tcsicmp(szTmp,szServiceName) == 0 )
	#else
					if ( _stricmp(szNodeName,szServiceName) == 0 )
	#endif
					{
						bFind = TRUE;
					}
				}
					
				memset(szNodeName,0,sizeof(szNodeName));
				itemIpref1->get(XML_SNMP_NAME,szNodeName);
				memset(szTmp,0,sizeof(szTmp));
#if defined _UNICODE || defined UNICODE
				MultiByteToWideChar(
				 CP_ACP,         // code page
				 0,              // character-type options
				 szNodeName,        // address of string to map
				strlen(szNodeName),      // number of bytes in string
				szTmp,       // address of wide-character buffer
				SERVICENAME_LEN);             // size of buffer);

				if ( ( _tcsicmp(szTmp,szVarName) == 0 ) && ( bFind ) )
#else
				if ( (_stricmp(szNodeName,szVarName) == 0 ) && ( bFind ))
#endif
				{
					itemIpref->removeChild(itemIpref1);
					itemIpref1->free();
					itemIpref->free();
					rootIpref->free();
					m_XMLDoc->save(m_szXMLFileName);
					return TRUE;
				}
				else
				{
					itemIpref1->free();
				}
				
				itemIpref1 = itemIpref->nextChild();
			}
		}
		itemIpref = rootIpref->nextChild();
	}
	if ( itemIpref )
	{
		itemIpref->free();
	}
	rootIpref->free();
	return TRUE;
}

ZQ::common::IPreference * CXMLFileOper::FindItemIprefByVarName(TCHAR *szServiceName,TCHAR *szVarName)
{
	if(m_XMLDoc == NULL)
		return NULL;

	ZQ::common::IPreference* rootIpref = m_XMLDoc->root();
	ZQ::common::IPreference* itemIpref = NULL;
	ZQ::common::IPreference* itemIpref1 = NULL;
	BOOL bFind = FALSE;
	
	itemIpref = rootIpref->firstChild(); 
	while(itemIpref != NULL)
	{
		char szNodeName[SERVICENAME_LEN]={0};
		itemIpref->name(szNodeName);
		
		if ( _stricmp(szNodeName,XML_SNMP_SERVICE) == 0 )
		{
			itemIpref1 = itemIpref->firstChild();
			while(itemIpref1 != NULL)
			{

				memset(szNodeName,0,sizeof(szNodeName));
				itemIpref1->name(szNodeName);

				TCHAR szTmp[SERVICENAME_LEN] ={0};
				if ( _stricmp(szNodeName,XML_SNMP_SERVICEDATA) == 0 )
				{
					memset(szNodeName,0,sizeof(szNodeName));
					itemIpref1->get(XML_SNMP_NAME,szNodeName);
					
	#if defined _UNICODE || defined UNICODE
					MultiByteToWideChar(
					 CP_ACP,         // code page
					 0,              // character-type options
					 szNodeName,        // address of string to map
					strlen(szNodeName),      // number of bytes in string
					szTmp,       // address of wide-character buffer
					SERVICENAME_LEN);             // size of buffer);

					if ( _tcsicmp(szTmp,szServiceName) == 0 )
	#else
					if ( _stricmp(szNodeName,szServiceName) == 0 )
	#endif
					{
						bFind = TRUE;
					}
				}
					
				memset(szNodeName,0,sizeof(szNodeName));
				itemIpref1->get(XML_SNMP_NAME,szNodeName);
				memset(szTmp,0,sizeof(szTmp));
#if defined _UNICODE || defined UNICODE
				MultiByteToWideChar(
				 CP_ACP,         // code page
				 0,              // character-type options
				 szNodeName,        // address of string to map
				strlen(szNodeName),      // number of bytes in string
				szTmp,       // address of wide-character buffer
				SERVICENAME_LEN);             // size of buffer);

				if ( ( _tcsicmp(szTmp,szVarName) == 0 ) && ( bFind ) )
#else
				if ( (_stricmp(szNodeName,szVarName) == 0 ) && ( bFind ))
#endif
				{
					itemIpref->free();
					rootIpref->free();
					return itemIpref1;
				}
				else
				{
					itemIpref1->free();
				}
				
				itemIpref1 = itemIpref->nextChild();
			}
		}
		itemIpref = rootIpref->nextChild();
	}
	if ( itemIpref )
	{
		itemIpref->free();
	}
	rootIpref->free();
	return NULL;
}

ZQ::common::IPreference* CXMLFileOper::FindItemIprefByServiceName(TCHAR *szServiceName)
{
	if(m_XMLDoc == NULL)
		return NULL;

	// change the xml 
		
	ZQ::common::IPreference* rootIpref = m_XMLDoc->root();
	ZQ::common::IPreference* itemIpref = NULL;
	ZQ::common::IPreference* itemIpref1 = NULL;

	itemIpref = rootIpref->firstChild(); 
	while(itemIpref != NULL)
	{
		char szNodeName[SERVICENAME_LEN]={0};
		itemIpref->name(szNodeName);
		
		if ( _stricmp(szNodeName,XML_SNMP_SERVICE) == 0 )
		{
			itemIpref1 = itemIpref->firstChild();
			while(itemIpref1 != NULL)
			{
				memset(szNodeName,0,sizeof(szNodeName));
				itemIpref1->name(szNodeName);

				if ( _stricmp(szNodeName,XML_SNMP_SERVICEDATA) == 0 )
				{
					memset(szNodeName,0,sizeof(szNodeName));
					itemIpref1->get(XML_SNMP_NAME,szNodeName);

					TCHAR szTmp[SERVICENAME_LEN] ={0};
	#if defined _UNICODE || defined UNICODE
					MultiByteToWideChar(
					 CP_ACP,         // code page
					 0,              // character-type options
					 szNodeName,        // address of string to map
					strlen(szNodeName),      // number of bytes in string
					szTmp,       // address of wide-character buffer
					SERVICENAME_LEN);             // size of buffer);

					if ( _tcsicmp(szTmp,szServiceName) == 0 )
	#else
					if ( _stricmp(szNodeName,szServiceName) == 0 )
	#endif
					{
						itemIpref1->free();
						rootIpref->free();
						return itemIpref;
					}
					else
					{
						itemIpref1->free();
					}
				}
				itemIpref1 = itemIpref->nextChild();
			}
		}
		itemIpref = rootIpref->nextChild();
	}
	if ( itemIpref )
	{
		itemIpref->free();
	}
	rootIpref->free();
	return NULL;
}

BOOL CXMLFileOper::SetItemDataToXML(TCHAR *szServiceName,TCHAR *szVarName,TCHAR *szNewValue,TCHAR *szOID, WORD wType,int iReadOnly)
{
	if ( m_XMLDoc == NULL || szServiceName == NULL || szVarName == NULL || szNewValue == NULL )
		return FALSE;

	ZQ::common::IPreference* rootIpref = m_XMLDoc->root();
	ZQ::common::IPreference* itemIpref = NULL;
		
	itemIpref = FindItemIprefByVarName(szServiceName,szVarName);
	if ( itemIpref )
	{
		if ( szOID )
		{
		
#if defined _UNICODE || defined UNICODE
			char strOID[SERVICENAME_LEN] ={0};
			WideCharToMultiByte(CP_ACP,NULL,szOID,-1,strOID,sizeof(strOID),NULL,NULL);

			itemIpref->set(XML_SNMP_OID,strOID);
#else
			itemIpref->set(XML_SNMP_OID,szOID);
#endif
		}
		switch ( wType )
		{
			case ZQSNMP_STR:
				itemIpref->set(XML_SNMP_TYPE,"ASN_INT");
				break;
			case ZQSNMP_INT:
				itemIpref->set(XML_SNMP_TYPE,"ASN_STR");
				break;
			case ZQSNMP_FLOAT:
				itemIpref->set(XML_SNMP_TYPE,"ASN_FLOAT");
				break;
			default:
				break;
		}
		
		switch ( iReadOnly )
		{
			case 0:
				itemIpref->set(XML_SNMP_ACCESS,"READWRITE");
				break;
			case 1:
				itemIpref->set(XML_SNMP_ACCESS,"READONLY");
				break;
			default:
				break;
		}
		
#if defined _UNICODE || defined UNICODE
		char strValue[FILENAME_LEN]={0};
		WideCharToMultiByte(CP_ACP,NULL,szNewValue,-1,strValue,sizeof(strValue),NULL,NULL);
		
		itemIpref->set(XML_SNMP_VALUE,strValue);
#else
		itemIpref->set(XML_SNMP_VALUE,szNewValue);
#endif
				
	itemIpref->free();
	}
	rootIpref->free();
	m_XMLDoc->save(m_szXMLFileName);
	return TRUE;
}

BOOL CXMLFileOper::WriteToXML(TCHAR *szServiceName,TCHAR *szVarName,TCHAR *szOID, WORD wType,BOOL bReadOnly, TCHAR *szVarValue)
{
	if ( m_XMLDoc == NULL || szServiceName == NULL || szVarName == NULL || szOID == NULL || szVarValue ==NULL )
		return FALSE;

	RemoveFromXML(szServiceName,szVarName);
	ZQ::common::IPreference* rootIpref = m_XMLDoc->root();
	ZQ::common::IPreference* itemIpref = NULL;
	ZQ::common::IPreference* itemIpref1 = NULL;
	
	itemIpref = FindItemIprefByServiceName(szServiceName);
	if ( itemIpref )
	{
		itemIpref1 =m_XMLDoc->newElement(XML_SNMP_VAR);

#if defined _UNICODE || defined UNICODE
		char strVarName[SERVICENAME_LEN]={0};
		char strOID[SERVICENAME_LEN] ={0};
		WideCharToMultiByte(CP_ACP,NULL,szVarName,-1,strVarName,sizeof(strVarName),NULL,NULL);
		WideCharToMultiByte(CP_ACP,NULL,szOID,-1,strOID,sizeof(strOID),NULL,NULL);

		itemIpref1->set(XML_SNMP_NAME,strVarName);
		itemIpref1->set(XML_SNMP_OID,strOID);
#else
		itemIpref1->set(XML_SNMP_NAME,szVarName);
		itemIpref1->set(XML_SNMP_OID,szOID);
#endif

		switch ( wType )
		{
			case ZQSNMP_STR:
				itemIpref1->set(XML_SNMP_TYPE,"ASN_STR");
				break;
			case ZQSNMP_INT:
				itemIpref1->set(XML_SNMP_TYPE,"ASN_INT");
				break;
			case ZQSNMP_FLOAT:
				itemIpref->set(XML_SNMP_TYPE,"ASN_FLOAT");
				break;
			default:
				itemIpref1->set(XML_SNMP_TYPE,"ASN_STR");
				break;
		}
		if ( bReadOnly )
		{
			itemIpref1->set(XML_SNMP_ACCESS,"READONLY");
		}
		else
		{
			itemIpref1->set(XML_SNMP_ACCESS,"READWRITE");
		}
		
#if defined _UNICODE || defined UNICODE
		char strValue[FILENAME_LEN]={0};
		WideCharToMultiByte(CP_ACP,NULL,szVarValue,-1,strValue,sizeof(strValue),NULL,NULL);
		
		itemIpref1->set(XML_SNMP_VALUE,strValue);
#else
		itemIpref1->set(XML_SNMP_VALUE,szVarValue);
#endif
		
		itemIpref->addNextChild(itemIpref1);
//		rootIpref->addNextChild(itemIpref); // 这里不再需要这步操作
		itemIpref1->free();
		itemIpref->free();
	}
	rootIpref->free();
	m_XMLDoc->save(m_szXMLFileName);
	return TRUE;
}