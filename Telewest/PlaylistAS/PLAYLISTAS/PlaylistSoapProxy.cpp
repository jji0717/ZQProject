// PlaylistSoapProxy.cpp: implementation of the PlaylistSoapProxy class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "PlaylistSoapProxy.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PlaylistSoapProxy::PlaylistSoapProxy(wchar_t* par_WsdlFile, wchar_t* par_WsmlFile, wchar_t* par_ServiceName, wchar_t* par_Port, wchar_t* par_Namespace)
{
	wcscpy(_wszSoapWSDLFilePath, par_WsdlFile);
	wcscpy(_wszSoapWSMLFilePath, par_WsmlFile);
	wcscpy(_wszSoapServiceName, par_ServiceName);
	wcscpy(_wszSoapPort, par_Port);
	wcscpy(_wszSoapNamespace, par_Namespace);

	_dwTimeout = DEFAULT_TIMEOUT;
	m_bInstanced = FALSE;
}

PlaylistSoapProxy::~PlaylistSoapProxy()
{
	if(m_bInstanced)
		m_pSoapClient.Release();
	
	m_pSoapClient = NULL;
	m_bInstanced = FALSE;

	CoUninitialize();
}

void PlaylistSoapProxy::RaiseError(LPOLESTR pMessage, HRESULT hr)
{
	ICreateErrorInfoPtr pCreateErrorInfo;
	IErrorInfoPtr pErrorInfo;

	if( SUCCEEDED(CreateErrorInfo(&pCreateErrorInfo)) )
	{

		pCreateErrorInfo->SetSource(L"comsup");
		pCreateErrorInfo->SetDescription(pMessage);

		pCreateErrorInfo.QueryInterface(__uuidof(IErrorInfo), &pErrorInfo);

		_com_raise_error(hr, pErrorInfo.Detach());

	}
	else
	{
		_com_raise_error(hr, NULL);
	}
}

void PlaylistSoapProxy::RaiseError(LPOLESTR pMessage, HRESULT hr, EXCEPINFO& excepinfo, DISPPARAMS& params, unsigned int uArgErr)
{
	_bstr_t Msg = pMessage;
	wchar_t szArgErr[34];

	switch(hr)
	{

	case DISP_E_EXCEPTION:
		Msg += " ";
		Msg += excepinfo.bstrDescription;
		SysFreeString(excepinfo.bstrSource);
		SysFreeString(excepinfo.bstrDescription);
		SysFreeString(excepinfo.bstrHelpFile);
		hr = excepinfo.scode;
		break;

	case DISP_E_TYPEMISMATCH:
		if(uArgErr != -1)
		{
			Msg += " Error in parameter ";
			Msg += _ultow(params.cArgs - uArgErr, szArgErr, 10);
		}
		break;

	}

	RaiseError(Msg, hr);
}

void PlaylistSoapProxy::Initialize()
{
	HRESULT hr;

	// Create a SoapClient object.
	CoInitialize(NULL);
	hr = m_pSoapClient.CreateInstance(__uuidof(SoapClient30));
	if( FAILED(hr) ) 
	{
		RaiseError(L"Cannot create SoapClient30 object.", hr);
	}

	m_bInstanced = TRUE;
	
	// initialize
	try
	{
//		m_pSoapClient->MSSoapInit2(
//			_variant_t(_wszSoapWSDLFilePath), 
//			_variant_t(_wszSoapWSMLFilePath), 
//			_bstr_t(_wszSoapServiceName),
//			_bstr_t(_wszSoapPort), 
//			_bstr_t(_wszSoapNamespace) );

		m_pSoapClient->MSSoapInit2(
			_variant_t(_wszSoapWSDLFilePath), 
			_variant_t(_wszSoapWSMLFilePath), 
			_bstr_t(L""),
			_bstr_t(L""),
			_bstr_t(L"") );
	}
	catch( _com_error Error )
	{

		_bstr_t Msg;
		Msg += L"Initializing the SoapClient object failed. ";
		Msg += Error.Description();

		RaiseError(Msg, Error.Error());
	}

	// set timeout property
	try
	{
		VARIANT vt;
		vt.vt = VT_I4;
		vt.lVal = _dwTimeout;
		hr = m_pSoapClient->put_ConnectorProperty(L"ConnectTimeout", vt);
		if(FAILED(hr))
		{
			_bstr_t Msg;
			Msg += L"Set SoapClient connect timeout failed.";
			RaiseError(Msg, hr);
		}

		hr = m_pSoapClient->put_ConnectorProperty(L"Timeout", vt);
		if(FAILED(hr))
		{
			_bstr_t Msg;
			Msg += L"Set SoapClient timeout failed.";
			RaiseError(Msg, hr);
		}
	}
	catch( _com_error Error )
	{

		_bstr_t Msg;
		Msg += L"Set the SoapClient time property failed. ";
		Msg += Error.Description();

		RaiseError(Msg, Error.Error());
	}
}

DISPID PlaylistSoapProxy::GetIDOfName(LPOLESTR lpName)
{
	HRESULT hr;
	DISPID dispid;

	if(m_pSoapClient == NULL) {
		RaiseError(L"SoapClient is not valid.", E_FAIL); 
	}

	hr = m_pSoapClient->GetIDsOfNames(IID_NULL, &lpName, 1, LOCALE_SYSTEM_DEFAULT, &dispid);
	if( FAILED(hr) ) 
	{

		_bstr_t Msg;
		Msg += L"Cannot get dispatch id of ";
		Msg += lpName;

		RaiseError(Msg, hr);

	}

	return dispid;
}

#ifndef CW2A
#define  CW2A(x)	_com_util::ConvertBSTRToString(x)
#endif

AELIST PlaylistSoapProxy::OnSetupSoapCall(char* homeId, char* deviceId, char* playlistId, char* streamId)
{
	HRESULT hr;
	DISPPARAMS params;
	VARIANT varg[4];
	_variant_t result;
	EXCEPINFO excepinfo;
	unsigned int uArgErr;
	IPlaylistModelPtr pList;

	CComBSTR v0(homeId);
	CComBSTR v1(deviceId);
	CComBSTR v2(playlistId);
	CComBSTR v3(streamId);

	//////////////////////////////////////////////////////////////////////////
	// prepare parameters and result
	static DISPID dispid = -1;
	if(dispid == -1) dispid = GetIDOfName(_T(ONSETUPCALLNAME));

	varg[0].vt = VT_BSTR;
	varg[0].bstrVal = v3;
	varg[1].vt = VT_BSTR;
	varg[1].bstrVal = v2;
	varg[2].vt = VT_BSTR;
	varg[2].bstrVal = v1;
	varg[3].vt = VT_BSTR;
	varg[3].bstrVal = v0;

	params.cArgs = 4;
	params.rgvarg = varg;
	params.cNamedArgs = 0;
	params.rgdispidNamedArgs = NULL;

	memset(&excepinfo, 0, sizeof(excepinfo));

	uArgErr = -1;

	//////////////////////////////////////////////////////////////////////////
	// invoke method and check result
	hr = m_pSoapClient->Invoke(
			dispid, 
			IID_NULL, 
			LOCALE_SYSTEM_DEFAULT, 
			DISPATCH_METHOD, 
			&params,
			&result, 
			&excepinfo, 
			&uArgErr);

	if( FAILED(hr) )
	{
		RaiseError(L"Invoke of getPlaylistDetailInformation failed.", hr, excepinfo, params, uArgErr);
	}

	if( result.vt != VT_DISPATCH )
	{
		RaiseError(L"getPlaylistDetailInformation did not return an PlaylistModel object.", E_FAIL);
	}

	hr = result.pdispVal->QueryInterface(&pList);
	if( FAILED(hr) )
	{
		RaiseError(L"getPlaylistDetailInformation did not return an PlaylistModel object.", hr);
	}

	//////////////////////////////////////////////////////////////////////////
	// compose AELIST
	CComPtr<IElementArray> pElementArray;
	long elemNum=0;
	long i;

	hr = pList->get_Elements(&pElementArray);
	if(FAILED(hr)) { RaiseError(L"Cannot get ElementArray from PlaylistModel object.", hr); }
	
	hr = pElementArray->get_Count(&elemNum);
	if(FAILED(hr)) { RaiseError(L"Cannot get count of ProductModleArray object.", hr); }
	
	AELIST outList;
	memset(&outList, 0x00, sizeof(AELIST));

	// if no element, return empty list
	if(elemNum==0) {
		outList.AECount = elemNum;
		return outList;
	}

	// has elements, so allocate new array for it
	AELEMENT* pNewAEs = new AELEMENT[elemNum];
	memset(pNewAEs, 0x00, sizeof(AELEMENT)*elemNum); 

	BSTR ElementID;
	DWORD dwAEUID;
	
	outList.AECount = elemNum;
	
	for(i=0; i<elemNum; i++)
	{
		hr = pElementArray->get_Item(i, &ElementID);
		if(FAILED(hr)) { RaiseError(L"Cannot get ProductModleType item from ProductModleArray object.", hr); }
		
		dwAEUID = 0;
		char* strAEID = _com_util::ConvertBSTRToString(ElementID);
		sscanf(strAEID, "%d", &dwAEUID);
		delete[] strAEID;

		pNewAEs[i].AEUID = dwAEUID;
		pNewAEs[i].PlayoutEnabled = TRUE;
		pNewAEs[i].EncryptionVendor = ITV_ENCRYPTION_VENDOR_None;
		pNewAEs[i].EncryptionDevice = ITV_ENCRYPTION_DEVICE_None;
	}

	outList.AELlist = (PAEARRAY)pNewAEs;
	return outList;
}

long PlaylistSoapProxy::OnPlaySoapCall(char* homeId, char* deviceId, char* playlistId, char* streamId)
{
	HRESULT hr;
	DISPPARAMS params;
	VARIANT varg[5];
	_variant_t result;
	EXCEPINFO excepinfo;
	unsigned int uArgErr;
	IPlaylistModelPtr pList;

	CComBSTR v0(homeId);
	CComBSTR v1(deviceId);
	CComBSTR v2(playlistId);
	CComBSTR v3(streamId);
	
	//////////////////////////////////////////////////////////////////////////
	// prepare parameters and result
	static DISPID dispid = -1;
	if(dispid == -1) dispid = GetIDOfName(_T(ONPLAYCALLNAME));
	
	varg[0].vt = VT_BSTR;
	varg[0].bstrVal = v3;
	varg[1].vt = VT_BSTR;
	varg[1].bstrVal = v2;
	varg[2].vt = VT_BSTR;
	varg[2].bstrVal = v1;
	varg[3].vt = VT_BSTR;
	varg[3].bstrVal = v0;
	
	params.cArgs = 4;
	params.rgvarg = varg;
	params.cNamedArgs = 0;
	params.rgdispidNamedArgs = NULL;

	memset(&excepinfo, 0, sizeof(excepinfo));

	uArgErr = -1;

	hr = m_pSoapClient->Invoke(
			dispid, 
			IID_NULL, 
			LOCALE_SYSTEM_DEFAULT, 
			DISPATCH_METHOD, 
			&params,
			&result, 
			&excepinfo, 
			&uArgErr);

	if( FAILED(hr) )
	{
		RaiseError(L"Invoke of playPlaylist failed.", hr, excepinfo, params, uArgErr);
	}

	if( result.vt != VT_I4 )
	{
		RaiseError(L"playPlaylist did not return an int.", E_FAIL);
	}

	return result.Detach().lVal;
}

long PlaylistSoapProxy::OnTeardownSoapCall(char* homeId, char* deviceId, char* playlistId, char* streamId, char* errorCode)
{
	HRESULT hr;
	DISPPARAMS params;
	VARIANT varg[5];
	_variant_t result;
	EXCEPINFO excepinfo;
	unsigned int uArgErr;
	IPlaylistModelPtr pList;

	CComBSTR v0(homeId);
	CComBSTR v1(deviceId);
	CComBSTR v2(playlistId);
	CComBSTR v3(streamId);
	CComBSTR v4(errorCode);

	//////////////////////////////////////////////////////////////////////////
	// prepare parameters and result
	static DISPID dispid = -1;
	if(dispid == -1) dispid = GetIDOfName(_T(ONTEARDOWNCALLNAME));
	
	varg[0].vt = VT_BSTR;
	varg[0].bstrVal = v4;
	varg[1].vt = VT_BSTR;
	varg[1].bstrVal = v3;
	varg[2].vt = VT_BSTR;
	varg[2].bstrVal = v2;
	varg[3].vt = VT_BSTR;
	varg[3].bstrVal = v1;
	varg[4].vt = VT_BSTR;
	varg[4].bstrVal = v0;

	params.cArgs = 5;
	params.rgvarg = varg;
	params.cNamedArgs = 0;
	params.rgdispidNamedArgs = NULL;

	memset(&excepinfo, 0, sizeof(excepinfo));

	uArgErr = -1;

	hr = m_pSoapClient->Invoke(
			dispid, 
			IID_NULL, 
			LOCALE_SYSTEM_DEFAULT, 
			DISPATCH_METHOD, 
			&params,
			&result, 
			&excepinfo, 
			&uArgErr);

	if( FAILED(hr) )
	{
		RaiseError(L"Invoke of teardownPlaylist failed.", hr, excepinfo, params, uArgErr);
	}

	if( result.vt != VT_I4 )
	{
		RaiseError(L"teardownPlaylist did not return an int.", E_FAIL);
	}

	return result.Detach().lVal;
}

void PlaylistSoapProxy::releaseAEList(AELIST exAElist)
{
	if(exAElist.AELlist)
		delete[] (PAELEMENT)exAElist.AELlist;
	
	exAElist.AELlist = NULL;
}