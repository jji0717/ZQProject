#ifndef __ACTIVEJMSLISTENER_H__
#define __ACTIVEJMSLISTENER_H__

// the event object that is passed with onMessage() is of type IDispatch.
// therefore we must obtain the messageId via IDispatch::Invoke (like VB does)
// this wrapper class provides this functionality.
class COnMessageEvent
{
public:
	COnMessageEvent(IDispatchPtr messageEvent);

	long getMessageId() const;

	//added by whp

	long getComsumerId() const;

	IDispatchPtr getMessageEvent() const;

protected:
	mutable long _messageId;

    mutable long _consumerId;

	IDispatchPtr _messageEvent;
};

/////////////////////////////////////////////////////////////////////////////
// CActiveJMSListener
class CActiveJMSListener : public CCmdTarget
{
public:
	CActiveJMSListener();

	virtual ~CActiveJMSListener();

	// override these in your specialized listener
	virtual void onMessage(COnMessageEvent *onMessageEvent);

	virtual void onException(COnMessageEvent *onMessageEvent);

	// gets pointer to listener interface
	IUnknownPtr getSource();

	// gets pointer to where store cookie (see AfxConnectionAdvise)
	DWORD *getCookieRef();

	DWORD getCookie();

protected:

	// the cookie obtained from AfxConnectionAdvise
	DWORD cookie;

	DECLARE_INTERFACE_MAP()

	BEGIN_INTERFACE_PART(ActiveJMSSource, ActiveJMS::ActiveJMSSource)	
	
		STDMETHOD(onMessage)(IDispatch * OnMessageEvent1);
		STDMETHOD(onException)(IDispatch * OnExceptionEvent1);

		// IDispatch
		STDMETHOD(GetTypeInfoCount)(unsigned int *);

		STDMETHOD(GetTypeInfo)(unsigned int, LCID, ITypeInfo **);

		STDMETHOD(GetIDsOfNames)(REFIID, LPOLESTR*, unsigned int, LCID, DISPID*);

		STDMETHOD(Invoke)(DISPID dispid, REFIID, LCID, unsigned short flags, DISPPARAMS *params, VARIANT *result, EXCEPINFO *exceptionInfo, unsigned int *argError);
	END_INTERFACE_PART(ActiveJMSSource)
};

#endif //__ACTIVEJMSLISTENER_H__