#ifndef _NATIVE_C_S_H
#define _NATIVE_C_S_H
//_NATIVE_C_S_H.h

#include "../../../Common/FileSystemOp.h"
#include "ContentImpl.h"
#include "Guid.h"
#include "Log.h"
#include "CombString.h"
#include "TimeUtil.h"
#include "DispatchCSInterface.h"


namespace ZQTianShan {
namespace ContentStore {


struct __tagCtx;
typedef struct __tagCtx Context;

class NativeCS : public IDispatchCS
{
public:
	NativeCS();
	virtual ~NativeCS();

	/// Portal entry, called during ContentStore initialization, for the portal implementation to initialize.
	/// If necessary, the portal should initialize the portal context _ctxPortal
	///@param[in] store reference to the ContentStore
	virtual void initializePortal(ContentStoreImpl& store);

	/// Portal entry, called during ContentStore uninitialization, for the portal implementation to cleanup its context
	/// if necessary.
	///@param[in] store reference to the ContentStore
	virtual void uninitializePortal(ContentStoreImpl& store);

	/// Portal entry for the portal implementation to fixup the path name of a file. For those case-insensitive file system
	/// such as NTFS, the portal implementation should covert the name to the same cases
	///@param[in] store reference to the ContentStore
	///@param[in] the pathname needs to fix up
	virtual std::string fixupPathname(ContentStoreImpl& store, const std::string& pathname);

	/// Portal entry for the portal implementation to create or validate if a path-of-volume is valid
	/// For example on NTFS, if this pathOfVolume refer to a folder, the portal must create the directory and return true if
	/// the directory exists and allow to create files under it
	///@param[in] store reference to the ContentStore
	///@param[in] the pathOfVolume the full pathname of volume to create or validate
	///@param[in] the volname
	///@true if the given pathOfVolume is valid and the portal can create files under pathOfVolume
	virtual bool createPathOfVolume(ContentStoreImpl& store, const std::string& pathOfVolume, const std::string& volname=NULL);

	/// Portal entry for the portal implementation to delete a path-of-volume
	/// For example on NTFS, if this pathOfVolume refer to a folder, the portal must unlink the directory and any files under it before
	/// returning true
	///@param[in] store reference to the ContentStore
	///@param[in] the pathOfVolume the full pathname of volume to delete
	///@true if the given pathOfVolume has been cleaned up and deleted
	virtual bool deletePathOfVolume(ContentStoreImpl& store, const std::string& pathOfVolume);

	/// Portal entry for the portal implementation to return the storage size.
	///@param[in] store reference to the ContentStore
	///@param[out] freeMB the free space left on the storage
	///@param[out] totalMB the total space of the storage
	///@param[in] rootPath the root path of the (virtual) volume, the portal should test the spaces from the given path
	virtual void getStorageSpace(ContentStoreImpl& store, uint32& freeMB, uint32& totalMB, const char* rootPath=NULL);

	/// Portal entry for the portal implementation to validate the name of the main filename that would be used as the
	/// content name.
	///@param[in] store reference to the ContentStore
	///@param[in] fileName the filename to validate
	///@param[in] contentType the content type for reference; "" if use the default contentType of the ContentStore
	///@return true if the filename is valid
	virtual bool validateMainFileName(ContentStoreImpl& store, std::string& fileName, const std::string& contentType);

	/// Portal entry for the portal implementation to list all the main files. Non-recursive to the sub directories
	/// For those ContentStore that support supplemental files for a main file, supplemental file names would be excluded in
	/// the result
	///@param[in] store reference to the ContentStore
	///@param[in] rootPath the root path to list
	///@return a FileInfo collection of main file, the filename should NOT includes the part of rootPath
	///@note the portal MUST throw exceptions if it is not ready to respond this query at the moment
	virtual ContentStoreImpl::FileInfos listMainFiles(ContentStoreImpl& store, const char* rootPath=NULL);
	//			throw (::TianShanIce::InvalidParameter, ::TianShanIce::InvalidStateOfArt);

	/// convert the member file name to the content name
	///@param[in] store reference to the ContentStore
	///@param[in] memberFilename the short name of member file of a content, the file name has been cut off the volume relatvie
	///           pathname
	virtual std::string memberFileNameToContentName(ContentStoreImpl& store, const std::string& memberFilename);

	/*
	/// Portal entry for the portal implementation to test if the content exists
	/// For those ContentStore that support supplemental files for a main file, this entry also validate the file set of the 
	/// content
	///@param[in] store reference to the ContentStore
	///@param[in] mainFilePathname the full path of main file on the filesystem
	///@return true if the content is resident with neccessary supplemental files
	static bool fileExists(ContentStoreImpl& store, const std::string& mainFilePathname);
	*/

	/// Portal entry for the portal implementation to test the file residential status of the member files of a content
	///@param[in] store reference to the ContentStore
	///@param[in] content reference to the content object
	///@param[in] mainFilePathname the full path of main file on the filesystem
	///@return the flags of the file residential status
	virtual uint64 checkResidentialStatus(ContentStoreImpl& store, uint64 flagsToTest, ContentImpl::Ptr pContent, const ::std::string& contentFullName, const ::std::string& mainFilePathname);

	/// Portal entry for the portal implementation to delete a content with all its supplemental files
	///@param[in] store reference to the ContentStore
	///@param[in] content reference to the content object
	///@param[in] mainFilePathname the full path of main file on the filesystem
	///@return true if all the disk files of this content has been confirmed not in the storage
	virtual bool deleteFileByContent(ContentStoreImpl& store, const ContentImpl& content, const ::std::string& mainFilePathname);

	/// Portal entry for the portal implementation to complete the renaming procedure
	///@param[in] store reference to the ContentStore
	///@param[in] mainFilePathname the full path name of main file on the filesystem that is about to rename
	///@param[in] newPathname the full path name of the new main file on the filesystem to rename to
	//@return false if renaming is not allowed or has not completed
	virtual bool completeRenaming(ContentStoreImpl& store, const ::std::string& mainFilePathname, const ::std::string& newPathname);

	/// Portal entry for the portal implementation to delete a content with all its supplemental files
	///@param[in] store reference to the ContentStore
	///@param[in, out] content reference to the content object that this entry should fill attributes into
	///           the following are some neccessary attributes that should be updated:
	///				- METADATA_FileSize                 file size in bytes
	///				- METADATA_SupportFileSize          file size subtotal, in bytes, of supplemental files excluding the main file
	///				- METADATA_PixelHorizontal          picture resoultion
	///				- METADATA_PixelVertical            picture resoultion 
	///				- METADATA_BitRate                  the encoded bitrate in bps
	///				- METADATA_PlayTime                 play time in msec
	///				- METADATA_FrameRate                frame rate in fps
	///				- METADATA_LocalType                local format type after save the content to local storage
	///				- METADATA_MD5CheckSum              the MD5 checksum of the main file (optional)
	///@param[in] mainFilePathname the full path of main file on the filesystem
	///@return true if some of the attributes are succesfully populated and updated into content metadata
	virtual bool populateAttrsFromFile(ContentStoreImpl& store, ContentImpl& content, const ::std::string& mainFilePathname);

	/// Portal entry for the portal implementation to setup and commit the active provision task
	///@param[in] store reference to the ContentStore
	///@param[in] content reference to the content object that this entry should fill attributes into
	///@param[in] sourceUrl - the source URL to provision this Content with
	///@param[in] sourceContentType - the enum type code to specify the source content type, coversion may happen to 
	///                               meet the type of this destination type
	///@param[in] overwrite         - true if need to overwrite a existing content
	///@param[in] startTimeUTC      - the scheduled start time, values are in UTC format, NULL if need to start immediately
	///@param[in] stopTimeUTC       - the scheduled stop time, values are in UTC format, NULL if no time-based cut off needed
	///@param[in] maxTransferBitrate - the limitation on transfer bitrate in Kbps, the transfering
	///                       should not exceed this given max bitrate, 0 if no limitation
	///@return the proxy to the Provision session
	virtual TianShanIce::ContentProvision::ProvisionSessionPrx submitProvision(
		ContentStoreImpl& store, 
		ContentImpl& content, 
		const ::std::string& contentName, 
		const ::std::string& sourceUrl, 
		const ::std::string& sourceType, 
		const ::std::string& startTimeUTC, 
		const ::std::string& stopTimeUTC, 
		const int maxTransferBitrate);
				//throw (::TianShanIce::InvalidParameter, TianShanIce::Storage::NoResourceException, 
	   //                ::TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt);

	/// Portal entry for the portal implementation to book a passive provision task
	///@param[in] store reference to the ContentStore
	///@param[in] content reference to the content object that this entry should fill attributes into
	///@param[out] pushUrl the URL to where the client can import content
	///@param[in] sourceContentType - the enum type code to specify the source content type, coversion may happen to 
	///                               meet the type of this destination type
	///@param[in] overwrite         - true if need to overwrite a existing content
	///@param[in] startTimeUTC      - the scheduled start time, values are in UTC format, NULL if need to start immediately
	///@param[in] stopTimeUTC       - the scheduled stop time, values are in UTC format, NULL if no time-based cut off needed
	///@param[in] maxTransferBitrate - the limitation on transfer bitrate in Kbps, the transfering
	///                       should not exceed this given max bitrate, 0 if no limitation
	///@return the proxy to the Provision session
	virtual TianShanIce::ContentProvision::ProvisionSessionPrx bookPassiveProvision(ContentStoreImpl& store, const ContentImpl& content, const ::std::string& contentName,
		::std::string& pushUrl, const ::std::string& sourceType, const ::std::string& startTimeUTC, const ::std::string& stopTimeUTC, const int maxTransferBitrate);
	//			throw (::TianShanIce::InvalidParameter, ::TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt);

	/// Portal entry for the portal implementation to expose the URL where the content can be export from
	///@param[in] store reference to the ContentStore
	///@param[in] content reference to the content object that this entry should fill attributes into
	///@param[in] targetCSType     the target contentstore after exporting
	///@param[in] transferProtocol the expected transfer protocol to export the content, if ContentStore does not support the
	///                              expected transfer protocol, InvalidStateOfArt exception will throw. 
	///@param[out] params		     the addition information to the export content, such as protocol, file count, file list, ttl, etc
	///@return the URL where the user can download the content from the storage
	virtual std::string getExportURL(ContentStoreImpl& store, ContentImpl& content, const ::TianShanIce::ContentProvision::ProvisionContentKey& contentkey, const ::std::string& transferProtocol, ::Ice::Int transferBitrate, ::Ice::Int& ttl, ::TianShanIce::Properties& params);

	// Portal entry for the portal implementation to cancel a on-going provision task attached on the given content
	//@param[in] store reference to the ContentStore
	//@param[in] content reference to the content object that this entry should fill attributes into
	//@param[in] provisionTaskPrx the provision task bound on the content
	//@note the entry implementation must return only if the canceling is confirmed, otherwise throw exceptions
	//@throw ServerError - if the canceling failed
	//@throw InvalidStateOfArt - if the canceling is not allowed in this state
	virtual void cancelProvision(ContentStoreImpl& store, ContentImpl& content, const ::std::string& provisionTaskPrx);
	//		throw (::TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt);

	virtual void notifyReplicasChanged(ContentStoreImpl& store, const ::TianShanIce::Replicas& replicasOld, const ::TianShanIce::Replicas& replicasNew);

private:
	Context*      _ctx;
	std::string   _urlExpr;
	bool          _isUrlGenActive;
};

}} // namespace

#endif//NativeCS.h