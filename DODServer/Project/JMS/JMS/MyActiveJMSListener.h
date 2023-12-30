#ifndef __MYACTIVEJMSLISTENER_H__
#define __MYACTIVEJMSLISTENER_H__

#include "ActiveJMSListener.h"

// forward declaration
class CActiveJMS;

class CMyActiveJMSListener : public CActiveJMSListener  
{
public:
	CMyActiveJMSListener(CActiveJMS* pActiveJMS);

	virtual ~CMyActiveJMSListener();

	virtual void onMessage(COnMessageEvent *onMessageEvent);

protected:
	CActiveJMS * m_pActiveJMS;
   
};

#endif //__MYACTIVEJMSLISTENER_H__