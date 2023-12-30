// ActiveSession.cpp: implementation of the ActiveSession class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ActiveSession.h"
#include "McpCommon.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ActiveSession::ActiveSession()
{
	wcscpy(m_szMulticastAddress,L"225.20.10.5");
	ZeroMemory(m_szPublishFilename,MAX_FILENAME_SIZE);
	ZeroMemory(m_szSubscribeFilename,MAX_FILENAME_SIZE);
	ZeroMemory(m_szPublisherIP,MAX_IP_SIZE);
	ZeroMemory(m_szPublisherInterfaceIP,MAX_IP_SIZE);
	
	m_dwTransportBitRate=-1;

	m_uSourcePort=5000;
	m_uDestPort=6000;

	m_SubscriberIpList.clear();
	m_SubscriberInterIpList.clear();

	m_bLiveTransfer=TRUE;
	m_bEnableVodpkg=TRUE;

	
	m_nVideoHSize=0;
	m_nVideoVSize=0;
	m_nVideoBitRate=0;
}

ActiveSession::~ActiveSession()
{
	m_SubscriberIpList.clear();
	m_SubscriberInterIpList.clear();

}

int ActiveSession::SetParam()
{
	
	if(m_szMulticastAddress[0]==0)
	{
#ifdef DEBUG
		OutputDebugString(L"SetParam In MCPPublish.dll Error Occur, \
								and MultiAddress is Invalid!\n");
#endif
		return -1;
	}

	if(m_szPublishFilename[0]==0)
	{
#ifdef DEBUG
		OutputDebugString(L"SetParam In MCPPublish.dll Error Occur, \
								and PublishFileName is Invalid!\n");
#endif
		return -2;
	}

	if(m_szSubscribeFilename[0]==0)
	{
#ifdef DEBUG
		OutputDebugString(L"SetParam In MCPPublish.dll Error Occur, \
								and SubscribeFilename is Invalid!\n");
#endif
		return -3;
	}

	if(m_szPublisherIP[0]==0)
	{
#ifdef DEBUG
		OutputDebugString(L"SetParam In MCPPublish.dll Error Occur, \
								and PublisherIP is Invalid!\n");
#endif
		return -4;
	}

	if(m_dwTransportBitRate<0)
	{
#ifdef DEBUG
		OutputDebugString(L"SetParam In MCPPublish.dll Error Occur, \
								and TransportBitRate is Invalid!\n");
#endif
		return -5;
	}

	if(m_SubscriberIpList.size()==0)
	{
#ifdef DEBUG
		OutputDebugString(L"SetParam In MCPPublish.dll Error Occur, \
								and m_SubscriberIpList is Empty!\n");
#endif
		return -6;
	}

	if(m_bEnableVodpkg)
		setVODpkgAttr(true, true, true, false, true);
	else
		setVODpkgAttr(false, true, true, false, true);

	if(m_bLiveTransfer)
		setXferAttr(MCP_XFER_LIVE, m_dwTransportBitRate, MCP_RTE_STREAM_VERSION_ONE, true, MCP_TIMEOUT);
	else
		setXferAttr(MCP_XFER_NORMAL, m_dwTransportBitRate, MCP_RTE_STREAM_VERSION_ONE, true, MCP_TIMEOUT);

	setMcpAttr(m_szMulticastAddress,MCP_EVENTINTERVAL,1);  // sleep one second After the session ends or fails
	
	setMediaAttr(m_szPublishFilename,
		m_nVideoBitRate,
		(unsigned short)m_nVideoVSize,
		(unsigned short)m_nVideoHSize,
		NULL,
		NULL
		);
	

	setPublisher(m_szPublisherIP, m_uSourcePort, m_szPublisherInterfaceIP);

	while(m_SubscriberIpList.size()!=0)
	{
		wstring ip1,ip2;
		ip1=m_SubscriberIpList.front();
		ip2=m_SubscriberInterIpList.front();

		addSubscriber(m_szSubscribeFilename,(BSTR)ip1.c_str(),m_uDestPort,(BSTR)ip2.c_str());

		m_SubscriberIpList.pop_front();
		m_SubscriberInterIpList.pop_front();
	}

	return 1;//success
}


void ActiveSession::SetMulticastAddress(BSTR strAddr)
{
	ZeroMemory(m_szMulticastAddress,MAX_IP_SIZE);
	wcscpy(m_szMulticastAddress,strAddr);
}

void ActiveSession::SetSourcePort(UINT uPort)
{
	m_uSourcePort=uPort;
}

void ActiveSession::SetDestport(UINT uPort)
{
	m_uDestPort=uPort;
}

void ActiveSession::SetPublishFilename(BSTR strFileName)
{

	ZeroMemory(m_szPublishFilename,MAX_FILENAME_SIZE);
	wcscpy(m_szPublishFilename,strFileName);
}

void ActiveSession::SetSubscribeFilename(BSTR strFileName)
{
	ZeroMemory(m_szSubscribeFilename,MAX_FILENAME_SIZE);
	wcscpy(m_szSubscribeFilename,strFileName);
}

void ActiveSession::SetPublisherIP(BSTR strAddr)
{
	ZeroMemory(m_szPublisherIP,MAX_IP_SIZE);
	wcscpy(m_szPublisherIP,strAddr);
}

void ActiveSession::SetPublisherInterfaceIP(BSTR strAddr)
{
	ZeroMemory(m_szPublisherInterfaceIP,MAX_IP_SIZE);
	wcscpy(m_szPublisherInterfaceIP,strAddr);
}

HRESULT ActiveSession::AddSubscriber(BSTR ip1,BSTR ip2)
{
	if(ip1==NULL || *ip1==0)return S_FALSE;

	m_SubscriberIpList.push_back(ip1);
	m_SubscriberInterIpList.push_back(ip2);
	return S_OK;
}

void ActiveSession::SetTransportRate(UINT uRate)
{
	m_dwTransportBitRate=uRate;
}


int ActiveSession::Start()
{
	int nRet;
	if(!(nRet=SetParam()))
		return nRet;

	if(!start())
	{
#ifdef DEBUG
		OutputDebugString(L"In McpPublish.dll Start() ,start thread fail\n");
#endif
		return -10;
	}

	return 1;
}

void ActiveSession::Stop()
{
	_bKeepPolling=false;
}

HRESULT ActiveSession::WaitForFinish(long nTimeOut)
{
	DWORD dwTimeToWait = nTimeOut;

	//wait for  mcp publish stopping	
	DWORD  dwTimeWaited = 0;
	while(GetHandle() && !wait(dwTimeToWait))
	{
		// send keep alive info to client until the mcp publish finish or error

		dwTimeWaited += dwTimeToWait;

		if (dwTimeWaited >= TIMEOUT_UPLOAD_PUBLISH)
		{
//			wchar_t szbuf[200];
//			wsprintf(szbuf,L"waiting for Publish Finish time out %d times, stop waiting.\n", 
//															TIMEOUT_UPLOAD_PUBLISH/1000);

			terminate();
			
			return S_FALSE;//TIME OUT
		}
	}

//	OutputDebugString(L"WaitForFinish normal exit\n");
	return S_OK;
}


BOOL ActiveSession::IsDone()
{
	if(getCurStatus() == MCP_SESSION_CLOSED)
		return TRUE;
	else if(getCurStatus() == MCP_SESSION_FAILED)
		return FALSE;

	return FALSE;
}

void ActiveSession::SetLiveTransfer(BOOL b)
{
	m_bLiveTransfer=b;
}

void ActiveSession::SetEnableVodpkg(BOOL b)
{
	m_bEnableVodpkg=b;
}

void ActiveSession::GetMulticastAddress(BSTR *strAddr)
{
	*strAddr=m_szMulticastAddress;
}

void ActiveSession::GetSourcePort(long *uPort)
{
	*uPort=m_uSourcePort;
}

void ActiveSession::GetDestport(long *uPort)
{
	*uPort=m_uDestPort;
}

void ActiveSession::GetPublishFilename(BSTR *strFileName)
{
	*strFileName=m_szPublishFilename;
}

void ActiveSession::GetSubscribeFilename(BSTR* strFileName)
{
	*strFileName=m_szSubscribeFilename;
}

void ActiveSession::GetPublisherIP(BSTR *strAddr)
{
	*strAddr=m_szPublisherIP;
}

void ActiveSession::GetPublisherInterfaceIP(BSTR *strAddr)
{
	*strAddr=m_szPublisherInterfaceIP;
}

void ActiveSession::GetTransportRate(long *uRate)
{
	*uRate=m_dwTransportBitRate;
}

void ActiveSession::GetLiveTransfer(BOOL *b)
{
	*b=m_bLiveTransfer;
}

void ActiveSession::GetEnableVodpkg(BOOL *b)
{
	*b=m_bEnableVodpkg;
}

void ActiveSession::SetVideoHSize(int nSize)
{
	m_nVideoHSize=nSize;
}

void ActiveSession::SetVideoVSize(int nSize)
{
	m_nVideoVSize=nSize;
}

void ActiveSession::SetVideoBitRate(int nRate)
{
	m_nVideoBitRate=nRate;
}
