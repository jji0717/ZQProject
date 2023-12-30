/*****************************************************************************
File Name:     SRManager.cpp
Author:        haiping.wan
Security:      SEACHANGE SHANGHAI
Description:   implements class CSRManager
Function Inventory: 
Modification Log:
When           Version        Who						What
------------------------------------------------------------------------------
2005/04/22     1.0            haiping.wan					Created
*******************************************************************************/
#include "StdAfx.h"
#include "SRManager.h"

CSRManager::CSRManager(void)
{
   m_ClientManager=new CClientManager(this);
   m_ResourceManager=new CResourceManager(this);
   m_Sessionmanager=new CSessionManager(this);
}
CSRManager::~CSRManager(void)
{
 /*  ::CoUninitialize();
   delete  m_ClientManager;
   delete  m_ResourceManager;
   delete  m_Sessionmanager;*/
   
}
void CSRManager::Initialize()
{ 
   ::CoInitialize(NULL);
	m_Sessionmanager->SetResourceManger(m_ResourceManager);
	m_ResourceManager->SetSessionManager(m_Sessionmanager);
	//m_ClientManager->m_parse.SetSessionManager(m_Sessionmanager);
	//m_ResourceManager->m_parse.SetSessionManager(m_Sessionmanager);
    
    int    nIndex;
    char   sModuleName[1025];
    //    char   sSystemDir[1025];
    CString   szsModuleName;
    CString   strCommandLine;
    DWORD dwSize = GetModuleFileName(NULL, sModuleName, 1024);
    sModuleName[dwSize] = '\0';
    szsModuleName = sModuleName;
    nIndex = szsModuleName.ReverseFind('\\');
    szsModuleName = szsModuleName.Left(nIndex+1)+"SRMConfiguration.xml";
    strcpy(m_strXMLFileName, szsModuleName.GetBuffer(0));
}

bool CSRManager::LoadConfiguration()
{
    HRESULT hr;
	MSXML2::IXMLDOMDocumentPtr		pDocument	= NULL;	
	MSXML2::IXMLDOMElementPtr		pRoot		= NULL;
	MSXML2::IXMLDOMNodePtr			pNode		= NULL;
    MSXML2::IXMLDOMNodePtr			pChildNode	= NULL;
	MSXML2::IXMLDOMParseErrorPtr	pErr		= NULL;
	MSXML2::IXMLDOMNamedNodeMapPtr	pNamedMap	= NULL;
	MSXML2::IXMLDOMNodePtr			pAttribute	= NULL;
	HANDLE hXMLFile =  INVALID_HANDLE_VALUE;  //xml file handle.
	DWORD dwSize = 0;						  //xml file size.
	char* pBuffer = 0;					      //buffer to load xml file content.
	DWORD dwRead = 0;

	CString strNodeName;
	COleVariant vIPAddress;

    COleVariant vHostIP;
	COleVariant vPort;
	// create instance.
	hr = pDocument.CreateInstance(__uuidof(MSXML2::DOMDocument));
	if( FAILED(hr))
	{
		Clog(0, "[GetAgentConfigurations]Failed to call CreateInstance. hr=%d.", hr);
		m_nError = (int) hr;
		strcpy( m_strError, "Failed to call CreateInstance.");
		return FALSE;
	}
	pDocument->PutvalidateOnParse(VARIANT_FALSE);
	pDocument->PutresolveExternals(VARIANT_FALSE);
	pDocument->PutpreserveWhiteSpace(VARIANT_TRUE);

	hXMLFile = CreateFile(m_strXMLFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if( hXMLFile == INVALID_HANDLE_VALUE )
	{
		m_nError = ::GetLastError();
		strcpy( m_strError, "Failed to open Controller configuration xml file.");
		Clog(0, "[GetAgentConfigurations]Failed to open xml file.m_nError=%d.", m_nError);
		return FALSE;
	}
	dwSize = GetFileSize( hXMLFile, NULL );
	if( dwSize ==  INVALID_FILE_SIZE )
	{
		m_nError = ::GetLastError();
		strcpy( m_strError, "Failed to get Controller configuration xml file size.");
		CloseHandle( hXMLFile );
		Clog(0, "[GetAgentConfigurations]Failed to GetFileSize. m_nError=%d.", m_nError);
		return FALSE;
	}
	pBuffer = new char[dwSize+1];
	ReadFile( hXMLFile, pBuffer, dwSize, &dwRead, NULL);
	pBuffer[dwSize]='\0';

	// load xml content.
    if(pDocument->loadXML((_bstr_t)CString(pBuffer))!=VARIANT_TRUE)
    {
		pErr = pDocument->GetparseError();
		m_nError = pErr->GeterrorCode();
		CString strErr = CString((wchar_t *)pErr->Getreason());
		strcpy( m_strError, strErr.GetBuffer(0) );
		Clog(0, "[GetAgentConfigurations]Failed to load xml contents. m_nError=%d, m_strError=%s.", m_nError, m_strError);
		delete[] pBuffer;
		CloseHandle( hXMLFile );
		return FALSE;
	}
	CloseHandle( hXMLFile );
	delete[] pBuffer;
	// get root node.
	pRoot = pDocument->GetdocumentElement();
	if(pRoot==NULL)
	{
		pErr = pDocument->GetparseError();
		m_nError = pErr->GeterrorCode();
		CString strErr = CString((wchar_t *)pErr->Getreason());
		strcpy( m_strError, strErr.GetBuffer(0) );
		Clog(0, "[GetAgentConfigurations]Failed to GetdocumentElement. m_nError=%d, m_strError=%s.", m_nError, m_strError);
		return FALSE;
	}
	//get first child node.
	pNode = pRoot->GetfirstChild();
	while(pNode)
	{
		strNodeName = CString((wchar_t*)pNode->GetnodeName());
        if(strNodeName.Compare("SRMServer")==0)
		{
		    pNamedMap = pNode->Getattributes();
			if( pNamedMap!=NULL )
			{
				pAttribute = pNamedMap->getNamedItem((_bstr_t)"IPAddress");
				if( pAttribute==NULL)
				{
					pNode = pNode->GetnextSibling();
					continue;
				}
				vHostIP= pAttribute->GetnodeValue();
				pAttribute = pNamedMap->getNamedItem((_bstr_t)"IPPort");
				if( pAttribute==NULL )
				{
					pNode = pNode->GetnextSibling();
					continue;
				}
				vPort = pAttribute->GetnodeValue();
				vPort.ChangeType(VT_INT);
				int Port = vPort.intVal;
				m_ClientManager->SetHost((CString((wchar_t*)vHostIP.bstrVal)).GetBuffer(0),Port);
		   }
		}
		else if(strNodeName.Compare("SRMClientToDSA")==0)
		{
            pChildNode=pNode->GetfirstChild();
			while(pChildNode)
			{
			 pNamedMap = pChildNode->Getattributes();
			 if( pNamedMap!=NULL )
			 {
				pAttribute = pNamedMap->getNamedItem((_bstr_t)"IPAddress");
				if( pAttribute==NULL)
				{
					pChildNode = pChildNode->GetnextSibling();
					continue;
				}
				vIPAddress = pAttribute->GetnodeValue();
				pAttribute = pNamedMap->getNamedItem((_bstr_t)"IPPort");
				if( pAttribute==NULL )
				{
					pChildNode = pChildNode->GetnextSibling();
					continue;
				}
				vPort = pAttribute->GetnodeValue();
				vPort.ChangeType(VT_INT);
				int Port = vPort.intVal;
				m_ResourceManager->AddOneDSA((CString((wchar_t*)vIPAddress.bstrVal)).GetBuffer(0),Port);
            }
            pChildNode = pChildNode->GetnextSibling();
		  }
		}
		pNode = pNode->GetnextSibling();
	}
  return true;
}

void CSRManager::Start()
{
	m_ClientManager->StartListen();
	m_ResourceManager->Create();
	m_ResourceManager->SendHeartBeat();

	m_Sessionmanager->StartCheck();
	m_ClientManager->SendHeartBeat();

}
BOOL CSRManager::create()
{
   Initialize();
   LoadConfiguration();
   Start();
   return TRUE;
}

void CSRManager::Destroy(void)
{
   
	printf("begin destroy");
    m_ClientManager->Destroy();
printf("begin destroy1");
	m_ResourceManager->Destroy();
printf("begin destroy2");
	m_Sessionmanager->Destroy();
printf("begin destroy3");

   delete  m_ClientManager;
   delete  m_ResourceManager;
   delete  m_Sessionmanager;
   ::CoUninitialize();

}
BOOL CSRManager::Pause()
{
	return TRUE;
}

BOOL CSRManager::Resume()
{
	return TRUE;
}















