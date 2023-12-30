#include <NativeThread.h>
#include <NativeThreadPool.h>
#include <Ice/Identity.h>
#include "TsStorage.h"
#include "TsRepository.h"
#include "ContentStore.h"
#include "SystemUtils.h"

#define  PageSize   1000 //page size when listContents page by page

class ContentLibEnv;

class ConnectEventChannelRequest : public ZQ::common::ThreadRequest
{
public: 
	ConnectEventChannelRequest(ContentLibEnv& env);
	virtual ~ConnectEventChannelRequest();

protected:
	virtual bool init();
	virtual int run(void);
	virtual void final(int retcode =0, bool bCancelled =false);

protected: 
	ContentLibEnv&		_env;
	SYS::SingleObject	_event;
	bool				_bExit;

}; // class ConnectIceStromRequest

class SyncContentStoreRequest : public ZQ::common::ThreadRequest
{
public: 
	SyncContentStoreRequest(ContentLibEnv& env, const std::string& strNetId);
	virtual ~SyncContentStoreRequest();

protected:
	virtual bool init();
	virtual int run(void);
	virtual void final(int retcode =0, bool bCancelled =false);

protected: 
	ContentLibEnv& _env;
	std::string  _NetId;

}; // class SyncContentStoreRequest

class AddContentRequest : public ZQ::common::ThreadRequest
{
public: 
	AddContentRequest(ContentLibEnv& env, const std::string& strNetId, const std::string& strVolumeName, const std::string& strContentName, const ::TianShanIce::Storage::ContentInfo& Info, const TianShanIce::Storage::ContentPrx contentProxy);
	virtual ~AddContentRequest();

	static int count() { return _gCount; };

protected:
	virtual bool init();
	virtual int run(void);
	virtual void final(int retcode =0, bool bCancelled =false);

protected: 
	ContentLibEnv&	_env;
	std::string		_NetId;
	std::string		_VolumeName;
	std::string		_ContentName;
	::TianShanIce::Storage::ContentInfo _Info;
	TianShanIce::Storage::ContentPrx _contentProxy;
	static int _gCount;

}; // class AddContentRequest

class LocateCmd : public ZQ::common::ThreadRequest
{
public: 
	LocateCmd(ContentLibEnv& env, const ::TianShanIce::Repository::AMD_ContentLib_locateContentPtr& amdCB, const ::TianShanIce::Properties& searchForMetaData, const ::TianShanIce::StrValues& expectedMetaDataNames, ::Ice::Int maxHop, bool pingOnly);
	virtual ~LocateCmd();

public:

	void execute(void) { start(); }

protected:

	virtual bool init(void)	{ return true; };
	virtual int run(void);

	// no more overwrite-able
	void final(int retcode =0, bool bCancelled =false) { delete this; }

protected: 
	ContentLibEnv&	_env;
	::TianShanIce::Repository::AMD_ContentLib_locateContentPtr _amdCB;
	::TianShanIce::Properties _searchForMetaData;
	::TianShanIce::StrValues _expectedMetaDataNames;
	::Ice::Int _maxHop;
	bool _pingOnly;
}; // class LocateCmd

class DeleteContentRequest : public ZQ::common::ThreadRequest
{
public: 
	DeleteContentRequest(ContentLibEnv& env, const std::string& strNetId, const std::string& strVolumeName, const std::string& strContentName);
	virtual ~DeleteContentRequest();

protected:
	virtual bool init();
	virtual int run(void);
	virtual void final(int retcode =0, bool bCancelled =false);

protected: 
	ContentLibEnv&	_env;
	std::string		_NetId;
	std::string		_VolumeName;
	std::string		_ContentName;
}; // class DeleteContentRequest

class PageSizeContentRequest : public ZQ::common::ThreadRequest
{
public: 
	PageSizeContentRequest(ContentLibEnv& env, TianShanIce::Storage::ContentInfos& contentInfos, std::vector<std::string>& contentsInDB, const std::string& strNetId, const std::string& strVolumeName);
	virtual ~PageSizeContentRequest();

protected:
	virtual bool init();
	virtual int run(void);
	virtual void final(int retcode =0, bool bCancelled =false);

protected: 
	ContentLibEnv&	_env;
	TianShanIce::Storage::ContentInfos& _ContentInfos;
	std::vector<std::string> _ContentsInDB;
	std::string		_NetId;
	std::string		_VolumeName;
}; // class PageSizeContentRequest

class UpdateContentRequest : public ZQ::common::ThreadRequest
{
public: 
	UpdateContentRequest(ContentLibEnv& env, const std::string& strNetId, const std::string& strVolumeName, const std::string& strContentName, const ::TianShanIce::Storage::ContentInfo& Info);
	virtual ~UpdateContentRequest();

protected:
	virtual bool init();
	virtual int run(void);
	virtual void final(int retcode =0, bool bCancelled =false);

protected: 
	ContentLibEnv&	_env;
	std::string		_NetId;
	std::string		_VolumeName;
	std::string		_ContentName;
	::TianShanIce::Storage::ContentInfo _Info;
}; // class UpdateContentRequest

///add by HL
class FullSyncRequest : public ZQ::common::ThreadRequest
{
public: 
	FullSyncRequest(ContentLibEnv& env, const std::string netId);
	virtual ~FullSyncRequest();

protected: // impls of ScheduleTask
	virtual bool init();
	virtual int run(void);
	virtual void final(int retcode =0, bool bCancelled =false);
    bool syncVolume();
	bool syncContent();
protected: 
	ContentLibEnv&	_env;
	std::string     _netId;
	TianShanIce::Storage::VolumeInfos _volumeInfos;
	TianShanIce::Storage::ContentStorePrx _contentStoreProxy;
}; // class FullSyncRequest

class LocateByPidPAidCmd : public ZQ::common::ThreadRequest
{
public: 
	LocateByPidPAidCmd(ContentLibEnv& env, const ::TianShanIce::Repository::AMD_ContentLib_locateContentByPIDAndPAIDPtr& amdCB, const ::std::string& netId, const ::std::string& volumeId, const ::std::string& providerId, const ::std::string& providerAssetId, const ::TianShanIce::StrValues& expectedMetaDataNames);
	virtual ~LocateByPidPAidCmd();

public:

	void execute(void) { start(); }

protected:

	virtual bool init(void)	{ return true; };
	virtual int run(void);

	// no more overwrite-able
	void final(int retcode =0, bool bCancelled =false) { delete this; }

protected: 
	ContentLibEnv&	_env;
	::TianShanIce::Repository::AMD_ContentLib_locateContentByPIDAndPAIDPtr _amdCB;
	::std::string _netId;
	::std::string _volumeId;
	::std::string _providerId;
	::std::string _providerAssetId;
	::TianShanIce::StrValues _expectedMetaDataNames;
}; // class LocateByPidPAidCmd

class LocateVolumesByNidCmd : public ZQ::common::ThreadRequest
{
public: 
	LocateVolumesByNidCmd(ContentLibEnv& env, const ::TianShanIce::Repository::AMD_ContentLib_locateVolumesByNetIDPtr& amdCB, const ::std::string& netId, const ::TianShanIce::StrValues& expectedMetaDataNames);
	virtual ~LocateVolumesByNidCmd();

public:

	void execute(void) { start(); }

protected:

	virtual bool init(void)	{ return true; };
	virtual int run(void);

	// no more overwrite-able
	void final(int retcode =0, bool bCancelled =false) { delete this; }

protected: 
	ContentLibEnv&	_env;
	::TianShanIce::Repository::AMD_ContentLib_locateVolumesByNetIDPtr _amdCB;
	::std::string _netId;
	::TianShanIce::StrValues _expectedMetaDataNames;
}; // class LocateByNidVidCmd

