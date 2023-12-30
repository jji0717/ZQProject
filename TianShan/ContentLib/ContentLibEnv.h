#include "time.h"
#include "Locks.h"
#include "TianShanDefines.h"
#include "MetaLibImpl.h"

#include "Guid.h"
#include "Log.h"
#include "ZQResource.h"
#include "FileLog.h"

//#include "ContentLibImpl.h"
#include "EventChannel.h"
#include "SyncThread.h"

#include "TianShanIce.h"
#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>

class ContentLibEnv
{
public:
	ContentLibEnv(Ice::CommunicatorPtr& communicator);
	~ContentLibEnv();

	bool init();

	void unInit();

public:
	bool ConnectEventChannel();
	bool sync(std::string netId);
	void initContentStore();

	ZQ::common::NativeThreadPool*				_pThreadPool;
	ZQ::common::NativeThreadPool*				_pThreadPoolForContents;
	::ZQTianShan::MetaLib::MetaLibImpl::Ptr		_lib;
	SyncThread*                                 _pSyncThread;
	ContentCacheThread*                         _pContentCacheThread;
	TianShanIce::Events::GenericEventSinkPtr	_contentlib;
	TianShanIce::Events::EventChannelImpl::Ptr	_eventChannel;
	Ice::CommunicatorPtr						_communicator;

	::Ice::ObjectAdapterPtr					_evtAdap;
	ZQADAPTER_DECLTYPE                      _adapter;
	std::string								_dbPath;
	std::string								_endpoint;
	ZQ::common::Mutex						_gMutex;

protected:
	bool					_bInited;

};
