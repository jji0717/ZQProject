#define _WINSOCK2API_

#include "SessionContextImpl.h"
#include "Environment.h"
#include "Authorization.h"

namespace HSNTree {

	SessionContextImpl::SessionContextImpl(Environment& env) : _env(env)
	{
		props[PropKeyLastScaleChange] = "-1";
	}

	SessionContextImpl::~SessionContextImpl()
	{
	}

	::HSNTree::SessionData SessionContextImpl::getSessionData(const ::Ice::Current&)
	{
		::IceUtil::Mutex::Lock lk(*this);

		::HSNTree::SessionData data;

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
				if (requestType == 1) // SeaChange spec
				{
					SYSTEMTIME time;
					GetLocalTime(&time);
					snprintf(szBuf, sizeof(szBuf) - 1, SC_ANNOUNCE_ENDOFSTREAM " " \
						SC_ANNOUNCE_ENDOFSTREAM_STRING " " \
						"%04d%02d%02dT%02d%02d%02dZ" \
						" \"Session Timeout\""
						, time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
					pServerRequest->printHeader(HeaderSeaChangeNotice, szBuf);
				}
				else // TianShan spec
				{
					pServerRequest->printHeader(HeaderTianShanNotice, (char*) TS_ANNOUNCE_SESSIONTIMEOUT);
					pServerRequest->printHeader(HeaderTianShanNoticeParam, "");
				}
				pServerRequest->post();
			}

			// DO: destroy client session
			if (true == _env._pSite->destroyClientSession(ident.name.c_str()))
				glog(InfoLevel, CLOGFMT(SessionContextImpl, "RtspProxySession(%s) destroied, reason: ClientSessionTimeout"), ident.name.c_str());
		}
		
		// DO: destroy weiwoo session
		try
		{
			TianShanIce::SRM::SessionPrx srvrSessPrx = NULL;
			_env.getWeiwooSessionPrx(srvrSessPrx, srvrSessPrxID);
			Ice::Context iceCtx;
			iceCtx["caller"] = "212020 Server session timeout from state[stInService]: rtspproxy client session timeout";
			iceCtx["caller_type"] = "rtsp_server_destroy"; // don't modify this, weiwoo will refer to it.
			srvrSessPrx->destroy(iceCtx);
			glog(InfoLevel, CLOGFMT(SessionContextImpl, "WeiwooSession(%s) destroied, reason: ClientSessionTimeout"), srvrSessPrxID.c_str());
		}
		catch (TianShanIce::BaseException& ex)
		{
			glog(ErrorLevel, CLOGFMT(SessionContextImpl, "destroyWeiwooSession(%s) caught(%s: %s)")
				, srvrSessPrxID.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			updateProperty(SYS_PROP(terminateReason), "220030 session timeout");
		}
		catch (Ice::Exception& ex)
		{
			glog(ErrorLevel, CLOGFMT(SessionContextImpl, "destroyWeiwooSession(%s) caught(%s)")
				, srvrSessPrxID.c_str(), ex.ice_name().c_str());
			updateProperty(SYS_PROP(terminateReason), "220030 session timeout");
		}
		
		// authorization
		sessionTeardown();

		// DO: remove session context
		_env.removeSessionCtx(ident, "ClientSessionTimeout");
	}

	::TianShanIce::SRM::SessionPrx SessionContextImpl::getSession(const ::Ice::Current&) const
	{
		glog(ZQ::common::Log::L_NOTICE, "PurchaseImpl::getSession()");
		return NULL;
	}

	void SessionContextImpl::provision(const ::Ice::Current&)
	{
		glog(ZQ::common::Log::L_NOTICE, "PurchaseImpl::provision()");
	}

	void SessionContextImpl::render(const ::TianShanIce::Streamer::StreamPrx& sPrx, const ::TianShanIce::SRM::SessionPrx& sessionPrx, const ::Ice::Current&)
	{
		glog(ZQ::common::Log::L_NOTICE, "PurchaseImpl::render()");
	}

	void SessionContextImpl::detach(const ::std::string& sessId, const ::TianShanIce::Properties& params, const ::Ice::Current&c)
	{
		::IceUtil::Mutex::Lock lk(*this);
		glog(ZQ::common::Log::L_NOTICE, "PurchaseImpl::detach()");
		::TianShanIce::Properties::const_iterator iter = params.begin();
		for (; iter != params.end(); iter++)
		{
			MAPSET(::TianShanIce::Properties, props, iter->first, iter->second);
		}
	}

	void SessionContextImpl::bookmark(const ::std::string& title, const ::TianShanIce::SRM::SessionPrx& sess, const ::Ice::Current& c)
	{
		glog(ZQ::common::Log::L_NOTICE, "PurchaseImpl::bookmark()");
	}

	::Ice::Int SessionContextImpl::getParameters(const ::TianShanIce::StrValues& params, const ::TianShanIce::ValueMap& vMap_in, ::TianShanIce::ValueMap& vMap_out, const ::Ice::Current& c) const
	{
		glog(ZQ::common::Log::L_NOTICE, "PurchaseImpl::getParameters()");
		return 1;
	}

	void SessionContextImpl::sessionTeardown(const ::Ice::Current& c)
	{
		::IceUtil::Mutex::Lock lk(*this);

		NAMESPACE(SessionData) sd;
		sd.serverSessionId = srvrSessID;
		sd.clientSessionId = ident.name;

		SYSTEMTIME st;
		GetLocalTime(&st);
		char strTime[48];
		sprintf(strTime, "%d-%02d-%02d %02d:%02d:%02d", st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
		sd.params["ViewEndTime"] = strTime;
		std::string	strTeardownReason ;
		std::string strTerminateReason;
		::TianShanIce::Properties::const_iterator itProp;
		itProp = props.find( SYS_PROP(teardownReason) );
		if (itProp != props.end())
		{
			strTeardownReason = itProp->second;
		}
		itProp = props.find (SYS_PROP(terminateReason));
		if (itProp != props.end())
		{
			strTerminateReason = itProp->second;
		}
		itProp = props.find ("streamerNetId");
		if (itProp != props.end())
		{
			sd.params["streamerNetId"] = itProp->second;
		}
		sd.params["teardownReason"] = strTeardownReason;
		sd.params["terminateReason"] = strTerminateReason;
		sd.params["Reason"] = (!strTeardownReason.empty() ? strTeardownReason : strTerminateReason);

		Authorization::sessionTeardown(_env._pCommunicator ,sd);
	}

} // end namespace HSNTree

