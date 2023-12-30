// MCastFwDServ.cpp: implementation of the MCastFwDServ class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "MCastFwdServ.h"
#include "InetAddr.h"
#include "Manpkg.h"
#include "GUID.h"
#include "ItvServiceTypes.h"
#include "locks.h"

#include <comutil.h>

ZQ::common::InetHostAddress gLocalAddrs;
DWORD MCastFwdServ::m_dwTunnelListenPort = DEFAULT_TUNNEL_LISTERN_PORT; // Tunnel's Listen port
wchar_t MCastFwdServ::m_wsConfigFileName[MaxPath] = _T("");
DWORD MCastFwdServ::m_dwReMcastTTL  = DEFAULT_REMCAST_TTL;
wchar_t MCastFwdServ::m_wsInstallationID[64] = _T("0");
wchar_t MCastFwdServ::m_DefaultTunnels[256] = _T("");

ZQ::common::IPreference* MCastFwdServ::m_IPreference = NULL;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//global variable
DWORD gdwServiceType = ITV_TYPE_PRIMARY_ZQ_MCASTFORWARD;
DWORD gdwServiceInstance = 0;
// DWORD gdwMaxUDPBufferSize = 0;
bool gbPeerStampFlag = false;
MCastFwdServ theServer;
ZQ::common::BaseSchangeServiceApplication * Application = &theServer; 
using ZQ::common::Log;
ZQ::common::Log* pProglog = &ZQ::common::NullLogger;


MCastFwdServ::MCastFwdServ()
: BaseSchangeServiceApplication(),McastFwd()
{//reset default value
	m_dwTunnelListenPort = DEFAULT_TUNNEL_LISTERN_PORT;
	wcsncpy(m_wsConfigFileName,DEFAULT_EXT_CONFIG_FILE_NAME,MaxPath);
//	_dwPortNumber = 1997;
	
	
}


HRESULT MCastFwdServ::OnInit(void)
{
	HRESULT hr = S_OK;

	wchar_t VerMsg[64];
	wsprintf(VerMsg,_T("**********************Core Version %S******************"),MCASTFWD_VERSION);
	logEvent(VerMsg,Log::L_DEBUG);

	pProglog = m_pReporter;
	ZQ::common::setGlogger(m_pReporter);

/* the TunnelListenPort registry setting is moved to xml configuration
   11/17/04 -andy

	BaseSchangeServiceApplication::getConfigValue(_T("TunnelListenPort"),
									&m_dwTunnelListenPort,
									DEFAULT_TUNNEL_LISTERN_PORT,
									true,
									true);
	
	//m_MCastFwd = new McastFwd(m_dwTunnelListenPort);
*/
	
	DWORD dwSize = MaxPath;
	getConfigValue(_T("ExtConfiguration"),
				m_wsConfigFileName,
				m_wsConfigFileName,
				&dwSize,
				true,
				true);	
	if(wcschr(m_wsConfigFileName,L'%'))
	{
		wchar_t buf[MaxPath];
		if(::ExpandEnvironmentStrings(m_wsConfigFileName,buf,MaxPath))		
			
		{
			memset(m_wsConfigFileName,0,MaxPath);
			wcsncpy(m_wsConfigFileName,buf,MaxPath);
		}
	}
	getConfigValue(_T("ReMcastTTL"),
				&m_dwReMcastTTL,
				m_dwReMcastTTL,
				true,
				true);
/* 11/17/04 -andy
	getConfigValue(_T("MaxUDPBufferSize"),
		&gdwMaxUDPBufferSize,
		gdwMaxUDPBufferSize,
		true,
		true);
*/
	
	getConfigValue(_T("PeerStampFlag"),
		&gbPeerStampFlag,
		gbPeerStampFlag,
		true);

	dwSize = 64*2;
	hr = BaseSchangeServiceApplication::getConfigValue(_T("InstallationID"),m_wsInstallationID,m_wsInstallationID,&dwSize,false,false);
	ZQ::common::Guid * pGuid =NULL;
	
	//generate a install id and set it in registry
	pGuid = new ZQ::common::Guid(_com_util::ConvertBSTRToString(m_wsInstallationID));
	if(!pGuid->isNil()&& hr == S_OK)
	{
		TunnelConnection::_localid = *pGuid;
		hr = S_OK;
	}
	else
	{	
		TunnelConnection::_localid.create();
		::mbstowcs(m_wsInstallationID,
					TunnelConnection::localidstr(),
					64*2);


		BaseSchangeServiceApplication::setConfigValue(_T("InstallationID"),
												m_wsInstallationID,
												64*2,
												REG_SZ,
												true);
	}	
	DWORD dwError = 0;
	BaseSchangeServiceApplication::manageVar(_T("InstallationID"),
											MAN_STR,
											(DWORD)m_wsInstallationID,
											false,
											&dwError);
	delete pGuid;
	pGuid = NULL;


	gLocalAddrs =  ZQ::common::InetHostAddress::getLocalAddress();
	TunnelConnection::_pDefaultConversation = (Conversation*)this;
	//TunnelConnection::_localid.create();
	ZQ::common::ComInitializer ComInit;

	ZQ::common::XMLPrefDoc XmlDoc(ComInit);

	// open the config file 
	try
	{
		wchar_t bufMsg[MaxPath + 64];
		wsprintf(bufMsg,_T("Opening config file %s"),m_wsConfigFileName);
		logEvent(bufMsg,Log::L_DEBUG);
		if (!XmlDoc.open(_com_util::ConvertBSTRToString(m_wsConfigFileName)))
		{
			// failed to open the configuration file
			memset(bufMsg,0,MaxPath + 64);
			wsprintf(bufMsg,_T("Opening config file %s failed, initialize McastFwd failed"),m_wsConfigFileName);
			logEvent(bufMsg,Log::L_ERROR);
			return S_FALSE;
		}
	}
	catch(ZQ::common::Exception e)
	{
		wchar_t bufMsg[MaxPath + 64];
		wsprintf(bufMsg,_T("Exception comming when opening config file %s"),m_wsConfigFileName);
		logEvent(bufMsg,Log::L_ERROR);
		return S_FALSE;
	}
	// get the config tree
	 m_IPreference = XmlDoc.root();
	if(m_IPreference ==NULL)
	{
		logEvent(_T("Invalid config file format:No root found"),Log::L_ERROR);
		return S_FALSE;
	}	
	
	/// set man variables
	
	BaseSchangeServiceApplication::manageVar(MCASTFWD_MANPKG_TUNNELS,
									MAN_COMPLEX,
									(DWORD)MgmtTunnels,
									true,
									&dwError);
	BaseSchangeServiceApplication::manageVar(MCASTFWD_MANPKG_LISTENERS,
									MAN_COMPLEX,
									(DWORD)MgmtConversation,
									true,
									&dwError);
	BaseSchangeServiceApplication::manageVar(MCASTFWD_MANPKG_RESENDERS,
									MAN_COMPLEX,
									(DWORD)MgmtResenders,
									true,
									&dwError);
	BaseSchangeServiceApplication::manageVar(MCASTFWD_MANPKG_DENYLIST,
									MAN_COMPLEX,
									(DWORD)MgmtDenylist,
									true,
									&dwError);

	//initialize Mcast forwarding
	bool bInited = initialize(m_IPreference, true, m_dwTunnelListenPort);
	m_IPreference->free();
	if(!bInited)
	{
		hr = S_FALSE;
	}
	return hr;
}

HRESULT MCastFwdServ::OnStart(void)
{

	HRESULT hr = BaseSchangeServiceApplication::OnStart();
	if(hr != S_OK)
	{
		return hr;
	}
	
	logEvent(_T("start MultiCast forwarding process..."),Log::L_DEBUG);
	
	if(!start())
	{
		hr = S_FALSE;
	}
	DWORD i = 0;
	wchar_t buf [64];
	memset(buf,0,128);
	for(i =0; i < _tunnels.size();i ++)
	{
		TunnelConnection * pTunnel = _tunnels.at(i);
		if(i < _tunnels.size() -1)
			wsprintf(buf,_T("%d,"),pTunnel->_id);			
		else 
			wsprintf(buf,_T("%d"),pTunnel->_id);			
	}
	if(buf != NULL)
	{
		wcscpy(m_DefaultTunnels,buf);
	}	
	DWORD dwError = 0 ;
	manageVar(L"DefaultTunnels", MAN_STR,(DWORD)(void*)m_DefaultTunnels,true,&dwError);
	
	//Sleep(2000);
//	m_MCastFwd->stop();
	return hr;
}
HRESULT MCastFwdServ::OnStop(void)
{
	HRESULT hr = S_OK;
	hr = BaseSchangeServiceApplication::OnStop();
	return hr;
}
bool MCastFwdServ::isHealth(void)
{
	
	return true;
}

HRESULT MCastFwdServ::OnUnInit(void)
{
	stop();	
	pProglog = &ZQ::common::NullLogger;
	ZQ::common::setGlogger();
	ZQ::common::gThreadPool.clean();

	::Sleep(2000);
	return S_OK;
}

UINT MCastFwdServ::MgmtTunnels(WCHAR *pwszCmd, WCHAR **ppwszBuffer, DWORD *pdwLength)
{
	WORD wCommand;
	swscanf(pwszCmd, L"%c\t", &wCommand);

	// Dispatch based on requested operation
	if (wCommand == MAN_FREE)
	{
		// Free the allocated response buffer
		if (*ppwszBuffer != NULL)
		{
			delete [] *ppwszBuffer;
			*ppwszBuffer = NULL;
		}
		return MAN_SUCCESS;
	}
	else if (wCommand != MAN_READ_VARS)
	{
		return MAN_BAD_PARAM;
	}
	WCHAR *pszTmp = wcschr(pwszCmd, '\t');
	WCHAR *pszWhich = pszTmp + 1;	

	//MCastFwdServ * server = (MCastFwdServ*)Application;
	if (0 == wcsncmp(pszWhich, MCASTFWD_MANPKG_TUNNELS, wcslen(MCASTFWD_MANPKG_TUNNELS)))
	{	
		DWORD dwTunnelCount = gTunnelManager._list.size(); //server->_tunnels.size();

//		gTunnelManager._lock.enter();
		TunnelConnection* pTunnel = NULL;
		Conversation * pConversation = NULL;

		DWORD dwVarCount = 3;
		DWORD dwColumnCount = 6;

		wchar_t wsOutPut[4096];
		wchar_t *tmp = wsOutPut;
		memset(wsOutPut,0,4096*2);

		//buffer header
		tmp += wsprintf(tmp,L"%d\t%d\n",dwVarCount,dwColumnCount);
		//////////////////////
		// Simple Variables
		//
		// <Type> <tab> <Name> <tab> <Value>
		//


		tmp += wsprintf(tmp, L"%d\t%s\t%d\n", MAN_INT, L"TunnelListenPort", gTunnelManager.getPort());

		wchar_t buf[256];
		memset(buf,0,512);

		wsprintf(buf, L"%S", gTunnelManager.getBindAddr());

		tmp += wsprintf(tmp, L"%d\t%s\t%s\n", MAN_STR, L"BindIP", buf);
		tmp += wsprintf(tmp, L"%d\t%s\t%d\n", MAN_COUNT, L"Count",dwTunnelCount);

		std::vector<DIS_TUNNEL> Dis_Tunnels;
		
		DWORD i = 0;
		{		
			ZQ::common::MutexGuard guard(gTunnelManager._lock);
			for(i = 0 ; i < dwTunnelCount; i++)
			{	DIS_TUNNEL disTunnel;
				pTunnel = gTunnelManager._list.at(i);
				
				disTunnel.ID = pTunnel->_id;
				disTunnel.PeerID = pTunnel->peeridstr();
				disTunnel.PeerIP = pTunnel->getRemoteAddr();
				disTunnel.EstablishTime = pTunnel->_establishTimeStr;
				disTunnel.status = pTunnel->getStatus();
				DWORD dwOwnerCount = pTunnel->_owners.size();
				if(0 == dwOwnerCount )
					disTunnel.Owner = "";
				ZQ::common::MutexGuard OwnerGuard(pTunnel->_ownersLock);
				for(DWORD j = 0 ; j < dwOwnerCount ; j++)
				{
					char buf[16];
					Conversation* pCon = pTunnel->_owners.at(j);
					
					if(j < dwOwnerCount - 1)
						sprintf(buf,"%d,",pCon->_id);
					else 
						sprintf(buf,"%d",pCon->_id);
					disTunnel.Owner += buf;
				}
				Dis_Tunnels.push_back(disTunnel);
			}
		}
		/////////////////////
		// Complex table
		// <see above>
		//
		// Column One:  <Type> <tab> <width> <tab> <RowCount> <tab> <Name> <newline>		
		DIS_TUNNEL disTunnel;

		tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_INT, dwTunnelCount, L"ID");
	
		for( i = 0; i < dwTunnelCount;i++)
		{   // column's row-entries
			// entry:  <text> <newline>		
			disTunnel = Dis_Tunnels.at(i);
			tmp += wsprintf(tmp, L"%d\n",disTunnel.ID);			
		}


		// Column two:  <Type> <tab> <width> <tab> <RowCount> <tab> <Name> <newline>
		tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_STR, dwTunnelCount, L"PeerUid");

		for( i = 0; i < dwTunnelCount;i++)
		{   // column's row-entries
			// entry:  <text> <newline>		
			disTunnel = Dis_Tunnels.at(i);
			tmp += wsprintf(tmp, L"%S\n",disTunnel.PeerID.c_str());			
		}



		// Column three:  <Type> <tab> <width> <tab> <RowCount> <tab> <Name> <newline>
		tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_STR, dwTunnelCount, L"PeerIP");
		for( i = 0; i < dwTunnelCount;i++)
		{   // column's row-entries
			// entry:  <text> <newline>		
			disTunnel = Dis_Tunnels.at(i);
			tmp += wsprintf(tmp, L"%S\n",disTunnel.PeerIP.c_str());
		}

//comment by kaliven lee because of the fault of the api ::GetRemotePort()
		//// Column four:  <Type> <tab> <width> <tab> <RowCount> <tab> <Name> <newline>
		//tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_INT, dwTunnelCount, L"PeerPort");
		//for( i = 0; i < dwTunnelCount;i++)
		//{   // column's row-entries
		//	// entry:  <text> <newline>		
		//	pTunnel = gTunnelManager._list.at(i);
		//	tmp += wsprintf(tmp, L"%d\n",pTunnel->_remotePort);
		//}

/* TODO : show local IP ,but we can't get localIP that tunnel used.(there are more than one net adapter in a machine)

		// Column five:  <Type> <tab> <width> <tab> <RowCount> <tab> <Name> <newline>	
		tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_STR, dwTunnelCount, L"LocalIP");
		for( i = 0; i < dwTunnelCount;i++)
		{   // column's row-entries
			// entry:  <text> <newline>		
			pTunnel = gTunnelManager._list.at(i);
			tmp += wsprintf(tmp, L"%S\n",pTunnel->getLocalAddr());
		}

		// Column six:  <Type> <tab> <width> <tab> <RowCount> <tab> <Name> <newline>	
		tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_INT, dwTunnelCount, L"LocalPort");
		for( i = 0; i < dwTunnelCount;i++)
		{   // column's row-entries
			// entry:  <text> <newline>		
			pTunnel = gTunnelManager._list.at(i);
			tmp += wsprintf(tmp, L"%d\n",pTunnel->_localPort);
		}
*/

		// Column seven:  <Type> <tab> <width> <tab> <RowCount> <tab> <Name> <newline>	
		tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_STR, dwTunnelCount, L"EstablishTime");
		for( i = 0; i < dwTunnelCount;i++)
		{   // column's row-entries
			// entry:  <text> <newline>		
			disTunnel = Dis_Tunnels.at(i);
			tmp += wsprintf(tmp, L"%S\n",disTunnel.EstablishTime.c_str());
		}


		// Column eight:  <Type> <tab> <width> <tab> <RowCount> <tab> <Name> <newline>	
		tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_STR, dwTunnelCount, L"Owner");
		for( i = 0; i < dwTunnelCount;i++)
		{   // column's row-entries
			// entry:  <text> <newline>		
			disTunnel = Dis_Tunnels.at(i);
			tmp += wsprintf(tmp, L"%S\n",disTunnel.Owner.c_str());			
		}
		tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_STR, dwTunnelCount, L"Status");
		for( i = 0; i < dwTunnelCount;i++)
		{   // column's row-entries
			// entry:  <text> <newline>		
			disTunnel = Dis_Tunnels.at(i);
			tmp += wsprintf(tmp, L"%S\n",disTunnel.status.c_str());			
		}

		Dis_Tunnels.clear();
//		gTunnelManager._lock.leave();
		*pdwLength = wcslen(wsOutPut);
		wchar_t* pOutputBuf = new wchar_t[*pdwLength+1];
		wcscpy(pOutputBuf, wsOutPut);
		*ppwszBuffer = pOutputBuf;
	}
	return MAN_SUCCESS;
}

UINT MCastFwdServ::MgmtConversation(WCHAR *pwszCmd, WCHAR **ppwszBuffer, DWORD *pdwLength)
{
	WORD wCommand;
	swscanf(pwszCmd, L"%c\t", &wCommand);

	// Dispatch based on requested operation
	if (wCommand == MAN_FREE)
	{
		// Free the allocated response buffer
		if (*ppwszBuffer != NULL)
		{
			delete [] *ppwszBuffer;
			*ppwszBuffer = NULL;
		}
		return MAN_SUCCESS;
	}
	else if (wCommand != MAN_READ_VARS)
	{
		return MAN_BAD_PARAM;
	}
	WCHAR *pszTmp = wcschr(pwszCmd, '\t');
	WCHAR *pszWhich = pszTmp + 1;	

	MCastFwdServ* server  = (MCastFwdServ*)Application;

	if (0 == wcsncmp(pszWhich, MCASTFWD_MANPKG_LISTENERS, wcslen(MCASTFWD_MANPKG_LISTENERS)))
	{	
		DWORD dwConversationCount = server->_conversations.size();
		Conversation* pConversation = NULL;
		
		DWORD dwVarCount = 1;
		DWORD dwColumnCount = 5;

		wchar_t wsOutPut[16*1024];
		wchar_t *tmp = wsOutPut;
		memset(wsOutPut,0,16*1024*2);

		//buffer header
		tmp += wsprintf(tmp,L"%d\t%d\n",dwVarCount,dwColumnCount);
		//////////////////////
		// Simple Variables
		//
		// <Type> <tab> <Name> <tab> <Value>
		//

		DWORD  i = 0;
		wchar_t buf [64];
		memset(buf,0,128);
		tmp += wsprintf(tmp, L"%d\t%s\t%d\n", MAN_COUNT, L"Count",dwConversationCount);

		//tmp += wsprintf(tmp, L"%d\t%s\t%d\n", MAN_COUNT, L"ListenerCount",_listenAddrs);


		std::vector<DIS_CONVERSATION> Dis_Converations;	

		{// keep guard live when operate			
			ZQ::common::MutexGuard guard(server->_conversationsLock);
			for(i =0 ; i < dwConversationCount; i ++)
			{
				DIS_CONVERSATION disConversation;
				pConversation = server->_conversations.at(i);
				disConversation.ID = pConversation->_id;
				disConversation.McastGroup = pConversation->_group.getHostAddress();
				disConversation.McastPort = pConversation->_port;

				disConversation.BindIP = pConversation->_bindaddrs;

				DWORD dwTunnlCount = pConversation->_tunnels.size();
				int j;
				for(j = 0; j < dwTunnlCount; j ++)
				{
					TunnelConnection *pTunnel = pConversation->_tunnels.at(i);
					char buf[16];
					if(j < dwTunnlCount -1)
						sprintf(buf,"%d,",pTunnel->_id);
					else 
						sprintf(buf,"%d",pTunnel->_id);
					disConversation.PrivateTunnel += buf;
				}

				Dis_Converations.push_back(disConversation);
			}	

		}//close guard

		/////////////////////
		// Complex table
		// <see above>
		//
		// Column One:  <Type> <tab> <width> <tab> <RowCount> <tab> <Name> <newline>
		tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_INT, dwConversationCount, L"ID");
		DIS_CONVERSATION disConversation;

		for ( i= 0 ; i < dwConversationCount;i++)
		{   // column's row-entries
			// entry:  <text> <newline>
			disConversation = Dis_Converations.at(i);
			tmp += wsprintf(tmp, L"%d\n",disConversation.ID);
		}

		// Column two:  <Type> <tab> <width> <tab> <RowCount> <tab> <Name> <newline>
		tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_STR, dwConversationCount, L"McastGroup");
		for ( i= 0 ; i < dwConversationCount;i++)
		{   // column's row-entries
			// entry:  <text> <newline>
			disConversation = Dis_Converations.at(i);
			tmp += wsprintf(tmp, L"%S\n",disConversation.McastGroup.c_str());
		}

		// Column three:  <Type> <tab> <width> <tab> <RowCount> <tab> <Name> <newline>
		tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_INT, dwConversationCount, L"McastPort");
		for ( i= 0 ; i < dwConversationCount;i++)
		{   // column's row-entries
			// entry:  <text> <newline>
			disConversation = Dis_Converations.at(i);
			tmp += wsprintf(tmp, L"%d\n",disConversation.McastPort);
		}
		// Column four:  <Type> <tab> <width> <tab> <RowCount> <tab> <Name> <newline>
		tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_STR, dwConversationCount, L"BindIP");
		for ( i= 0 ; i < dwConversationCount;i++)
		{   // column's row-entries
			// entry:  <text> <newline>
			disConversation = Dis_Converations.at(i);
			tmp += wsprintf(tmp, L"%S\n",disConversation.BindIP.c_str());
		}

		// Column two:  <Type> <tab> <width> <tab> <RowCount> <tab> <Name> <newline>
		tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_STR, dwConversationCount, L"PrivateTunnel");
		for ( i= 0 ; i < dwConversationCount;i++)
		{   // column's row-entries
			// entry:  <text> <newline>
			disConversation = Dis_Converations.at(i);
			tmp += wsprintf(tmp, L"%S\n",disConversation.PrivateTunnel.c_str());
		}

		Dis_Converations.clear();
		// Column six:  <Type> <tab> <width> <tab> <RowCount> <tab> <Name> <newline>	
		/*		tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_STR, dwConversationCount, L"Peers");
		for (int i= 0 ; i < _conversations.size();it++)
		{   // column's row-entries
		// entry:  <text> <newline>

		//			wchar_t buf[256];
		std::string str; 
		pConversation = _conversations.at(i);
		for (int j = 0; j <pConversation->_tunnels.size(); j++ )
		{	if(str.empty())	
		{
		str += ";"
		}
		TunnelConnection * pTunnel;
		str += pTunnel->peerid().toString();				
		}
		tmp += wsprintf(tmp, L"%S\n",str.c_str());
		}
		*/
		*pdwLength = wcslen(wsOutPut);
		wchar_t* pOutputBuf = new wchar_t[*pdwLength+1];
		wcscpy(pOutputBuf, wsOutPut);
		*ppwszBuffer = pOutputBuf;
	}
	return MAN_SUCCESS;
}

UINT MCastFwdServ::MgmtResenders(WCHAR *pwszCmd, WCHAR **ppwszBuffer, DWORD *pdwLength)
{
	WORD wCommand;
	swscanf(pwszCmd, L"%c\t", &wCommand);

	// Dispatch based on requested operation
	if (wCommand == MAN_FREE)
	{
		// Free the allocated response buffer
		if (*ppwszBuffer != NULL)
		{
			delete [] *ppwszBuffer;
			*ppwszBuffer = NULL;
		}
		return MAN_SUCCESS;
	}
	else if (wCommand != MAN_READ_VARS)
	{
		return MAN_BAD_PARAM;
	}
	WCHAR *pszTmp = wcschr(pwszCmd, '\t');
	WCHAR *pszWhich = pszTmp + 1;	


	MCastFwdServ* server = (MCastFwdServ*) Application;
	if (0 == wcsncmp(pszWhich, MCASTFWD_MANPKG_RESENDERS, wcslen(MCASTFWD_MANPKG_RESENDERS)))
	{	
		DWORD dwResenderCount = server->_remcasters.size();
		ZQ::common::UDPMulticast* pResender = NULL;
		Conversation * pConversation = NULL;

		DWORD dwVarCount =2;
		DWORD dwColumnCount = 3;

		wchar_t wsOutPut[4096];
		wchar_t *tmp = wsOutPut;
		memset(wsOutPut,0,4096*2);

		//buffer header
		tmp += wsprintf(tmp,L"%d\t%d\n",dwVarCount,dwColumnCount);
		//////////////////////
		// Simple Variables
		//
		// <Type> <tab> <Name> <tab> <Value>
		//
		DWORD i;
		for( i = 0; i < server->_conversations.size();i++)
		{
			pConversation = server->_conversations.at(i);
			dwResenderCount += pConversation->_remcasters.size();
		}


		tmp += wsprintf(tmp, L"%d\t%s\t%d\n", MAN_INT, L"ReMcastTTL",m_dwReMcastTTL);
		tmp += wsprintf(tmp, L"%d\t%s\t%d\n", MAN_COUNT, L"Count",dwResenderCount);

		/// get resend info
		std::vector<DIS_REMCASTER> RemcasterList;
		{
			for ( i = 0; i < server->_remcasters.size(); i++)
			{   // column's row-entries
				// entry:  <text> <newline>
				DIS_REMCASTER remcaster;
				ZQ::common::tpport_t port;
				pResender = server->_remcasters.at(i);								
				remcaster.BindIP = pResender->getLocal(&port).getHostAddress();
				remcaster.bindPort = port;
				remcaster.Owner = "-1";
				RemcasterList.push_back(remcaster);
			}

			for( i = 0; i < server->_conversations.size();i++)
			{
				pConversation = server->_conversations.at(i);
				for(DWORD j = 0; j < pConversation->_remcasters.size();j++)
				{
					DIS_REMCASTER remcaster;
					ZQ::common::tpport_t port;
					pResender = server->_remcasters.at(i);								
					remcaster.BindIP = pResender->getLocal(&port).getHostAddress();
					remcaster.bindPort = port;
					char buf[16];
					memset(buf,0,16);
					remcaster.Owner = itoa(pConversation->_id,buf,16);
					RemcasterList.push_back(remcaster);
				}
			}

		}

		/////////////////////
		// Complex table
		// <see above>
		//
		// Column One:  <Type> <tab> <width> <tab> <RowCount> <tab> <Name> <newline>


		tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_STR, dwResenderCount, L"BindIP");
		for ( i = 0; i <dwResenderCount; i++)
		{   // column's row-entries
			// entry:  <text> <newline>
			tmp += wsprintf(tmp, L"%S\n",RemcasterList.at(i).BindIP.c_str());
		}

		
		// Column Two:  <Type> <tab> <width> <tab> <RowCount> <tab> <Name> <newline>
		tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_INT, dwResenderCount, L"BindPort");
		for ( i = 0; i <dwResenderCount; i++)
		{   // column's row-entries
			// entry:  <text> <newline>
			tmp += wsprintf(tmp, L"%d\n",RemcasterList.at(i).bindPort);
		}

		// Column Three:  <Type> <tab> <width> <tab> <RowCount> <tab> <Name> <newline>
		tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_INT, dwResenderCount, L"Owner");
		for ( i = 0; i <dwResenderCount; i++)
		{   // column's row-entries
			// entry:  <text> <newline>
			tmp += wsprintf(tmp, L"%S\n",RemcasterList.at(i).Owner.c_str());
		}

		*pdwLength = wcslen(wsOutPut);
		wchar_t* pOutputBuf = new wchar_t[*pdwLength+1];
		wcscpy(pOutputBuf, wsOutPut);
		*ppwszBuffer = pOutputBuf;
	}
	return MAN_SUCCESS;
}

UINT MCastFwdServ::MgmtDenylist(WCHAR *pwszCmd, WCHAR **ppwszBuffer, DWORD *pdwLength)
{
	WORD wCommand;
	swscanf(pwszCmd, L"%c\t", &wCommand);

	// Dispatch based on requested operation
	if (wCommand == MAN_FREE)
	{
		// Free the allocated response buffer
		if (*ppwszBuffer != NULL)
		{
			delete [] *ppwszBuffer;
			*ppwszBuffer = NULL;
		}
		return MAN_SUCCESS;
	}
	else if (wCommand != MAN_READ_VARS)
	{
		return MAN_BAD_PARAM;
	}
	WCHAR *pszTmp = wcschr(pwszCmd, '\t');
	WCHAR *pszWhich = pszTmp + 1;	
	MCastFwdServ* server = (MCastFwdServ*)Application;
	if (0 == wcsncmp(pszWhich, MCASTFWD_MANPKG_DENYLIST, wcslen(MCASTFWD_MANPKG_DENYLIST)))
	{	
		DWORD dwDenyListCount = server->_localDenylist.size();
		Conversation * pConversation = NULL;
		node_t* pNode = NULL;


		DWORD dwVarCount = 1;
		DWORD dwColumnCount = 4;

		wchar_t wsOutPut[4096];
		wchar_t *tmp = wsOutPut;
		memset(wsOutPut,0,4096*2);

		//buffer header
		tmp += wsprintf(tmp,L"%d\t%d\n",dwVarCount,dwColumnCount);
		//////////////////////
		// Simple Variables
		//
		// <Type> <tab> <Name> <tab> <Value>
		//
		DWORD k ;
		for ( k = 0; k < server->_conversations.size();k++)
		{
			pConversation = server->_conversations.at(k);
			dwDenyListCount += pConversation->_localDenylist.size();
		}
		dwDenyListCount += Conversation::_senderList.size();
		
		

		tmp += wsprintf(tmp, L"%d\t%s\t%d\n", MAN_COUNT, L"Count",dwDenyListCount);

		///collect the information of denylist
		std::vector<DIS_DENY> DisDenyList;
		{
			
			for(int i = 0;i < Conversation::_senderList.size();i++)
			{
				DIS_DENY deny;
				node_t Node;
				Conversation::_senderList.getNode(&Node,i);
				deny.DenyIP = Node.host.getHostAddress();
				deny.Mask = Node.mask.getHostAddress();
				deny.Port = Node.port;
				deny.Owner = "-1";
				DisDenyList.push_back(deny);
			}
			
			for ( i = 0; i < server->_localDenylist.size(); i++)
			{   // column's row-entries
				// entry:  <text> <newline>
				node_t Node;
				DIS_DENY deny;
				server->_localDenylist.getNode(&Node,i);
				deny.DenyIP = Node.host.getHostAddress();
				deny.Mask = Node.mask.getHostAddress();
				deny.Port = Node.port;
				deny.Owner = "0";
				DisDenyList.push_back(deny);
			}

			for(k = 0; k< server->_conversations.size(); k++)
			{
				pConversation = server->_conversations.at(k);
				for(int j =0 ; j< pConversation->_localDenylist.size(); j ++)
				{
					node_t Node;
					DIS_DENY deny;
					pConversation->_localDenylist.getNode(&Node,j);
					deny.DenyIP = Node.host.getHostAddress();
					deny.Mask = Node.mask.getHostAddress();
					deny.Port = Node.port;
					char buf [16];
					memset(buf,0,16);
					deny.Owner = itoa(pConversation->_id,buf,10);
					DisDenyList.push_back(deny);
				}
			}

		}

		/////////////////////
		// Complex table
		// <see above>
		//
		// Column One:  <Type> <tab> <width> <tab> <RowCount> <tab> <Name> <newline>
		tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_STR, dwDenyListCount, L"Net");
		DWORD i =0;
		for(i= 0; i < dwDenyListCount; i++)
		{
			tmp += wsprintf(tmp, L"%S\n",DisDenyList.at(i).DenyIP.c_str());
		}

		// Column TWO:  <Type> <tab> <width> <tab> <RowCount> <tab> <Name> <newline>
		tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_STR, dwDenyListCount, L"Mask");
		for(i= 0; i < dwDenyListCount; i++)
		{
			tmp += wsprintf(tmp, L"%S\n",DisDenyList.at(i).Mask.c_str());
		}
		// Column three:  <Type> <tab> <width> <tab> <RowCount> <tab> <Name> <newline>
		tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_INT, dwDenyListCount, L"Port");
		for(i= 0; i < dwDenyListCount; i++)
		{
			tmp += wsprintf(tmp, L"%d\n",DisDenyList.at(i).Port);
		}
		// Column four:  <Type> <tab> <width> <tab> <RowCount> <tab> <Name> <newline>
		tmp += wsprintf(tmp, L"%d\t0\t%d\t%s\n", MAN_STR, dwDenyListCount, L"Owner");
		for(i= 0; i < dwDenyListCount; i++)
		{
			tmp += wsprintf(tmp, L"%S\n",DisDenyList.at(i).Owner.c_str());
		}

		*pdwLength = wcslen(wsOutPut);
		wchar_t* pOutputBuf = new wchar_t[*pdwLength+1];
		wcscpy(pOutputBuf, wsOutPut);
		*ppwszBuffer = pOutputBuf;
	}
	return MAN_SUCCESS;
}

