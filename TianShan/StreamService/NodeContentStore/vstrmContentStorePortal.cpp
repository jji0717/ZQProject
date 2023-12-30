
#include <zq_common_conf.h>
#include <TianShanIce.h>
#include <TsContentProv.h>
#include "embededContentStore.h"
#include "vstrmContentStore.h"
#include "StreamSmithConfig.h"
#include "VvxParser.h"
#include "VV2Parser.h"
#include <TimeUtil.h>
#include <seafileInfo.h>
#include <ContentProvisionWrapper.h>
#include <vsiolib.h>
#include "vstrmCsPortal.h"

#ifndef envlog
#  define envlog ( store._log)
#endif // envlog

//this is a user metadata in the content, if 1 means NPVR session
#define	METADATA_PARAM_NPVR_TYPE						"nPVRCopy"
#define METADATA_PARAM_NPVR_PROVIDERID					"ProviderId"
#define METADATA_PARAM_NPVR_PROVIDERASSETID				"ProviderAssetId"
#define METADATA_PARAM_NPVR_SUBSCRIBERID				"SubscriberId"




namespace ZQTianShan 
{
namespace ContentStore 
{

#define MOLOG (store._log)


void ContentStoreImpl::notifyReplicasChanged(ContentStoreImpl& store, const ::TianShanIce::Replicas& replicasOld, const ::TianShanIce::Replicas& replicasNew)
{	
	return;
}
void ContentStoreImpl::initializePortal(ContentStoreImpl& store)
{
	if( NULL != store._ctxPortal )
		return ;

	PortalCtx* pCtx= new PortalCtx(store._thpool);
	assert( pCtx != NULL );

	pCtx->idxEnv =  new  ZQ::IdxParser::IdxParserEnv();
	assert( pCtx->idxEnv != NULL );
	pCtx->idxEnv->InitVstrmEnv(ZQ::StreamSmith::NCSBridge::getVstrmHandle());
	pCtx->idxEnv->AttchLogger(&store._log);

	pCtx->cpWrapper = NULL;
	pCtx->fileSystemEventSinker = NULL;
	if( GAPPLICATIONCONFIGURATION.embededContenStoreCfg.csEnableCpc >= 1 )
	{
		pCtx->cpWrapper	= new ContentProvisionWrapper(MOLOG);
	}
	pCtx->fileSystemEventSinker = new ZQ::StreamSmith::vstrmFileSystemSink(store);

	store._ctxPortal =  reinterpret_cast<void*>(pCtx);

	if ( NULL == store._ctxPortal )
		return;
	pCtx->fileSystemEventSinker->attchVstrmHandle( ZQ::StreamSmith::NCSBridge::getVstrmHandle() );
	pCtx->fileSystemEventSinker->start();
	if( GAPPLICATIONCONFIGURATION.embededContenStoreCfg.csEnableCpc >= 1 )
	{
		ContentProvisionWrapper::Ptr& cpWrapper = pCtx->cpWrapper;
		Ice::Identity csIdent = store._adapter->getCommunicator()->stringToIdentity(SERVICE_NAME_ContentStore);
		::TianShanIce::Storage::ContentStoreExPrx  csPrx = ::TianShanIce::Storage::ContentStoreExPrx::uncheckedCast(store._adapter->createProxy(csIdent));

		if (!cpWrapper->init( store._adapter->getCommunicator(), 
			csPrx , 
			GAPPLICATIONCONFIGURATION.embededContenStoreCfg.csCpcEndpoint ,
			GAPPLICATIONCONFIGURATION.embededContenStoreCfg.CpcConfig.sessionRegisterInterval) )
		{	
			MOLOG(ZQ::common::Log::L_ERROR,CLOGFMT(ContentStoreImpl,"initialize ContentProvisionWrapper failed"));
			return;
		}	
	}
}

void ContentStoreImpl::uninitializePortal(ContentStoreImpl& store)
{
	if( GAPPLICATIONCONFIGURATION.embededContenStoreCfg.csEnableCpc >= 1 )
	{
		if (NULL != store._ctxPortal)
		{
			PortalCtx* pCtx = (PortalCtx*)(store._ctxPortal);
			ContentProvisionWrapper::Ptr& cpWrapper = pCtx->cpWrapper;
			cpWrapper->unInit();
			cpWrapper = NULL;
		}
	}


	if ( NULL != store._ctxPortal )
	{
		PortalCtx* pCtx = (PortalCtx*)(store._ctxPortal);
		ZQ::StreamSmith::vstrmFileSystemSink* pFilesystemSink = pCtx->fileSystemEventSinker;
		pFilesystemSink->stop( );
		delete pFilesystemSink;

		if( pCtx->idxEnv )
		{
			pCtx->idxEnv->DetachLogger();
			delete pCtx->idxEnv;
		}
	}

	if( NULL != store._ctxPortal )		
	{
		PortalCtx* pCtx = (PortalCtx*)(store._ctxPortal);
		delete pCtx;
		pCtx = NULL;
	}

	store._ctxPortal = NULL;
}

bool ContentStoreImpl::createPathOfVolume(ContentStoreImpl& store, const std::string& pathOfVolume)
{
	return vstrmCSPortalI( store._ctxPortal , store ).createPathOfVolume(pathOfVolume);
}

bool ContentStoreImpl::deletePathOfVolume(ContentStoreImpl& store, const std::string& pathOfVolume)
{
	return vstrmCSPortalI( store._ctxPortal , store ).deletePathOfVolume(pathOfVolume);
}

//void  ContentStoreImpl::getStorageSpace(ContentStoreImpl& store, uint32& freeMB, uint32& totalMB)
void ContentStoreImpl::getStorageSpace(ContentStoreImpl& store, uint32& freeMB, uint32& totalMB, const char* rootPath)
{
	vstrmCSPortalI( store._ctxPortal , store ).getStorageSpace(freeMB,totalMB,rootPath);
}

bool ContentStoreImpl::validateMainFileName(ContentStoreImpl& store, std::string& contentName,
											const std::string& contentType)
{
	return vstrmCSPortalI( store._ctxPortal , store ).validateMainFileName( contentName , contentType );
}

ContentStoreImpl::FileInfos ContentStoreImpl::listMainFiles(ContentStoreImpl& store, const char* rootPath)
{
	return vstrmCSPortalI( store._ctxPortal , store ).listMainFiles( rootPath );
}

bool ContentStoreImpl::deleteFileByContent(ContentStoreImpl& store, const ContentImpl& content, const ::std::string& mainFilePathname)
{
	return vstrmCSPortalI( store._ctxPortal , store ).deleteFileByContent( content, mainFilePathname );
}


bool ContentStoreImpl::populateAttrsFromFile(ContentStoreImpl& store,
											 ContentImpl& content, 
											 const ::std::string& contentName )
{
	return vstrmCSPortalI( store._ctxPortal , store ).populateAttrsFromFile( content, contentName );
}


std::string ContentStoreImpl::memberFileNameToContentName(ContentStoreImpl& store, const std::string& memberFilename)
{
	return vstrmCSPortalI( store._ctxPortal , store ).memberFileNameToContentName( memberFilename );
}

uint64 ContentStoreImpl::checkResidentialStatus(ContentStoreImpl& store ,
												uint64 flagsToTest, 
												ContentImpl::Ptr pContent, 
												const ::std::string& contentFullName, 
												const ::std::string& mainFilePathname)
{
	return vstrmCSPortalI( store._ctxPortal , store ).checkResidentialStatus( flagsToTest , pContent , contentFullName , mainFilePathname );
}

bool ContentStoreImpl::completeRenaming(ContentStoreImpl& store, const ::std::string& oldName, const ::std::string& newName)
{
	return vstrmCSPortalI( store._ctxPortal , store ).completeRenaming( oldName , newName );
}


TianShanIce::ContentProvision::ProvisionSessionPrx 
ContentStoreImpl::submitProvision(ContentStoreImpl& store, ContentImpl& content, const ::std::string& contentName,
								  const ::std::string& sourceUrl, const ::std::string& sourceType,
								  const ::std::string& startTimeUTC, const ::std::string& stopTimeUTC, const int maxTransferBitrate)
								  throw (::TianShanIce::InvalidParameter, ::TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt)
{
	if( GAPPLICATIONCONFIGURATION.embededContenStoreCfg.csEnableCpc < 1 )
	{
		MOLOG( ZQ::common::Log::L_WARNING , CLOGFMT( ContentStoreImpl , "Cpc is disabled" ) );
		return NULL;
	}

	/* provision session exists */
	std::string strProvisionSess = content.provisionPrxStr;
	if(!strProvisionSess.empty()) 
	{
		TianShanIce::ContentProvision::ProvisionSessionPrx session;
		try 
		{
			session = TianShanIce::ContentProvision::ProvisionSessionPrx::checkedCast(
				store._adapter->getCommunicator()->stringToProxy(strProvisionSess));
		}
		catch (const Ice::Exception& ex) 
		{
			MOLOG(ZQ::common::Log::L_WARNING, 
				LOGFMT("[%s] Open provision session[%s] for updateScheduledTime() caught exception[%s]"),
				contentName.c_str(), strProvisionSess.c_str(), ex.ice_name().c_str());
		}

		try
		{
			std::string start, stop;
			session->getScheduledTime(start, stop);

			//
			// need to change the time to tianshan time to compare, because IM use localtime+timezero sometime, but we always use utc
			//
#pragma message("need to change the time to tianshan time to compare, because IM use localtime+timezero sometime, but we always use utc")
			if(start != startTimeUTC || stop != stopTimeUTC) 
			{
				MOLOG(ZQ::common::Log::L_INFO, 
					LOGFMT("[%s] update schedule time: [start (%s --> %s) stop (%s --> %s"),
					contentName.c_str(), start.c_str(), startTimeUTC.c_str(), stop.c_str(), stopTimeUTC.c_str());

				session->updateScheduledTime(startTimeUTC, stopTimeUTC);
			}
		} 
		catch (const Ice::Exception& ex) 
		{
			MOLOG(ZQ::common::Log::L_ERROR, 
				LOGFMT("failed to update schedule time [start: (%s) stop: (%s)] for (%s): (%s)"),
				startTimeUTC.c_str(), stopTimeUTC.c_str(), contentName.c_str(), ex.ice_name().c_str());

			ZQTianShan::_IceThrow<TianShanIce::ServerError>(
				MOLOG,
				EXPFMT(MediaClusterCS, csexpInternalError, "failed to updateScheduledTime() with start[%s] stop[%s]"), 
				startTimeUTC.c_str(),
				stopTimeUTC.c_str()
				);
		}

		return session;
	}

	int transferBitrate = maxTransferBitrate;
	if (!transferBitrate)
	{
		transferBitrate = GAPPLICATIONCONFIGURATION.embededContenStoreCfg.CpcConfig.defaultProvisionBW * 1000;
	}
	//
	// find out if it is a NPVR session
	//
	bool bNPVRSession = false;
	{
		::TianShanIce::Properties metaDatas = content.getMetaData(Ice::Current());
		std::string keyname = std::string(USER_PROP_PREFIX) + METADATA_PARAM_NPVR_TYPE;
		::TianShanIce::Properties::const_iterator it = metaDatas.find(keyname);
		if (it!=metaDatas.end())
		{
			if (atoi(it->second.c_str()))
			{
				bNPVRSession = true;				
			}

			//
			// dump the NPVR parameters
			//

			MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(MediaClusterCS, "(%s) NPVR propaties: [%s] = [%s]"), 
				contentName.c_str(), it->first.c_str(), it->second.c_str());

			keyname = std::string(USER_PROP_PREFIX) + METADATA_PARAM_NPVR_PROVIDERID;
			it = metaDatas.find(keyname);
			if (it!=metaDatas.end())
			{
				MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(MediaClusterCS, "(%s) NPVR propaties: [%s] = [%s]"), 
					contentName.c_str(), it->first.c_str(), it->second.c_str());
			}

			keyname = std::string(USER_PROP_PREFIX) + METADATA_PARAM_NPVR_PROVIDERASSETID;
			it = metaDatas.find(keyname);
			if (it!=metaDatas.end())
			{
				MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(MediaClusterCS, "(%s) NPVR propaties: [%s] = [%s]"), 
					contentName.c_str(), it->first.c_str(), it->second.c_str());
			}

			keyname = std::string(USER_PROP_PREFIX) + METADATA_PARAM_NPVR_SUBSCRIBERID;
			it = metaDatas.find(keyname);
			if (it!=metaDatas.end())
			{
				MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(MediaClusterCS, "(%s) NPVR propaties: [%s] = [%s]"), 
					contentName.c_str(), it->first.c_str(), it->second.c_str());
			}
		}
	}

	std::string strFilePathName = content.getMainFilePathname(Ice::Current());

	//remove the first possible '\' or "\\"
	if (strFilePathName[0]=='\\' || strFilePathName[0]=='/')
		strFilePathName = strFilePathName.substr(1);


	::TianShanIce::Storage::ContentPrx	contentPrx;
	{
		// get the content proxy
		contentPrx = ::TianShanIce::Storage::ContentPrx::uncheckedCast(store._adapter->createProxy(content.ident));
	}

	TianShanIce::ContentProvision::ProvisionContentKey	contentKey;
	contentKey.content = contentName;
	contentKey.contentStoreNetId = store._netId;
	contentKey.volume = content.identVolume.name;

	ContentProvisionWrapper::Ptr& cpWrapper = ((PortalCtx*)store._ctxPortal)->cpWrapper;;

	TianShanIce::ContentProvision::ProvisionSessionPrx pPrx = cpWrapper->activeProvision(
		contentPrx,
		contentKey,
		strFilePathName,	
		sourceUrl,
		sourceType, 
		startTimeUTC,
		stopTimeUTC, 
		transferBitrate,
		bNPVRSession);

	if (bNPVRSession)
	{
		//get property and set to content 
		std::string strnPVRLeadCopy;

		::TianShanIce::Properties propers = pPrx->getProperties();
		::TianShanIce::Properties::const_iterator it = propers.find(METADATA_nPVRLeadCopy);
		if (it!=propers.end())
		{
			strnPVRLeadCopy = it->second;
		}

		MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(MediaClusterCS, "[%s] nPVR lead session copy [%s]"), 
			strFilePathName.c_str(), strnPVRLeadCopy.c_str());

		::TianShanIce::Storage::UnivContentPrx uniContent = ::TianShanIce::Storage::UnivContentPrx::uncheckedCast(contentPrx);
		TianShanIce::Properties metaData;
		metaData[METADATA_nPVRLeadCopy] = strnPVRLeadCopy;
		try
		{
			uniContent->setMetaData(metaData);
		}
		catch (const Ice::Exception& ) 
		{
		}
		catch (...) 
		{
		}
	}

	return pPrx;
}

TianShanIce::ContentProvision::ProvisionSessionPrx 
ContentStoreImpl::bookPassiveProvision(ContentStoreImpl& store, const ContentImpl& content,
									   const ::std::string& contentName, ::std::string& pushUrl, 
									   const ::std::string& sourceType, 
									   const ::std::string& startTimeUTC, const ::std::string& stopTimeUTC, 
									   const int maxTransferBitrate)										
{
	if( GAPPLICATIONCONFIGURATION.embededContenStoreCfg.csEnableCpc < 1 )
	{
		MOLOG( ZQ::common::Log::L_WARNING , CLOGFMT( ContentStoreImpl , "Cpc is disabled" ) );
		return NULL;
	}

	std::string strFilePathName = content.getMainFilePathname(Ice::Current());

	//remove the first possible '\' or "\\"
	if (strFilePathName[0]=='\\' || strFilePathName[0]=='/')
		strFilePathName = strFilePathName.substr(1);

	::TianShanIce::Storage::ContentPrx	contentPrx;
	{
		// get the content proxy
		contentPrx = ::TianShanIce::Storage::ContentPrx::uncheckedCast(store._adapter->createProxy(content.ident));
	}

	TianShanIce::ContentProvision::ProvisionContentKey	contentKey;
	contentKey.content = contentName;
	contentKey.contentStoreNetId = store._netId;
	contentKey.volume = content.identVolume.name;

	ContentProvisionWrapper::Ptr& cpWrapper = ((PortalCtx*)store._ctxPortal)->cpWrapper;;

	TianShanIce::ContentProvision::ProvisionSessionPrx pPrx = cpWrapper->passiveProvision(
		contentPrx,
		contentKey,
		strFilePathName,					  
		sourceType, 
		startTimeUTC,
		stopTimeUTC, 
		maxTransferBitrate,
		pushUrl);

	return pPrx;


}

std::string ContentStoreImpl::getExportURL(ContentStoreImpl& store, ContentImpl& content,
										   const std::string& contentName,
										   const ::std::string& transferProtocol,
										   ::Ice::Int transferBitrate, 
										   ::Ice::Int& ttl, 
										   ::TianShanIce::Properties& params)
{
	if( GAPPLICATIONCONFIGURATION.embededContenStoreCfg.csEnableCpc < 1 )
	{
		MOLOG( ZQ::common::Log::L_WARNING , CLOGFMT( ContentStoreImpl , "Cpc is disabled" ) );
		return "";
	}

	ContentProvisionWrapper::Ptr& cpWrapper = ((PortalCtx*)store._ctxPortal)->cpWrapper;;

	//const std::string& protocal, const std::string& filename, int transferBitrate, int& nTTL, int& permittedBitrate
	int transBitrate = transferBitrate;
	int nTTL = 0;
	int permittedBitrate;

	/* invalidate the protocol. */
	if(transferProtocol != TianShanIce::Storage::potoFTP){
		ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt>(
			MOLOG,
			EXPFMT(MediaClusterCS, csexpUnsupportProto, "protocol (%s) not supported"), transferProtocol.c_str()
			);
	}

#pragma message ( __MSGLOC__ "TODO: change getExportUrl interface to add transfer bitrate etc.")
	std::string strExposeUrl = cpWrapper->getExposeUrl(transferProtocol, contentName, transBitrate, nTTL, permittedBitrate);

	ttl = nTTL;

	{
		std::ostringstream oss;
		oss << ttl;		
		params[expTTL] = oss.str();

		oss.str("");
		oss << permittedBitrate;
		params[expTransferBitrate] = oss.str();		
	}

	time_t now = time(0);
	char window[255];

	ZQ::common::TimeUtil::Time2Iso(now, window, 255);
	std::string stStart = window;
	params[expTimeWindowStart] = stStart;

	ZQ::common::TimeUtil::Time2Iso(now+ttl, window, 255);
	std::string stEnd = window;
	params[expTimeWindowEnd] = stEnd;


	MOLOG(ZQ::common::Log::L_DEBUG, 
		LOGFMT("(%s) getExportURL [URL: (%s) ttl: (%d) timeWindowStart: (%s) timeWindowEnd: (%s) bitrate: (%d)"), 
		contentName.c_str(), strExposeUrl.c_str(), ttl, stStart.c_str(), stEnd.c_str(), permittedBitrate);

	return strExposeUrl;
}

void ContentStoreImpl::cancelProvision(ContentStoreImpl& store, ContentImpl& content, const ::std::string& provisionTaskPrx)
{
	if( GAPPLICATIONCONFIGURATION.embededContenStoreCfg.csEnableCpc < 1 )
	{
		MOLOG( ZQ::common::Log::L_WARNING , CLOGFMT( ContentStoreImpl , "Cpc is disabled" ) );
		return ;
	}

	try
	{
		content.provisionPrxStr = provisionTaskPrx;

		//check content state;
		Ice::Current c;
		::TianShanIce::Storage::ContentState state = content.getState(c);
		if (::TianShanIce::Storage::csProvisioning != state)
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>("ContentStoreImpl",1,"Invalid State Of Art");
			return;
		}
		else
		{
			Ice::Current c;
			content.cancelProvision( c );
		}
	}
	catch (::TianShanIce::ServerError& )
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>("ContentStoreImpl",1,"Content Server Error");
	}
	return;
}

}}//namespace ZQTianShan::ContentStore

namespace ZQ
{
namespace StreamSmith
{
vstrmFileSystemSink::vstrmFileSystemSink( ZQTianShan::ContentStore::ContentStoreImpl& st )
:store(st)
{
	_bQuit = false;
	_vstrmHandle = NULL;	
}

vstrmFileSystemSink::~vstrmFileSystemSink()
{	
	if(!_bQuit)
	{
		stop();
	}
	if( _vstrmHandle )
	{
		VstrmClassCloseEx(_vstrmHandle);
	}
}
void vstrmFileSystemSink::attchVstrmHandle( VHANDLE vstrmHandle )
{
}
void vstrmFileSystemSink::stop( )
{
	if( _bQuit )
	{
		_bQuit = true;
		char szBuf[256];
		memset( szBuf, 0, sizeof(szBuf) );
		ZQ::common::Guid uid;
		uid.create();
		uid.toString( szBuf, sizeof(szBuf ) );	
		VstrmCreateFile( _vstrmHandle , szBuf , GENERIC_WRITE , FILE_SHARE_READ, NULL,OPEN_ALWAYS , FILE_ATTRIBUTE_NORMAL,NULL);
		waitHandle( INFINITE );
		VstrmDeleteFile( _vstrmHandle, szBuf );
	}
}
bool vstrmFileSystemSink::init()
{
	if( _vstrmHandle )
	{
		VstrmClassCloseEx(_vstrmHandle);
	}
	VstrmClassOpenEx( &_vstrmHandle );	
	return  true;
}

int vstrmFileSystemSink::run( )
{
	envlog(ZQ::common::Log::L_INFO , CLOGFMT(vstrmFileSystemSink,"enter vstrm file event scan process"));

	VHANDLE			hClass = INVALID_HANDLE_VALUE;

	if( VstrmClassOpenEx( &hClass ) != VSTRM_SUCCESS )
	{
		envlog(ZQ::common::Log::L_ERROR,CLOGFMT(vstrmFileSystemSink,"Open vstrm class failed"));
		return 1;
	}


	VSTRM_FILE_EVENT	fileEvent;
	memset( &fileEvent , 0, sizeof(fileEvent) );

	CHAR				oldFileName[ MAX_PATH ];
	CHAR				newFileName[ MAX_PATH ];
	BOOL				bOK;

	memset( oldFileName , 0 ,sizeof(oldFileName) );
	memset( newFileName , 0 ,sizeof(newFileName) );




	HANDLE	_fileHandle = VstrmFindFirstFileNotification(	hClass,	// Vstrm class handle
		&fileEvent,		// File event type
		oldFileName,	// Old file name
		newFileName );	// New file name
	if ( _fileHandle == NULL  )
	{
		char szError[1024];
		memset( szError , 0 , sizeof(szError) );
		VstrmClassGetErrorText(hClass , VstrmGetLastError() , szError, sizeof(szError )-1 );
		envlog(ZQ::common::Log::L_ERROR , 
			CLOGFMT(vstrmFileSystemSink , "VstrmFindFirstFileNotification() failed 0x%08X : %s" ),
			VstrmGetLastError() ,
			szError);
		return 1;
	}
	char* pNewFile = NULL;
	char* pOldFile = NULL;

	static size_t iPrefixLength = gStreamSmithConfig.embededContenStoreCfg.ctntAttr.ignoreFileEventPrefix.length();

	const std::string& strNamePrefix = gStreamSmithConfig.embededContenStoreCfg.ctntAttr.ignoreFileEventPrefix;
	const std::string& strNameInvalidChar = gStreamSmithConfig.embededContenStoreCfg.ctntAttr.ignoreInvalidCharacter;

	::TianShanIce::Properties params;

	do
	{
		pNewFile = newFileName;
		pOldFile = oldFileName;
		if ( !strNameInvalidChar.empty() &&
			( strstr( pNewFile , strNameInvalidChar.c_str() ) != NULL || strstr( pOldFile , strNameInvalidChar.c_str() ) != NULL ) )
		{
			pNewFile = NULL ;
			pOldFile = NULL;
		}
		else
		{
			if( !strNamePrefix.empty() )
			{
				char* pDelimiter = strstr( pNewFile , strNamePrefix.c_str() );
				if( pDelimiter )
				{
					pNewFile += iPrefixLength;
				}
				pDelimiter = strstr( pOldFile , strNamePrefix.c_str() );
				if( pDelimiter )
				{
					pOldFile += iPrefixLength;
				}
			}
		}

		if( pOldFile && pNewFile  )
		{
			switch( fileEvent )
			{
			case VSTRM_FILE_EVENT_LOST:
				{
					//printf( "!!! FILE EVENT LOST\n" );
					envlog(ZQ::common::Log::L_WARNING , CLOGFMT(vstrmFileSystemSink , "[FILEEVENT]   !!! File event lost, wahahahahahaha" ));
				}
				break;

			case VSTRM_FILE_NEW:				
				{
					//printf( "File NEW: %s\n", newFileName );
					envlog(ZQ::common::Log::L_INFO , CLOGFMT(vstrmFileSystemSink , "[FILEEVENT]   File NEW: %s" ), pNewFile );			
					//_monitor.onFileNew( pNewName );

					std::string fileName = ZQTianShan::ContentStore::ContentStoreImpl::fixupPathname( store , pNewFile );
					store.OnFileEvent(	TianShanIce::Storage::fseFileCreated,
						fileName, 
						params ,Ice::Current() );
				}
				break;

			case VSTRM_FILE_CREATED:
				{							
					envlog(ZQ::common::Log::L_INFO , CLOGFMT(vstrmFileSystemSink , "[FILEEVENT]   File CREATED: %s"), pNewFile );
					//_monitor.onFileCreated( pNewName );
					std::string fileName = ZQTianShan::ContentStore::ContentStoreImpl::fixupPathname( store , pNewFile );
					store.OnFileEvent(	TianShanIce::Storage::fseFileModified,
						fileName,
						params , Ice::Current() );
				}
				break;

			case VSTRM_FILE_MODIFIED:			
				{
					//printf( "File MODIFIED: %s\n", newFileName );
					envlog(ZQ::common::Log::L_INFO , CLOGFMT(vstrmFileSystemSink , "[FILEEVENT]   File MODIFIED: %s"), pNewFile  );
					std::string fileName = ZQTianShan::ContentStore::ContentStoreImpl::fixupPathname( store , pNewFile );
					store.OnFileEvent(	TianShanIce::Storage::fseFileModified,
						fileName,
						params , Ice::Current() );
				}
				break;

			case VSTRM_FILE_DELETE_PENDING:
				{
					//printf( "File DELETE PENDING: %s\n", oldFileName );
					envlog(ZQ::common::Log::L_INFO , CLOGFMT(vstrmFileSystemSink , "[FILEEVENT]   File DELETE PENDING: %s"), pOldFile );
				}
				break;

			case VSTRM_FILE_DELETED:
				{
					//printf( "File DELETED: %s\n", oldFileName );
					envlog(ZQ::common::Log::L_INFO , CLOGFMT(vstrmFileSystemSink , "[FILEEVENT]   File DELETED: %s"), pOldFile );
					std::string fileName = ZQTianShan::ContentStore::ContentStoreImpl::fixupPathname( store , pOldFile );
					store.OnFileEvent(	TianShanIce::Storage::fseFileDeleted,
						fileName,
						params , Ice::Current() );
				}
				break;

			case VSTRM_FILE_RENAMED:
				{
					//printf( "File RENAMED: %s --> %s\n",oldFileName, newFileName );
					envlog(ZQ::common::Log::L_INFO , CLOGFMT(vstrmFileSystemSink , "[FILEEVENT]   File RENAMED:  %s --> %s "),
						pOldFile , pNewFile );
					params["newFilename"] = ZQTianShan::ContentStore::ContentStoreImpl::fixupPathname( store , pNewFile ); 
					std::string oldFileName = ZQTianShan::ContentStore::ContentStoreImpl::fixupPathname( store , pOldFile );
					store.OnFileEvent(	TianShanIce::Storage::fseFileRenamed,
						oldFileName,
						params,Ice::Current());
					params.clear();
				}
				break;

			case VSTRM_FILE_PRIVATE_DATA_CHANGE:
				//printf( "File PRIVATE_DATA_CHANGE: %s\n", oldFileName );
				envlog(ZQ::common::Log::L_INFO , CLOGFMT(vstrmFileSystemSink , "[FILEEVENT]   File PRIVATE_DATA_CHANGE: %s"), pOldFile );
				break;

			default:
				//printf( "!!! Unknown file event \n" );
				envlog(ZQ::common::Log::L_WARNING , CLOGFMT(vstrmFileSystemSink ,"[FILEEVENT]   unknown file event" ));
				break;
			}
		}

		if ( _bQuit ) break;

		memset( oldFileName , 0 ,sizeof(oldFileName) );
		memset( newFileName , 0 ,sizeof(newFileName) );	

		bOK = VstrmFindNextFileNotification(	hClass,
			_fileHandle,
			&fileEvent,
			oldFileName,
			newFileName );
		if (!bOK)
		{
			//printf("VstrmFindNextFileNotification() failed 0x%08X\n",VstrmGetLastError() );
			envlog(ZQ::common::Log::L_WARNING , 
				CLOGFMT( vstrmFileSystemSink , "VstrmFindNextFileNotification() failed 0x%08X"),
				VstrmGetLastError() );
			break;
		}
		if ( _bQuit ) break;
	} while( 1 );

	if (_fileHandle != INVALID_HANDLE_VALUE ) 
	{
		try
		{
			VstrmFindCloseFileNotification( hClass, _fileHandle );
			_fileHandle = INVALID_HANDLE_VALUE;
		}
		catch (...) 
		{
		}
	}	
	if( hClass )
	{
		VstrmClassCloseEx( hClass );
		hClass = NULL;
	}	
	envlog(ZQ::common::Log::L_INFO , CLOGFMT(vstrmFileSystemSink,"leave vstrm file event scan process"));
	return 1;
}

VHANDLE vstrmFileSystemSink::getVstrmHandle()
{
	return _vstrmHandle;
}

void vstrmFileSystemSink::final()
{

}




ReplicaStatusReporter::ReplicaStatusReporter( )
{
	_hQuitEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	_bQuit	=	false;
}

ReplicaStatusReporter::~ReplicaStatusReporter()
{
	if(_hQuitEvent)
	{
		CloseHandle(_hQuitEvent);
		_hQuitEvent = NULL;
	}
}
void ReplicaStatusReporter::stop()
{
	if(!_bQuit)
	{
		_bQuit = true;
		SetEvent( _hQuitEvent ) ;
		waitHandle(5000);
	}
}
int ReplicaStatusReporter::run()
{
	while ( !_bQuit )
	{
		runTimerTask();
		if(_bQuit)
			break;
		WaitForSingleObject(_hQuitEvent,updateInterval);
	}
	return 1;
}

void ReplicaStatusReporter::runTimerTask( )
{
	if( replicaListenerEndpoint.empty() )
	{
		updateInterval = 60 * 60 * 1000;
		_store->_log(ZQ::common::Log::L_WARNING,CLOGFMT(ReplicaStatusReporter,"no listener endpoint , do not report update to subscriber"));
		return;
	}
	//step 1
	//connect to replica listener
	TianShanIce::ReplicaSubscriberPrx	subscriber = NULL;

	std::string		strListenerEndpoint ;
	if( replicaListenerEndpoint.find(":") != std::string::npos )
	{
		strListenerEndpoint = replicaListenerEndpoint;
	}
	else
	{
		strListenerEndpoint = SERVICE_NAME_ContentStore":" + replicaListenerEndpoint;
	}

	try
	{
		subscriber	= TianShanIce::ReplicaSubscriberPrx::checkedCast( 
			_store->_adapter->getCommunicator()->stringToProxy(strListenerEndpoint) 
			);
		if( !subscriber )
		{
			_store->_log(ZQ::common::Log::L_ERROR,
				CLOGFMT( ReplicaStatusReporter , "can't connect to replica listener[%s]"),
				strListenerEndpoint.c_str() );
		}
	}
	catch( const Ice::Exception& ex)
	{//If I catch an exception
		//I should quit the report process
		_store->_log(ZQ::common::Log::L_ERROR,
			CLOGFMT(ReplicaStatusReporter,"caught Ice Exception:%s when connect to replica listener[%s]"),
			ex.ice_name().c_str() ,
			strListenerEndpoint.c_str()	);
		subscriber = NULL;
	}
	//collect replicas
	::TianShanIce::Replicas  reps = _store->exportStoreReplicas();

	//report it to subscriber
	int iNextReportInterval = defaultUpdateInterval;
	try
	{
		if( subscriber )
		{
			iNextReportInterval = subscriber->updateReplica( reps ) * 500;
			if( iNextReportInterval <= 0 )
			{
				_store->_log(ZQ::common::Log::L_INFO,
					CLOGFMT(ReplicaStatusReporter,"subscriber return the update interval is [%d] , adjust it to [%d]") ,
					iNextReportInterval ,
					defaultUpdateInterval );
				iNextReportInterval = defaultUpdateInterval;
			}
			_store->_log(ZQ::common::Log::L_INFO, 
				CLOGFMT(ReplicaStatusReporter,"updaet replica to [%s] ok, and interval for next report is [%d]"),strListenerEndpoint.c_str() ,iNextReportInterval);
		}
		else
		{
			_store->_log(ZQ::common::Log::L_INFO,
				CLOGFMT(ReplicaStatusReporter,"failed to update replica to [%s] , and interval for next report is [%d] "),
				strListenerEndpoint.c_str(), iNextReportInterval );
		}
	}
	catch( const Ice::Exception& ex )
	{
		_store->_log(ZQ::common::Log::L_ERROR,
			CLOGFMT(ReplicaStatusReporter,"failed to report replicas status to subscriber and caught ice exception:[%s] and interval for next report is [%d] "),
			ex.ice_name().c_str() ,iNextReportInterval);
	}

	///update timer so that we can report the replicas again
	if( iNextReportInterval < 10 * 1000 )
		iNextReportInterval = 10 * 1000;

	updateInterval = iNextReportInterval;
}

}}//namespace ZQ::StreamSmith
