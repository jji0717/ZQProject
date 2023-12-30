
#ifndef __ngod_session_view_implementation_header_file_h__
#define __ngod_session_view_implementation_header_file_h__
#include <fstream>
#include "./Ice/ngod.h"


namespace NGOD
{

class NgodSessionManager;
struct ViewData
{
	NGOD::CtxDatas _all;
	NGOD::CtxDatas _someGroup;
	Ice::Long _lastAccessTime;
};

class SessionViewImpl : public NGOD::SessionView, public IceUtil::AbstractMutexI<IceUtil::RecMutex>
{
public:
	SessionViewImpl( NgodEnv& env , NgodSessionManager& sessManager );
	virtual ~SessionViewImpl(void);
	typedef std::map<std::string, std::vector<std::string> > SopMap;
public:
	virtual ::Ice::Int		getTimeoutValue(const ::Ice::Current& = ::Ice::Current());
	virtual ::Ice::Int		getAllContexts(::Ice::Int& clientId, const ::Ice::Current& = ::Ice::Current());
	virtual ::Ice::Int		getContextsBySG(const ::std::string&, const ::Ice::Current& = ::Ice::Current());
	virtual NGOD::CtxDatas	getRange(::Ice::Int, ::Ice::Int, const ::Ice::Current& = ::Ice::Current());
	virtual NGOD::CtxDatas	getRangeBySG(::Ice::Int, ::Ice::Int, const ::std::string&, const ::Ice::Current& = ::Ice::Current());
	virtual void			getNgodUsage(::NGOD::NgodUsage&, ::std::string&, const ::Ice::Current& = ::Ice::Current());
	virtual void			getImportChannelUsage(::NGOD::ImportChannelUsageS&, const ::Ice::Current& = ::Ice::Current());
	virtual void			resetCounters(const ::Ice::Current& = ::Ice::Current());
	virtual void			enableStreamers(const ::TianShanIce::StrValues& streamerNames, bool enable, const ::Ice::Current& cur = ::Ice::Current());

private:

	void updateSOPXML(const std::string strFile);
	void groupBySop(const TianShanIce::StrValues& streamerNames, SopMap& streamersOfSop);

	void clearResource();

private:
	NgodEnv&						mEnv;
	NgodSessionManager&				mSessManager;
	std::map<Ice::Int, ViewData*>	_viewDataMap;
	IceUtil::RecMutex				_lockMap;
	IceUtil::Mutex					_fileLock;
	Ice::Int						_nextClientId;
};
typedef IceUtil::Handle<SessionViewImpl> SessionViewImplPtr;

}
#endif//__ngod_session_view_implementation_header_file_h__
