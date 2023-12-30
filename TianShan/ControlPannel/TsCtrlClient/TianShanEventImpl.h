// TianShanEventImpl.h: interface for the TianShanEventImpl class.
//
//////////////////////////////////////////////////////////////////////
#ifndef __TIANSHANEVENT_H
#define __TIANSHANEVENT_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class TianShanEventImpl : public TianShanIce::Events::BaseEventSink  
{
public:
	TianShanEventImpl();
	virtual ~TianShanEventImpl();
	virtual void ping(::Ice::Long timestamp, const ::Ice::Current& ic = ::Ice::Current());
};

#endif // !defined(AFX_TIANSHANEVENTIMPL_H__D89D81FE_22A2_4E9E_A8D7_52195CA7C83B__INCLUDED_)
