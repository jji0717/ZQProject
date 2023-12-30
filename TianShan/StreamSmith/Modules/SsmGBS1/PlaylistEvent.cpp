#include "./PlaylistEvent.h"

extern ZQ::common::Log* s1log;

namespace TianShanS1
{
	
PlaylistEvent::PlaylistEvent(Environment& env) : _env(env)
{
}

PlaylistEvent::~PlaylistEvent()
{
}

void PlaylistEvent::ping(::Ice::Long lv, const ::Ice::Current& ic)
{
}

void PlaylistEvent::OnItemStepped(const ::std::string& proxy, const ::std::string& playlistId, 
								  ::Ice::Int userCtrlNum,::Ice::Int prevCtrlNum, 
								  const ::TianShanIce::Properties& prty,
								  const ::Ice::Current& ic) const
{
	try 
	{
		SSMLOG(DebugLevel, CLOGFMT(PlaylistEvent, "OnItemStepped(%s)"), proxy.c_str());
/*
		// if previous item is empty, just ignore the event
		TianShanIce::Properties tempMap = prty;
		if (tempMap["prevItemName"].empty())
		{
			bool bSendItemStepped = false;
			std::vector<DefaultParamHolder>::iterator it;
			for (it = _tsConfig._defaultParams._paramDatas.begin();
				it != _tsConfig._defaultParams._paramDatas.end(); it ++)
			{
				if ((*it)._name == "SendItemStepped" && atoi((*it)._value.c_str()) != 0)
				{
					bSendItemStepped = true;
					break;
				}
			}
			if (!bSendItemStepped)
			{
				SSMLOG(InfoLevel, CLOGFMT(PlaylistEvent, "OnItemStepped(%s) previous item name is empty and SendItemStepped is disabled, so ignore this event"), proxy.c_str());
				return;
			}
		}

		std::vector<Ice::Identity> idents;
		idents = _env._pStreamIdx->findFirst(playlistId, 1);
		if (idents.size() == 0)
			return;

		SessionContextPrx cltSessPrx = NULL;
		SessionData cltSessCtx;
		if (false == _env.openSessionCtx(idents[0], cltSessPrx, cltSessCtx))
			return;

		::Ice::Int announceNumber = cltSessPrx->addAnnounceSeq();
		SSMLOG(InfoLevel, CLOGFMT(PlaylistEvent, "OnItemStepped(%s:%s)"), proxy.c_str(), cltSessCtx.ident.name.c_str());

		IServerRequest* pServerRequest = NULL;
		SmartServerRequest smtRequest(pServerRequest);
		pServerRequest = _env._pSite->newServerRequest(idents[0].name.c_str());
		if (NULL == pServerRequest)
		{
			SSMLOG(ErrorLevel, CLOGFMT(PlaylistEvent, "OnItemStepped(%s:%s), create server request failed."), proxy.c_str(), cltSessCtx.ident.name.c_str());
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
			std::string itemname = tempMap["prevItemName"];
			SYS::TimeStamp ts;
			snprintf(szBuf, sizeof(szBuf) - 1, SC_ANNOUNCE_ITEMSTEPPED " " \
				SC_ANNOUNCE_ITEMSTEPPED_STRING " " "%04d%02d%02dT%02d%02d%02dZ" " " "\"%s\"" \
				, ts.year, ts.month, ts.day, ts.hour, ts.minute, ts.second, itemname.c_str());
			pServerRequest->printHeader(HeaderSeaChangeNotice, szBuf);
		}
		else // TianShan spec
		{
			TianShanIce::Streamer::PlaylistPrx playlistPrx = NULL;
			TianShanIce::Application::PurchasePrx purchasePrx = NULL;
			if (0 == stricmp(cltSessCtx.rangePrefix.c_str(), "clock"))
			{
				std::string scale, bcastPos;
				Ice::Int ctrlNum = 0, offset = 0;
				if (true == _env.getPlaylistPrx(playlistPrx, cltSessCtx.streamPrxID) && true == _env.getPurchasePrx(purchasePrx, cltSessCtx.purchasePrxID) && true == _env.getPlaylistPlayInfo(playlistPrx, cltSessCtx.streamPrxID, scale, ctrlNum, offset))
					_env.PlayInfo2UtcTime(purchasePrx, cltSessCtx.purchasePrxID, ctrlNum, offset, bcastPos);
				if (tempMap.find("sys.primaryItemNPT") != tempMap.end())
				{
					snprintf(szBuf, sizeof(szBuf) - 1, "BcastPos=%s;prev_item=%s;current_item=%s;primaryItemNPT=%s"
					, bcastPos.c_str() , tempMap["prevItemName"].c_str(), tempMap["currentItemName"].c_str(), tempMap["sys.primaryItemNPT"].c_str());
				}
				else
				{
					snprintf(szBuf, sizeof(szBuf) - 1, "BcastPos=%s;prev_item=%s;current_item=%s"
					, bcastPos.c_str() , tempMap["prevItemName"].c_str(), tempMap["currentItemName"].c_str());
				}
			}
			else 
			{
				std::string scale;
				Ice::Int curPos = 0, totalPos = 0;
				if (true == _env.getPlaylistPrx(playlistPrx, cltSessCtx.streamPrxID))
					_env.getStreamPlayInfo(playlistPrx, cltSessCtx.streamPrxID, scale, curPos, totalPos);
				if (tempMap.find("sys.primaryItemNPT") != tempMap.end())
				{
					snprintf(szBuf, sizeof(szBuf) - 1, "npt=%d.%03d;prev_item=%s;current_item=%s;primaryItemNPT=%s"
						, curPos / 1000, curPos % 1000, tempMap["prevItemName"].c_str(), tempMap["currentItemName"].c_str(), tempMap["sys.primaryItemNPT"].c_str());
				}
				else
				{
					snprintf(szBuf, sizeof(szBuf) - 1, "npt=%d.%03d;prev_item=%s;current_item=%s"
						, curPos / 1000, curPos % 1000, tempMap["prevItemName"].c_str(), tempMap["currentItemName"].c_str());
				}
			}
			pServerRequest->printHeader(HeaderTianShanNotice, (char*)TS_ANNOUNCE_ITEMSTEPPED);				
			pServerRequest->printHeader(HeaderTianShanNoticeParam, szBuf);
		}
		pServerRequest->post();

		SSMLOG(DebugLevel, CLOGFMT(PlaylistEvent, "OnItemStepped(%s:%s) has been sent out."), proxy.c_str(), cltSessCtx.ident.name.c_str());
*/
	}
	catch (...)
	{
		SSMLOG(DebugLevel, CLOGFMT(PlaylistEvent, "OnItemStepped(%s) caught unexpet exception"), proxy.c_str());
	}
}

} // namespace TianShanS1

