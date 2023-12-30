// PlaylistEventSinkI.cpp: implementation of the PlaylistEventSinkI class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PlaylistEventSinkImpl.h"
#include "ColorBarContainerControl.h"
// for the log 
#include <string>
#include <fstream>
// for the log 

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


PlaylistEventSinkImpl::PlaylistEventSinkImpl(CColorBarContainerControl *pCBContainControl)
{
	this->m_pCBContainControl = pCBContainControl;
}

PlaylistEventSinkImpl::~PlaylistEventSinkImpl()
{
	if ( m_pCBContainControl )
	{
		delete m_pCBContainControl;
	}

}

void PlaylistEventSinkImpl::ping(::Ice::Long lv, const ::Ice::Current& ic)
{
	/*
	#ifdef _LOG
	// for the log 
	std::ofstream osfileBegin("PlaylistEvent.log",std::ios_base::app);
	std::string strTemp;
	strTemp  ="PlayEvent Stream:";
	strTemp +="proxy =";
	strTemp += proxy.c_str();
	strTemp +=", playlistId=";
	strTemp += playlistId.c_str();

	char s[20];
	memset(s,0,sizeof(s));
	itoa(userCtrlNum,s,10);

	strTemp +=", userCtrlNum=";
	strTemp +=s;

	memset((void*)s,0,sizeof(s));
	strTemp +=", Properties.size";
	itoa(prty.size(),s,10);
	strTemp +=s;

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
#endif
	*/

	
}

void PlaylistEventSinkImpl::OnItemStepped(const ::std::string&, const ::std::string&, ::Ice::Int, ::Ice::Int, const ::TianShanIce::Properties&, const ::Ice::Current& ) const 
{
}
/*
void PlaylistEventSinkImpl::OnEndOfItem(const ::std::string& proxy, const ::std::string& playlistId, ::Ice::Int userCtrlNum, const ::TianShanIce::Properties& prty, const ::Ice::Current& ic) const
{
#ifdef _LOG
	// for the log 
	std::ofstream osfileBegin("PlaylistEvent.log",std::ios_base::app);
	std::string strTemp;
	strTemp  ="PlayEvent Stream:";
	strTemp +="proxy =";
	strTemp += proxy.c_str();
	strTemp +=", playlistId=";
	strTemp += playlistId.c_str();

	char s[20];
	memset(s,0,sizeof(s));
	itoa(userCtrlNum,s,10);

	strTemp +=", userCtrlNum=";
	strTemp +=s;

	memset((void*)s,0,sizeof(s));
	strTemp +=", Properties.size";
	itoa(prty.size(),s,10);
	strTemp +=s;

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
#endif

}
*/
