#include "./TeardownHandler.h"
#include <urlstr.h>

#define CRLF	"\r\n"

#ifdef _DEBUG
#include <iostream>
using namespace std;
#endif

TeardownHandler::TeardownHandler(NGODEnv& ssm, IStreamSmithSite* pSite, IClientRequestWriter* pReq) : RequestHandler(ssm, pSite, pReq)
{
	_method = "TEARDOWN";
	_inoutMap[MAP_KEY_METHOD] = _method;
#ifdef _DEBUG
	cout<<"construct TEARDOWN handler"<<endl;
#endif

}

TeardownHandler::~TeardownHandler()
{
#ifdef _DEBUG
	cout<<"deconstruct TEARDOWN handler"<<endl;
#endif

}

RequestProcessResult TeardownHandler::process()
{
	HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(TeardownHandler, "start processing"));

	if (false == _canProcess)
	{
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(TeardownHandler, "failed to process the request because[%s]"), szBuf);
		HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(TeardownHandler, "failed to process the request because[%s]"), szBuf);
		return RequestError;
	}
	if (_ngodConfig._MessageFmt.rtspNptUsage <= 0)
	{
		if (!handshake(_requireProtocol, 0, 2, false))
		{
			return RequestError;
		}
	}
	else
	{
		_requireProtocol = NgodVerCode_R2_DecNpt;
	}

	std::string onDemandID;
	onDemandID = getRequestHeader(NGOD_HEADER_ONDEMANDSESSIONID);
	_pResponse->setHeader(NGOD_HEADER_ONDEMANDSESSIONID, onDemandID.c_str());	


	if (false == getContext())
	{
		responseError(RESPONSE_INTERNAL_ERROR);
		return RequestError;
	}
	if (_ngodConfig._MessageFmt.rtspNptUsage <= 0 && _requireProtocol == NgodVerCode_UNKNOWN)
	{
		_requireProtocol = atoi(_context.prop["RequireR2"].c_str());
	}

	// add by zjm to support session history
	std::stringstream ss;
	ss << "<ResponseData>" << CRLF
	   << "<ODRMSessionHistory>" << CRLF;
	if (_ngodConfig._sessionHistory.enableHistory > 0)
	{
		std::string strServerIP;
		std::string strRemoteIP;
		uint16 port;
		getServerAddr(strServerIP, port);
		getRemoteAddr(strRemoteIP, port);
		ss  << "<ODRMSession " 
			<< "componentName=\""       << "ODRM-to-Fill" 
			<< "\" ODSessionID=\""      << _context.onDemandID
			<< "\" setupDate=\""        << _context.prop["setupDate"] 
		    << "\" ODRMIpAddr=\""       << strServerIP // "ODRM-to-Fill"
			<< "\" SMIpAddr=\""         << strRemoteIP // "ODRM-to-Fill"
			<< "\" resultCode=\""       << getRequestHeader(NGOD_HEADER_XREASON)
			<< "\" teardownDate=\""     << NgodUtilsClass::generatorISOTime();
		if (_context.prop["sessionGroup"] != "")
		{
			ss << "\" sessionGroup=\""     << _context.prop["sessionGroup"];
		}
		ss << "\" >" << CRLF;
		ss << "<PlayoutHistory time=\"" << NgodUtilsClass::generatorISOTime() << "\" >" << CRLF;

		int ctrlNum = 0;
		NGODr2c1::PlaylistItemSetupInfos::iterator resIter = _context.setupInfos.begin();
		for (;  resIter != _context.setupInfos.end(); resIter++)
		{
			ctrlNum++; 
			ss  << "<StreamResources " 
				<< "ID=\""                    << ctrlNum
				<< "\" SOP=\""                << _context.sopname
				<< "\" filename=\""           << resIter->contentName
				<< "\" providerID=\""         << resIter->privateData["providerId"].strs[0]
				<< "\" assetID=\""            << resIter->privateData["providerAssetId"].strs[0]
				<< "\" segmentNumber=\""      << ctrlNum
				<< "\" />" << CRLF;
		}

		/*NGODr2c1::StreamResources::iterator resIter = _context.streamRes.begin();
		for (; resIter != _context.streamRes.end(); resIter++)
		{
			ss  << "<StreamResources " 
				<< "ID=\""                    << resIter->ctrlNum
				<< "\" SOP=\""                   << _context.sopname
				<< "\" filename=\""           << resIter->fileName
				<< "\" providerID=\""         << resIter->providerID
				<< "\" assetID=\""            << resIter->assetID
				<< "\" segmentNumber=\""      << resIter->ctrlNum
				<< "\" />" << CRLF;
		}*/
	}

	
	if (true == _ssmNGODr2c1._pSite->destroyClientSession(_session.c_str()))
		HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(TeardownHandler, "rtspProxy session is destroyed"));

	INOUTMAP inoutMap;
	::TianShanIce::Streamer::StreamPrx streamPrx = NULL;
	inoutMap[MAP_KEY_STREAMFULLID] = _context.streamFullID;
	std::string strStopPoint ;
	if (true == _ssmNGODr2c1.getStream(streamPrx, inoutMap))
	{
		_ssmNGODr2c1.getPositionAndScale(streamPrx, inoutMap, _requireProtocol);
		strStopPoint = inoutMap[MAP_KEY_STREAMPOSITION_STOPPOINT];
	}
	else
	{
		HANDLERLOG(ZQ::common::Log::L_WARNING,HANDLERLOGFMT(TeardownHandler, "failed to find the stream instance"));
	}

	// add by zjm to support session history
	if (_ngodConfig._sessionHistory.enableHistory > 0)
	{
		if (_context.prop["hasEndEvent"] != "true")
		{
			std::string strPosition = inoutMap[MAP_KEY_STREAMPOSITION];
			NGODr2c1::SessionEventRecord sessionEvent;
			sessionEvent.eventType = NGODr2c1::EndEvent;
			sessionEvent.eventTime = NgodUtilsClass::generatorISOTime();
			sessionEvent.prop["reason"] = "USER";
						
			std::string strNPT = inoutMap["SessionAssetNPT"];
			if (strNPT.empty() )
			{
				strNPT = "EOS";
			}
			sessionEvent.NPT = strNPT;
			sessionEvent.streamResourceID = inoutMap["stopIndex"];
			_context.sessionEvents.push_back(sessionEvent);
			//_pContextPrx->addEventRecord(sessionEvent);
		}
	}

	//destroyStream();
	if ( !mStreamPrx  ) 
	{
		try
		{
			if( !_context.streamFullID.empty()  )
				mStreamPrx = TianShanIce::Streamer::PlaylistPrx::checkedCast(_ssmNGODr2c1._pCommunicator->stringToProxy(_context.streamFullID));
		}		
		catch (const Ice::ObjectNotExistException& ) 
		{			
			mStreamPrx = NULL;
		}
		catch( const Ice::Exception& ex )
		{
			HANDLERLOG(ZQ::common::Log::L_ERROR,HANDLERLOGFMT(TeardownHandler, "failed to contact the stream instance"));
			HANDLEREVENTLOG(ZQ::common::Log::L_ERROR,HANDLERLOGFMT(TeardownHandler, "failed to contact the stream instance"));
			//response error
			responseError(RESPONSE_INTERNAL_ERROR);
			return RequestError;
		}
	}
	if (mStreamPrx) 
	{
		try
		{
			mStreamPrx->destroy();
			mStreamPrx = NULL;
		}
		catch ( const Ice::ObjectNotExistException& ) 
		{
			mStreamPrx = NULL;
		}
		catch( const Ice::Exception& ex )
		{
			HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(TeardownHandler, "destroy stream caught exception[%s]"),
				ex.ice_name().c_str() );
			HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(TeardownHandler, "destroy stream caught exception[%s]"),
				ex.ice_name().c_str() );
			//response error
			responseError(RESPONSE_INTERNAL_ERROR);
			return RequestError;
		}

	}

	if (_ngodConfig._sessionHistory.enableHistory > 0)
	{
		ss << "<EventHistory>" << CRLF;
		NGODr2c1::SessionEventRecords::iterator eventIter = _context.sessionEvents.begin();
		for (; eventIter != _context.sessionEvents.end(); eventIter++)
		{
			if (_ngodConfig._sessionHistory.enablePlayEvent <= 0 && eventIter->prop["newState"] == "PLAY")
			{
				// play event is disable
				continue;
			}
			if (_ngodConfig._sessionHistory.enablePauseEvent <= 0 && eventIter->prop["newState"] == "PAUSE")
			{
				// pause event is disable
				continue;
			}
			switch (eventIter->eventType)
			{
			case NGODr2c1::UserEvent:
				ss  << "<UserEvent time=\""     << eventIter->eventTime
					<< "\" NPT=\""              << eventIter->NPT
					<< "\" streamResourcesID=\"" << eventIter->streamResourceID
					<< "\" newState=\""         << eventIter->prop["newState"]
					<< "\" scale=\""            << eventIter->prop["scale"]
					<< "\" />" << CRLF;
				break;
			case NGODr2c1::StartStreamEvent:
				ss	<< "<StartStreamEvent time=\"" << eventIter->eventTime
					<< "\" NPT=\""                 << eventIter->NPT
					<< "\" streamResourcesID=\""    << eventIter->streamResourceID
					<< "\" newState=\""            << eventIter->prop["newState"]
					<< "\" scale=\""               << eventIter->prop["scale"]
					<< "\" />" << CRLF;
				break;
			case NGODr2c1::EndEvent:
				ss	<< "<EndEvent time=\""        << eventIter->eventTime
					<< "\" NPT=\""                << eventIter->NPT
					<< "\" streamResourcesID=\""   << eventIter->streamResourceID
					<< "\" reason=\""             << eventIter->prop["reason"]
					<< "\" />" << CRLF;
				break;
			case NGODr2c1::RecoverableError:
				ss	<< "<RecoverableError time=\""   << eventIter->eventTime
					<< "\" NPT=\""                   << eventIter->NPT
					<< "\" streamResourcesID=\""      << eventIter->streamResourceID
					<< "\" newState=\""              << eventIter->prop["newState"]
					<< "\" scale=\""                 << eventIter->prop["scale"]
					<< "\" />" << CRLF;
				break;
			case NGODr2c1::Transition:
				ss	<< "<Transition time=\""        << eventIter->eventTime
					<< "\" NPT=\""                  << eventIter->NPT
					<< "\" streamResourcesID=\""     << eventIter->streamResourceID
					<< "\" newState=\""             << eventIter->prop["newState"]
					<< "\" scale=\""                << eventIter->prop["scale"]
					<< "\" reason=\""               << eventIter->prop["reason"]
					<< "\" newNPT=\""               << eventIter->prop["newNPT"]
					<< "\" newStreamResourcesID=\"" << eventIter->prop["newStreamResourcesID"]
					<< "\" />" << CRLF;
				break;
			default:
				break;
			}
		}
		ss << "</EventHistory>"   << CRLF;
		ss << "</PlayoutHistory>" << CRLF;
		ss << "</ODRMSession>"  << CRLF;
	}
	ss  << "</ODRMSessionHistory>"<< CRLF
		<< "</ResponseData>"      << CRLF;

	removeContext((false == _xreason.empty()) ? _xreason : "teardown");

	std::string tmp = getRequestHeader("NeedResponse", ZQ::common::Log::L_NOTICE);
	if (0 == stricmp(tmp.c_str(), "no"))
	{
		HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(TeardownHandler, "muted the response per user's request"));
		return RequestProcessed;
	}
	_pResponse->setHeader( "StopPoint", strStopPoint.c_str() );
	_pResponse->setHeader(NGOD_HEADER_ONDEMANDSESSIONID, onDemandID.c_str());
	_pResponse->setHeader(NGOD_HEADER_CONTENTTYPE, "text/xml");
	_retContent = ss.str();
	_pResponse->printf_postheader(_retContent.c_str());
	responseOK();
	return RequestProcessed;
}