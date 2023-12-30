
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

//#include "StdAfx.h"
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
BOOL g_IsStopAll;

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

	g_IsStopAll = FALSE;

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
	sTmp += _T("DataStream.ini");
	sConfig += _T("configuration.xml");
	sLogPath += _T("PortController.log");

	//memset( m_struIP.cAddr, 0, MAX_ADDRESS_LEN );
	memset( m_struIP.szConfigPath, 0, MAX_PATH );

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

	ClogEstablishSettings("DataStream.log",LOG_DEBUG,LOGFILE_SIZE );

	// added by Cary
#ifndef _NO_FIX_LOG
	unsigned int dbgLevel;
	dbgLevel = GetPrivateProfileInt("System","LogLevel", (INT )ISvcLog::L_DEBUG, sTmp );
	service_log.setSevel((ISvcLog::LogLevel) dbgLevel);

	unsigned int LogSize;
	LogSize = GetPrivateProfileInt("System","LogSize", 0x6400000, sTmp );
	service_log.setMaxSize(LogSize);

	void setPortControllerLog(ISvcLog* log);
	setPortControllerLog(&service_log); 
#endif // #ifndef _NO_FIX_LOG

	Clog( LOG_DEBUG,_T("\r\n\r\n//-------------Begin Run----------------------------------\r\n.") );
	
	//create ConnectManager
	m_pConnectManager = new CListenManager( &m_struIP );
	
	// whether the config file has exists.
	
	Clog( LOG_DEBUG, _T("Load File: %s"), m_struIP.szConfigPath );
	//////////////////////////////////////////////////////
	m_pConnectManager->m_pServerParser->m_DeviceManager.SetLogFile( m_struIP.szPortCtlLogFile );
	
	m_pConnectManager->m_pServerParser->m_iHaveConfig = 1;
	m_pConnectManager->m_pServerParser->m_DeviceManager.Initialize( m_struIP.szConfigPath );
	
	m_pConnectManager->m_pServerParser->m_iControllerID = m_struIP.iControllerID;
	// start connect thread
	m_pConnectManager->Start();
	Clog( LOG_DEBUG,_T("CConnectManager Created.") );

	if (!m_iceService.start()) {
		glog(ISvcLog::L_CRIT, "%s, IceService::start() failed.", __FUNCTION__);
		service_log.beep(CSvcLog::BEEP_INIT_FAILED);
	}
	
}

void CDeviceAgent::UnInitialize()
{
	Clog( LOG_DEBUG,_T("DeviceAgent::UnInitialize.") );
	g_IsStopAll = TRUE;
	m_pConnectManager->Stop();

	if( m_pConnectManager )
	{
		delete m_pConnectManager;
		m_pConnectManager = NULL;
	}

	// added by Cary
#ifndef _NO_FIX_LOG
	service_log.uninit();
#endif

	m_iceService.stop();
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
