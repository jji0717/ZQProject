// ActiveSession.h: interface for the ActiveSession class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ACTIVESESSION_H__E40A2152_1D7E_4566_9997_40DBDD85B23D__INCLUDED_)
#define AFX_ACTIVESESSION_H__E40A2152_1D7E_4566_9997_40DBDD85B23D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning(disable:4786)

#include <list>
#include <string>
using namespace std;

#include "McpCommon.h"
#include "McpSession/McpSession.h"
class ActiveSession  : public McpSession
{
public:
	ActiveSession();
	virtual ~ActiveSession();

public:
	void SetVideoBitRate(int nRate);
	void SetVideoVSize(int nSize);
	void SetVideoHSize(int nSize);
	BOOL IsDone();
	HRESULT WaitForFinish(long nTimeOut);
	void Stop();
	int Start();

    void SetMulticastAddress(BSTR strAddr);
	void SetSourcePort(UINT uPort);
	void SetDestport(UINT uPort);
	void SetPublishFilename(BSTR strFileName);
	void SetSubscribeFilename(BSTR strFileName);
	void SetPublisherIP(BSTR strAddr);
	void SetPublisherInterfaceIP(BSTR strAddr);
	void SetTransportRate(UINT uRate);
	void SetLiveTransfer(BOOL b);
	void SetEnableVodpkg(BOOL b);
	HRESULT AddSubscriber(BSTR ip1,BSTR ip2);

	
    void GetMulticastAddress(BSTR *strAddr);
	void GetSourcePort(long *uPort);
	void GetDestport(long *uPort);
	void GetPublishFilename(BSTR *strFileName);
	void GetSubscribeFilename(BSTR* strFileName);
	void GetPublisherIP(BSTR *strAddr);
	void GetPublisherInterfaceIP(BSTR *strAddr);
	void GetTransportRate(long *uRate);
	void GetLiveTransfer(BOOL *b);
	void GetEnableVodpkg(BOOL *b);

private:
	int SetParam();

private://property
	wchar_t		m_szMulticastAddress[MAX_IP_SIZE];
	UINT		m_uSourcePort;
	UINT		m_uDestPort;
	wchar_t		m_szPublishFilename[MAX_FILENAME_SIZE];
	wchar_t		m_szSubscribeFilename[MAX_FILENAME_SIZE];
	wchar_t		m_szPublisherIP[MAX_IP_SIZE];
	wchar_t		m_szPublisherInterfaceIP[MAX_IP_SIZE];
	int			m_nCodingError;
	DWORD		m_dwTransportBitRate;
	BOOL		m_bLiveTransfer;
	BOOL		m_bEnableVodpkg;
	
	int			m_nVideoHSize;
	int			m_nVideoVSize;
	int			m_nVideoBitRate;

	list<wstring> m_SubscriberIpList;
	list<wstring> m_SubscriberInterIpList;
};

#endif // !defined(AFX_ACTIVESESSION_H__E40A2152_1D7E_4566_9997_40DBDD85B23D__INCLUDED_)
