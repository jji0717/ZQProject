#include "stdafx.h"
#include "ConfigPage.h"

//////////////////////////////////////////////////////////////////////////////////////////
//class CBroadcastConfigPage
CUnknown * WINAPI CBroadcastConfigPage::CreateInstance(LPUNKNOWN lpUnknown,HRESULT *pHr)
{
	CUnknown* pUnknown = new CBroadcastConfigPage(lpUnknown, pHr);

	return pUnknown;
}

CBroadcastConfigPage::CBroadcastConfigPage(LPUNKNOWN lpUnknown,HRESULT *pHr):
m_pBMLConfig(0), 
m_pIPConfig(0),
//m_hWndFile(0),
CBasePropertyPage(NAME("BML Configuration"), lpUnknown, IDD_DIALOG_BMLCONFIGURATIONPAGE, IDS_BMLCONFIGURATIONPAGE_TITLE)
{
	*pHr = S_OK;
	m_bDirty = 1;
}

CBroadcastConfigPage::~CBroadcastConfigPage()
{
	if( m_pBMLConfig )
	{
		m_pBMLConfig->Release();
		m_pBMLConfig = 0;
	}
	if( m_pIPConfig )
	{
		m_pIPConfig->Release();
		m_pIPConfig = 0;
	}
}
HRESULT CBroadcastConfigPage::OnConnect(IUnknown *pUnknown)
{
	if( m_pBMLConfig )
	{
		m_pBMLConfig->Release();
		m_pBMLConfig = 0;
	}
	HRESULT hr = pUnknown->QueryInterface(IID_IRateControl, (void **)&m_pBMLConfig);
	if( FAILED(hr) )
	{
		m_pBMLConfig = 0;
		return E_NOINTERFACE;
	}
	if( m_pIPConfig )
	{
		m_pIPConfig->Release();
		m_pIPConfig = 0;
	}
	hr = pUnknown->QueryInterface(IID_IIPSetting, (void **)&m_pIPConfig);
	if( FAILED(hr) )
	{
		m_pIPConfig = 0;
		return E_NOINTERFACE;
	}

	return NOERROR;
}

HRESULT CBroadcastConfigPage::OnDisconnect()
{
	if( m_pBMLConfig )
	{
		m_pBMLConfig->Release();
		m_pBMLConfig = 0;
	}
	if( m_pIPConfig )
	{
		m_pIPConfig->Release();
		m_pIPConfig = 0;
	}
	return NOERROR;
}

HRESULT CBroadcastConfigPage::OnActivate()
{
	ReflectBML();
	return NOERROR;
}

HRESULT CBroadcastConfigPage::OnDeactivate()
{
	return NOERROR;
}

HRESULT CBroadcastConfigPage::OnApplyChanges()
{
	EnterBML();
	return NOERROR;
}

INT_PTR CBroadcastConfigPage::OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	if( uMsg==WM_INITDIALOG )
	{
		m_hWndChannelCount = GetDlgItem(hwnd, IDC_CHANNELCOUTT );
		m_hWndIndex = GetDlgItem(hwnd, IDC_NINDEX);
		m_hWndPID = GetDlgItem(hwnd, IDC_PID);
		m_hWndRate = GetDlgItem(hwnd, IDC_RATE);

//((CComboBox*)GetDlgItem(IDC_Combox1))->SetCurSel(0);
	//	ComboBox aa= GetDlgItem(hwnd, IDC_Combox1);
	}
	else if( uMsg==WM_COMMAND )
	{
	}
	return CBasePropertyPage::OnReceiveMessage(hwnd, uMsg, wParam, lParam);
}

HRESULT CBroadcastConfigPage::ReflectBML()
{
	USES_CONVERSION;

	HRESULT hr = S_OK;
//	if( m_hWndBML==0 || m_hWndFile==0 || m_pBMLConfig==0 )
//	{
//		return E_FAIL;
	//}
	LPOLESTR lpBML = 0;
	LPOLESTR lpFile = 0;

//	hr = m_pBMLConfig->GetBMLConfigure(&lpBML, &lpFile);
	if( FAILED(hr) )
	{
		return hr;
	}

//	SetWindowText(m_hWndBML, OLE2A(lpBML) );
//	SetWindowText(m_hWndFile, OLE2A(lpFile) );

	return S_OK;
}
HRESULT CBroadcastConfigPage::EnterBML()
{
	USES_CONVERSION;
	HRESULT hr = S_OK;

	char strBML[MAX_PATH];
	//	char strFile[MAX_PATH];

	GetWindowText(m_hWndChannelCount, strBML, MAX_PATH-1);
	int kk=atoi(strBML);
	kk=1;
	hr = m_pBMLConfig->SetChannelCount(1);

	//m_pBMLConfig->SetTotalRate(800);//is 800k
	m_pBMLConfig->SetTotalRate(500);//is 500k
	m_pBMLConfig->SetFilePath("D:\\temp");

	ZQSBFCHANNELINFO aa;

	/*
	WORD wPID;			// Channel PID.
	WORD wRate;			// Kilo bits 
	WORD wAccessType;	// MEMORYTYPE=0 :FILETYPE=1
	BOOL bEnable;		// Channel anble or disable.
	// ------------------------------------------------------ Modified by zhenan_ji at 2005Äê3ÔÂ17ÈÕ 9:55:51
	char cDescriptor[4];
	//for
	WORD wRepeatTime;	// for subchinna
	BOOL bBeDetect;		// 	Automotion
	WORD wBeDetectInterval; //manual time .
	WORD wBeEncrypted;   
	WORD wChannelType;
	WORD nStreamType;
	char 		szPath[MAX_PATH];		//Catalog path 
*/

	aa.wPID=0xbf;
	aa.wRate=400;
	aa.wAccessType=1;
	aa.bEnable=TRUE;
	strcpy(aa.cDescriptor,"HIER");
	aa.nStreamType=188;	
	hr = m_pBMLConfig->ConfigChannel(0,&aa);


ZQSBFIPPORTINFO *bb[1];

bb[0]= new ZQSBFIPPORTINFO;
	bb[0]->wSendType=1;//UDP
	strcpy(bb[0]->cDestIp, "10.30.4.100");
	bb[0]->wSourcePort=3467;
	bb[0]->wDestPort=2000;
	strcpy(bb[0]->cSourceIp, "192.168.80.145");



	//strcpy(bb[0]->cDestIp, "192.168.80.81");
	//strcpy(bb[0]->cDestIp, "192.168.80.61");
 
	m_pIPConfig->SetIPPortConfig(bb,1);
	m_pBMLConfig->SetPmtPID(0x128);

 //delete bb[0];
 //bb[0]=NULL;

	return hr;
}