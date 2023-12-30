// ProvisionProgressImpl.cpp: implementation of the ProvisionProgressImpl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ProvisionProgressImpl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ProvisionProgressImpl::ProvisionProgressImpl()
{
	m_pFun = NULL;
}

ProvisionProgressImpl::~ProvisionProgressImpl()
{
	if ( m_pFun)
	{
		m_pFun = NULL;
	}
}

ProvisionProgressImpl::ProvisionProgressImpl(RegEvent_Proc pFun)
{
	m_pFun = pFun;
}

void ProvisionProgressImpl::SetFunction(RegEvent_Proc pFun)
{
	m_pFun = pFun;
}

/// event will be fired when a content provision is processing with progress
///@param[in] netId the net Id of the specified contentstore
///@param[in] contentname content name that can be used to access Contnet thru ContentStore::openContent()
///@param[in] processed the processed provision units in the current step
///@param[in] total the total provision units in the current step. The unit can percentage, or file size in KB, etc
///@param[in] stepNo the sequence number of the current step
///@param[in] totalSteps the total step number must be performed for this provision procedure
void ProvisionProgressImpl::OnProgress(const ::std::string& netId, const ::std::string& contentname, const ::std::string& triggerTimeUTC, ::Ice::Long processed, ::Ice::Long total, ::Ice::Int stepNo, ::Ice::Int totalSteps, const ::Ice::Current& ic)
{
	string strNewCategory;
	string strNewCurTime;
	string strNewMessage;
	int i = 2;
	strNewCategory = "OnProgress";
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

	memset(s2,0,sizeof(s2));
	sprintf(s2,"%s%d%s%d%s%d%s%d"," ,processed=",processed," ,total=",total," ,stepNo=",stepNo," ,totalSteps=",totalSteps);
	strNewMessage += s2;
	if ( m_pFun)
	{
		(*m_pFun)(strNewCategory,i,strNewCurTime,strNewMessage);
	}
}

void ProvisionProgressImpl::ping(::Ice::Long timestamp, const ::Ice::Current& ic)
{
}
