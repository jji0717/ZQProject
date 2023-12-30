// File Name : CRGSessionImpl.h

#ifndef __EVENT_IS_VODI5_CONTEXT_IMPLEMENT_H__
#define __EVENT_IS_VODI5_CONTEXT_IMPLEMENT_H__

#include <IceUtil/IceUtil.h>
#include <fstream>

#include "Log.h"

#include "SsmGBss.h"

namespace  GBss
{

// forward declare
class Environment;

class CRGSessionImpl : public SsmGBss::CRGSession, public IceUtil::AbstractMutexI<IceUtil::Mutex>
{
public:
	typedef IceInternal::Handle<CRGSessionImpl> Ptr;
	
	CRGSessionImpl(ZQ::common::Log& fileLog, Environment& env);

	~CRGSessionImpl();

public:
	virtual void renew(::Ice::Long ttl, const ::Ice::Current& current = ::Ice::Current());

	virtual ::std::string getAnnounceSeq(const ::Ice::Current& current = ::Ice::Current());

	virtual void OnTimer(const ::Ice::Current& current = ::Ice::Current());

	virtual ::std::string getGlobalSessId(const ::Ice::Current& current = ::Ice::Current()) const;

	virtual ::TianShanIce::Properties getMetaData(const ::Ice::Current& current = ::Ice::Current()) const;

	virtual ::TianShanIce::Streamer::StreamPrx getStream(::std::string& strStreamerNetId, ::std::string& strStreamId, const ::Ice::Current& current = ::Ice::Current()) const;

	virtual void setMetaData(const ::TianShanIce::Properties& newMetaData, const ::Ice::Current& current = ::Ice::Current());

	virtual void attachStream(const ::TianShanIce::Streamer::StreamPrx& streamPrx, const ::Ice::Current& current = ::Ice::Current());

	virtual bool destroy(const ::Ice::Current& current = ::Ice::Current());

private:

private:
	ZQ::common::Log& _fileLog;
	Environment& _env;
};

typedef CRGSessionImpl::Ptr CRGSessionImplPtr;

class GBssServantImpl : public SsmGBss::GBssServant
{
public:
	typedef IceInternal::Handle<GBssServantImpl> Ptr;

	GBssServantImpl(ZQ::common::Log& fileLog, Environment& env);

	~GBssServantImpl();

public:
	virtual ::SsmGBss::StreamersStatisticsWithStamp getStreamerInfos(const ::Ice::Current& current = ::Ice::Current());

	virtual ::SsmGBss::ImportChannelsStatisticsWithStamp getImportChannelStat(const ::Ice::Current& current = ::Ice::Current());

	virtual void enableStreamers(const ::TianShanIce::StrValues& streamerNames, bool adminEnabled, const ::Ice::Current& current = ::Ice::Current());

	virtual void resetCounters(const ::Ice::Current& current = ::Ice::Current());

	virtual void updateReplica_async(const ::TianShanIce::AMD_ReplicaSubscriber_updateReplicaPtr& callback, 
		const ::TianShanIce::Replicas& reps, const ::Ice::Current& current = ::Ice::Current());

private:
	void updateSourceXML(const std::string& strFile);

private:
	ZQ::common::Log& _fileLog;
	Environment& _env;
	IceUtil::Mutex _fileLock;
};

typedef GBssServantImpl::Ptr GBssServantImplPtr;

} // end GBss

#endif // end __EVENT_IS_VODI5_CONTEXT_IMPLEMENT_H__
