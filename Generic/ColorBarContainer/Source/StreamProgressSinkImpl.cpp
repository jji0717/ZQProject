// StreamProgressSinkImpl.cpp: implementation of the CStreamProgressSinkImpl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StreamProgressSinkImpl.h"
#include "ColorBarContainerControl.h"
// for the log 
#include <string>
#include <fstream>
// for th log 

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

StreamProgressSinkImpl::StreamProgressSinkImpl()
{
	m_pCBContainControl = NULL;
}

StreamProgressSinkImpl::StreamProgressSinkImpl(CColorBarContainerControl  *pCBContainControl)
{
	this->m_pCBContainControl = pCBContainControl;
}

StreamProgressSinkImpl::~StreamProgressSinkImpl()
{
	if ( this->m_pCBContainControl )
	{
		delete m_pCBContainControl;
	}
}

std::string  StreamProgressSinkImpl::GetItemNum(const std::string &ctrlNum)
{
	int ipos,ipos1;
	ipos  = ctrlNum.find("Num=");
	ipos1 = ctrlNum.find("&");
	std::string strReturn;
	strReturn.empty();
	strReturn ="";
	
	for ( int i = ipos +4; i < ipos1 ; i ++ )
	{
		strReturn +=ctrlNum.at(i);
	}
	return strReturn;
}

//根据用户当前的位置画线
void StreamProgressSinkImpl::OnProgress(const ::std::string& proxy ,const ::std::string& uid, ::Ice::Int done, ::Ice::Int total, ::Ice::Int step, ::Ice::Int totalsteps,  const ::std::string& comment, const ::Ice::Current&  ic) const
{
	//
	std::string StreamUid;
	std::string strCtrlNum;
	double dRatio;
	StreamUid  = uid;
	
	/*
	int iCursorId;
	iCursorId  = -1;
	int ipos,ipos1;
	ipos  = comment.find("Num=");
	ipos1 = comment.find("&");
	std::string strReturn;
	strReturn.empty();
	strReturn ="";
	
	for ( int i = ipos +4; i < ipos1 ; i ++ )
	{
		strReturn +=comment.at(i);
	}
	strCtrlNum = strReturn;
	iCursorId  = atoi(strCtrlNum.c_str()) + 1;
	*/
		
	dRatio = ( ((double)done)/((double)total));
	if ( m_pCBContainControl )
	{
//		StreamUid ="TestStream1"; //// for the test
//		m_pCBContainControl->UserDrawLine(StreamUid,dRatio,comment,1);//draw the line by the params,//被CreateCursorAndDrawLine()函取代
		m_pCBContainControl->CreateCursorAndDrawLine(StreamUid,comment,dRatio);
	}

#ifdef _LOG
	// for the log 
	std::ofstream osfile("OnProgress.log",std::ios_base::app);
	std::string strTemp;
	char s[20];
	strTemp  ="OnProgress Stream:";
	strTemp +="proxy =";
	strTemp += proxy.c_str();
	strTemp +=", uid=";
	strTemp += uid.c_str();

	itoa(done,s,10);
	strTemp +=", Done=";
	strTemp += s;

	memset(s,0,sizeof(s));
	itoa(total,s,10);
	strTemp +=", Total=";
	strTemp +=s;

	memset(s,0,sizeof(s));
	itoa(step,s,10);
	strTemp +=", step=";
	strTemp +=s;


	memset(s,0,sizeof(s));
	itoa(totalsteps,s,10);
	strTemp +=", totalsteps=";
	strTemp +=s;

	strTemp +=", Comment=";
	strTemp +=comment;

	strTemp +=", Ice::Current: ice Identity=";
	strTemp +=ic.id.name.c_str();
	strTemp +=", ice category=";
	strTemp +=ic.id.category.c_str();

	strTemp +=", ice'facet =";
	strTemp +=ic.facet;

	strTemp +=", ice'operation=";
	strTemp +=ic.operation;

	if ( ic.mode == Normal)
	{
		strTemp +=",ic'mode=Normal";
	}
	else if ( ic.mode == Nonmutating )
	{
		strTemp +=",ic'mode=Nonmutating";
	}
	else 
	{
		strTemp +=",ic'mode=Idempotent";
	}
	
	char s1[10];
	itoa(ic.ctx.size(),s1,10);
	strTemp +=", ic'Context size=";
	strTemp +=s1;

	memset((void*)s1,0,sizeof(s1));
	itoa(ic.requestId,s1,10);
	strTemp +=",ic' requestid=";
	strTemp +=s1;


	osfile << strTemp.c_str();
	osfile << std::endl;
#endif
}


void StreamProgressSinkImpl::ping(::Ice::Long timestamp, const ::Ice::Current& ic )
{
	
}
