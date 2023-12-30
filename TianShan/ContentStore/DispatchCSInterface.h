#ifndef _IBASE_C_S_H
#define _IBASE_C_S_H

#include "ContentImpl.h"

namespace ZQTianShan {
namespace ContentStore {

class IDispatchCS
{
public:
	virtual ~IDispatchCS(){};
	virtual void initializePortal(ContentStoreImpl& store) = 0;
	virtual void uninitializePortal(ContentStoreImpl& store) = 0;
	virtual std::string fixupPathname(ContentStoreImpl& store, const std::string& pathname)= 0;
	virtual bool createPathOfVolume(ContentStoreImpl& store, const std::string& pathOfVolume, const std::string& volname=NULL)= 0;
	virtual bool deletePathOfVolume(ContentStoreImpl& store, const std::string& pathOfVolume) = 0;
	virtual void getStorageSpace(ContentStoreImpl& store, uint32& freeMB, uint32& totalMB, const char* rootPath=NULL) = 0;
	virtual bool validateMainFileName(ContentStoreImpl& store, std::string& fileName, const std::string& contentType) = 0;
	virtual ContentStoreImpl::FileInfos listMainFiles(ContentStoreImpl& store, const char* rootPath=NULL) = 0;
	virtual std::string memberFileNameToContentName(ContentStoreImpl& store, const std::string& memberFilename) = 0;
	virtual uint64 checkResidentialStatus(ContentStoreImpl& store, uint64 flagsToTest, ContentImpl::Ptr pContent, const ::std::string& contentFullName, const ::std::string& mainFilePathname) = 0;
	virtual bool deleteFileByContent(ContentStoreImpl& store, const ContentImpl& content, const ::std::string& mainFilePathname) = 0;
	virtual bool completeRenaming(ContentStoreImpl& store, const ::std::string& mainFilePathname, const ::std::string& newPathname) = 0;
	virtual bool populateAttrsFromFile(ContentStoreImpl& store, ContentImpl& content, const ::std::string& mainFilePathname) = 0;
	virtual TianShanIce::ContentProvision::ProvisionSessionPrx submitProvision(
		ContentStoreImpl& store, 
		ContentImpl& content, 
		const ::std::string& contentName, 
		const ::std::string& sourceUrl, 
		const ::std::string& sourceType, 
		const ::std::string& startTimeUTC, 
		const ::std::string& stopTimeUTC, 
		const int maxTransferBitrate) = 0;

	virtual TianShanIce::ContentProvision::ProvisionSessionPrx bookPassiveProvision(ContentStoreImpl& store, const ContentImpl& content, const ::std::string& contentName,
		::std::string& pushUrl, const ::std::string& sourceType, const ::std::string& startTimeUTC, const ::std::string& stopTimeUTC, const int maxTransferBitrate) = 0;

	virtual std::string getExportURL(ContentStoreImpl& store, ContentImpl& content, const ::TianShanIce::ContentProvision::ProvisionContentKey& contentkey, const ::std::string& transferProtocol, ::Ice::Int transferBitrate, ::Ice::Int& ttl, ::TianShanIce::Properties& params) = 0;
	virtual void cancelProvision(ContentStoreImpl& store, ContentImpl& content, const ::std::string& provisionTaskPrx) = 0;
	virtual void notifyReplicasChanged(ContentStoreImpl& store, const ::TianShanIce::Replicas& replicasOld, const ::TianShanIce::Replicas& replicasNew) = 0;
};

}} // namespace

#endif//_IBASE_C_S_H