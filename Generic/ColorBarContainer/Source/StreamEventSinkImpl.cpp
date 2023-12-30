// StreamEventSinkImpl.cpp: implementation of the StreamEventSinkImpl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StreamEventSinkImpl.h"
#include "ColorBarContainerControl.h"
// for the log 
#include <string>
#include <fstream>
// for the log 

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

StreamEventSinkImpl::StreamEventSinkImpl()
{
	m_pCBContainControl = NULL;
}

StreamEventSinkImpl::StreamEventSinkImpl(CColorBarContainerControl  *pCBContainControl)
{
	m_pCBContainControl = pCBContainControl; 
}

StreamEventSinkImpl::~StreamEventSinkImpl()
{
	if ( m_pCBContainControl )
	{
		m_pCBContainControl = NULL;
	}
}

void StreamEventSinkImpl::OnEndOfStream(const ::std::string& proxy, const ::std::string& uid, const ::Ice::Current& ic) const
{
	std::string StreamUid;
	StreamUid = uid;
//	m_pCBContainControl->CreateContainMap(StreamUid);
	
#ifdef _LOG
	// for the log 
	std::ofstream osfileEnd("EndOfStream.log",std::ios_base::app);
	std::string strTemp;
	strTemp  ="StreamEndof Stream:";
	strTemp +="proxy =";
	strTemp +=proxy.c_str();
	strTemp +=", uid=";
	strTemp += uid.c_str();
	
	strTemp +=", Ice::Current: ice Identity=";
	strTemp +=ic.id.name.c_str();
	strTemp +=", ice category=";
	strTemp +=ic.id.category.c_str();

	strTemp +=", ice'facet =";
	strTemp +=ic.facet;

	strTemp +=", ice'operation=";
	strTemp +=ic.operation;

	if ( ic.mode == Normal )
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


	osfileEnd << strTemp.c_str();
	osfileEnd << std::endl;
#endif
}

// 我的理解：在这个消息触发过程中，先根据Stream.uid来找到chanel.uid,即可以定位是哪个组件，然后根据颜色累加的原理生成颜色参数，然后利用这个参数调用组件的接口CreateCurSorID来创建用户的ID，用户的ID是累加的，直到150时则不再创建用户，在OnProgress的过程中comment参数中含有Item索引值，该索引值就是用户的ID号值。
void StreamEventSinkImpl::OnBeginningOfStream(const ::std::string& proxy, const ::std::string& uid, const ::Ice::Current& ic) const
{
	std::string StreamUid;
	StreamUid = uid;
//	m_pCBContainControl->CreateContainMap(StreamUid);

#ifdef _LOG
	// for the log 
	std::ofstream osfileBegin("BeginOfStream.log",std::ios_base::app);
	std::string strTemp;
	strTemp  ="StreamBeginof Stream:";
	strTemp +="proxy =";
	strTemp += proxy.c_str();
	strTemp +=", uid=";
	strTemp += uid.c_str();

	strTemp +=", Ice::Current: ice Identity=";
	strTemp +=ic.id.name.c_str();
	strTemp +=", ice category=";
	strTemp +=ic.id.category.c_str();

	strTemp +=", ice'facet =";
	strTemp +=ic.facet;

	strTemp +=", ice'operation=";
	strTemp +=ic.operation;

	if ( ic.mode == Normal )
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

	osfileBegin << strTemp.c_str();
	osfileBegin << std::endl;
	//
#endif
}

void StreamEventSinkImpl::OnSpeedChanged(const ::std::string& proxy, const ::std::string& uid,   ::Ice::Float prevSpeed,   ::Ice::Float currentSpeed, const ::Ice::Current& ic) const
{
#ifdef _LOG
	// for the log 
	std::string strTemp;
	strTemp  ="SpeedChanged Stream:";
	strTemp +="proxy =";
	strTemp += proxy.c_str();
	strTemp +=", uid=";
	strTemp += uid.c_str();

	long lSpeed;
	lSpeed = (long)prevSpeed;

	char s2[20];
	ltoa(lSpeed,s2,10);

	strTemp +=", PrevSpeed=";
	strTemp += s2;

	memset((void*)s2,0,sizeof(s2));
	lSpeed  = currentSpeed;
	ltoa(lSpeed,s2,10);
	strTemp +=s2;

	strTemp +=", CurrentSpeed=";
	strTemp += s2;
	
	
	strTemp +=", Ice::Current: ice Identity=";
	strTemp +=ic.id.name.c_str();
	strTemp +=", ice category=";
	strTemp +=ic.id.category.c_str();

	strTemp +=", ice'facet =";
	strTemp +=ic.facet;

	strTemp +=", ice'operation=";
	strTemp +=ic.operation;

	if ( ic.mode == Normal )
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

	std::ofstream osfile("SpeedChanged.log",std::ios_base::app);
	osfile << strTemp.c_str();
#endif
}

// 根据我对LOG文件的仔细发现，当Prevstate==stProvisioned 并且curState ==stInService时，正是新的Stream开始的时候，这时可以调用组件的CursorId来创建用户的ID号值。
void StreamEventSinkImpl::OnStateChanged(const ::std::string& proxy, const ::std::string& uid, ::TianShanIce::State Prevstate, ::TianShanIce::State curState, const ::Ice::Current& ic) const
{
	std::string StreamUid;
	StreamUid = uid;

	if ( m_pCBContainControl )
	{
		/*
		if (curState == stInService)
		{
			///TODO: create a new cursor if uid is not in the list
		}
		else if (curState <> stInService)
		{
			// find pCusor by uid
			// delete pCursor
		}
		*/
		if ( Prevstate == stProvisioned && ( curState == stInService )) // 标识一个新Stream的开始
		{
			// for the test
//			StreamUid ="TestStream1";
			// 仔细想想，只要前状态为stProvisioned，当前状态状态为stInService就要创建新用户，而不管该用户和原来的用户点击同一Stream,所以这里不需要进行存在判断。
		//	bool bExist;
		//	bExist = m_pCBContainControl->IsExistByStreamId(StreamUid);
        //  if ( !bExist )
			{
				m_pCBContainControl->CreateContainMap(StreamUid); 
				//m_pCBContainControl->CreateUserCursorMap(StreamUid); //被CreateCursorAndDrawLine()函取代
			}
		}
	}

#ifdef _LOG
	// for the log 
	std::ofstream osfileState("StateChanged.log",std::ios_base::app);
	std::string strTemp;
	strTemp  ="Stated Stream:";
	strTemp +="proxy =";
	strTemp += proxy.c_str();
	strTemp +=", uid=";
	strTemp += uid.c_str();

	if ( Prevstate == stNotProvisioned )
	{
		strTemp +=", PrevState =stNotProvisioned";
	}
	else if ( Prevstate == stProvisioned )
	{
		strTemp +=", PrevState =stProvisioned";
	}
	else if ( Prevstate == stInService )
	{
		strTemp +=", PrevState =stInService";
	}
	else
	{
		strTemp +=", PrevState =stOutOfService";

	}

	if ( curState == stNotProvisioned )
	{
		strTemp +=", curState =stNotProvisioned";
	}
	else if ( curState == stProvisioned )
	{
		strTemp +=", curState =stProvisioned";
	}
	else if ( curState == stInService )
	{
		strTemp +=", curState =stInService";
	}
	else
	{
		strTemp +=", curState =stOutOfService";

	}

	strTemp +=", Ice::Current: ice Identity=";
	strTemp +=ic.id.name.c_str();
	strTemp +=", ice category=";
	strTemp +=ic.id.category.c_str();

	strTemp +=", ice'facet =";
	strTemp +=ic.facet;

	strTemp +=", ice'operation=";
	strTemp +=ic.operation;

	if ( ic.mode == Normal )
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

	osfileState << strTemp.c_str();
	osfileState << std::endl;
#endif

}

void StreamEventSinkImpl::OnExit(const ::std::string& proxy, const ::std::string& uid, ::Ice::Int exitCode, const ::std::string& reason, const ::Ice::Current& ic ) const 
{
	// for the log 
#ifdef _LOG	
	std::ofstream osfile("Exit.log",std::ios_base::app);
	std::string strTemp;
	strTemp  ="Exit Stream:";
	strTemp +="proxy =";
	strTemp += proxy.c_str();
	strTemp +="uid=";
	strTemp += uid.c_str();

	char s2[10];
	itoa(exitCode,s2,10);
	strTemp +=", ExitCode=";
	strTemp +=s2;
	strTemp +=", reason=";
	strTemp +=reason.c_str();
	
	strTemp +=", Ice::Current: ice Identity=";
	strTemp +=ic.id.name.c_str();
	strTemp +=", ice category=";
	strTemp +=ic.id.category.c_str();

	strTemp +=", ice'facet =";
	strTemp +=ic.facet;

	strTemp +=", ice'operation=";
	strTemp +=ic.operation;

	if ( ic.mode == Normal )
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
#endif
}

void StreamEventSinkImpl::ping(::Ice::Long timestamp, const ::Ice::Current& ic )
{
	
}
