// ModSoapModel.cpp : Implementation of CModSoapModel
#include "stdafx.h"
#include "OTEModSoapUDTMapper.h"
#include "ModSoapModel.h"

/////////////////////////////////////////////////////////////////////////////
// CModSoapModel

STDMETHODIMP CModSoapModel::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IModSoapModel
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		//if (InlineIsEqualGUID(*arr[i],riid))
		if (*arr[i] == riid)
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CModSoapModel::get_DeviceID(BSTR *pVal)
{
	*pVal = m_strDeviceID.Copy();
	return S_OK;
}

STDMETHODIMP CModSoapModel::put_DeviceID(BSTR newVal)
{
	m_strDeviceID = newVal;
	return S_OK;
}

STDMETHODIMP CModSoapModel::get_TicketID(BSTR *pVal)
{
	*pVal = m_strTicketID.Copy();
	return S_OK;
}

STDMETHODIMP CModSoapModel::put_TicketID(BSTR newVal)
{
	m_strTicketID = newVal;
	return S_OK;
}

STDMETHODIMP CModSoapModel::get_StreamID(BSTR *pVal)
{
	*pVal = m_strStreamID.Copy();
	return S_OK;
}

STDMETHODIMP CModSoapModel::put_StreamID(BSTR newVal)
{
	m_strStreamID = newVal;
	return S_OK;
}

STDMETHODIMP CModSoapModel::get_PurchaseTime(BSTR *pVal)
{
	*pVal = m_strPurchaseTime.Copy();
	return S_OK;
}

STDMETHODIMP CModSoapModel::put_PurchaseTime(BSTR newVal)
{
	m_strPurchaseTime = newVal;
	return S_OK;
}

STDMETHODIMP CModSoapModel::get_ErrorCode(BSTR *pVal)
{
	*pVal = m_strErrorCode.Copy();
	return S_OK;
}

STDMETHODIMP CModSoapModel::put_ErrorCode(BSTR newVal)
{
	m_strErrorCode = newVal;
	return S_OK;
}


STDMETHODIMP CModSoapModel::get_ProviderID(BSTR *pVal)
{
	*pVal = m_strProviderID.Copy();
	return S_OK;
}

STDMETHODIMP CModSoapModel::put_ProviderID(BSTR newVal)
{
	m_strProviderID = newVal;
	return S_OK;
}

STDMETHODIMP CModSoapModel::get_ProviderAssetID(BSTR *pVal)
{
	*pVal = m_strProviderAssetID.Copy();
	return S_OK;
}

STDMETHODIMP CModSoapModel::put_ProviderAssetID(BSTR newVal)
{
	m_strProviderAssetID = newVal;
	return S_OK;
}
