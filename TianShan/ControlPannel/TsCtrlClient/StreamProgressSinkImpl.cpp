// StreamProgressSinkImpl.cpp: implementation of the StreamProgressSinkImpl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StreamProgressSinkImpl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

StreamProgressSinkImpl::StreamProgressSinkImpl(RegEvent_Proc pFun)
{	
	m_pFun = pFun;
	m_cUid = (char*)malloc(ITEMLEN);
	memset(m_cUid,0,sizeof(m_cUid));
	m_cComment = (char*)malloc(ITEMLEN);
	memset(m_cComment,0,sizeof(m_cComment));
	m_iCurCtrlNum=-1;
}
StreamProgressSinkImpl::StreamProgressSinkImpl()
{
	m_pFun = NULL;
	m_cUid = (char*)malloc(ITEMLEN);
	memset(m_cUid,0,sizeof(m_cUid));
	m_cComment = (char*)malloc(ITEMLEN);
	memset(m_cComment,0,sizeof(m_cComment));
	m_iCurCtrlNum=-1;
}

StreamProgressSinkImpl::~StreamProgressSinkImpl()
{
	if ( m_cUid)
	{
		free(m_cUid);
		m_cUid = NULL;
	}
	if ( m_cComment)
	{
		free(m_cComment);
		m_cComment = NULL;
	}
	if ( m_pFun)
	{
		m_pFun = NULL;
	}
}

void StreamProgressSinkImpl::SetFunction(RegEvent_Proc pFun)
{
	m_pFun = pFun;
}

/// event fired when the end of the stream has some progress
///@param[in] proxy  the proxy string to access the stream object
///@param[in] id	 the value of ident.name to identify the stream instance in a string format
///@param[in] done	 work with "total" to give a progress description: done/total
///@param[in] total	 work with "done" to give a progress description: done/total
///@param[in] step	 work with "totalsteps" to give a progress description in a multiple steps' stream: "step <step> of <totalsteps> steps"
///@param[in] totalsteps work with "step" to give a progress description in a multiple steps' stream: "step <step> of <totalsteps> steps"
void StreamProgressSinkImpl::OnProgress(const ::std::string& proxy ,const ::std::string& uid, ::Ice::Int done, ::Ice::Int total, ::Ice::Int step, ::Ice::Int totalsteps,  const ::std::string& comment, const ::Ice::Current&  ic ) const
{
	string strNewCategory;
	string strNewCurTime;
	string strNewMessage;
	int iCurCtrlNum;
	
	int i = 2;
	strNewCategory = "OnProgress";
	SYSTEMTIME sysTime;
	memset(&sysTime,0,sizeof(sysTime));
	GetLocalTime(&sysTime);
	char s2[100]={0};
	sprintf(s2,"%04d%s%02d%s%02d%s%02d%s%02d%s%02d",sysTime.wYear,"-",sysTime.wMonth,"-",sysTime.wDay," ",sysTime.wHour,":",sysTime.wMinute,":",sysTime.wSecond);
	strNewCurTime = s2;
	
	strNewMessage ="uid=";
	strNewMessage += uid.c_str();
	memset(s2,0,sizeof(s2));
	sprintf(s2,"%s%d%s%d%s%d%s%d"," ,doned=",done," ,total=",total," ,step=",step," ,totalsteps=",totalsteps);
	strNewMessage += s2;
	strNewMessage +=" ,comment=";
	strNewMessage += comment;
	if ( m_pFun)
	{
		(*m_pFun)(strNewCategory,i,strNewCurTime,strNewMessage);
//		Sleep(PROGRESS_INTERTIME);
	}

	// add by me  for  callback proc

	
	int ipos,ipos1;
	ipos  = comment.find("Num=");
	ipos1 = comment.find("&");
	string strReturn;
	strReturn ="";
	
	for (  i = ipos +4; i < ipos1 ; i ++ )
	{
		strReturn +=comment.at(i);
	}
	if ( strReturn.empty())
	{
		iCurCtrlNum = 0;
	}
	else
	{
		iCurCtrlNum  = atoi(strReturn.c_str());
	}
	if ( ( _stricmp(m_cUid,uid.c_str()) !=0 ) || ( _stricmp(m_cComment,comment.c_str()) != 0 )  )
	{
		sprintf(m_cUid,"%s",uid.c_str());
		sprintf(m_cComment,"%s",comment.c_str());
		if ( m_StateProc)
		{
			(*m_StateProc)(uid,"OnProgress","",iCurCtrlNum);
		}
	}
}


void StreamProgressSinkImpl::ping(::Ice::Long timestamp, const ::Ice::Current& ic )
{
	int ib = 20;
}
