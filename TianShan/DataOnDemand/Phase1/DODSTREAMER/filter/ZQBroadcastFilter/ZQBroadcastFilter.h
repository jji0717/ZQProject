//------------------------------------------------------------------------------
// File: Dump.h
//
// Desc: DirectShow sample code - definitions for dump renderer.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------
//#pragma once
class CChannelManager;
class CChannel;
class CBufferSend;
class CBroadcastFilter;

class CBroadcastPin : public CBaseInputPin
{
public:
	CBroadcastPin(TCHAR   *pName,CBroadcastFilter *pFilter,
		HRESULT *phr,LPOLESTR  pPinName);
	HRESULT HandleWriteFailure();
	~CBroadcastPin();
	HRESULT	CheckMediaType(const CMediaType *pMediaType);
	HRESULT BreakConnect();
	HRESULT Active();
	STDMETHODIMP Receive(IMediaSample *pSample);
	CChannelManager *  m_pChannelManager;

	//when recive data,current write operation relative filename.
	char m_cCurFileName[MAX_PATH];	

	//current operation file.
	HANDLE m_hFile;

	//denote self index in all channel gather;
	int m_nIndex;

	//temp variable for memory mode
	long m_RecvLength;
	BOOL m_bPinEnable;
	
	//temp variable for memory mode
	BYTE * m_pcReceiveData;
};

#ifndef _NO_FIX_LOG
#include "fltinit.h"
#endif

class CBroadcastFilter : public CCritSec, 
	public CBaseFilter, 
	public IRateControl, 
	public IIPSetting, 
#ifndef _NO_FIX_LOG
	public IFilterInit, 
#endif
	public ISpecifyPropertyPages
{
public:
	// Constructor
	DECLARE_IUNKNOWN
	CBroadcastFilter(LPUNKNOWN pUnk,HRESULT *phr);
	~CBroadcastFilter();    
	static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);
	// Pin enumeration
	CBasePin * GetPin(int n);
	int GetPinCount();
	CCritSec m_Lock;    
	// Open and close the file as necessary
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);	
	STDMETHODIMP GetPages(CAUUID* pPages);
	STDMETHODIMP Run(REFERENCE_TIME tStart);
	STDMETHODIMP Stop();

	STDMETHODIMP SetChannelCount( DWORD dwChannelCount ); 
	STDMETHODIMP ConfigChannel( int nPinIndex, ZQSBFCHANNELINFO* pChannelInfo); 
	STDMETHODIMP SetTotalRate(DWORD wTotalRate); 
	STDMETHODIMP SetFilePath(char *cfilepath); 
	STDMETHODIMP SetPmtPID( WORD wPID); 
	STDMETHODIMP Enable( WORD wPID, BOOL bAnable);
	STDMETHODIMP GetChannelInfo( WORD wPID, ZQSBFCHANNELINFO* pChannelInfo);
	STDMETHODIMP GetTotalRate(DWORD* pwTotalRate); 
	STDMETHODIMP GetFilePath(char *cfilepath); 
	STDMETHODIMP GetPmtPID( WORD* pwPID); 

	STDMETHODIMP SetIPPortConfig( ZQSBFIPPORTINFO* pInfoList[], int nCount); 
	STDMETHODIMP GetIPPortConfig( int nIndex, ZQSBFIPPORTINFO* pInfo);

	// added by Cary
#ifndef _NO_FIX_LOG
	virtual void initLog(ISvcLog* log)
	{
		extern ISvcLog* service_log;
		service_log = log;
	}
#endif

public:

	//director all channel and port
	CChannelManager *  m_pChannelManager;

	//check all channel sendpacket 's sum + 2(pat, pmt) < totalsendpacket;
	int m_nAllChannelRatePacketNumber;
};