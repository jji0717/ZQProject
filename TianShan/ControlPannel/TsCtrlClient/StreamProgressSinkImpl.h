// StreamProgressSinkImpl.h: interface for the StreamProgressSinkImpl class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __STREAMPROGRESSSINK_H
#define __STREAMPROGRESSSINK_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "ZQEventsCtrl.h"

class StreamProgressSinkImpl : public TianShanIce::Streamer::StreamProgressSink  
{
public:
	StreamProgressSinkImpl(RegEvent_Proc pFun);
	StreamProgressSinkImpl();
	virtual ~StreamProgressSinkImpl();
	void SetFunction(RegEvent_Proc pFun);

	virtual void OnProgress(const ::std::string& proxy ,const ::std::string& uid, ::Ice::Int done, ::Ice::Int total, ::Ice::Int step, ::Ice::Int totalsteps,  const ::std::string& comment, const ::Ice::Current&  ic = ::Ice::Current()) const;
	virtual void ping(::Ice::Long timestamp, const ::Ice::Current& ic = ::Ice::Current());
private:
	RegEvent_Proc m_pFun;
	char *m_cUid;
	char *m_cComment;
	int  m_iCurCtrlNum;
};
extern RegPlayListStateProc m_StateProc; // for the playliststate proc

#endif // !defined(AFX_STREAMPROGRESSSINKIMPL_H__4558925F_4826_4BC0_A9DA_50CCAA5EDB02__INCLUDED_)
