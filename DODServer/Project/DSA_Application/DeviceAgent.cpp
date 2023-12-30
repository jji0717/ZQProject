
/*
**	FILENAME			DeviceAgent.cpp
**
**	PURPOSE				This file include a struct/class: CDeviceAgent;
**						the class CDeviceAgent is used as a primary control class in the service, when service
**						start, the class will be created and all work will start along with the class activation.
**						
**
**	CREATION DATE		01-25-2005
**	LAST MODIFICATION	01-25-2005
**
**	AUTHOR				Leon.li (Interactive ZQ)
**
**
*/

#include "StdAfx.h"
//#include "DeviceInfo.h"
#include "DeviceAgent.h"
#include "clog.h"
#include "common.h"
#include "scqueue.h"
#include <direct.h>
#include <Winnetwk.h>
/*
#include <vstrmtypes.h>
#include <iccardioctl.h>
#include <sg2010drvconst.h>
#include <sg2010drvcou.h>
#include <vstrmstatus.h>
#include <dacdefs.h>
#include <VstrmConstants.h>
#include <vsmapi.h>
#include <vstrmportchars.h>
#include <vstrmsesschars.h>
#include <vstrmfp.h>
#include <vstrmuser.h>
//#include "..\..\..\sdk\inc\vstrmconstants.h"
*/
#include <WinIoctl.h>

#pragma warning(disable:4307)

CDeviceAgent *g_pDeviceAgent;

CDeviceAgent::CDeviceAgent()
{
	Initialize();
}

CDeviceAgent::~CDeviceAgent()
{
	UnInitialize();
}

void CDeviceAgent::Initialize()
{	// set current directory
	TCHAR szPath[MAX_PATH];
	GetModuleFileName( NULL, szPath, MAX_PATH );
	TCHAR drive[16],dir[MAX_PATH],fname[MAX_PATH],ext[16];
	_tsplitpath( szPath,drive,dir,fname,ext );
	_tcscpy( szPath, drive );
	_tcscat( szPath, dir );
	SetCurrentDirectory( szPath );

	//get configuration
	CString sTmp,sConfig = _T(""), sLogPath; //( szPath ), sConfig( szPath );
	sLogPath = szPath;
	sTmp = szPath;
	sConfig = szPath;
	sTmp += _T("DeviceAgent.ini");
	sConfig += _T("configuration.xml");
	sLogPath += _T("PortController.log");

	//memset( m_struIP.cAddr, 0, MAX_ADDRESS_LEN );
	//memset( m_struIP.szConfigPath, 0, MAX_PATH );

	GetPrivateProfileString("System","IP","192.168.80.111",m_struIP.cAddr,MAX_ADDRESS_LEN, sTmp );
	m_struIP.wPort = GetPrivateProfileInt("System","Port",1300, sTmp ); 
	m_struIP.iReConnInterval = GetPrivateProfileInt("System","ReconnectInterval",3, sTmp ); 
	m_struIP.iControllerID = GetPrivateProfileInt("System","ControllerID",3, sTmp ); 
	m_struIP.iType = GetPrivateProfileInt("System","Type", 1, sTmp ); 
	GetPrivateProfileString("System","ConfigPath",sConfig, m_struIP.szConfigPath, MAX_PATH, sTmp ); 
	GetPrivateProfileString("System","LogFile",sLogPath, m_struIP.szPortCtlLogFile, MAX_PATH, sTmp ); 

	//m_struIP.szConfigPath[MAX_PATH-1] = 0;
	if( _tcscmp( m_struIP.szConfigPath, "" ) == 0 )
		_stprintf( m_struIP.szConfigPath, "%s", sConfig );

	ClogEstablishSettings("DeviceAgent.log",LOG_DEBUG,LOGFILE_SIZE );
	Clog( LOG_DEBUG,_T("\r\n\r\n//-------------Begin Run----------------------------------\r\n."), m_struIP.cAddr );

	//Clog( LOG_DEBUG,_T("Current Remote IP Address: %s."), m_struIP.cAddr );
	//Clog( LOG_DEBUG,_T("Current Remote Port: %d."), m_struIP.wPort );
	//Clog( LOG_DEBUG,_T("ReconnectInterval: %d."), m_struIP.iReConnInterval );
	//Clog( LOG_DEBUG,_T("ConfigPath: %s."), m_struIP.szConfigPath );

	vcDeviceItem.reserve( 40 );
	//create ConnectManager
	//m_pConnectManager = new CSingleConnect( &m_struIP, NULL );
	m_pConnectManager = new CListenManager( &m_struIP );

	// whether the config file has exists.
	if( FileExists( m_struIP.szConfigPath ) )
	{
		Clog( LOG_DEBUG, _T("Load File: %s"), m_struIP.szConfigPath );
		//////////////////////////////////////////////////////
		//CMarkup xmlDOM;
		//xmlDOM.Load( m_struIP.szConfigPath );
		//xmlDOM.ResetPos();
		//CString strXML = xmlDOM.GetDoc();
		//////////////////////////////////////////////////////
		m_pConnectManager->m_pServerParser->m_DeviceManager.SetLogFile( m_struIP.szPortCtlLogFile );

		m_pConnectManager->m_pServerParser->m_iHaveConfig = 1;
		m_pConnectManager->m_pServerParser->m_dwDManagerOpen = m_pConnectManager->m_pServerParser->m_DeviceManager.Initialize( m_struIP.szConfigPath );		
		if( m_pConnectManager->m_pServerParser->m_dwDManagerOpen == MEDIAPUMP_NOERROR )
		{
			m_pConnectManager->m_pServerParser->QueryDeviceList();
		}
		else
		{
			;//MessageBox( _T("InitManager Fail."), _T("Hint"), MB_OK | MB_ICONWARNING );
		}
	}
	else
		m_pConnectManager->m_pServerParser->m_iHaveConfig = 0;

	m_pConnectManager->m_pServerParser->m_iControllerID = m_struIP.iControllerID;
	// start connect thread
	m_pConnectManager->Start();
	Clog( 3,_T("CConnectManager Created.") );
  
}
  
void CDeviceAgent::UnInitialize()
{
//	m_pConnectManager->Stop();

	CDeviceInfoParser::s_pDeviceInfoParser->m_bPermitStatus = false;

	//m_pConnectManager->m_pServerParser->m_DeviceManager.CloseManager();

	
//	m_pConnectManager->m_pServerParser->m_DeviceManager.StopPort( 0 );
//	m_pConnectManager->m_pServerParser->m_DeviceManager.ClosePort( 0 );

	//delete ConnectManager	
	if( m_pConnectManager )
	{
		delete m_pConnectManager;
		m_pConnectManager = NULL;
	}
	vcDeviceItem.clear();

}

bool CDeviceAgent::FileExists( LPCTSTR lpszFilename )
{
	if( lpszFilename == NULL )
		return false;
	HANDLE hFile = ::CreateFile(lpszFilename, GENERIC_READ, FILE_SHARE_READ, NULL,
					OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if( hFile == INVALID_HANDLE_VALUE ) 
		return false;
	CloseHandle(hFile);
	return true;
}