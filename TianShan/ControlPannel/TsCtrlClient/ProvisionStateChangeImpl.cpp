// ProvisionStateChangeImpl.cpp: implementation of the ProvisionStateChangeImpl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ProvisionStateChangeImpl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ProvisionStateChangeImpl::ProvisionStateChangeImpl()
{
	m_pFun = NULL;
}

ProvisionStateChangeImpl::~ProvisionStateChangeImpl()
{
	if ( m_pFun)
	{
		m_pFun = NULL;
	}
}

ProvisionStateChangeImpl::ProvisionStateChangeImpl(RegEvent_Proc pFun)
{
	m_pFun = pFun;
}

void ProvisionStateChangeImpl::SetFunction(RegEvent_Proc pFun)
{
	m_pFun = pFun;
}

/// event will be fired when a content's provision status is changed
///@param[in] netId the net Id of the specified contentstore
///@param[in] contentname content name that can be used to access Contnet thru ContentStore::openContent()
void ProvisionStateChangeImpl::OnStateChange(const ::std::string& netId, const ::std::string& contentname, const ::std::string& triggerTimeUTC, ::TianShanIce::Storage::ContentProvisionStatus previousStatus, ::TianShanIce::Storage::ContentProvisionStatus currentStatus, ::Ice::Int errorCode, const ::std::string& errorMsg, const ::Ice::Current& ic)
{
	string strNewCategory;
	string strNewCurTime;
	string strNewMessage;
	int i = 1;
	strNewCategory = "OnStateChanged";
	SYSTEMTIME sysTime;
	memset(&sysTime,0,sizeof(sysTime));
	GetLocalTime(&sysTime);
	char s2[100]={0};
	sprintf(s2,"%04d%s%02d%s%02d%s%02d%s%02d%s%02d",sysTime.wYear,"-",sysTime.wMonth,"-",sysTime.wDay," ",sysTime.wHour,":",sysTime.wMinute,":",sysTime.wSecond);
	strNewCurTime = s2;
	
	strNewMessage ="netId=";
	strNewMessage+=netId.c_str();
	strNewMessage+=" ,contentname=";
	strNewMessage+=contentname.c_str();
	strNewMessage+=" ,triggerTime=";
	strNewMessage+=triggerTimeUTC.c_str();

	string strTemp;
	switch(previousStatus)
	{
	case TianShanIce::Storage::cpsSetup:
		strTemp ="Setup";
		break;
	case TianShanIce::Storage::cpsStart:
		strTemp ="Start";
		break;
	case TianShanIce::Storage::cpsStreamable:
		strTemp ="Streamable";
		break;
	case TianShanIce::Storage::cpsProvisioned:
		strTemp ="Provisioned";
		break;
	case TianShanIce::Storage::cpsFailed:
		strTemp ="Failed";
		break;
	case TianShanIce::Storage::cpsDestroyed:
		strTemp="Destroyed";
		break;
	default:
		strTemp ="Setup";
		break;
	}

	strNewMessage+=" ,previousStatus=";
	strNewMessage+=strTemp;

	switch(currentStatus)
	{
	case TianShanIce::Storage::cpsSetup:
		strTemp ="Setup";
		break;
	case TianShanIce::Storage::cpsStart:
		strTemp ="Start";
		break;
	case TianShanIce::Storage::cpsStreamable:
		strTemp ="Streamable";
		break;
	case TianShanIce::Storage::cpsProvisioned:
		strTemp ="Provisioned";
		break;
	case TianShanIce::Storage::cpsFailed:
		strTemp ="Failed";
		break;
	case TianShanIce::Storage::cpsDestroyed:
		strTemp="Destroyed";
		break;
	default:
		strTemp ="Setup";
		break;
	}

	strNewMessage+=" ,currentStatus=";
	strNewMessage+=strTemp;

	
	memset(s2,0,sizeof(s2));
	sprintf(s2,"%s%d"," ,errorCode=",errorCode);
	strNewMessage += s2;
	strNewMessage +=" ,errorMsg=";
	strNewMessage +=errorMsg;
	
	if ( m_pFun)
	{
		(*m_pFun)(strNewCategory,i,strNewCurTime,strNewMessage);
	}

}

void ProvisionStateChangeImpl::ping(::Ice::Long timestamp, const ::Ice::Current& ic )
{
}


