// File Name : SetupHandler.cpp

#include "SetupHandler.h"
#include "Environment.h"
#include "OpenVBOConfig.h"
#include "StreamersConfig.h"
#include "CRGSessionImpl.h"
#include "CRGSessionManager.h"

#include "stroprt.h"
#include "RtspRelevant.h"
#include "urlstr.h"
#include "TianShanIceHelper.h"

namespace EventISVODI5 {

static bool parseNPTTime(char* nptTime, double& npttime)
{
	// [rfc2326] 3.6 Normal Play Time
	//   npt-time     =   "now" | npt-sec | npt-hhmmss
	//   npt-sec      =   1*DIGIT [ "." *DIGIT ]
	//   npt-hhmmss   =   npt-hh ":" npt-mm ":" npt-ss [ "." *DIGIT ]
	//   npt-hh       =   1*DIGIT     ; any positive number
	//   npt-mm       =   1*2DIGIT    ; 0-59
	//   npt-ss       =   1*2DIGIT    ; 0-59

	char* pTail = nptTime + strlen(nptTime);
	for (; *nptTime == ' ' || *nptTime == '\t'; nptTime++);
	for (; pTail > nptTime && (*(pTail-1) == ' ' || *(pTail-1) == '\t'); pTail--);
	*pTail = '\0';

	if (strlen(nptTime) <=0)
		return false;

	if (0 == strcmp(nptTime, "now"))
	{
		npttime = 0.0; // TODO: replaced with RTSP_NPT_NOW
		return true;
	}

	if (strchr(nptTime, ':') !=NULL)
	{
		int hh=0, mm=0, ss=0, msec=0;
		int ret = sscanf(nptTime, "%d:%2d:%2d.%3d", &hh, &mm, &ss, &msec);
		if (ret <3)
			return false;

		npttime = (hh*60 + mm) *60 +ss;

		if (ret >3)
		{
			if (msec<10)
				npttime += (float)msec/10;
			else if (msec<100)
				npttime += (float)msec/100;
			else 
				npttime += (float)msec/1000;
		}

		return true;
	}

	if (sscanf(nptTime, "%lf", &npttime) >0)
		return true;

	return false;
}

static bool parseNPTRange(char* str, double& rangeStart, double& rangeEnd)
{
	// [rfc2326] 3.6 Normal Play Time
	//   npt-range    =   ( npt-time "-" [ npt-time ] ) | ( "-" npt-time )
	//   Examples:
	//     npt=123.45-125
	//     npt=12:05:35.3-
	//     npt=now-

	char* pos = strchr(str, '-');
	if (NULL == pos)
		return false;
	*pos = '\0';

	rangeStart = rangeEnd = 0.0;
	bool ret1 = parseNPTTime(str,   rangeStart);
	bool ret2 = parseNPTTime(pos+1, rangeEnd);
	if (!ret1 && !ret2)
		return false;

	return true;
}

// used by std::sort
struct OrdinalCmp 
{
	bool operator()(const SelectIntentionParam::PlaylistItemInfo& a, 
		const SelectIntentionParam::PlaylistItemInfo& b) const
	{
		return a.ordinal < b.ordinal;
	}
};

SetupHandler::SetupHandler(ZQ::common::Log& fileLog, Environment& env, 
						   IStreamSmithSite* pSite, IClientRequestWriter* pReq)
: RequestHandler(fileLog, env, pSite, pReq, "SETUP")
{
}

SetupHandler::~SetupHandler()
{

}

RequestProcessResult SetupHandler::doContentHandler()
{
	HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(SETUP, "start processing"));
	if (!_session.empty())
	{
		// HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(SETUP, "client specified sessionId[%s]"), _session.c_str());

		if (_openVBOConfig._response._setupFailureWithSessId > 0)
			_response->setHeader(HeaderSession, _session.c_str());
	}

	std::string connId = getRequestHeader("SYS#ConnID");
	std::string srmId  = _env.getSRMByConn(connId);
	// bool bSRMconn = _env.isSRMconn(connId);

	if (srmId.empty() && !_openVBOConfig._requestAdjust._defaultSRMID.empty())
	{
		// force to add register defaultSRMID by this connection
		srmId = _openVBOConfig._requestAdjust._defaultSRMID;
		_env.registerSRMConn(srmId, connId);
		HANDLERLOG(ZQ::common::Log::L_WARNING, HANDLERLOGFMT(SETUP, "no SRMId associated by conn, force to taking defaultSRM[%s] that maps to this conn[%s]"), _openVBOConfig._requestAdjust._defaultSRMID.c_str(), connId.c_str());
	}

	if (srmId.empty())
	{
		_statusCode = 401;
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SETUP, "unauthorized connection[%s] %s"), connId.c_str(), _szBuf);
		return RequestError;
	}

	// get transport detail
	Environment::Props transportMap;
	std::string transportData = getRequestHeader(HeaderTransport);
	if (!getTransportDetail(transportData, transportMap))
	{
		_statusCode = 461;
		return RequestError;
	}

	// get play list items
	SelectionEnv& selectionEnv = _env.getSelectionEnv();
	SelectionIntention selectionIntention(selectionEnv, _session, _sequence, _method);
	SelectIntentionParam& selectionIntentionParam = selectionIntention.getParameter();
	selectionIntentionParam.identifier = transportMap["source"]; // request streamer source
	selectionIntentionParam.requestBW = 0;   //
	selectionIntentionParam.groupName = "test";
	if (!getAssetsInfo(selectionIntentionParam))
		return RequestError;

	std::sort(selectionIntentionParam.playlist.begin(), selectionIntentionParam.playlist.end(), OrdinalCmp());

	// create client session
	CRGSessionImplPtr sessCtx = NULL;
	IClientSession* clientSess = _site->createClientSession(NULL, _strOriginalURI.c_str());
	if (NULL == clientSess || NULL == clientSess->getSessionID())
	{
		_statusCode = 500;
		snprintf(_szBuf, sizeof(_szBuf) - 1, "failed to create client session");
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SETUP, "%s"), _szBuf);
		return RequestError;
	}

	_session = clientSess->getSessionID();

	do {
		_statusCode = 500; // initialized with 500

		// set session header
		_response->setHeader(HeaderSession, _session.c_str());

		// step X.1 do the selection
		StreamerSelection streamerSelection(selectionEnv, _env.getResourceManager(), selectionIntention);
		if (!streamerSelection.findFirstStreamer())
		{
			_statusCode = streamerSelection.getLastError();
			HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SETUP, "failed to find first Streamer"));
			break; // go out of do-while(0)
		}

		// compose resource according transport details
		ZQTianShan::Util::updateResourceData<std::string>(_resourceMap, TianShanIce::SRM::rtEthernetInterface, "destIP", transportMap["destination"]);
		ZQTianShan::Util::updateResourceData<Ice::Int>(_resourceMap, TianShanIce::SRM::rtEthernetInterface, "destPort", atoi(transportMap["client_port"].c_str()));
		if (0 == _openVBOConfig._PlaylistControl._ignoreDestMac)
			ZQTianShan::Util::updateResourceData<std::string>( _resourceMap, TianShanIce::SRM::rtEthernetInterface, "destMac", transportMap["client"]);

		ZQTianShan::Util::updateResourceData<Ice::Long>( _resourceMap, TianShanIce::SRM::rtTsDownstreamBandwidth , "bandwidth", (Ice::Long)streamerSelection.getAdjustedBandwidth() );

		// step X.2 create the stream
		_statusCode = 503; // initialized with 503
		TianShanIce::Streamer::StreamSmithAdminPrx streamSmithAdminPrx = NULL;
		bool bSuccess = false;
		for (int i = 0; i <= _streamersConfig._streamingResource._retryCount; i++)
		{
			if (!streamerSelection.findNextStreamer())
			{
				_statusCode = streamerSelection.getLastError();
				continue;
			}

			ZQTianShan::Util::updateResourceData<std::string>( _resourceMap, TianShanIce::SRM::rtStreamer, "NetworkId" , streamerSelection.getSelectedStreamerNetId() );
			streamSmithAdminPrx = streamerSelection.getStreamerProxy();
			if (createStream(streamSmithAdminPrx, _resourceMap) && renderPlaylist(streamerSelection))
			{
				bSuccess = true;
				break;
			}
		} // end for

		if (!bSuccess)
		{
			HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SETUP, "failed to create stream on all Streamer candidates"));
			break; // go out of do-while(0)
		}

		streamerSelection.commit();

		// step X.3 prepare the DB record
		_statusCode = 500; // initialized with 500
		// watch session
		_env.watchSession(_session);

		// add session context into evictor
		sessCtx = new (std::nothrow) CRGSessionImpl(_fileLog, _env);
		if (sessCtx == NULL)
		{
			_statusCode = 500;
			HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SETUP, "failed to create session[%s] context"), _session.c_str());
			break; // do-while(0)
		}

		sessCtx->ident = _env.getSessionManager().getIdentity(_session);
		sessCtx->requestURL = _strOriginalURI;
		sessCtx->streamerNetId = streamerSelection.getSelectedStreamerNetId();
		sessCtx->streamerSource = _env.getStreamerSource(sessCtx->streamerNetId);
		sessCtx->streamId = _playlistPrx->getIdent().name;
		sessCtx->expiration = ZQTianShan::now() + (Ice::Long) (_openVBOConfig._rtspSession._timeout) * 1000;
		sessCtx->STBConnectionID = "";
		sessCtx->announceSeq = 1;
		sessCtx->stream = _playlistPrx;
		sessCtx->playlist = _setupInfos;
		sessCtx->sessStatusFlag = 0;

		int64 usedBindWidth = streamerSelection.getAdjustedBandwidth();
		snprintf(_szBuf, sizeof(_szBuf) - 1, "%lld", usedBindWidth);
		sessCtx->metadata[SESSION_META_DATA_USED_BANDWIDTH] = _szBuf; 
		std::string strImportChannelName = streamerSelection.getSelectedImportChannelName();
		if (!strImportChannelName.empty())
			sessCtx->metadata[SESSION_META_DATA_IMPORT_CHANNEL_NAME] = strImportChannelName;

		MAPSET(::TianShanIce::Properties, sessCtx->metadata, SESSION_META_DATA_SRM_ID, srmId);

		// add to storage
		if (!_env.getSessionManager().addSession(sessCtx, _strLastError))
		{
			_statusCode = 500;
			HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SETUP, "failed to add session[%s] context into DB"), _session.c_str());
			break; // do-while(0)
		}

		// step X.4 compse the response to send
		_statusCode = 200; // confirm the succeed

		// set transport header
		size_t nStart = transportData.find("source");
		if (nStart != std::string::npos)
		{
			size_t nEnd = transportData.find(";", nStart);
			transportData.replace(nStart, nEnd - nStart + 1, "");
		}
		_response->setHeader(HeaderTransport, transportData.c_str());

		// set location header 
		std::string rtspServerIP = getRequestHeader("SYS#LocalServerIP");
		std::string rtspServerPort = getRequestHeader("SYS#LocalServerPort");
		std::string location = "rtsp://" + rtspServerIP + ":" + rtspServerPort;
		_response->setHeader("Location", location.c_str());

		// set content body
		if (!_requestItems.empty())
		{
			_response->setHeader(HeaderContentType, "text/parameters");
			std::stringstream ss;
			std::vector<std::string>::iterator iter = _requestItems.begin();
			for (; iter != _requestItems.end(); iter++)
				ss << *iter << CRLF;

			/*std::vector<SelectIntentionParam::PlaylistItemInfo>::iterator iter;
			iter = selectionIntentionParam.playlist.begin();
			for(; iter != selectionIntentionParam.playlist.end(); iter++)
			{
			ss << "playlist_item:" << iter->paid << ";ordinal=" << iter->ordinal 
			<< ";in=" << iter->cuein << ";out=" << iter->cueout
			<< ""<< ";nptlength=" << CRLF;
			}*/

			std::string strContent = ss.str();
			_response->printf_postheader(strContent.c_str());
		}

		HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(SETUP, "session[%s] processed successfully"), _session.c_str());
		return RequestProcessed;

	} while(0);

	_env.getSessionManager().removeSession(_session, _strLastError);

	if (NULL != clientSess)
	{
		//  notice: clientSess->release(); didn't unregister self from sessMgr, so call destroyClientSession() instead
		_site->destroyClientSession(_session.c_str());
		HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(SETUP, "client session released"));
	}

	HANDLERLOG(ZQ::common::Log::L_WARNING, HANDLERLOGFMT(SETUP, "failed, context cleaned"));
}

bool SetupHandler::createStream( TianShanIce::Streamer::StreamSmithAdminPrx streamServiceAdminPrx, 
								TianShanIce::SRM::ResourceMap& resMap )
{
	if (_playlistPrx != NULL)
	{
		try
		{
			_playlistPrx->destroy();			
		}
		catch (...) 
		{
		}
		_playlistPrx = NULL;
	}

	std::string streamServicePrxStr = _env.getProxyString(streamServiceAdminPrx);
	try
	{
		TianShanIce::Properties props;
		TianShanIce::Streamer::StreamPrx streamPrx = streamServiceAdminPrx->createStreamByResource(resMap, props);
		_playlistPrx = TianShanIce::Streamer::PlaylistPrx::uncheckedCast(streamPrx);
		if(!_playlistPrx)
		{
			_statusCode = 500;
			HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SETUP, "failed to create stream on [%s]"), streamServicePrxStr.c_str());
			return false;
		}

		HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(SETUP, "successfully created stream[%s]"), _env.getProxyString(streamPrx).c_str());
	}
	catch (const TianShanIce::InvalidParameter& ex)
	{
		//_statusCode = 771;
		_statusCode = 404;
		HANDLERLOG(ZQ::common::Log::L_WARNING, HANDLERLOGFMT(SETUP, "caught exception [%s] when create stream on [%s]"), ex.ice_name().c_str(), streamServicePrxStr.c_str());
		return false;
	}
	catch( const Ice::TimeoutException& ex)
	{
		_statusCode = 503;
		HANDLERLOG(ZQ::common::Log::L_WARNING, HANDLERLOGFMT(SETUP, "caught exception [%s] when create stream on [%s]"), ex.ice_name().c_str(), streamServicePrxStr.c_str());
		return false; 
	}
	catch( const Ice::SocketException& ex)
	{
		_statusCode = 503;
		HANDLERLOG(ZQ::common::Log::L_WARNING, HANDLERLOGFMT(SETUP, "caught exception [%s] when create stream on [%s]"), ex.ice_name().c_str(), streamServicePrxStr.c_str());
		return false; 
	}
	catch (const Ice::Exception& ex) 
	{
		_statusCode = 500;
		HANDLERLOG(ZQ::common::Log::L_WARNING, HANDLERLOGFMT(SETUP, "caught exception [%s] when create stream on [%s]"), ex.ice_name().c_str(), streamServicePrxStr.c_str());
		return false;
	}

	return true;
}

bool SetupHandler::renderPlaylist(StreamerSelection& streamerSelection)
{
	std::string streamPrxStr = _env.getProxyString(streamerSelection.getStreamerProxy());
	int32 currentElementIndex = 1; //used as CtrlNum based on 1
	const ElementInfoS& plInfos = streamerSelection.getElements();
	int32 iTotalElementCount =(int32) plInfos.size();
	ElementInfoS::const_iterator itElement = plInfos.begin();
	for(; itElement != plInfos.end(); itElement ++)
	{
		TianShanIce::Streamer::PlaylistItemSetupInfo setupInfo;
		setupInfo.privateData.clear();

		setupInfo.contentName = itElement->fullContentName(streamerSelection.getSelectedVolumeName());
		setupInfo.criticalStart = 0;
		setupInfo.inTimeOffset = itElement->cueIn;
		setupInfo.outTimeOffset = itElement->cueOut;
		setupInfo.flags = itElement->flags;
		setupInfo.spliceIn = false;
		setupInfo.spliceOut = false;
		setupInfo.forceNormal = false;

		::TianShanIce::ValueMap& infoPD = setupInfo.privateData;
		infoPD.clear();
		ZQTianShan::Util::updateValueMapData( setupInfo.privateData , "providerId", itElement->pid);
		ZQTianShan::Util::updateValueMapData( setupInfo.privateData , "providerAssetId", itElement->paid );
		const VolumeAttrEx& volAttr = streamerSelection.getSelectedVolumeAttr();
		if( volAttr.bSupportNas )
		{			
			ZQTianShan::Util::updateValueMapData( setupInfo.privateData , "storageLibraryUrl" , itElement->urls, false );
			HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(RenderPlaylistCommand,"add url[%s] to item[%s][%d]"), ZQTianShan::Util::dumpTianShanIceStrValues(itElement->urls).c_str() , setupInfo.contentName.c_str(),currentElementIndex);						
		}
		else
		{
			setupInfo.privateData.erase("storageLibraryUrl");
			HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(RenderPlaylistCommand,"do not add url to item[%s][%d] per supportNasStream[%s]"), setupInfo.contentName.c_str(),currentElementIndex ,	volAttr.bSupportNas ? "true":"false" );
		}
		if ( iTotalElementCount == currentElementIndex + 1 )
		{
			setupInfo.privateData.erase("TianShan-flag-pauselastuntilnext");
		}

		try
		{
			_playlistPrx->pushBack(currentElementIndex , setupInfo);
			_setupInfos.push_back(setupInfo); //
		}
		catch(const TianShanIce::InvalidParameter& ex)
		{
			//_statusCode = 771;
			_statusCode = 404;
			snprintf(_szBuf, sizeof(_szBuf) - 1, "caught exception:[%s] when pushback items onto playlist[%s]", ex.ice_name().c_str(), streamPrxStr.c_str());
			HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SETUP, "%s"), _szBuf);
			return false;
		}
		catch (const Ice::Exception& ex) 
		{
			_statusCode = 500;
			snprintf(_szBuf, sizeof(_szBuf) - 1, "caught exception:[%s] when pushback items onto playlist[%s]", ex.ice_name().c_str(), streamPrxStr.c_str());
			HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SETUP, "%s"), _szBuf);
			return false;
		}
		++currentElementIndex;
	}

	bool bEnableEOT = _openVBOConfig._PlaylistControl._enableEOT > 0;
	try
	{
		_playlistPrx->enableEoT(bEnableEOT);
		_playlistPrx->commit();		
	}
	catch(const TianShanIce::BaseException& ex)
	{
		if(ex.errorCode == EXT_ERRCODE_BANDWIDTH_EXCEEDED)
		{
			//_statusCode = 776;
			_statusCode = 453;
		}
		else
		{
			_statusCode = 500;
		}
		snprintf(_szBuf, sizeof(_szBuf) - 1, "caught exception[%s] when invoke enableEOT/commit onto playlist[%s]", ex.message.c_str(), streamPrxStr.c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SETUP, "%s"), _szBuf);
		return false;
	}
	catch(const Ice::Exception& ex)
	{
		_statusCode = 500;
		snprintf(_szBuf, sizeof(_szBuf) - 1, "caught exception[%s] when invoke enableEOT/commit onto playlist[%s]", ex.ice_name().c_str(), streamPrxStr.c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SETUP, "%s"), _szBuf);
		return false;
	}
	return true;
}

static void chopForPidPaid(const std::string& contentName, std::string& paid, std::string& pid, const char* defaultPid ="stupid.com", uint nonGuidPaidSize=20)
{
	size_t pos = contentName.find_last_of("\\/");
	paid =  (std::string::npos == pos) ? contentName : contentName.substr(pos+1);
	pid = "";

	ZQ::common::Guid guid(paid.c_str());

	if (guid.isNil() && paid.length() > nonGuidPaidSize)
	{
		pid = paid.substr(nonGuidPaidSize);
		paid = paid.substr(0, nonGuidPaidSize);

		pos = pid.find_first_not_of(".-_");
		if (std::string::npos == pos)
			pid = "";
		else if (0 != pos)
			pid = pid.substr(pos);
	}

	if (pid.empty() && NULL != defaultPid)
		pid = defaultPid;
}

bool SetupHandler::getAssetsInfo(SelectIntentionParam& selectIntentionParam)
{
	// get original URL
	const char* pURL = _request->getUri(_szBuf, sizeof(_szBuf) - 1);
	if (!pURL || strlen(pURL) <= 0)
	{
		_statusCode = 400;
		snprintf(_szBuf, sizeof(_szBuf) - 1, "Request URL is empty");
		HANDLERLOG(ZQ::common::Log::L_ERROR, CLOGFMT(SetupHandler, "%s"), _szBuf);
		return false;
	}
	_strOriginalURI = pURL;

	getContentBody();
	std::string& strContent = _requestBody;
	if (strContent != "")
	{
		// parse playlist_item:<asset>;ordinal=<ordinal>[;in=<NPT_in>][;out=<NPT_out>][;trickmode=<trickmode>]
		// ->
		std::string strItem;
		size_t nStart = strContent.find("playlist_item:");
		int itemIndex = 1;
		while (nStart != std::string::npos)
		{
			nStart += strlen("playlist_item:");
			size_t nEnd = strContent.find("playlist_item:", nStart);
			if (nEnd == std::string::npos)
				strItem = strContent.substr(nStart, nEnd);
			else
				strItem = strContent.substr(nStart, nEnd - nStart);

			size_t nNotReturn = strItem.find_last_not_of("\r\n");
			size_t nReturn    = strItem.find_last_of("\r\n");
			if ( std::string::npos != nNotReturn && std::string::npos != nReturn && nNotReturn < nReturn )
				strItem.erase(nNotReturn + 1);

			SelectIntentionParam::PlaylistItemInfo info;
			if (!getAssetInfo(strItem, info))
			{
				_statusCode = 400;
				snprintf(_szBuf, sizeof(_szBuf) - 1, "bad play list item format");
				HANDLERLOG(ZQ::common::Log::L_ERROR, CLOGFMT(SetupHandler, "%s"), _szBuf);
				return false;
			}
			selectIntentionParam.playlist.push_back(info);
			nStart = strContent.find("playlist_item:", nEnd);
			itemIndex++;
		} // end while
	}
	else
	{
		HANDLERLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(SetupHandler, "get asset in request URL[%s]"), _strOriginalURI.c_str());
		ZQ::common::URLStr urlPsr(_strOriginalURI.c_str(), true);
		std::string pAsset = urlPsr.getPath();
		if (pAsset.empty())
		{
			_statusCode = 400;
			snprintf(_szBuf, sizeof(_szBuf) - 1, "no Assets found in Request");
			HANDLERLOG(ZQ::common::Log::L_ERROR, CLOGFMT(SetupHandler, "%s"), _szBuf);
			return false;
		}

		SelectIntentionParam::PlaylistItemInfo info;
//		info.paid = pAsset;
//		info.pid = _openVBOConfig._requestAdjust._defaultPID;
		chopForPidPaid(pAsset, info.paid, info.pid, _openVBOConfig._requestAdjust._defaultPID.c_str());
		selectIntentionParam.playlist.push_back(info);
	}
	if (selectIntentionParam.playlist.empty())
	{
		_statusCode = 400;
		snprintf(_szBuf, sizeof(_szBuf) - 1, "no Assets found in Request");
		HANDLERLOG(ZQ::common::Log::L_ERROR, CLOGFMT(SetupHandler, "%s"), _szBuf);
		return false; 
	}
	_strOriginalURI = "rtsp://" + _strOriginalURI;
	return true;
}

bool SetupHandler::getAssetInfo(const std::string& strItem, SelectIntentionParam::PlaylistItemInfo& info)
{
	std::vector<std::string> ItemProps;
	ZQ::StringOperation::splitStr(strItem, ";", ItemProps);
	if (ItemProps.size() < 2)
	{
		return false;
	}

	//parse <asset>;ordinal=<ordinal>[;in=<NPT_in>][;out=<NPT_out>][;trickmode=<trickmode>]
	// info.pid = _openVBOConfig._requestAdjust._defaultPID;
	std::string requestItem;
	std::vector<std::string> props;
	size_t size = 0;
	for (size_t i = 0; i < ItemProps.size(); i++)
	{
		props.clear();
		ZQ::StringOperation::splitStr(ItemProps[i], "=", props);
		size = props.size();
		if (size > 2)
		{
			return false;
		}
		if (1 == size && 0 == i)
		{
			// info.paid = ItemProps[0];
			chopForPidPaid(ItemProps[0], info.paid, info.pid, _openVBOConfig._requestAdjust._defaultPID.c_str());

			requestItem += ItemProps[0];
			continue;
		}
		if ("ordinal" == props[0] && 2 == size)
		{
			info.ordinal = atoi(props[1].c_str());

			requestItem +=  ";ordinal=" + props[1];
			continue;
		}
		if ("in" == props[0] && 2 == size)
		{
			double cueIn;
			parseNPTTime((char*)props[1].c_str(), cueIn);
			info.cuein = (int64)(cueIn * 1000);
			//info.cuein = atoi(props[1].c_str());
			
			requestItem +=  ";in=" + props[1];
			continue;
		}
		if ("out" == props[0] && 2 == size)
		{
			double cueOut;
			parseNPTTime((char*)props[1].c_str(), cueOut);
			info.cueout = (int64)(cueOut * 1000);
			//info.cueout = atoi(props[1].c_str());

			requestItem += ";out=" + props[1];
			continue;
		}

		if ("trickmode" == props[0] && 2 == size)
		{
			int trickmode = atoi(props[1].c_str()); // 5d spec: trickmode is a decimal representation of bitmask value
			info.restrictionFlag = 0;

			// converting eventIS trick mode definition to TianShanIce::Streamer::PLISFlagXXXX
			if (trickmode & 0x01) // 0x01 == PAUSE
				info.restrictionFlag |= TianShanIce::Streamer::PLISFlagNoPause;
			if (trickmode & 0x02) // 0x02 == FF
				info.restrictionFlag |= TianShanIce::Streamer::PLISFlagNoFF;
			if (trickmode & 0x04) // 0x04 == REW
				info.restrictionFlag |= TianShanIce::Streamer::PLISFlagNoRew;
			if (trickmode & 0x08) // 0x08 == PLAY // not implemented
				info.restrictionFlag |= 0;
			if (trickmode & 0x10) // 0x10 == SKIP disallowed, newly added in eventIS 5d spec v1.4
				info.restrictionFlag |= TianShanIce::Streamer::PLISFlagNoSeek;

			requestItem += ";trickmode=" + props[1];
			continue;
		}
		if("in" != props[0] && "out" != props[0] && "trickmode" != props[0] )
		{
			requestItem += ";" + ItemProps[i];
			continue;
		}
	} // end for
	
	/* bug#21855 not to present the optional parameters if not available
	if (NULL == strstr((char*)strItem.c_str(), "ordinal"))
	{
		requestItem +=  ";ordinal=";
	}

	
	if (NULL == strstr((char*)strItem.c_str(), "in"))
	{
		requestItem +=  ";in=";
	}

	if (NULL == strstr((char*)strItem.c_str(), "out"))
	{
		requestItem +=  ";out=";
	}

	if (NULL == strstr((char*)strItem.c_str(), "trickmode"))
	{
		requestItem +=  ";trickmode=";
	}
	
	
	if (NULL == strstr((char*)strItem.c_str(), "nptlength"))
	{
		requestItem +=  ";nptlength=";
	}
	*/
	
	requestItem = "playlist_item:" + requestItem;

	_requestItems.push_back(requestItem);
	return true;
}

bool SetupHandler::getTransportDetail(const std::string& strTransportdata, std::map<std::string, std::string>& transportMap)
{
	transportMap.clear();
	std::vector<std::string> Items;
	std::vector<std::string> Transports;
	ZQ::StringOperation::splitStr(strTransportdata, ";", Transports);
	std::vector<std::string>::iterator iter = Transports.begin();
	for (; iter != Transports.end(); iter++)
	{
		Items.clear();
		ZQ::StringOperation::splitStr(*iter, "=", Items);
		if (Items.size() != 2)
		{
			continue;
		}
		transportMap.insert(std::make_pair(Items[0], Items[1]));
	}
	if (transportMap["source"].empty() || transportMap["destination"].empty() || transportMap["client_port"].empty())
	{
		_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(SetupHandler, "bad transport [%s]"), strTransportdata.c_str());
		return false;
	}
	return true;
}

} // end EventISVODI5
