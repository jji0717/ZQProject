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
#include "TextWriter.h"


class MagSender  
{
public:	
	MagSender();
	virtual ~MagSender();

	bool init(const char* pText=NULL,const char* pType=NULL);
	void uninit();

	void iceMessage(const MSGSTRUCT& msgStruct, const MessageIdentity& mid, void* ctx);
	void textMessage(const MSGSTRUCT& msgStruct, const MessageIdentity& mid, void* ctx);

private:
	char* getType(const char* pText);

private:
	IceSender	_ICESender;
	TextWriter  _TextWriter;

};

#endif // !defined(AFX_MagSender_H__7F2824DC_0A65_4A5D_9F63_635624CA6152__INCLUDED_)
