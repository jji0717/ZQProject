// SessionSinkI.h: interface for the SessionSinkI class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SESSIONSINKI_H__58AE8F33_0DBF_470D_9A29_F459E82B67AE__INCLUDED_)
#define AFX_SESSIONSINKI_H__58AE8F33_0DBF_470D_9A29_F459E82B67AE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <tssrm.h>

using namespace TianShanIce::SRM;

class WeiwooAdmin;

class SessionSinkI:public SessionEventSink
{
public:
	SessionSinkI();
	~SessionSinkI();

    void OnNewSession(const ::std::string& id, const ::std::string& proxy, const ::Ice::Current& = ::Ice::Current());
	
    void OnDestroySession(const ::std::string& id, const ::Ice::Current& = ::Ice::Current());
	
    void OnStateChanged(const ::std::string& id, const ::std::string& proxy , ::TianShanIce::State st, ::TianShanIce::State, const ::Ice::Current& = ::Ice::Current());

	void ping(::Ice::Long , const ::Ice::Current& /* = ::Ice::Current */);


	void	SetAdmin(WeiwooAdmin* pAdmin)
	{
		m_pWeiwooAdmin=pAdmin;
	}

public:
	WeiwooAdmin* m_pWeiwooAdmin;
};

#endif // !defined(AFX_SESSIONSINKI_H__58AE8F33_0DBF_470D_9A29_F459E82B67AE__INCLUDED_)
