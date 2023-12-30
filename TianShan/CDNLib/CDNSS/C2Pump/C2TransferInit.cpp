
#include <ZQ_common_conf.h>
#include <FileSystemOp.h>

#include "C2StreamerEnv.h"
#include "C2StreamerService.h"

#include "C2SessionHelper.h"
#include "C2Session.h"
#include "C2TunnelBwmon.h"


#if defined ZQ_OS_MSWIN
	#define	SESSFMT(x,y) 	CLOGFMT(x, " REQUEST[%s]\t"##y), request->requestHint.c_str() 
#elif defined ZQ_OS_LINUX
	#define	SESSFMT(x,y) 	CLOGFMT(x, " REQUEST[%s]\t"y), request->requestHint.c_str()
#endif	

namespace C2Streamer
{

int32 C2Session::registerSession( const RequestParamPtr request , RequestResponseParamPtr response )
{
	if(request->transferRate <= 0) {
		updateLastError(request,response,errorCodeBadRequest,"bad transferRate[%ld]bps", request->transferRate);
		return errorCodeBadRequest;
	}
	int32 bitrateInflation = request->getConfWriter()->mBitrateInflationPercent;
	int32 calcedBitrate = bitrateInflation < 80 ? 80 : bitrateInflation;
	calcedBitrate = calcedBitrate > 150 ? 150 : calcedBitrate;
	calcedBitrate = calcedBitrate * request->transferRate / 100;

	/*
	mCurrentDataRunner->getTransferRate()	= request->transferRate + request->transferRate * bitrateInflation / 100;
	if( mCurrentDataRunner->getTransferRate() <= 0)
	{
		updateLastError(request,response,errorCodeBadRequest,"bad transferRate[%lld]bps",mCurrentDataRunner->getTransferRate());
		return errorCodeBadRequest;
	}
	*/
	///register session into port manager and client manager
	PortManager& portManager = mSvc.getPortManager();
	C2ClientManager& clientManager = mSvc.getClientManager();
	
	if( METHOD_TRANSFER_INIT == request->method )
	{
		TransferInitRequestParamPtr requestTransfer = TransferInitRequestParamPtr::dynamicCast(request);
		TransferInitResponseParamPtr responseTransfer = TransferInitResponseParamPtr::dynamicCast(response);
		
		mCurrentDataRunner->setTransferRate(calcedBitrate);
		mSessionTransferRate = calcedBitrate;
		if( mCurrentDataRunner->getTransferRate() <= 0)
		{
			updateLastError(request,response,errorCodeBadRequest,"bad transferRate[%lld]bps",mCurrentDataRunner->getTransferRate());
			return errorCodeBadRequest;
		}
		
		mServerTransferAddress = "";

		//check if the transferAddress is valid or not
		if( requestTransfer->transferAddress.empty() )
		{
			MLOG(ZQ::common::Log::L_DEBUG,SESSFMT(registerSession,"no transferAddress is specified, trying to allocated it"));
			std::vector<std::string> availPorts =  portManager.getAvailPorts(requestTransfer->transferRate, mSessionId	);
			std::vector<std::string>::const_iterator it = availPorts.begin();
			for( ; it != availPorts.end(); it ++ )
			{
				if(portManager.registerSession( *it, mSessionId, requestTransfer->transferRate))
				{
					mServerTransferAddress = *it;
					break;
				}
			}
			if(mServerTransferAddress.empty())
			{
				updateLastError(requestTransfer,responseTransfer, errorCodeBadRequest,"failed to allocate any available port to this session");
				return errorCodeBadRequest;
			}
			else
			{
				MLOG(ZQ::common::Log::L_INFO,SESSFMT(registerSession,"allocated [%s] to this session"),mServerTransferAddress.c_str() );
			}
		}
		else
		{	
			if( !portManager.registerSession( requestTransfer->transferAddress , mSessionId , requestTransfer->transferRate ) )
			{
				//FIXME: how to response if failed to register session into port manager ?
				updateLastError(requestTransfer,responseTransfer, errorCodeBadRequest,"failed to register session into portManager");
				return errorCodeBadRequest;
			}

			mServerTransferAddress = requestTransfer->transferAddress; //copy requested transfer address into our record
		}

		responseTransfer->confirmedTransferIp = mServerTransferAddress;
		responseTransfer->confirmedTransferPort = mEnv.getConfig().mLocalBindPort;

	///register session into client manager
	//C2ClientManager& clientManager = mSvc.getClientManager();
		clientManager.updateClient( requestTransfer->clientTransfer , requestTransfer->ingressCapacity );
		if( !clientManager.registerSession( requestTransfer->clientTransfer , mSessionId , calcedBitrate) )
		{
			updateLastError(requestTransfer, responseTransfer , errorCodeBadRequest , "client[%s] has not enough bandwidth to serve the request", requestTransfer->clientTransfer.c_str() );
			return errorCodeBadRequest;
		}

		mClientTransferAddress	= requestTransfer->clientTransfer;		
		mCurrentDataRunner->setTransferDelay(requestTransfer->transferDelay);
	}
	else if( METHOD_UDP_INIT == request->method )
	{
		UdpStreamInitRequestParamPtr requestUdp = UdpStreamInitRequestParamPtr::dynamicCast(request);
		UdpStreamInitResponseParamPtr responseUdp = UdpStreamInitResponseParamPtr::dynamicCast(response);
		mCurrentDataRunner->setTransferRate(calcedBitrate);
		mSessionTransferRate = calcedBitrate;
		if( mCurrentDataRunner->getTransferRate() <= 0)
		{
			updateLastError(request,response,errorCodeBadRequest,"bad transferRate[%lld]bps",mCurrentDataRunner->getTransferRate());
			return errorCodeBadRequest;
		}
		mServerTransferAddress = "";
		if( requestUdp->udpServerAddress.empty() )
		{
			MLOG(ZQ::common::Log::L_DEBUG,SESSFMT(registerSession,"no udpServerAddress is specified, trying to allocated it"));
			std::vector<std::string> availPorts =  portManager.getAvailPorts(calcedBitrate, mSessionId);
			std::vector<std::string>::const_iterator it = availPorts.begin();
			for( ; it != availPorts.end(); it ++ )
			{
				if(portManager.registerSession( *it, mSessionId, calcedBitrate))
				{
					mServerTransferAddress = *it;
					break;
				}
			}
			if(mServerTransferAddress.empty())
			{
				updateLastError(requestUdp,responseUdp, errorCodeBadRequest,"failed to allocate any available port to this session");
				return errorCodeBadRequest;
			}
			else
			{
				MLOG(ZQ::common::Log::L_INFO,SESSFMT(registerSession,"allocated [%s] to this session"),mServerTransferAddress.c_str() );
			}
		}
		else
		{	
			if( !portManager.registerSession( requestUdp->udpServerAddress , mSessionId , calcedBitrate ) )
			{
				//FIXME: how to response if failed to register session into port manager ?
				updateLastError(requestUdp,responseUdp, errorCodeBadRequest,"failed to register session into portManager");
				return errorCodeBadRequest;
			}

			mServerTransferAddress = requestUdp->udpServerAddress; //copy requested transfer address into our record
		}

		responseUdp->confirmedUdpIp = mServerTransferAddress;
		//responseUdp->confirmedTransferPort = mEnv.getConfig().mLocalBindPort;
		mClientTransferAddress = "";
		
		clientManager.updateClient( requestUdp->udpClientAddress , requestUdp->ingressCapacity );
		if( !clientManager.registerSession( requestUdp->udpClientAddress , mSessionId , calcedBitrate) )
		{
			updateLastError(requestUdp, responseUdp , errorCodeBadRequest , "client[%s] has not enough bandwidth to serve the request", requestUdp->udpClientAddress.c_str() );
			return errorCodeBadRequest;
		}

		mClientTransferport = requestUdp->udpClientPort;
		mClientTransferAddress = requestUdp->udpClientAddress;
		mCurrentDataRunner->setTransferDelay(requestUdp->transferDelay);
	}
	else
		return errorCodeBadRequest;
	return errorCodeOK;
}

int32 C2Session::registerShadowSession( const TransferInitRequestParamPtr request, TransferInitResponseParamPtr response)
{
	MLOG(ZQ::common::Log::L_DEBUG, SESSFMT(registerShadowSession, "register shadow session with UpSide[local:<%s> peer: <%s:%hu> ]"),
			request->upStreamLocalIp.c_str(), request->upStreamPeerIp.c_str(), request->upStreamPeerPort);

	if(request->transferRate <=0) {
		updateLastError(request,response,errorCodeBadRequest,"bad transferRate[%ld]bps", request->transferRate );
	}

	mSessionType = request->requestType;

	int32 bitrateInflation = request->getConfWriter()->mBitrateInflationPercent;
	bitrateInflation = bitrateInflation < 80 ? 80 : bitrateInflation;
	bitrateInflation = bitrateInflation > 150 ? 150 : bitrateInflation;
	int32 calcedBitrate = request->transferRate * bitrateInflation / 100;

	mCurrentDataRunner->setTransferRate(calcedBitrate);
	mSessionTransferRate = calcedBitrate;

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

	int readerType = request->getConfUrlRule()->readerType;
	
	//if clientType ==2 (proxy mode), take Shadow Session as local session
	if( readerType == CLIENT_TYPE_DISKAIO && BASE_SESSION_TYPE(request->requestType) == SESSION_TYPE_SHADOW )
	{
		return registerShadowSession( request, response );
	}
	else if( BASE_SESSION_TYPE(request->requestType) != SESSION_TYPE_COMPLEX) 
	{
		C2SessFile sf(mEnv, readerType);
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
			updateLastError(request,response,errorCodeBadRange, SESSFMT(processTransferInit(),"range[%s] is not acceptable according to file data range[%lld-%lld]"),
					 reqRange.toString().c_str(), sf.dataStartPos() , sf.dataEndPos() );
			return errorCodeBadRange;
		}
		//now I know the request range
		mCurrentDataRunner->setTransferRange(reqRange); // copy request range into our record

		if( !mCurrentDataRunner->getTransferRange().bStartValid )
		{
			TransferRange tmpRange = mCurrentDataRunner->getTransferRange();
			tmpRange.bStartValid = true;
			tmpRange.startPos = sf.dataStartPos();
			mCurrentDataRunner->setTransferRange(tmpRange);
		}
		//mFileName						= sf.getFileFullPath();
		response->availRange			= composeFileRange( sf, readerType );
	}
		
	mCurrentDataRunner->setReqFilename( request->fileName );
	mRequestFileName = request->fileName;
	mSessionTransferRate 	= request->transferRate;
	mbMainFile						= request->isMainFile;
	if(mbMainFile) {
		mIndexFilePathname 			= request->indexFilePathname;
	}

	mRequestFileExts				= request->subFileExts;
	
	int32 retCode = registerSession( request , response);
	if( retCode != errorCodeOK ) return retCode;
	
	mTimeoutInterval		= request->transferTimeout;	

	IAttrBridge* iBridge 	= mEnv.getAttrBridge();
	if( iBridge && readerType == CLIENT_TYPE_DISKAIO )
		response->openForWrite	= iBridge->isFileBeingWritten( request->fileName , mSessionId );
	else
		response->openForWrite	= false;
	response->timeout		= request->transferTimeout;
	response->transferId	= constructResponseSessId( mSessionId );
	mSessionType			= request->requestType;

	//register session to filename pattern 
	MLOG.debug(SESSFMT(processTransferInit, "bind sess[%s] with filepath[%s]"), mSessionId.c_str(), request->fileName.c_str());
	mSvc.getSessManager().updateSess2Path(mSessionId, request->fileName);
	//mConfWriter 			= request->getConfWriter();
	//mConfUrlRule			= request->getConfUrlRule();


	/*
	{
		PortManager::PortAttr pa;
		if(! mSvc.getPortManager().getPortAttr( mServerTransferAddress, pa ) )
		{
			updateLastError(request,response,errorCodeInternalError,"failed to get mtu from portmanager for [%s]",
				mServerTransferAddress.c_str());
			return errorCodeInternalError;
		}
		mCurrentDataRunner->mEthMtu = pa.ethMtu;
		if( mEnv.getConfig().mSendPacketSize < (uint32)pa.ethMtu ) {
			mCurrentDataRunner->mEthMtu = mEnv.getConfig().mSendPacketSize;
		}
	}
	*/

	if( request->exposeIndex || readerType == CLIENT_TYPE_HYBRID ) {
		MLOG(ZQ::common::Log::L_INFO, SESSFMT(processTransferInit,"expose asset information"));
		AssetAttribute::Ptr attr = mSvc.getSessManager().getAssetAttribute( mSessionId, request->fileName, readerType );
		// add for async send buffer
	   if(attr)
	   {
			if (request->evtLoop == NULL)
			{
				  TransferInitCallbackForOther::Ptr sinker = new TransferInitCallbackForOther(mEnv, this, attr);
				  if ( attr->asyncWait( sinker )) {
						sinker->transferInitWait();
				  }
			}
			else //request->eventLoop != NULL
			{
				  TransferInitCallbackForLoop::Ptr sinker = new TransferInitCallbackForLoop(mEnv, this, request->evtLoop, response, attr);
				  if (attr->asyncWait( sinker) ) {
						return errorWorkingInProcess;
				  }
			}
			if( attr->lastError() == 0 ) {
				  response->baseinfo = attr->assetBaseInfo();
				  response->memberinfo = attr->assetMemberInfo();
				  response->openForWrite = attr->pwe();
				  mReaderType = attr->suggestedReaderType();
			} else {
				MLOG.error(SESSFMT(processTransferInit, "failed to get asset information for[%s]"), request->fileName.c_str());
				int lastError = attr->lastError();
				if(lastError == AssetAttribute::ASSET_NOTFOUND) {
					lastError = errorCodeContentNotFound;
				} else {
					lastError = errorCodeInternalError;
				}
				updateLastError(request, response, lastError, "processTransferInit() failed to query index info for [%s] due to [%d]", request->fileName.c_str(), lastError);
				return lastError;
			}
	   }
	   else
	   {
		   assert(false);
		   MLOG(ZQ::common::Log::L_WARNING, SESSFMT(processTransferInit,"failed to get asset information for[%s]"), request->fileName.c_str() );
	   }
	  /*
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
		}*/
	}

	MLOG(ZQ::common::Log::L_INFO, SESSFMT(processTransferInit, "successfully created session with filename[%s] bitrate[%d] delay[%d] range[%s] requestType[%d], mtu[%d]"),
		request->fileName.c_str() , request->transferRate , request->transferDelay, request->requestRange.toString().c_str() , 
		request->requestType, mCurrentDataRunner->getEthMtu());

	return errorCodeOK;
}

int32 C2Session::processUdpInit(const UdpStreamInitRequestParamPtr request , UdpStreamInitResponseParamPtr response)
{
	MLOG(ZQ::common::Log::L_DEBUG, SESSFMT(processUdpInit, "init udp session "));

	int readerType = request->getConfUrlRule()->readerType;
	
	int32 retCode = registerSession( request , response);
	if( retCode != errorCodeOK ) {
		return retCode;
	}
	
	mTimeoutInterval		= request->udpTimeOut;	
	response->timeout		= request->udpTimeOut;
	//aresponse->transferId	= constructResponseSessId( mSessionId );
	mSessionType			= request->requestType;
	mSessionTransferRate 	= request->transferRate;

	mConfWriter 			= request->getConfWriter();
	mReaderType 			= request->getConfUrlRule()->readerType;

	mUdpSessionState		= UDPSTATE_NONE;

	//LibAsync::EventLoop* evtLoop = request->evtLoop;
	LibAsync::EventLoop* evtLoop = response->responseHandler->getLoop();
	assert( evtLoop != NULL);
	mReachFileEndCb = new DataReadCallback(this, evtLoop, true);
	mDataReadCb = new DataReadCallback(this,evtLoop,true);
	mWritableCb = new HttpWritableCallback( this );
	mAsyncTimer = new C2SessionAsyncTimer(this, evtLoop);
	mAsyncStopRunner = new SessionStopRunnerAsyncWork(this, evtLoop);

	mCurrentDataRunner->updateSessProperty(request->sessProp);
	if(!response->responseHandler->setLocalAddr(mServerTransferAddress, 0)) {
		MLOG.info(SESSFMT(processTransferInit, "failed to set server addr[%s]"), mServerTransferAddress.c_str());
	}
	mResponseHandler = response->responseHandler;

	MLOG(ZQ::common::Log::L_INFO, SESSFMT(processTransferInit, "successfully create udp session"));
	return errorCodeOK;
}

}//namespace C2Streamer


