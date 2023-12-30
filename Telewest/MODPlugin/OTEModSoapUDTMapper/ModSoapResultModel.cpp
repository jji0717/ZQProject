// ModSoapResultModel.cpp : Implementation of CModSoapResultModel
#include "stdafx.h"
#include "OTEModSoapUDTMapper.h"
#include "ModSoapResultModel.h"

/////////////////////////////////////////////////////////////////////////////
// CModSoapResultModel

STDMETHODIMP CModSoapResultModel::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IModSoapResultModel
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (::ATL::InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CModSoapResultModel::get_Result(long *pVal)
{
	*pVal = m_lResult;
	return S_OK;
}

STDMETHODIMP CModSoapResultModel::put_Result(long newVal)
{
	m_lResult = newVal;
	return S_OK;
}

STDMETHODIMP CModSoapResultModel::get_Price(double *pVal)
{
	*pVal = m_fPrice;
	return S_OK;
}

STDMETHODIMP CModSoapResultModel::put_Price(double newVal)
{
	m_fPrice = newVal;
	return S_OK;
}

STDMETHODIMP CModSoapResultModel::get_RentalDuration(long *pVal)
{
	*pVal = m_lRentalDuration;
	return S_OK;
}

STDMETHODIMP CModSoapResultModel::put_RentalDuration(long newVal)
{
	m_lRentalDuration = newVal;
	return S_OK;
}
