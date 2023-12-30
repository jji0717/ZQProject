#include "DescribeRequest.h"
#include "urlstr.h"

namespace TianShanS1
{
	FixupDescribe::FixupDescribe(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse)
		: FixupRequest(env, pSite, pReq, pResponse)
	{
	}

	FixupDescribe::~FixupDescribe()
	{
	}

	bool FixupDescribe::process()
	{
		return true;
	}

	HandleDescribe::HandleDescribe(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse)
		: ContentHandler(env, pSite, pReq, pResponse)
	{
	}

	HandleDescribe::~HandleDescribe()
	{
	}

	bool HandleDescribe::process()
	{
//		RTSP/1.0 200 OK
//		Content-type: application/sdp
//		Server: TianShan RtspProxy 1.0; ssm_tianshan_s1 1.0; ssm_enseo 1.0
//		Content-Length: 194
//		Cseq: 2
//		Cache-Control: no-cache
//
//		v=0
//		o=- 1204301939899802 2 IN IP4 192.168.255.60
//		t=0 0
//		c=IN IP4 0.0.0.0
//		m=video 0 MP2T/DVBC/QAM 0
//		a=control:request_url  _env._config._defaultMap["transport"];

		////////add by lxm at 2008.12.22 to support VLC player/////////
		std::string scVLCAppData = getRequestHeader(HeaderUserAgent);
		////////////////////////////////////////////////////////////////

		std::string sTransType;
		std::string transStr = getRequestHeader(HeaderTransport);
		if (NULL != strstr(transStr.c_str(), "/DVBC/") || NULL != strstr(transStr.c_str(), "/QAM"))
			sTransType = "MP2T/DVBC/QAM";
		else if (NULL != strstr(transStr.c_str(), "AVP/UDP"))
			sTransType = "MP2T/AVP/UDP";
		////////add by lxm at 2008.12.22 to support VLC player/////////
		else if (scVLCAppData.find(VLCFormat) != std::string::npos)
			sTransType = HeaderVLCTransport;
		///////////////////////////////////////////////////////////////
		else // use configured default value
		{
			std::vector<DefaultParamHolder>::iterator it;
			for (it = _tsConfig._defaultParams._paramDatas.begin();
				it != _tsConfig._defaultParams._paramDatas.end(); it ++)
			{
			if ((*it)._name == "Transport")
				sTransType = (*it)._value;
			}
		}

		_pResponse->setHeader(HeaderContentType, (char*) getRequestHeader(HeaderAccept).c_str());
		_pResponse->setHeader(HeaderCacheControl, "no-cache");

	/*
		v=0
		o=- 1170074172300486 1 IN IP4 192.168.43.1
		s=SnapTV Live
		i=LIVE_3sat.de.xmltv.snap.tv
		t=0 0
		a=tool:LIVE555 Streaming Media v2007.01.17
		a=type:broadcast
		a=control:*
		a=source-filter: incl IN IP4 * 192.168.43.1
		a=rtcp-unicast: reflection
		a=range:npt=0-
		a=x-qt-text-nam:SnapTV Live
		a=x-qt-text-inf:LIVE_3sat.de.xmltv.snap.tv
		m=video 1234 udp 33
		c=IN IP4 239.1.1.11/1
		a=control:track1
	*/


		//std::string retContent = "v=0\r\n";
		//unsigned __int64 ntpTime = (unsigned __int64)CNtpTime::GetCurrentTime();
		//_snprintf(_szBuf, sizeof(_szBuf) - 1, "o=- %llu 2 IN IP4 %s\r\n", 
		//	ntpTime, getRequestHeader("SYS#LocalServerIP").c_str());
		//retContent += _szBuf;
		//retContent += "t=0 0\r\n";
		//retContent += "c=IN IP4 0.0.0.0\r\n";
		//retContent += "m=video 0 " + sTransType + " 0\r\n";
		//retContent += "a=control:" + getUrl();
		std::string retContent = std::string("m=video 0 ") + sTransType + " 33\r\nc=IN IP4 0.0.0.0/255\r\na=control:basic";
		_pResponse->printf_postheader(retContent.c_str());
		
		_statusCode = 200;
		composeResponse(_statusCode);
		//composeRightResponse();
		return true;
	}

	DescribeResponse::DescribeResponse(Environment& env, IStreamSmithSite* pSite, IClientRequest* pReq, IServerResponse* pResponse)
		: FixupResponse(env, pSite, pReq, pResponse)
	{
	}

	DescribeResponse::~DescribeResponse()
	{
	}

	bool DescribeResponse::process()
	{
		return true;
	}

} // namespace TianShanS1

