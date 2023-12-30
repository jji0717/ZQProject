// ActiveJMSListener.cpp : implementation file
//

#include "stdafx.h"
#include "ActiveJMSListener.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

COnMessageEvent::COnMessageEvent(IDispatchPtr onMessageEvent)
	: _messageEvent(onMessageEvent)
	, _messageId(-1)
	,_consumerId(-1)
{
}

IDispatchPtr COnMessageEvent::getMessageEvent() const
{
	return _messageEvent;
}

long COnMessageEvent::getMessageId() const
{
	if (_messageId == -1)
	{
		try
		{
			// obtain messageId via Invoke

			IDispatchPtr msg = getMessageEvent();
			DISPID dispid;
			LPOLESTR name = L"getMessageId";
			const LCID lcid = LOCALE_SYSTEM_DEFAULT;

			// obtain dispid
			_com_util::CheckError(msg->GetIDsOfNames(IID_NULL, (LPOLESTR*)&name, 1, lcid, &dispid));
			
			// invoke getMessageId()
			DISPPARAMS params = { 0, 0, 0, 0 };
			_variant_t result;
			_com_util::CheckError(msg->Invoke(dispid, IID_NULL, lcid, DISPATCH_METHOD, &params, &result, 0, 0));

			_messageId = result;
		}
		catch (_com_error &)
		{
			TRACE0("COnMessageEvent::getMessageId() failed");
		}
	}

	return _messageId;
}
//added by whp
long COnMessageEvent::getComsumerId() const
{
      if (_consumerId == -1)
	{
		try
		{
			// obtain messageId via Invoke
			IDispatchPtr msg = getMessageEvent();
			DISPID dispid;
			LPOLESTR name = L"getMessageConsumerId";
			const LCID lcid = LOCALE_SYSTEM_DEFAULT;

			// obtain dispid
			_com_util::CheckError(msg->GetIDsOfNames(IID_NULL, (LPOLESTR*)&name, 1, lcid, &dispid));
			
			// invoke getMessageConsumerId()
			DISPPARAMS params = { 0, 0, 0, 0 };
			_variant_t result;
			_com_util::CheckError(msg->Invoke(dispid, IID_NULL, lcid, DISPATCH_METHOD, &params, &result, 0, 0));

			_consumerId = result;
		}
		catch (_com_error &)
		{
			TRACE0("COnMessageEvent::getComsumerId() failed");
		}
	}
	return _consumerId; 
}



/////////////////////////////////////////////////////////////////////////////
// CActiveJMSListener

CActiveJMSListener::CActiveJMSListener()
	: cookie(0)
{
}

CActiveJMSListener::~CActiveJMSListener()
{
}

IUnknownPtr CActiveJMSListener::getSource()
{
	return &m_xActiveJMSSource;
}

void CActiveJMSListener::onMessage(COnMessageEvent *onMessageEvent)
{
}

void CActiveJMSListener::onException(COnMessageEvent *onMessageEvent)
{
}

DWORD *CActiveJMSListener::getCookieRef()
{
	return &cookie;
}

DWORD CActiveJMSListener::getCookie()
{
	return *getCookieRef();
}

BEGIN_INTERFACE_MAP(CActiveJMSListener, CCmdTarget)
	INTERFACE_PART(CActiveJMSListener, __uuidof(ActiveJMS::ActiveJMSSource), ActiveJMSSource)
END_INTERFACE_MAP()

STDMETHODIMP_(ULONG) CActiveJMSListener::XActiveJMSSource::AddRef()
{
	METHOD_PROLOGUE_EX_(CActiveJMSListener, ActiveJMSSource)

	return pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) CActiveJMSListener::XActiveJMSSource::Release()
{
	METHOD_PROLOGUE_EX_(CActiveJMSListener, ActiveJMSSource)

	return pThis->ExternalRelease();
}

STDMETHODIMP CActiveJMSListener::XActiveJMSSource::QueryInterface(
	REFIID iid, LPVOID* ppvObj)
{
	METHOD_PROLOGUE_EX_(CActiveJMSListener, ActiveJMSSource)

	return pThis->ExternalQueryInterface(&iid, ppvObj);
}

// because ActiveJMS::ActiveJMSSource is a dispinterface, these are not beeing called.
STDMETHODIMP CActiveJMSListener::XActiveJMSSource::onMessage(IDispatch * onMessageEvent)
{
	return S_OK;
}

STDMETHODIMP CActiveJMSListener::XActiveJMSSource::onException(IDispatch * OnMessageEvent1)
{
	return S_OK;
}

// IDispatch
STDMETHODIMP CActiveJMSListener::XActiveJMSSource::GetTypeInfoCount(unsigned int *)
{
	return E_NOTIMPL;
}

STDMETHODIMP CActiveJMSListener::XActiveJMSSource::GetTypeInfo(unsigned int, LCID, ITypeInfo **)
{
	return E_NOTIMPL;
}

STDMETHODIMP CActiveJMSListener::XActiveJMSSource::GetIDsOfNames(REFIID, LPOLESTR*, unsigned int, LCID, DISPID*)
{
	return E_NOTIMPL;
}

STDMETHODIMP CActiveJMSListener::XActiveJMSSource::Invoke(DISPID dispid, REFIID, LCID, unsigned short flags, DISPPARAMS *params, VARIANT *result, EXCEPINFO *exceptionInfo, unsigned int *argError)
{
	METHOD_PROLOGUE_EX_(CActiveJMSListener, ActiveJMSSource)

	HRESULT hr = S_OK;

	switch (dispid)
	{
		case 0:
			pThis->onException(&COnMessageEvent(_variant_t(params->rgvarg[0])));
			break;
		case 1:
			pThis->onMessage(&COnMessageEvent(_variant_t(params->rgvarg[0])));
			break;
		default:
			hr = DISP_E_MEMBERNOTFOUND;
			break;
	}

	return hr;
}

