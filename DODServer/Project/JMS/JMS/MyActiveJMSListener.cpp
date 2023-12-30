// MyActiveJMSListener.cpp: implementation of the CMyActiveJMSListener class.
//                      Invoke OnMessage event and process
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MyActiveJMSListener.h"
#include "ActiveJMS.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMyActiveJMSListener::CMyActiveJMSListener(CActiveJMS* pActiveJMS)
{
	//get handler of active JMS
    m_pActiveJMS=pActiveJMS;
}

CMyActiveJMSListener::~CMyActiveJMSListener()
{
  
}

//invoke message
void CMyActiveJMSListener::onMessage(COnMessageEvent *onMessageEvent)
{
	try
	{
	ActiveJMS::ActiveJMSDispatchPtr objActiveJMS  = m_pActiveJMS->objActiveJMS;
	long lngConsumerID = onMessageEvent->getComsumerId();

	//construct a received message
    //CJMSMessage::CJMSMessage( CActiveJMS* pActiveJMS,BOOL bSend ,long lMessageID ,BOOL bMode)
	CJMSMessage jmsmessage(m_pActiveJMS,FALSE,onMessageEvent->getMessageId());

	//parse message
    m_pActiveJMS->Dispatch(lngConsumerID,&jmsmessage);

    //TRACE("\n %s",objActiveJMS->getTextOnTextMessage(lngConsumerID));

    //Active JMS hangs onto the message object until you tell it to purge it... if you don't need anymore, get rid of it!
    objActiveJMS->purge(lngConsumerID);
	}
	catch(_com_error &e)
	{
		  CString msg = CString((LPCWSTR)e.Description());
		  msg += L"\r\n\r\n";
		  TRACE(msg);
	}

}