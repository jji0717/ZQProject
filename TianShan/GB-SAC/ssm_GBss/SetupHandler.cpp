// File Name : SetupHandler.cpp

#include "SetupHandler.h"

#include "Environment.h"

#include "GBssConfig.h"

#include "StreamersConfig.h"

#include "CRGSessionImpl.h"

#include "CRGSessionManager.h"

#include "stroprt.h"

#include "RtspRelevant.h"

#include "urlstr.h"

#include "TianShanIceHelper.h"

#include "RtspHeaderDefines.h"

namespace GBss
{

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
: RequestHandler(fileLog, env, pSite, pReq)
{
	_method = "SETUP";
}

SetupHandler::~SetupHandler()
{

}

RequestProcessResult SetupHandler::doContentHandler()
{
	HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(SetupHandler, "start process"));
	if (_GBssConfig._response._setupFailureWithSessId > 0)
	{
		_response->setHeader(HeaderSession, _session.c_str());
	}

	// get transport detail
	std::map<std::string, std::string> transportMap;
	std::string transportData = getRequestHeader(HeaderTransport);
	if (!getTransportDetail(transportData, transportMap))
	{
		_statusCode = 461;
		return RequestError;
	}

	std::string bit_rate;
	Ice::Long bitrate = 0;
	if (transportMap.find("bit_rate") != transportMap.end())
	{
		bitrate = _atoi64(transportMap["bit_rate"].c_str());
		bit_rate = "bit_rate=" + transportMap["bit_rate"];
		size_t index = transportData.find(bit_rate);
		if (index != std::string::npos)
		{
			if (index+bit_rate.size()<transportData.size())
			{
				transportData.erase(index, bit_rate.size()+1);
			}
			else
			{
				transportData.erase(index-1, bit_rate.size()+1);
			}
		}
	}

	std::string serviceGroup, stbIP;
	std::string serviceGroupIP = getRequestHeader(HeaderServiceGroupIP);
	size_t index = serviceGroupIP.find(":");
	if(index != std::string::npos)
	{
		serviceGroup = serviceGroupIP.substr(0, index);
		stbIP = serviceGroupIP.substr(index+1, serviceGroupIP.size());
		HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(SetupHandler, "Service-Group-IP[%s:%s]"), serviceGroup.c_str(), stbIP.c_str());
	}

	// stupid code to ensure _env._SRMMap is not empty
	std::vector<std::string> srmConnections;
	_env.getSRMConnectionIDs(srmConnections);
	if (srmConnections.size() <=0)
	{
		std::string strConnID = getRequestHeader("SYS#ConnID");
		_env.addIntoSRMMap(".default", strConnID);
	}

	// get play list items
	SelectionEnv& selectionEnv = _env.getSelectionEnv();
	SelectionIntention selectionIntention(selectionEnv, _session, _sequence, _method);
	SelectIntentionParam& selectionIntentionParam = selectionIntention.getParameter();
//	selectionIntentionParam.identifier = transportMap["source"]; // request streamer source
	selectionIntentionParam.requestBW = bitrate;   //
	selectionIntentionParam.groupName = "test";
	if (!getAssetsInfo(selectionIntentionParam))
	{
		return RequestError;
	}
	std::sort(selectionIntentionParam.playlist.begin(), selectionIntentionParam.playlist.end(), OrdinalCmp());

	// create client session
	IClientSession* pSession = _site->createClientSession(NULL, _strOriginalURI.c_str());
	if (NULL == pSession || NULL == pSession->getSessionID())
	{
		_statusCode = 500;
		snprintf(_szBuf, sizeof(_szBuf) - 1, "failed to create client session");
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), _szBuf);
		return RequestError;
	}
	_session = pSession->getSessionID();

	// set session header
	_response->setHeader(HeaderSession, _session.c_str());

	_response->setHeader(HeaderGlobalSessId, getRequestHeader(HeaderGlobalSessId).c_str());

	StreamerSelection streamerSelection(selectionEnv, _env.getResourceManager(), selectionIntention);
	if (!streamerSelection.findFirstStreamer())
	{
		_statusCode = streamerSelection.getLastError();
		snprintf(_szBuf, sizeof(_szBuf) - 1, "failed to find first Streamer");
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), _szBuf);
		return RequestError;
	}

	// compose resource according transport details
	ZQTianShan::Util::updateResourceData<std::string>(_resourceMap, TianShanIce::SRM::rtEthernetInterface, "destIP", transportMap["destination"]);
	ZQTianShan::Util::updateResourceData<Ice::Int>(_resourceMap, TianShanIce::SRM::rtEthernetInterface, "destPort", atoi(transportMap["client_port"].c_str()));
	if (0 == _GBssConfig._PlaylistControl._ignoreDestMac)
	{		
		ZQTianShan::Util::updateResourceData<std::string>( _resourceMap, TianShanIce::SRM::rtEthernetInterface, "destMac", transportMap["client"]);
	}
	ZQTianShan::Util::updateResourceData<Ice::Long>( _resourceMap, TianShanIce::SRM::rtTsDownstreamBandwidth , "bandwidth", (Ice::Long)streamerSelection.getAdjustedBandwidth() );


	TianShanIce::Streamer::StreamSmithAdminPrx streamSmithAdminPrx = NULL;
	bool bSuccess = false;
	for (int i = 0; i <= _streamersConfig._streamingResource._retryCount; i++)
	{
		if (streamerSelection.findNextStreamer())
		{
			ZQTianShan::Util::updateResourceData<std::string>( _resourceMap, TianShanIce::SRM::rtStreamer, "NetworkId" , streamerSelection.getSelectedStreamerNetId() );
			streamSmithAdminPrx = streamerSelection.getStreamerProxy();
			if (createStream(streamSmithAdminPrx, _resourceMap, getRequestHeader(HeaderGlobalSessId)) && renderPlaylist(streamerSelection))
			{
				bSuccess = true;
				break;
			}
		}
		else
		{
			_statusCode = streamerSelection.getLastError();
		}
	} // end for
	if (!bSuccess)
	{
		return RequestError;
	}

	streamerSelection.commit();

	// watch session
	_env.watchSession(_session);

	// add session context into evictor
	CRGSessionImplPtr sesssionContext = new (std::nothrow) CRGSessionImpl(_fileLog, _env);
	if (sesssionContext == NULL)
	{
		_statusCode = 500;
		snprintf(_szBuf, sizeof(_szBuf) - 1, "failed to create session[%s] context", _session.c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), _szBuf);
		return RequestError;
	}
	sesssionContext->ident = _env.getSessionManager().getIdentity(_session);
	sesssionContext->requestURL = _strOriginalURI;
	sesssionContext->streamerNetId = streamerSelection.getSelectedStreamerNetId();
	sesssionContext->streamerSource = _env.getStreamerSource(sesssionContext->streamerNetId);
	sesssionContext->streamId = _playlistPrx->getIdent().name;
	sesssionContext->expiration = ZQTianShan::now() + (Ice::Long) (_GBssConfig._rtspSession._timeout) * 1000;
	sesssionContext->STBConnectionID = "";
	sesssionContext->announceSeq = 1;
	sesssionContext->stream = _playlistPrx;
	sesssionContext->playlist = _setupInfos;
	sesssionContext->globalSessionId = getRequestHeader(HeaderGlobalSessId);

	std::string sourceIP;
	int sourcePort;
	if( !getStreamSourceAddressInfo( sourceIP, sourcePort ))
		return RequestError;

	int64 usedBindWidth = streamerSelection.getAdjustedBandwidth();
	snprintf(_szBuf, sizeof(_szBuf) - 1, "%lld", usedBindWidth);
	sesssionContext->metadata[SESSION_META_DATA_USED_BANDWIDTH] = _szBuf; 
	std::string strImportChannelName = streamerSelection.getSelectedImportChannelName();
	if (!strImportChannelName.empty())
	{
		sesssionContext->metadata[SESSION_META_DATA_IMPORT_CHANNEL_NAME] = strImportChannelName;
	}

	// add to storage
	if (!_env.getSessionManager().addSession(sesssionContext, _strLastError))
	{
		_statusCode = 500;
		return RequestError;
	}

	// set transport header
	size_t nStart = transportData.find("source");
	if (nStart != std::string::npos)
	{
		size_t nEnd = transportData.find(";", nStart);
		transportData.replace(nStart, nEnd - nStart + 1, "");
	}
	else
	{
		std::ostringstream ossTransExt;
		if (transportData[transportData.size()-1] != ';')
		{
			ossTransExt << ";";
		}
		ossTransExt <<	"source="<<sourceIP<<";server_port="<<sourcePort<<";bit_rate="<<usedBindWidth;
		transportData += ossTransExt.str();
	}
	_response->setHeader(HeaderTransport, transportData.c_str());

	// set location header 
	std::string rtspServerIP = getRequestHeader("SYS#LocalServerIP");
	std::string rtspServerPort = getRequestHeader("SYS#LocalServerPort");
// 	std::string location = "rtsp://" + rtspServerIP + ":" + rtspServerPort;
// 	_response->setHeader("Location", location.c_str());

	//generate control session
    //ControlSession: ID;ControlHost:xxx;ControlPort:xxx;ControlTimeout:xxx
	std::stringstream oss;
    oss << _session << ";ControlHost:" << rtspServerIP << ";ControlPort:" << rtspServerPort << ";ControlTimeout:" << _GBssConfig._rtspSession._timeout;
	_response->setHeader(HeaderControlSession, oss.str().c_str());

	// set content body
	if (!_requestItems.empty())
	{
		_response->setHeader(HeaderContentType, "text/parameters");
		std::stringstream ss;
		std::vector<std::string>::iterator iter = _requestItems.begin();
		for (; iter != _requestItems.end(); iter++)
		{
			ss << *iter << CRLF;
		}

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
	return RequestProcessed;
}

bool SetupHandler::createStream( TianShanIce::Streamer::StreamSmithAdminPrx streamServiceAdminPrx, 
								TianShanIce::SRM::ResourceMap& resMap, std::string globalSessId )
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
		Ice::Context ctx;
		ctx["CLIENTSESSIONID"] = globalSessId;
		TianShanIce::Streamer::StreamPrx streamPrx = streamServiceAdminPrx->createStreamByResource(resMap, ctx);
		_playlistPrx = TianShanIce::Streamer::PlaylistPrx::uncheckedCast(streamPrx);
		if(!_playlistPrx)
		{
			_statusCode = 500;
			HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "failed to create stream on [%s]"), streamServicePrxStr.c_str());
			return false;
		}
		HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(SetupHandler, "successfully created stream[%s] on [%s]"), _env.getProxyString(streamPrx).c_str() ,streamServicePrxStr.c_str());
	}
	catch (const TianShanIce::InvalidParameter& ex)
	{
		//_statusCode = 771;
		_statusCode = 404;
		HANDLERLOG(ZQ::common::Log::L_WARNING, HANDLERLOGFMT(SetupHandler, "caught exception [%s] when create stream on [%s]"), ex.ice_name().c_str(), streamServicePrxStr.c_str());
		return false;
	}
	catch (const Ice::Exception& ex) 
	{
		_statusCode = 500;
		HANDLERLOG(ZQ::common::Log::L_WARNING, HANDLERLOGFMT(SetupHandler, "caught exception [%s] when create stream on [%s]"), ex.ice_name().c_str(), streamServicePrxStr.c_str());
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
			HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SetupHandler, "%s"), _szBuf);
			return false;
		}
		catch (const Ice::Exception& ex) 
		{
			_statusCode = 500;
			snprintf(_szBuf, sizeof(_szBuf) - 1, "caught exception:[%s] when pushback items onto playlist[%s]", ex.ice_name().c_str(), streamPrxStr.c_str());
			HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SetupHandler, "%s"), _szBuf);
			return false;
		}
		++currentElementIndex;
	}

	bool bEnableEOT = _GBssConfig._PlaylistControl._enableEOT > 0;
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
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SetupHandler, "%s"), _szBuf);
		return false;
	}
	catch(const Ice::Exception& ex)
	{
		_statusCode = 500;
		snprintf(_szBuf, sizeof(_szBuf) - 1, "caught exception[%s] when invoke enableEOT/commit onto playlist[%s]", ex.ice_name().c_str(), streamPrxStr.c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(SetupHandler, "%s"), _szBuf);
		return false;
	}
	return true;
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
		size_t nStart = strContent.find("a=x-playlist-item: ");
		int itemIndex = 1;
		while (nStart != std::string::npos)
		{
			nStart += strlen("a=x-playlist-item: ");
			size_t nEnd = strContent.find("a=x-playlist-item: ", nStart);
			if (nEnd == std::string::npos)
			{
				strItem = strContent.substr(nStart, nEnd);
			}
			else
			{
				strItem = strContent.substr(nStart, nEnd - nStart);
			}
			SelectIntentionParam::PlaylistItemInfo info;
			if (!getAssetInfo(strItem, info))
			{
				_statusCode = 400;
				snprintf(_szBuf, sizeof(_szBuf) - 1, "bad play list item format");
				HANDLERLOG(ZQ::common::Log::L_ERROR, CLOGFMT(SetupHandler, "%s"), _szBuf);
				return false;
			}
			selectIntentionParam.playlist.push_back(info);
			nStart = strContent.find("a=x-playlist-item:", nEnd);
			itemIndex++;
		} // end while
	}
	else
	{
		HANDLERLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(SetupHandler, "get asset in request URL[%s]"), _strOriginalURI.c_str());
		ZQ::common::URLStr urlPsr(_strOriginalURI.c_str(), true);
		std::string pAsset = urlPsr.getPath();
		if ("" == pAsset)
		{
			_statusCode = 400;
			snprintf(_szBuf, sizeof(_szBuf) - 1, "no Assets found in Request");
			HANDLERLOG(ZQ::common::Log::L_ERROR, CLOGFMT(SetupHandler, "%s"), _szBuf);
			return false;
		}
		SelectIntentionParam::PlaylistItemInfo info;
		info.paid = pAsset;
		info.pid = _GBssConfig._requestAdjust._defaultPID;
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
	ZQ::StringOperation::splitStr(strItem, " \t", ItemProps);
	if (ItemProps.size() < 1)
	{
		return false;
	}

	info.paid = ItemProps[0];

	//parse <contentID> <type> [bitrate] [range] [tricks/[F][R][P]]
	info.pid = _GBssConfig._requestAdjust._defaultPID;
	std::string type, bitrate;
	size_t size = 0;
	for (size_t i = 1; i < ItemProps.size(); i++)
	{
		if (ItemProps[i].find("-") != std::string::npos)
		{
			std::vector<std::string> props;
			props.clear();			
			ZQ::StringOperation::splitStr(ItemProps[i], "-", props);
			if (props.size() > 0)
				info.cuein = atoi(props[0].c_str());
			if (props.size() > 1)
				info.cueout = atoi(props[1].c_str());
			continue;
		}
		if (ItemProps[i].find("tricks/") != std::string::npos)
		{
			std::string strTemp = ItemProps[i];
			std::transform(strTemp.begin(),strTemp.end(), strTemp.begin(), ::tolower); 
			std::string::const_iterator it = strTemp.begin()+7; //skip "tricks/"
			for( ; it != strTemp.end() ; it++ )
			{
				switch( *it )
				{
				case 'r':
					info.restrictionFlag |= TianShanIce::Streamer::PLISFlagNoRew;
					break;
				case 'f':
					info.restrictionFlag |= TianShanIce::Streamer::PLISFlagNoFF;
					break;
				case 'p':
					info.restrictionFlag |= TianShanIce::Streamer::PLISFlagNoPause;
					break;
				default:
					break;
				}
			}	
			continue;
		}
		if (1 == i)
		{
			bool bhasDigit = false;
			std::string::const_iterator it = ItemProps[i].begin();
			for( ; it != ItemProps[i].end() ; it++ )
			{
				if(isdigit(*it))
				{
					bhasDigit = true;
					break;
				}
			}
			if(!bhasDigit)
				type = ItemProps[i];
			else
			{
				bitrate = ItemProps[i];
				info.bandWidth = atoi(bitrate.c_str());
			}
		}
		if (2 == i)
		{
			if(ItemProps[i].find("-") == std::string::npos && ItemProps[i].find("tricks/") == std::string::npos)
			{
				bitrate = ItemProps[i];
				info.bandWidth = atoi(bitrate.c_str());
				continue;
			}
		}
	} // end for
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
	if (transportMap["destination"].empty() || transportMap["client_port"].empty())
	{
		_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(SetupHandler, "bad transport [%s]"), strTransportdata.c_str());
		return false;
	}
	return true;
}

} // end GBss
