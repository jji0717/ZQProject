
#include <assert.h>
#include "vstrmCsPortal.h"
#include <algorithm>
#include <cctype>
#include <TianShanIceHelper.h>

#define MLOG mLogger

#ifdef ZQ_OS_MSWIN
#define		PORTALFMT(x,y) 	"[vstrmCSPortalI]TID[%06X][%12s]\t"##y,GetCurrentThreadId(),#x
#elif defined ZQ_OS_LINUX
#define		PORTALFMT(x,y) 	"[vstrmCSPortalI]TID[%06X][%12s]\t"##y,pthread_self(),#x
#else
#error "NOT SUPPORTED OS"
#endif


#define PLOG (*mpLogger)

namespace ZQTianShan 
{
namespace ContentStore 
{

//////////////////////////////////////////////////////////////////////////
PortalCtx::	PortalCtx( ZQ::common::NativeThreadPool& p )
:pool(p)
{
	VstrmClassOpenEx(&vstrmHandle );
}
PortalCtx::~PortalCtx( )
{
	if( vstrmHandle )
	{
		VstrmClassCloseEx(vstrmHandle);
		vstrmHandle = NULL;
	}
}

void PortalCtx::onVolumeMounted( const std::string& volumeName , const std::string& path )
{
	ZQ::common::MutexGuard gd(mVolumeInfoMutex);
	VolumeInfoS::const_iterator it = mVolumes.begin();
	for( ; it != mVolumes.end() ; it ++ )
	{
		if ( it->volumeName == volumeName )
		{
			PLOG(ZQ::common::Log::L_ERROR,
				CLOGFMT(PortalCtx,"onVolumeMounted() duplicated volume [%s] mounted"),
				volumeName.c_str() );
			return;
		}
	}
	VolumeInfo info;
	info.path				=	path;
	info.volumeName			=	volumeName;
	mVolumes.push_back( info );
}



//////////////////////////////////////////////////////////////////////////
//
#define METADATA_LEADSESSION()	SYS_PROP(nPvRLeadSessContent)

#define METADATA_SERVERMODE()	SYS_PROP(ServerMode)

#define METADATA_INDEXPARSEDOK( ) SYS_PROP(IndexDataParsedOk)

#define SERVERMODE_EDGESERVER	"EdgeServer"

#define STRINDEXPARSEDOK		"Yes"

//	 METADATA_nPVRLeadCopy
void vstrmCSPortalI::OnVolumeMounted(const ::Ice::Identity& identVolume, const ::std::string& path)
{
	assert(mPortalCtx != NULL );
	mPortalCtx->onVolumeMounted(identVolume.name , path);
}

vstrmCSPortalI::vstrmCSPortalI( void* p , ContentStoreImpl& store , ContentImpl::Ptr content )
:mStore(store),
mLogger(store._log)
{
	if( p )
	{
		PortalCtx* pCtx = reinterpret_cast<PortalCtx*>(p);
		assert( pCtx != NULL );
		mVstrmHandle		= pCtx->vstrmHandle;
		mIdxParserEnv		= pCtx->idxEnv;	
		mPortalCtx			= pCtx;		
	}
	mContentPtr			= content;
	mbEdgeServer		= ( gStreamSmithConfig.serverMode == 2 );
}
vstrmCSPortalI::~vstrmCSPortalI()
{
}

int vstrmCSPortalI::getFileAsContentType( const std::string& fileName ) const
{
	assert( mPortalCtx != NULL );
	if ( mPortalCtx->bSupportNpVr )
	{
		if ( std::string::npos == fileName.find('_') )
		{
			return CONTENT_TYPE_LEAD_SESS;
		}
		else
		{
			return CONTENT_TYPE_VIRTUAL_SESS;
		}
	}
	else
	{
		return CONTENT_TYPE_NORMAL;
	}
}

std::string vstrmCSPortalI::getLeadSessionContentName( const std::string& virtualSessionContent )
{
	if( !isVirtualSessionFile(virtualSessionContent ) )
	{
		return "";
	}
	else
	{

		std::string::size_type pos ;		
		pos = virtualSessionContent.find_last_of("\\/");
		std::string result;
		if ( pos != std::string::npos )
		{
			result =  virtualSessionContent.substr(pos + 1 );
		}
		else
		{
			result = virtualSessionContent;
		}
		pos = result.find('_');
		if( pos != std::string::npos )
		{
			return result.substr( 0 , pos );
		}
		else
		{
			return "";
		}
	}
}



std::string vstrmCSPortalI::scanLeadSessionExtFilePathName( const std::string& virtualSessionContent )
{
	assert( mPortalCtx != NULL );
	std::string	leadSessName = getLeadSessionContentName( virtualSessionContent );
	if( leadSessName.empty() )
		return "";
	PortalCtx::VolumeInfoS volumes;
	{		
		ZQ::common::MutexGuard gd(mPortalCtx->mVolumeInfoMutex);
		volumes	=	mPortalCtx->mVolumes;		
	}
	PortalCtx::VolumeInfoS::const_iterator it = volumes.begin();
	for( ; it != volumes.end() ; it ++ )
	{
		
		std::string extFileVVX = it->path + leadSessName + ".VVX";
		std::string extFileVV2 = it->path + leadSessName + ".VV2";
		std::string extFileIndex = it->path + leadSessName + ".INDEX";
		if( checkFileExistence(extFileVVX) )
		{
			return extFileVVX;
		}
		else if ( checkFileExistence(extFileVV2) )
		{
			return extFileVV2;
		}
		else if( checkFileExistence(extFileIndex))
		{
			return extFileIndex;
		}
	}
	return "";
}

const char* vstrmCSPortalI::getLastVstrmError( )
{
	memset( mErrorBuffer , 0 , sizeof(mErrorBuffer) );
	VstrmClassGetErrorText(mVstrmHandle , VstrmGetLastError() , mErrorBuffer, sizeof(mErrorBuffer )-1 );
	return mErrorBuffer;
}

std::string	vstrmCSPortalI::fixupPathname( const std::string& pathname ) const
{
	return mStore.fixupPathname( mStore , pathname );
}

bool vstrmCSPortalI::createPathOfVolume( const std::string& pathOfVolume )
{
#pragma message(__MSGLOC__"how to implement it ?")
	return true;
}

bool vstrmCSPortalI::deletePathOfVolume( const std::string& pathOfVolume )
{
#pragma message(__MSGLOC__"how to implement it ?")
	return true;
}

void vstrmCSPortalI::getStorageSpace( uint32& freeMB, uint32& totalMB, const char* rootPath/*=NULL */)
{
	LARGE_INTEGER	freeSpace;
	LARGE_INTEGER	totalSpace;
	freeMB = totalMB = 0;
	if( rootPath && rootPath[0] != NULL )
	{
		ZQ::StreamSmith::SfuInformation  seafileInformation(mLogger);
		ZQ::StreamSmith::SfuInformation ::sfVolumeInfos sfInofs;
		if(!seafileInformation.retrieveVolumeInformation( sfInofs , rootPath ) && sfInofs.size() == 0 )
		{
			MLOG(ZQ::common::Log::L_WARNING,
				PORTALFMT(getStorageSpace,"can't get volume information with rootPath[%s]"),
				rootPath );
			return;
		}
		ZQ::StreamSmith::SfuInformation ::sfVolumeInfos::const_iterator itVolInfo = sfInofs.begin();
		freeSpace.QuadPart		= static_cast<LONGLONG>(itVolInfo->volumeBytesFree);
		totalSpace.QuadPart		= static_cast<LONGLONG>(itVolInfo->volumeSize);
	}
	else
	{
		if( VSTRM_SUCCESS != ( VstrmClassGetStorageData( mVstrmHandle ,&freeSpace , &totalSpace ) ) )
		{			
			MLOG(ZQ::common::Log::L_ERROR ,
				PORTALFMT(getStorageSpace,"can't get Storage space with error:%s"),
				getLastVstrmError()	);
			return;
		}		
	}
	freeMB	=	static_cast<uint32>( freeSpace.QuadPart / ( 1024 * 1024 ) );
	totalMB	=	static_cast<uint32>( totalSpace.QuadPart / ( 1024 * 1024 ) );
}

bool vstrmCSPortalI::validateMainFileName( std::string& contentName, const std::string& contentType )
{
	std::string name = fixupPathname(contentName);
	int bpos = static_cast<int>(name.find_last_of("\\/"));
	int epos = static_cast<int>(name.find_last_of("."));

	if (epos < bpos)
	{
		contentName = name.substr(bpos +1);
		return true;
	}

	std::string extfn1 = name.substr(epos+1);
	std::transform(extfn1.begin(), extfn1.end(), extfn1.begin(), (int(*)(int)) std::toupper);
	if( extfn1 =="INDEX")
	{
		contentName = name.substr(bpos+1, epos - bpos - 1);
	}
	else
	{
		std::string extfn = name.substr(epos+1, 2);
		std::transform(extfn.begin(), extfn.end(), extfn.begin(), (int(*)(int)) std::toupper);	
		if (0 == extfn.compare("VV") ||
			0 == extfn.compare("FF") || 
			0 == extfn.compare("FR"))
		{
			contentName = name.substr(bpos+1, epos - bpos - 1);
		}
		else 
			contentName = name.substr(bpos +1);
	}

	return true;
}

bool vstrmCSPortalI::ignorableItem( const std::string& itemName ) const
{	
	if( !mbEdgeServer )	
	{//all items should be monitored if is not in EdgeServer mode
		return false;
	}
	else
	{//in EdgeServer mode
		std::string::size_type pos = itemName.find_last_of('.');
		if( pos != std::string::npos )
		{
			std::string extName = itemName.substr( pos + 1);
			std::transform(extName.begin(), extName.end(), extName.begin(), (int(*)(int)) std::toupper);
			if( extName.find("0X") == 0 )
			{//can be ignore
				return true;
			}
		}
		return false;
	}
}

bool vstrmCSPortalI::isSubFile( const std::string& fileName ) const
{
	std::string name = fixupPathname(fileName);
	int bpos = static_cast<int>(name.find_last_of("\\/"));
	int epos = static_cast<int>(name.find_last_of("."));

	if (epos < bpos)
	{		
		return false;
	}
	std::string extfn1 = name.substr(epos+1);
	std::transform(extfn1.begin(), extfn1.end(), extfn1.begin(), (int(*)(int)) std::toupper);
	if( extfn1 =="INDEX")
	{
		return true;
	}
	else
	{
		std::string extfn = name.substr(epos+1, 2);
		std::transform(extfn.begin(), extfn.end(), extfn.begin(), (int(*)(int)) std::toupper);

		if (0 == extfn.compare("VV") ||
			0 == extfn.compare("FF") || 
			0 == extfn.compare("FR"))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

bool vstrmCSPortalI::findFiles( const std::string& searchPattern , std::vector<std::string>& files )
{	
	static size_t iPrefixLength				= gStreamSmithConfig.embededContenStoreCfg.ctntAttr.ignorePrefix.length();
	static const std::string& strNamePrefix		= gStreamSmithConfig.embededContenStoreCfg.ctntAttr.ignorePrefix;
	static const std::string& strNameInvalidChar	= gStreamSmithConfig.embededContenStoreCfg.ctntAttr.ignoreInvalidCharacter;

	DLL_FIND_DATA_EXTENDED	findData;
	VSTRM_FIND_FILE_FLAGS	findFlag;
	findFlag._rsvd					= 0;
	findFlag.ReturnHiddenFiles		= false;
	
	VHANDLE findHandle = VstrmFindFirstFileEx3( mVstrmHandle , searchPattern.c_str()  , findFlag , &findData );

	if ( findHandle == INVALID_HANDLE_VALUE )
	{
		return false;
	}
	do
	{
		std::string fileName ;

		const char* pDelimiter = NULL;
		if( !strNameInvalidChar.empty() && strstr( findData.w.cFileName, strNameInvalidChar.c_str() ) != NULL )
		{
			continue;
		}
		if( !strNamePrefix.empty() )
		{
			if( (pDelimiter = strstr( findData.w.cFileName,strNamePrefix.c_str() )) != NULL  )
			{
				pDelimiter = pDelimiter + iPrefixLength;
			}
			else
			{
				pDelimiter = findData.w.cFileName;
			}
		}
		else
		{
			pDelimiter = findData.w.cFileName;
		}		
		if( pDelimiter )
		{
			fileName = pDelimiter;
			if(  strstr( fileName.c_str() , "*" ) == NULL && !fileName.empty() )
			{
				files.push_back(fileName);
			}
		}		
	}while( VstrmFindNextFileEx3( mVstrmHandle , findHandle , &findData ) );

	VstrmFindClose(  mVstrmHandle ,findHandle );
	return files.size() > 0;
}

ContentStoreImpl::FileInfos vstrmCSPortalI::listMainFiles( const char* rootPath/*=NULL*/ )
{
	static size_t iPrefixLength				= gStreamSmithConfig.embededContenStoreCfg.ctntAttr.ignorePrefix.length();
	static const std::string& strNamePrefix		= gStreamSmithConfig.embededContenStoreCfg.ctntAttr.ignorePrefix;
	static const std::string& strNameInvalidChar	= gStreamSmithConfig.embededContenStoreCfg.ctntAttr.ignoreInvalidCharacter;

	ContentStoreImpl::FileInfos outValues;
	outValues.clear();
	
	DLL_FIND_DATA_EXTENDED	findData;
	VSTRM_FIND_FILE_FLAGS	findFlag;
	findFlag._rsvd					= 0;
	findFlag.ReturnHiddenFiles		= false;

	std::string		strfileName  = "";
	bool bHasRootPath = false;
	size_t rtPathLen = 0;

	if( rootPath != NULL && rootPath[0] != NULL )
	{
		bHasRootPath = true;
		rtPathLen = strlen(rootPath);
		if( !strNamePrefix.empty() )
		{
			if( strNamePrefix.at(strNamePrefix.length() -1) != '\\' )
			{
				strfileName = strNamePrefix + "\\" + rootPath + "*";
			}
			else
			{
				strfileName = strNamePrefix + rootPath + "*";
			}
		}
		else
		{
			strfileName = std::string("\\") + rootPath + "*";
		}

	}
	else
	{
		strfileName = "*";
	}

	VHANDLE findHandle = VstrmFindFirstFileEx3( mVstrmHandle , strfileName.c_str()  , findFlag , &findData );

	if ( findHandle == INVALID_HANDLE_VALUE )
	{		
		MLOG(ZQ::common::Log::L_ERROR,
			PORTALFMT(listMainFiles,"failed to execute VstrmFindFirstFileEx3:%s"),
			getLastVstrmError());
		return outValues;
	}

	do
	{
		if ( findData.VstrmFileVersion.StructureLength > FIELD_OFFSET( VSTRM_FILE_VERSION, Modulus ) )
		{
			//if (findData.VstrmFileVersion.RAIDLevel == 5)
			{				
				std::string fileName ;

				const char* pDelimiter = NULL;
				if( !strNameInvalidChar.empty() && strstr( findData.w.cFileName, strNameInvalidChar.c_str() ) != NULL )
				{
					continue;
				}
				if( !strNamePrefix.empty() )
				{
					if( (pDelimiter = strstr( findData.w.cFileName,strNamePrefix.c_str() )) != NULL  )
					{
						pDelimiter = pDelimiter + iPrefixLength;
					}
					else
					{
						pDelimiter = findData.w.cFileName;
					}
				}
				else
				{
					pDelimiter = findData.w.cFileName;
				}
				if( bHasRootPath )
				{//erase root path
					const char * pTemp = strstr( pDelimiter , rootPath );
					if (  pTemp != NULL && pTemp == pDelimiter )
					{
						pDelimiter += strlen( rootPath );
					}
				}
				if( pDelimiter )
				{
					fileName = pDelimiter;
					if(  strstr( fileName.c_str() , "*" ) == NULL && !fileName.empty() )
					{						
						//outValues.push_back( fileName );
						SYSTEMTIME st;
						FileTimeToSystemTime(&findData.w.ftLastWriteTime , &st);						
						addMemberFileToFileInfos(fileName , outValues , st );
					}
				}
			}
		}
	}while( VstrmFindNextFileEx3( mVstrmHandle , findHandle , &findData ) );

	VstrmFindClose(  mVstrmHandle ,findHandle );
	return outValues;
}

bool vstrmCSPortalI::addMemberFileToFileInfos( const std::string& memberFileName , 
											  ContentStoreImpl::FileInfos& infos ,
											  SYSTEMTIME& st)
{	
	std::string fileName = memberFileNameToContentName(memberFileName);
	ContentStoreImpl::FileInfos::iterator it = infos.begin();
	for( ; it != infos.end() ; it ++ )
	{		
		if( it->filename == fileName )
		{
			SYSTEMTIME lastWrite;
			ZQ::common::TimeUtil::Iso2Time(it->stampLastWrite.c_str() ,lastWrite );			
			if( compareSystemTime(lastWrite,st) == 0 )
				return true;
			else
				break;
		}
	}

	char szTimeBuffer[256];
	szTimeBuffer[sizeof(szTimeBuffer)-1] = 0;
	ZQ::common::TimeUtil::Time2Iso( st , szTimeBuffer , sizeof(szTimeBuffer)-1 ,false );	

	if( it != infos.end() )
	{
		it->stampLastWrite.assign(szTimeBuffer);
	}
	else
	{
		ContentStoreImpl::FileInfo info;
		info.filename				= fileName;
		info.stampLastWrite			= szTimeBuffer;
		infos.push_back( info );
	}

	return true;
}

std::string vstrmCSPortalI::memberFileNameToContentName( const std::string& memberFilename )
{
	std::string contentName = fixupPathname( memberFilename);
	//remove the "/\"
	{
		std::string::size_type bpos = contentName.find_last_of("\\/");
		if (bpos!=std::string::npos)
			contentName = contentName.substr(bpos +1);
	}

	//process the "."
	std::string::size_type epos = contentName.find_last_of(".");
	if (epos==std::string::npos)
		return contentName;

	std::string extfn1 = contentName.substr(epos+1);
	std::transform(extfn1.begin(), extfn1.end(), extfn1.begin(), (int(*)(int)) std::toupper);
	if( mbEdgeServer && extfn1.find("0X") == 0 )
	{
		return contentName.substr( 0 , epos );
	}
	else if( extfn1 =="INDEX")
	{
		return contentName.substr( 0 , epos );
	}

	std::string extfn = contentName.substr( epos+1 , 2 );

	std::transform(extfn.begin(), extfn.end(), extfn.begin(), (int(*)(int)) std::toupper);

	if (0 == extfn.compare("VV") ||
		0 == extfn.compare("FF") || 
		0 == extfn.compare("FR") )		
	{
		return contentName.substr( 0 , epos );
	}
	else
	{
		return contentName;
	}

// 	std::vector<std::string>& subFileExtNames = GAPPLICATIONCONFIGURATION.embededContenStoreCfg.subFileExtNameSet.subFileExtNames;
// 	std::vector<std::string>::const_iterator itFileExt = subFileExtNames.begin( );
// 	for( ; itFileExt != subFileExtNames.end() ; itFileExt ++ )
// 	{
// 		if( stricmp( itFileExt->c_str() , extfn.c_str() ) == 0 )
// 		{
// 			return contentName.substr(0, epos);
// 		}
// 	}
//	return contentName;
}

int	vstrmCSPortalI::compareSystemTime( SYSTEMTIME& s1 , SYSTEMTIME& s2 )
{
	time_t t1 = ZQ::common::TimeUtil::Systime2Time(s1);
	time_t t2 = ZQ::common::TimeUtil::Systime2Time(s2);
	return static_cast<int>(t1 - t2);
}

bool vstrmCSPortalI::getMetaData( IN const TianShanIce::Properties& metaData, IN const std::string& key , OUT std::string& value )
{
	value = "";
	TianShanIce::Properties::const_iterator itMetaData = metaData.find(key);
	if( itMetaData != metaData.end() )
	{
		value = itMetaData->second;
		return true;
	}
	else
	{
		return false;
	}
}
bool vstrmCSPortalI::getMetaData(IN const ContentImpl& content, IN const std::string& key , OUT std::string& value )
{
	return getMetaData(content.metaData,key,value);
}
void vstrmCSPortalI::setMetaData( IN ContentImpl& content , IN const std::string& key , const int32& value )
{
	char szBuf[256];
	szBuf[ sizeof(szBuf) - 1 ] = 0;
	snprintf( szBuf , sizeof(szBuf)-1 , "%d" , value );
	content.metaData[key] = szBuf;
	MLOG(ZQ::common::Log::L_DEBUG,PORTALFMT(setMetaData,"MainFile[%s] set metaData[%s]=[%s]"),
		content.ident.name.c_str(),	key.c_str() , szBuf);
}
void vstrmCSPortalI::setMetaData( IN ContentImpl& content , IN const std::string& key , const uint32& value )
{
	char szBuf[256];
	szBuf[ sizeof(szBuf) - 1 ] = 0;
	snprintf( szBuf , sizeof(szBuf)-1 , "%u" , value );
	content.metaData[key] = szBuf;
	MLOG(ZQ::common::Log::L_DEBUG,PORTALFMT(setMetaData,"MainFile[%s] set metaData[%s]=[%s]"),
		content.ident.name.c_str(),key.c_str() , szBuf);
}

void vstrmCSPortalI::setMetaData( IN ContentImpl& content , IN const std::string& key , const std::string& value )
{	
	content.metaData[key] = value;
	MLOG(ZQ::common::Log::L_DEBUG,PORTALFMT(setMetaData,"MainFile[%s] set metaData[%s]=[%s]"),
		content.ident.name.c_str(), key.c_str() , value.c_str() );
}

void vstrmCSPortalI::setMetaData( IN ContentImpl& content , IN const std::string& key , const int64& value )
{
	char szBuf[256];
	szBuf[ sizeof(szBuf) - 1 ] = 0;
	snprintf( szBuf , sizeof(szBuf)-1 , "%lld" , value );
	content.metaData[key] = szBuf;
	MLOG(ZQ::common::Log::L_DEBUG,PORTALFMT(setMetaData,"MainFile[%s] set metaData[%s]=[%s]"),
		content.ident.name.c_str(),key.c_str() , szBuf);
}

void vstrmCSPortalI::setMetaData( IN ContentImpl& content , IN const std::string& key , const uint64& value )
{
	char szBuf[256];
	szBuf[ sizeof(szBuf) - 1 ] = 0;
	snprintf( szBuf , sizeof(szBuf)-1 , "%llu" , value );
	content.metaData[key] = szBuf;
	MLOG(ZQ::common::Log::L_DEBUG,PORTALFMT(setMetaData,"MainFile[%s] set metaData[%s]=[%s]"),
		content.ident.name.c_str(),key.c_str() , szBuf);
}

void vstrmCSPortalI::clearFileSetInMetaData( ContentImpl& content  )
{
	std::string strFileCount;
	if( getMetaData( content , METADATA_SUBFILECOUNT , strFileCount) )
	{
		int32 iCount = atoi(strFileCount.c_str() );
		char buf[256];
		for( int32 i = 0 ; i< iCount ; i ++ )
		{
			sprintf(buf , "%s%d" , METADATA_SUBFILENAME, i );
			content.metaData.erase(buf);
		}
		content.metaData.erase(METADATA_SUBFILECOUNT );
	}
}

bool vstrmCSPortalI::populateAttrsFromFile( ContentImpl& content, const ::std::string& mainFilePathnameReal)
{
	bool					bnPvRCopy = false;
	std::string				strIndexFile = "";
	std::string				mainFilePathname = mainFilePathnameReal;
	
	if(getMetaData( content , METADATA_nPVRLeadCopy , strIndexFile))
	{
		bnPvRCopy = checkFileExistence(strIndexFile);
	}
	if( !bnPvRCopy && isVirtualSessionFile( content.ident.name ) )
	{
		strIndexFile	=	scanLeadSessionExtFilePathName( content.ident.name );
		if( strIndexFile.empty() )
		{
			MLOG(ZQ::common::Log::L_WARNING,
				PORTALFMT(populateAttrsFromFile,"MainFile[%s] is a virtual session but index file does not exist"),
				mainFilePathname.c_str() );
			return false;
		}
		bnPvRCopy		=	true;
	}
	
	bool bNpvrServer = ( gStreamSmithConfig.serverMode == 1 );//This is a npvr server
	if( bNpvrServer )
	{
// 		std::string::size_type pos = mainFilePathname.find_last_of('_');
// 		if( pos != std::string::npos )
// 		{
// 			mainFilePathname = mainFilePathname.substr( 0 , pos );
// 		}
	}

	MLOG(ZQ::common::Log::L_INFO,
		PORTALFMT(populateAttrsFromFile,"MainFile[%s][%s] populate attr with specified indexFilePath [%s]"),
		mainFilePathname.c_str(),
		mainFilePathnameReal.c_str(),
		strIndexFile.c_str());
	
	ZQ::IdxParser::IndexFileParser	idxParser( *mIdxParserEnv );	
	ZQ::IdxParser::IndexData		idxData;

	Ice::Int bLocalContentForVstrm = 0;
	

	ZQTianShan::Util::getPropertyDataWithDefault( content.metaData , SYS_PROP(ContentInLocalFileSystem) , 0 , bLocalContentForVstrm );
//	ZQTianShan::Util::getPropertyDataWithDefault( content.metaData , SYS_PROP(ContentPropertyParsedOK) , 0 , bParseOkByLoadAll );

	if( !idxParser.ParseIndexFileFromVstrm( mainFilePathname,idxData,bnPvRCopy,strIndexFile ,
											bLocalContentForVstrm != 0,
											bLocalContentForVstrm != 0) )
	{		
		MLOG(ZQ::common::Log::L_ERROR,
			PORTALFMT(populateAttrsFromFile,"MainFile[%s][%s] failed to populate attribute with specified index file[%s]"),
			mainFilePathname.c_str(),
			mainFilePathnameReal.c_str(),
			strIndexFile.c_str());
		
		if( idxParser.getLastErrorCode() == ERROR_CODE_OBJECT_NOT_FOUND )
		{
			MLOG( ZQ::common::Log::L_INFO, PORTALFMT(populateAttrsFromFile,"MainFile[%s][%s], set dirty to [false] because OBJECT_NOT_FOUND"),
				mainFilePathname.c_str(),
				mainFilePathnameReal.c_str() );

			content.bDirtyAttrs = false;
		}
		return false;
	}
	if( bLocalContentForVstrm != 1 )//need record all metadata
	{
		//clear file set in content's metaData
		clearFileSetInMetaData(content);

		//successfully parse the index file
		MLOG(ZQ::common::Log::L_DEBUG,
			PORTALFMT(populateAttrsFromFile,"MainFile[%s] successfully parse the index file"),
			mainFilePathname.c_str());

		if(idxData.getIndexType() == ZQ::IdxParser::IndexData::INDEX_TYPE_VVX )
		{
			setMetaData( content , METADATA_SubType , TianShanIce::Storage::subctVVX );
		}
		else if( idxData.getIndexType() == ZQ::IdxParser::IndexData::INDEX_TYPE_VV2 )
		{
			setMetaData( content , METADATA_SubType , TianShanIce::Storage::subctVV2 );
		}
		else if(idxData.getIndexType() == ZQ::IdxParser::IndexData::INDEX_TYPE_VVC)
		{
			setMetaData( content , METADATA_SubType , "VVC" );
		}
		else
		{
			MLOG(ZQ::common::Log::L_ERROR,
				PORTALFMT(populateAttrsFromFile,"MainFile[%s] unknown index format"),
				mainFilePathname.c_str() );
			return false;
		}

		setMetaData( content , METADATA_BitRate , idxData.getMuxBitrate() );

		{
			if( mbEdgeServer )
				setMetaData( content , METADATA_SERVERMODE() , SERVERMODE_EDGESERVER);
		}
		setMetaData( content , METADATA_FILENAME_MAIN , idxData.getMainFilePathName() );

		setMetaData( content ,METADATA_INDEXPARSEDOK() , std::string( STRINDEXPARSEDOK ) );

		if( bnPvRCopy  )
		{
			int64 lMainFileSize = idxData.getMainFileSize();
			int32 lMuxRate = idxData.getMuxBitrate();
			if( lMuxRate != 0 )
			{
				setMetaData( content , METADATA_PlayTime , lMainFileSize * 8000 / lMuxRate );
			}
			else
			{
				MLOG(ZQ::common::Log::L_DEBUG,
					PORTALFMT(populateAttrsFromFile,"MainFile[%s] [npvrCopy ]invalid muxRate[%d] can't get playTime"),
					mainFilePathname.c_str(), lMuxRate	);
			}
		}
		else
		{
			setMetaData( content , METADATA_PlayTime , idxData.getPlayTime() );
		}

		setMetaData( content , METADATA_FileSize , idxData.getMainFileSize() );

		char	buf[256];

		int iSubFileIndex = 0;


		int32 iSubFileCount = idxData.getSubFileCount();
		for(int i =0 ;  i< iSubFileCount ;i ++ )
		{

			const std::string& strExtension =  idxData.getSubFileName( i ) ;
			if( !strExtension.empty() )
			{
				std::string	subFileName = mainFilePathnameReal + strExtension;				
				sprintf(buf , "%s%d" , METADATA_SUBFILENAME, iSubFileIndex++ );
				setMetaData( content , buf , subFileName );
			}
			else
			{				
				sprintf(buf , "%s%d" , METADATA_SUBFILENAME, iSubFileIndex++ );
				setMetaData( content , buf , mainFilePathnameReal );				
			}
		}

		if(!bnPvRCopy)
		{	
			const std::string& tmpIndexFileName = idxData.getIndexFilePathName();
			sprintf(buf , "%s%d" , METADATA_SUBFILENAME, iSubFileIndex++ );
			setMetaData( content , buf , tmpIndexFileName );
		}
		else
		{
			setMetaData( content, METADATA_nPVRLeadCopy , strIndexFile );
		}

		if( iSubFileIndex > 0 )
		{
			sprintf( buf ,"%d" , iSubFileIndex );
			setMetaData( content , METADATA_SUBFILECOUNT , buf );
		}
	}
	else
	{
		setMetaData( content , METADATA_PlayTime , idxData.getPlayTime() );
		int64	lFileSize = idxData.getMainFileSize();
		if( lFileSize <= 0 )
		{
			std::string mainFileName ;
			getMetaData( content , METADATA_FILENAME_MAIN , mainFileName );
			if( !mainFileName.empty() )
			{
				checkFileCharacters( mainFileName ,0 ,0, &lFileSize );
				setMetaData( content , METADATA_FileSize , lFileSize );
			}
			else
			{
				MLOG(ZQ::common::Log::L_ERROR,PORTALFMT(populateAttrsFromFile,"no mainfilesize is parsed , and can't get main file name , do not update filesize"));
			}
		}
		else
		{
			setMetaData( content , METADATA_FileSize , lFileSize );
		}
	}

	setMetaData( content , SYS_PROP(ContentInLocalFileSystem) , idxData.isLocalContent() ? 1 : 0  );
	//setMetaData( content , SYS_PROP(ContentPropertyParsedOK) , 1 );

	MLOG( ZQ::common::Log::L_INFO , 
		PORTALFMT(populateAttrsFromFile,"end populate properties for Content[%s]"),
		mainFilePathnameReal.c_str());

	return true;
}

std::string vstrmCSPortalI::showCheckResidentialFlag( uint64 flag )
{
	char	szTemp[1024];
	char	*p = szTemp;
	size_t	lenTemp = sizeof(szTemp)-1;	
	szTemp[lenTemp] = 0;
	if( flag & ( 1 << TianShanIce::Storage::frfResidential) )
	{
		int iRet = snprintf(p,lenTemp ,"%s " , "frfResidential" );
		p += iRet;
		lenTemp -= iRet ;
	}
	if( flag & ( 1 << TianShanIce::Storage::frfReading ) )
	{
		int iRet = snprintf(p,lenTemp ,"%s " , "frfReading" );
		p += iRet;
		lenTemp -= iRet ;
	}
	if( flag & ( 1 << TianShanIce::Storage::frfWriting ) )
	{
		int iRet = snprintf(p,lenTemp ,"%s " , "frfWriting" );
		p += iRet;
		lenTemp -= iRet ;
	}

	if( flag & ( 1 << TianShanIce::Storage::frfAbsence ) )
	{
		int iRet = snprintf(p,lenTemp ,"%s " , "frfAbsence" );
		p += iRet;
		lenTemp -= iRet ;
	}

	if( flag & ( 1 << TianShanIce::Storage::frfCorrupt ) )
	{
		int iRet = snprintf(p,lenTemp ,"%s " , "frfCorrupt" );
		p += iRet;
		lenTemp -= iRet ;
	}

	if( flag & ( 1 << TianShanIce::Storage::frfDirectory ) )
	{
		int iRet = snprintf(p,lenTemp ,"%s " , "frfDirectory" );
		p += iRet;
		lenTemp -= iRet ;
	}
	return szTemp;

}

bool vstrmCSPortalI::checkFileCharacters( const std::string& file ,
											int32* readerCount , 
											int32* writerCount, 
											int64* fileSize )
{
	DLL_FIND_DATA	findData;
	VHANDLE findHandle = VstrmFindFirstFile( mVstrmHandle , file.c_str()  , &findData );
	if( findHandle != INVALID_HANDLE_VALUE )
	{
		if ( findData.VstrmFileVersion.StructureLength > FIELD_OFFSET( VSTRM_FILE_VERSION, Modulus ) )
		{
			if(fileSize)
			{
				*fileSize = (((int64)findData.w.nFileSizeHigh << 32) | findData.w.nFileSizeLow );
			}
			if(readerCount)
			{
				*readerCount = findData.ActiveReaderCount;
			}
			if(writerCount)
			{
				*writerCount = findData.ActiveWriterCount;
			}
			MLOG(ZQ::common::Log::L_INFO,
				PORTALFMT(checkFileCharacters,"get file[%s]'s characters fileSize[%lld] readerCount[%d] writerCount[%d]"),
				file.c_str(),
				(((int64)findData.w.nFileSizeHigh << 32) | findData.w.nFileSizeLow ),
				findData.ActiveReaderCount,
				findData.ActiveWriterCount);
		}
		VstrmFindClose( mVstrmHandle , findHandle );
		return true;
	}
	else
	{
		MLOG(ZQ::common::Log::L_WARNING,
			PORTALFMT(checkFileCharacters,"can't find file[%s] when checkFileCharacters"),
			file.c_str() );

		return false;
	}
}

std::string vstrmCSPortalI::showStringSet( const STRINGSET& strSet )
{
	char szTemp[2048];
	size_t lenTemp =sizeof(szTemp) -1;
	szTemp[lenTemp] = 0;
	szTemp[0] = 0;
	char*	p = szTemp;
	STRINGSET::const_iterator it = strSet.begin() ;
	for( ;  it != strSet.end() && lenTemp > 0 ; it ++ )
	{
		int iRet = snprintf(p,lenTemp,"%s ",it->c_str() );
		p += iRet;
		lenTemp -= iRet;
	}
	return szTemp;
}
bool vstrmCSPortalI::getFileSet(	const ContentImpl* content, 
									const std::string& mainFilePathName ,
									STRINGSET& fileSet,
									bool* bAllImportant)
{	
	if(content)
	{
		getFileSetFromMetaData( content->metaData , fileSet );
	}
	if(fileSet.empty() )
	{
		MLOG(ZQ::common::Log::L_DEBUG,PORTALFMT(getFileSet,"can't get fileset fom metadata for mainFile[%s]"), mainFilePathName.c_str() );
		if( !mbEdgeServer )
		{
			bool bNpVr = false;
			std::string indexFilePath ;
			if(content)
			{
				if( getMetaData(content->metaData , METADATA_nPVRLeadCopy ,indexFilePath ))
				{
					if( !indexFilePath.empty() )
					{
						bNpVr = true;
						MLOG(ZQ::common::Log::L_INFO,
							PORTALFMT(getFileSet,"MainFile[%s] current content is a nPVR copy"),
							mainFilePathName.c_str() );
					}
				}
			}
			if( !bNpVr )
			{
				indexFilePath = scanLeadSessionExtFilePathName( mainFilePathName );
				bNpVr = !indexFilePath.empty();
			}
			getFileSetFromFS( mainFilePathName , indexFilePath, bNpVr , fileSet );
		}
		else
		{
			getFileSetFromFS(mainFilePathName , "" , false , fileSet );
		}
	}
	return fileSet.size() > 0 ;
}
bool vstrmCSPortalI::getFileSet( ContentImpl::Ptr pContent,
								const std::string& mainFilePathName , 
								STRINGSET& fileSet ,
								bool* bAllImportant)
{
	return getFileSet( pContent.get() , mainFilePathName , fileSet , bAllImportant );
}

uint64 vstrmCSPortalI::checkResidentialStatus( uint64 flagsToTest, ContentImpl::Ptr pContent,
												const ::std::string& contentFullName,
												const ::std::string& mainFilePathname )
{
	MLOG(ZQ::common::Log::L_INFO,
		PORTALFMT(checkResidentialStatus,"MainFile[%s] checkResidential with flag[%s]"),
		mainFilePathname.c_str() , showCheckResidentialFlag(flagsToTest).c_str() );
	
	STRINGSET	fileSet;
	bool		bAllImportant = true;
	if( !getFileSet( pContent , mainFilePathname , fileSet , &bAllImportant ) )
	{
		MLOG(ZQ::common::Log::L_WARNING, PORTALFMT(checkResidentialStatus,"MainFile[%s] can't get file set"),
			mainFilePathname.c_str( ) );
	}

	uint64	flagToRet = 0;
	if( flagsToTest & ( 1 << TianShanIce::Storage::frfResidential) )
	{
		bool bExistence = false;
		if( pContent && pContent->stampLastUpdated <=0 )
		{
			flagToRet |= ( 1 << TianShanIce::Storage::frfResidential );
			flagToRet |= ( 1 << TianShanIce::Storage::frfAbsence );
		}		
		else if(fileSet.size() > 0 )
		{
			STRINGSET::const_iterator it = fileSet.begin() ;
			for( ; it != fileSet.end() ; it ++ )
			{
				if( checkFileExistence( *it ) )
				{
					bExistence = true;
					break;
				}
			}
		}
		else
		{
			bExistence = checkFileExistence(mainFilePathname);
		}
		if( bExistence )
		{
			flagToRet |= ( 1 << TianShanIce::Storage::frfResidential );
		}
		else
		{
			MLOG(ZQ::common::Log::L_DEBUG,
				PORTALFMT( checkResidentialStatus,"failed to check fileSet's frfResidential for MainFile[%s]"),
				mainFilePathname.c_str( ) );
		}
	}	
	if( flagsToTest & ( 1 << TianShanIce::Storage::frfAbsence ) )
	{
		bool bAbsence = true;
		bool bAnyOneExist = false;
		if(!bAllImportant )
		{
			bAbsence = true;//If there is no any ref copy , just set it to true 
			STRINGSET::const_iterator it = fileSet.begin();
			for( ; !bAbsence && it != fileSet.end() ; it ++ )
			{
				if( checkFileExistence( *it ) )
				{//set absence to false if there is any file exit and bAllImportant = false
					bAbsence = false;
					bAnyOneExist = true;
					break;
				}
			}
		}
		else
		{
			bAbsence = (fileSet.size() <= 0);
			STRINGSET::const_iterator it = fileSet.begin();
			for( ; /*!bAbsence &&*/ it != fileSet.end() ; it ++ )
			{
				if( !checkFileExistence( *it ) )
				{
					bAbsence = true;					
				}
				else
				{
					bAnyOneExist = true;
				}
			}
		}
		if (bAbsence)
			flagToRet |= ( 1<<TianShanIce::Storage::frfAbsence) ;
		
		if( mbEdgeServer )
		{
			//if( flagToRet & (1<<TianShanIce::Storage::frfResidential))
			if( bAnyOneExist )
			{//edge server mode 
				//if any member file exist , all file exist
				flagToRet &= (~(1<<TianShanIce::Storage::frfAbsence));
			}
		}
	}
	
	if( flagsToTest & ( 1<< TianShanIce::Storage::frfReading  )|| 
		flagsToTest & ( 1 << TianShanIce::Storage::frfWriting ) )
	{
		STRINGSET::const_iterator it = fileSet.begin() ;
		int32	readerCount;
		int32	writerCount;		
		for( ; it != fileSet.end() ; it ++ )
		{
			if( checkFileCharacters(*it ,&readerCount ,& writerCount , NULL ) )
			{

				if( flagsToTest &  ( 1<< TianShanIce::Storage::frfReading ) )
				{				
					if( readerCount > 0  )
					{
						flagToRet |=  ( 1<<TianShanIce::Storage::frfReading ) ;
					}			
				}
				if( flagsToTest &  ( 1 << TianShanIce::Storage::frfWriting ) )
				{				
					if( writerCount > 0  )
					{
						flagToRet |= ( 1 << TianShanIce::Storage::frfWriting );
					}				
				}
			}			
		}
	}

	if( flagsToTest &  ( 1 << TianShanIce::Storage::frfCorrupt  ))
	{//ignore
	}

	if( flagsToTest & ( 1 << TianShanIce::Storage::frfDirectory ) )
	{//ignore	
	}

	flagsToTest = (flagToRet & flagsToTest);

	MLOG(ZQ::common::Log::L_INFO,
		PORTALFMT(checkResidentialStatus,"MainFile[%s] check Residential return [%s]"),
		mainFilePathname.c_str(),
		showCheckResidentialFlag(flagToRet).c_str());

	return flagsToTest;
}

bool vstrmCSPortalI::deleteFileByContent( const ContentImpl& content, 
										 const ::std::string& mainFilePathname )
{	

	STRINGSET fileSet;
	std::string deleteMark;
	if( isLeadSessionFile(mainFilePathname) )
	{
		if( getMetaData(content, LEAD_SESSION_DELETE_MARK() , deleteMark ))
		{//we can delete it
			MLOG(ZQ::common::Log::L_INFO,PORTALFMT(deleteFileByContent,"lead session content[%s] , can be deleted now"),
				mainFilePathname.c_str() );
		}
		else
		{
			MLOG(ZQ::common::Log::L_INFO,PORTALFMT(deleteFileByContent,"add content [%s] to later delete procudure"),
				mainFilePathname.c_str() );			
			addContentToLaterDeleteProcudure( mainFilePathname );
			return true;
		}
	}
	else if ( isVirtualSessionFile( mainFilePathname ) )
	{
		std::string leadSessionIndexFilePath ;
		if( !getMetaData(content,METADATA_nPVRLeadCopy ,leadSessionIndexFilePath ) )
		{
			leadSessionIndexFilePath = scanLeadSessionExtFilePathName(mainFilePathname);
		}
		if( !leadSessionIndexFilePath.empty() )
		{
			if( isSubFile(leadSessionIndexFilePath))
			{
				std::string::size_type dotPos = leadSessionIndexFilePath.find_last_of('.');
				if( dotPos != std::string::npos )
				{
					leadSessionIndexFilePath = leadSessionIndexFilePath.substr(0 , dotPos );
				}
			}
			MLOG(ZQ::common::Log::L_INFO,PORTALFMT(deleteFileByContent,"add content [%s] to later delete procudure"),
				leadSessionIndexFilePath.c_str() );
			addContentToLaterDeleteProcudure( leadSessionIndexFilePath );
		}
	}

	getFileSet( &content , mainFilePathname , fileSet );
	MLOG(ZQ::common::Log::L_INFO,
		PORTALFMT(deleteFileByContent,"delete by MainFile[%s] and fileSet :: [%s]"),
		mainFilePathname.c_str(), showStringSet(fileSet).c_str() );
	STRINGSET::const_iterator it = fileSet.begin() ;
	for( ; it != fileSet.end() ; it ++ )
	{
		if( deleteVstrmFile( *it ) )
		{
			MLOG(ZQ::common::Log::L_INFO,PORTALFMT(deleteFileByContent,"file [%s] is deleted"),it->c_str() );
		}
		else
		{
			MLOG(ZQ::common::Log::L_INFO,PORTALFMT(deleteFileByContent,"failed to delete file [%s] "),it->c_str() );
		}
	}

	bool bExist = false;
	for( ; it != fileSet.end() ; it ++ )
	{
		if( checkFileExistence(*it))
		{
			bExist = true;
			break;
		}
	}
	MLOG(ZQ::common::Log::L_INFO,
		PORTALFMT(deleteFileByContent,"delete by MainFile[%s] and confirm[%s]"),
		mainFilePathname.c_str(), (bExist ? "Fail" : "OK") );
	return !bExist;
}
bool vstrmCSPortalI::deleteVstrmFile( const std::string& filePathName )
{
	return VstrmDeleteFile( mVstrmHandle , filePathName.c_str() );
}

bool vstrmCSPortalI::completeRenaming( const ::std::string& /*mainFilePathname*/, 
										 const ::std::string& /*newPathname*/ )
{
	return true;
}

void vstrmCSPortalI::getFileSetFromFS( const std::string& mainFilePathName ,
										const std::string& indexFilePath ,  
										bool bNpvrCopy  , 
										STRINGSET& fileSet,
										bool* bAllImportant)
{
	ZQ::IdxParser::IndexFileParser idxParser( *mIdxParserEnv );
	ZQ::IdxParser::IndexData idxData;
	if( idxParser.ParseIndexFileFromVstrm( mainFilePathName , idxData , bNpvrCopy , indexFilePath ) )
	{		
		{
			int iSubFileCount = idxData.getSubFileCount();
			int	iSubFileIndex = 0;
			for(int i =0 ;  i< iSubFileCount ;i ++ )
			{
				const std::string& strSubfileExtension =  idxData.getSubFileName( i ) ;
				if( !strSubfileExtension.empty() )
				{
					std::string	subFileName = mainFilePathName + strSubfileExtension;
					fileSet.push_back( subFileName );				
				}
				else
				{
					fileSet.push_back( mainFilePathName );
				}
			}	
			if(!bNpvrCopy)
			{
				fileSet.push_back(idxData.getIndexFilePathName());				
			}		
		}
	}
	else
	{
		fileSet.clear();
	}
}

void vstrmCSPortalI::getRealFileSetFromMetaData( const TianShanIce::Properties& metaData , STRINGSET& fileSet )
{
	char buf[512];
	int iSubFileIndex = 0;
	std::string		value;
	while ( 1 )
	{
		sprintf( buf , "%s%d" , METADATA_SUBFILENAME , iSubFileIndex );
		if( getMetaData(metaData , buf , value ) )
		{
			fileSet.push_back( value );
		}
		else
		{
			break;
		}
		iSubFileIndex++;
	}
	getMetaData( metaData , METADATA_FILENAME_MAIN , value );
	if( !value.empty() )
	{
		fileSet.push_back(value);
	}
}

void vstrmCSPortalI::getFileSetFromMetaData( const TianShanIce::Properties& metaData  , 
											STRINGSET& fileSet,
											bool* bAllImportant)
{

	char buf[512];
	int iSubFileIndex = 0;
	std::string value;
	//check if there is any concerned file exist
	//METADATA_CONCERNEDFILE
	bool  bEdgeServer = false;
	if( getMetaData(metaData , METADATA_SERVERMODE() , value ) )
	{
		if( value == SERVERMODE_EDGESERVER )
		{
			bEdgeServer = true;
		}
	}
	if( bEdgeServer )
	{
		if( getMetaData(metaData , METADATA_FILENAME_MAIN , value ) )
		{
			fileSet.push_back( value );
		}		
	}
	//else
	{
		getMetaData( metaData , METADATA_INDEXPARSEDOK() , value );
		if( value == STRINDEXPARSEDOK )
		{
			while ( 1 )
			{
				sprintf( buf , "%s%d" , METADATA_SUBFILENAME , iSubFileIndex );
				if( getMetaData(metaData , buf , value ) )
				{
					fileSet.push_back( value );
				}
				else
				{
					break;
				}
				iSubFileIndex++;
			}
		}
	}
	if(getMetaData(metaData,METADATA_LEADSESSION(),value))
	{
		if( !value.empty() && bAllImportant )
		{
			*bAllImportant = false;
		}
	}
	MLOG(ZQ::common::Log::L_DEBUG,PORTALFMT(getFileSetFromMetaData,"get file set:[%s]"),ZQTianShan::Util::dumpStreamInfoValues(fileSet).c_str() );
}

bool vstrmCSPortalI::checkFileExistence( const std::string& filePath )
{
	DLL_FIND_DATA	findData;
	VHANDLE findHandle = VstrmFindFirstFile( mVstrmHandle , filePath.c_str()  , &findData );
	if( findHandle != INVALID_HANDLE_VALUE )
	{		
		VstrmFindClose( mVstrmHandle , findHandle );
		return true;
	}
	else
	{
		return false;
	}
}



}}//namespace ZQTianShan::ContentStore
