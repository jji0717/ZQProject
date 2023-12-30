#ifndef _tianshan_ngod_environment_header_file_h__
#define _tianshan_ngod_environment_header_file_h__

//#include "ZQ_common_conf.h"
//#include "SelectionResourceManager.h"

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include "TianShanDefines.h"
#include <Log.h>
#include <Locks.h>

#include "ZQ_common_conf.h"
#include "SelectionResourceManager.h"

#include "snmp/ZQSnmp.h"

namespace NGOD
{

class NgodEnv
{
public:
	NgodEnv(ZQ::common::Log& mainLogger, ZQ::common::Log& eventLogger);
	virtual ~NgodEnv(void);

public:

	inline Ice::CommunicatorPtr& getCommunicator()
	{
		return mIc;
	}

	inline ZQADAPTER_DECLTYPE getObjAdapter( )
	{
		return mAdapter;
	}

	inline ZQ::common::Log*		getMainLogger()
	{
		return mMainLogger;
	}

	inline ZQ::common::Log*		getEventLogger( )
	{
		return mEventLogger;
	}
	inline const std::string&	moduleName( ) const
	{
		return mModuleName;
	}
	inline SelectionEnv&		getSelEnv( )
	{
		return mSelEnv;
	}
	inline NgodResourceManager&	getSelResManager( )
	{
		return mSelManager;
	}
	inline ZQ::common::NativeThreadPool& getThreadPool( )
	{
		return mThreadPool;
	}
	inline std::map<std::string , std::string >& getFailoverTestStreamers(void)
	{
		return mFailoverTestStreamers;
	}
	SelectionRMSessionStateCache& getStatCache(void)
	{
		return mStatCache; 
	}
	uint16						incAndGetSeqNumber( );
	
	void						updateLastEventRecvTime( int64 t);
	
	int64						getLastEventRecvTime( ) const;

public:
	
	std::string						mModuleName;
	ZQ::common::NativeThreadPool	mThreadPool;
	Ice::CommunicatorPtr			mIc;
	ZQADAPTER_DECLTYPE				mAdapter;
	ZQ::common::Log*				mMainLogger;
	ZQ::common::Log*				mEventLogger;	
	SelectionEnv					mSelEnv;
	NgodResourceManager				mSelManager;
	uint16							mGlobaSeq;
	SelectionRMSessionStateCache    mStatCache;

	//std::vector<std::string>		mFailoverTestStreamers;
	std::map<std::string , std::string > mFailoverTestStreamers;
private:
	int64							mLastEventRecvTime;

public: // snmp porting
	ZQ::SNMP::ModuleMIB         _mmib;
	ZQ::SNMP::SubAgent          _snmpSA;

	uint32 snmp_dummyGet() { return 0; } // {uint32 i; snmp_refreshSOPUsage(i); return 0; }
	void snmp_refreshSOPUsage(const uint32&);
	void snmp_refreshIcUsage(const uint32&);

	uint32 snmp_getLogLevel_Main() { return mMainLogger ? mMainLogger->getVerbosity() :0; }
	void   snmp_setLogLevel_Main(const uint32& newLevel) { if (mMainLogger) mMainLogger->setVerbosity(newLevel); }
	uint32 snmp_getLogLevel_Event() { return mEventLogger ? mEventLogger->getVerbosity() :0; }
	void   snmp_setLogLevel_Event(const uint32& newLevel) { if (mEventLogger) mEventLogger->setVerbosity(newLevel); }
};

}

#define MLOG				(*mEnv.getMainLogger())
#define ELOG				(*mEnv.getEventLogger())


#define PROXY2STR(x)		(mEnv.getCommunicator()->proxyToString(x))
#define STR2PROXY(x)		(mEnv.getCommunicator()->stringToProxy(x))
/*
#ifndef min
#define min(x,y) (x>y?y:x)
#endif

#ifndef max
#define max(x,y) (x>y?x:y)
#endif
*/
#define SYSKEY(x)	"syskey."#x


#define EVENTLOGFMT(_X, _T) "%-18s Sess(%s)Seq(%s)Req(%p)Mtd(%s)ODSess(%s) " _T, #_X, request->sessionId.c_str(), request->cseq.c_str(), request.get(), request->verbstr.c_str(), request->ondemandId.c_str()

#ifdef ZQ_OS_LINUX
#define _vsnprintf		vsnprintf
#define _vsnwprintf		vswprintf
#define _snprintf		snprintf
#define _snwprintf		vswprintf

#include <ctype.h>
#endif

#endif//_tianshan_ngod_environment_header_file_h__
