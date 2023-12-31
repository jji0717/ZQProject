
#include <zq_common_conf.h>
#include <TianShanIce.h>
#include <TsContentProv.h>
#include "embededContentStore.h"
#include "vstrmContentStore.h"
#include "StreamSmithConfig.h"
#include "VvxParser.h"
#include "VV2Parser.h"
#include <TimeUtil.h>
#include <TianShanIceHelper.h>
#include <seafileInfo.h>
#include <ContentProvisionWrapper.h>
#include <vsiolib.h>
#include "vstrmCsPortal.h"
#include "VstrmTlvTypes.h"



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

vstrmContentStoreImpl::vstrmContentStoreImpl( ZQ::common::Log& log, ZQ::common::Log& eventlog, ZQ::common::NativeThreadPool& threadPool, ZQADAPTER_DECLTYPE& adapter, const char* databasePath /* =NULL */ )
:ContentStoreImpl(log,eventlog,threadPool,adapter,databasePath)
{

}

void vstrmContentStoreImpl::OnVolumeMounted(const ::Ice::Identity& identVolume, const ::std::string& path)
{
	ZQTianShan::ContentStore::vstrmCSPortalI( _ctxPortal , *this  ).OnVolumeMounted(identVolume,path);
}


#define MOLOG (store._log)


vstrmFileSystemSink*					fileSystemEventSinker = NULL;

DeleteLaterProcudure*					deleteLaterProcedure = NULL;

#ifdef SUPPORT_VSTRM_VSIS_EVENT
VsisEventSinker*						vsisEventSinker = NULL;
#endif//SUPPORT_VSTRM_VSIS_EVENT

void ContentStoreImpl::notifyReplicasChanged(ContentStoreImpl& store, const ::TianShanIce::Replicas& replicasOld, const ::TianShanIce::Replicas& replicasNew)
{	
	return;
}

void adjustIngorePrefix(VHANDLE vstrmHandle)
{
	//adjust ignoreFileEventPrefix ignorePrefix
	CLUSTER_INFO clusterInfo;
	ULONG ret;
	if( VstrmClassGetClusterDataEx( vstrmHandle ,&clusterInfo,sizeof(clusterInfo) , &ret) != VSTRM_SUCCESS )
	{
		return;
	}

}

void ContentStoreImpl::initializePortal(ContentStoreImpl& store)
{
	if( NULL != store._ctxPortal )
		return ;

	PortalCtx* pCtx= new PortalCtx(store._thpool);
	assert( pCtx != NULL );

	pCtx->mpLogger		=	&store._log;
	
	pCtx->bSupportNpVr	=	(gStreamSmithConfig.serverMode == 1);
	pCtx->idxEnv		=  new  ZQ::IdxParser::IdxParserEnv();
	assert( pCtx->idxEnv != NULL );
	pCtx->idxEnv->InitVstrmEnv(ZQ::StreamSmith::NCSBridge::getVstrmHandle());
	pCtx->idxEnv->AttchLogger(&store._log);
	//If service is running at edge mode, just use Vstrm API to parse index file
	
	if (gStreamSmithConfig.embededContenStoreCfg.ctntAttr.attrFromVstm != 0)
	{
		pCtx->idxEnv->setUseVstrmIndexParseAPI(true);
		MOLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(ContentStoreImpl,"use vstrm api anyway, due to attrFromVstm is unequal to 0"));
	}
	else
	{
		pCtx->idxEnv->setUseVstrmIndexParseAPI( ( gStreamSmithConfig.serverMode == 2 ) );//EdgeServer
	}

	pCtx->idxEnv->setUseVsOpenAPI( gStreamSmithConfig.embededContenStoreCfg.ctntAttr.useVsOpenAPI >= 1 );
	pCtx->idxEnv->setSkipZeroByteFile( gStreamSmithConfig.embededContenStoreCfg.ctntAttr.skipZeroByteFiles >= 1 );

	//pCtx->idxEnv->setUseVstrmIndexParseAPI( true );
	adjustIngorePrefix(ZQ::StreamSmith::NCSBridge::getVstrmHandle());

	pCtx->cpWrapper		= NULL;
	
	if( GAPPLICATIONCONFIGURATION.embededContenStoreCfg.csEnableCpc >= 1 )
	{
		pCtx->cpWrapper	= new ContentProvisionWrapper(MOLOG);
	}
	
	fileSystemEventSinker = new vstrmFileSystemSink(store,pCtx,pCtx->bSupportNpVr);
	
	deleteLaterProcedure  = new DeleteLaterProcudure( store._thpool , pCtx , store );

#ifdef SUPPORT_VSTRM_VSIS_EVENT
	vsisEventSinker		  = new VsisEventSinker(store,pCtx);
#endif//SUPPORT_VSTRM_VSIS_EVENT

	store._ctxPortal =  reinterpret_cast<void*>(pCtx);

	if ( NULL == store._ctxPortal )
		return;
	fileSystemEventSinker->attchVstrmHandle( ZQ::StreamSmith::NCSBridge::getVstrmHandle() );
	fileSystemEventSinker->start();
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
#ifdef SUPPORT_VSTRM_VSIS_EVENT
	if( vsisEventSinker)
	{
		vsisEventSinker->start();
	}
#endif//SUPPORT_VSTRM_VSIS_EVENT

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
	if( deleteLaterProcedure )
	{
		deleteLaterProcedure->stop( );		
	}

	if ( NULL != store._ctxPortal )
	{
		PortalCtx* pCtx = (PortalCtx*)(store._ctxPortal);
		vstrmFileSystemSink* pFilesystemSink = fileSystemEventSinker;
		pFilesystemSink->stop( );
		delete pFilesystemSink;
		pFilesystemSink = NULL;
		fileSystemEventSinker = NULL;

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
#ifdef SUPPORT_VSTRM_VSIS_EVENT
	if( vsisEventSinker )
	{
		vsisEventSinker->stop();
		delete vsisEventSinker;
	}
#endif//SUPPORT_VSTRM_VSIS_EVENT

	store._ctxPortal = NULL;
}

bool ContentStoreImpl::createPathOfVolume(ContentStoreImpl& store, const std::string& pathOfVolume,const std::string& volname)
{
	return vstrmCSPortalI( store._ctxPortal , store  ).createPathOfVolume(pathOfVolume,volname);
}

bool ContentStoreImpl::deletePathOfVolume(ContentStoreImpl& store, const std::string& pathOfVolume)
{
	return vstrmCSPortalI( store._ctxPortal , store  ).deletePathOfVolume(pathOfVolume);
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
	return vstrmCSPortalI( store._ctxPortal , store  ).deleteFileByContent( content, mainFilePathname );
}


bool ContentStoreImpl::populateAttrsFromFile(ContentStoreImpl& store,
	ContentImpl& content, 
	const ::std::string& contentName )
{
	return vstrmCSPortalI( store._ctxPortal , store , &content ).populateAttrsFromFile( content, contentName );
}


std::string ContentStoreImpl::memberFileNameToContentName(ContentStoreImpl& store, const std::string& memberFilename)
{
	return vstrmCSPortalI( store._ctxPortal , store ).memberFileNameToContentName( memberFilename );
}

uint64 ContentStoreImpl::checkResidentialStatus(ContentStoreImpl& store, uint64 flagsToTest, 
	ContentImpl::Ptr pContent, const ::std::string& contentFullName, const ::std::string& mainFilePathname)
{
	return vstrmCSPortalI( store._ctxPortal , store , pContent ).checkResidentialStatus( flagsToTest , pContent , contentFullName , mainFilePathname );
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
	::TianShanIce::Properties metaDatas = content.getMetaData(Ice::Current());
	{		
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
		metaDatas,
		bNPVRSession);
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


std::string ContentStoreImpl::getExportURL(ContentStoreImpl& store,
												ContentImpl& content, 
												const ::TianShanIce::ContentProvision::ProvisionContentKey& contentkey, 
												const ::std::string& transferProtocol, 
												::Ice::Int transferBitrate,
												::Ice::Int& ttl,
												::TianShanIce::Properties& params)
// std::string ContentStoreImpl::getExportURL(ContentStoreImpl& store, ContentImpl& content,
// 	const std::string& contentName,
// 	const ::std::string& transferProtocol,
// 	::Ice::Int transferBitrate, 
// 	::Ice::Int& ttl, 
// 	::TianShanIce::Properties& params)
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
	std::string strExposeUrl = cpWrapper->getExposeUrl(transferProtocol, contentkey, transBitrate, nTTL, permittedBitrate);

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
		contentkey.content.c_str(), strExposeUrl.c_str(), ttl, stStart.c_str(), stEnd.c_str(), permittedBitrate);

	return strExposeUrl;
}

void ContentStoreImpl::cancelProvision(ContentStoreImpl& store, ContentImpl& content, const ::std::string& provisionTaskPrx)
{
	if( GAPPLICATIONCONFIGURATION.embededContenStoreCfg.csEnableCpc < 1 )
	{
		MOLOG( ZQ::common::Log::L_WARNING , CLOGFMT( ContentStoreImpl , "Cpc is disabled" ) );
		return ;
	}

	ContentProvisionWrapper::Ptr& cpWrapper = ((PortalCtx*)store._ctxPortal)->cpWrapper;

	std::string contentName = content._name();
	cpWrapper->cancelProvision(contentName, provisionTaskPrx);
}

}}//namespace ZQTianShan::ContentStore

namespace ZQTianShan
{
namespace ContentStore
{
vstrmFileSystemSink::vstrmFileSystemSink( ZQTianShan::ContentStore::ContentStoreImpl& st ,  PortalCtx* ctx ,bool supportNpvr )
	:store(st),
	_portCtx(ctx),
	_bSupportNpvr(supportNpvr)
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

// 	static size_t iPrefixLength = gStreamSmithConfig.embededContenStoreCfg.ctntAttr.ignoreFileEventPrefix.length();
// 
// 	const std::string& strNamePrefix = gStreamSmithConfig.embededContenStoreCfg.ctntAttr.ignoreFileEventPrefix;
// 	const std::string& strNameInvalidChar = gStreamSmithConfig.embededContenStoreCfg.ctntAttr.ignoreInvalidCharacter;

	::TianShanIce::Properties params;

	vstrmCSPortalI dummyPortal(NULL,store,NULL);

	do
	{
		pNewFile = newFileName;
		pOldFile = oldFileName;

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
					std::string fileName = helperFixupPathname( pNewFile , _bSupportNpvr );
					if( 1 /*!dummyPortal.ignorableItem(fileName)*/ )
					{
						store.OnFileEvent(	TianShanIce::Storage::fseFileCreated, fileName, params ,Ice::Current() );
					}
					else
					{
						envlog(ZQ::common::Log::L_INFO,CLOGFMT(vstrmFileSystemSink,	"[FILEEVENT] File [%s] is ignored with File NEW event"),
							pNewFile);
					}
				}
				break;

			case VSTRM_FILE_CREATED:
				{							
					envlog(ZQ::common::Log::L_INFO , CLOGFMT(vstrmFileSystemSink , "[FILEEVENT]   File CREATED: %s"), pNewFile );
					//_monitor.onFileCreated( pNewName );
					std::string fileName = helperFixupPathname( pNewFile , _bSupportNpvr );
					if( 1 /*!dummyPortal.ignorableItem(fileName)*/ )
					{
						store.OnFileEvent(	TianShanIce::Storage::fseFileModified, fileName, params , Ice::Current() );
					}
					else
					{
						envlog(ZQ::common::Log::L_INFO,CLOGFMT(vstrmFileSystemSink,	"[FILEEVENT] File [%s] is ignored with File CREATED event"),
							pNewFile);
					}
				}
				break;

			case VSTRM_FILE_MODIFIED:			
				{
					//printf( "File MODIFIED: %s\n", newFileName );
					envlog(ZQ::common::Log::L_INFO , CLOGFMT(vstrmFileSystemSink , "[FILEEVENT]   File MODIFIED: %s"), pNewFile  );
					std::string fileName = helperFixupPathname( pNewFile , _bSupportNpvr );
// 					if( !dummyPortal.ignorableItem(fileName) )
// 					{
						store.OnFileEvent(	TianShanIce::Storage::fseFileModified, fileName, params , Ice::Current() );
// 					}
// 					else
// 					{
// 						envlog(ZQ::common::Log::L_INFO,CLOGFMT(vstrmFileSystemSink,
// 							"[FILEEVENT] File [%s] is ignored with File MODIFIED event"),
// 							pNewFile);
// 					}
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
					std::string fileName = helperFixupPathname( pOldFile , _bSupportNpvr );
					vstrmCSPortalI portal(_portCtx , store , NULL );
					if( portal.isLeadSessionFile(fileName) )
					{
						if( !portal.isSubFile(fileName))
						{
							envlog(ZQ::common::Log::L_WARNING,
								CLOGFMT(vstrmFileSystemSink,"[FILEEVENT]   File DELETED: file [%s] is a lead session file and not a index file , just ignore it"),
								fileName.c_str() );
							break;
						}
					}
					store.OnFileEvent(	TianShanIce::Storage::fseFileDeleted, fileName, params , Ice::Current() );
				}
				break;

			case VSTRM_FILE_RENAMED:
				{
					//printf( "File RENAMED: %s --> %s\n",oldFileName, newFileName );
					envlog(ZQ::common::Log::L_INFO , CLOGFMT(vstrmFileSystemSink , "[FILEEVENT]   File RENAMED:  %s --> %s "),
						pOldFile , pNewFile );
					params["newFilename"] = helperFixupPathname( pNewFile , _bSupportNpvr );
					
					std::string oldFileName =helperFixupPathname( pOldFile , _bSupportNpvr );

					store.OnFileEvent(	TianShanIce::Storage::fseFileRenamed, oldFileName, params,Ice::Current());
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

//////////////////////////////////////////////////////////////////////////
#ifdef SUPPORT_VSTRM_VSIS_EVENT

VsisEventSinker::VsisEventSinker( ZQTianShan::ContentStore::ContentStoreImpl& st , PortalCtx* ctx )
:store(st),mPortCtx(ctx),
mbQuit(false),mVstrmHandle(INVALID_HANDLE_VALUE),
mMultiReceiver(st)
{
	
}
VsisEventSinker::~VsisEventSinker()
{
	if(mVstrmHandle != INVALID_HANDLE_VALUE )
	{
		VstrmClassCloseEx(mVstrmHandle);
	}
	
}

#define IMPORTNODEID "sys.ImportEdgeNode"

void VsisEventSinker::stop()
{
	if( gStreamSmithConfig.embededContenStoreCfg.vsisEventConf.enable > 0 )
	{
		mMultiReceiver.stop();
	}
}
bool VsisEventSinker::start()
{
	if( gStreamSmithConfig.embededContenStoreCfg.vsisEventConf.enable > 0 )
	{
		std::ostringstream ossClusterId;
		ossClusterId<<gStreamSmithConfig.mediaClusterId;

		mMultiReceiver.start(	gStreamSmithConfig.szServiceID , 
			ossClusterId.str(), 
			gStreamSmithConfig.embededContenStoreCfg.vsisEventConf.localIp,
			gStreamSmithConfig.embededContenStoreCfg.vsisEventConf.groupIp,
			gStreamSmithConfig.embededContenStoreCfg.vsisEventConf.groupPort);
	}
	return ZQ::common::NativeThread::start();
}
int VsisEventSinker::run()
{
	VSTATUS					vStatus;
	PUINT8					bufAddr;
	PUINT8					bufEnd;
	PVS_TLV					pVsTlv;
	PVSIS_USER_EVENT_TLV	pVSISEvent					= NULL;
	HANDLE	vsisEventHandle								= INVALID_HANDLE_VALUE;
	VSTRM_VSIS_EVENT		eventType					= VSTRM_VSIS_EVENT_LOST;
	PCHAR					assetName					= "Unknown";
	USHORT					isPWE						= 0;
	BOOL					bOK;
	vStatus = VstrmClassOpenEx( &mVstrmHandle );
	if (vStatus != VSTRM_SUCCESS)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(VsisEventSinker,"failed to open vstrm handle") );
		return -1;
	}
	pVSISEvent = (PVSIS_USER_EVENT_TLV) malloc (sizeof(VSIS_USER_EVENT_TLV) + VSIS_EVENT_MAX_TLV_BUFFER_SIZE);
	if(!pVSISEvent)
	{
		VstrmClassCloseEx(mVstrmHandle);
		mVstrmHandle = INVALID_HANDLE_VALUE;
		envlog(ZQ::common::Log::L_ERROR,CLOGFMT(VsisEventSinker,"not enough memeory to hold VSISEvent"));
		return -1;
	}

	pVSISEvent->tlvBufferLength = VSIS_EVENT_MAX_TLV_BUFFER_SIZE;

	envlog(ZQ::common::Log::L_INFO,CLOGFMT(VsisEventSinker,"sinker started"));
	// Get the first VSIS event.  This routine blocks until a vsis event occurs.
	vsisEventHandle = VstrmFindFirstVsisNotification(mVstrmHandle,(PVSIS_USER_EVENT_TLV)pVSISEvent);
	if (vsisEventHandle == INVALID_HANDLE_VALUE)
	{
		envlog(ZQ::common::Log::L_ERROR,CLOGFMT(VsisEventSinker,"failed to get VSIS event"));
		return -1;
	}

	Ice::Current dummyIc;

	while( !mbQuit )
	{
		// Get the "VSIS Event" block TLV parameters 
		bufAddr = (PUINT8)&pVSISEvent->tlvBuffer;
		bufEnd = bufAddr + pVSISEvent->tlvBufferLength;
		// Locate first TLV within the block
		pVsTlv = (PVS_TLV) bufAddr;
		
		// Process TLVs till end of "VSIS Event" block
		while ((pVsTlv) && ((pVsTlv->tag & VS_TAG_BLOCK_ID_MASK) == VS_TAG_BLOCK_ID_VSIS_EVENT))
		{
			switch (pVsTlv->tag)
			{
			case VS_TAG_VSIS_EVENT_TYPE:					eventType					= (VSTRM_VSIS_EVENT) VsGetTlvInteger(pVsTlv);	break;
			case VS_TAG_VSIS_ASSET_NAME:					assetName					= (PCHAR)			 pVsTlv->value.vText; 		break;
			case VS_TAG_VSIS_ASSET_PWE:						isPWE						= (USHORT)			 VsGetTlvInteger(pVsTlv);	break;
			}			
			pVsTlv = VsLocateNextTlv(pVsTlv, bufEnd); // Locate next TLV
		}
		switch(eventType)
		{
		case VSTRM_VSIS_EVENT_LOST:
			{
				envlog(ZQ::common::Log::L_INFO,CLOGFMT(VsisEventSinker,"got VSTRM_VSIS_EVENT_LOST"));
			}
			break;

		case VSTRM_VSIS_IMPORT_STARTED:
			{				
				VsisEventMultiReceiver::VsisEventInfo info;
				info.assetName	= assetName;
				info.vsisEvent	= VSTRM_VSIS_IMPORT_STARTED;
				mMultiReceiver.onEvent( info );
			}
			break;

		case VSTRM_VSIS_MAIN_ASSET_COMPLETE:
			{
				VsisEventMultiReceiver::VsisEventInfo info;
				info.assetName	= assetName;
				info.vsisEvent	= VSTRM_VSIS_MAIN_ASSET_COMPLETE;
				mMultiReceiver.onEvent( info );
			}
			break;

		case VSTRM_VSIS_ASSET_TRICKS_COMPLETE:
			{
				VsisEventMultiReceiver::VsisEventInfo info;
				info.assetName	= assetName;
				info.vsisEvent	= VSTRM_VSIS_ASSET_TRICKS_COMPLETE;
				mMultiReceiver.onEvent( info );
			}
			break;

		case VSTRM_VSIS_DELETE_ASSET:
			{
				VsisEventMultiReceiver::VsisEventInfo info;
				info.assetName	= assetName;
				info.vsisEvent	= VSTRM_VSIS_DELETE_ASSET;
				mMultiReceiver.onEvent( info );
				
			}
			break;
		default:
			{
				envlog(ZQ::common::Log::L_INFO,CLOGFMT(VsisEventSinker,"got unkown event"));
			}
			break;
		}

		eventType = VSTRM_VSIS_EVENT_LOST;
		assetName = "Unknown";
		isPWE = 0;
		bOK = VstrmFindNextVsisNotification(mVstrmHandle, vsisEventHandle, (PVSIS_USER_EVENT_TLV)pVSISEvent);
		if (!bOK)
		{
			envlog(ZQ::common::Log::L_ERROR,CLOGFMT(VsisEventSinker,"failed to find next vsis event"));
			break;
		}
	}
	VstrmFindCloseVsisNotification(mVstrmHandle, vsisEventHandle);
	envlog(ZQ::common::Log::L_INFO,CLOGFMT(VsisEventSinker,"sinker quiting ..."));
	return 0;
}

//////////////////////////////////////////////////////////////////////////
///VsisEventMultiReceiver
VsisEventMultiReceiver::VsisEventInfo::VsisEventInfo()
{
	vsisEvent = VSTRM_VSIS_EVENT_LOST;
}

size_t VsisEventMultiReceiver::VsisEventInfo::dataSize( ) const
{
	return nodeNetId.length() + 2 + clusterId.length() + 2 + assetName.length() + 2 + sizeof(int) + 2;
}
#define VSIS_EVENT_MAGIC_TAG "VsIsEvent"

bool VsisEventMultiReceiver::VsisEventInfo::toBuffer( char* buffer , size_t& bufSize ) const
{
	if( bufSize < dataSize() )
		return false;
	static std::string mgaicTag = VSIS_EVENT_MAGIC_TAG;
	ZQTianShan::Util::BufferMarshal m;
	m<< mgaicTag << nodeNetId << clusterId << assetName << (int)vsisEvent;
	size_t sz = 0 ;
	const char* p = (const char*)m.getBuffer( sz );
	if( !p )
		return false;
	memcpy( buffer , p , MIN(bufSize,sz) );
	return (MIN(bufSize,sz)) > 0 ;
}

bool VsisEventMultiReceiver::VsisEventInfo::fromBuffer( const char* buffer , size_t bufSize )
{
	ZQTianShan::Util::BufferMarshal m;
	
	m.attachBuffer((void*)buffer,bufSize);
	int eventType;
	m>> magic>> nodeNetId>> clusterId>> assetName>> eventType;
	
	if( magic != std::string(VSIS_EVENT_MAGIC_TAG) )
		return false;

	vsisEvent = ( VSTRM_VSIS_EVENT )eventType;
	return true;
}

VsisEventMultiReceiver::VsisEventMultiReceiver( ZQTianShan::ContentStore::ContentStoreImpl& s )
:store(s),
mReceiver(NULL),
mMulticaster(NULL)
{
}
VsisEventMultiReceiver::~VsisEventMultiReceiver()
{
	clearResource();
}
void VsisEventMultiReceiver::onEvent( VsisEventInfo& info )
{
	info.clusterId	= mClusterId;
	info.nodeNetId	= mNodeNetId;
	
	dispatchEvent( info );

	if( mMulticaster && mReceiver )
		multicastEvent( info );
}

void VsisEventMultiReceiver::clearResource( )
{
	if( mReceiver )
	{
		delete mMulticaster;
		mMulticaster = NULL;
	}
	if( mReceiver )
	{
		delete mReceiver;
		mReceiver = NULL;
	}
}

bool VsisEventMultiReceiver::start( const std::string& nodeNetId , const std::string clusterId , const std::string& localIp , const std::string& groupIp , int groupPort)
{
	clearResource();
	if( groupIp.empty() || localIp.empty() )
	{
		envlog(ZQ::common::Log::L_ERROR,CLOGFMT(VsisEventMultiReceiver,"invalid group or local address"));
		return false;
	}
	
	ZQ::common::InetMcastAddress groupAddress( groupIp.c_str() );	
	ZQ::common::InetAddress localAddress(localIp.c_str());
	ZQ::common::InetAddress localAddress1( "0.0.0.0");

	mReceiver = new ZQ::common::UDPReceive( localAddress1 , groupPort );
	mReceiver->setMulticast(true);

	ZQ::common::Socket::Error err = mReceiver->join( groupAddress );
	mReceiver->setCompletion(true); // make the socket block-able

	if (err != ZQ::common::Socket::errSuccess)	
	{
		envlog(ZQ::common::Log::L_ERROR,CLOGFMT(VsisEventMultiReceiver,"failed to crate multicast receive for groupAddress[%s:%d]"),groupIp.c_str() , groupPort );
		return false;
	}

	mMulticaster = new ZQ::common::UDPMulticast( localAddress , 0 );
	mMulticaster->setGroup( groupAddress, groupPort );
	mMulticaster->setTimeToLive(10);
	if( !mMulticaster->isActive() )
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(VsisEventMultiReceiver, "initialze sender failed: group:[%s]:%d local[%s]"), 
			groupIp.c_str(), groupPort ,localIp.c_str() );
		return false;
	}
	
	envlog(ZQ::common::Log::L_INFO,CLOGFMT(VsisEventMultiReceiver,"start multiRecevier: localIp[%s] groupIp[%s] groupPort[%d]"),
		localIp.c_str() , groupIp.c_str() , groupPort );

	mNodeNetId	= nodeNetId;
	mClusterId	= clusterId;
	mbQuit = false;

	return ZQ::common::NativeThread::start();

}

void VsisEventMultiReceiver::stop( )
{
	mbQuit = true;
	VsisEventInfo info;
	info.vsisEvent = VSTRM_VSIS_EVENT_LOST;
	multicastEvent(info);
	waitHandle(10000);
}

int VsisEventMultiReceiver::run()
{
	char buffer[1024];
	while(!mbQuit)
	{
		try 
		{
			ZQ::common::InetHostAddress from;
			int sport;
			size_t sz = sizeof(buffer);
			int len = mReceiver->receiveFrom( buffer , sz , from , sport );
			VsisEventInfo msg;
			if( msg.fromBuffer(buffer , len ) )
			{
				if( msg.clusterId == mClusterId && msg.nodeNetId != mNodeNetId )
					dispatchEvent( msg , true );
			}
		}
		catch(...) {}
	}
	return 0;
}
void VsisEventMultiReceiver::dispatchEvent( const VsisEventInfo& info , bool bMulticast )
{
	std::string tag = bMulticast ? (std::string("MULTICAST:")+info.nodeNetId) : std::string("VSIS");

	const char* assetName = info.assetName.c_str();

	Ice::Current dummyIc;
	switch(info.vsisEvent)
	{
	case VSTRM_VSIS_IMPORT_STARTED:
		{				
			envlog(ZQ::common::Log::L_INFO,CLOGFMT(VsisEventSinker,"got VSTRM_VSIS_IMPORT_STARTED from [%s], asset[%s] "), tag.c_str() , assetName );
			TianShanIce::Properties props;ZQTianShan::Util::updatePropertyData(props, IMPORTNODEID , info.nodeNetId )	;
			store.OnFileEvent( TianShanIce::Storage::fseFileCreated , assetName , props , dummyIc );
		}
		break;

	case VSTRM_VSIS_MAIN_ASSET_COMPLETE:
		{
			envlog(ZQ::common::Log::L_INFO,CLOGFMT(VsisEventSinker,"got VSTRM_VSIS_MAIN_ASSET_COMPLETE from [%s], asset[%s]"), tag.c_str() , assetName );
			TianShanIce::Properties props;ZQTianShan::Util::updatePropertyData(props, IMPORTNODEID , info.nodeNetId )	;
			store.OnFileEvent( TianShanIce::Storage::fseFileModified , assetName , props , dummyIc );
		}
		break;

	case VSTRM_VSIS_ASSET_TRICKS_COMPLETE:
		{
			envlog(ZQ::common::Log::L_INFO,CLOGFMT(VsisEventSinker,"got VSTRM_VSIS_ASSET_TRICKS_COMPLETE from [%s], asset[%s]"), tag.c_str() , assetName );
			TianShanIce::Properties props;ZQTianShan::Util::updatePropertyData(props, IMPORTNODEID , info.nodeNetId )	;
			store.OnFileEvent( TianShanIce::Storage::fseFileModified , assetName , props , dummyIc );
		}
		break;

	case VSTRM_VSIS_DELETE_ASSET:
		{
			envlog(ZQ::common::Log::L_INFO,CLOGFMT(VsisEventSinker,"got VSTRM_VSIS_DELETE_ASSET from [%s], asset[%s]"), tag.c_str() , assetName );
			TianShanIce::Properties props;ZQTianShan::Util::updatePropertyData(props, IMPORTNODEID , info.nodeNetId )	;
			store.OnFileEvent( TianShanIce::Storage::fseFileDeleted , assetName , props , dummyIc );
		}
		break;
	default:
		break;
	}
}

void VsisEventMultiReceiver::multicastEvent( const VsisEventInfo& info )
{
	char buffer[1024];
	size_t sz = sizeof(buffer);
	if( info.toBuffer(buffer,sz) && mMulticaster )
	{
		size_t ret = mMulticaster->send(buffer,sz);
		if( ret != sz )
		{
			envlog(ZQ::common::Log::L_ERROR,CLOGFMT(VsisEventMultiReceiver,"failed to multicast event"));
		}
	}
}
#endif//SUPPORT_VSTRM_VSIS_EVENT

//////////////////////////////////////////////////////////////////////////
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
	_store = NULL;
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

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
///
void	addContentToLaterDeleteProcudure( const std::string& contentName )
{
	assert( deleteLaterProcedure != NULL );
	deleteLaterProcedure->pushContent(contentName);
}

DeleteLaterProcudure::DeleteLaterProcudure( ZQ::common::NativeThreadPool& pool ,PortalCtx* ctx ,ContentStoreImpl& s)
:ZQ::common::ThreadRequest(pool),
store(s)
{
	mbQuit		=	false;
	portalCtx	=	ctx;
	assert( ctx != NULL );
	mbRunning	=	false;
	mExecCount	=	0;
}
DeleteLaterProcudure::~DeleteLaterProcudure()
{
}

void DeleteLaterProcudure::pushContent( const std::string& contentName )
{
	bool bStart = false;
	{
		ZQ::common::MutexGuard gd( mMutex );
		CONTENTLIST::const_iterator it = mCtntList.begin();
		for( ; it != mCtntList.end(); it ++ )
		{
			if( *it == contentName )
				return;
		}
		mCtntList.push_front(contentName);
		bStart = ( mCtntList.size() == 1 && !mbRunning );
	}
	if( bStart )
		start();
}

std::string	DeleteLaterProcudure::extractFileName(  const std::string& fileName ) const
{
	std::string strResult;
	std::string::size_type bpos = fileName.find_last_of("\\/");
	if( bpos != std::string::npos )
	{
		strResult = fileName.substr( bpos + 1 );
	}
	else
	{
		strResult = fileName;
	}
	return strResult;
}
bool DeleteLaterProcudure::init( )
{
	ZQ::common::MutexGuard gd(mMutex);
	mbRunning = true;
	mExecCount++;
	return true;
}
void DeleteLaterProcudure::final(int retcode , bool bCancelled )
{
	mCurContent = "";
	bool bStart = false;
	if(!mbQuit)
	{
		{
			ZQ::common::MutexGuard gd(mMutex);
			if( mCtntList.size() > 0 )
			{
				bStart = true;
			}
			else
			{
				mbRunning = false;
				mExecCount--;
			}
		}
		if( bStart )
		{
			start();
		}		
	}
	else
	{
		ZQ::common::MutexGuard gd(mMutex);
		mbRunning = false;
		mExecCount--;
	}
}

int DeleteLaterProcudure::run( )
{	
	{
		ZQ::common::MutexGuard gd(mMutex);
		if( mCtntList.size() > 0 )
		{
			mCurContent = mCtntList.back();
			mCtntList.pop_back();
		}
		else
		{
			mCurContent = "";
		}
		envlog(ZQ::common::Log::L_DEBUG,CLOGFMT(DeleteLaterProcudure,"current runing task with count[%d]"),
			mExecCount);
	}

	if (mbQuit || mCurContent.empty() )		
	{		
		return 1;
	}	
	vstrmCSPortalI portal(portalCtx , store , NULL );
	if ( portal.isLeadSessionFile(mCurContent) )
	{
		std::string contentName = extractFileName(mCurContent);
		PortalCtx::VolumeInfoS vols;
		{
			ZQ::common::MutexGuard gd(portalCtx->mVolumeInfoMutex);
			vols = portalCtx->mVolumes;
		}
		PortalCtx::VolumeInfoS::const_iterator it = vols.begin() ; 
		for( ; it != vols.end() ; it ++ )
		{
			std::string fullName = it->path + contentName + "_*";
			envlog(ZQ::common::Log::L_INFO,
				CLOGFMT(DeleteLaterProcudure,"check virtual session [%s] "),
				fullName.c_str()	);		
			if( portal.checkFileExistence(fullName) )
			{
				envlog(ZQ::common::Log::L_INFO,
					CLOGFMT(DeleteLaterProcudure,"there are virtual session associated with lead session [%s] , do not delete it"),
					mCurContent.c_str()	);				
				return 1;
			}
		}
	}
	envlog(ZQ::common::Log::L_INFO,CLOGFMT(DeleteLaterProcudure,"no session associated with [%s] , delete its index file"),
		mCurContent.c_str() );

	std::vector<std::string> files;
	
	portal.getFileSetFromFS( mCurContent , "" , false , files );
	if( files.size() == 0 )
	{
		portal.findFiles(mCurContent+".ff*",files);
		portal.findFiles(mCurContent+".fr*",files);
		portal.findFiles(mCurContent+".vv*",files);
		portal.findFiles(mCurContent+".idx",files);
	}
	files.push_back(mCurContent);
	{
		std::vector<std::string>::const_iterator itFile = files.begin();
		for( ; itFile != files.end() ; itFile ++ )
		{	
			if( portal.deleteVstrmFile( *itFile ) )
			{
				envlog(ZQ::common::Log::L_INFO,
					CLOGFMT(DeleteLaterProcudure,"delete file [%s]"),
					itFile->c_str() );
			}
		}
	}
	
	
	return 1;
}

void	DeleteLaterProcudure::stop( )
{
#pragma message(__MSGLOC__"TODO:ill design")
	bool bNeedDelete = false;
	{
		ZQ::common::MutexGuard gd(mMutex);
		if( mbRunning )
			mbQuit = true;
		else
		{
			bNeedDelete = true;			
		}
	}
	if(bNeedDelete)
		delete this;
}

}}//namespace ZQ::StreamSmith
