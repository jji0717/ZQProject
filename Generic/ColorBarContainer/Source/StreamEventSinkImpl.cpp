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

// �ҵ���⣺�������Ϣ���������У��ȸ���Stream.uid���ҵ�chanel.uid,�����Զ�λ���ĸ������Ȼ�������ɫ�ۼӵ�ԭ��������ɫ������Ȼ���������������������Ľӿ�CreateCurSorID�������û���ID���û���ID���ۼӵģ�ֱ��150ʱ���ٴ����û�����OnProgress�Ĺ�����comment�����к���Item����ֵ��������ֵ�����û���ID��ֵ��
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

// �����Ҷ�LOG�ļ�����ϸ���֣���Prevstate==stProvisioned ����curState ==stInServiceʱ�������µ�Stream��ʼ��ʱ����ʱ���Ե��������CursorId�������û���ID��ֵ��
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
		if ( Prevstate == stProvisioned && ( curState == stInService )) // ��ʶһ����Stream�Ŀ�ʼ
		{
			// for the test
//			StreamUid ="TestStream1";
			// ��ϸ���룬ֻҪǰ״̬ΪstProvisioned����ǰ״̬״̬ΪstInService��Ҫ�������û��������ܸ��û���ԭ�����û����ͬһStream,�������ﲻ��Ҫ���д����жϡ�
		//	bool bExist;
		//	bExist = m_pCBContainControl->IsExistByStreamId(StreamUid);
        //  if ( !bExist )
			{
				m_pCBContainControl->CreateContainMap(StreamUid); 
				//m_pCBContainControl->CreateUserCursorMap(StreamUid); //��CreateCursorAndDrawLine()��ȡ��
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
