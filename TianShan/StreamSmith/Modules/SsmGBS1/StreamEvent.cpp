#include "./StreamEvent.h"
#include <TianShanIceHelper.h>

extern ZQ::common::Log* s1log;

namespace TianShanS1
{

StreamEvent::StreamEvent(Environment& env) : _env(env)
{
}

StreamEvent::~StreamEvent()
{
}

void StreamEvent::ping(::Ice::Long lv, const ::Ice::Current& ic)
{
}

void StreamEvent::OnEndOfStream(const ::std::string& proxy, const ::std::string& playlistId, const TianShanIce::Properties& props , const ::Ice::Current& ic) const
{
	try
	{
		SSMLOG(DebugLevel, CLOGFMT(StreamEvent, "OnEndOfStream(%s)"), proxy.c_str());

		std::vector<Ice::Identity> idents;
		idents = _env._pStreamIdx->findFirst(playlistId, 1);
		if (idents.size() == 0)
			return;
	
		SessionContextPrx cltSessPrx = NULL;
		SessionData cltSessCtx;
		if (false == _env.openSessionCtx(idents[0], cltSessPrx, cltSessCtx))
			return;

		::Ice::Int announceNumber = cltSessPrx->addAnnounceSeq();
		SSMLOG(InfoLevel, CLOGFMT(StreamEvent, "OnEndOfStream(%s:%s)"), proxy.c_str(), cltSessCtx.ident.name.c_str());

		IServerRequest* pServerRequest = NULL;
		SmartServerRequest smtRequest(pServerRequest);
		pServerRequest = _env._pSite->newServerRequest(idents[0].name.c_str());
		if (NULL == pServerRequest)
		{
			SSMLOG(ErrorLevel, CLOGFMT(StreamEvent, "OnEndOfStream(%s:%s), create server request failed."), proxy.c_str(), cltSessCtx.ident.name.c_str());
			return;
		}

		char szBuf[2048];
		szBuf[sizeof(szBuf) - 1] = '\0';
		std::string hdrStr = "ANNOUNCE " + cltSessCtx.url + " RTSP/1.0";
		pServerRequest->printCmdLine(hdrStr.c_str());
		pServerRequest->printHeader(HeaderSession, (char*) idents[0].name.c_str());
		pServerRequest->printHeader(HeaderServer, (char*) _env._serverHeader.c_str());
		snprintf(szBuf, sizeof(szBuf) - 1, "%d", announceNumber);
		pServerRequest->printHeader(HeaderSequence, szBuf);
		pServerRequest->printHeader("GlobalSession", (char *)cltSessCtx.props["GlobalSession"].c_str());

		std::string pos_str;
		ZQTianShan::Util::getPropertyDataWithDefault( props , "streamNptPosition" , "" , pos_str );
		SYS::TimeStamp ts;
		snprintf(szBuf, sizeof(szBuf) - 1, SC_ANNOUNCE_ENDOFSTREAM " " \
			SC_ANNOUNCE_ENDOFSTREAM_STRING " " "Event-Date=%04d%02d%02dT%02d%02d%02d.%03dZ " "npt=%s"\
			, ts.year, ts.month, ts.day, ts.hour, ts.minute, ts.second, ts.millisecond, pos_str.c_str());
		pServerRequest->printHeader("notice", szBuf);

		pServerRequest->post();
		SSMLOG(DebugLevel, CLOGFMT(StreamEvent, "OnEndOfStream(%s:%s) has been sent out."), proxy.c_str(), cltSessCtx.ident.name.c_str());
	}
	catch (...)
	{
		SSMLOG(DebugLevel, CLOGFMT(StreamEvent, "OnEndOfStream(%s) caught unexpet exception"), proxy.c_str());
	}
}

void StreamEvent::OnBeginningOfStream(const ::std::string& proxy, const ::std::string& playlistId, const TianShanIce::Properties& props , const ::Ice::Current& ic) const
{
	try 
	{
		SSMLOG(DebugLevel, CLOGFMT(StreamEvent, "OnBeginningOfStream(%s)"), proxy.c_str());

		std::vector<Ice::Identity> idents;
		idents = _env._pStreamIdx->findFirst(playlistId, 1);
		if (idents.size() == 0)
			return;

		SessionContextPrx cltSessPrx = NULL;
		SessionData cltSessCtx;
		if (false == _env.openSessionCtx(idents[0], cltSessPrx, cltSessCtx))
			return;

		::Ice::Int announceNumber = cltSessPrx->addAnnounceSeq();
		SSMLOG(InfoLevel, CLOGFMT(StreamEvent, "OnBeginningOfStream(%s:%s)"), proxy.c_str(), cltSessCtx.ident.name.c_str());

		IServerRequest* pServerRequest = NULL;
		SmartServerRequest smtRequest(pServerRequest);
		pServerRequest = _env._pSite->newServerRequest(idents[0].name.c_str());
		if (NULL == pServerRequest)
		{
			SSMLOG(ErrorLevel, CLOGFMT(StreamEvent, "OnBeginningOfStream(%s:%s), create server request failed."), proxy.c_str(), cltSessCtx.ident.name.c_str());
			return;
		}

		char szBuf[2048];
		szBuf[sizeof(szBuf) - 1] = '\0';
		std::string hdrStr = "ANNOUNCE " + cltSessCtx.url + " RTSP/1.0";
		pServerRequest->printCmdLine(hdrStr.c_str());
		pServerRequest->printHeader(HeaderSession, (char*) idents[0].name.c_str());
		pServerRequest->printHeader(HeaderServer, (char*) _env._serverHeader.c_str());
		snprintf(szBuf, sizeof(szBuf) - 1, "%d", announceNumber);
		pServerRequest->printHeader(HeaderSequence, szBuf);
		pServerRequest->printHeader("GlobalSession", (char *)cltSessCtx.props["GlobalSession"].c_str());

		std::string pos_str;
		ZQTianShan::Util::getPropertyDataWithDefault( props , "streamNptPosition" , "" , pos_str );
		SYS::TimeStamp ts;
		snprintf(szBuf, sizeof(szBuf) - 1, SC_ANNOUNCE_BEGINOFSTREAM " " \
			"Start-of-Stream Reached" " " "Event-Date=%04d%02d%02dT%02d%02d%02d.%03dZ " "npt=%s"\
			, ts.year, ts.month, ts.day, ts.hour, ts.minute, ts.second, ts.millisecond, pos_str.c_str());
		pServerRequest->printHeader("notice", szBuf);

		pServerRequest->post();

		SSMLOG(DebugLevel, CLOGFMT(StreamEvent, "OnBeginningOfStream(%s:%s) has been sent out."), proxy.c_str(), cltSessCtx.ident.name.c_str());
	}
	catch (...)
	{
		SSMLOG(DebugLevel, CLOGFMT(StreamEvent, "OnBeginningOfStream(%s) caught unexpet exception"), proxy.c_str());
	}
}

void StreamEvent::OnSpeedChanged(const ::std::string& proxy, const ::std::string& playlistId, ::Ice::Float prevSpeed, ::Ice::Float currentSpeed, const TianShanIce::Properties& props , const ::Ice::Current& ic) const
{
	try 
	{
		SSMLOG(DebugLevel, CLOGFMT(StreamEvent, "OnSpeedChanged(%s)"), proxy.c_str());
/*
		std::vector<Ice::Identity> idents;
		idents = _env._pStreamIdx->findFirst(playlistId, 1);
		if (idents.size() == 0)
			return;

		SessionContextPrx cltSessPrx = NULL;
		SessionData cltSessCtx;
		if (false == _env.openSessionCtx(idents[0], cltSessPrx, cltSessCtx))
			return;

		std::map<std::string, std::string>::const_iterator icItor = ic.ctx.find(ICE_CONTEXT_EVENT_SEQUENCE);
		std::string curSeq = (ic.ctx.end() != icItor) ? icItor->second : "-1"; // because -1 is the initial value
		if (!cltSessPrx->canSendScaleChange(curSeq))
		{
			_env._fileLog(DebugLevel, CLOGFMT(StreamEvent, "OnSpeedChanged(%s), canSendScaleChange() return false, ignore this event"), proxy.c_str());
			return;
		}

		::Ice::Int announceNumber = cltSessPrx->addAnnounceSeq();
		SSMLOG(InfoLevel, CLOGFMT(StreamEvent, "OnSpeedChanged(%s:%s)"), proxy.c_str(), cltSessCtx.ident.name.c_str());

		IServerRequest* pServerRequest = NULL;
		SmartServerRequest smtRequest(pServerRequest);
		pServerRequest = _env._pSite->newServerRequest(idents[0].name.c_str());
		if (NULL == pServerRequest)
		{
			SSMLOG(ErrorLevel, CLOGFMT(StreamEvent, "OnSpeedChanged(%s:%s), create server request failed."), proxy.c_str(), cltSessCtx.ident.name.c_str());
			return;
		}

		char szBuf[2048];
		szBuf[sizeof(szBuf) - 1] = '\0';
		std::string hdrStr = "ANNOUNCE " + cltSessCtx.url + " RTSP/1.0";
		pServerRequest->printCmdLine(hdrStr.c_str());
		pServerRequest->printHeader(HeaderSession, (char*) idents[0].name.c_str());
		pServerRequest->printHeader(HeaderServer, (char*) _env._serverHeader.c_str());
		snprintf(szBuf, sizeof(szBuf) - 1, "%d", announceNumber);
		pServerRequest->printHeader(HeaderSequence, szBuf);

		if (cltSessCtx.requestType == 1) // SeaChange spec
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "%f", currentSpeed);
			pServerRequest->printHeader(HeaderScale, szBuf);
			SYS::TimeStamp ts;
			snprintf(szBuf, sizeof(szBuf) - 1, SC_ANNOUNCE_SCALECHANGED " " \
				SC_ANNOUNCE_SCALECHANGED_STRING " " "%04d%02d%02dT%02d%02d%02dZ" \
				, ts.year, ts.month, ts.day, ts.hour, ts.minute, ts.second);
			pServerRequest->printHeader(HeaderSeaChangeNotice, szBuf);
		}
		else // TianShan spec
		{
			TianShanIce::Streamer::PlaylistPrx playlistPrx = NULL;
			TianShanIce::Application::PurchasePrx purchasePrx = NULL;
			std::string tmp = "";
			TianShanIce::Properties::const_iterator it = cltSessCtx.props.find("sys.primaryItemNPT");
			if(it != cltSessCtx.props.end())
			{
				tmp = it->second;
			}
			else
			{
				it = props.find("sys.primaryItemNPT");
				if(it != props.end())
					tmp = it->second;
			}
			if (0 == stricmp(cltSessCtx.rangePrefix.c_str(), "clock"))
			{
				std::string scale, bcastPos;
				Ice::Int ctrlNum = 0, offset = 0;
				if (true == _env.getPlaylistPrx(playlistPrx, cltSessCtx.streamPrxID) && true == _env.getPurchasePrx(purchasePrx, cltSessCtx.purchasePrxID) && true == _env.getPlaylistPlayInfo(playlistPrx, cltSessCtx.streamPrxID, scale, ctrlNum, offset))
					_env.PlayInfo2UtcTime(purchasePrx, cltSessCtx.purchasePrxID, ctrlNum, offset, bcastPos);
				if(!tmp.empty())
				{
					snprintf(szBuf, sizeof(szBuf) - 1, "BcastPos=%s;Scale=%f;primaryItemNPT=%s"
						, bcastPos.c_str(), currentSpeed, tmp.c_str());
				}
				else
				{
					snprintf(szBuf, sizeof(szBuf) - 1, "BcastPos=%s;Scale=%f"
						, bcastPos.c_str(), currentSpeed);
				}
			}
			else 
			{
				std::string scale;
				Ice::Int curPos = 0, totalPos = 0;
				if (true == _env.getPlaylistPrx(playlistPrx, cltSessCtx.streamPrxID))
					_env.getStreamPlayInfo(playlistPrx, cltSessCtx.streamPrxID, scale, curPos, totalPos);
				if(!tmp.empty())
				{
					snprintf(szBuf, sizeof(szBuf) - 1, "npt=%d.%03d;Scale=%f;primaryItemNPT=%s"
						, curPos / 1000, curPos % 1000, currentSpeed, tmp.c_str());
				}
				else
				{
					snprintf(szBuf, sizeof(szBuf) - 1, "npt=%d.%03d;Scale=%f"
						, curPos / 1000, curPos % 1000, currentSpeed);
				}
			}
			pServerRequest->printHeader(HeaderTianShanNotice, (char*)TS_ANNOUNCE_SCALECHANGED);
			pServerRequest->printHeader(HeaderTianShanNoticeParam, szBuf);

			if (3 == cltSessCtx.requestType)// NGOD spec
			{
				SYS::TimeStamp ts;
				snprintf(szBuf, sizeof(szBuf) - 1, SC_ANNOUNCE_SCALECHANGED " " \
					SC_ANNOUNCE_SCALECHANGED_STRING " eventdate=" "%04d%02d%02dT%02d%02d%02d.%03dZ" \
					, ts.year, ts.month, ts.day, ts.hour, ts.minute, ts.second, ts.millisecond);
				pServerRequest->printHeader(HeaderNotice, szBuf);
				pServerRequest->printHeader(HeaderTianShanNotice, "Stream::0004 Scale Changed");
			}
		}
		pServerRequest->post();

		SSMLOG(DebugLevel, CLOGFMT(StreamEvent, "OnSpeedChanged(%s:%s) has been sent out."), proxy.c_str(), cltSessCtx.ident.name.c_str());
*/
	}
	catch (...)
	{
		SSMLOG(DebugLevel, CLOGFMT(StreamEvent, "OnSpeedChanged(%s) caught unexpet exception"), proxy.c_str());
	}
}

void StreamEvent::OnStateChanged(const ::std::string& proxy, const ::std::string& playlistId, ::TianShanIce::Streamer::StreamState prevState, ::TianShanIce::Streamer::StreamState currentState, const TianShanIce::Properties& props ,  const ::Ice::Current& ic) const 
{
	try 
	{
		SSMLOG(DebugLevel, CLOGFMT(StreamEvent, "OnStateChanged(%s)"), proxy.c_str());
/*
		std::vector<Ice::Identity> idents;
		idents = _env._pStreamIdx->findFirst(playlistId, 1);
		if (idents.size() == 0)
			return;

		SessionContextPrx cltSessPrx = NULL;
		SessionData cltSessCtx;
		if (false == _env.openSessionCtx(idents[0], cltSessPrx, cltSessCtx))
			return;

		if (cltSessCtx.requestType == 1) // SeaChange spec
			return; // not send

		// TianShan spec
		::Ice::Int announceNumber = cltSessPrx->addAnnounceSeq();
		SSMLOG(InfoLevel, CLOGFMT(StreamEvent, "OnStateChanged(%s:%s)"), proxy.c_str(), cltSessCtx.ident.name.c_str());

		IServerRequest* pServerRequest = NULL;
		SmartServerRequest smtRequest(pServerRequest);
		pServerRequest = _env._pSite->newServerRequest(idents[0].name.c_str());
		if (NULL == pServerRequest)
		{
			SSMLOG(ErrorLevel, CLOGFMT(StreamEvent, "OnStateChanged(%s:%s), create server request failed."), proxy.c_str(), cltSessCtx.ident.name.c_str());
			return;
		}

		char szBuf[2048];
		szBuf[sizeof(szBuf) - 1] = '\0';
		std::string hdrStr = "ANNOUNCE " + cltSessCtx.url + " RTSP/1.0";
		pServerRequest->printCmdLine(hdrStr.c_str());
		pServerRequest->printHeader(HeaderSession, (char*) idents[0].name.c_str());
		pServerRequest->printHeader(HeaderServer, (char*) _env._serverHeader.c_str());
		snprintf(szBuf, sizeof(szBuf) - 1, "%d", announceNumber);
		pServerRequest->printHeader(HeaderSequence, szBuf);

		std::string stateDept;
		switch (currentState)
		{
		case TianShanIce::Streamer::stsSetup: stateDept = "init"; break;
		case TianShanIce::Streamer::stsStreaming: stateDept = "play"; break;
		case TianShanIce::Streamer::stsPause: stateDept = "pause"; break;
		case TianShanIce::Streamer::stsStop: stateDept = "stop"; break;
		}
		TianShanIce::Streamer::PlaylistPrx playlistPrx = NULL;
		TianShanIce::Application::PurchasePrx purchasePrx = NULL;
		std::string tmp = "";
		TianShanIce::Properties::const_iterator it = cltSessCtx.props.find("sys.primaryItemNPT");
		if(it != cltSessCtx.props.end())
		{
			tmp = it->second;
		}
		else
		{
			it = props.find("sys.primaryItemNPT");
			if(it != props.end())
				tmp = it->second;
		}
		if (0 == stricmp(cltSessCtx.rangePrefix.c_str(), "clock"))
		{
			std::string scale, bcastPos;
			Ice::Int ctrlNum = 0, offset = 0;
			if (true == _env.getPlaylistPrx(playlistPrx, cltSessCtx.streamPrxID) && true == _env.getPurchasePrx(purchasePrx, cltSessCtx.purchasePrxID) && true == _env.getPlaylistPlayInfo(playlistPrx, cltSessCtx.streamPrxID, scale, ctrlNum, offset))
				_env.PlayInfo2UtcTime(purchasePrx, cltSessCtx.purchasePrxID, ctrlNum, offset, bcastPos);
			if(!tmp.empty())
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "BcastPos=%s;presentation_state=%s;primaryItemNPT=%s"
					, bcastPos.c_str(), stateDept.c_str(), tmp.c_str());
			}
			else
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "BcastPos=%s;presentation_state=%s"
					, bcastPos.c_str(), stateDept.c_str());
			}
		}
		else 
		{
			std::string scale;
			Ice::Int curPos = 0, totalPos = 0;
			if (true == _env.getPlaylistPrx(playlistPrx, cltSessCtx.streamPrxID))
				_env.getStreamPlayInfo(playlistPrx, cltSessCtx.streamPrxID, scale, curPos, totalPos);
			if(!tmp.empty())
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "npt=%d.%03d;presentation_state=%s;primaryItemNPT=%s"
					, curPos / 1000, curPos % 1000, stateDept.c_str(), tmp.c_str());
			}
			else
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "npt=%d.%03d;presentation_state=%s"
					, curPos / 1000, curPos % 1000, stateDept.c_str());
			}
		}
		pServerRequest->printHeader(HeaderTianShanNotice, (char*)TS_ANNOUNCE_STATECHANGED);
		pServerRequest->printHeader(HeaderTianShanNoticeParam, szBuf);
		pServerRequest->post();

		SSMLOG(DebugLevel, CLOGFMT(StreamEvent, "OnStateChanged(%s:%s) has been sent out."), proxy.c_str(), cltSessCtx.ident.name.c_str());
*/
	}
	catch (...)
	{
		SSMLOG(DebugLevel, CLOGFMT(StreamEvent, "OnStateChanged(%s) caught unexpet exception"), proxy.c_str());
	}
}

void StreamEvent::OnExit(const ::std::string& proxy, const ::std::string& playlistId, ::Ice::Int nExitCode, const ::std::string& sReason, const ::Ice::Current& ic) const
{
	SSMLOG(DebugLevel, CLOGFMT(StreamEvent, "OnExit(%s)"), proxy.c_str());
}

void StreamEvent::OnExit2(const ::std::string& proxy, const ::std::string& playlistId, ::Ice::Int nExitCode, const ::std::string& sReason, const ::TianShanIce::Properties& props, const ::Ice::Current&) const
{
	SSMLOG(DebugLevel, CLOGFMT(StreamEvent, "OnExit2(%s)"), proxy.c_str());
}

} // namespace TianShanS1

