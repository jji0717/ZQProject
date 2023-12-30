#define _WINSOCKAPI_

#include "./SessionContextImpl.h"
#include "./Environment.h"

extern ZQ::common::Log* s1log;

namespace TianShanS1 {

	SessionContextImpl::SessionContextImpl(Environment& env) : _env(env)
	{
		props[PropKeyLastScaleChange] = "-1";
	}

	SessionContextImpl::~SessionContextImpl()
	{
	}

	::TianShanS1::SessionData SessionContextImpl::getSessionData(const ::Ice::Current&)
	{
		::IceUtil::Mutex::Lock lk(*this);

		::TianShanS1::SessionData data;

		data.ident = ident;
		data.streamID = streamID;
		data.streamPrxID = streamPrxID;
		data.purchasePrxID = purchasePrxID;
		data.srvrSessID = srvrSessID;
		data.srvrSessPrxID = srvrSessPrxID;
		data.rangePrefix = rangePrefix;
		data.requestType = requestType;
		data.announceSeq = announceSeq;
		data.url = url;
		data.props = props;

		return data;
	}

	::Ice::Int SessionContextImpl::addAnnounceSeq(const ::Ice::Current&)
	{
		::IceUtil::Mutex::Lock lk(*this);
		++announceSeq;
		return announceSeq;
	}

	void SessionContextImpl::setRangePrefix(const ::std::string& rngPf, const ::Ice::Current&)
	{
		::IceUtil::Mutex::Lock lk(*this);

		rangePrefix = rngPf;
	}

	bool SessionContextImpl::canSendScaleChange(const ::std::string& curSeq, const ::Ice::Current&)
	{
		::IceUtil::Mutex::Lock lk(*this);

		if (atoi(props[PropKeyLastScaleChange].c_str()) >= atoi(curSeq.c_str()))
		{
			_env._fileLog(DebugLevel, CLOGFMT(StreamEvent, "canSendScaleChange(%s:%s), previous sequence [%s] >= current sequence [%s], return false"), 
				streamID.c_str(), ident.name.c_str(), props[PropKeyLastScaleChange].c_str(), curSeq.c_str());
			return false;
		}
		props[PropKeyLastScaleChange] = curSeq;
		return true;
	}

	std::string SessionContextImpl::getProperty(const ::std::string& key, const ::Ice::Current& ) const
	{
		::IceUtil::Mutex::Lock lk(*this);
		TianShanIce::Properties::const_iterator it = props.find(key);
		if( it == props.end() )
		{
			return "";
		}
		else
		{
			return it->second;
		}
	}

	void SessionContextImpl::updateProperty(const ::std::string& key, const ::std::string& value, const ::Ice::Current& )
	{	
		::IceUtil::Mutex::Lock lk(*this);
		if(value.empty() )
		{
			props.erase(key);
		}
		else
		{
			props[key] = value;
		}
	}


	void SessionContextImpl::onTimer(const ::Ice::Current& ic)
	{
		::IceUtil::Mutex::Lock lk(*this);

		char szBuf[2048];
		szBuf[sizeof(szBuf) - 1] = '\0';
		IServerRequest* pServerRequest = NULL;
		SmartServerRequest smt(pServerRequest);
		pServerRequest = _env._pSite->newServerRequest(ident.name.c_str());
		if (NULL != pServerRequest)
		{
			std::string hdrStr = "ANNOUNCE " + url + " RTSP/1.0";
			pServerRequest->printCmdLine(hdrStr.c_str());
			pServerRequest->printHeader(HeaderSession, (char*) ident.name.c_str());
			pServerRequest->printHeader(HeaderServer, (char*) _env._serverHeader.c_str());
			snprintf(szBuf, sizeof(szBuf) - 1, "%d", ++announceSeq); // increace announce sequence before send event
			pServerRequest->printHeader(HeaderSequence, szBuf);
			if(3 == requestType) // NGOD spec
			{
				SYS::TimeStamp ts;
				snprintf(szBuf, sizeof(szBuf) - 1, SC_ANNOUNCE_ENDOFSTREAM " " \
					SC_ANNOUNCE_ENDOFSTREAM_STRING " " "event-date=%04d%02d%02dT%02d%02d%02d.%03dZ " "npt=0"\
					, ts.year, ts.month, ts.day, ts.hour, ts.minute, ts.second, ts.millisecond);
				pServerRequest->printHeader(HeaderNotice, szBuf);
				pServerRequest->printHeader(HeaderRequire, "com.comcast.ngod.c1");
			}
			else if (requestType == 1) // SeaChange spec
			{
				SYS::TimeStamp ts;
				snprintf(szBuf, sizeof(szBuf) - 1, SC_ANNOUNCE_ENDOFSTREAM " " \
					SC_ANNOUNCE_ENDOFSTREAM_STRING " " \
					"%04d%02d%02dT%02d%02d%02dZ" \
					" \"Session Timeout\""
					, ts.year, ts.month, ts.day, ts.hour, ts.minute, ts.second);
				pServerRequest->printHeader(HeaderSeaChangeNotice, szBuf);
			}
			else // TianShan spec
			{
				pServerRequest->printHeader(HeaderTianShanNotice, (char*) TS_ANNOUNCE_SESSIONTIMEOUT);
				pServerRequest->printHeader(HeaderTianShanNoticeParam, "");
			}
			pServerRequest->post();
		}

		if(3 == requestType) // the session is as of NGOD spec
		{
			// announce 5402 Client Session Terminated
			IServerRequest* pServerRequest2 = NULL;
			SmartServerRequest smt2(pServerRequest2);
			pServerRequest2 = _env._pSite->newServerRequest(ident.name.c_str());
			if(pServerRequest2)
			{
				std::string hdrStr = "ANNOUNCE " + url + " RTSP/1.0";
				pServerRequest2->printCmdLine(hdrStr.c_str());
				pServerRequest2->printHeader(HeaderSession, (char*) ident.name.c_str());
				pServerRequest2->printHeader(HeaderServer, (char*) _env._serverHeader.c_str());
				SYS::TimeStamp ts;
				snprintf(szBuf, sizeof(szBuf) - 1, "5402" " " \
					"Client Session Terminated" " " "event-date=%04d%02d%02dT%02d%02d%02d.%03dZ " "npt=0"\
					, ts.year, ts.month, ts.day, ts.hour, ts.minute, ts.second, ts.millisecond);
				pServerRequest2->printHeader(HeaderNotice, szBuf);
				pServerRequest2->printHeader(HeaderRequire, "com.comcast.ngod.s1");
				pServerRequest2->post();
			}
		}

		// DO: destroy client session
		if (true == _env._pSite->destroyClientSession(ident.name.c_str()))
			SSMLOG(InfoLevel, CLOGFMT(SessionContextImpl, "RtspProxySession(%s) destroied, reason: ClientSessionTimeout"), ident.name.c_str());
		
		// DO: destroy weiwoo session
		try
		{
			TianShanIce::SRM::SessionPrx srvrSessPrx = NULL;
			_env.getWeiwooSessionPrx(srvrSessPrx, srvrSessPrxID);
			Ice::Context iceCtx;
			iceCtx["caller"] = "212020 Server session timeout from state[stInService]: rtspproxy client session timeout";
			iceCtx["caller_type"] = "rtsp_server_destroy"; // don't modify this, weiwoo will refer to it.
			srvrSessPrx->destroy(iceCtx);
			SSMLOG(InfoLevel, CLOGFMT(SessionContextImpl, "WeiwooSession(%s) destroied, reason: ClientSessionTimeout"), srvrSessPrxID.c_str());
		}
		catch (TianShanIce::BaseException& ex)
		{
			SSMLOG(ErrorLevel, CLOGFMT(SessionContextImpl, "destroyWeiwooSession(%s) caught(%s: %s)")
				, srvrSessPrxID.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (Ice::Exception& ex)
		{
			SSMLOG(ErrorLevel, CLOGFMT(SessionContextImpl, "destroyWeiwooSession(%s) caught(%s)")
				, srvrSessPrxID.c_str(), ex.ice_name().c_str());
		}
		
		// DO: remove session context
		_env.removeSessionCtx(ident, "ClientSessionTimeout");
	}

} // end namespace TianShanS1

