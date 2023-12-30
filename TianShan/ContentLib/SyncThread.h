// stl
#include <string>
#include <vector>

// zq common
#include "NativeThread.h"
#include "Locks.h"
#include "SystemUtils.h"

//tinshan ice
#include "TsStorage.h"

#include "ContentLibImpl.h"

class SyncThread : public ZQ::common::NativeThread
{
public:
	SyncThread(ContentLibImpl::Ptr contentLibPtr);
	~SyncThread();
	bool	notify();
public: 
	void	stop();

protected: 
	virtual bool init(void);
	virtual int	 run();
	virtual void final(void);
	bool			runNotify();
	bool            runTimeout();
	bool			openHandles();
	void			closeHandles();

protected: 
	ContentLibImpl::Ptr _contentLibPtr;
	ZQ::common::Mutex	_lock;
	SYS::SingleObject	_hNotify;
	bool			_bStop;
	uint32			_dTime;
}; // class SyncThread	

class ContentCacheThread : public ZQ::common::NativeThread
{
	struct CurrentCache
	{
		std::string _netId;
		std::string _volumeName;
		bool bExpire;
		std::vector < std::string > _cacheList;
	};
public:
	ContentCacheThread(ContentLibImpl::Ptr contentLibPtr, ::ZQTianShan::MetaLib::MetaLibImpl& lib);
	~ContentCacheThread();
	bool	notify();
public: 
	void	stop();
	TianShanIce::Repository::MetaObjectInfos getContentList(std::string NetId, std::string VolumeName, int startCount, int maxCount, int& size, const ::TianShanIce::StrValues& expectedMetaDataNames);
	bool		addContent(std::string NetId, std::string VolumeName, std::string ContentName);
	bool        removeContent(std::string NetId, std::string VolumeName, std::string ContentName);

protected: 
	virtual bool init(void);
	virtual int	 run();
	virtual void final(void);
	bool			runNotify();
	bool            runTimeout();
	bool			openHandles();
	void			closeHandles();
	bool            rebuild();
	bool			cache(std::string NetId, std::string VolumeName);
	bool            cleanCache();

private: 
	ContentLibImpl::Ptr _contentLibPtr;
	::ZQTianShan::MetaLib::MetaLibImpl& _lib;
	ZQ::common::Mutex	_lockOfMap;
	ZQ::common::Mutex	_lockOfCache;
	ZQ::common::Mutex	_lockOfTime;
	SYS::SingleObject	_hNotify;
	bool			_bStop;
	uint32			_dTime;
	Ice::Long		_lastQuery;
	std::map<std::string , std::vector < std::string > > _ContentsInDB;
	CurrentCache _currentCache;
}; // class ContentCacheThread	
