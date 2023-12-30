// BaseSender.h: interface for the BaseSender class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BASESENDER_H__A8D94855_B123_483D_B3DD_F063BD1AD182__INCLUDED_)
#define AFX_BASESENDER_H__A8D94855_B123_483D_B3DD_F063BD1AD182__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "MsgSenderInterface.h"

class BaseSender  
{
public:
	BaseSender();
	virtual ~BaseSender();

	virtual void Close()=0;
	virtual void AddMessage(const MSGSTRUCT& msgStruct)=0;
	virtual bool GetParFromFile(const char* pFileName)=0; 
	virtual bool init(void)=0;

};

#endif // !defined(AFX_BASESENDER_H__A8D94855_B123_483D_B3DD_F063BD1AD182__INCLUDED_)
