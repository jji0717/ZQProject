
#include <ZQ_common_conf.h>
#include <FileSystemOp.h>

#include "C2StreamerEnv.h"
#include "C2StreamerService.h"

#include "C2SessionHelper.h"
#include "C2Session.h"
#include "C2TunnelBwmon.h"


#if defined ZQ_OS_MSWIN
	#define	SESSFMT(x,y) 	"%s/%08X/REQUEST[%s]\t"##y, request->requestHint.c_str() , GetCurrentThreadId(),#x	
#elif defined ZQ_OS_LINUX
	#define	SESSFMT(x,y) 	"%s/%08X/REQUEST[%s]\t"y, request->requestHint.c_str() ,  (unsigned int)gettid(),#x	
#endif	

namespace C2Streamer
{

int32 C2Session::registerSession( const TransferInitRequestParamPtr request , TransferInitResponseParamPtr response )
{
	mDataRunner->mSessTransferRate	= request->transferRate;
	if( mDataRunner->mSessTransferRate <= 0)
	{
		updateLastError(request,response,errorCodeBadRequest,"bad transferRate[%lld]bps",mDataRunner->mSessTransferRate);
		return errorCodeBadRequest;
	}

	///register session into port manager
	PortManager& portManager = mSvc.getPortManager();
	mServerTransferAddress = "";
	
	//check if the transferAddress is valid or not
	if( request->transferAddress.empty() )
	{
		MLOG(ZQ::common::Log::L_DEBUG,SESSFMT(registerSession,"no transferAddress is specified, trying to allocated it"));
		std::vector<std::string> availPorts =  portManager.getAvailPorts(request->transferRate, mSessionId	);
		std::vector<std::string>::const_iterator it = availPorts.begin();
		for( ; it != availPorts.end(); it ++ )
		{
			if(portManager.registerSession( *it, mSessionId, request->transferRate))
			{
				mServerTransferAddress = *it;
				break;
			}
		}
		if(mServerTransferAddress.empty())
		{
			updateLastError(request,response,errorCodeBadRequest,"failed to allocate any available port to this session");
			return errorCodeBadRequest;
		}
		else
		{
			MLOG(ZQ::common::Log::L_INFO,SESSFMT(registerSession,"allocated [%s] to this session"),mServerTransferAddress.c_str() );
		}
	}
	else
	{	
		if( !portManager.registerSession( request->transferAddress , mSessionId , request->transferRate ) )
		{
			//FIXME: how to response if failed to register session into port manager ?
			updateLastError(request,response,errorCodeBadRequest,"failed to register session into portManager");
			return errorCodeBadRequest;
		}
		
		mServerTransferAddress = request->transferAddress; //copy requested transfer address into our record
	}

	response->confirmedTransferIp = mServerTransferAddress;
	response->confirmedTransferPort = mEnv.getConfig().mLocalBindPort;

	///register session into client manager
	C2ClientManager& clientManager = mSvc.getClientManager();
	clientManager.updateClient( request->clientTransfer , request->ingressCapacity );
	if( !clientManager.registerSession( request->clientTransfer , mSessionId , request->transferRate ) )
	{
		updateLastError(request, response , errorCodeBadRequest , "client[%s] has not enough bandwidth to serve the request", request->clientTransfer.c_str() );
		return errorCodeBadRequest;
	}

	mClientTransferAddress	= request->clientTransfer;		
	mDataRunner->mSessTransferDelay	= request->transferDelay;	
	return errorCodeOK;
}

int32 C2Session::registerShadowSession( const TransferInitRequestParamPtr request, TransferInitResponseParamPtr response)
{
	MLOG(ZQ::common::Log::L_DEBUG, SESSFMT(registerShadowSession, "register shadow session with UpSide[local:<%s> peer: <%s:%hu> ]"),
			request->upStreamLocalIp.c_str(), request->upStreamPeerIp.c_str(), request->upStreamPeerPort);

	mSessionType = request->requestType;

	mDataRunner->mSessTransferRate 	= request->transferRate;

	mTimeoutInterval		= request->transferTimeout;

	int err =  registerSession(request, response);
	if ( err != errorCodeOK )
		return err;

	const std::string& localIp = response->confirmedTransferIp;

	const std::string&	upStreamPeerIp = request->upStreamPeerIp;
	uint16		upStreamPeerPort = request->upStreamPeerPort;
	const std::string&  upStreamLocalIp = request->upStreamLocalIp;

	IptablesRule& ipr = mSvc.getIptablesRule();
	unsigned short natPort = ipr.getPort( localIp, upStreamPeerIp, upStreamPeerPort, upStreamLocalIp );
	
	if( natPort == 0 )
	{
		updateLastError( request,response,errorCodeBadRequest,"registerShadowSession() not enough udpport for peerAddress[%s:%hu] localIp[%s]",
				upStreamPeerIp.c_str(), upStreamPeerPort, localIp.c_str() );	
		return errorCodeBadRequest;
	}

	char szNatPort[128];
	sprintf(szNatPort,"%hu", natPort);	
	response->confirmedTransferPort = szNatPort;

	response->transferId	= constructResponseSessId( mSessionId );

	MLOG(ZQ::common::Log::L_INFO, SESSFMT(registerShadowSession,"DNAT created for this session, localAddress[%s:%hu], OutIp[%s], peerAddress[%s:%hu]"),
			localIp.c_str(), natPort, upStreamLocalIp.c_str(), upStreamPeerIp.c_str() , upStreamPeerPort);
	
	return errorCodeOK;
}

int32 C2Session::processTransferInit( const TransferInitRequestParamPtr request , TransferInitResponseParamPtr response )
{
	MLOG(ZQ::common::Log::L_DEBUG, SESSFMT(processTransferInit, "init session with filename[%s] bitrate[%d] delay[%d] range[%s] requestType[%d]"),
		request->fileName.c_str() , request->transferRate , request->transferDelay, request->requestRange.toString().c_str() , 
		request->requestType );
	
	//if clientType ==2 (proxy mode), take Shadow Session as local session
	if( mEnv.getConfig().clientType != 2 && BASE_SESSION_TYPE(request->requestType) == SESSION_TYPE_SHADOW )
	{
		return registerShadowSession( request, response );
	}
	else if( BASE_SESSION_TYPE(request->requestType) != SESSION_TYPE_COMPLEX) 
	{
		C2SessFile sf(mEnv);
		sf.process( request->fileName , mSessionId );

		if( !sf.isValid() )
		{
			updateLastError(request,response,errorCodeContentNotFound,"processTransferInit() failed to find regular file[%s]",request->fileName.c_str());
			return errorCodeContentNotFound;
		}

		TransferRange reqRange = request->requestRange;
		if(reqRange.bStartValid && (reqRange.startPos < 0) ){
			reqRange.startPos += sf.dataEndPos();
			if( reqRange.startPos < 0 ) {
				reqRange.startPos = 0;
			}
			MLOG(ZQ::common::Log::L_INFO, SESSFMT(processTransferInit,"fix request range[%s] to [%s]"),
					request->requestRange.toString().c_str(), reqRange.toString().c_str() );
		}
		
		//step 2 check request range with file size
		if( !sf.checkRequestRange( reqRange ) )
		{
			updateLastError(request,response,errorCodeBadRange,"processTransferInit() range[%s] is not acceptable according to file data range[%lld-%lld]",
					 reqRange.toString().c_str(), sf.dataStartPos() , sf.dataEndPos() );
			return errorCodeBadRange;
		}
		//now I know the request range
		mDataRunner->mRequestRange		= reqRange; // copy request range into our record

		if( !mDataRunner->mRequestRange.bStartValid )
		{
			mDataRunner->mRequestRange.bStartValid = true;
			mDataRunner->mRequestRange.startPos = sf.dataStartPos();
		}
		mFileName						= sf.getFileFullPath();
		response->availRange			= composeFileRange( sf );
	}
		
	mDataRunner->mSessTransferRate 	= request->transferRate;
	mRequestFileName				= request->fileName;
	mContentName					= request->fileName;
	mbMainFile						= request->isMainFile;
	if(mbMainFile) {
		mIndexFilePathname 			= request->indexFilePathname;
	}

	mRequestFileExts				= request->subFileExts;
	
	int32 retCode = registerSession( request , response);
	if( retCode != errorCodeOK ) return retCode;
	
	mTimeoutInterval		= request->transferTimeout;	

	IAttrBridge* iBridge 	= mEnv.getAttrBridge();
	if( iBridge )
		response->openForWrite	= iBridge->isFileBeingWritten( request->fileName , mSessionId );
	else
		response->openForWrite	= false;
	response->timeout		= request->transferTimeout;
	response->transferId	= constructResponseSessId( mSessionId );
	mSessionType			= request->requestType;


	PortManager::PortAttr pa;
	if(! mSvc.getPortManager().getPortAttr( mServerTransferAddress, pa ) )
	{
		updateLastError(request,response,errorCodeInternalError,"failed to get mtu from portmanager for [%s]",
			mServerTransferAddress.c_str());
		return errorCodeInternalError;
	}
	mDataRunner->mEthMtu = pa.ethMtu;
	if( mEnv.getConfig().mSendPacketSize < pa.ethMtu ) {
		mDataRunner->mEthMtu = mEnv.getConfig().mSendPacketSize;
	}

	if( request->exposeIndex ) {
		MLOG(ZQ::common::Log::L_INFO, SESSFMT(processTransferInit,"expose asset information"));
		AssetAttribute::Ptr attr = mSvc.getSessManager().getAssetAttribute( request->fileName );
		if(attr) {
			attr->wait();
			if( attr->lastError() == 0 ) {
				response->baseinfo = attr->assetBaseInfo();
				response->memberinfo = attr->assetMemberInfo();
				response->openForWrite = attr->pwe();
			}
		} else {
			MLOG(ZQ::common::Log::L_WARNING, SESSFMT(processTransferInit,"failed to get asset information for[%s]"),
					request->fileName.c_str() );
		}
	}

	MLOG(ZQ::common::Log::L_INFO, SESSFMT(processTransferInit, "successfully created session with filename[%s] bitrate[%d] delay[%d] range[%s] requestType[%d], mtu[%d]"),
		request->fileName.c_str() , request->transferRate , request->transferDelay, request->requestRange.toString().c_str() , 
		request->requestType, mDataRunner->mEthMtu );

	return errorCodeOK;
}

}//namespace C2Streamer


