// ===========================================================================
// Copyright (c) 2004 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Ident : $Id: NGODSession.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/StreamService/NSS/NGODSession.cpp $
// 
// 66    11/22/16 6:21p Hui.shao
// 
// 70    11/22/16 10:56a Hui.shao
// 
// 69    11/21/16 4:42p Hongquan.zhang
// 
// 68    11/21/16 12:02p Hui.shao
// 
// 67    8/02/16 1:26p Hui.shao
// 
// 66    8/02/16 10:17a Hui.shao
// Cisco only has npt=nptPrimary
// 
// 65    7/05/16 2:29p Hui.shao
// ticket#19405 Ads integration with Cisco VSS
// 
// 64    6/21/16 5:13p Hui.shao
// 
// 63    11/25/15 3:30p Hui.shao
// case-insensitve on configuration vender and customers
// 
// 62    10/26/15 12:09p Hui.shao
// 
// 61    10/22/15 2:51p Hui.shao
// ItemStepped parameter mapping
// 
// 60    10/21/15 4:24p Hui.shao
// 
// 59    10/21/15 11:22a Hui.shao
// 
// 58    10/21/15 11:06a Hui.shao
// 
// 57    10/19/15 4:47p Hui.shao
// convert Trasition ANNOUNCE to ItemStepped
// 
// 56    5/12/15 11:27a Hui.shao
// set a max penalty to start with
// 
// 55    5/06/15 4:54p Hui.shao
// tested drop request due to penalties
// ===========================================================================

#ifndef    LOGFMTWITHTID
#  define  LOGFMTWITHTID
#endif 

#include "NGODSession.h"
#include "NSSEnv.h"
#include "strHelper.h"
#include "SsServiceImpl.h"
#include "NSSUtil.h"
#include "urlstr.h"
#include "NSSCfgLoader.h"

#include "TianShanIceHelper.h"

extern ZQ::common::NativeThreadPool* gPool;
extern ZQTianShan::NSS::NSSBaseConfig::NSSHolder *pNSSBaseConfig;

namespace ZQ {
namespace StreamService {

extern SsServiceImpl* pServiceInstance;

}}

#define NPT_NOW (-24567)

namespace ZQTianShan {
namespace NGODSS {

using namespace ZQ::common;

// -----------------------------
// class NGODSession
// -----------------------------
NGODSession::NGODSession(NGODSessionGroup& group, const std::string& ODSessId, const std::string& streamDestUrl, const char* baseURL)
: RTSPSession( group._env.getMainLogger(), group._env.getMainThreadPool(), streamDestUrl.c_str(), NULL, group._env._rtspTraceLevel, group._env._sessTimeout, ODSessId.c_str()),
  _group(group)
{
	_stampLastMessage = ZQ::common::now(); // this is not needed if have ZQCommon.dll > v2.0.4.12 or v1.16.4.18
	_group.add(*this);
	_log(Log::L_DEBUG, CLOGFMT(NGODSession, "ODSess[%s] instantized in SessionGroup[%s]"), _sessGuid.c_str(), getSessionGroupName().c_str());
}

std::string	NGODSession::getSessionGroupName() const
{ return _group.getName(); }


NGODSession::~NGODSession()
{
	destroy();
}

void NGODSession::destroy()
{
	_group.remove(*this);
	RTSPSession::destroy();
	_log(Log::L_INFO, CLOGFMT(NGODSession, "Session[%s, %s] of group[%s] destroyed"), _sessGuid.c_str(), _sessionId.c_str(), getSessionGroupName().c_str());
}

std::string NGODSession::getBaseURL()
{
	return _group.getBaseURL();
}

// overwrite OnResponse() to signal the event
void NGODSession::OnResponse(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString)
{
	int64 stampStart = ZQ::common::now();

	// wildcast to check "551 Option Not Supported" about ECR: Protocol Versioning for RTSP protocols
	NGODClient* pClient = (NGODClient*) &rtspClient;
	if (resultCode == RTSPSink::rcOptionNotSupport)
	{
		if (pResp->headers.end() != pResp->headers.find("Unsupported"))
		{
			std::string& valOfResponse = pResp->headers["Unsupported"];
			if (std::string::npos != valOfResponse.find("decimal_npts"))
				pClient->_bDecimalNpt =false;
		}
	}

	if (!pClient->_bNptHandshaked)
		pClient->_bNptHandshaked = true;
	
	// enh#20635 pass FinalNPT to MOD
	_props.erase(SYS_PROP(FinalNPT));
	if (pReq->_commandName == "TEARDOWN" && pResp->headers.end() != pResp->headers.find("FinalNPT"))
		MAPSET(TianShanIce::Properties, _props, SYS_PROP(FinalNPT), pResp->headers["FinalNPT"]); // TODO: how to pass it back to MOD?

	RTSPSession::OnResponse(rtspClient, pReq, pResp, resultCode, resultString); // do the dispatching
	if (pReq)
		((NGODClient*) &rtspClient)->wakeupByCSeq(pReq->cSeq);

	_log(Log::L_INFO, CLOGFMT(NGODSession, "Session[%s, %d, %s] OnResponse() dispatched, latency[%d]msec: %d %s"), _sessGuid.c_str(), pResp->cSeq, getSessionGroupName().c_str(), (int)(ZQ::common::now() - stampStart), resultCode, resultString);
}

void NGODSession::OnRequestError(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RequestError errCode, const char* errDesc)
{
	RTSPSession::OnRequestError(rtspClient, pReq, errCode, errDesc);

	if (!pReq)
		return;

	switch(errCode)
	{
	case Err_InvalidParams:
		_resultCode = rcBadParameter;
		break;

	case Err_RequestTimeout:
		_resultCode = rcRequestTimeout;
		break;

	case Err_ConnectionLost:
	case Err_SendFailed:
	default:
		_resultCode = rcServiceUnavail;
	}

	((NGODClient*) &rtspClient)->wakeupByCSeq(pReq->cSeq, false);
}

void NGODSession::OnResponse_SETUP(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString)
{
	RTSPSession::OnResponse_SETUP(rtspClient, pReq, pResp, resultCode, resultString);
	_log(Log::L_DEBUG, CLOGFMT(NGODSession, "Session[%s, %s, %d, %s] OnResponse_SETUP(), return [%d %s]"), _sessGuid.c_str(), _sessionId.c_str(), pResp->cSeq, getSessionGroupName().c_str(), resultCode, resultString);
	_resultCode = resultCode;
	_stampSetup = now();
	if (pResp->headers.find("TianShan-NoticeParam") != pResp->headers.end())
	{
		std::string noticeParam = pResp->headers["TianShan-NoticeParam"];
		if(noticeParam.find("primaryItemNPT") != std::string::npos)
			_primartItemNPT = noticeParam.substr(noticeParam.find('=')+1, noticeParam.size());
	}

	if (resultCode != RTSPSink::rcOK)
		return;

	_group.updateIndex(*this);

	//		updateIndex(this);
	std::vector<std::string> temp;
	ZQ::common::stringHelper::SplitString(pResp->contentBody, temp, "\r\n", "\r\n");
	for (size_t i = 0; i < temp.size(); i++)
	{
		if (temp[i][0] == 'a')
		{
			std::string::size_type pos_begin	= std::string::npos;
			if ((pos_begin=temp[i].find(":"))!= std::string::npos)				
			{
				::std::string strKey   = temp[i].substr(2, pos_begin-2);
				if (strKey.compare("control")==0)
				{
					_controlUri = temp[i].substr(pos_begin+1);
					_log(Log::L_DEBUG, CLOGFMT(NGODSession, "Session[%s] OnResponse_SETUP(), got controlURI[%s]"), _sessGuid.c_str(), _controlUri.c_str());
				}
			}
			break;
		}
	}
}	

void NGODSession::OnResponse_PLAY(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString)
{
	_log(Log::L_DEBUG, CLOGFMT(NGODSession, "Session[%s, %d, %s] OnResponse_PLAY(), return [%d %s]"), _sessGuid.c_str(), pResp->cSeq, getSessionGroupName().c_str(), resultCode, resultString);
	_resultCode = resultCode;
	parseStreamInfo(pResp);
	if(resultCode == RTSPSink::rcOK)
	{
	}	
	else if(resultCode == RTSPSink::rcSessNotFound)
	{
		destroy();
	}
	else
	{
    }
}

void NGODSession::OnResponse_TEARDOWN(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString)
{
	_log(Log::L_DEBUG, CLOGFMT(NGODSession, "Session[%s, %d, %s] OnResponse_TEARDOWN(), return [%d %s]"), _sessGuid.c_str(), pResp->cSeq, getSessionGroupName().c_str(), resultCode, resultString);
	_resultCode = resultCode;
	_sessionHistory = pResp->contentBody;
	if (resultCode == RTSPSink::rcSessNotFound)
		destroy();
}

void NGODSession::OnResponse_PAUSE(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString)
{
	_log(Log::L_DEBUG, CLOGFMT(NGODSession, "Session[%s, %d, %s] OnResponse_PAUSE(), return [%d %s]"), _sessGuid.c_str(), pResp->cSeq, getSessionGroupName().c_str(), resultCode, resultString);
	_resultCode = resultCode;
	parseStreamInfo(pResp);
	if (resultCode == RTSPSink::rcSessNotFound)
		destroy();
}

void NGODSession::OnResponse_GET_PARAMETER(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString)
{
	_log(Log::L_DEBUG, CLOGFMT(NGODSession, "Session[%s, %d, %s] OnResponse_GET_PARAMETER(), return [%d %s]"), _sessGuid.c_str(), pResp->cSeq, getSessionGroupName().c_str(), resultCode, resultString);
	_resultCode = resultCode;
	if(pResp->contentBody.empty())
	{
		_streamInfos.scale = 0.0f;
		_streamInfos.timeoffset = 0;
		_streamInfos.state = "init";
	}
	else
	{
		std::vector<std::string> temp;
		ZQ::common::stringHelper::SplitString(pResp->contentBody, temp, "\r\n", "\r\n");
		for (size_t i = 0; i < temp.size(); i++)
		{
			std::string::size_type pos_begin	= std::string::npos;
			if((pos_begin=temp[i].find(":"))!= std::string::npos)				
			{
				::std::string strKey   = temp[i].substr(0, pos_begin);
				::std::string strValue = temp[i].substr(pos_begin+1, temp[i].size());
				_log(Log::L_DEBUG, CLOGFMT(NGODSession, "Session[%s] OnResponse_GET_PARAMETER(), get content[%s]=[%s]"), _sessGuid.c_str(), strKey.c_str(), strValue.c_str());
				if(strKey.compare("scale")==0)
				{
					_streamInfos.scale = (float) atof(strValue.c_str());
				}
				else if (strKey.compare("position")==0)
				{
					_streamInfos.timeoffset = (int64) (1000.0f * atof(strValue.c_str()));
				}
				else if (strKey.compare("presentation_state")==0)
				{
					_streamInfos.state = strValue;
				}
			}		
		}
	}

	if (resultCode == RTSPSink::rcSessNotFound)
	{
		destroy();
    }
}

void NGODSession::OnResponse_SET_PARAMETER(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString)
{
	_log(Log::L_DEBUG, CLOGFMT(NGODSession, "Session[%s, %d, %s] OnResponse_SET_PARAMETER(), return [%d %s]"), _sessGuid.c_str(), pResp->cSeq, getSessionGroupName().c_str(), resultCode, resultString);
	_resultCode = resultCode;
	if (resultCode == RTSPSink::rcSessNotFound)
		destroy();
}

void NGODSession::OnANNOUNCE(RTSPClient& rtspClient, RTSPMessage::Ptr& pInMessage)
{
	_log(Log::L_DEBUG, CLOGFMT(NGODSession, "Session[%s, %s] OnANNOUNCE()"), _sessGuid.c_str(), getSessionGroupName().c_str());
	if (pInMessage->headers.end() == pInMessage->headers.find("Session"))
		return;

	std::string rtspSessId = pInMessage->headers["Session"];
	int cseq =0;
	if (pInMessage->headers.find("CSeq") != pInMessage->headers.end())
		cseq = atoi(pInMessage->headers["CSeq"].c_str());

	int64 stampStart = ZQ::common::now();
	size_t i =0;
	int AnnounceCode = -1;
	TianShanIce::Properties props;
	ZQ::StreamService::StreamParams paras;
	char buf[32];

	// parsing header Notice
	std::string notice = "";
	std::vector<std::string> paramTokens;
	TianShanIce::Properties params;
	std::string serverSession = _props["ServerSession"];

	if (pInMessage->headers.find("Notice") != pInMessage->headers.end())
	{
		notice = pInMessage->headers["Notice"];
		if (0 == pNSSBaseConfig->_videoServer.customer.compare("henan")) 
		{
			MAPSET(TianShanIce::Properties, props, "user.NGOD-Notice", notice); // ticket#16629, enh#19927
			_log(Log::L_DEBUG, CLOGFMT(NGODSession, "OnANNOUNCE(), Session[%s] cseq(%d) added props[user.NGOD-Notice] per customer[henan]"), _sessGuid.c_str(), pInMessage->cSeq);
		}
	}

	_log(Log::L_DEBUG, CLOGFMT(NGODSession, "OnANNOUNCE(), Session[%s] cseq(%d) Notice[%s]"), _sessGuid.c_str(), pInMessage->cSeq, notice.c_str());
	ZQ::common::stringHelper::SplitString(notice, paramTokens, " ", " ");
	if (paramTokens.size() >0)
		AnnounceCode = atoi(paramTokens[0].c_str());

	for (i = 1; i < paramTokens.size(); i++)
	{
		std::string::size_type pos = paramTokens[i].find("=");
		if (std::string::npos == pos)
			continue;

		std::string key = paramTokens[i].substr(0, pos);
		std::string val = paramTokens[i].substr(pos+1);

		if (key.empty())
			continue;

		if (val.length() >=2 && '"' == val[0] && '"' == val[val.length()-1])
			val = val.substr(1, val.length()-2);

		MAPSET(TianShanIce::Properties, params, key, val);
	}

	// parsing header TianShan-NoticeParam
	std::string tianShanNoticeParam;
	if (pInMessage->headers.find("TianShan-NoticeParam") != pInMessage->headers.end())
		tianShanNoticeParam = pInMessage->headers["TianShan-NoticeParam"];

	paramTokens.clear();
	ZQ::common::stringHelper::SplitString(tianShanNoticeParam, paramTokens, ";", ";");
	for (i = 0; i < paramTokens.size(); i++)
	{
		std::string::size_type pos = paramTokens[i].find("=");
		if (std::string::npos == pos)
			continue;

		std::string key = paramTokens[i].substr(0, pos);
		std::string val = paramTokens[i].substr(pos+1);

		if (key.empty())
			continue;

		if (val.length() >=2 && '"' == val[0] && '"' == val[val.length()-1])
			val = val.substr(1, val.length()-2);

		MAPSET(TianShanIce::Properties, params, key, val);
	}

	// parsing header AD-NoticeParam, ticket#19405 Ads integration with Cisco VSS
	if(pInMessage->headers.find("AD-NoticeParam") != pInMessage->headers.end())
	{
		MAPSET(TianShanIce::Properties, params, "nptBy", "1");
		if (std::string::npos != pNSSBaseConfig->_videoServer.vendor.find("cisco")) 
		{
			// AD-NoticeParam: npt=900.000;scale=1;ItemPAID=90000::GMOV0120160616200100;ADItemDuration=32000;TotalADItemDuration=64000;ADStatus=Middle;EcPAID=10033::GMOV0120160620
			std::string strAdItemDur;
			std::string adNoticeParam = pInMessage->headers["AD-NoticeParam"];

			paramTokens.clear();
			ZQ::common::stringHelper::SplitString(adNoticeParam, paramTokens, ";", ";");
			bool isCurrentAds = false;
			for (i = 0; i < paramTokens.size(); i++)
			{
				std::string::size_type pos = paramTokens[i].find("=");
				if (std::string::npos == pos)
					continue;

				std::string key = paramTokens[i].substr(0, pos);
				std::string val = paramTokens[i].substr(pos+1);

				if (key.empty())
					continue;

				if (val.length() >=2 && '"' == val[0] && '"' == val[val.length()-1])
					val = val.substr(1, val.length()-2);

				if (key == "ItemPAID")
				{ 
					MAPSET(TianShanIce::Properties, params, "currentItem", val);
					size_t pos = val.find("::");
					if (std::string::npos == pos)
						MAPSET(TianShanIce::Properties, params, "currentPAID", val);
					else
					{
						MAPSET(TianShanIce::Properties, params, "currentPAID", val.substr(pos+2));
						MAPSET(TianShanIce::Properties, params, "currentPID",  val.substr(0, pos));
					}

					continue;
				}

				if (key == "EcPAID")
				{ 
					MAPSET(TianShanIce::Properties, params, "prevItem", val);
					size_t pos = val.find("::");
					if (std::string::npos == pos)
						MAPSET(TianShanIce::Properties, params, "prevPAID", val);
					else
					{
						MAPSET(TianShanIce::Properties, params, "prevPAID", val.substr(pos+2));
						MAPSET(TianShanIce::Properties, params, "prevPID",  val.substr(0, pos));
					}

					continue;
				}

				if (key== "npt" && !val.empty()) // Cisco take npt as nptPrimary
				{
					MAPSET(TianShanIce::Properties, params, "nptPrimary", val);
					// NO continue statement here
				}

				if (key== "ADItemDuration")
					strAdItemDur = val;
				if (key== "ADStatus" && !val.empty())
					isCurrentAds = true;

				MAPSET(TianShanIce::Properties, params, key, val);
			}

			if (isCurrentAds && !strAdItemDur.empty())
				MAPSET(TianShanIce::Properties, params, "currentDur", strAdItemDur);
		}
	}

	// parsing header StreamStatus per NGOD-I03, ticket#12446
	std::string streamStatus;
	if (pInMessage->headers.find("StreamStatus") != pInMessage->headers.end())
	{
		streamStatus = pInMessage->headers["StreamStatus"];
		if (!streamStatus.empty() && 0 == pNSSBaseConfig->_videoServer.customer.compare("henan"))
		{
			MAPSET(TianShanIce::Properties, props, "user.NGOD-StreamStatus", streamStatus); // ticket#16629, enh#19927
			_log(Log::L_DEBUG, CLOGFMT(NGODSession, "OnANNOUNCE() sess[%s] cseq(%d) added props[user.NGOD-StreamStatus] per customer[henan]"), _sessGuid.c_str(), pInMessage->cSeq);
		}
	}

	paramTokens.clear();
	ZQ::common::stringHelper::SplitString(streamStatus, paramTokens, " ", " ");
	for (i = 0; i < paramTokens.size(); i++)
	{
		std::string::size_type pos = paramTokens[i].find("=");
		if (std::string::npos == pos)
			continue;

		std::string key = paramTokens[i].substr(0, pos);
		std::string val = paramTokens[i].substr(pos+1);

		if (key.empty())
			continue;

		if (val.length() >=2 && '"' == val[0] && '"' == val[val.length()-1])
			val = val.substr(1, val.length()-2);

		MAPSET(TianShanIce::Properties, params, key, val);
	}

	// process the parameters collected
	std::string paramPrintStr;
	TianShanIce::Properties newParams;

	for (TianShanIce::Properties::iterator itParam = params.begin(); itParam != params.end(); itParam++)
	{
		paramPrintStr += itParam->first + "=" + itParam->second + "; ";
		MAPSET(TianShanIce::Properties, newParams, std::string("sys.") + itParam->first, itParam->second);

		if (0 == itParam->first.compare("npt"))
		{
			paras.mask |= MASK_TIMEOFFSET;
			int64 ret	= 0;
			// sscanf( strValue.c_str(),"%lx", &ret );
			// paras.timeoffset = ret * 1000; // convert sec to msec
			paras.timeoffset = NGODClient::parseNptValue(itParam->second.c_str());
			if (paras.timeoffset >=0)
			{
				snprintf(buf, sizeof(buf)-2, "%lld", paras.timeoffset);
				MAPSET(TianShanIce::Properties, props, "sys.npt", buf);
			}
			else if (NPT_NOW == paras.timeoffset)
				MAPSET(TianShanIce::Properties, props, "sys.npt", "now");

			continue;
		}

		if (0 == itParam->first.compare("presentation_state"))
		{
			paras.mask |= MASK_STATE;
			paras.streamState = convertState(itParam->second);
			continue;
		}

		if (0 == itParam->first.compare("scale"))
		{
			paras.mask |= MASK_SCALE;
			paras.scale = (float) atof(itParam->second.c_str());
			continue;
		}
	}

	params = newParams;
	
	_log(Log::L_DEBUG, CLOGFMT(NGODSession, "Session[%s] OnANNOUNCE(), params: %s"), _sessGuid.c_str(), paramPrintStr.c_str());

	paramPrintStr = "";
	for (TianShanIce::Properties::iterator it = props.begin(); it != props.end(); it++)
		paramPrintStr += it->first + "[" + it->second + "] ";

	_log(Log::L_DEBUG, CLOGFMT(NGODSession, "Session[%s] OnANNOUNCE(%d), flags[%x] pos[%lld] dur[%lld] scale[%.2f] sstate[%d] cstate[%d] %s"), _sessGuid.c_str(), AnnounceCode, paras.mask, paras.timeoffset, paras.duration, paras.scale, paras.streamState, paras.contentState, paramPrintStr.c_str());

	std::string          respStartLine;
	RTSPMessage::AttrMap respHeaders;
	MAPSET(RTSPMessage::AttrMap, respHeaders, "OnDemandSessionId", getOnDemandSessionId());
	MAPSET(RTSPMessage::AttrMap, respHeaders, "Session", rtspSessId);

	if (0 == pNSSBaseConfig->_videoServer.customer.compare("henan"))
	{
		respStartLine = "RTSP/1.0 200 OK";
		_log(Log::L_DEBUG, CLOGFMT(NGODSession, "OnANNOUNCE(), Session[%s] cseq(%d) enabled resp per customer[henan]"), _sessGuid.c_str(), pInMessage->cSeq);
	}

	switch (AnnounceCode)
	{
	case RTSPSink::racStateChanged: // call lib OnEvent
		// ticket#16685
		if (0 == pNSSBaseConfig->_videoServer.customer.compare("henan") && std::string::npos != pNSSBaseConfig->_videoServer.vendor.find("moto"))
		{
			MAPSET(TianShanIce::Properties, props, "user.LSCP-RespCode", "40");
			_log(Log::L_DEBUG, CLOGFMT(NGODSession, "OnANNOUNCE() Session[%s] cseq(%d) included props[user.LSCP-RespCode] per vendor[Moto] customer[henan]"), _sessGuid.c_str(), pInMessage->cSeq);
		}

		ZQ::StreamService::pServiceInstance->OnStreamEvent( ZQ::StreamService::SsServiceImpl::seStateChanged, _sessGuid, paras, props );
		break;

	case RTSPSink::racScaleChanged:
		// ticket#16685
		if (0 == pNSSBaseConfig->_videoServer.customer.compare("henan") && std::string::npos != pNSSBaseConfig->_videoServer.vendor.find("cisco"))
		{
			MAPSET(TianShanIce::Properties, props, "user.LSCP-RespCode", "40");
			_log(Log::L_DEBUG, CLOGFMT(NGODSession, "OnANNOUNCE() Session[%s] cseq(%d) included props[user.LSCP-RespCode] per vendor[Cisco] customer[henan]"), _sessGuid.c_str(), pInMessage->cSeq);
		}

		if (0 == pNSSBaseConfig->_videoServer.vendor.compare("cisco")) // exactly match vendor=Cisco per ticket#11287 and the later enh#17798 here
		{
			// ticket#11287, stupid integration by a pm who led an idle life
			paras.mask &= ~MASK_TIMEOFFSET;
			if (NPT_NOW == paras.timeoffset)
			{
				MAPSET(TianShanIce::Properties, props, "sys.includeNpt", "LatestKnown");
				MAPSET(TianShanIce::Properties, props, "sys.goneMode", "T");
			}

			_log(Log::L_WARNING, CLOGFMT(NGODSession, "Session[%s] substituted %d Scale-Changed to EOS per vendor[%s], npt[%lld]"), _sessGuid.c_str(), AnnounceCode, pNSSBaseConfig->_videoServer.vendor.c_str(), paras.timeoffset);
			ZQ::StreamService::pServiceInstance->OnStreamEvent(ZQ::StreamService::SsServiceImpl::seGone, _sessGuid, paras, props);
			break;
		}
		else if (0 == pNSSBaseConfig->_videoServer.vendor.compare("cisco.regular"))
		{
			// ticket#13151: convert ScaleChanged(12) to ScaleChanged(0) when Cisco.regular && scale>2 && NPT_NOW == paras.timeoffset
			if (paras.scale >2 && NPT_NOW == paras.timeoffset)
				paras.scale =0.0;
		}
				
		ZQ::StreamService::pServiceInstance->OnStreamEvent( ZQ::StreamService::SsServiceImpl::seScaleChanged, _sessGuid, paras, props );
		break;

	case RTSPSink::racEndOfStream:
		ZQ::StreamService::pServiceInstance->OnStreamEvent( ZQ::StreamService::SsServiceImpl::seGone, _sessGuid, paras, props );
		break;

	case RTSPSink::racBeginOfStream:
		ZQ::StreamService::pServiceInstance->OnStreamEvent( ZQ::StreamService::SsServiceImpl::seGone, _sessGuid, paras, props );
		break;

	case RTSPSink::racSessionInProgress:
		respStartLine = "RTSP/1.0 200 OK";
		break;

	case RTSPSink::racTransition:
	case RTSPSink::racFakedItemStepped:
		{
			ZQ::StreamService::EventData ed;
			ed.eventType       = ZQ::StreamService::ICE_EVENT_TYPE_ITEMSETPPED;
			ed.eventProp       = params;
			ed.eventSeq        = -1;

			ed.playlistId.name     = getOnDemandSessionId();
			ed.playlistId.category = ZQ::StreamService::EVICTOR_NAME_STREAM;
			// ed.playlistProxyString  = ???

			///////////////////////////////////////
			// initialize with some dummy values for those MUST parameters
			char strTemp[100];
			ZQTianShan::Util::updateValueMapData(ed.eventData, ZQ::StreamService::EVENT_ITEMSTEP_PREV_ITEM_NAME,    "na");
			ZQTianShan::Util::updateValueMapData(ed.eventData, ZQ::StreamService::EVENT_ITEMSTEP_NEXT_ITEM_NAME,    "na");
			ZQTianShan::Util::updateValueMapData(ed.eventData, ZQ::StreamService::EVENT_ITEMSTEP_ITEM_PREV_CTRLNUM, (Ice::Int) -1);
			ZQTianShan::Util::updateValueMapData(ed.eventData, ZQ::StreamService::EVENT_ITEMSTEP_ITEM_CUR_CTRLNUM,  (Ice::Int) -1);
			ZQTianShan::Util::updateValueMapData(ed.eventData, ZQ::StreamService::EVENT_ITEMSTEP_TIME_STAMP_UTC,    ZQTianShan::TimeToUTC(ZQ::common::now(), strTemp, sizeof(strTemp)-2));

			ZQTianShan::Util::updateValueMapData(ed.eventData, ZQ::StreamService::EVENT_ITEMSTEP_PREV_ITEM_PID,  "na");
			ZQTianShan::Util::updateValueMapData(ed.eventData, ZQ::StreamService::EVENT_ITEMSTEP_PREV_ITEM_PAID, "na");
			ZQTianShan::Util::updateValueMapData(ed.eventData, ZQ::StreamService::EVENT_ITEMSTEP_PREV_STREAMSOURCE,  "na");
			ZQTianShan::Util::updateValueMapData(ed.eventData, ZQ::StreamService::EVENT_ITEMSTEP_CUR_ITEM_PID,  "na");
			ZQTianShan::Util::updateValueMapData(ed.eventData, ZQ::StreamService::EVENT_ITEMSTEP_CUR_ITEM_PAID, "na");
			ZQTianShan::Util::updateValueMapData(ed.eventData, ZQ::StreamService::EVENT_ITEMSTEP_CUR_STREAMSOURCE, "na");

			ZQTianShan::Util::updateValueMapData(ed.eventData, ZQ::StreamService::EVENT_ITEMSTEP_ITEM_CUR_TIMEOFFSET, (Ice::Int) 0);
			ZQTianShan::Util::updateValueMapData(ed.eventData, ZQ::StreamService::EVENT_ITEMSTEP_EVENT_SEQ,           (Ice::Int) 0);
			ZQTianShan::Util::updateValueMapData(ed.eventData, ZQ::StreamService::EVENT_ITEMSTEP_CLSUTERID,           (Ice::Int) 0);

			ZQTianShan::Util::updateValueMapData(ed.eventData, ZQ::StreamService::EVENT_ITEMSTEP_CUR_ITEM_FLAGS,      (Ice::Int) 0);
			ZQTianShan::Util::updateValueMapData(ed.eventData, ZQ::StreamService::EVENT_ITEMSTEP_PREV_ITEM_FLAGS,     (Ice::Int) 0);
						
			ZQTianShan::Util::updatePropertyData(ed.eventProp, SYS_PROP(ServerSession),  serverSession);

			// scan params, see if there is any value needs to adjust ed.eventData
			// refer to IceEventSender::translateItemStepProperty()
			for (TianShanIce::Properties::iterator itP = params.begin(); itP != params.end(); itP++)
			{
				if (itP->first == ZQ::StreamService::EVENT_ITEMSTEP_EVENT_SEQ)
				{
					ed.eventSeq = atol(itP->second.c_str()); ed.eventSeq++;
					ZQTianShan::Util::updateValueMapData(ed.eventData, ZQ::StreamService::EVENT_ITEMSTEP_EVENT_SEQ, ed.eventSeq);
					continue;
				}

#define COPY_STR_PARAM_TO_VMAP(_PropKEY, _DKEY)  if (itP->first == #_PropKEY) { ZQTianShan::Util::updateValueMapData(ed.eventData, ZQ::StreamService::_DKEY, itP->second); continue; }
#define COPY_INT_PARAM_TO_VMAP(_PropKEY, _DKEY)  if (itP->first == #_PropKEY) { ZQTianShan::Util::updateValueMapData(ed.eventData, ZQ::StreamService::_DKEY, (Ice::Int) atol(itP->second.c_str())); continue; }

				COPY_STR_PARAM_TO_VMAP(currentItem, EVENT_ITEMSTEP_NEXT_ITEM_NAME);
				COPY_STR_PARAM_TO_VMAP(prevItem,    EVENT_ITEMSTEP_PREV_ITEM_NAME);

				COPY_INT_PARAM_TO_VMAP(prevCtrlNum,    EVENT_ITEMSTEP_ITEM_PREV_CTRLNUM);
				COPY_INT_PARAM_TO_VMAP(currentCtrlNum, EVENT_ITEMSTEP_ITEM_CUR_CTRLNUM);
				// COPY_STR_PARAM_TO_VMAP(EVENT_ITEMSTEP_TIME_STAMP_UTC);

				COPY_STR_PARAM_TO_VMAP(currentPID,   EVENT_ITEMSTEP_CUR_ITEM_PID);
				COPY_STR_PARAM_TO_VMAP(currentPAID,  EVENT_ITEMSTEP_CUR_ITEM_PAID);
				COPY_INT_PARAM_TO_VMAP(currentDur,   EVENT_ITEMSTEP_CUR_ITEM_DUR);
				COPY_INT_PARAM_TO_VMAP(currentFlags, EVENT_ITEMSTEP_CUR_ITEM_FLAGS);

				COPY_STR_PARAM_TO_VMAP(prevPID,      EVENT_ITEMSTEP_PREV_ITEM_PID);
				COPY_STR_PARAM_TO_VMAP(prevPAID,     EVENT_ITEMSTEP_PREV_ITEM_PAID);
				COPY_INT_PARAM_TO_VMAP(prevDur,      EVENT_ITEMSTEP_PREV_ITEM_DUR);
				COPY_INT_PARAM_TO_VMAP(prevFlags,    EVENT_ITEMSTEP_PREV_ITEM_FLAGS);
			}

			ZQ::StreamService::pServiceInstance->getEventSender().postEvent(ed);
			_log(Log::L_DEBUG, CLOGFMT(NGODSession, "Session[%s] event ItemStepped issued per Transition(%d) ANNOUNCE: %s"), _sessGuid.c_str(), AnnounceCode, paramPrintStr.c_str());
		}
		break;

	default:
		break;
	}

	if (!respStartLine.empty())
	{
		NGODClient* client = (NGODClient*) &rtspClient;
		if (client)
			client->sendTousyMsg(respStartLine, respHeaders, "", cseq);
	}

	_log(Log::L_INFO, CLOGFMT(NGODSession, "Session[%s] @grp[%s] cseq(%d) OnANNOUNCE[%d] dispatched w/ latency[%d]msec: flags[%x] pos[%lld] dur[%lld] scale[%.2f] sstate[%d] cstate[%d] %s"), _sessGuid.c_str(), getSessionGroupName().c_str(), pInMessage->cSeq,
		AnnounceCode, (int)(ZQ::common::now() - stampStart), 
		paras.mask, paras.timeoffset, paras.duration, paras.scale, paras.streamState, paras.contentState, paramPrintStr.c_str());
}

void NGODSession::OnSessionTimer()
{
	if (_stampLastMessage>0 && _stampSetup <=0)
	{
		destroy();
		return;
	}

	NGODClient* client = _group.getC1Client(controlUri()); 
	if (!client)
		return;

	RTSPRequest::AttrList parameterNames;
	RTSPMessage::AttrMap headers;
	MAPSET(RTSPMessage::AttrMap, headers, "OnDemandSessionId", _sessGuid);
	//		MAPSET(RTSPMessage::AttrMap, headers, "Require",          client->requireString());
	MAPSET(RTSPMessage::AttrMap, headers, "Content-Type",     "text/parameters");
	client->sendGET_PARAMETER(*this, parameterNames, NULL,    headers);
	_log(Log::L_DEBUG, CLOGFMT(NGODSession, "Session[%s] OnSessionTimer(), trigger PING"), _sessGuid.c_str());
}

bool NGODSession::parseStreamInfo(const RTSPMessage::Ptr& pResp)
{
//	ZQ::common::MutexGuard g(_mutex);

	//reset streaminfo so that we can assure duration is 0 if it is not returned from streaming server
	_streamInfos = StreamInfos();
	if (pResp->headers.find("Range") != pResp->headers.end())
	{
		std::vector<std::string> temp;
		ZQ::common::stringHelper::SplitString(pResp->headers["Range"], temp, "=- ", "=- ");
		if (temp.size() >= 2)
			_streamInfos.timeoffset = NGODClient::parseNptValue(temp[1].c_str());

		if( temp.size() >= 3)
			_streamInfos.duration = NGODClient::parseNptValue(temp[2].c_str());
	}

	if (pResp->headers.find("Scale") != pResp->headers.end())
	{
		_streamInfos.scale = (float) atof(pResp->headers["Scale"].c_str());
	}

	_tianShanNotice = "";
	if (pResp->headers.find("TianShan-Notice") != pResp->headers.end())
	{
		_tianShanNotice = pResp->headers["TianShan-Notice"];
	}

	return true;
}

NGODClient*	NGODSession::getR2Client()
{
	return _group.getR2Client();
}

NGODClient* NGODSession::getC1Client()
{
	return _group.getC1Client(_controlUri);
}


// -----------------------------
// class NGODClient
// -----------------------------
bool NGODClient::_bTryDecimalNpt =false;

NGODClient::NGODClient(NGODSessionGroup& group, ClientType type, Log& log, NativeThreadPool& thrdpool, InetHostAddress& bindAddress, const std::string& baseURL, const char* userAgent, Log::loglevel_t verbosityLevel, tpport_t bindPort)
: RTSPClient(log, thrdpool, bindAddress, baseURL, userAgent, verbosityLevel, bindPort), _group(group), _type(type), _bDecimalNpt(false), _bNptHandshaked(true), _cContinuousTimeoutInConn(0), _cContinuousTimeout(0), _stampLastRespInTime(0)
{
	if (_bTryDecimalNpt)
	{
		_bDecimalNpt = true;
		_bNptHandshaked = false;
	}

	int poolSize, activeCount, pendingSize;
	TCPSocket::getThreadPoolStatus(poolSize, activeCount, pendingSize);
//	TCPSocket::getPendingSize(poolSize);
	TCPSocket::resizeThreadPool(poolSize+1);

	// adjust the connect timeout
	if (pNSSBaseConfig->_videoServer.SessionInterfaceRequestTimeout >= 1000)
	{
		// validate the connect timeout, which is necessary when apply setClientTimeout()
		if (pNSSBaseConfig->_videoServer.SessionInterfaceConnectTimeout< 500 || pNSSBaseConfig->_videoServer.SessionInterfaceConnectTimeout > pNSSBaseConfig->_videoServer.SessionInterfaceRequestTimeout)
			pNSSBaseConfig->_videoServer.SessionInterfaceConnectTimeout = max(pNSSBaseConfig->_videoServer.SessionInterfaceRequestTimeout-1000, pNSSBaseConfig->_videoServer.SessionInterfaceRequestTimeout*3/4);

		setClientTimeout(pNSSBaseConfig->_videoServer.SessionInterfaceConnectTimeout, pNSSBaseConfig->_videoServer.SessionInterfaceRequestTimeout);
	}
}

NGODClient::~NGODClient()
{
	std::string str;
	ZQ::common::MutexGuard g(_lkEventMap);
	for (EventMap::iterator it = _eventMap.begin(); it !=_eventMap.end(); it ++)
	{
		char buf[64];
//		Event* pEvent = (Event*) it->second;
		snprintf(buf, sizeof(buf)-2, "seq(%d)[%p] ", it->first, it->second.get());
		str += buf;

//		if (NULL == pEvent)
//			continue;
//		delete pEvent;
	}
	_eventMap.clear();

	_log(Log::L_DEBUG, CLOGFMT(NGODClient, "~NGODClient() conn[%s]: cleaned await events: %s"), connDescription(), str.c_str());
}

int NGODClient::sendTousyMsg(const std::string& startLine, RTSPMessage::AttrMap& headers, const std::string& body, const int cseq)
{
	RTSPMessage::Ptr pMessage = new RTSPMessage((cseq <= 0) ? lastCSeq() : cseq);
	pMessage->startLine = startLine;

	MAPSET(RTSPMessage::AttrMap, headers, "Require", (NC_R2 == _type) ? "com.comcast.ngod.r2" : "com.comcast.ngod.c1" );
	return sendMessage(pMessage, headers);
}

/*
void NGODClient::sendPing()
{
	RTSPMessage::Ptr pMessage = new RTSPMessage(lastCSeq());
	pMessage->startLine = "PING * RTSP/1.0";
	RTSPMessage::AttrMap     headers;
	sendMessage(pMessage, headers);
}
*/

uint32 NGODClient::getPenalty()
{
	int period = _stampLastRespInTime ? (int)(ZQ::common::now() - _stampLastRespInTime) :0;
	int penalty = _cContinuousTimeout;

	// set a max penalty to start with
	if (pNSSBaseConfig->_videoServer.dropRequestByPenalties >0 && penalty > pNSSBaseConfig->_videoServer.dropRequestByPenalties*2)
		penalty = pNSSBaseConfig->_videoServer.dropRequestByPenalties*2;
	
	penalty -= period /1000; // decrease by 1Hz
	if (penalty <=0)
		return 0;

	_log(Log::L_DEBUG, CLOGFMT(NGODClient, "getPenalty() sessgroup[%s]-conn[%s] penalty[%d] by timeouts[%d] and dropRequestByPenalties[%d] within [%d]msec"), _group.getName().c_str(), connDescription(), 
			penalty, _cContinuousTimeout, pNSSBaseConfig->_videoServer.dropRequestByPenalties, period);
	return penalty; 
}

void NGODClient::OnResponse(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString)
{
	if (!pReq)
		return;

	int64 stampStart = ZQ::common::now();
	_log(Log::L_DEBUG, CLOGFMT(NGODClient, "OnResponse() sessgroup[%s]-conn[%s] %s(%d) received response: %d %s"), _group.getName().c_str(), connDescription(), pReq->_commandName.c_str(), pReq->cSeq, resultCode, resultString);

	do
	{
		// wildcast to check "551 Option Not Supported" about ECR: Protocol Versioning for RTSP protocols
		if (resultCode == RTSPSink::rcOptionNotSupport)
		{
			if (pResp->headers.end() != pResp->headers.find("Unsupported"))
			{
				std::string& valOfResponse = pResp->headers["Unsupported"];
				if (std::string::npos != valOfResponse.find("decimal_npts"))
					_bDecimalNpt =false;
			}
		}

		_bNptHandshaked = true;

		if (pReq->_commandName == "GET_PARAMETER")
		{
			if (resultCode != RTSPSink::rcOK) 
				break; // ingore failed GET_PARAMETER

			// GET_PARAMETER ok
			std::string groupname = _group.getName();
			if (pResp->headers.end() != pResp->headers.find("SessionGroup"))
			{
				std::string& gnOfResponse = pResp->headers["SessionGroup"];
				if (0 != groupname.compare(gnOfResponse))
				{
					_log(Log::L_WARNING, CLOGFMT(NGODClient, "OnResponse() sessgroup[%s]-conn[%s] %s(%d) unmatched SessionGroup[%s] in response, ignore"),
						groupname.c_str(), connDescription(), pReq->_commandName.c_str(), pReq->cSeq, gnOfResponse.c_str());
					break;
				}
			}

			std::vector<NGODClient::Session_Pair> list;
			size_t index = 0;
			std::string key("session_list:");
			index = pResp->contentBody.find(key);
			if (std::string::npos == index)
			{
				_log(Log::L_DEBUG, CLOGFMT(NGODClient, "OnResponse() conn[%s] %s(%d) SessionGroup[%s] no session_list found in response, ignore"), 
					connDescription(), pReq->_commandName.c_str(), pReq->cSeq, groupname.c_str());
				break;
			}

			index += key.length();
			// chop string to get session_list: line
			std::string line = pResp->contentBody.substr(index, pResp->contentBody.size());
			_log(Log::L_DEBUG, CLOGFMT(NGODClient, "OnResponse() SessionGroup[%s] session_list[%s]"), groupname.c_str(), line.c_str());
			std::vector<std::string> tmpVec;	
			stringHelper::SplitString(line, tmpVec, " \t", " \t\r\n","","");
			for (size_t i = 0; i < tmpVec.size(); i++)
			{
				NGODClient::Session_Pair sessPair;
				index = tmpVec[i].find(":");
				if(index != std::string::npos)
				{
					sessPair._sessionId = tmpVec[i].substr(0, index);
					sessPair._onDemandSessionId = tmpVec[i].substr(index+1, tmpVec[i].size());						
				}
				else sessPair._sessionId = tmpVec[i];

				list.push_back(sessPair);
			}

			_group.OnSessionListOfSS(list);
			break;
		} // if GET_PARAMETER

	} while(0);

	// wake up the waiting
	wakeupByCSeq(pReq->cSeq);

	_log(Log::L_INFO, CLOGFMT(NGODClient, "OnResponse() sessgroup[%s]-conn[%s] %s(%d) response dispatched w/ latency[%d]msec: %d %s"), _group.getName().c_str(), connDescription(), pReq->_commandName.c_str(), pReq->cSeq, (int)(ZQ::common::now() - stampStart), resultCode, resultString);
}

void NGODClient::OnServerRequest(RTSPClient& rtspClient, const char* cmdName, const char* url, RTSPMessage::Ptr& pInMessage)
{
	_log(Log::L_INFO, CLOGFMT(NGODClient, "OnServerRequest() conn[%s] received peer request %s(%d)"), connDescription(), pInMessage->startLine.c_str(), pInMessage->cSeq);
	do
	{
		//check if it is session in progress
		if (0 == strcmp(cmdName, "ANNOUNCE"))
		{
			std::string notice = "";
			if (pInMessage->headers.find("Notice") == pInMessage->headers.end())
				break; // ignore those ANNOUNCE with no Notice

			notice = pInMessage->headers["Notice"];
			std::vector<std::string> temp;
			ZQ::common::stringHelper::SplitString(notice, temp, " ", " ");
			if (temp.empty())
				break; // ignore those ANNOUNCE with illegal Notice

			int AnnounceCode = atoi(temp[0].c_str());

			// the session must not found when reaches NGODClient::OnServerRequest(), otherwise should be NGODSession::OnServerRequest
			switch (AnnounceCode)
			{
			case RTSPSink::racSessionInProgress:
				{
					std::string cseq;
					if (pInMessage->headers.find("CSeq") != pInMessage->headers.end())
						cseq = pInMessage->headers["CSeq"];

					RTSPMessage::Ptr pMessage = new RTSPMessage(atoi(cseq.c_str()));
					pMessage->startLine = "RTSP/1.0 454 Session Not Found";
					RTSPMessage::AttrMap headers;
					bool bSessIdReceived = false;

					if (pInMessage->headers.end() == pInMessage->headers.find("Session"))
					{
						MAPSET(RTSPMessage::AttrMap, headers, "Session", pInMessage->headers["Session"]);
						bSessIdReceived = true;
					}

					// copy the necessary headers from the ANNOUNCE
					if (pInMessage->headers.end() != pInMessage->headers.find("OnDemandSessionId"))
						MAPSET(RTSPMessage::AttrMap, headers, "OnDemandSessionId", pInMessage->headers["OnDemandSessionId"]);

					// send the response to the ANNOUNCE
					sendMessage(pMessage, headers); 

					if (bSessIdReceived && std::string::npos != pNSSBaseConfig->_videoServer.vendor.find("cisco"))
					{
						// ticket#12420 Cisco Integration
						// the NGOD R2 spec states the client "MAY or MAY NOT send a TEARDOWN after 454", but according to ticket#12420,
						// Cisco didn't follow 454 to clean the session and expect a TREARDOWN here

						// MAPSET(RTSPMessage::AttrMap, headers, "Reason",  "428 \"Session List Missmatch\"");
						MAPSET(RTSPMessage::AttrMap, headers, "Reason", "428 \"Session Orphan Detected on SS\"");

						sendRequest(new RTSPRequest(*this, lastCSeq(), "TEARDOWN"), headers);
					}
				}
				break;

			case RTSPSink::racClientSessionTerminated:
				break;

			default:
				break;
			}
		}
	} while(0);
}

void NGODClient::OnRequestError(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RequestError errCode, const char* errDesc)
{
	if (!pReq)
		return;

	RTSPClient::OnRequestError(rtspClient, pReq, errCode, errDesc);

	// wake up the waiting
	wakeupByCSeq(pReq->cSeq, false);
}

#define REQ_PROTO(_CONN_TYPE) "com.comcast.ngod." #_CONN_TYPE

const char* NGODClient::requireString()
{
	if (NC_R2 == _type)
		return ((!_bNptHandshaked || _bDecimalNpt ) ? REQ_PROTO(r2) ", " REQ_PROTO(r2) ".decimal_npts" : REQ_PROTO(r2));

	return ((!_bNptHandshaked || _bDecimalNpt ) ? REQ_PROTO(c1) ", " REQ_PROTO(c1) ".decimal_npts" : REQ_PROTO(c1));
}

std::string NGODClient::formatNptValue(long msec, const char* ngodRequire)
{
	char buf[32];
	bool decimalNpt = _bDecimalNpt;
	if (NULL != ngodRequire && strlen(ngodRequire) >0)
		decimalNpt = (NULL != ::strstr(ngodRequire, ".decimal_npts"));

	if (decimalNpt)
		snprintf(buf, sizeof(buf)-2, "%d.%03d", msec/1000, msec%1000);
	else
		snprintf(buf, sizeof(buf)-2, "%x", msec);

	return buf;
}

long NGODClient::parseNptValue(const char* nptFieldStr)
{
	if (NULL == nptFieldStr)
		return 0;

	long hex=0;
	float fsec =0.0;
	
	if (NULL != strstr(nptFieldStr, "now"))
		return NPT_NOW;

	if (NULL != strchr(nptFieldStr, '.'))
	{
		sscanf(nptFieldStr, "%f", &fsec);
		return (long) ((fsec+0.0005) *1000);
	}

	int ret =sscanf(nptFieldStr, "%lx", &hex);
	if (ret <=0)
		ret =sscanf(nptFieldStr, "0[xX]%lx", &hex);
	
	if (ret <=0) // kind of impossible
		return (long) ((fsec+0.0005) *1000);

	return hex;
}

void NGODClient::OnConnected()
{
	RTSPClient::OnConnected();

	if (Socket::stConnected != TCPSocket::state())
		return; // the current connecting has problem, skip the handling

	if (NC_R2 == _type)	//need sync and set_parameter
	{
		if (now() - _group.getLastSync() > 60000)
		{	
			if(_group.sync(true) > 0)
				_group.setLastSync(now(), NGODSessionGroup::Sync);
		}

		RTSPRequest::AttrMap paramMap;
		paramMap.insert(std::make_pair("session_groups", _group.getName()));
		
		RTSPMessage::AttrMap headers;
		MAPSET(RTSPMessage::AttrMap, headers, "SessionGroup", _group.getName());
//		MAPSET(RTSPMessage::AttrMap, headers, "Require",      requireString());
		MAPSET(RTSPMessage::AttrMap, headers, "Content-Type", "text/parameters");
		
		sendSET_PARAMETER(paramMap, NULL, headers);
	}
}

void NGODClient::OnError()
{
	RTSPClient::OnError();
	if(NC_R2 == _type)
	{
		_group.setStatus(NGODSessionGroup::Idle);
	}
}

int NGODClient::OnRequestPrepare(RTSPRequest::Ptr& pReq)
{
	if (!pReq || !_group._env.isRunning())
		return -1;

	MAPSET(RTSPMessage::AttrMap, pReq->headers, "Require", requireString());

	ZQ::common::MutexGuard g(_lkEventMap);
	if (pReq->cSeq<=0 || _eventMap.end() != _eventMap.find(pReq->cSeq))
		return -2;
	
	Event::Ptr pEvent = new Event();
	if (!pEvent)
		return -3;
	
	MAPSET(EventMap, _eventMap, pReq->cSeq, pEvent);

	_log(Log::L_DEBUG, CLOGFMT(NGODClient, "OnRequestPrepare() conn[%s](%d): added syncEvent[%p]"),
		connDescription(), pReq->cSeq, pEvent.get());

	return 0;
}

void NGODClient::OnRequestClean(RTSPRequest& req)
{
	ZQ::common::MutexGuard g(_lkEventMap);
	EventMap::iterator it = _eventMap.find(req.cSeq);
	if (_eventMap.end() == it)
		return;

	_log(Log::L_DEBUG, CLOGFMT(NGODClient, "OnRequestClean() conn[%s](%d): syncEvent[%p] cleaned"),
		connDescription(), req.cSeq, it->second.get());

//	Event* pEvent = (Event*)it->second;
	_eventMap.erase(it);

//	if (NULL != pEvent)
//		delete pEvent;

}

bool NGODClient::waitForResponse(uint32 cseq)
{
	Event::Ptr pEvent =NULL;

	{
		ZQ::common::MutexGuard g(_lkEventMap);
		EventMap::iterator it = _eventMap.find(cseq);
		if (_eventMap.end() != it)
			pEvent = it->second;
	}

	if (!pEvent)
		return false;

	_log(Log::L_DEBUG, CLOGFMT(NGODClient, "waitForResponse() conn[%s](%d): waiting for syncEvent[%p] timeout[%d]"),
		connDescription(), cseq, pEvent.get(), _messageTimeout);

	if (SYS::SingleObject::SIGNALED == pEvent->wait(_messageTimeout) && pEvent->isSuccess() )
	{
		_stampLastRespInTime      = ZQ::common::now();
		_cContinuousTimeoutInConn =0;
		_cContinuousTimeout       =0;
		return true;
	}

	_cContinuousTimeout++;
	_log(Log::L_DEBUG, CLOGFMT(NGODClient, "waitForResponse() SessionGroup[%s] conn[%s] encounted [%d/%d] timeouts in the past [%d]msec"),
		_group.getName().c_str(), connDescription(), _cContinuousTimeoutInConn, _cContinuousTimeout, (int)(ZQ::common::now() - _stampLastRespInTime));

	if (pNSSBaseConfig->_videoServer.SessionInterfaceDisconnectAtTimeout > 1 && isConnected() && ++_cContinuousTimeoutInConn >= (uint)pNSSBaseConfig->_videoServer.SessionInterfaceDisconnectAtTimeout)
	{
		_log(Log::L_WARNING, CLOGFMT(NGODClient, "waitForResponse() SessionGroup[%s] conn[%s]: encounted timeout[%d]msec continously for [%d/%d]times but limitation[%d], give it up and reconnecting"),
			_group.getName().c_str(), connDescription(), _messageTimeout, _cContinuousTimeoutInConn, _cContinuousTimeout, pNSSBaseConfig->_videoServer.SessionInterfaceDisconnectAtTimeout);

		disconnect();
		_cContinuousTimeoutInConn =0;
	}

	return false;
}

void NGODClient::wakeupByCSeq(uint32 cseq, bool success)
{
	Event::Ptr pEvent =NULL;

	{
		ZQ::common::MutexGuard g(_lkEventMap);
		EventMap::iterator it = _eventMap.find(cseq);
		if (_eventMap.end() != it)
			pEvent = it->second;
	}

	_log(Log::L_DEBUG, CLOGFMT(NGODClient, "wakeupByCSeq() conn[%s](%d): signalling syncEvent[%p] succ[%c]"),
		connDescription(), cseq, pEvent.get(), success?'T':'F');

	if (!pEvent)
		return;

	pEvent->signal(success);
}


// -----------------------------
// class NGODSessionGroup
// -----------------------------
NGODSessionGroup::SessionGroupMap NGODSessionGroup::_groupMap;
NGODSessionGroup::StringIndex     NGODSessionGroup::_idxGroupBaseUrl;
Mutex NGODSessionGroup::_lockGroups;

NGODSessionGroup::NGODSessionGroup(NSSEnv& env, const std::string& name, const std::string& baseURL, int max, Ice::Long syncInterval)
: _env(env), _name(name), _baseURL(baseURL), _max(max), _status(Idle), _syncInterval(syncInterval), _stampLastSync(0),
_R2Client(*this, NGODClient::NC_R2, env.getMainLogger(), env._rtspThpool, env._bindAddr, baseURL, env._userAgent.c_str(), env._rtspTraceLevel)  
{
	_idxGroupBaseUrl.insert(StringIndex::value_type(_baseURL, name));
}

#define _log        (_env.getMainLogger())
NGODSessionGroup::~NGODSessionGroup()
{
	{
		MutexGuard g(_lockSessMap);
		_sessMap.clear();
	}

	clearNGODClient();
}

void NGODSessionGroup::add(NGODSession& sess)
{
	MutexGuard g(_lockSessMap);
	MAPSET(SessionMap, _sessMap, sess._sessGuid, &sess);
	if (!sess._sessionId.empty())
		_sessIdIndex.insert(StringIndex::value_type(sess._sessionId, sess._sessGuid));
}

void NGODSessionGroup::remove(NGODSession& sess)
{
	if (!sess._sessionId.empty())
		eraseSessionId(sess._sessionId, sess);

	MutexGuard g(_lockSessMap);
	_sessMap.erase(sess._sessGuid);
}

void NGODSessionGroup::updateIndex(NGODSession& sess, const char* indexName, const char* oldValue)
{
	std::string oldSessId = oldValue? oldValue:sess._sessionId;

	if (!oldSessId.empty())
		eraseSessionId(oldSessId, sess);

	MutexGuard g(_lockSessMap);
	_sessIdIndex.insert(StringIndex::value_type(sess._sessionId, sess._sessGuid));
}

NGODSession::List NGODSessionGroup::lookupByIndex(const char* sessionId, const char* indexName)
{
	NGODSession::List list;

	if (NULL == sessionId || (NULL != indexName && 0 !=stricmp(indexName, "SessionId")))
		return list; // unsupported input parameters

	MutexGuard g(_lockSessMap);
	StringIdxRange range = _sessIdIndex.equal_range(sessionId);

	for (StringIndex::iterator it = range.first; it != range.second; ++it)
	{
		SessionMap::iterator itS = _sessMap.find(it->second);
		if (_sessMap.end() ==itS)
			continue;

		list.push_back(itS->second);
	}

	return list;
}

NGODSession::Ptr NGODSessionGroup::lookupByOnDemandSessionId(const char* sessOnDemandId)
{
	if (NULL == sessOnDemandId)
		return NULL;

	MutexGuard g(_lockSessMap);
	SessionMap::iterator it = _sessMap.find(sessOnDemandId);
	if (_sessMap.end() ==it)
		return NULL;

	return it->second;
}

void NGODSessionGroup::eraseSessionId(const std::string& sessionId, NGODSession& sess)
{
	MutexGuard g(_lockSessMap);
	bool bFound =true;
	while (bFound)
	{
		StringIdxRange range = _sessIdIndex.equal_range(sessionId);
		bFound = false;

		for (StringIndex::iterator it = range.first; it != range.second; ++it)
		{
			if (0 != it->second.compare(sess._sessGuid))
				continue;

			bFound = true;
			_sessIdIndex.erase(it);
			break;
		}
	}
}

NGODClient* NGODSessionGroup::getC1Client(const std::string& controlURL)
{
	if (controlURL.empty())
		return NULL;

	// cut off the path part from the _controlUri to take only the base url
	ZQ::common::URLStr url(controlURL.c_str(), false);
	char baseUrl[256];
	snprintf(baseUrl, sizeof(baseUrl)-2, "%s://%s:%d/", url.getProtocol(), url.getHost(), url.getPort());

	MutexGuard g(_lockClients);
	NGODClientMap::iterator iter = _C1ClientMap.find(baseUrl);
	if (iter != _C1ClientMap.end())
		return iter->second;

	//	if no connection exists, establish a new one
	_log(Log::L_DEBUG, CLOGFMT(NGODSessionGroup, "SessionGroup[%s] adding C1 connection to baseUrl[%s] per ctrlUrl[%s]"), _name.c_str(), baseUrl, controlURL.c_str());
	NGODClient* client = new NGODClient(*this, NGODClient::NC_C1, _log, _env._rtspThpool, _env._bindAddr, baseUrl, _env._userAgent.c_str(), _env._rtspTraceLevel);
	if (NULL ==client)
		return NULL;
	
	_C1ClientMap.insert(std::make_pair(baseUrl, client));

	return client;
}

NGODClient* NGODSessionGroup::getR2Client()
{ 
	return &_R2Client;
}

int NGODSessionGroup::sync(bool bOnConnected)
{
	_log(Log::L_DEBUG, CLOGFMT(NGODSessionGroup, "SessionGroup[%s] sync() per %s"), _name.c_str(), bOnConnected ? "connected" :"timer");
	if (!bOnConnected && Idle != _status)
	{
		_log(Log::L_DEBUG, CLOGFMT(NGODSessionGroup, "SessionGroup[%s] is already in sync"), _name.c_str());
		return 0;
	}

	if (bOnConnected && Socket::stConnected != _R2Client.state())
		return -1;

	//set group status
	RTSPRequest::AttrList parameterNames;
	parameterNames.push_back("session_list");
//	parameterNames.push_back("connection_timeout");
	
	RTSPMessage::AttrMap headers;
	MAPSET(RTSPMessage::AttrMap, headers, "SessionGroup", _name);
//	MAPSET(RTSPMessage::AttrMap, headers, "Require",      "com.comcast.ngod.r2");
	MAPSET(RTSPMessage::AttrMap, headers, "Content-Type", "text/parameters");
	return _R2Client.sendGET_PARAMETER(parameterNames, NULL, headers);
}

void NGODSessionGroup::OnSessionListOfSS(const std::vector<NGODClient::Session_Pair>& listOnSS)
{
	_log(Log::L_DEBUG, CLOGFMT(NGODSessionGroup, "OnSessionListOfSS() SessionGroup[%s] %d sessions on SS"), _name.c_str(), listOnSS.size());

	int64 stampStart= ZQ::common::now();
	size_t cSSOrphan =0, cSynced=0, cLocalOrphan=0, cDestroyed=0, cTorndown=0;

	try {
	
		std::vector<NGODSession::Ptr> _awaitDestroySession;
		int64 stampLastSyncThrsh = getLastSync() -20*1000; // 20sec earilier to minimize the mis-destroy

		{
			MutexGuard g(_lockSessMap);
			for (std::vector<NGODClient::Session_Pair>::const_iterator iter = listOnSS.begin();	iter != listOnSS.end(); iter++)
			{
				std::string tmpStr = iter->_onDemandSessionId + ":" + iter->_sessionId;
				bool bFound = false;

				// find session by key
				if (!iter->_onDemandSessionId.empty() && _sessMap.end() != _sessMap.find(iter->_onDemandSessionId))
					bFound = true;
				else if (!iter->_sessionId.empty() && _sessIdIndex.end() != _sessIdIndex.find(iter->_sessionId))
					bFound = true;

				if (bFound)
				{
					cSynced ++;
//					_log(Log::L_DEBUG, CLOGFMT(NGODSessionGroup, "OnSessionListOfSS() SessionGroup[%s] session[%s] sync-ed"), _name.c_str(), tmpStr.c_str());
					continue;
				}

				cSSOrphan ++;
				_log(Log::L_INFO, CLOGFMT(NGODSessionGroup, "OnSessionListOfSS() SessionGroup[%s] SS-orphan[%s] found, tearing it down"), _name.c_str(), tmpStr.c_str());

				//TODO: remove session from media server by TEARDOWN

				RTSPMessage::AttrMap headers;
				MAPSET(RTSPMessage::AttrMap, headers, "SessionGroup", _name);
				MAPSET(RTSPMessage::AttrMap, headers, "Session", iter->_sessionId);
				if (!iter->_onDemandSessionId.empty())
					MAPSET(RTSPMessage::AttrMap, headers, "OnDemandSessionId", iter->_onDemandSessionId);

				// MAPSET(RTSPMessage::AttrMap, headers, "Reason",  "428 \"Session List Missmatch\"");
				MAPSET(RTSPMessage::AttrMap, headers, "Reason", "428 \"Session Orphan Detected on SS\"");

				_R2Client.sendTousyMsg("TEARDOWN * RTSP/1.0", headers);

				cTorndown++;
			}

			//TODO: a bit stupid algorithm, need to be replaced -andy
			// sessions only in NSS
			for (SessionMap::iterator sessIter = _sessMap.begin(); sessIter != _sessMap.end(); sessIter++)
			{
				std::string onDemandSessionId = (sessIter->second)->_sessGuid;
				std::vector<NGODClient::Session_Pair>::const_iterator pIter = find_if(listOnSS.begin(), listOnSS.end(), NGODClient::FindByOnDemandSessionID(onDemandSessionId));
				if (pIter == listOnSS.end() && 
					((sessIter->second)->getStampSetup() > 0 )&&
					(sessIter->second)->getStampSetup() < stampLastSyncThrsh)
				{
					_awaitDestroySession.push_back(sessIter->second);
				}
			}
		}

		cLocalOrphan = _awaitDestroySession.size();

#if 0
		// sync streams in database
		::TianShanIce::Streamer::SsPlaylistS sessions = ZQ::StreamService::pServiceInstance->listSessions();

		for (::TianShanIce::Streamer::SsPlaylistS::iterator sIter = sessions.begin(); sIter != sessions.end(); sIter++)
		{
			std::string groupName = (*sIter)->getAttribute(SESSION_GROUP);
			if (groupName != _name)
				continue;

			std::string sessName  = (*sIter)->getAttribute(ONDEMANDNAME_NAME);
			std::vector<NGODClient::Session_Pair>::const_iterator pIter = find_if(listOnSS.begin(), listOnSS.end(), NGODClient::FindByOnDemandSessionID(sessName));

			if (pIter != listOnSS.end())
				continue;

			std::string timeStamp = (*sIter)->getAttribute(SETUP_TIMESTAMP);
			if(_atoi64(timeStamp.c_str()) >= stampLastSyncThrsh)
			{
				//if sess setup after sync, do not destroy it
				char szSessSetupTime[256];
				char szSyncStartTime[256];
				ZQTianShan::TimeToUTC(_atoi64(timeStamp.c_str()), szSessSetupTime, sizeof(szSessSetupTime));
				ZQTianShan::TimeToUTC(getLastSync(), szSyncStartTime, sizeof(szSyncStartTime));

				_log(Log::L_DEBUG, CLOGFMT(NGODSessionGroup, "OnSessionListOfSS() SessionGroup[%s] local-db-orphan[%s] setup-time[%s] after sync start-time[%s], skip destroying this round"), _name.c_str(), sessName.c_str(), szSessSetupTime, szSyncStartTime);
				continue;
			}

			// destroy this session
			Ice::Context ctx;
			ctx.insert(std::make_pair("caller", "SYNC"));
			(*sIter)->destroy(ctx);
			cDestroyed++;
		}
#endif // 0

		for (std::vector<NGODSession::Ptr>::iterator desIter = _awaitDestroySession.begin(); desIter != _awaitDestroySession.end(); desIter++)
		{
			_log(Log::L_INFO, CLOGFMT(NGODSessionGroup, "OnSessionListOfSS() destroying local-orphan[%s]"), (*desIter)->_sessGuid.c_str());
			(*desIter)->destroy();
			cDestroyed++;
		}
	}
	catch(...)
	{
	}

	setLastSync(now(), Idle);
	_log(Log::L_INFO, CLOGFMT(NGODSessionGroup, "syncSessionList() SessionGroup[%s] done, took [%lld]msec: sync-ed[%d], SS-orphans[%d]->torndown[%d]; local-orphans[%d]->destroyed[%d]"), _name.c_str(), 
		ZQ::common::now() -stampStart, cSynced, cSSOrphan, cTorndown, cLocalOrphan, cDestroyed);
}

void NGODSessionGroup::OnTimer(const ::Ice::Current& c)
{
	int64 stampNow = now();

	if (getLastSync() + getSyncInterval() > stampNow) // do not need sync now
		return;

	sync();
	setLastSync(stampNow, Sync);
}

NGODSession::Ptr NGODSessionGroup::createSession(const std::string& ODSessId, const std::string& strmDestUrl, const char* baseURL)
{
	NGODSessionGroup::Ptr pSessGroup = NULL;

	typedef std::vector<std::string> GroupNameList;
	GroupNameList gnlist;

	if (ODSessId.empty())
		return NULL;

	size_t count = 0;
	if (NULL == baseURL || 0 == *baseURL || '*' == *baseURL)
		gnlist =getAllSessionGroup();
	else
	{
		MutexGuard g(_lockGroups);
		StringIdxRange range = _idxGroupBaseUrl.equal_range(baseURL);
		for (StringIndex::iterator it = range.first; it != range.second; it++)
			gnlist.push_back(it->second);
	}

	int startIdx = rand(); 
	count = gnlist.size();

	MutexGuard g(_lockGroups);
	for (size_t i = 0; i< count; i++)
	{
		std::string& groupName = gnlist[(startIdx +i) %count];
		if (_groupMap.end() == _groupMap.find(groupName))
			continue;

		NGODSessionGroup::Ptr pGroup = _groupMap[groupName];
		if (NULL == pGroup || pGroup->size() >= (size_t) pGroup->getMaxSize())
			continue;

		pSessGroup = pGroup;
		glog(Log::L_DEBUG, CLOGFMT(NGODSessionGroup, "createSession() SessionGroup[%s] selected for session[%s]"), pSessGroup->getName().c_str(), ODSessId.c_str());
		break;
	}

	if (NULL == pSessGroup)
	{
		glog(Log::L_WARNING, CLOGFMT(NGODSessionGroup, "createSession() session[%s]: none of %d valid SessionGroup(s) has quota available"), ODSessId.c_str(), count);
		return NULL;
	}

//	NGODSessionGroup& group, const std::string& ODSessId, const char* streamDestUrl, const char* filePath
	return (new NGODSession(*pSessGroup, ODSessId, strmDestUrl));
}

NGODSession::Ptr NGODSessionGroup::openSession(const std::string& ODSessId, const std::string& groupName, bool bRestore)
{
	NGODSessionGroup::Ptr pSessGroup = NGODSessionGroup::findSessionGroup(groupName);
	if (NULL == pSessGroup)
		return NULL;

	NGODSession::Ptr clientSession = pSessGroup->lookupByOnDemandSessionId(ODSessId.c_str());

	if (NULL != clientSession)
		return clientSession;

	if (!bRestore)
		return NULL;

	return (new NGODSession(*pSessGroup, ODSessId, "", pSessGroup->getBaseURL().c_str()));
}


NGODSessionGroup::Ptr NGODSessionGroup::findSessionGroup(const std::string& name)
{
	NGODSessionGroup::Ptr group;
	
	MutexGuard g(_lockGroups);
	SessionGroupMap::iterator it = _groupMap.find(name);
	if (_groupMap.end() !=it)
		group = it->second;

	return group;
}

std::vector<std::string> NGODSessionGroup::getAllSessionGroup()
{
	MutexGuard g(_lockGroups);
	std::vector<std::string> result;

	for (SessionGroupMap::iterator it = _groupMap.begin(); it != _groupMap.end(); it++)
		result.push_back(it->first);

	return result;
}

void NGODSessionGroup::clearSessionGroup()
{
	MutexGuard g(_lockGroups);

	_groupMap.clear();
	_idxGroupBaseUrl.clear();
}

void NGODSessionGroup::clearNGODClient()
{
	MutexGuard g(_lockClients);
	NGODClientMap::iterator iter = _C1ClientMap.begin();
	for (; iter != _C1ClientMap.end(); iter++)
	{
		try
		{
			NGODClient* pClient = iter->second;
			if (pClient)
				delete pClient;
		}
		catch(...)
		{
		}
	}

	_C1ClientMap.clear();

//	if (_R2Client)
//		delete _R2Client;
//	_R2Client = NULL;
}

Ice::Long NGODSessionGroup::checkAll()
{
	std::vector<NGODSessionGroup::Ptr> groupsToOntimer;
	::Ice::Long stampNow = now();
	::Ice::Long _nextWakeup = stampNow + 1000;

	{
		MutexGuard g(_lockGroups);

		for (SessionGroupMap::iterator iter = _groupMap.begin(); iter != _groupMap.end(); iter++)
		{
			try
			{
				NGODSessionGroup::Ptr group = iter->second;
				if (NULL == group)
					continue;

				if (stampNow - group->getLastSync() > group->getSyncInterval())
					groupsToOntimer.push_back(group);
				else
					_nextWakeup = (_nextWakeup > group->getSyncInterval()) ? group->getSyncInterval() : _nextWakeup;
			}
			catch(...)	{}
		}
	}

	for (std::vector<NGODSessionGroup::Ptr>::iterator iter2 = groupsToOntimer.begin(); iter2 != groupsToOntimer.end(); iter2++)
	{
		try
		{
			(*iter2)->OnTimer();
		}
		catch(...)	{}
	}

	groupsToOntimer.clear();
	return _nextWakeup;
}

}} // namespaces
