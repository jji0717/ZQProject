
#include <urlstr.h>
#include <time.h>
#include <TianShanIceHelper.h>
#include "./SetupHandler.h"
#include "./PenaltyManager.h"
#include "./HelperClass.h"
#include <TianShanIceHelper.h>

#include <TimeUtil.h>

SetupHandler::SetupHandler(NGODEnv& ssm, IStreamSmithSite* pSite, IClientRequestWriter* pReq) 
: RequestHandler(ssm, pSite, pReq)
,mIntention( _ssmNGODr2c1.mSelectionEnv,"","",""),
mSelStreamer( _ssmNGODr2c1.mSelectionEnv, _ssmNGODr2c1.mResManager , mIntention)
{
	_method = "SETUP";
	_inoutMap[MAP_KEY_METHOD] = _method;
}

SetupHandler::~SetupHandler()
{
}

RequestProcessResult SetupHandler::process()
{
	HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(SetupHandler, "start process"));
	if (false == _canProcess)
	{
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SetupHandler, "can't process the request, lastError[%s]"), szBuf );
		HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SetupHandler, "can't process the request, lastError[%s]"), szBuf );
		return RequestError;
	}

	// add by zjm for versioning
	if (_ngodConfig._MessageFmt.rtspNptUsage <= 0)
	{
		if (!handshake(_requireProtocol, 0, 2))
		{
			return RequestError;
		}
		// remeber require options
		char versionCode[8];
		sprintf(versionCode, "%d", _requireProtocol);
		_context.prop.insert(std::make_pair("RequireR2", versionCode));
		HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(SetupHandler, "Inserted R2 Require header in context[RequireR2 : %s]"), versionCode);
	}

	std::string			originalURI, strResourceURI;
	std::string			rtspServerIP, rtspServerPort;
	std::string			strRequestVolume;
	std::string			strStartPoint;

	rtspServerIP		= getRequestHeader("SYS#LocalServerIP");
	rtspServerPort		= getRequestHeader("SYS#LocalServerPort");
	originalURI			= getRequestHeader(ORIGINAL_URI);
	strResourceURI		= getRequestHeader(RESOURCE_URI);
	strStartPoint		= getRequestHeader(NGOD_START_POINT);


	IClientSession* pSession = _pSite->createClientSession(NULL, strResourceURI.c_str());
	if (NULL == pSession || NULL == pSession->getSessionID())
	{
		snprintf(szBuf, sizeof(szBuf) - 1, "create rtspProxy client session failed");
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SetupHandler, "%s"), szBuf);
		HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SetupHandler, "%s"), szBuf);
		responseError(RESPONSE_INTERNAL_ERROR);
		return RequestError;
	}
	//assign new session id to class member
	_session = pSession->getSessionID();

	mIntention.setCseq( _sequence );
	mIntention.setMethod( _method );
	mIntention.setSessId( _session );

	HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(SetupHandler, "created rtspProxy client session with URL: [%s]"), 
		strResourceURI.c_str());

	
	_inoutMap[MAP_KEY_SESSION] = _session;
	//HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(SetupHandler, "rtspProxy client session created successfully"));

	// DO: get OnDemandSessionID
	std::string onDemandID;
	onDemandID = getRequestHeader(NGOD_HEADER_ONDEMANDSESSIONID);
	_pResponse->setHeader(NGOD_HEADER_ONDEMANDSESSIONID, onDemandID.c_str());

	// get request Volume header
	//actually the key word 'volume' means nothing to me but used to select volume
	strRequestVolume			= getRequestHeader(NGOD_HEADER_VOLUME);

	int					startPointOffset = 0;
	int					startPointIdx = 0;
	bool				bHasStartPoint = false;
	if( !strStartPoint.empty() )
	{
		if( _ngodConfig._MessageFmt.rtspNptUsage >= 1)//enable rtsp npt usage
		{
			float ftmp = 0.0f;
			int itmp = 0;
			sscanf(strStartPoint.c_str(),"%d %f",&startPointIdx , &ftmp);
			startPointOffset = (int)(ftmp*1000);
		}
		else
		{
			if (_ngodConfig._protocolVersioning.enableVersioning > 0 &&
				_requireProtocol == NgodVerCode_R2_DecNpt)
			{
				float ftmp = 0.0f;
				int itmp = 0;
				sscanf(strStartPoint.c_str(),"%d %f",&startPointIdx , &ftmp);
				startPointOffset = (int)(ftmp*1000);

			}
			else
			{
				sscanf(strStartPoint.c_str() ,"%d %x",&startPointIdx, &startPointOffset);
			}
		}
		HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(SetupHandler, "requested StartPoint[index=%d, offset=%d]"), startPointIdx, startPointOffset);
		bHasStartPoint = true;
	}

	Ice::Int			iClientPort = 0;	//client port
	long				lbandwidth	= 0;	//needed bandwidth to pump the stream
	Ice::Int			iServerPort = 0;	//specify a server port to pump stream
	std::string			strSOP;				//sop name or sop group
	std::string			strDestIp;			//stream destination IP
	std::string			strDestMac;			//stream destination mac address
	std::string			sessionGroup;		//session group
	std::string			connectionID;		//current connection id


	//get transport field value
	std::string transportData = getRequestHeader( NGOD_HEADER_TRANSPORT );

	//parse transport detail such as SOP ,bandwidth ,client port ,client IP , client mac , server port
	getTransportDetail(		transportData, 
							strSOP, 
							lbandwidth, 
							iClientPort, 
							strDestIp,
							strDestMac, 
							iServerPort );

	std::string clientSessId = getRequestHeader( NGOD_HEADER_CLIENTSESSID ); //this is also poke hole session id
	std::string clientId = clientSessId.empty()? strDestMac : clientSessId;
	std::size_t pos = clientId.find_first_of("/\\");
	if (std::string::npos != pos)
		clientId.erase(pos);

	while (std::string::npos != (pos = clientId.find_first_of(" \t-:")))
		clientId.erase(pos, 1);

	if (clientId.length() >12)
		clientId.erase(12);
	
	//at here , all PID+PAID is prepared and ready to query LAM
	
	//we get a new rtsp session here and it's good to use
	#pragma message ( __MSGLOC__ "WARNING: Sentry should parse this message to publish event")
	getRequestHeader(NGOD_HEADER_ONDEMANDSESSIONID);
	
	if (0 != _ngodConfig._response._setupFailureWithSessId)
	{
		_pRequest->setHeader(NGOD_HEADER_SESSION, (char*)_session.c_str());
	}	

	// DO: get SessionGroup	
#pragma message ( __MSGLOC__ "WARNING: Sentry should parse this message to publish event")
	sessionGroup = getRequestHeader(NGOD_HEADER_SESSIONGROUP);
	
	// DO: Get connectionID	
	connectionID = getRequestHeader("SYS#ConnID");

	// DO: initialize a new session context
	_context.ident.name			= _session;
	_context.ident.category		= SERVANT_TYPE;
    _context.onDemandID			= onDemandID;
    _context.normalURL			= originalURI;
    _context.resourceURL		= strResourceURI;
    _context.connectID			= connectionID;
    _context.groupID			= sessionGroup;
	_context.expiration			= ZQTianShan::now() + (Ice::Long) (_ngodConfig._rtspSession._timeout) * 1000;
	_context.announceSeq		= 1; //initialize announce sequence to 1 because this is in setup stage

	_context.prop.insert(std::make_pair("RTSPServerIP", rtspServerIP));
	
	
	//clear private data map
	//why should I call it here is not clear ?
	_pdMap.clear();

	prepareNatInfo( clientSessId );

	//////////////////////////////////////////////////////////////////////////
	// DO: support encryption
	getContentBody();	
	const char* next_content = _requestBody.c_str();
	const char* pTemp = NULL;	
	pTemp = strstr( next_content , "a=X-motorola-ecm:" );
	//prepare encryption data here
	//Encryption data maybe not present here
	prepareEncryptionData( pTemp );

	//prepare TianShanIce SRM resource ,and this is to be used in create stream call
	

	//fill destination
	
	ZQTianShan::Util::updateResourceData<std::string>( mResMap , TianShanIce::SRM::rtEthernetInterface , "destIP" , strDestIp );
	ZQTianShan::Util::updateResourceData<Ice::Int>( mResMap , TianShanIce::SRM::rtEthernetInterface , "destPort" , (Ice::Int)iClientPort );
	if( _ngodConfig._PlaylistControl.ignoreDestMac == 0 )
	{		
		ZQTianShan::Util::updateResourceData<std::string>( mResMap , TianShanIce::SRM::rtEthernetInterface , "destMac" , strDestMac );
	}
	ZQTianShan::Util::updateResourceData<Ice::Int>( mResMap , TianShanIce::SRM::rtEthernetInterface , "srcPort" , (Ice::Int)iServerPort );
	
	
	std::string streamingSourceInfo = "";

	//long						maxBW				= 0;
	bool						bNeedTry			= false;
	DWORD						dwCreateStreamTime	= GetTickCount();	
	int							iRetryCount			= 0;		
	bool						bMaybeNeedCache		= false;

	//hard code for 'library' ??
	if ( stricmp( strRequestVolume.c_str() , "library" ) == 0 )
	{//request volume is library ,so treat it as remote mode		
		bMaybeNeedCache = true;
	}
	else
	{
		bMaybeNeedCache = false;
	}


	//prepare asset information such as cuein cueout or stream control restriction flag	

	if ( !prepareAssetInfo( strResourceURI, clientId) ) 
	{
		HANDLERLOG(ZQ::common::Log::L_ERROR , HANDLERLOGFMT(SetupHandler,"can't prepare asset item infomation from url[%s] "),strResourceURI.c_str()	);		
		HANDLEREVENTLOG(ZQ::common::Log::L_ERROR , HANDLERLOGFMT(SetupHandler,"can't prepare asset item infomation from url[%s] "), strResourceURI.c_str()	);	
		responseError( RESPONSE_INVALID_PARAMETER );		
		return RequestError;
	}
	
	// translate log into ssm_ngod2_sentry.log by zjm
	HANDLEREVENTLOG(ZQ::common::Log::L_INFO , HANDLERLOGFMT(SetupHandler,
		"SessionHistoryLog ClientSessionId(%s)OnDemandSessionId(%s)SOP(%s)Destination(%s)Port(%d)PAID(%s)Bandwidth(%ld)"), 
		clientSessId.c_str(), onDemandID.c_str(), strSOP.c_str(), strDestIp.c_str(), iClientPort, mFirstPaid.c_str(), lbandwidth);

	SelectIntentionParam& para = mIntention.getParameter();
	para.identifier	= strSOP;
	para.groupName	= strSOP;
	para.requestBW	= lbandwidth;
	
	if( !mSelStreamer.findFirstStreamer() )
	{
		HANDLERLOG(ZQ::common::Log::L_ERROR , HANDLERLOGFMT(SetupHandler,"failed to get available streamer ") );
		return RequestError;
	}	
	
	// translate log into ssm_ngod2_sentry.log by zjm	
    HANDLEREVENTLOG(ZQ::common::Log::L_INFO , HANDLERLOGFMT(SetupHandler, "SessionHistoryLog Bandwidth(%ld)"), lbandwidth);

	//fill bandwidth
	ZQTianShan::Util::updateResourceData<Ice::Long>( mResMap , TianShanIce::SRM::rtTsDownstreamBandwidth , "bandwidth" , (Ice::Long)mSelStreamer.getAdjustedBandwidth() );

	bool bSkipVolume = false;
	bool bAddPenalty = false;
	
	for( iRetryCount=0; iRetryCount <= _sopConfig._sopRestrict._retryCount ;iRetryCount++ )
	{
		if( !mSelStreamer.findNextStreamer( bSkipVolume , bAddPenalty ) )
		{
			HANDLERLOG(ZQ::common::Log::L_ERROR , HANDLERLOGFMT(SetupHandler,"failed to get available streamer ") );
			responseError( errorCodeTransformer(mSelStreamer.getLastError()	) );
			return RequestError;
		}
		bSkipVolume = false;
		bAddPenalty = false;

		//prepare streamerId so that we can pass it to stream service to identify which streamer we want to use
		ZQTianShan::Util::updateResourceData<std::string>( mResMap,TianShanIce::SRM::rtStreamer, "NetworkId" , mSelStreamer.getSelectedStreamerNetId() );
		
		if( !createStream() )
		{
			bSkipVolume = false;
			bAddPenalty = true;
			continue;
		}
		//render playlist
		if( !renderPlaylist( _context.setupInfos ) )
		{
			bSkipVolume = true;
			bAddPenalty = false;
			continue;
		}
		_context.streamNetId		= mSelStreamer.getSelectedStreamerNetId();
		_context.sopname			= strSOP;
		_context.importChannelName	= mSelStreamer.getSelectedImportChannelName();

		try
		{
			_context.streamFullID = _ssmNGODr2c1._pCommunicator->proxyToString( mStreamPrx );
			_context.streamShortID = mStreamPrx->getIdent().name;
		}
		catch (const Ice::Exception& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "caught exception[%s] when reading stream instance", ex.ice_name().c_str());
			HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), szBuf);
			HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), szBuf);
			//responseError(RESPONSE_INTERNAL_ERROR);
			continue;
		}
		{
			try
			{
				//get source ip and port
				TianShanIce::ValueMap value;
				if(!mStreamPrx->getInfo( TianShanIce::Streamer::infoSTREAMSOURCE , value))
				{
					/*iRetryCount ++;	*/			
					HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SetupHandler, "failed to read source IP and port from stream instance" ));
					HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SetupHandler, "failed to read source IP and port from stream instance" ));
					continue;
				}				
				std::string sourceIp ;
				Ice::Int	sourcePort;
				HelperClass::getValueMapData( value , "StreamingSourceIp" , sourceIp );
				HelperClass::getValueMapData( value , "StreamingSourcePort", sourcePort );
				HANDLERLOG(ZQ::common::Log::L_INFO , HANDLERLOGFMT(SetupHandler,"got streaming source IP[%s], port[%d] from stream instance"),
					sourceIp.c_str() , sourcePort );
				char szStreamSourceBuf[256]={0};
				snprintf( szStreamSourceBuf,sizeof(szStreamSourceBuf)-1,"source=%s;server_port=%d", sourceIp.c_str() , sourcePort );
				streamingSourceInfo = szStreamSourceBuf;
			}
			catch( const TianShanIce::BaseException& ex)
			{
				//iRetryCount ++;				
				HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SetupHandler, "failed to get streaming source IP and port :exception[%s] %s" ), ex.ice_name().c_str(), ex.message.c_str());
				HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SetupHandler, "failed to get streaming source IP and port :exception[%s] %s" ), ex.ice_name().c_str(), ex.message.c_str());
				continue;
			}
			catch( const Ice::Exception& ex)
			{
				//iRetryCount ++;				
				HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SetupHandler, "failed to get streaming source IP and port : exception[%s]" ) ,ex.ice_name().c_str());
				HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SetupHandler, "failed to get streaming source IP and port : exception[%s]" ) ,ex.ice_name().c_str());
				continue;
			}
		}
		break;
	}

	if( bHasStartPoint )
	{
		startPointIdx = startPointIdx < 0 ? 0 : startPointIdx;
		int itemCount = static_cast<int>(_context.setupInfos.size());
		startPointIdx = startPointIdx < itemCount ? startPointIdx : (itemCount - 1);
		startPointOffset = startPointOffset < 0 ? 0 : startPointOffset;
		startPointOffset -= static_cast<int>( _context.setupInfos[startPointIdx].inTimeOffset );

		HANDLERLOG(ZQ::common::Log::L_INFO,HANDLERLOGFMT(SetupHandler,"adjusted StartPoint to [%d][%d] per inTimeOffset[%lld]"),
			startPointIdx , startPointOffset, _context.setupInfos[startPointIdx].inTimeOffset );

		ZQTianShan::Util::updatePropertyData<int>(_context.prop,REQUEST_STARTPOINT_IDX,startPointIdx);
		ZQTianShan::Util::updatePropertyData<int>(_context.prop,REQUEST_STARTPOINT_OFFSET,startPointOffset);
	}	

	// add by zjm to support session history
	if (_ngodConfig._sessionHistory.enableHistory > 0)
	{
		_context.prop.insert(std::make_pair("setupDate", NgodUtilsClass::generatorISOTime()));
		_context.prop.insert(std::make_pair("sessionGroup", getRequestHeader(NGOD_HEADER_SESSIONGROUP)));
		_context.prop.insert(std::make_pair("firstPlay", "true"));		
	}

	//Add bandwidth into Context
	_context.usedBandwidth		= mSelStreamer.getAdjustedBandwidth();

	if (false == addContext())
	{
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SetupHandler, "failed to add context into db" ));
		HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SetupHandler, "failed to add context into db" ));
		responseError(RESPONSE_INTERNAL_ERROR);			
		return RequestError;
	}

#pragma message ( __MSGLOC__ "WARNING: Sentry should parse this message to publish event")
	// translate log into ssm_ngod2_sentry.log by zjm
	//HANDLERLOG(ZQ::common::Log::L_INFO , HANDLERLOGFMT(SetupHandler, "SessionHistoryLog Streamer(%s)"), _context.streamNetId.c_str());
	HANDLEREVENTLOG(ZQ::common::Log::L_INFO , HANDLERLOGFMT(SetupHandler, "SessionHistoryLog Streamer(%s)Bandwidth(%ld)PAID(%s)"), 
		_context.streamNetId.c_str(), lbandwidth , mFirstPaid.c_str() );
	
	updateCounters( );

	_ssmNGODr2c1._pSessionWatchDog->watchSession( _context.ident, (long) (_ngodConfig._rtspSession._timeout) * 1000);

	mSelStreamer.commit();

//	pRenewCmd->start();

	//get streaming source ip and port
	transportData = transportData + ";";
	transportData = transportData + streamingSourceInfo;
	
	char stampUTP[20];
	memset(stampUTP, 0, 20);
	time_t tnow = time(NULL);
	__int64 seconds1900 = __int64(tnow) + 2208988800;
	snprintf(stampUTP, 19, "%lld", seconds1900);

	std::string retContent;
	retContent = "v=0\r\n";
	retContent += std::string("o=- ") + _session + " " + stampUTP + " IN IP4 " + rtspServerIP + "\r\n";
	retContent += "s=\r\n";
	retContent += "c=IN IP4 " + rtspServerIP +"\r\n";
	retContent += "t=0 0\r\n";	
	if( stricmp( _ngodConfig._response._streamCtrlProt.c_str() ,"lscp") == 0 )
	{
		ZQ::common::Variant var;
		_ssmNGODr2c1.getCoreInfo(IStreamSmithSite::INFORMATION_LSCP_PORT , var);
		char szLscpPort[256];
		szLscpPort[ sizeof(szLscpPort)-1 ] = 0;
		snprintf(szLscpPort,sizeof(szLscpPort)-1,"%d",(int32)var);
		retContent += std::string("a=control:lscp://") + rtspServerIP + ":" + szLscpPort + "/" + _session + "\r\n";		
	}
	else
	{
		retContent += std::string("a=control:rtsp://") + rtspServerIP + ":" + rtspServerPort + "/" + _session + "\r\n";

	}
	
	//HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(SetupHandler,"preparing response"));
	// DO: Response OK	
	_pRequest->setHeader(NGOD_HEADER_SESSION, (char*)_session.c_str());
	_pResponse->setHeader(NGOD_HEADER_TRANSPORT, transportData.c_str());
	_pResponse->setHeader(NGOD_HEADER_CONTENTTYPE, getRequestHeader(NGOD_HEADER_CONTENTTYPE).c_str());
	_pResponse->printf_postheader(retContent.c_str());
	responseOK();
	return RequestProcessed;
}

void	SetupHandler::getTransportDetail( const std::string& strTransportdata,
								std::string& strSOP	,	long& lbandwidth ,		int& iClientPort , 
								std::string& strDestIp,	std::string& strDestMac,int& iServerPort )
{
	std::vector<std::string> Transports;
	ZQ::StringOperation::splitStr(strTransportdata, ";", Transports);
	int Transports_size = Transports.size();
	for (int cur = 0; cur < Transports_size; cur++)
	{
		std::string left, right;
		left = ZQ::StringOperation::getLeftStr(Transports[cur],"=");
		right = ZQ::StringOperation::getRightStr(Transports[cur],"=");
		if (left.empty() || right.empty())
			continue;
		
		if (left == "client_port")
		{
			iClientPort =	atoi(right.c_str());			
		}
		else if (left == "bandwidth")
		{
			lbandwidth = atol( right.c_str() );
			continue;
		}
		else if (left == "sop_name" || left == "sop_group")
		{
			strSOP = right;
		}
		else if (left == "destination") 
		{
			strDestIp = right;
		}
		else if(left == "client")
		{
			strDestMac = right;
		}
		else if(left == "source")
		{
			iServerPort = atoi(right.c_str());
		}
	}
}

bool SetupHandler::createStream(  )
{
	if ( mStreamPrx != NULL ) 
	{
		try
		{
			mStreamPrx->destroy();			
		}
		catch (...) 
		{
		}
		mStreamPrx = NULL;
	}

	std::string streamServicePrxStr =  _ssmNGODr2c1._pCommunicator->proxyToString( mSelStreamer.getStreamerProxy() ).c_str() ;
	try
	{
		Ice::Context ctx;
		ctx["CLIENTSESSIONID"] = _context.onDemandID;
		HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(SetupHandler,"creating stream on[%s]"), streamServicePrxStr.c_str());
		
		mStreamPrx = TianShanIce::Streamer::PlaylistPrx::uncheckedCast( mSelStreamer.getStreamerProxy()->createStreamByResource( mResMap , ctx ) );

		if(!mStreamPrx)
		{			
			HANDLERLOG(ZQ::common::Log::L_WARNING, HANDLERLOGFMT(SetupHandler,"failed to create stream on [%s]"), streamServicePrxStr.c_str());
			return false;
		}	
		HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(SetupHandler,"successfully created stream[%s] on [%s]"),
			_ssmNGODr2c1._pCommunicator->proxyToString(mStreamPrx).c_str() ,streamServicePrxStr.c_str());
	}
	catch (const TianShanIce::BaseException& ex) 
	{		
		HANDLERLOG(ZQ::common::Log::L_WARNING, HANDLERLOGFMT(SetupHandler,"caught exception [%s]  when  create stream on[%s]"),
			ex.message.c_str(), streamServicePrxStr.c_str());
		return false;
	}
	catch( const Ice::ConnectFailedException& ex)
	{
		HANDLERLOG(ZQ::common::Log::L_WARNING, HANDLERLOGFMT(SetupHandler,"caught exception [%s]  when create stream on [%s]"),
			ex.ice_name().c_str(), streamServicePrxStr.c_str() );
		return false;
	}
	catch( const Ice::ConnectionLostException& ex)
	{
		HANDLERLOG(ZQ::common::Log::L_WARNING, HANDLERLOGFMT(SetupHandler,"caught exception [%s]  when create stream on [%s]"),
			ex.ice_name().c_str(), streamServicePrxStr.c_str());		
		return false;
	}
	catch( const Ice::ConnectTimeoutException& ex)
	{
		HANDLERLOG(ZQ::common::Log::L_WARNING, HANDLERLOGFMT(SetupHandler,"caught exception [%s]  when create stream on [%s]"),
			ex.ice_name().c_str(), streamServicePrxStr.c_str() );		
		return false;
	}
	catch (const Ice::Exception& ex) 
	{		
		HANDLERLOG(ZQ::common::Log::L_WARNING, HANDLERLOGFMT(SetupHandler,"caught exception [%s] when create stream on [%s]"),
			ex.ice_name().c_str(), streamServicePrxStr.c_str());
		return false;
	}
	return true;
}
std::string SetupHandler::prepareNatInfo( const std::string& pokeHoleSessId )
{
	if ( !pokeHoleSessId.empty() )
	{
		ZQTianShan::Util::updateResourceData<Ice::Int>( mResMap , TianShanIce::SRM::rtEthernetInterface , "natPenetrating", (Ice::Int)1 );
		ZQTianShan::Util::updateResourceData<std::string>( mResMap , TianShanIce::SRM::rtEthernetInterface , "pokeholeSession", pokeHoleSessId );
		HANDLERLOG(ZQ::common::Log::L_INFO , HANDLERLOGFMT(SetupHandler,"enable NAT penetrating, PokeHoleSession[%s]"), pokeHoleSessId.c_str() );		
		return pokeHoleSessId;
	}
	else
	{
		return std::string("");
	}
}

void SetupHandler::updateCounters()
{
	ZQ::common::MutexGuard gd( _ssmNGODr2c1._lockSopMap );

	ZQ::common::Config::Holder<NGOD2::SOPRestriction>&	sopRes = _sopConfig._sopRestrict;
	std::map<std::string , NGOD2::SOPRestriction::SopHolder>::iterator itSop = sopRes._sopDatas.find( _context.sopname );
	if( itSop == sopRes._sopDatas.end() )
	{
		return;		
	}
	NGOD2::SOPRestriction::SopHolder& sopHolderData = itSop->second;
	std::vector<NGOD2::Sop::StreamerHolder>& streamers = sopHolderData._streamerDatas;
	std::vector<NGOD2::Sop::StreamerHolder>::iterator itStreamer = streamers.begin();
	for( ; itStreamer != streamers.end() ; itStreamer++ )
	{
		if( itStreamer->_netId == _context.streamNetId )
		{//find it
			if (itStreamer->_usedSession < LLONG_MAX)
			{
				itStreamer->_usedSession++;
			}
			else
			{
				itStreamer->_usedSession = 1;
				itStreamer->_failedSession = 1;
			}
			itStreamer->_setupCountersTotal++;
			if( mbRemoteSession )
			{
				itStreamer->_setupCountersForRemote ++;
			}
		}
	}
}

bool SetupHandler::prepareAssetInfo( const std::string& strURL , const std::string& clientId )
{
	ZQ::common::URLStr uri( strURL.c_str(), true );
	char szItemBuf[256];
	int iItemCount = 0;
	
	SelectIntentionParam&	para = mIntention.getParameter();
//SetupHandler::ItemInfoS& infos = _handler._itemInfos;
	const char* pID = NULL;
	do 
	{
		sprintf(szItemBuf,"item%d",iItemCount);		pID	=	uri.getVar(szItemBuf);
		sprintf(szItemBuf,"cueIn%d",iItemCount);	const char* pCuein = uri.getVar(szItemBuf);
		sprintf(szItemBuf,"cueOut%d",iItemCount);	const char* pCueout = uri.getVar(szItemBuf);
		sprintf(szItemBuf, "DisableF%d", iItemCount);const char* pDisableF = uri.getVar(szItemBuf);
		sprintf(szItemBuf, "DisableR%d", iItemCount);const char* pDisableR = uri.getVar(szItemBuf);
		sprintf(szItemBuf, "DisableP%d", iItemCount);const char* pDisableP = uri.getVar(szItemBuf);

		if ( pID && pID[0] != 0 ) 
		{
			SelectIntentionParam::PlaylistItemInfo info;			
			if ( pCuein && strlen(pCuein)>0 )
				info.cuein = atoi(pCuein);
			if ( pCuein && strlen(pCueout)>0 ) 
				info.cueout = atoi(pCueout);
			if (pDisableF && atoi(pDisableF) == 1)
				info.restrictionFlag += TianShanIce::Streamer::PLISFlagNoFF;
			if (pDisableR && atoi(pDisableR) == 1)
				info.restrictionFlag += TianShanIce::Streamer::PLISFlagNoRew;
			if (pDisableP && atoi(pDisableP) == 1)
				info.restrictionFlag += TianShanIce::Streamer::PLISFlagNoPause;
			//chop the id into pid and paid
			const char* pDelimiter = strstr( pID , "#");
			if( pDelimiter )
			{
				info.pid.assign( pID , pDelimiter - pID);
				info.paid.assign( pDelimiter+1 );
			}
			else
			{
				info.pid = pID;
				info.paid.clear();				
				//add also log an error here
				char szLocalBuf[1024];
				szLocalBuf[sizeof(szLocalBuf)-1] = 0;
				snprintf(szLocalBuf,sizeof(szLocalBuf)-1,HANDLERLOGFMT(SetupHandler, "invalid PID PAID[%s]"), pID);
				std::string notice_str;
				notice_str = NgodUtilsClass::generatorNoticeString(NGOD_ANNOUNCE_SERVER_RESOURCES_UNAVAILABLE, NGOD_ANNOUNCE_SERVER_RESOURCES_UNAVAILABLE_STRING);
				setResponseString(RESPONSE_SSF_INVALID_REQUEST, notice_str.c_str());
				return	false;
			}	
			info.sid	= clientId;			
			para.playlist.push_back(info);
		}		
		iItemCount++;		
	} while( pID && strlen(pID)> 0 );
	
	if( para.playlist.size() > 0 )
	{
		mFirstPaid = para.playlist[0].paid;
	}

	return para.playlist.size() > 0 ;
}

void SetupHandler::fillEncryptionData( const std::string&	volumeName,	const std::string&	 strProviderId, const std::string&	strAssetId,	
									  ::TianShanIce::ValueMap&	infoPrivateData )
{
	//prepare encryption data from each item
	std::string strProviderAssetId = strProviderId + "#" + strAssetId;
	SetupHandler::ECMDataMAP::iterator itEcm = mEcmDatas.find ( strProviderAssetId );
	if (itEcm != mEcmDatas.end ())
	{
		ZQTianShan::Util::updateValueMapData( infoPrivateData, "Tianshan-ecm-data:programNumber" , ( Ice::Int )itEcm->second.iProgNum );
		ZQTianShan::Util::updateValueMapData( infoPrivateData , "Tianshan-ecm-data:Freq_1" , (Ice::Int)itEcm->second.iFrq1 );

		int keyNum =  (int)itEcm->second.vecKeyOffset.size();//atoi(temp_strs[4].c_str());
		if (keyNum > 0/* && (int)temp_strs.size() >= 5 + keyNum * 2*/)
		{
			ZQTianShan::Util::updateValueMapData( infoPrivateData, "Tianshan-ecm-data:keyoffsets" ,itEcm->second.vecKeyOffset, false  );
			ZQTianShan::Util::updateValueMapData( infoPrivateData, "Tianshan-ecm-data:keys" ,itEcm->second.vecKeys, false  );
			ZQTianShan::Util::updateValueMapData( infoPrivateData, "Tianshan-ecm-data:preEncryption-Enable", (Ice::Int)1 );
			ZQTianShan::Util::updateValueMapData( infoPrivateData, "TianShan-flag-pauselastuntilnext", (Ice::Int)1 );
			ZQTianShan::Util::updateValueMapData( infoPrivateData, "library", volumeName );			
		}
		else 
		{
			HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RenderPlaylistCommand, "content format error for encription"));
			HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RenderPlaylistCommand, "content format error for encription"));
		}		
	}
	else 
	{
		HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(RenderPlaylistCommand, "no Encryption data was found with [%s]"),
			strProviderAssetId.c_str());
	}
}

bool SetupHandler::renderPlaylist( NGODr2c1::PlaylistItemSetupInfos& setupInfos )
{
	setupInfos.clear();

	mbRemoteSession = false;

	//check the volume information
	//playlistInfo
	std::string		streamPrxStr = _ssmNGODr2c1._pCommunicator->proxyToString( mSelStreamer.getStreamerProxy() );
	HANDLERLOG(ZQ::common::Log::L_INFO,HANDLERLOGFMT(RenderPlaylistCommand,"rendering playlist items onto streamer[%s]"),streamPrxStr.c_str() );
	
	const ElementInfoS& plInfos = mSelStreamer.getElements();	
	int32			iTotalElementCount =(int32) plInfos.size();
	int32			currentElementIndex = 1; //used as CtrlNum based on 1

	std::string notice_str;
	notice_str = NgodUtilsClass::generatorNoticeString(NGOD_ANNOUNCE_INTERNAL_SERVER_ERROR, NGOD_ANNOUNCE_INTERNAL_SERVER_ERROR_STRING);

	for( ElementInfoS::const_iterator itElement = plInfos.begin() ; itElement != plInfos.end() ; itElement ++ )
	{
		TianShanIce::Streamer::PlaylistItemSetupInfo setupInfo;
		setupInfo.privateData.clear();
		fillEncryptionData( mSelStreamer.getSelectedVolumeName() , itElement->pid , itElement->paid, setupInfo.privateData );

		setupInfo.contentName			=	itElement->fullContentName( mSelStreamer.getSelectedVolumeName() );
		setupInfo.criticalStart			=	0;
		setupInfo.inTimeOffset			=	itElement->cueIn;
		setupInfo.outTimeOffset			=	itElement->cueOut;
		setupInfo.flags					=	itElement->flags;
		setupInfo.spliceIn				=	false;
		setupInfo.spliceOut				=	false;
		setupInfo.forceNormal			=	false;

		//
		ZQTianShan::Util::updateValueMapData( setupInfo.privateData , "providerId" , itElement->pid );
		ZQTianShan::Util::updateValueMapData( setupInfo.privateData , "providerAssetId" , itElement->paid );
		
		const VolumeAttrEx& volAttr = mSelStreamer.getSelectedVolumeAttr();
		if( !volAttr.bLocalPlaylist )
		{			
			ZQTianShan::Util::updateValueMapData( setupInfo.privateData , "storageLibraryUrl" , itElement->urls, false );
			HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(RenderPlaylistCommand,"add url[%s] to item[%s][%d]"),
				ZQTianShan::Util::dumpTianShanIceStrValues(itElement->urls).c_str() , setupInfo.contentName.c_str(),currentElementIndex);						
		}
		else
		{
			HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(RenderPlaylistCommand,"do not add url to item[%s][%d] per supportNasStream[%s]"),					
				setupInfo.contentName.c_str(),currentElementIndex ,	volAttr.bSupportNas ? "true":"false" );
			setupInfo.privateData.erase("storageLibraryUrl");
		}
		if ( iTotalElementCount == currentElementIndex + 1 )
		{
			setupInfo.privateData.erase("TianShan-flag-pauselastuntilnext");
		}

		try
		{
			mStreamPrx->pushBack( currentElementIndex , setupInfo);
			setupInfos.push_back( setupInfo ); //
		}
		catch( const TianShanIce::InvalidParameter& ex )
		{
			char szLocalBuf[1024];
			szLocalBuf[sizeof(szLocalBuf)-1] = 0;
			snprintf(szLocalBuf,sizeof(szLocalBuf)-1, HANDLERLOGFMT(RenderPlaylistCommand , "caught exception:[%s] when pushback items onto playlist[%s]" ),
				ex.message.c_str() ,
				streamPrxStr.c_str() );
			HANDLERLOG(ZQ::common::Log::L_ERROR , szLocalBuf );
			setResponseString( RESPONSE_SSF_ASSET_NOT_FOUND , notice_str.c_str());
			return false;
		}
		catch (const TianShanIce::BaseException& ex) 
		{
			char szLocalBuf[1024];
			szLocalBuf[sizeof(szLocalBuf)-1] = 0;
			snprintf(szLocalBuf,sizeof(szLocalBuf)-1, HANDLERLOGFMT(RenderPlaylistCommand, "caught exception:[%s] when pushback items onto playlist[%s]" ),
				ex.message.c_str() ,
				streamPrxStr.c_str() );
			HANDLERLOG(ZQ::common::Log::L_ERROR, szLocalBuf );
			setResponseString( RESPONSE_INTERNAL_ERROR , notice_str.c_str());
			return false;
		}
		catch( const Ice::ConnectFailedException& ex)
		{
			char szLocalBuf[1024];
			szLocalBuf[sizeof(szLocalBuf)-1] = 0;
			snprintf(szLocalBuf,sizeof(szLocalBuf)-1, HANDLERLOGFMT(RenderPlaylistCommand , "caught exception:[%s] when pushback item onto playlist[%s]" ),
				ex.ice_name().c_str() ,
				streamPrxStr.c_str()  );
			HANDLERLOG(ZQ::common::Log::L_ERROR , szLocalBuf );
			setResponseString( RESPONSE_INTERNAL_ERROR , notice_str.c_str());			
			return false;
		}
		catch( const Ice::ConnectionLostException& ex)
		{
			char szLocalBuf[1024];
			szLocalBuf[sizeof(szLocalBuf)-1] = 0;
			snprintf(szLocalBuf,sizeof(szLocalBuf)-1, HANDLERLOGFMT(RenderPlaylistCommand , "caught exception:[%s] when pushback items onto playlist[%s]" ),
				ex.ice_name().c_str() ,
				streamPrxStr.c_str()  );
			HANDLERLOG(ZQ::common::Log::L_ERROR , szLocalBuf );
			setResponseString( RESPONSE_INTERNAL_ERROR , notice_str.c_str());			
			return false;
		}
		catch( const Ice::ConnectTimeoutException& ex)
		{
			char szLocalBuf[1024];
			szLocalBuf[sizeof(szLocalBuf)-1] = 0;
			snprintf(szLocalBuf,sizeof(szLocalBuf)-1, HANDLERLOGFMT(RenderPlaylistCommand , "caught exception:[%s] when pushback item onto playlist[%s]" ),
				ex.ice_name().c_str() ,
				streamPrxStr.c_str()  );
			HANDLERLOG(ZQ::common::Log::L_ERROR , szLocalBuf );
			setResponseString( RESPONSE_INTERNAL_ERROR , notice_str.c_str());
			return false;
		}
		catch (const Ice::Exception& ex) 
		{
			char szLocalBuf[1024];
			szLocalBuf[sizeof(szLocalBuf)-1] = 0;
			snprintf(szLocalBuf,sizeof(szLocalBuf)-1, HANDLERLOGFMT(RenderPlaylistCommand , "caught exception:[%s] when pushback item onto playlist[%s]" ),
				ex.ice_name().c_str() ,
				streamPrxStr.c_str()  );
			HANDLERLOG(ZQ::common::Log::L_ERROR,szLocalBuf);
			setResponseString( RESPONSE_INTERNAL_ERROR , notice_str.c_str());

			HANDLERLOG(ZQ::common::Log::L_ERROR , szLocalBuf );
			return false;
		}
		catch (...) 
		{
			char szLocalBuf[1024];
			szLocalBuf[sizeof(szLocalBuf)-1] = 0;
			snprintf(szLocalBuf,sizeof(szLocalBuf)-1,HANDLERLOGFMT(RenderPlaylistCommand , "caught unknown exception when pushback item into playlist[%s]" ),					
				streamPrxStr.c_str() );
			HANDLERLOG(ZQ::common::Log::L_ERROR,szLocalBuf);
			setResponseString( RESPONSE_INTERNAL_ERROR , notice_str.c_str() );

			return false;
		}
		++currentElementIndex;
	}
	

	//bool bEnableEOT	= _handler._ssmNGODr2c1._playlistControlEnableEOT;
	bool bEnableEOT		=	_ngodConfig._PlaylistControl.enableEOT >= 1;
	try
	{
		mStreamPrx->enableEoT( bEnableEOT );
		if (!bEnableEOT)
			HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(RenderPlaylistCommand, "disabled EOT onto playlist"));
		mStreamPrx->commit( );		
	}
	catch( const TianShanIce::BaseException& ex )
	{
		char szLocalBuf[1024];
		szLocalBuf[sizeof(szLocalBuf)-1] = 0;
		snprintf(szLocalBuf,sizeof(szLocalBuf)-1 , HANDLERLOGFMT(RenderPlaylistCommand,"caught exception[%s] when invoke enableEOT/commit"),
			ex.message.c_str() );
		HANDLERLOG(ZQ::common::Log::L_ERROR,szLocalBuf);

		if( ex.errorCode == EXT_ERRCODE_BANDWIDTH_EXCEEDED )
		{
			setResponseString( RESPONSE_SSF_NO_NETWORKBANDWIDTH , szLocalBuf );
		}
		else
		{
			setResponseString( RESPONSE_INTERNAL_ERROR , szLocalBuf );
		}

		return false;
	}
	catch( const Ice::ConnectFailedException& ex)
	{
		char szLocalBuf[1024];
		szLocalBuf[sizeof(szLocalBuf)-1] = 0;
		snprintf(szLocalBuf,sizeof(szLocalBuf)-1, HANDLERLOGFMT(RenderPlaylistCommand,"caught exception[%s] when invoke enableEOT/commit"),
			ex.ice_name().c_str() );
		HANDLERLOG(ZQ::common::Log::L_ERROR,szLocalBuf);
		setResponseString(RESPONSE_INTERNAL_ERROR, notice_str.c_str());
	}
	catch( const Ice::ConnectionLostException& ex)
	{
		char szLocalBuf[1024];
		szLocalBuf[sizeof(szLocalBuf)-1] = 0;
		snprintf(szLocalBuf,sizeof(szLocalBuf)-1 , HANDLERLOGFMT(RenderPlaylistCommand,"caught exception[%s] when invoke enableEOT/commit"),
			ex.ice_name().c_str() );
		HANDLERLOG(ZQ::common::Log::L_ERROR,szLocalBuf);
		setResponseString(RESPONSE_INTERNAL_ERROR, notice_str.c_str());
	}
	catch( const Ice::ConnectTimeoutException& ex)
	{
		char szLocalBuf[1024];
		szLocalBuf[sizeof(szLocalBuf)-1] = 0;
		snprintf(szLocalBuf,sizeof(szLocalBuf)-1 , 
			HANDLERLOGFMT(RenderPlaylistCommand,"caught exception[%s] when invoke enableEOT/commit"),
			ex.ice_name().c_str() );
		HANDLERLOG(ZQ::common::Log::L_ERROR,szLocalBuf);
		setResponseString(RESPONSE_INTERNAL_ERROR, notice_str.c_str());
	}
	catch(const Ice::Exception& ex)
	{
		char szLocalBuf[1024];
		szLocalBuf[sizeof(szLocalBuf)-1] = 0;
		snprintf(szLocalBuf,sizeof(szLocalBuf)-1 , 
			HANDLERLOGFMT(RenderPlaylistCommand,"caught exception[%s] when invoke enableEOT/commit"),
			ex.ice_name().c_str() );
		HANDLERLOG(ZQ::common::Log::L_ERROR,szLocalBuf);
		setResponseString(RESPONSE_INTERNAL_ERROR, notice_str.c_str());
		return false;
	}
	catch(...)
	{
		char szLocalBuf[1024];
		szLocalBuf[sizeof(szLocalBuf)-1] = 0;
		snprintf(szLocalBuf,sizeof(szLocalBuf)-1 , 
			HANDLERLOGFMT(RenderPlaylistCommand,"caught unknown exception when invoke enableEOT/commit"));
		HANDLERLOG(ZQ::common::Log::L_ERROR,szLocalBuf);
		setResponseString(RESPONSE_INTERNAL_ERROR, notice_str.c_str());		
		return false;
	}

	return (currentElementIndex > 0);
}

bool SetupHandler::prepareEncryptionData( const char*	pContent  )
{
	if(! (pContent && pContent[0] ) )
	{
		return false;
	}

	mEcmDatas.clear( );

	std::string notice_str;
	notice_str = NgodUtilsClass::generatorNoticeString(NGOD_ANNOUNCE_UNABLE_ENCRPT, NGOD_ANNOUNCE_UNABLE_ENCRPT_STRING);
	const char*		next_content = NULL;
	
	while ( pContent != NULL )
	{
		pContent += strlen("a=X-motorola-ecm:");
		next_content = pContent;
		std::vector<std::string> temp_strs;
		ZQ::StringOperation::splitStr(pContent, " \r\n\t", temp_strs);		
		if (temp_strs.size() >= 5 
			&& ZQ::StringOperation::isInt(temp_strs[2].c_str())
			&& ZQ::StringOperation::isInt(temp_strs[3].c_str())
			&& ZQ::StringOperation::isInt(temp_strs[4].c_str()))
		{
			SetupHandler::ECMData ecm;
			ecm.vecKeyOffset.clear();
			ecm.vecKeys.clear ();

			std::string	providerID;
			std::string	assetID;

			providerID = temp_strs[0];
			assetID = temp_strs[1];
			ecm.iProgNum = atoi(temp_strs[2].c_str());
			ecm.iFrq1 = atoi(temp_strs[3].c_str());
			int keyNum = atoi(temp_strs[4].c_str());
			if (keyNum > 0 && (int)temp_strs.size() >= 5 + keyNum * 2)
			{
				for (int tCur = 0; tCur < keyNum; tCur ++)
				{
					int tv = atoi(temp_strs[5 + tCur * 2].c_str());					
					ecm.vecKeyOffset.push_back (tv);

					std::string tmpStr = temp_strs[6 + tCur * 2];					
					ecm.vecKeys.push_back (tmpStr);
				}
			}
			else 
			{
				HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PrepareEncryptionData, "Content format error for encryption"));
				HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PrepareEncryptionData, "Content format error for encryption"));
				setResponseString(RESPONSE_SSF_INVALID_REQUEST, notice_str.c_str());
				return false;
			}

			mEcmDatas.insert(SetupHandler::ECMDataMAP::value_type(providerID+"#"+assetID,ecm));
			HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(PrepareEncryptionData,"encryption data parsed and applied"));
		}
		else 
		{
			HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PrepareEncryptionData, "Content format error for encryption"));
			HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PrepareEncryptionData, "Content format error for encryption"));
			setResponseString(RESPONSE_SSF_INVALID_REQUEST, notice_str.c_str());
			return false;
		}

		pContent = strstr(next_content, "a=X-motorola-ecm:");
	}
	return true;
}
