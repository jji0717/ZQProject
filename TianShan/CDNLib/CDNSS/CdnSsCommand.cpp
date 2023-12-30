
#include <boost/thread.hpp>
#include <ContentSysMD.h>
#include <TianShanIceHelper.h>
#include <C2Locator.h>
#include "CdnSSConfig.h"
#include <assert.h>
#include "../CDNDefines.h"
#include <C2StreamerLib.h>
#include <strHelper.h>
#include <urlstr.h>
#include <ContentUser.h>
#include <C2StreamerEnv.h>
#include "CdnStreamerManager.h"
#include "CdnSsCommand.h"

#if defined ZQ_OS_MSWIN
#define	PLFMT(x,y) 	"%s/%08X/CdnSSCommand[%s]\t"##y,mStreamCtx.id().c_str(),GetCurrentThreadId(),#x	
#elif defined ZQ_OS_LINUX
#define	PLFMT(x,y) 	"%s/%08X/CdnSSCommand[%s]\t"y,mStreamCtx.id().c_str(),(unsigned int)pthread_self(),#x
#endif	
using namespace C2Streamer;
const std::string KEY_ITEM_FULL_PATH_NAME		= "reserved.ItemFullPathName";
const std::string KEY_ITEM_ALL_EXT_NAMES		= "reserved.ItemAllExtNames";

namespace ZQ
{
namespace StreamService
{

bool	SsServiceImpl::listAllReplicas( SsServiceImpl& ss, OUT SsReplicaInfoS& infos )
{
	CdnSsEnvironment* cdnEnv = dynamic_cast<CdnSsEnvironment*>(getSsEnv());
	assert( cdnEnv != NULL );
	CdnStreamerManager& manager = cdnEnv->getStreamerManager();
	return manager.listStreamer(infos);
}

bool SsServiceImpl::allocateStreamResource(	SsServiceImpl& ss, 
										IN const std::string& streamerReplicaId ,
										IN const std::string& portId ,
										IN const TianShanIce::SRM::ResourceMap&	resource )
{
	//actually , nothing should be done
	return true;
}


bool SsServiceImpl::releaseStreamResource( SsServiceImpl& ss,
											SsContext& ctx,
										IN const std::string& streamerReplicaId,
										 TianShanIce::Properties& props)
{
	//actually , nothing should be done
	return true;
}

int32	SsServiceImpl::doValidateItem( SsServiceImpl& ss, 
										IN SsContext& ctx, 
										INOUT TianShanIce::Streamer::PlaylistItemSetupInfo& info )
{
	CdnSsEnvironment* cdnEnv = dynamic_cast<CdnSsEnvironment*>(getSsEnv());
	assert( cdnEnv != NULL );
//	CdnStreamerManager& manager = cdnEnv->getStreamerManager();
	CdnSsCommand command(cdnEnv,ss,ctx);
	//CDN_EXTENSIONNAME
	int32 ret = command.doValidateItem(info);
	if( ret != ERR_RETURN_SUCCESS )
	{
		std::string extName;
		ZQTianShan::Util::getValueMapDataWithDefault(info.privateData,CDN_EXTENSIONNAME,"",extName);
		info.contentName = info.contentName + extName;
		ZQTianShan::Util::updateValueMapData(info.privateData,CDN_EXTENSIONNAME,"");
		return command.doValidateItem( info );
	}
	else
	{
		return ret;
	}	
}

int32 SsServiceImpl::doCommit( SsServiceImpl& ss, 
								IN SsContext& ctx, 
								IN PlaylistItemSetupInfos& items  ,
								INOUT TianShanIce::SRM::ResourceMap& crResource)
								
{
	//create a stream
	CdnSsEnvironment* cdnEnv = dynamic_cast<CdnSsEnvironment*>(getSsEnv());
	assert( cdnEnv != NULL );
	CdnSsCommand command(cdnEnv,ss,ctx);
	return command.doCommit( items , crResource );
}


int32 SsServiceImpl::doLoad(	SsServiceImpl& ss,
								SsContext& ctx,
								IN const TianShanIce::Streamer::PlaylistItemSetupInfo& itemInfo, 
								IN int64 timeoffset,
								IN float scale,								
								INOUT StreamParams& ctrlParams,
								OUT std::string&	streamId )
{
 	CdnSsEnvironment* cdnEnv = dynamic_cast<CdnSsEnvironment*>(getSsEnv());
 	assert( cdnEnv != NULL );
 //	CdnStreamerManager& manager = cdnEnv->getStreamerManager();
 	CdnSsCommand command(cdnEnv,ss,ctx);
 	return command.doLoad(itemInfo ,streamId );		
}

int32 SsServiceImpl::doPlay(	 SsServiceImpl& ss ,
								SsContext& ctx,								
								IN const std::string& streamId,
								IN int64 timeOffset,
								IN float scale,
								INOUT StreamParams& ctrlParams )
{	
	return ERR_RETURN_SUCCESS;
}


int32 SsServiceImpl::doPause(	 SsServiceImpl& ss ,
								SsContext& ctx,								
								IN const std::string& streamId , 						
								INOUT StreamParams& ctrlParams )
{
	return ERR_RETURN_NOT_SUPPORT;
}


int32 SsServiceImpl::doResume(  SsServiceImpl& ss ,
								SsContext& ctx,								
								IN const std::string& streamId ,						
								INOUT StreamParams& ctrlParams )
{
	return ERR_RETURN_NOT_SUPPORT;
}

int32 SsServiceImpl::doReposition( SsServiceImpl& ss ,
								SsContext& ctx,								
								IN const std::string& streamId ,
								IN int64 timeOffset,
								IN const float& scale,
								INOUT StreamParams& ctrlParams )
{
	return ERR_RETURN_NOT_SUPPORT;
}

int32 SsServiceImpl::doChangeScale(	 SsServiceImpl& ss ,
										SsContext& ctx,										
										IN const std::string& streamId ,								
										IN float newScale,
										INOUT StreamParams& ctrlParams )

{
	return ERR_RETURN_NOT_SUPPORT;
}

int32 SsServiceImpl::doDestroy(	 SsServiceImpl& ss ,
								IN SsContext& ctx,
								IN const std::string& streamId)
{
	CdnSsEnvironment* cdnEnv = dynamic_cast<CdnSsEnvironment*>(getSsEnv());
	assert( cdnEnv != NULL );
//	CdnStreamerManager& manager = cdnEnv->getStreamerManager();
	CdnSsCommand command(cdnEnv,ss,ctx);
	return command.doDestroy(streamId,"");
}

int32 SsServiceImpl::doGetStreamAttr(	 SsServiceImpl& ss ,
									IN	SsContext& ctx,
									IN	const std::string& streamId,
									IN  const TianShanIce::Streamer::PlaylistItemSetupInfo& info,
									OUT StreamParams& ctrlParams )
{

	return ERR_RETURN_SUCCESS;
}



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//CdnSsCommand



CdnSsCommand::CdnSsCommand(CdnSsEnvironment * environment , SsServiceImpl& serviceImpl , SsContext& contextKey)
:ss(serviceImpl), mStreamCtx(contextKey), env(environment),
mStreamerManager(environment->getStreamerManager())
{
}

CdnSsCommand::~CdnSsCommand()
{

}



int32 CdnSsCommand::doLoad( const TianShanIce::Streamer::PlaylistItemSetupInfo& info ,
							  std::string& sessId )
{
	
	Ice::Long		timeout = 0;

	const TianShanIce::Properties& plParams = mStreamCtx.getContextProperty();			
	ZQTianShan::Util::getPropertyData ( plParams , CDN_TRANSFERTIMEOUT , timeout );

	try
	{
		ZQTianShan::Util::getPropertyData ( plParams , CDN_TRANSFERID , sessId );
	}
	catch( const TianShanIce::InvalidParameter&)
	{
		ENVLOG(ZQ::common::Log::L_ERROR, PLFMT(doLoad,"no session id is found"));
		return ERR_RETURN_SERVER_ERROR;
	}

	try
	{
		TianShanIce::SCS::TransferSessionPrx transferSess = TianShanIce::SCS::TransferSessionPrx::uncheckedCast( ss.getPathTicket(mStreamCtx.id()));
		if( transferSess)
		{	
			std::string		openForWrite;
			std::string		availRange;
			std::string		transferPortNum;		

			
			ZQTianShan::Util::getPropertyData ( plParams , CDN_AVAILRANGE , availRange );
			ZQTianShan::Util::getPropertyData ( plParams , CDN_OPENFORWRITE , openForWrite );
			ZQTianShan::Util::getPropertyData ( plParams , CDN_TRANSFERPORTNUM, transferPortNum);
			
			C2Streamer::updateSessionTimer(sessId,timeout);

			TianShanIce::ValueMap values;
			ZQTianShan::Util::updateValueMapData( values , CDN_TRANSFERID, sessId );
			ZQTianShan::Util::updateValueMapData( values , CDN_TRANSFERTIMEOUT , timeout );
			ZQTianShan::Util::updateValueMapData( values , CDN_AVAILRANGE , availRange);
			ZQTianShan::Util::updateValueMapData( values , CDN_OPENFORWRITE , openForWrite );
			//ZQTianShan::Util::updateValueMapData( values , CDN_TRANSFERPORTNUM, gCdnSSConfig.c2StreamerConfig.httpBindPort );
			ZQTianShan::Util::updateValueMapData( values, CDN_TRANSFERPORTNUM, transferPortNum );

			transferSess->setProps( values );
		}
		else
		{
			ENVLOG(ZQ::common::Log::L_DEBUG, PLFMT(doLoad,"no ticket associated with this request, just ignore"));
		}
	}
	catch(const Ice::Exception& ex)
	{
		ENVLOG(ZQ::common::Log::L_INFO , PLFMT(doLoad,"failed to update transferId to client[%s]"),ex.ice_name().c_str());
	}

	C2Streamer::updateSessionTimer(sessId,timeout);

	return ERR_RETURN_SUCCESS;
}

void parseExtensionNames( const std::string& extnames, std::set<std::string>& fileexts)
{
	if(extnames.empty())		return;
	const char* p = extnames.c_str();
	const char* pStart = p;
	while( *p != 0)
	{
		if( *p == ';')
		{
			std::string ext(pStart,p-pStart);
			fileexts.insert(ext);
			p++;
			pStart = p;
		}
		else
		{
			p++;
		}
	}
	if( pStart !=p )
	{
		std::string ext(pStart, p-pStart);
		fileexts.insert(ext);
	}
}

int32 CdnSsCommand::doCommit(const ZQ::StreamService::PlaylistItemSetupInfos &items,
							 TianShanIce::SRM::ResourceMap &crResource)
{
	if( items.size() <= 0 )
	{
		ENVLOG(ZQ::common::Log::L_ERROR,PLFMT(doCommit,"no item found in the request"));
		return ERR_RETURN_INVALID_PARAMATER;
	}
	
	std::string sessId;
	std::ostringstream  oss;

	std::string clientTransfer;
	std::string transferAddress;
	int64 ingressCapacity						= 0;
	int64 allocatedIngressCapacity				= 0;
	std::string fileName;
	int64 transferRate							= 0;
	int32 transferTimeout						= 500;
	std::string range;
	std::string requestType;
	int64 delay									= 0;	
	C2Streamer::TransferRange					trange;
	int64 upStreamBandwidth						= 0; //used for shadow session
	std::string upSessionUrl;
	std::string indexFilePathname;
	int			isMainFile = 0;
	int			isPwe = 0;

	const TianShanIce::Properties&	crProps = mStreamCtx.getContextProperty();
	int 		exposeIndex  = 0;
	ZQTianShan::Util::getPropertyDataWithDefault( crProps, SYS_PROP(exposeIndex),0,exposeIndex);


	ZQ::common::URLStr upUrl( NULL,true );

	using namespace ZQTianShan::Util;
	using namespace TianShanIce::SRM;

	transferTimeout = env->streamsmithConfig.iPlaylistTimeout;

	ENVLOG(ZQ::common::Log::L_DEBUG,	PLFMT(doCommit,"trying to create a c2 session"));
	bool hasCrResource = false;
	std::string hint = mStreamCtx.id() + " Dump Resource ";

	dumpTianShanResourceMap( crResource, ENVLOG, hint );
	
		getResourceDataWithDefault( crResource, rtEthernetInterface, "destIP", clientTransfer, clientTransfer );
		getResourceDataWithDefault( crResource, rtEthernetInterface, "srcIP", transferAddress, transferAddress );
		getResourceDataWithDefault( crResource, rtTsDownstreamBandwidth, "bandwidth", transferRate, transferRate );
		getResourceDataWithDefault( crResource, rtTsDownstreamBandwidth, "bandwidth", ingressCapacity, ingressCapacity , 1 );
		getResourceDataWithDefault( crResource, rtTsUpstreamBandwidth, "bandwidth", upStreamBandwidth, upStreamBandwidth );
		getResourceDataWithDefault( crResource, rtTsUpstreamBandwidth, "sessionURL", upSessionUrl, upSessionUrl );
	
	
	hasCrResource = !clientTransfer.empty();

	const TianShanIce::Properties& ctxProps = mStreamCtx.getContextProperty();

	int shadowIndexNotFound = 0;
	getPropertyDataWithDefault(ctxProps,SHADOW_INDEX_CONTENT_NOT_FOUND,0,shadowIndexNotFound);
	if(shadowIndexNotFound && gCdnSSConfig.c2StreamerConfig.defaultReaderType == CLIENT_TYPE_DISKAIO )  // expose index == false will not trigger this problem
	{
		ENVLOG(ZQ::common::Log::L_ERROR,PLFMT(doCommit,
			"shadow index session for content[%s], which is not found in DB, reject request"),
			items[0].contentName.c_str());
		
		return ERR_RETURN_INVALID_STATE_OF_ART;
	}


	getPropertyDataWithDefault( ctxProps, "Range", range, range );
	getPropertyDataWithDefault( ctxProps, "CDNType", requestType, requestType );
	getPropertyDataWithDefault( ctxProps, "TansferDelay", delay, delay);
	trange.parse(range);
	getPropertyDataWithDefault( ctxProps, REQUEST_IS_MAINFILE, 0, isMainFile );
	getPropertyDataWithDefault( ctxProps, REQUEST_ASSET_ISPWE, 0, isPwe );
	getPropertyDataWithDefault( ctxProps, REQUEST_ASSET_INDEXFILE_PATH, "", indexFilePathname );

	if (upStreamBandwidth <= 0 )
		ZQTianShan::Util::getValueMapData( items[0].privateData , KEY_ITEM_FULL_PATH_NAME , fileName );//get real file name
	else if( gCdnSSConfig.c2StreamerConfig.defaultReaderType != CLIENT_TYPE_DISKAIO ) {
		std::string strSubType;
		ZQTianShan::Util::getValueMapDataWithDefault( items[0].privateData , CDN_SUBTYPE , "" , strSubType );
		fileName  = items[0].contentName;
		if( !strSubType.empty() ) {
			if( strSubType.at(0) != '.') {
				fileName += '.';
			}
			fileName += strSubType;
		}
		std::string::size_type posLastSlash = fileName.rfind('/');
		if( posLastSlash != std::string::npos ) {
			fileName = fileName.substr(posLastSlash+1);
		}
	}

	std::string allExtNames;
	if( upStreamBandwidth <= 0 )
		ZQTianShan::Util::getValueMapDataWithDefault( items[0].privateData, KEY_ITEM_ALL_EXT_NAMES, "", allExtNames);

	oss.str("");
	
	try
	{
		ZQTianShan::Util::getValueMapDataWithDefault( items[0].privateData , CDN_RANGE , "" , range );
        trange.parse(range);

		ZQTianShan::Util::getValueMapData( items[0].privateData , CDN_CLIENTTRANSFER , clientTransfer );
		ZQTianShan::Util::getValueMapData( items[0].privateData , CDN_TRANSFERADDRESS , transferAddress );
		ZQTianShan::Util::getValueMapData( items[0].privateData , CDN_INGRESSCAPACITY , ingressCapacity );
		
		////////////////////////////////////////////////////////
		///make AllocatedIngressCapacity = IngressCapcity first
		////////////////////////////////////////////////////////

		allocatedIngressCapacity = ingressCapacity;

		
		ZQTianShan::Util::getValueMapData( items[0].privateData , CDN_TRANSFERRATE , transferRate );		
		
			
		ZQTianShan::Util::getValueMapDataWithDefault( items[0].privateData , CDN_DELAY , 0 , delay );
		ZQTianShan::Util::getValueMapDataWithDefault( items[0].privateData , CDN_CDNTYPE , "" , requestType );
		
	}
	catch( const TianShanIce::InvalidParameter& ex)
	{
		if( !hasCrResource )
		{
			ENVLOG(ZQ::common::Log::L_ERROR,PLFMT(doCommit,"invalid parameter [%s]"),ex.message.c_str());
			ex.ice_throw();
			return ERR_RETURN_INVALID_PARAMATER;
		}
	}
	if( transferRate <=0 ) {
		ZQTianShan::Util::getPropertyDataWithDefault( mStreamCtx.getContextProperty(), METADATA_BitRate, 0, transferRate );
		ENVLOG(ZQ::common::Log::L_INFO,PLFMT( doCommit, "no ransferrate is found, use asset't bitrate[%ld] instead"),transferRate);
	}

	ENVLOG(ZQ::common::Log::L_INFO,	PLFMT(doCommit,"initiate transfer with clientTransfer[%s] transferAddress[%s] "
		"ingressCapacity[%ld] allocatedIngressCapacity[%ld] fileName[%s] transferRate[%ld] "
		"transferTimeout[%d] range[%s] delay[%ld] exts[%s] upStreamBW[%ld]" ),
		clientTransfer.c_str(),
		transferAddress.c_str() ,
		ingressCapacity ,
		allocatedIngressCapacity,
		fileName.c_str(),
		transferRate ,
		transferTimeout ,
		range.c_str(),
		delay,
		allExtNames.c_str(),
	    upStreamBandwidth	);

	//get parameter from privateData of item information
	//C2Streamer::int32		cTransferInit( TransferInitRequestParamPtr request , TransferInitResponseParamPtr response );
	C2Streamer::TransferInitRequestParamPtr request = new C2Streamer::TransferInitRequestParam( *mStreamerManager.getC2Env(), URLRULE_C2INIT );
	C2Streamer::TransferInitResponseParamPtr response = new C2Streamer::TransferInitResponseParam();

	request->method				= C2Streamer::METHOD_TRANSFER_INIT;
	request->sessionId			= mStreamCtx.id();

	request->sessProp.queryIndex = exposeIndex > 0;

	request->clientTransfer		= clientTransfer;
	request->transferAddress	= transferAddress;
	request->ingressCapacity	= ingressCapacity;
	request->fileName			= fileName;
	request->transferRate		= transferRate;
	request->transferTimeout	= transferTimeout;
	request->requestRange		= trange;
	request->transferDelay		= delay;
	request->isMainFile			= isMainFile >= 1;
	request->indexFilePathname	= indexFilePathname;
	request->exposeIndex		= exposeIndex != 0;

	if( request->isMainFile ) {
		C2Streamer::C2IndexRecordCenter& ircenter = env->getStreamerManager().getC2Service()->getIndexRecordCenter();
		ircenter.tryParsing( indexFilePathname, isPwe >= 1 );
	}

	request->requestType		= allExtNames.empty()? C2Streamer::SESSION_TYPE_NORMAL : C2Streamer::SESSION_TYPE_COMPLEX;

	if( upStreamBandwidth > 0 )
	{
		static int clienttype = gCdnSSConfig.c2StreamerConfig.defaultReaderType;
		if( clienttype != CLIENT_TYPE_DISKAIO ) {
			upSessionUrl = std::string("http://www.test.com/")+ TRANSFERSESSION_PREFIX + "/" + mStreamCtx.id();
			ENVLOG(ZQ::common::Log::L_INFO, PLFMT(doCommit,"fake a upSessionUrl: %s"), upSessionUrl.c_str() );
		}
		request->requestType	= C2Streamer::SESSION_TYPE_SHADOW;
		request->transferRate	= upStreamBandwidth;// use upStreamBandwidth if it is a shadow session

		std::string upStreamBindAddr;
		getResourceDataWithDefault( crResource, rtTsUpstreamBandwidth,"bindAddress", upStreamBindAddr, upStreamBindAddr );
		request->upStreamLocalIp = upStreamBindAddr;

		ENVLOG(ZQ::common::Log::L_INFO, PLFMT(doCommit,"trying to create shadow session with url [%s] bindAddr[%s]"),
				upSessionUrl.c_str(), upStreamBindAddr.c_str() );
		if( upSessionUrl.empty() || !upUrl.parse(upSessionUrl.c_str()) )
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(ENVLOG,"CDNSS",400,PLFMT(doCommit,"shadow session but no session url is found"));
		}
		request->upStreamPeerIp = upUrl.getHost();
		request->upStreamPeerPort = (uint16)upUrl.getPort();

	}
	request->requestType		|= stricmp( requestType.c_str(),"SeaChange") == 0 ? C2Streamer::SESSION_TYPE_SCHANGEREQ : 0;

	parseExtensionNames( allExtNames, request->subFileExts);

	int32 iRet = C2Streamer::cTransferInit( request , response );
	if( iRet != C2Streamer::errorCodeOK )
	{
		// Do not throw InvalidParameter here due to doCommit() itself has no input parameters
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(ENVLOG, "CDNSS", iRet, PLFMT(doCommit, "failed to load [%s]: %s"),	fileName.c_str(), response->errorText.c_str());
	}
	else
	{
		sessId = response->transferId;
		ENVLOG(ZQ::common::Log::L_INFO,PLFMT(doCommit,"loaded item[%s]: transferId[%s] timeout[%d] availRange[%s] openForWrite[%s] upSessionUrl[%s]"),
			fileName.c_str() , sessId.c_str() , transferTimeout , response->availRange.toString().c_str() , response->openForWrite ? "yes" : "no" ,
			upSessionUrl.c_str() );
	}

	if ( upStreamBandwidth > 0 )
	{//shadow session
		if( !upSessionUrl.empty() )
		{
			mStreamCtx.updateContextProperty( CDN_TRANSFERID, upUrl.getPath() );
			mStreamCtx.updateContextProperty( CDN_TRANSFERTIMEOUT, (Ice::Long)transferTimeout );
			mStreamCtx.updateContextProperty( CDN_AVAILRANGE, response->availRange.toString() );
			mStreamCtx.updateContextProperty( CDN_OPENFORWRITE, response->openForWrite ? "yes":"no" );


			upUrl.setHost( response->confirmedTransferIp.c_str() );
			upUrl.setPort( atoi(response->confirmedTransferPort.c_str()) );
			std::string newUpSessUrl = std::string(upUrl.generate());
			updateResourceData( crResource, rtTsDownstreamBandwidth, "sessionURL", newUpSessUrl );


			ENVLOG(ZQ::common::Log::L_DEBUG, PLFMT(doCommit,"shadow session created, update url from [%s] to [%s]"),
					upSessionUrl.c_str(), newUpSessUrl.c_str() );
		}
		else
		{
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(ENVLOG,"CDNSS",500,PLFMT(doCommit,"Shadow session, but no upstream session url is found"));
		}
	}
	else
	{
		mStreamCtx.updateContextProperty( CDN_TRANSFERID, sessId );
		mStreamCtx.updateContextProperty( CDN_TRANSFERTIMEOUT, (Ice::Long)transferTimeout );
		mStreamCtx.updateContextProperty( CDN_AVAILRANGE, response->availRange.toString() );
		mStreamCtx.updateContextProperty( CDN_OPENFORWRITE, response->openForWrite ? "yes":"no" );

		std::string sessionUrl = "http://"+response->confirmedTransferIp + ":" + response->confirmedTransferPort + "/"+sessId;
		updateResourceData( crResource, rtTsDownstreamBandwidth, "sessionURL", sessionUrl);
	}

	mStreamCtx.updateContextProperty( CDN_TRANSFERPORT, response->confirmedTransferIp );
	mStreamCtx.updateContextProperty( CDN_TRANSFERPORTNUM, response->confirmedTransferPort );

	updateResourceData( crResource, rtEthernetInterface, "srcIP", response->confirmedTransferIp );
	int c2bindHttpPort = atoi( response->confirmedTransferPort.c_str() );
	updateResourceData( crResource, rtEthernetInterface, "srcPort", c2bindHttpPort );


	ss.registerStreamId( sessId, mStreamCtx.id() );//register sessionId, so that we stream service library can process the stream event
	mStreamCtx.updateContextProperty( STREAMING_ATTRIBUTE_STREAM_SESSION_ID, sessId);
	return ERR_RETURN_SUCCESS;
}

int32 CdnSsCommand::doDestroy( const std::string& sessionId , const std::string& clientTransfer )
{	
	C2Streamer::TransferTermRequestParamPtr request = new C2Streamer::TransferTermRequestParam(*mStreamerManager.getC2Env(), URLRULE_C2TERM );
	C2Streamer::TransferTermResponseParamPtr response = new C2Streamer::TransferTermResponseParam();

	request->method			= C2Streamer::METHOD_TRANSFER_TERM;
	request->clientTransfer = clientTransfer;
	request->sessionId		= sessionId;

	C2Streamer::cTransferTerm( request , response );

	return ERR_RETURN_SUCCESS;
}

void CdnSsCommand::doClearResource( const TianShanIce::ValueMap& playlistParams )
{//No resource need to be cleared so far , nothing to be done
	
}

int32 CdnSsCommand::doValidateItem(TianShanIce::Streamer::PlaylistItemSetupInfo &info)
{
	const TianShanIce::Properties&	crProps = mStreamCtx.getContextProperty();
	const TianShanIce::SRM::ResourceMap& crResource = mStreamCtx.getContextResources();
	int64 upStreamBandwidth = 0;
	int exposeIndex  = 0;
	ZQTianShan::Util::getResourceDataWithDefault( crResource, TianShanIce::SRM::rtTsUpstreamBandwidth, "bandwidth", upStreamBandwidth, upStreamBandwidth );
	ZQTianShan::Util::getPropertyDataWithDefault( crProps, SYS_PROP(exposeIndex),0,exposeIndex);
	bool bShadowSession = upStreamBandwidth > 0 ;

	if( bShadowSession ) 
	{
		ENVLOG(ZQ::common::Log::L_INFO, PLFMT(doValidateItem,"comes in a shadow session item[%s], exposeIndex[%s]") ,
				info.contentName.c_str(), exposeIndex?"true":"false" );
		if( !exposeIndex )
		{
			return ERR_RETURN_SUCCESS;
		}
	}
#ifndef ZQ_CDN_UMG
	TianShanIce::Storage::ContentStorePrx csPrx = env->getCsPrx();
	if( !csPrx )
	{
#endif

		std::string strItem = info.contentName;
		std::string::size_type pos = strItem.find_last_of('/');
		if( pos != std::string::npos )
		{
			strItem = strItem.substr(pos+1);
		}
		ZQTianShan::Util::updateValueMapData( info.privateData , KEY_ITEM_FULL_PATH_NAME , strItem );
		ENVLOG(ZQ::common::Log::L_WARNING,PLFMT(doValidateItem,"No ContentStore is connected , use original content name [%s]") , strItem.c_str() );
		return ERR_RETURN_SUCCESS;
#ifndef ZQ_CDN_UMG
	}
	
	TianShanIce::Storage::UnivContentPrx ctntPrx = NULL;
	{
		char szPvBuf[1024];
		char *p = szPvBuf;
		size_t bufsize= sizeof(szPvBuf)-1;
		ZQTianShan::Util::dumpTianShanValueMap(info.privateData, p,bufsize, std::string(""));
		ENVLOG(ZQ::common::Log::L_DEBUG, PLFMT(doValidateItem,"validate item:%s  %s"),
				ZQTianShan::Util::dumpPlaylistItemSetupInfo(info).c_str(),szPvBuf);
	}
	try
	{
		std::string realFullPathName;

		std::string		strSubType;
		std::string		allExtNames;
		std::string		strExtName;
		int64			bitrate = 0 ;
		bool			bMainFile = false;
		bool			bPwe = false;
		std::string		strIndexFilePathname;

		ZQTianShan::Util::getValueMapDataWithDefault( info.privateData , CDN_SUBTYPE , "" , strSubType );
		if( bShadowSession && exposeIndex ) 
		{
			if( stricmp(strSubType.c_str() , CDN_SUBTYPE_Index ) == 0  ) 
			{
				std::string pid,paid;
				std::string upSessionUrl;
				using namespace TianShanIce::SRM;
				ZQTianShan::Util::getValueMapDataWithDefault( info.privateData, std::string(METADATA_ProviderId), std::string(""), pid);
				ZQTianShan::Util::getValueMapDataWithDefault( info.privateData, std::string(METADATA_ProviderAssetId), std::string(""), paid);
				ZQTianShan::Util::getResourceDataWithDefault( mStreamCtx.getContextResources(), rtTsUpstreamBandwidth, "sessionURL", upSessionUrl, upSessionUrl );
				try {
					ctntPrx = TianShanIce::Storage::UnivContentPrx::checkedCast( csPrx->openContentByFullname( info.contentName ) );
					if(!findShadowIndexContent(ctntPrx, upSessionUrl,info.contentName ,pid,paid,strSubType) ) {
						ENVLOG(ZQ::common::Log::L_ERROR,PLFMT(doValidateItem,"shadow index session, but no content is found in DB"));
						mStreamCtx.updateContextProperty( SHADOW_INDEX_CONTENT_NOT_FOUND, (int)1 );
					}  else {
						ENVLOG(ZQ::common::Log::L_INFO,PLFMT(doValidateItem,"shadow index session, information already updated"));
					}
				} catch( const Ice::ObjectNotExistException& ) {
					ENVLOG(ZQ::common::Log::L_ERROR,PLFMT(doValidateItem,"shadow index session, got ObjectNotExistException"));
					mStreamCtx.updateContextProperty( SHADOW_INDEX_CONTENT_NOT_FOUND, (int)1 );
				}
				catch( const Ice::Exception& ex )
				{
					ENVLOG(ZQ::common::Log::L_ERROR,PLFMT(doValidateItem,"shadow index session, got [%s]"),
						ex.ice_name().c_str() );
					mStreamCtx.updateContextProperty( SHADOW_INDEX_CONTENT_NOT_FOUND, (int)1 );
				}				
			}
			return ERR_RETURN_SUCCESS;
		}
		
		//get content proxy in any case
		//else if( stricmp(strSubType.c_str() , CDN_SUBTYPE_Index ) == 0  || stricmp( strSubType.c_str() , CDN_SUBTYPE_NormalForward ) == 0 )
		{
			ctntPrx = TianShanIce::Storage::UnivContentPrx::checkedCast( csPrx->openContentByFullname( info.contentName ) );
			if( !ctntPrx )
			{
				ENVLOG(ZQ::common::Log::L_ERROR, PLFMT(doValidateItem,"can't open content with name[%s]" ),
					info.contentName.c_str() );
				return ERR_RETURN_ASSET_NOTFOUND;
			}

			TianShanIce::Storage::ContentState state =  ctntPrx->getState();

			bPwe = state >= TianShanIce::Storage::csProvisioning && state <= TianShanIce::Storage::csProvisioningStreamable;
		}

		if( stricmp(strSubType.c_str() , CDN_SUBTYPE_Index ) == 0 )
		{//if request the index file , then remember to query content's metadata to find out the index file name or extension
			//get index file name as real full path name
			realFullPathName = getFileNameWithoutMountPath( ctntPrx , METADATA_INDEXFILENAME(), &bitrate );
		}
		else if( stricmp( strSubType.c_str() , CDN_SUBTYPE_NormalForward ) == 0  )
		{
			bMainFile = true;
			realFullPathName = getFileNameWithoutMountPath( ctntPrx , METADATA_MAIFILENAME(), &bitrate );
			TianShanIce::Properties props = ctntPrx->getMetaData();
			ZQTianShan::Util::getPropertyDataWithDefault( props, METADATA_INDEXFILENAME(), "", strIndexFilePathname );
		}
		else if ( stricmp ( strSubType.c_str(), "*" ) == 0 )
		{
			//trying to get all extension name of current content
			realFullPathName = getFilePathNameFromContentName(info.contentName );
			TianShanIce::Properties props = ctntPrx->getMetaData();
			ZQTianShan::Util::getPropertyDataWithDefault( props, "sys.allextentionnames","",allExtNames);
			ZQTianShan::Util::getPropertyDataWithDefault( props , METADATA_BitRate, 0, bitrate );
		}		
		else
		{//not index file or main file, so get the extension name
			
			ZQTianShan::Util::getValueMapDataWithDefault( info.privateData , CDN_EXTENSIONNAME , "", strExtName );
			if( strExtName.length() > 0  )
			{
				if(  strExtName.at(0) == '.' ) 
					realFullPathName = getFilePathNameFromContentName(info.contentName) + strExtName;
				else
					realFullPathName = getFilePathNameFromContentName(info.contentName) + "." + strExtName;
			}
			else
			{
				realFullPathName = getFilePathNameFromContentName(info.contentName) +  strExtName;
			}

			TianShanIce::Properties props = ctntPrx->getMetaData();
	
			std::string mainfileExtName;
			ZQTianShan::Util::getPropertyDataWithDefault( props,  METADATA_MAIFILE_EXTNAME(), "", mainfileExtName );
			if( strExtName == mainfileExtName || (std::string(".")+strExtName) == mainfileExtName ){
				bMainFile = true;
				ZQTianShan::Util::getPropertyDataWithDefault( props, METADATA_INDEXFILENAME(), "", strIndexFilePathname );
			}
		}

		if( realFullPathName.empty()) {
			ENVLOG(ZQ::common::Log::L_ERROR,PLFMT(doValidateItem,"failed to get filename for [%s]"),info.contentName.c_str());
			return ERR_RETURN_ASSET_NOTFOUND;
		}

			
		ZQTianShan::Util::updateValueMapData( info.privateData , KEY_ITEM_FULL_PATH_NAME , realFullPathName );
		ZQTianShan::Util::updateValueMapData( info.privateData , KEY_ITEM_ALL_EXT_NAMES	, allExtNames );
		mStreamCtx.updateContextProperty( METADATA_BitRate, bitrate );
		mStreamCtx.updateContextProperty( REQUEST_IS_MAINFILE, bMainFile ? 1: 0 );
		mStreamCtx.updateContextProperty( REQUEST_ASSET_INDEXFILE_PATH, strIndexFilePathname );
		mStreamCtx.updateContextProperty( REQUEST_ASSET_ISPWE, bPwe ? 1 : 0 );


		ENVLOG(ZQ::common::Log::L_DEBUG,PLFMT(doValidateItem,"get real pathname[%s] for item[%s] and subType[%s] isMainFile[%s] ext[%s] allexts[%s]"),
			realFullPathName.c_str() , info.contentName.c_str() , strSubType.c_str() ,
			bMainFile?"true":"false", strExtName.c_str(), allExtNames.c_str() );

	}
	catch( const TianShanIce::BaseException& ex )
	{
		ENVLOG(ZQ::common::Log::L_ERROR, PLFMT(doValidateItem,"failed to open content with name[%s] with TianShan Exception [%s]" ),
			info.contentName.c_str() , ex.message.c_str() );
		return ERR_RETURN_ASSET_NOTFOUND;
	}
	catch( const Ice::Exception& ex)
	{
		ENVLOG(ZQ::common::Log::L_ERROR, PLFMT(doValidateItem,"failed to open content with name[%s] with Ice Exception [%s]" ),
			info.contentName.c_str() , ex.ice_name().c_str() );
		return ERR_RETURN_ASSET_NOTFOUND;
	}
#endif
	return ERR_RETURN_SUCCESS;	
}

#ifndef ZQ_CDN_UMG

std::string CdnSsCommand::getFilePathNameFromContentName( const std::string& name )
{
	std::string::size_type pos = name.find('/');
	if( pos == std::string::npos )
		return name;
	pos = name.find('/',pos + 1);
	if( pos == std::string::npos )
		return name;
	return name.substr(pos+1);
}

std::string CdnSsCommand::getFileNameWithoutMountPath( const std::string& filepath)
{
	static std::string docroot = ZQTianShan::Util::fsFixupPath( env->getC2DocRoot() );
	static size_t addone = docroot.length() > 0 ? (docroot.at(docroot.length()-1) != '/' ? 1 : 0) : 0;
	std::string::size_type drlen = docroot.length();
	std::string::size_type prefixPos = filepath.find(docroot);
	std::string value;
	value = filepath;
	if( prefixPos != std::string::npos )
	{			
		size_t pos = prefixPos + addone + drlen;
		if( pos < filepath.length() )
			value = value.substr( pos );
	}
	ENVLOG(ZQ::common::Log::L_INFO, PLFMT(getFileNameWithoutMountPath,"get file path[%s]"),value.c_str());	
	return value;

}
std::string	CdnSsCommand::getFileNameWithoutMountPath( TianShanIce::Storage::UnivContentPrx& ctntPrx, const std::string& key, int64* bitrate )
{
	std::string value;
	if( !key.empty())
	{
		TianShanIce::Properties ctntMetedata = ctntPrx->getMetaData();
		ZQTianShan::Util::getPropertyDataWithDefault( ctntMetedata , key , "" , value );
		if( bitrate ) {
			ZQTianShan::Util::getPropertyDataWithDefault( ctntMetedata, METADATA_BitRate, 0, *bitrate);
		}

		std::string assetIndexGenericInfo;
		std::string assetIndexSubfileInfo;
		ZQTianShan::Util::getPropertyDataWithDefault( ctntMetedata, METADATA_IDX_GENERIC_INFO, "", assetIndexGenericInfo);
		ZQTianShan::Util::getPropertyDataWithDefault( ctntMetedata, METADATA_IDX_SUBFULE_INFO, "", assetIndexSubfileInfo);

		mStreamCtx.updateContextProperty( CDN_IDXCONTENT_GENERIC, assetIndexGenericInfo );
		mStreamCtx.updateContextProperty( CDN_IDXCONTENT_SUBFILES, assetIndexSubfileInfo );

		return getFileNameWithoutMountPath(value);
	}
	else
	{
		value = ctntPrx->getName();
	}
	ENVLOG(ZQ::common::Log::L_INFO, PLFMT(getFileNameWithoutMountPath,"get file path[%s]"),value.c_str());	
	return value;
}

bool CdnSsCommand::findShadowIndexContent(  TianShanIce::Storage::UnivContentPrx contentPrx, 
										  const std::string& upstreamUrl,
										  const std::string& name,
										  const std::string& pid, const std::string& paid, 
										  const std::string& subtype)
{
	try
	{
		TianShanIce::Properties ctntMetedata = contentPrx->getMetaData();
		
		std::string assetIndexGenericInfo;
		std::string assetIndexSubfileInfo;
		ZQTianShan::Util::getPropertyDataWithDefault( ctntMetedata, METADATA_IDX_GENERIC_INFO, "", assetIndexGenericInfo);
		ZQTianShan::Util::getPropertyDataWithDefault( ctntMetedata, METADATA_IDX_SUBFULE_INFO, "", assetIndexSubfileInfo);

		if( assetIndexGenericInfo.empty() && assetIndexSubfileInfo.empty() ) {
			mStreamerManager.getShadowIndex(upstreamUrl,name,pid,paid,subtype);
			return false;
		}

		mStreamCtx.updateContextProperty( CDN_IDXCONTENT_GENERIC, assetIndexGenericInfo );
		mStreamCtx.updateContextProperty( CDN_IDXCONTENT_SUBFILES, assetIndexSubfileInfo );
		return true;
	}
	catch( const TianShanIce::BaseException& ex )
	{
		mStreamerManager.getShadowIndex( upstreamUrl,name,pid,paid,subtype );
		return false;
	}
	catch( const Ice::Exception& ex) {
		mStreamerManager.getShadowIndex( upstreamUrl,name,pid,paid,subtype );
		return false;
	}
	return true;
}
#endif
}}
