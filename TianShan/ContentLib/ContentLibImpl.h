#include <TsRepository.h>
#include "ContentReplicaImpl.h"
#include "ContentStoreReplicaImpl.h"
#include "MetaVolumeImpl.h"
#include "MetaLibImpl.h"
#include "ContentLibFactory.h"

class ContentLibEnv;

class ContentLibImpl : virtual public ::TianShanIce::Repository::ContentLib //, virtual public ::ZQTianShan::MetaLib::MetaLibImpl
{
public:
	ContentLibImpl(::ZQTianShan::MetaLib::MetaLibImpl& _lib, ContentLibEnv& env);
	~ContentLibImpl();

public:

	typedef ::IceInternal::Handle< ContentLibImpl > Ptr;

	virtual ::TianShanIce::Repository::ContentStoreReplicaPrx toStoreReplica(const ::std::string&, const ::Ice::Current& = ::Ice::Current());

	virtual ::TianShanIce::Repository::MetaVolumePrx toVolume(const ::std::string&, const ::Ice::Current& = ::Ice::Current());

	virtual ::TianShanIce::Repository::ContentReplicaPrx toContentReplica(const ::std::string&, const ::Ice::Current& = ::Ice::Current());

	virtual void locateContent_async(const ::TianShanIce::Repository::AMD_ContentLib_locateContentPtr&, const ::TianShanIce::Properties&, const ::TianShanIce::StrValues&, ::Ice::Int, bool, const ::Ice::Current& = ::Ice::Current());

	virtual void locateContentByPIDAndPAID_async(const ::TianShanIce::Repository::AMD_ContentLib_locateContentByPIDAndPAIDPtr&, const ::std::string&, const ::std::string&, const ::std::string&, const ::std::string&, const ::TianShanIce::StrValues&, const ::Ice::Current& = ::Ice::Current());

	virtual void locateContentByNetIDAndVolume_async(const ::TianShanIce::Repository::AMD_ContentLib_locateContentByNetIDAndVolumePtr&, const ::std::string&, const ::std::string&, const ::TianShanIce::StrValues&, ::Ice::Int, ::Ice::Int, const ::Ice::Current& = ::Ice::Current()) const;

	virtual void locateVolumesByNetID_async(const ::TianShanIce::Repository::AMD_ContentLib_locateVolumesByNetIDPtr&, const ::std::string&, const ::TianShanIce::StrValues&, const ::Ice::Current& = ::Ice::Current());

	virtual ::TianShanIce::Repository::MetaObjectInfo openObject(const ::std::string&, const ::TianShanIce::StrValues&, const ::Ice::Current& = ::Ice::Current());

	virtual ::std::string createObject(const ::std::string&, ::Ice::Long, const ::Ice::Current& = ::Ice::Current());

	virtual ::std::string getAdminUri(const ::Ice::Current& = ::Ice::Current());

	virtual ::TianShanIce::State getState(const ::Ice::Current& = ::Ice::Current());

	virtual void lookup_async(const ::TianShanIce::Repository::AMD_MetaDataLibrary_lookupPtr&, const ::std::string&, const ::TianShanIce::Properties&, const ::TianShanIce::StrValues&, const ::Ice::Current& = ::Ice::Current());

	virtual void ping(::Ice::Long timestamp, const ::Ice::Current&);

	virtual void post(
		const ::std::string& category,
		::Ice::Int eventId,
		const ::std::string& eventName,
		const ::std::string& stampUTC,
		const ::std::string& sourceNetId,
		const ::TianShanIce::Properties& params,
		const ::Ice::Current& /* = ::Ice::Current */
		);

	bool syncContentStore(const std::string& strNetId);

	void addContentStoreReplica(const std::string& strNetId, const std::string& strEndpoint);

	bool connectContentStore(const std::string& strEndpoint, std::string& strNetId);

public:
	void addContentReplica(const std::string& strNetId, const std::string& strVolume, const std::string& strContentName);

	void updateContentReplica(const std::string& strNetId, const std::string& strVolume, const std::string& strContentName, const ::TianShanIce::Storage::ContentInfo& Info);

	void addMetaVolume(const std::string& strNetId, const std::string& strVolume);

	void deleteContentReplica(const std::string& strNetId, const std::string& strVolume, const std::string& strContentName);

	void deleteContentStoreReplica(const std::string& strNetId);

	void deleteMetaVolume(const std::string& strNetId, const std::string& strVolume);

	void updateContentMetadata(const std::string& strNetId, const std::string& strVolume, const std::string& strContentName);

	bool getContentStoreProxy(const std::string& strNetId, 	TianShanIce::Storage::ContentStorePrx& contentStoreProxy);

	bool updateContentStore(const std::string& strNetId);

	bool removeContentStore(const std::string&netId, const std::string& strEndpoint);

private:
//	Ice::ObjectAdapterPtr _adapter;
	ZQTianShan::MetaLib::ContentLibFactory::Ptr _factory;
	::ZQTianShan::MetaLib::MetaLibImpl& _lib;
	TianShanIce::State _state;
	std::string _strAdminURL;
	std::map<std::string, TianShanIce::Storage::ContentStorePrx> _contentStroeProxies;
	ZQ::common::Mutex _mutex; // lock for _contentStroeProxies
	ContentLibEnv& _env;
	Ice::Long timeToSync;
	Ice::Long contentCount;
};
