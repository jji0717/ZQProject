// ModSoapResultModel.h : Declaration of the CModSoapResultModel

#ifndef __MODSOAPRESULTMODEL_H_
#define __MODSOAPRESULTMODEL_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CModSoapResultModel
class ATL_NO_VTABLE CModSoapResultModel : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CModSoapResultModel, &CLSID_ModSoapResultModel>,
	public ISupportErrorInfo,
	public IDispatchImpl<IModSoapResultModel, &IID_IModSoapResultModel, &LIBID_OTEMODSOAPUDTMAPPERLib>
{
public:
	CModSoapResultModel()
		: m_lResult(0), m_fPrice(0.0f), m_lRentalDuration(0)
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_MODSOAPRESULTMODEL)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CModSoapResultModel)
	COM_INTERFACE_ENTRY(IModSoapResultModel)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

// IModSoapResultModel
public:
	STDMETHOD(get_RentalDuration)(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_RentalDuration)(/*[in]*/ long newVal);
	STDMETHOD(get_Price)(/*[out, retval]*/ double *pVal);
	STDMETHOD(put_Price)(/*[in]*/ double newVal);
	STDMETHOD(get_Result)(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_Result)(/*[in]*/ long newVal);

private:
	long    m_lResult;
	double  m_fPrice;
	long    m_lRentalDuration;
};

#endif //__MODSOAPRESULTMODEL_H_
