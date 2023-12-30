#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#define _WINSOCK2API_

#include "NGODr2c1.h"
#include <TsStreamer.h>
#include <IceUtil/IceUtil.h>
#include <Locks.h>
#include <vector>
#include <map>
#include <NativeThread.h>
#include <string>

class NGODEnv;

namespace NGODr2c1
{
	
class ContextImpl : public Context, public IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
{
public:

	typedef IceInternal::Handle<ContextImpl> Ptr;

	ContextImpl(NGODEnv& env);
	~ContextImpl();
	
	virtual void addEventRecord(const ::NGODr2c1::SessionEventRecord& eventRecord, const ::Ice::Current& cur = ::Ice::Current());
	virtual void getState(::NGODr2c1::ctxData& data, const ::Ice::Current& = ::Ice::Current())  const;
	virtual void renew(::Ice::Long ttl, const ::Ice::Current& = ::Ice::Current());
	virtual void increaseAnnSeq(const ::Ice::Current& = ::Ice::Current());
	virtual void onTimer(const ::Ice::Current& = ::Ice::Current());

	virtual ::TianShanIce::Streamer::StreamPrx getStream(const ::Ice::Current& c= ::Ice::Current()) const;

	virtual void updateCtxProp(const ::std::string& key, const ::std::string& val, const ::Ice::Current& = ::Ice::Current());

	virtual ::std::string getCtxPropItem(const ::std::string& key, const ::Ice::Current& c= ::Ice::Current()) const;
protected:

	NGODEnv& _env;
	TianShanIce::Streamer::StreamPrx _streamPrx;
	
};
typedef ContextImpl::Ptr ContextImplPtr;
typedef IceInternal::Handle<ctxData> ctxDataPtr;

struct ViewData
{
	CtxDatas _all;
	CtxDatas _someGroup;
	Ice::Long _lastAccessTime;
};

class SessionViewImpl : public IceUtil::AbstractMutexI<IceUtil::RecMutex>, public SessionView
{
public: 
	typedef IceInternal::Handle<SessionViewImpl> Ptr;
    typedef std::map<std::string, std::vector<std::string> > SopMap;

	SessionViewImpl(NGODEnv& env);
	virtual ~SessionViewImpl();
	virtual ::Ice::Int getTimeoutValue(const ::Ice::Current& = ::Ice::Current());
	virtual ::Ice::Int getAllContexts(::Ice::Int& clientId, const ::Ice::Current& = ::Ice::Current());
	virtual ::Ice::Int getContextsBySG(const ::std::string&, const ::Ice::Current& = ::Ice::Current());
	virtual CtxDatas getRange(::Ice::Int, ::Ice::Int, const ::Ice::Current& = ::Ice::Current());
	virtual CtxDatas getRangeBySG(::Ice::Int, ::Ice::Int, const ::std::string&, const ::Ice::Current& = ::Ice::Current());
	virtual void getNgodUsage(::NGODr2c1::NgodUsage&, ::std::string&, const ::Ice::Current& = ::Ice::Current());
	virtual void getImportChannelUsage(::NGODr2c1::ImportChannelUsageS&, const ::Ice::Current& = ::Ice::Current());
	virtual void resetCounters(const ::Ice::Current& = ::Ice::Current());
	virtual void enableStreamers(const ::TianShanIce::StrValues& streamerNames, bool enable, const ::Ice::Current& cur = ::Ice::Current());
private:
	void updateSOPXML(const std::string strFile);
	void groupBySop(const TianShanIce::StrValues& streamerNames, SopMap& streamersOfSop);
private: 
	NGODEnv& _env;
	std::map<Ice::Int, ViewData*> _viewDataMap;
	IceUtil::RecMutex _lockMap;
	IceUtil::Mutex    _fileLock;
	Ice::Int _nextClientId;

}; // class SessionView

typedef SessionViewImpl::Ptr SessionViewImplPtr;

}

#endif // __CONTEXT_H__