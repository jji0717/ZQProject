// StreamEventSinkImpl.cpp: implementation of the StreamEventSinkImpl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StreamEventSinkImpl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
string   EnmuToString(TianShanIce::State &stateValue);

string   EnmuToString(TianShanIce::State & stateValue)
{
	string strReturn;
	switch(stateValue)
	{
	case TianShanIce::stNotProvisioned:
		strReturn ="NotProvisioned";
		break;
	case TianShanIce::stProvisioned:
		strReturn ="Provisioned";
		break;
	case TianShanIce::stInService:
		strReturn ="InService";
		break;
	case TianShanIce::stOutOfService:
		strReturn ="OutOfService";
		break;
	default:
		strReturn ="NotProvisioned";
		break;
	}
	return strReturn;
}

StreamEventSinkImpl::StreamEventSinkImpl(RegEvent_Proc pFun)
{	
	m_pFun = pFun;
}
StreamEventSinkImpl::StreamEventSinkImpl()
{
//	m_pFun = pFun;
	m_pFun = NULL;
}

StreamEventSinkImpl::~StreamEventSinkImpl()
{
	if ( m_pFun)
	{
		m_pFun = NULL;
	}
}

void StreamEventSinkImpl::SetFunction(RegEvent_Proc pFun)
{
	m_pFun = pFun;
}

/// event fired when the end of the stream session has been reached, the resource bound on this
/// stream session will be released after this event.
///@param[in] proxy  the proxy string to access the stream object
///@param[in] id	 the value of ident.name to identify the stream instance in a string format
///@note for a playlist with multiple items, this event means the end of playlist has been reached
///      if you wish to sink the end of an item in a playlist, please use PlaylistEventSink
///@sa   PlaylistEventBind::OnEndOfItem()
void StreamEventSinkImpl::OnEndOfStream(const ::std::string& proxy, const ::std::string& uid, const ::Ice::Current& ic) const 
{

	string strNewCategory;
	string strNewCurTime;
	string strNewMessage;
	int i = 0;
	strNewCategory = "OnEndOfStream";
	SYSTEMTIME sysTime;
	memset(&sysTime,0,sizeof(sysTime));
	GetLocalTime(&sysTime);
	char s2[100]={0};
	sprintf(s2,"%04d%s%02d%s%02d%s%02d%s%02d%s%02d",sysTime.wYear,"-",sysTime.wMonth,"-",sysTime.wDay," ",sysTime.wHour,":",sysTime.wMinute,":",sysTime.wSecond);
	strNewCurTime = s2;
	
	strNewMessage ="";
	strNewMessage +="uid=";
	strNewMessage += uid.c_str();
	strNewMessage +=", is EndOfStream";;

/*
	ZQ::common::MutexGuard gd(m_Mutex);
	m_EventData.iIndex = i;
	m_EventData.strCateGory = strNewCategory;
	m_EventData.strDataTime = strNewCurTime;
	m_EventData.strEventData = strNewMessage;
	if ( m_pFun)
	{
		(*m_pFun)(m_EventData.strCateGory,m_EventData.iIndex,m_EventData.strDataTime,m_EventData.strEventData);
	}
*/
	if ( m_pFun)
	{
		(*m_pFun)(strNewCategory,i,strNewCurTime,strNewMessage);
	}
}

/// event fired when the beginning of the stream session has been reached, especially for rewind operations
/// the resource bound on this stream session will be released after this event.
///@param[in] proxy  the proxy string to access the stream object
///@param[in] id	 the value of ident.name to identify the stream instance in a string format
///@note for a playlist with multiple items, this event means the end of playlist has been reached
///      if you wish to sink the end of an item in a playlist, please use PlaylistEventSink
///@note this event doesn't equal to the stream start event, please use StreamEventSink::OnStateChanged()
///      and StreamProgressSink::OnProgress() instead if you are insterested at stream start
///@sa   PlaylistEventBind::OnEndOfItem()
void StreamEventSinkImpl::OnBeginningOfStream(const ::std::string& proxy, const ::std::string& uid, const ::Ice::Current& ic )const 
{
	string strNewCategory;
	string strNewCurTime;
	string strNewMessage;
	int i = 1;
	strNewCategory = "OnBeginningOfStream";
	SYSTEMTIME sysTime;
	memset(&sysTime,0,sizeof(sysTime));
	GetLocalTime(&sysTime);
	char s2[100]={0};
	sprintf(s2,"%04d%s%02d%s%02d%s%02d%s%02d%s%02d",sysTime.wYear,"-",sysTime.wMonth,"-",sysTime.wDay," ",sysTime.wHour,":",sysTime.wMinute,":",sysTime.wSecond);
	strNewCurTime = s2;
	

	strNewMessage ="";
	strNewMessage +="uid=";
	strNewMessage += uid.c_str();
	strNewMessage +=", is BeginningOfStream";
	
/*
	ZQ::common::MutexGuard gd(m_Mutex);
	m_EventData.iIndex = i;
	m_EventData.strCateGory = strNewCategory;
	m_EventData.strDataTime = strNewCurTime;
	m_EventData.strEventData = strNewMessage;
	if ( m_pFun)
	{
		(*m_pFun)(m_EventData.strCateGory,m_EventData.iIndex,m_EventData.strDataTime,m_EventData.strEventData);
	}
*/
	if ( m_pFun)
	{
		(*m_pFun)(strNewCategory,i,strNewCurTime,strNewMessage);
	}

	// add by me  for  callback proc
	if ( m_StateProc)
	{
		(*m_StateProc)(uid,"BeginOfStream","",-1);
	}
}

/// event fired when a stream's state has been changed
///@param[in] proxy the proxy string to access the stream object
///@param[in] id	the value of ident.name to identify the stream instance in a string format
///@param[in] prevSpeed the previous play speed of the stream
///@param[in] currentSpeed the current play speed of the stream
void StreamEventSinkImpl::OnSpeedChanged(const ::std::string& proxy, const ::std::string& uid, ::Ice::Float prevSpeed, ::Ice::Float currentSpeed, const ::Ice::Current& ic ) const 
{

	string strNewCategory;
	string strNewCurTime;
	string strNewMessage;
	int i = 2;
	strNewCategory = "OnSpeedChanged";
	SYSTEMTIME sysTime;
	memset(&sysTime,0,sizeof(sysTime));
	GetLocalTime(&sysTime);
	char s2[100]={0};
	sprintf(s2,"%04d%s%02d%s%02d%s%02d%s%02d%s%02d",sysTime.wYear,"-",sysTime.wMonth,"-",sysTime.wDay," ",sysTime.wHour,":",sysTime.wMinute,":",sysTime.wSecond);
	strNewCurTime = s2;
	
	strNewMessage ="";
	strNewMessage +="uid=";
	strNewMessage += uid.c_str();
	
	memset(s2,0,sizeof(s2));
	sprintf("%s%f%s%f"," ,prevSpeed=",prevSpeed," ,currentSpeed=",currentSpeed);
	strNewMessage +=s2;
	
//	strNewMessage +=ic.facet;
//	strNewMessage +=ic.operation;
/*
	ZQ::common::MutexGuard gd(m_Mutex);
	m_EventData.iIndex = i;
	m_EventData.strCateGory = strNewCategory;
	m_EventData.strDataTime = strNewCurTime;
	m_EventData.strEventData = strNewMessage;
	if ( m_pFun)
	{
		(*m_pFun)(m_EventData.strCateGory,m_EventData.iIndex,m_EventData.strDataTime,m_EventData.strEventData);
	}
*/
	if ( m_pFun)
	{
		(*m_pFun)(strNewCategory,i,strNewCurTime,strNewMessage);
	}
}

/// event fired when a stream's state has been changed
///@param[in] proxy the proxy string to access the stream object
///@param[in] id	the value of ident.name to identify the stream instance in a string format
///@param[in] prevState the previous state of the stream
///@param[in] currentState the current state of the stream
void StreamEventSinkImpl::OnStateChanged(const ::std::string& proxy, const ::std::string& uid, ::TianShanIce::State Prevstate, ::TianShanIce::State curState, const ::Ice::Current&  ic) const
{
	string strNewCategory;
	string strNewCurTime;
	string strNewMessage;
	string stateMsg;
	string strCurState;
	int iCurCtrlNum = -1;

	int i = 0;
	strNewCategory = "OnStateChanged";
	SYSTEMTIME sysTime;
	memset(&sysTime,0,sizeof(sysTime));
	GetLocalTime(&sysTime);
	char s2[100]={0};
	sprintf(s2,"%04d%s%02d%s%02d%s%02d%s%02d%s%02d",sysTime.wYear,"-",sysTime.wMonth,"-",sysTime.wDay," ",sysTime.wHour,":",sysTime.wMinute,":",sysTime.wSecond);
	strNewCurTime = s2;
	
	strNewMessage="";
	strNewMessage +="uid=";
	strNewMessage += uid.c_str();
	
	string strTemp;
	switch(Prevstate)
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
		
	
	strNewMessage +=" ,PrevState=";
//	strTemp = EnmuToString(Prevstate);
	strNewMessage+=strTemp.c_str();
	
	strNewMessage +=" ,curState=";
	switch(curState)
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
	strCurState = strTemp;

	
	
//	strTemp = EnmuToString(curState);
	strNewMessage +=strTemp.c_str();
	
/*
	ZQ::common::MutexGuard gd(m_Mutex);
	m_EventData.iIndex = i;
	m_EventData.strCateGory = strNewCategory;
	m_EventData.strDataTime = strNewCurTime;
	m_EventData.strEventData = strNewMessage;
	if ( m_pFun)
	{
		(*m_pFun)(m_EventData.strCateGory,m_EventData.iIndex,m_EventData.strDataTime,m_EventData.strEventData);
	}
*/
	if ( m_pFun)
	{
		(*m_pFun)(strNewCategory,i,strNewCurTime,strNewMessage);
	}

	// add by me  for  callback proc
	if ( m_StateProc)
	{
		(*m_StateProc)(uid,"OnStateChanged",strCurState,-1);
	}
}

/// event fired when a stream exit
///@param[in] proxy	   the proxy string to access the stream object
///@param[in] id	   the value of ident.name to identify the stream instance in a string format
///@param[in] exitCode the exist code of the stream
///@param[in] reason   a description of the exist reason
void StreamEventSinkImpl::OnExit(const ::std::string& proxy, const ::std::string& uid, ::Ice::Int exitCode, const ::std::string& reason, const ::Ice::Current& ic ) const 
{
	string strNewCategory;
	string strNewCurTime;
	string strNewMessage;
	int i = 1;
	strNewCategory = "OnExit";
	SYSTEMTIME sysTime;
	memset(&sysTime,0,sizeof(sysTime));
	GetLocalTime(&sysTime);
	char s2[100]={0};
	sprintf(s2,"%04d%s%02d%s%02d%s%02d%s%02d%s%02d",sysTime.wYear,"-",sysTime.wMonth,"-",sysTime.wDay," ",sysTime.wHour,":",sysTime.wMinute,":",sysTime.wSecond);
	strNewCurTime = s2;
	
	strNewMessage="";
	strNewMessage="";
	strNewMessage +="uid=";
	strNewMessage += uid.c_str();
	strNewMessage +=" is Exit,";


	memset(s2,0,sizeof(s2));
	sprintf(s2,"%s%d","exitcode=",exitCode);
	strNewMessage +=s2;

	/*
	ZQ::common::MutexGuard gd(m_Mutex);
	m_EventData.iIndex = i;
	m_EventData.strCateGory = strNewCategory;
	m_EventData.strDataTime = strNewCurTime;
	m_EventData.strEventData = strNewMessage;
	if ( m_pFun)
	{
		(*m_pFun)(m_EventData.strCateGory,m_EventData.iIndex,m_EventData.strDataTime,m_EventData.strEventData);
	}
	*/
	if ( m_pFun)
	{
		(*m_pFun)(strNewCategory,i,strNewCurTime,strNewMessage);
	}

	// add by me  for  callback proc
	if ( m_StateProc)
	{
		(*m_StateProc)(uid,"OnExit","",-1);
	}
}

void StreamEventSinkImpl::ping(::Ice::Long timestamp, const ::Ice::Current& ic )
{

}