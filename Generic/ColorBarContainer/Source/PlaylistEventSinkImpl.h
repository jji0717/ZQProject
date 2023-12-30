// PlaylistEventSinkI.h: interface for the PlaylistEventSinkI class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLAYLISTEVENTSINKI_H__C1939314_6314_4050_8DFF_A838AA84172D__INCLUDED_)
#define AFX_PLAYLISTEVENTSINKI_H__C1939314_6314_4050_8DFF_A838AA84172D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <IceStorm/IceStorm.h>
#include "TsStreamer.h"

class CColorBarContainerControl;
class PlaylistEventSinkImpl : public ::TianShanIce::Streamer::PlaylistEventSink  
{
public:
	PlaylistEventSinkImpl(CColorBarContainerControl *pCBContainControl);
	virtual ~PlaylistEventSinkImpl();
	virtual void ping(::Ice::Long lv, const ::Ice::Current& ic = ::Ice::Current());
	virtual void OnItemStepped(const ::std::string&, const ::std::string&, ::Ice::Int, ::Ice::Int, const ::TianShanIce::Properties&, const ::Ice::Current& = ::Ice::Current()) const ;
private:
	CColorBarContainerControl  *m_pCBContainControl;
};

#endif // !defined(AFX_PLAYLISTEVENTSINKI_H__C1939314_6314_4050_8DFF_A838AA84172D__INCLUDED_)
