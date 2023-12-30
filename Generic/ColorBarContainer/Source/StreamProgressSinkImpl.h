// StreamProgressSinkImpl.h: interface for the CStreamProgressSinkImpl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STREAMPROGRESSSINKIMPL_H__71858431_5D93_4C66_A7A5_1D090806548E__INCLUDED_)
#define AFX_STREAMPROGRESSSINKIMPL_H__71858431_5D93_4C66_A7A5_1D090806548E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <IceStorm/IceStorm.h>
#include "TsStreamer.h"

class CColorBarContainerControl;
class StreamProgressSinkImpl  : public ::TianShanIce::Streamer::StreamProgressSink
{
public:
	StreamProgressSinkImpl(CColorBarContainerControl  *pCBContainControl);
	StreamProgressSinkImpl();
	virtual ~StreamProgressSinkImpl();

	virtual void OnProgress(const ::std::string& proxy ,const ::std::string& uid, ::Ice::Int done, ::Ice::Int total, ::Ice::Int step, ::Ice::Int totalsteps,  const ::std::string& comment, const ::Ice::Current&  ic = ::Ice::Current()) const;
	virtual void ping(::Ice::Long timestamp, const ::Ice::Current& ic = ::Ice::Current());

private:
	std::string  GetItemNum(const std::string &ctrlNum);

private:
	CColorBarContainerControl  *m_pCBContainControl;


};

#endif // !defined(AFX_STREAMPROGRESSSINKIMPL_H__71858431_5D93_4C66_A7A5_1D090806548E__INCLUDED_)
