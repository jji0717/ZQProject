#ifndef __RTSPTHREADREQUEST_H__
#define __RTSPTHREADREQUEST_H__

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <TianShanDefines.h>

#include "common_structure.h"

#define SENDBUFMAXSIZE 2048
class RTSPRequestSender : public ::ZQ::common::ThreadRequest
{
public:
	RTSPRequestSender(::ZQ::common::FileLog *logFile, ::ZQ::common::NativeThreadPool &pool, CVSSRtspSession *sess);
	~RTSPRequestSender();

	int run(void);
	void final(int retcode, bool bCancelled)
	{
		delete this;
	}
private:
	::ZQ::common::FileLog *_pLog;
	CVSSRtspSession *_pSess;
	char hint[128];
};

class RTSPResponseRecver : public ::ZQ::common::ThreadRequest
{
public:
	RTSPResponseRecver(::ZQ::common::FileLog *logFile, ::ZQ::common::NativeThreadPool &pool, CVSSRtspSession *sess,  RtspCSeqSignal &rtspCSeqSignal, ::Ice::CommunicatorPtr communicator, ZQADAPTER_DECLTYPE adapter, bool IsRelease);
	~RTSPResponseRecver();

	int run(void);
	void final(int retcode, bool bCancelled)
	{
		delete this;
	}
private:
	::ZQ::common::FileLog *_pLog;
	CVSSRtspSession *_pSess;
	RtspCSeqSignal &_rtspCSeqSignal;
	bool _isReleas;
	char hint[128];

	::Ice::CommunicatorPtr	_communicator;
	ZQADAPTER_DECLTYPE	_adapter;

	void chopMsg(::std::string &strMsg, strlist &msgList);
	bool parseMsg(::std::string &msg);
	void sendAnnounceResponse(CVSSRtspSession *cvssRtspSession, uint32 iCSeq);

};
#endif __RTSPTHREADREQUEST_H__