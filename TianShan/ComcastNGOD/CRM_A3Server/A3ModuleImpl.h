// FileName : A3ModuleImpl.h
// Author   : Junming Zheng
// Date     : 2009-05
// Desc     :

#ifndef  __CRG_PLUGIN_A3SERVER_MODULE_IMPLEMENT_H__
#define  __CRG_PLUGIN_A3SERVER_MODULE_IMPLEMENT_H__

#include "TianShanDefines.h"
#include "A3Module.h"
#include "AssetIdx.h"
#include "VolumeIdx.h"

namespace CRG
{
namespace Plugin
{
namespace A3Server
{

class A3Client;

class A3ContentI : public virtual A3Module::A3Content, 
	               public virtual ::IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
{
public:
	A3ContentI(Ice::Identity contentIdent, 
		TianShanIce::Storage::ContentPrx contentProxy, 
		TianShanIce::Properties contentMetaData, 
		TianShanIce::Storage::ContentState contentState,
		std::string strResponseURL = "");
	A3ContentI();
	~A3ContentI();

public:
	typedef IceUtil::Handle<A3ContentI> Ptr;
	 virtual void getAssetId(::std::string& providerId, ::std::string& providerAssetId, const ::Ice::Current& cur) const;

	 virtual void getVolumeInfo(::std::string& contentStoreNetId, ::std::string& volumeName, const ::Ice::Current& cur) const;

	 virtual ::TianShanIce::Properties getMetaData(const ::Ice::Current& cur) const;

     virtual ::TianShanIce::Storage::ContentState getState(const ::Ice::Current& cur) const;  

	 virtual ::TianShanIce::Storage::ContentPrx theContent(const ::Ice::Current& cur) const;

	 virtual ::Ice::Identity getIdentity(const ::Ice::Current&cur) const;

	 virtual ::std::string getResponseURL(const ::Ice::Current& cur) const;

	 virtual ::TianShanIce::StatedObjInfo getInfo(const ::TianShanIce::StrValues& expectedMetaData, const ::Ice::Current& cur) const;

	 virtual ::TianShanIce::Properties getUpdateMetaData(const ::Ice::Current& cur);

	 virtual ::TianShanIce::Storage::ContentState getUpdateState(const ::Ice::Current& cur);

	 virtual void OnContentEvent(const ::std::string& contentEventName, const ::TianShanIce::Properties& params, const ::Ice::Current& cur);
};
typedef A3ContentI::Ptr A3ContentIPtr;

class A3FacedeI : virtual public A3Module::A3Facede
{
public:
	A3FacedeI(ZQADAPTER_DECLTYPE pAdapter, 
		Freeze::EvictorPtr a3Content,
		A3Module::AssetIdxPtr assetIdx, 
		A3Module::VolumeIdxPtr volumeIdx,
		A3Client* a3Client,
		std::map<std::string, TianShanIce::Storage::ContentStorePrx>& contentStoreProxies,
		std::string strAdminURL = "");
	~A3FacedeI();
	typedef IceUtil::Handle<A3FacedeI> Ptr;

public:
	virtual ::A3Module::A3ContentPrx openA3Content(const ::std::string& provideId, 
		const ::std::string& providerAssetId, 
		const ::std::string& contentStoreNetId, 
		const ::std::string& volumeName, 
		const ::Ice::Current& cur = ::Ice::Current());

	virtual ::A3Module::A3Contents findContentsByAsset(const ::std::string& providerId, 
		const ::std::string&  providerAssetId, 
		const ::Ice::Current& cur = ::Ice::Current());

	virtual ::A3Module::A3Contents listContentsByVolume(const ::std::string& contentStoreNetId, 
		const ::std::string& volumeName, 
		const ::Ice::Current& cur = ::Ice::Current());

	virtual ::A3Module::A3Assets listAssets(const ::std::string&providerId, 
		const ::std::string&providerAssetId, 
		::Ice::Int maxCount, 
		bool included,
		const ::Ice::Current& cur = ::Ice::Current());

	virtual bool deleteA3Content(const ::std::string& provideId, 
		const ::std::string& providerAssetId, 
		const ::std::string& contentStoreNetId, 
		const ::std::string& volumeName, 
		const ::Ice::Current& cur = ::Ice::Current());
	 
	virtual ::std::string getAdminUri(const ::Ice::Current& cur = ::Ice::Current());
	 
	virtual ::TianShanIce::State getState(const ::Ice::Current& cur = ::Ice::Current());

public:
	virtual void post(const ::std::string& category, ::Ice::Int eventId, const ::std::string& eventName, 
		const ::std::string& stampUTC, const ::std::string& sourceNetId, 
		const ::TianShanIce::Properties& params, const ::Ice::Current& cur);

	// TODO : implement ping?
	virtual void ping(::Ice::Long timestamp, const ::Ice::Current& cur);

public:
	/// update A3 Contents lib
	void updateA3ContentLib();

	/// get volume names from contentStore
	bool getVolumes(const std::string& strNetId, std::vector<std::string>& volumeNames);

	/// create content in volume
	A3Module::A3ContentPrx createContent(const std::string& strPID, const std::string& strPAID,
		const std::string& strNetId, const std::string& strVolume, 
		const std::string& strResponseURL = "", std::string strContentType = "");

	/// get special volume info
	bool getVolumeInfo(const std::string& strNetId, const std::string& strVolume, 
		Ice::Long& totalMB, Ice::Long& freeMB, int& state);


private:
	void updateContentStore(const std::string& strNetId);

	bool getContentStoreProxy(const std::string& strNetId, 	TianShanIce::Storage::ContentStorePrx& contentStoreProxy );

	/// add content into freeze
	A3Module::A3ContentPrx addContentToEvictor(const std::string& strPID, const std::string& strPAID,
		const std::string& strNetId, const std::string& strVolume, const std::string& strResponseURL = "");
	A3Module::A3ContentPrx addContentToEvictor(const Ice::Identity& contentIdent, 
		const std::string& strResponseURL = "");

	/// delete content from freeze
	void deleteContentFromEvictor( const std::string& strPID, const std::string& strPAID,
		const std::string& strNetId, const std::string& strVolume);
	void deleteContentFromEvictor(const Ice::Identity& contentIdent);

	void sendTransferStatus(const std::string& strPID, const std::string& strPAID, 
		const std::string& strNetId, const std::string& strVolume, const std::string& contentState);

	/// open volume 
	inline bool openVolume(const std::string& strNetId, const std::string& strVolumeName,
		TianShanIce::Storage::VolumePrx& volumeProxy);

	/// open content
	inline bool openContentByFullname(const std::string& strNetId, const std::string& strContentName, 
		TianShanIce::Storage::ContentPrx& contentProxy);

private:
	void addAsset(const Ice::Identity& contentIdent);
	void addAsset(const std::string& strPID, const std::string& strPAID);
	void deleteAsset(const Ice::Identity& contentIdent);
	void deleteAsset(const std::string& strPID, const std::string& strPAID);
private:
	ZQADAPTER_DECLTYPE _pAdapter;
	Freeze::EvictorPtr _a3Content;
	A3Module::AssetIdxPtr _assetIdx;
	A3Module::VolumeIdxPtr _volumeIdx;
	A3Client* _a3Client;
	std::map<std::string, TianShanIce::Storage::ContentStorePrx>& _contentStroeProxies;
	TianShanIce::State _state;
	std::string _strAdminURL;

private:
	::IceUtil::RWRecMutex _assetMutex;
	::A3Module::A3Assets _a3Assets;

};
typedef A3FacedeI::Ptr A3FacedeIPtr;

class A3ContentFactory : public Ice::ObjectFactory
{
public:
	typedef IceUtil::Handle<A3ContentFactory> Ptr;
	A3ContentFactory(ZQADAPTER_DECLTYPE pAdapter);
	~A3ContentFactory();

	// Operations from ObjectFactory
	virtual Ice::ObjectPtr create(const std::string& type);
	virtual void destroy();
private:
	ZQADAPTER_DECLTYPE _pAdapter;
};
typedef A3ContentFactory::Ptr A3ContentFactoryPtr;


} // end for A3Server
} // end for Plugin
} // end for CRG

#endif

