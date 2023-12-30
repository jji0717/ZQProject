// SessionEventSinkImpl.cpp: implementation of the SessionEventSinkImpl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SessionEventSinkImpl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SessionEventSinkImpl::SessionEventSinkImpl()
{
	m_pFun = NULL;
}

SessionEventSinkImpl::~SessionEventSinkImpl()
{
	if ( m_pFun)
	{
		m_pFun = NULL;
	}
}

SessionEventSinkImpl::SessionEventSinkImpl(RegEvent_Proc pFun)
{
	m_pFun = pFun;
}

void SessionEventSinkImpl::SetFunction(RegEvent_Proc pFun)
{
	m_pFun = pFun;
}

/// event fired when a new session is created
///@param[in] sessId the session id of the newly created session
///@param[in] proxy the proxy string to access this session
void SessionEventSinkImpl::OnNewSession(const ::std::string& sessId, const ::std::string& proxy, const ::Ice::Current&  ic)
{
	string strNewCategory;
	string strNewCurTime;
	string strNewMessage;
	int i = 1;
	strNewCategory = "OnNewSession";
	SYSTEMTIME sysTime;
	memset(&sysTime,0,sizeof(sysTime));
	GetLocalTime(&sysTime);
	char s2[100]={0};
	sprintf(s2,"%04d%s%02d%s%02d%s%02d%s%02d%s%02d",sysTime.wYear,"-",sysTime.wMonth,"-",sysTime.wDay," ",sysTime.wHour,":",sysTime.wMinute,":",sysTime.wSecond);
	strNewCurTime = s2;
	
	strNewMessage ="";
	strNewMessage +="SessionId=";
	strNewMessage += sessId.c_str();
	strNewMessage += " is created";
	
	if ( m_pFun)
	{
		(*m_pFun)(strNewCategory,i,strNewCurTime,strNewMessage);
	}
}


/// event fired when a session is destroyed
///@param[in] sessId the session id of the session
void SessionEventSinkImpl::OnDestroySession(const ::std::string& sessId, const ::Ice::Current& ic )
{
	string strNewCategory;
	string strNewCurTime;
	string strNewMessage;
	int i = 0;
	strNewCategory = "OnDestroySession";
	SYSTEMTIME sysTime;
	memset(&sysTime,0,sizeof(sysTime));
	GetLocalTime(&sysTime);
	char s2[100]={0};
	sprintf(s2,"%04d%s%02d%s%02d%s%02d%s%02d%s%02d",sysTime.wYear,"-",sysTime.wMonth,"-",sysTime.wDay," ",sysTime.wHour,":",sysTime.wMinute,":",sysTime.wSecond);
	strNewCurTime = s2;
	
	strNewMessage ="";
	strNewMessage +="SessionId=";
	strNewMessage += sessId.c_str();
	strNewMessage += " is destroyed";
	
	if ( m_pFun)
	{
		(*m_pFun)(strNewCategory,i,strNewCurTime,strNewMessage);
	}
}

/// event fired when a session's state has been changed
///@param[in] sessId the session id of the newly created session
///@param[in] proxy the proxy string to access this session
///@param[in] prevState the previous state of the session
///@param[in] currentState the current state of the session
void SessionEventSinkImpl::OnStateChanged(const ::std::string& sessId, const ::std::string& proxy, ::TianShanIce::State prevState, ::TianShanIce::State currentState, const ::Ice::Current& ic  )
{
	string strNewCategory;
	string strNewCurTime;
	string strNewMessage;
	int i = 2;
	strNewCategory = "OnStateChanged";
	SYSTEMTIME sysTime;
	memset(&sysTime,0,sizeof(sysTime));
	GetLocalTime(&sysTime);
	char s2[100]={0};
	sprintf(s2,"%04d%s%02d%s%02d%s%02d%s%02d%s%02d",sysTime.wYear,"-",sysTime.wMonth,"-",sysTime.wDay," ",sysTime.wHour,":",sysTime.wMinute,":",sysTime.wSecond);
	strNewCurTime = s2;
	
	strNewMessage="";
	strNewMessage+="SessionId=";
	strNewMessage+=sessId.c_str();
	

	string strTemp;
	switch(prevState)
	{
	case TianShanIce::stNotProvisioned:
		strTemp ="NotProvisioned";
		break;
	case TianShanIce::stProvisioned:
		strTemp ="Provisioned";
		break;
	case TianShanIce::stInService:
		strTemp ="InService";
		break;
	case TianShanIce::stOutOfService:
		strTemp ="OutOfService";
		break;
	default:
		strTemp ="NotProvisioned";
		break;
	}
		
	strNewMessage +=" ,prevState=";
	strNewMessage+=strTemp.c_str();
	
	strNewMessage +=" ,currentState=";
	switch(currentState)
	{
	case TianShanIce::stNotProvisioned:
		strTemp ="NotProvisioned";
		break;
	case TianShanIce::stProvisioned:
		strTemp ="Provisioned";
		break;
	case TianShanIce::stInService:
		strTemp ="InService";
		break;
	case TianShanIce::stOutOfService:
		strTemp ="OutOfService";
		break;
	default:
		strTemp ="NotProvisioned";
		break;
	}
	
	strNewMessage +=strTemp.c_str();
	
	
	if ( m_pFun)
	{
		(*m_pFun)(strNewCategory,i,strNewCurTime,strNewMessage);
	}
}

void SessionEventSinkImpl::ping(::Ice::Long timestamp, const ::Ice::Current& ic )
{
	
}
