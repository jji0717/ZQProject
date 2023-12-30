// PlaylistEventSinkI.h: interface for the PlaylistEventSinkI class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLAYLISTEVENTSINKI_H__C1939314_6314_4050_8DFF_A838AA84172D__INCLUDED_)
#define AFX_PLAYLISTEVENTSINKI_H__C1939314_6314_4050_8DFF_A838AA84172D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TsStreamer.h"
#include <ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <IceStorm/IceStorm.h>
#include <EventChannel.h>
#include "ssm_PauseTV_s1.h"
//#include "ScLog.h"
#include "FileLog.h"

class PlaylistEventSinkI : public TianShanIce::Streamer::PlaylistEventSink  
{
protected:
	//static ZQ::common::ScLog *		pLog;
	static ZQ::common::FileLog *		pLog;
public:
	//PlaylistEventSinkI(ZQ::common::ScLog * log);
	PlaylistEventSinkI(ZQ::common::FileLog * log);
	virtual ~PlaylistEventSinkI();
	void ping(::Ice::Long lv, const ::Ice::Current& ic = ::Ice::Current());	
	void OnItemStepped(const ::std::string& proxy, const ::std::string& playlistId, ::Ice::Int curUserCtrlNum,Ice::Int preUserCtrlNum, const ::TianShanIce::Properties& prty, const ::Ice::Current& ic = ::Ice::Current()) const;
};


#endif // !defined(AFX_PLAYLISTEVENTSINKI_H__C1939314_6314_4050_8DFF_A838AA84172D__INCLUDED_)
