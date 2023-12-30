// SessionSinkI.cpp: implementation of the SessionSinkI class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "SessionSinkI.h"
#include "WeiwooAdmin.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SessionSinkI::SessionSinkI()
{
	m_pWeiwooAdmin=NULL;
}

SessionSinkI::~SessionSinkI()
{

}

void SessionSinkI::ping(::Ice::Long , const ::Ice::Current& )
{
}
void SessionSinkI::OnNewSession(const ::std::string& id, const ::std::string& proxy, const ::Ice::Current& /* = ::Ice::Current( */)
{
	if(m_pWeiwooAdmin)
		m_pWeiwooAdmin->onNewSess(id);
}
void SessionSinkI::OnDestroySession(const ::std::string& id, const ::Ice::Current& /* = ::Ice::Current( */)
{
	if(m_pWeiwooAdmin)
		m_pWeiwooAdmin->onDestroySess(id);
}
void SessionSinkI::OnStateChanged(const ::std::string& id, const ::std::string& proxy ,
								  ::TianShanIce::State st, ::TianShanIce::State, const ::Ice::Current& /* = ::Ice::Current( */)
{
	std::string	strState;
	switch(st)
	{
	case TianShanIce::stNotProvisioned:
		strState="NotProvisioned";
		break;
	case TianShanIce::stInService:
		strState="InService";
		break;
	case TianShanIce::stProvisioned:
		strState="Provisioned";
		break;
	case TianShanIce::stOutOfService:
		strState="OutOfService";
		break;
	default:
		strState="Unknown";
		break;
	}
	if(m_pWeiwooAdmin)
		m_pWeiwooAdmin->onSessStateChanged(id,strState);
}