// File Name : RequestHandler.cpp

#include "RequestHandler.h"

#include "GBssConfig.h"

#include "StreamersConfig.h"

#include "Environment.h"

#include "SelectionResourceManager.h"

#include "stroprt.h"

#include "CRGSessionManager.h"

// stream smith RTSP Define
#include "RtspRelevant.h"

#include "RtspHeaderDefines.h"

#include <TianShanIceHelper.h>

namespace GBss
{

RequestHandler::RequestHandler(ZQ::common::Log& fileLog, Environment& env, 
							   IStreamSmithSite* pSite, IClientRequestWriter* pReq)
:_fileLog(fileLog), _env(env), _site(pSite), _request(pReq), 
_response(NULL), _method("Unimplemented Method"), _sessionProxy(NULL), _playlistPrx(NULL),
_statusCode(200), _startTime(ZQTianShan::now()), _returnType(RETURN_SYNC)
{
	_szBuf[sizeof(_szBuf) - 1] = '\0';
}

RequestHandler::~RequestHandler()
{

}

RequestProcessResult RequestHandler::doContentHandler()
{
	HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(RequestHandler, "start processing"));
	_statusCode = 405;
	return RequestProcessed;
}

RequestProcessResult RequestHandler::process()
{
	if (!preprocess())
	{
		_statusCode = 400;
		composeResponse();
		HANDLERLOG(ZQ::common::Log::L_ALERT, CLOGFMT(RequestHandler, "%s"), _szBuf);
		return RequestError;
	}
	_sequence = getRequestHeader(HeaderSequence);
	_session = getRequestHeader(HeaderSession);
	if ("" == _session)
	{
		_session = _env.getGlobalSession();
	}

	RequestProcessResult ret = doContentHandler();
	if (RETURN_SYNC == _returnType) // sync process
	{
		composeResponse();
	}
	Ice::Long timeUsed = ZQTianShan::now() - _startTime;
	HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(RequestHandler, "process reqeust used time[%lld ms]"), timeUsed);
	return ret;
}

bool RequestHandler::preprocess()
{
	if (NULL == _site)
	{
		_statusCode = 500;
		snprintf(_szBuf, sizeof(_szBuf) - 1, "StreamSmithSite is empty");
		return false;
	}
	if (NULL == _request)
	{
		_statusCode = 500;
		snprintf(_szBuf, sizeof(_szBuf) - 1, "Client Request is empty");
		return false;
	}
	_response = _request->getResponse();
	if (NULL == _response)
	{
		_statusCode = 500;
		snprintf(_szBuf, sizeof(_szBuf) - 1, "Response Object is null");
		return false;
	}
	return true;
}

std::string RequestHandler::getRequestHeader(const char *pHeader, int logLevel)
{
	if (NULL == pHeader || 0 == strlen(pHeader))
	{
		return "";
	}
	memset(_szBuf, 0, sizeof(_szBuf));
	uint16 szBufLen = sizeof(_szBuf) - 1;
	const char* pRet = _request->getHeader(pHeader, _szBuf, &szBufLen);
	std::string ret = (NULL != pRet) ? pRet : "";
	ZQ::StringOperation::trimAll(ret);
	HANDLERLOG(logLevel, HANDLERLOGFMT(RequestHandler, "get header [%s: %s]"), pHeader, ret.c_str());
	return ret;
}

void RequestHandler::getContentBody(int logLevel)
{
	std::string contentSize = getRequestHeader(HeaderContentLength);
	if ("" == contentSize)
	{
		_requestBody = "";
		return;
	}

	uint32 contentLen = atoi(contentSize.c_str()) + 1;
	unsigned char* pContentBody = new unsigned char[contentLen];
	memset(pContentBody, 0, contentLen); 
	const char* pRet = _request->getContent(pContentBody, &contentLen);
	_requestBody = (NULL != pRet) ? pRet : "";
	delete []pContentBody;
	HANDLERLOG(logLevel, HANDLERLOGFMT(RequestHandler, "get content body: [%s]"), _requestBody.c_str());
}

bool RequestHandler::getSessionContext()
{
	if (!findSession())
	{
		return false;
	}
	try
	{
		_sessionContext = _sessionProxy->getMetaData();
		HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(RequestHandler, "successed to get session[%s] context"), _session.c_str());
	}
	catch(const Ice::ObjectNotExistException& ex)
	{
		_statusCode = 454;
		snprintf(_szBuf, sizeof(_szBuf) - 1, "get session[%s] context caught[%s]", _session.c_str(), ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), _szBuf);
		return false;
	}
	catch (const Ice::Exception& ex)
	{
		_statusCode = 500;
		snprintf(_szBuf, sizeof(_szBuf) - 1, "get session[%s] context caught[%s]", _session.c_str(), ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), _szBuf);
		return false;
	}
	return true;
}

void RequestHandler::composeResponse()
{
	// error response 
	if (_statusCode < 200 || _statusCode >= 300)
	{
		std::string strErrorNotice = _strLastError.empty() ? _szBuf : _strLastError;
		_response->setHeader(HeaderNotice, strErrorNotice.c_str());
	}

	std::string strNeedResponse = getRequestHeader("NeedResponse");
	if (strNeedResponse == "no")
	{
		HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(PingHandler, "it need't response"));
		return ;
	}
	std::string statusMsg;
	switch (_statusCode)
	{
	case 100:
		statusMsg = "Continue";
		break;

	case 200: // "RTSP/1.0 200 OK"
		statusMsg = "OK"; 
		break; 

	case 400: // "RTSP/1.0 400 Bad Request"
		statusMsg = "Bad Request";
		break; 

	case 401: // "RTSP/1.0 401 Unauthorized"
		statusMsg = "Unauthorized"; 
		break; 

	case 404: // "RTSP/1.0 404 Not Found"
		statusMsg = "Not Found";
		break; 

	case 405: // "RTSP/1.0 405 Method Not Allowed"
		statusMsg = "Method Not Allowed";
		break; 

	case 406: // "RTSP/1.0 406 Not Acceptable"
		statusMsg = "Not Acceptable"; 
		break; 

	case 451: 
		statusMsg = "Parameter Not Understood";
		break; 

	case 452: // "RTSP/1.0 452 Conference Not Found"
		statusMsg = "Conference Not Found"; 
		break;

	case 453: // "RTSP/1.0 453 Not Enough Bandwidth"
		statusMsg = "Not Enough Bandwidth";
		break; 

	case 454: // "RTSP/1.0 454 Session Not Found"
		statusMsg = "Session Not Found"; 
		break; 

	case 455: // "RTSP/1.0 455 Method Not Valid in This State"
		statusMsg = "Method Not Valid in This State"; 
		break; 

	case 457: // "RTSP/1.0 457 Invalid Range"
		statusMsg = "Invalid Range";
		break;

	case 461: // "RTSP/1.0 461 Unsupported Transport"
		statusMsg = "Unsupported Transport";
		break; 

	case 500: // "RTSP/1.0 500 Internal Server Error"
		statusMsg = "Internal Server Error";
		break; 

	case 501: // "RTSP/1.0 501 Not Implemented"
		statusMsg = "Not Implemented";
		break; 

	case 503: // "RTSP/1.0 503 Service Unavailable"
		statusMsg = "Service Unavailable";
		break; 

	case 610: // "RTSP/1.0 610 Unexpect Client Error"
		statusMsg = "Unexpect Client Error";
		break; 

	case 620: // "RTSP/1.0 620 Unexpect Server Error" 
		statusMsg = "Unexpect Server Error"; 
		break;

	case 770:
		_statusCode = 620;
		//statusMsg = "ServerSetupFailed No Response";
		statusMsg = "Unexpect Server Error";
		break;

	case 771:
		_statusCode = 404;
		statusMsg = "Asset Not Found";
		break;

	case 775:
		_statusCode = 453;
		//statusMsg = "ServerSetupFailed InsufficientVolumeBandwidth";
		statusMsg = "Not Enough Bandwidth";
		break;

	case 776:
		_statusCode = 453;
		//statusMsg = "ServerSetupFailed InsufficientNetworkBandwidth";
		statusMsg = "Not Enough Bandwidth";
		break;

	case 777:
		_statusCode = 400;
		// statusMsg = "ServerSetupFailed InvalidRequest";
		statusMsg = "Bad Request";
		break;

	default:
		_statusCode = 500;
		statusMsg = "Internal Server Error";
		break;
	}

	// RTSP Header 
	char buf[128];
	snprintf(buf, sizeof(buf)-2, "RTSP/1.0 %d %s", _statusCode, statusMsg.c_str());
	_response->printf_preheader(buf);

	// Sequence 
	_response->setHeader(HeaderSequence, _sequence.c_str());
	
	// optional Method header 
	_response->setHeader(HeaderMethodCode, _method.c_str());

	_response->setHeader(HeaderServer, ZQ_COMPONENT_NAME);
	
	if (_sessionProxy)
	{
		_response->setHeader(HeaderGlobalSessId, _sessionProxy->getGlobalSessId().c_str());
	}
	
	_response->setHeader("Date", (char*)_env.getUTCTime().c_str());

	_response->post();
}

bool RequestHandler::renewSession(Ice::Long ttl)
{
	if (!findSession())
	{
		return false;
	}

	// DO: renew client session
	try
	{
		_sessionProxy->renew(ttl);
		HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(RequestHandler, "ClientSession(%s) renewed(%lldms)"), _session.c_str(), ttl);
	}
	catch( const Ice::ObjectNotExistException& ex )
	{
		_statusCode = 454;
		snprintf(_szBuf, sizeof(_szBuf) - 1, "renewClientSession(%s) caught(%s)", _session.c_str(), ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), _szBuf);
		return false;
	}
	catch (const Ice::Exception& ex)
	{
		_statusCode = 500;
		snprintf(_szBuf, sizeof(_szBuf) - 1, "renewClientSession(%s) caught(%s)", _session.c_str(), ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), _szBuf);
		return false;
	}

	// watch session
	_env.watchSession(_session);
	return true;
}

bool RequestHandler::updateSessionMetaData(const std::string &key, const std::string &val)
{
	if (!findSession())
	{
		return false;
	}

	TianShanIce::Properties sessionMetaData;
	sessionMetaData.insert(std::make_pair(key, val));
	try
	{
		_sessionProxy->setMetaData(sessionMetaData);
	}
	catch (const Ice::ObjectNotExistException& ex)
	{
		_statusCode = 454;
		snprintf(_szBuf, sizeof(_szBuf) - 1, "caught [%s] when set session meta data", ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), _szBuf);
		return false;
	}
	catch (const Ice::Exception& ex)
	{
		_statusCode = 500;
		snprintf(_szBuf, sizeof(_szBuf) - 1, "caught [%s] when set session meta data", ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), _szBuf);
		return false;
	}
	return true;
}

bool RequestHandler::getPositionAndScale(std::string& range, std::string& scale)
{
	if (!findPlaylist())
	{
		return false;
	}

	TianShanIce::ValueMap valMap;
	try
	{
		_playlistPrx->getInfo(::TianShanIce::Streamer::infoSTREAMNPTPOS, valMap);
	}
	catch(const Ice::TimeoutException& ex)
	{
		addPenalty();
		_statusCode = 503;
		snprintf(_szBuf, sizeof(_szBuf) - 1, "caught [%s] when get stream info", ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), _szBuf);
		return false;
	}
	catch(const Ice::ConnectionRefusedException& ex)
	{
		addPenalty();
		_statusCode = 503;
		snprintf(_szBuf, sizeof(_szBuf) - 1, "caught [%s] when get stream info", ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), _szBuf);
		return false;
	}
	catch(const Ice::Exception& ex)
	{
		_statusCode = 500;
		snprintf(_szBuf, sizeof(_szBuf) - 1, "caught [%s] when get stream info", ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), _szBuf);
		return false;
	}

	// get stream scale 
	TianShanIce::ValueMap::iterator itScale = valMap.find("scale");

	// get stream current position
	TianShanIce::ValueMap::iterator itCurPosition = valMap.find("playposition");

	// get stream total position
	TianShanIce::ValueMap::iterator itTotalPosition = valMap.find("totalplaytime");


	if (itScale != valMap.end() && !itScale->second.strs.empty() 
		&& itCurPosition != valMap.end() && !itCurPosition->second.ints.empty()
		&& itTotalPosition != valMap.end() && !itTotalPosition->second.ints.empty())
	{
		scale = itScale->second.strs[0];
		int cur = itCurPosition->second.ints[0];
		int total = itTotalPosition->second.ints[0];
		if (total != 0)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%03d.%03d-%03d.%03d", cur / 1000, cur % 1000, total / 1000, total % 1000);
		}
		else
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "%03d.%03d-", cur / 1000, cur % 1000 );
		}
		range = _szBuf;
	}
	return true;
}

bool RequestHandler::getStopNPT(int& playPosition, int& npt)
{
	if (!findPlaylist())
	{
		return false;
	}

	TianShanIce::ValueMap valMap;
	try
	{
		_playlistPrx->getInfo(::TianShanIce::Streamer::infoSTREAMNPTPOS, valMap);
		ZQTianShan::Util::getValueMapDataWithDefault( valMap , "playposition" , 0 , playPosition );
		ZQTianShan::Util::getValueMapDataWithDefault( valMap , "itemOffset", -1 , npt );
		TianShanIce::Streamer::StreamState state = _playlistPrx->getCurrentState();
		if(state == TianShanIce::Streamer::stsStop)
		{
			HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(RequestHandler, "playlist is at stopped stage, set npt to -1"));
			npt = -1;
		}
	}
	catch(const Ice::TimeoutException& ex)
	{
		addPenalty();
		_statusCode = 503;
		snprintf(_szBuf, sizeof(_szBuf) - 1, "caught [%s] when get stream info", ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), _szBuf);
		return false;
	}
	catch(const Ice::ConnectionRefusedException& ex)
	{
		addPenalty();
		_statusCode = 503;
		snprintf(_szBuf, sizeof(_szBuf) - 1, "caught [%s] when get stream info", ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), _szBuf);
		return false;
	}
	catch(const Ice::Exception& ex)
	{
		_statusCode = 500;
		snprintf(_szBuf, sizeof(_szBuf) - 1, "caught [%s] when get stream info", ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), _szBuf);
		return false;
	}

	return true;
}

bool RequestHandler::getStreamSourceAddressInfo( std::string& sourceIP, int& sourcePort )
{
	if (!findPlaylist())
	{
		return false;
	}

	TianShanIce::ValueMap valMap;
	try
	{
		_playlistPrx->getInfo(::TianShanIce::Streamer::infoSTREAMSOURCE, valMap);
		ZQTianShan::Util::getValueMapDataWithDefault( valMap , "StreamingSourceIp" , "" , sourceIP );
		ZQTianShan::Util::getValueMapDataWithDefault( valMap , "StreamingSourcePort", 0 , sourcePort );
	}
	catch(const Ice::TimeoutException& ex)
	{
		addPenalty();
		_statusCode = 503;
		snprintf(_szBuf, sizeof(_szBuf) - 1, "caught [%s] when get stream info", ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), _szBuf);
		return false;
	}
	catch(const Ice::ConnectionRefusedException& ex)
	{
		addPenalty();
		_statusCode = 503;
		snprintf(_szBuf, sizeof(_szBuf) - 1, "caught [%s] when get stream info", ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), _szBuf);
		return false;
	}
	catch(const Ice::Exception& ex)
	{
		_statusCode = 500;
		snprintf(_szBuf, sizeof(_szBuf) - 1, "caught [%s] when get stream info", ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), _szBuf);
		return false;
	}

	return true;
}

void RequestHandler::addPenalty()
{
	_env.getResourceManager().applyPenalty(_streamerNetId, _streamersConfig._streamingResource._maxPenaltyValue);
}

bool RequestHandler::getPlaylistState(TianShanIce::Streamer::StreamState& streamState)
{
	if (!findPlaylist())
	{
		return false;
	}
	try
	{
		streamState = _playlistPrx->getCurrentState();
	}
	catch(const Ice::TimeoutException& ex)
	{
		addPenalty();
		_statusCode = 503;
		snprintf(_szBuf, sizeof(_szBuf) - 1, "caught [%s] when get stream state", ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), _szBuf);
		return false;
	}
	catch(const Ice::ConnectionRefusedException& ex)
	{
		addPenalty();
		_statusCode = 503;
		snprintf(_szBuf, sizeof(_szBuf) - 1, "caught [%s] when get stream state", ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), _szBuf);
		return false;
	}
	catch (const Ice::Exception& ex)
	{
		_statusCode = 500;
		snprintf(_szBuf, sizeof(_szBuf) - 1, "caught [%s] when get stream state", ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), _szBuf);
		return false;
	}
	return true;
}

std::string RequestHandler::getStateString(TianShanIce::Streamer::StreamState &streamState)
{
	std::string strState;
	switch(streamState)
	{
	case TianShanIce::Streamer::stsSetup:
		{
			strState = "stopped";
		}
		break;
	case TianShanIce::Streamer::stsStreaming: 
		{
			strState = "playing";
		}
		break;
	case TianShanIce::Streamer::stsPause: 
		{
			strState = "paused";
		}
		break;
	case TianShanIce::Streamer::stsStop:
		{
			strState = "stopped";
		}
		break;
	default: 
		{
			strState = "stopped";
		}
		break;
	}
	return strState;
}


bool RequestHandler::findPlaylist()
{
	if (NULL != _playlistPrx)
	{
		HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(RequestHandler, "stream proxy already gained"));
		return true;
	}
	if (!findSession())
	{
		return false;
	}

	try
	{
		_playlistPrx = TianShanIce::Streamer::PlaylistPrx::uncheckedCast(_sessionProxy->getStream(_streamerNetId, _streamId));
	}
	catch (const Ice::ObjectNotExistException& ex)
	{
		_statusCode = 454;
		snprintf(_szBuf, sizeof(_szBuf) - 1, "caught [%s] when got play list[%s]", ex.ice_name().c_str(), _streamId.c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), _szBuf);
		return false;
	}
	catch (const Ice::Exception& ex)	
	{
		_statusCode = 500;
		snprintf(_szBuf, sizeof(_szBuf) - 1, "caught [%s] when got play list[%s]", ex.ice_name().c_str(), _streamId.c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), _szBuf);
		return false;
	}	
	if (NULL == _playlistPrx)
	{
		_statusCode = 454;
		snprintf(_szBuf, sizeof(_szBuf) - 1, "play list is empty");
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(RequestHandler, "%s"), _szBuf);
		return false;
	}
	HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(RequestHandler, "stream: [%s] gained successfully"), _streamId.c_str());
	return true;
}

bool RequestHandler::findSession()
{
	if (NULL != _sessionProxy)
	{
		HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(RequestHandler, "session proxy already gained"));
		return true;
	}

	// if session exist
	_sessionProxy = _env.getSessionManager().findSession(_session, _strLastError, _statusCode);
	return NULL != _sessionProxy;
}

} // end GBss
