
#ifndef _ZQ_StreamSmith_Embed_ContentStore_Portal_VSTRM_IMPLEMENT_HEADER_FILE_H__
#define _ZQ_StreamSmith_Embed_ContentStore_Portal_VSTRM_IMPLEMENT_HEADER_FILE_H__

#include <zq_common_conf.h>
#include <vector>
#include <TianShanIce.h>
#include <TsContentProv.h>
#include "embededContentStore.h"
#include "vstrmContentStore.h"
#include "StreamSmithConfig.h"
#include <TimeUtil.h>
#include <seafileInfo.h>
#include <vsiolib.h>
#include <ContentProvisionWrapper.h>
#include <vsiolib.h>
#include "IndexFileParser.h"


#define CONTENT_TYPE_NORMAL				0
#define CONTENT_TYPE_LEAD_SESS			1
#define CONTENT_TYPE_VIRTUAL_SESS		2

#define LEAD_SESSION_DELETE_MARK()		SYS_PROP(LeadSessionDeleteMark)

namespace ZQTianShan 
{
namespace ContentStore 
{

struct PortalCtx
{
public:
	PortalCtx(ZQ::common::NativeThreadPool& p);
	~PortalCtx( );
	
	void	onVolumeMounted( const std::string& volumeName , const std::string& path );	

public:
	ZQ::common::NativeThreadPool&			pool;
	ZQ::common::Log*						mpLogger;
	bool									bSupportNpVr;
	VHANDLE									vstrmHandle;
	ContentProvisionWrapper::Ptr			cpWrapper;
	
	ZQ::IdxParser::IdxParserEnv*			idxEnv;

	struct VolumeInfo 
	{
		std::string			volumeName;
		std::string			path;
	};
	typedef std::vector<VolumeInfo>			VolumeInfoS;
	VolumeInfoS								mVolumes;
	ZQ::common::Mutex						mVolumeInfoMutex;
};


class vstrmCSPortalI
{
public:
	vstrmCSPortalI( void* p , ContentStoreImpl& store , ContentImpl::Ptr content = NULL );
	virtual ~vstrmCSPortalI( );
public:

	std::string				fixupPathname( const std::string& pathname ) const;

	bool					createPathOfVolume( const std::string& pathOfVolume ,const std::string& volume);

	bool					deletePathOfVolume( const std::string& pathOfVolume );

	void					getStorageSpace( uint32& freeMB, uint32& totalMB, const char* rootPath=NULL );

	bool					validateMainFileName( std::string& fileName, const std::string& contentType );

	ContentStoreImpl::FileInfos		listMainFiles( const char* rootPath=NULL );

	std::string				memberFileNameToContentName( const std::string& memberFilename );

	uint64					checkResidentialStatus( uint64 flagsToTest,
													ContentImpl::Ptr pContent,
													const ::std::string& contentFullName,
													const ::std::string& mainFilePathname);

	bool					deleteFileByContent( const ContentImpl& content, const ::std::string& mainFilePathname );

	
	bool					completeRenaming( const ::std::string& mainFilePathname, 
												const ::std::string& newPathname );

	bool					populateAttrsFromFile( ContentImpl& content, const ::std::string& mainFilePathname);
	
	
	// Events of a content record has just been created, deleted or state changed
	void					OnContentCreated(const ::Ice::Identity& identContent) {}
	
	void					OnContentDestroyed(const ::Ice::Identity& identContent) {}
	
	void					OnContentStateChanged(const ::Ice::Identity& identVolume, 
													const ::TianShanIce::Storage::ContentState previousState, 
													const ::TianShanIce::Storage::ContentState newState) {}

	// Events of a volume has just been created, mounted
	void					OnSubVolumeCreated(const ::Ice::Identity& identVolume) {}
	void					OnVolumeMounted(const ::Ice::Identity& identVolume, const ::std::string& path);

	// Events of a content's provision has just been started, stopped, in-progress, or becomes streamable
	void					OnContentProvisionStarted(const ::Ice::Identity& identContent) {}	
	void					OnContentProvisionStopped(const ::Ice::Identity& identContent) {}
	void					OnContentProvisionProgress(const ::Ice::Identity& identContent, ::Ice::Long processed, ::Ice::Long total) {}
	void					OnContentProvisionStreamable(const ::Ice::Identity& identContent) {}

	//detect item 
	bool					ignorableItem( const std::string& itemName ) const;


public:
	inline	 bool			isNormalSessionFile( const std::string& fileName ) const
	{
		return getFileAsContentType(fileName) == CONTENT_TYPE_NORMAL ;
	}
	inline   bool			isLeadSessionFile( const std::string& fileName ) const
	{
		return getFileAsContentType(fileName) == CONTENT_TYPE_LEAD_SESS ;
	}
	inline   bool			isVirtualSessionFile( const std::string& fileName ) const
	{
		return getFileAsContentType(fileName) == CONTENT_TYPE_VIRTUAL_SESS ;
	}

	bool					isSubFile( const std::string& fileName ) const;

	bool					checkFileExistence( const std::string& filePath );	

	bool					deleteVstrmFile( const std::string& filePathName );

	///find if the file exist and return the files
	bool					findFiles( const std::string& searchPattern , std::vector<std::string>& files );

	bool					checkFileCharacters( const std::string& file , int32* readerCount , int32* writerCount, int64* fileSize );

protected:

	bool					addMemberFileToFileInfos( const std::string& memberFileName ,
														ContentStoreImpl::FileInfos& infos ,
														SYSTEMTIME& st);
	
	int						getFileAsContentType( const std::string& fileName ) const;
	

	

	std::string				getLeadSessionContentName( const std::string& virtualSessionContent );

	std::string				scanLeadSessionExtFilePathName( const std::string& virtualSessionContent );	

	
private:

	typedef std::vector<std::string>	STRINGSET;

	int						compareSystemTime( SYSTEMTIME& s1 , SYSTEMTIME& s2 );

	bool					getMetaData( IN const ContentImpl& content, IN const std::string& key , OUT std::string& value );
	bool					getMetaData( IN const TianShanIce::Properties& metaData, IN const std::string& key , OUT std::string& value );

	void					setMetaData( IN ContentImpl& content , IN const std::string& key , const int32& value );
	void					setMetaData( IN ContentImpl& content , IN const std::string& key , const uint32& value );
	void					setMetaData( IN ContentImpl& content , IN const std::string& key , const int64& value );
	void					setMetaData( IN ContentImpl& content , IN const std::string& key , const uint64& value );
	void					setMetaData( IN ContentImpl& content , IN const std::string& key , const std::string& value );

	bool					getFileSet( ContentImpl::Ptr pContent, const std::string& mainFilePathName , STRINGSET& fileSet ,bool* bAllImportant = NULL );
	bool					getFileSet( const ContentImpl* content, const std::string& mainFilePathName , STRINGSET& fileSet ,bool* bAllImportant = NULL );

	
	void					getFileSetFromMetaData( const TianShanIce::Properties& metaData  , STRINGSET& fileSet, bool* bAllImportant = NULL);

	//get real file set include main file , index file in the list
	void					getRealFileSetFromMetaData( const TianShanIce::Properties& metaData , STRINGSET& fileset );

	void					clearFileSetInMetaData( ContentImpl& content );

	void					getFileSetFromFS( const std::string& mainFilePathName  , const std::string& indexFilePath ,  bool bNpvrCopy  , STRINGSET& fileSet , bool* bAllImportant = NULL);

	

	const char*				getLastVstrmError( );

	std::string				showCheckResidentialFlag( uint64 flag );

	std::string				showStringSet( const STRINGSET& strSet );
	
	friend class DeleteLaterProcudure;
	

private:

	VHANDLE									mVstrmHandle;
	ZQ::common::Log&						mLogger;
	ContentStoreImpl&						mStore;
	ZQ::IdxParser::IdxParserEnv*			mIdxParserEnv;
	char									mErrorBuffer[1024];
	PortalCtx*								mPortalCtx;
	
	//if no ContentImpl is available
	//just let it as NULL
	//or else 
	ContentImpl::Ptr						mContentPtr;

	bool									mbEdgeServer;
};

std::string helperFixupPathname( const std::string& pathName , bool bSupportNpvr );

}}//ZQTianShan::ContentStore
#endif//_ZQ_StreamSmith_Embed_ContentStore_Portal_VSTRM_IMPLEMENT_HEADER_FILE_H__
