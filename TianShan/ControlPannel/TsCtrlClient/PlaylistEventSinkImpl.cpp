// PlaylistEventSinkImpl.cpp: implementation of the PlaylistEventSinkImpl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PlaylistEventSinkImpl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PlaylistEventSinkImpl::PlaylistEventSinkImpl()
{
	m_pFun = NULL;
}

PlaylistEventSinkImpl::~PlaylistEventSinkImpl()
{
	if ( m_pFun)
	{
		m_pFun = NULL;
	}

}

PlaylistEventSinkImpl::PlaylistEventSinkImpl(RegEvent_Proc pFun)
{
	m_pFun = pFun;
}

void PlaylistEventSinkImpl::SetFunction(RegEvent_Proc pFun)
{
	m_pFun = pFun;
}

void PlaylistEventSinkImpl::ping(::Ice::Long lv, const ::Ice::Current& ic )
{

}

/// event fired when the end of an item within a playlist has been stepped in execution, the resource bound on this
/// stream session will be held till StreamEventSink::OnEndOfStream().
///@param[in] proxy  the proxy string to access the playlist object
///@param[in] playlistId the playlist id
///@param[in] currentUserCtrlNum the user's control number of the current item in the playlist after stepping
///@param[in] prevUserCtrlNum the user's control number of the previous item in the playlist before stepping
///                           invalid user's control number such as -1, -2 may be given when there is no previous item
///@note if you wish to sink the end of an whole playlist, please use StreamEventSink::OnEndOfStream()
///@sa   StreamEventSink::OnEndOfStream()
void PlaylistEventSinkImpl::OnItemStepped(const ::std::string& proxy, const ::std::string& playlistId, ::Ice::Int currentUserCtrlNum, ::Ice::Int prevUserCtrlNum, const ::TianShanIce::Properties& ItemProps, const ::Ice::Current& ic ) const 
{
	string strNewCategory;
	string strNewCurTime;
	string strNewMessage;
	int i = 0;
	int iCurCtrlNum = (int)currentUserCtrlNum;
	
	strNewCategory = "OnItemStepped";
	SYSTEMTIME sysTime;
	memset(&sysTime,0,sizeof(sysTime));
	GetLocalTime(&sysTime);
	char s2[100]={0};
	sprintf(s2,"%04d%s%02d%s%02d%s%02d%s%02d%s%02d",sysTime.wYear,"-",sysTime.wMonth,"-",sysTime.wDay," ",sysTime.wHour,":",sysTime.wMinute,":",sysTime.wSecond);
	strNewCurTime = s2;
	
	strNewMessage ="";
	strNewMessage +="playlistId=";
	strNewMessage +=playlistId;
	
	memset(s2,0,sizeof(s2));
	sprintf("%s%d%s%d"," ,currentUserCtrlNum=",currentUserCtrlNum," ,prevUserCtrlNum=",prevUserCtrlNum);
	strNewMessage +=s2;
	
	if ( m_pFun)
	{
		(*m_pFun)(strNewCategory,i,strNewCurTime,strNewMessage);
	}

	// add by me  for  callback proc
	if ( m_StateProc)
	{
		(*m_StateProc)(playlistId,"ItemSteppd","",iCurCtrlNum);
	}
}