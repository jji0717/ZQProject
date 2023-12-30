#include "ERMIMsgHanler.h"
#include "Ice/Ice.h"
#include "TianShanDefines.h"
#include "strHelper.h"
namespace ZQTianShan {
	namespace EdgeRM {

#define MLOG (_log)
ERMIMsgHanler::ERMIMsgHanler(ZQ::common::Log& log): _log(log)
{
}

ERMIMsgHanler::~ERMIMsgHanler(void)
{
}

bool ERMIMsgHanler::HandleMsg(ZQRtspCommon::IRtspReceiveMsg* receiveMsg, ZQRtspCommon::IRtspSendMsg* sendMsg)
{
	switch(receiveMsg->getVerb())
	{
	case RTSP_MTHD_SETUP:
		{
			return doSetup(receiveMsg, sendMsg);
		}
		break;
	case RTSP_MTHD_TEARDOWN:
		{
			return doTeardown(receiveMsg, sendMsg);
		}
		break;
	case RTSP_MTHD_GET_PARAMETER:
		{
			return GetParameter(receiveMsg, sendMsg);
		}
		break;
	case RTSP_MTHD_SET_PARAMETER:
		{
			return SetParameter(receiveMsg, sendMsg);
		}
		break;
	case RTSP_MTHD_RESPONSE:
		{
			return Response(receiveMsg, sendMsg);
		}
		break;
	default:
		sendMsg->setStartline(ResponseMethodNotAllowed);
		sendMsg->post();
		return true;
	}	
}
void ERMIMsgHanler::onCommunicatorError(ZQ::DataPostHouse::IDataCommunicatorPtr communicator)
{

}

bool ERMIMsgHanler::doSetup(ZQRtspCommon::IRtspReceiveMsg* request, ZQRtspCommon::IRtspSendMsg* response)
{
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ERMIMsgHanler, "CONN["FMT64U"]enter doSetup()"), request->getCommunicator()->getCommunicatorId());
	Ice::Long lStart = ZQTianShan::now();

	try
	{
		std::string strMethod = METHOD_SETUP;
		//handle ERMI S6 message
		std::string strCSeq = request->getHeader(ERMI_HEADER_SEQ);
		std::string onClientSessionId = request->getHeader(ERMI_HEADER_CLABCLIENTSESSIONID);
		std::string strSessionGroup = request->getHeader(ERMI_HEADER_SESSIONGROUP);
		std::string strTransPort = request->getHeader(ERMI_HEADER_TRANSPORT);
		std::string strContent = request->getContent();
		std::string sessId ="";
		generateSessionID(sessId);

		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(ERMIMsgHanler, "CONN["FMT64U"][SETUP][CSeq:%s] clabClientSessionId[%s]sessGroup[%s]TransPort[%s]Content[%s]"),
			request->getCommunicator()->getCommunicatorId(),strCSeq.c_str(), onClientSessionId.c_str(), strSessionGroup.c_str(), strTransPort.c_str(),strContent.c_str());

		response->setHeader(ERMI_HEADER_SEQ, strCSeq.c_str());
		response->setHeader(ERMI_HEADER_CLABCLIENTSESSIONID, onClientSessionId.c_str());
		response->setHeader(ERMI_HEADER_MTHDCODE, METHOD_SETUP);
		response->setHeader(ERMI_HEADER_SESSIONGROUP, strSessionGroup.c_str());
		response->setHeader(ERMI_HEADER_SESSION, sessId.c_str());

		//post response
		response->setStartline(ResponseOK);
		//post back
		response->post();

	}
	catch (...)
	{
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ERMIMsgHanler, "failed to setup session"));
		response->setStartline(ResponseInternalError);
		response->post();
		return false;

	}
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(ERMIMsgHanler, "CONN["FMT64U"]Leave doSetup() took %d ms"),request->getCommunicator()->getCommunicatorId(),ZQTianShan::now() - lStart);

  return true;;
}
bool ERMIMsgHanler::doTeardown(ZQRtspCommon::IRtspReceiveMsg* request, ZQRtspCommon::IRtspSendMsg* response)
{
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ERMIMsgHanler, "CONN["FMT64U"]enter doTeardown()"), request->getCommunicator()->getCommunicatorId());
	Ice::Long lStart = ZQTianShan::now();

	try
	{
		std::string strMethod = METHOD_SETUP;
		//handle ERMI S6 message
		std::string strCSeq = request->getHeader(ERMI_HEADER_SEQ);
		std::string onClientSessionId = request->getHeader(ERMI_HEADER_CLABCLIENTSESSIONID);
		std::string strSessionGroup = request->getHeader(ERMI_HEADER_SESSIONGROUP);
		std::string sessId = request->getHeader(ERMI_HEADER_SESSION);
		std::string reason = request->getHeader(ERMI_HEADER_CLABREASON);

		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(ERMIMsgHanler, "CONN["FMT64U"][TEAR_DOWN][CSeq:%s]session[%s] clabClientSessionId[%s]sessGroup[%s] reason[%s]"),
			request->getCommunicator()->getCommunicatorId(),strCSeq.c_str(), sessId.c_str(), onClientSessionId.c_str(), strSessionGroup.c_str(), reason.c_str());

		response->setHeader(ERMI_HEADER_SEQ, strCSeq.c_str());
		response->setHeader(ERMI_HEADER_CLABCLIENTSESSIONID, onClientSessionId.c_str());
		response->setHeader(ERMI_HEADER_MTHDCODE, METHOD_TEARDOWN);
		response->setHeader(ERMI_HEADER_SESSIONGROUP, strSessionGroup.c_str());
		response->setHeader(ERMI_HEADER_SESSION, sessId.c_str());

		//post response
		response->setStartline(ResponseOK);
		//post back
		response->post();
	}
	catch (...)
	{
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ERMIMsgHanler, "failed to teardwon session"));
		response->setStartline(ResponseInternalError);
		response->post();
		return true;

	}
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(ERMIMsgHanler, "CONN["FMT64U"]Leave doTeardown() took %d ms"), request->getCommunicator()->getCommunicatorId(), ZQTianShan::now() - lStart);

     return true;
}
bool ERMIMsgHanler::GetParameter(ZQRtspCommon::IRtspReceiveMsg* request, ZQRtspCommon::IRtspSendMsg* response)
{
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ERMIMsgHanler, "CONN["FMT64U"]enter GetParameter()"), request->getCommunicator()->getCommunicatorId());

	Ice::Long lStart = ZQTianShan::now();

	std::string  strMethod = METHOD_GETPARAMETER;
	std::string onClientSessionId = "";

	//handle NGOD S6 message
	std::string strCSeq = request->getHeader(ERMI_HEADER_SEQ);
	std::string sessId = request->getHeader(ERMI_HEADER_SESSION);
	std::string strContent = request->getContent();
	std::string require = request->getHeader(NGOD_HEADER_REQUIRE);

	//set session id header
	response->setHeader(ERMI_HEADER_SEQ, strCSeq.c_str());
	response->setHeader(ERMI_HEADER_MTHDCODE, METHOD_GETPARAMETER);


	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(ERMIMsgHanler, "CONN["FMT64U"][GET_PARAMETER][CSeq:%s]session[%s] content[%s]"),
		request->getCommunicator()->getCommunicatorId(),strCSeq.c_str(), sessId.c_str(), strContent.c_str());

	// if the request is session ping
	if(sessId.size() > 0  && strContent.size() == 2 && strContent =="\r\n")
	{
		response->setStartline(ResponseOK);
		response->setHeader(NGOD_HEADER_SESSION, sessId.c_str());
		response->post();
		return true;
	}
	// get parameter: session_list , connection_timeout, servicegroup
	else
	{
		try
		{
			::TianShanIce::StrValues contents;
			::ZQ::common::stringHelper::SplitString(strContent, contents, " \r\n\t", " \r\n\t");

			if(contents.size() <1)
			{
				response->setStartline(ResponseInternalError);
				response->post();
				return true;
			}

			::std::string strContents = "";
			for (int i = 0; i < contents.size(); i++)
			{
				if (contents[i] == "connection_timeout")
				{
					strContents += ::std::string("connection_timeout: 60\r\n");
				}
				else if (contents[i] == "sessionGroups")
				{
					strContents += "\r\n";
				}
				/*else if (contents[i] == "clab-session-list")
				{
					strContents += "\r\n";
				}*/
				else 
				{
					response->setStartline(ResponseBadRequest);
					response->post();
					return true;
				}
			}

			response->setStartline(ResponseOK);
			response->setContent(strContents.c_str());
			response->post();
		}
		catch(...)
		{
			MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(GetParameter, "CONN["FMT64U"]Get Parameter caught unknown exception(%d)"), request->getCommunicator()->getCommunicatorId(), ::GetLastError());
			response->setStartline(ResponseInternalError);
			response->post();
		}
	}

	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(GetParameter, "CONN["FMT64U"]Leave GetParameter() took %d ms"), request->getCommunicator()->getCommunicatorId(), ZQTianShan::now() - lStart);
	return true;
 return true;
}
bool ERMIMsgHanler::SetParameter(ZQRtspCommon::IRtspReceiveMsg* request, ZQRtspCommon::IRtspSendMsg* response)
{
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ERMIMsgHanler, "CONN["FMT64U"]enter SetParameter()"), request->getCommunicator()->getCommunicatorId());

	Ice::Long lStart = ZQTianShan::now();

	::std::string strMethod = METHOD_SETPARAMETER;
	//handle ERMI  message
	::std::string strCSeq = request->getHeader(NGOD_HEADER_SEQ);
	//set session id header
	response->setHeader(ERMI_HEADER_SEQ, strCSeq.c_str());
	response->setHeader(ERMI_HEADER_MTHDCODE, METHOD_SETPARAMETER);
	std::string require = request->getHeader(ERMI_HEADER_REQUIRE);

	try
	{
		std::string strContent = request->getContent();
		TianShanIce::StrValues strSessionGroups;

		ZQ::common::stringHelper::SplitString(strContent, strSessionGroups, ": \t\r\n", " \r\n");

		if(strSessionGroups.size() > 0 && strSessionGroups[0] == "clab-SessionGroup:")
		{
			response->setStartline(ResponseOK);
			response->post();
		}
		else
		{
			response->setStartline(ResponseBadRequest);
			response->post();
		}
	}
	catch(...)
	{
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ERMIMsgHanler, "CONN["FMT64U"]Set Parameter caught unknown exception (%d)"), request->getCommunicator()->getCommunicatorId(),::GetLastError());
		response->setStartline(ResponseInternalError);
		response->post();
		return true;
	}
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(ERMIMsgHanler, "CONN["FMT64U"]Leave SetParameter() took %d ms"),request->getCommunicator()->getCommunicatorId(), ZQTianShan::now() - lStart);
	return true;
}
bool ERMIMsgHanler::Response(ZQRtspCommon::IRtspReceiveMsg* request, ZQRtspCommon::IRtspSendMsg* response)
{
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ERMIMsgHanler, "CONN["FMT64U"]enter Response()"), request->getCommunicator()->getCommunicatorId());

	Ice::Long lStart = ZQTianShan::now();
	::std::string strMethod = "RESPONSE";

	//handle ERMI message
	::std::string onClientSessionId = request->getHeader(ERMI_HEADER_CLABCLIENTSESSIONID);
	::std::string sessId = request->getHeader(ERMI_HEADER_SESSION);
	::std::string strCSeq = request->getHeader(ERMI_HEADER_SEQ);
	::std::string startline = request->getStartline();


	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(ERMIMsgHanler, "CONN["FMT64U"][RESPONSE][CSeq:%s]session[%s] clabClientSessionId[%s]startline[%s]"),
		request->getCommunicator()->getCommunicatorId(),strCSeq.c_str(), sessId.c_str(), onClientSessionId.c_str(), startline.c_str());

	if(sessId.size()< 1 || onClientSessionId.size() <1)
	{
		MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ERMIMsgHanler, "BCONN["FMT64U"][CSeq:%s]session[%s]Bad Response"),
			request->getCommunicator()->getCommunicatorId(),strCSeq.c_str(), sessId.c_str());
		return true;
	}

	::TianShanIce::StrValues responseCodes;
	::ZQ::common::stringHelper::SplitString(startline, responseCodes, " \r\n"); 

	if(responseCodes.size() < 2)
	{
		MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ERMIMsgHanler, "BCONN["FMT64U"][CSeq:%s]session[%s]Bad Response, missing response code or reason"),
			request->getCommunicator()->getCommunicatorId(),strCSeq.c_str(), sessId.c_str());
		return true;
	}

	int32 nRet = atoi(responseCodes[1].c_str());
	if(nRet == 200)
	{
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(ERMIMsgHanler, "BCONN["FMT64U"][CSeq:%s]session[%s]session in progress"),
			request->getCommunicator()->getCommunicatorId(),strCSeq.c_str(), sessId.c_str());
	}
	else if(nRet == 454)
	{
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(ERMIMsgHanler, "BCONN["FMT64U"][CSeq:%s]session[%s]session not exists"),
			request->getCommunicator()->getCommunicatorId(),strCSeq.c_str(), sessId.c_str());
	}

	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(ERMIMsgHanler, "BCONN["FMT64U"][CSeq:%s]session[%s]Leave Response() took %d ms"),
		request->getCommunicator()->getCommunicatorId(), strCSeq.c_str(), sessId.c_str(), ZQTianShan::now() - lStart);

	return true;
  return true;
}
//#define  _RTSP_PROXY
void ERMIMsgHanler::generateSessionID(std::string& sessionID)
{
	char buf[256];

	time_t timet;
	struct tm* pstm;
	uint32 id = 0;
	static uint32 sLastLWord = 0;
	ZQ::common::MutexGuard gd(_genIdCritSec);
	{
		timet = time(0);
		pstm = gmtime(&timet);
		if(pstm)
			id =(uint32)pstm->tm_hour << 27 | 
			(uint32)pstm->tm_min << 21 | 
			(uint32)pstm->tm_sec << 15;

		id = id | (pstm->tm_mday<<10 )| (sLastLWord ++);
		if(sLastLWord > ( 1 << 10 ) )
			sLastLWord = 0;
	}

#ifdef _RTSP_PROXY
	if ( GAPPLICATIONCONFIGURATION.lUseLongSessionId >= 1 )
	{
		sprintf(buf, "9%011u",  id);
	}
	else
#endif
		sprintf(buf, "%u",  id);

	sessionID = buf;
}

	}//namespace EdgeRM
}//namespace ZQTianShan
