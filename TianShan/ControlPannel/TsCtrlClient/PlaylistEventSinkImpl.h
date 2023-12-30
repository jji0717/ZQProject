// PlaylistEventSinkImpl.h: interface for the PlaylistEventSinkImpl class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __PLAYLISTEVENTSINK_H
#define __PLAYLISTEVENTSINK_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "ZQEventsCtrl.h"

class PlaylistEventSinkImpl : public TianShanIce::Streamer::PlaylistEventSink    
{
public:
	PlaylistEventSinkImpl();
	PlaylistEventSinkImpl(RegEvent_Proc pFun);
	virtual ~PlaylistEventSinkImpl();
	void SetFunction(RegEvent_Proc pFun);

	virtual void ping(::Ice::Long lv, const ::Ice::Current& ic = ::Ice::Current());
	virtual void OnItemStepped(const ::std::string& proxy, const ::std::string& playlistId, ::Ice::Int currentUserCtrlNum, ::Ice::Int prevUserCtrlNum, const ::TianShanIce::Properties& ItemProps, const ::Ice::Current& ic= ::Ice::Current()) const ;

private:
	RegEvent_Proc m_pFun;
};

extern RegPlayListStateProc m_StateProc; // for the playliststate proc

#endif // !defined(AFX_PLAYLISTEVENTSINKIMPL_H__A28632BA_D7EE_4BDE_A474_D080A2D29D05__INCLUDED_)
