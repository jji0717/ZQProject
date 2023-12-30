// MagSender.h: interface for the MagSender class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MagSender_H__7F2824DC_0A65_4A5D_9F63_635624CA6152__INCLUDED_)
#define AFX_MagSender_H__7F2824DC_0A65_4A5D_9F63_635624CA6152__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "MsgSenderInterface.h"
#include "IceSender.h"
#include "JmsSender.h"
#include "TextWriter.h"


class MagSender  
{
public:	
	MagSender();
	virtual ~MagSender();

	bool init(const char* pText=NULL,const char* pType=NULL);
	void uninit();

	void iceMessage(const MSGSTRUCT& msgStruct);
	void jmsMessage(const MSGSTRUCT& msgStruct);
	void textMessage(const MSGSTRUCT& msgStruct);

private:
	char* getType(const char* pText);

private:
	IceSender	_ICESender;
	JmsSender	_JMSSender;
	TextWriter  _TextWriter;

};

#endif // !defined(AFX_MagSender_H__7F2824DC_0A65_4A5D_9F63_635624CA6152__INCLUDED_)
