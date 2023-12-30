#include "StdAfx.h"
#include "AAAQuery.h"
#include "Surf_Tianshan.h"
#include "SystemUtils.h"
#include "TimeUtil.h"
#include "HttpClient.h"
#include "XMLPreferenceEx.h"
#include "ZQ_common_conf.h"
#include "strHelper.h"
#include "../GB_MOD2/MODDefines.h"
#include "urlstr.h"
using namespace ZQ::common;
const std::string XML_HEADER = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n";
#define AAAFmt(_C, _X) CLOGFMT(_C, "[%s][%s] " _X), aaaInfo.sessionID.c_str(), aaaInfo.ident.name.c_str()

extern ZQ::common::Log * PMHOlog;
#define MLOG (*PMHOlog)

namespace ZQTianShan {
	namespace Application{
		namespace MOD{
AAAQuery::AAAQuery(void)
{
	_ltransactionID = 0;
}

AAAQuery::~AAAQuery(void)
{
}
int AAAQuery::OnAuthorize(AAAInfo& aaaInfo, AEReturnData& aedata, ::TianShanIce::ValueMap& privData)
{
	MLOG(ZQ::common::Log::L_DEBUG, AAAFmt(AAAQuery, "Entry OnAuthorize() at endpoint [%s]"),aaaInfo.endpoint.c_str());
	Ice::Long lStart = ZQTianShan::now();

	int retCode = IAAA::AAAQUERYSUCCESS;
	aaaInfo.errorCode = IAAA::AAAQUERYSUCCESS;

	std::string strErrMsg ="";

	std::string locateRequestIP = "";
	int nport = 0;
	int recvTimeout = 20;//second

	std::string netid, senderId, receiverId, transactionID, contentIdRegex;
   
	TianShanIce::Properties::iterator itor;
	itor = aaaInfo.prop.find(PD_KEY_LocalBind);
	if(itor != aaaInfo.prop.end())
      locateRequestIP = itor->second;
    
	itor = aaaInfo.prop.find(PD_KEY_Port);
	if(itor != aaaInfo.prop.end())
		nport = atoi(itor->second.c_str());

	itor = aaaInfo.prop.find(PD_KEY_ReceiverID);
	if(itor != aaaInfo.prop.end())
		receiverId = itor->second;

	itor = aaaInfo.prop.find(PD_KEY_SMNAME);
	if(itor != aaaInfo.prop.end())
		senderId = itor->second;

	itor = aaaInfo.prop.find(PD_KEY_TimeOut);
	if(itor != aaaInfo.prop.end())
		recvTimeout = atoi(itor->second.c_str());

	char strTemp[65] = "";
	{
		ZQ::common::MutexGuard guard(_mutex);
		_ltransactionID++;
		sprintf(strTemp, "_%lld\0", _ltransactionID);	
	}
	transactionID =  aaaInfo.ident.name + strTemp;

    contentIdRegex = "$0";
	itor = aaaInfo.prop.find(PD_KEY_ContentID);
	if(itor != aaaInfo.prop.end())
		contentIdRegex = itor->second;
	
	//httpclient
	std::auto_ptr<ZQ::common::HttpClient>	pAutoHttpClient;
	ZQ::common::HttpClient* phttpclient = new ZQ::common::HttpClient();
	if(!phttpclient)
		return false; 
	pAutoHttpClient.reset(phttpclient); 
	pAutoHttpClient->init(ZQ::common::HttpClient::HTTP_IO_KEEPALIVE);
	pAutoHttpClient->setLog(&MLOG);
	pAutoHttpClient->setLocalHost(locateRequestIP, nport);

	pAutoHttpClient->setHeader(NULL,NULL);
	pAutoHttpClient->setHeader("Content-Type", "text/xml");

	ZQ::common::URLStr url(aaaInfo.endpoint.c_str());
//	url.setPath(URLKEY_AUTHORIZE);

	std::string urlstr  = url.generate() ;//+ URLKEY_AUTHORIZE;
	if (pAutoHttpClient->httpConnect(urlstr.c_str(), ZQ::common::HttpClient::HTTP_POST))
	{
		strErrMsg = "failed to connect to url " + urlstr + " with error: " + pAutoHttpClient->getErrorstr();
		retCode = IAAA::AUTHORFAILED;
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "OnAuthorize()[%s]"), strErrMsg.c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "AAAQuery", retCode, "%s", strErrMsg.c_str());
		return retCode;
	}

	std::string strAuthorRequest;
	try
	{
		std::string utctime;
		time_t currentTime;
		time(&currentTime);
		char buftime[65]="";
		TimeUtil::Time2Iso(currentTime, buftime, 64);

		char localTime[65]="";
        ZQ::common::TimeUtil::Iso2Local(buftime, localTime, sizeof(localTime) -1);

		utctime = localTime;

		std::ostringstream buf;
		buf << XML_HEADER ;
		buf << "<Message>\n";
		buf << "  <Header>\n" ;
		buf << "    <SenderID>" << senderId << "</SenderID>\n" ;
		buf << " 	<ReceiverID>"<< receiverId << "</ReceiverID>\n" ;
		buf << " 	<TransactionID>" << transactionID << "</TransactionID>\n" ;
		buf << " 	<Version>1.0</Version>\n" ;
		buf << " 	<Time>"<< utctime <<"</Time>\n" ;
		buf << " 	<OpCode>SM_AAA_S2_AUTHORIZE_REQ</OpCode>\n" ;
		buf << " 	<MsgType>REQ</MsgType>\n" ;
		buf << "  </Header>\n" ;
		buf << "  <Body>\n" ;
		buf << "    <AuthorizedRequestInfo \n";
		buf << " 	    sessionID=\"" << aaaInfo.sessionID << "\"\n";
		buf << " 	    userID=\"" << aaaInfo.userID << "\"\n";
		buf << " 	    entitlementCode=\"" << aaaInfo.entitlementCode << "\">\n";
		buf << " 	</AuthorizedRequestInfo>\n";
		buf << "  </Body>\n" ;
		buf << "</Message>\n" ;

		strAuthorRequest = buf.str();

	}
	catch(...)
	{
		strErrMsg = std::string("composing authorize request caught exception:") + pAutoHttpClient->getErrorstr();
		retCode = IAAA::AUTHORFAILED;
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "OnAuthorize()[%s]"), strErrMsg.c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "AAAQuery", retCode, "%s", strErrMsg.c_str());
		return retCode;
	}

	std::string strResponseConent;
	char buf[256]= "";
	sprintf(buf, AAAFmt(AAAQuery, "sending authorize request"));
	MLOG.hexDump(ZQ::common::Log::L_INFO, strAuthorRequest.c_str(), strAuthorRequest.size(), buf, true);

	if(pAutoHttpClient->httpSendContent(strAuthorRequest.c_str(), strAuthorRequest.size()))
	{
		strErrMsg = std::string("send authorize request with error:") + pAutoHttpClient->getErrorstr();
		retCode = IAAA::AUTHORFAILED;
		pAutoHttpClient->uninit();	
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "OnAuthorize()[%s]"), strErrMsg.c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "AAAQuery", retCode, "%s", strErrMsg.c_str());
		return retCode;
	}
	if (pAutoHttpClient->httpEndSend() )
	{
		strErrMsg = std::string("httpEndSend authorize request with error:") + pAutoHttpClient->getErrorstr();
		retCode = IAAA::AUTHORFAILED;
		pAutoHttpClient->uninit();	
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "OnAuthorize()[%s]"), strErrMsg.c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "AAAQuery", retCode, "%s", strErrMsg.c_str());
		return retCode;
	}

	pAutoHttpClient->setRecvTimeout(recvTimeout);
	if (pAutoHttpClient->httpBeginRecv())
	{
		strErrMsg = std::string("begin receive authorize request response with error:") + pAutoHttpClient->getErrorstr();
		retCode = IAAA::AUTHORFAILED;
		pAutoHttpClient->uninit();		
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "OnAuthorize()[%s]"), strErrMsg.c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "AAAQuery", retCode, "%s", strErrMsg.c_str());
		return retCode;
	}


	int status = pAutoHttpClient->getStatusCode();

	if(status != 200)
	{
		strErrMsg = std::string("send authorize request with error:") + pAutoHttpClient->getMsg();
		retCode = IAAA::AUTHORFAILED;
		pAutoHttpClient->uninit();	
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "OnAuthorize()[%s]"), strErrMsg.c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "AAAQuery", retCode, "%s", strErrMsg.c_str());
		return retCode;
	}


	std::string strRC = "";
	while(!pAutoHttpClient->isEOF())
	{
		strRC.clear();
		if(pAutoHttpClient->httpContinueRecv())
		{
			strErrMsg = std::string("continue receiver authorize request response with error:") + pAutoHttpClient->getErrorstr();
			retCode = IAAA::AUTHORFAILED;
			pAutoHttpClient->uninit();	
			MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "OnAuthorize()[%s]"), strErrMsg.c_str());
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "AAAQuery", retCode, "%s", strErrMsg.c_str());
			return retCode;
		}
		pAutoHttpClient->getContent(strRC);
		strResponseConent += strRC;
	}

	if ( pAutoHttpClient->httpEndRecv() )
	{
		strErrMsg = std::string("finished receiver authorize request response with error:")+ pAutoHttpClient->getErrorstr();
		retCode = IAAA::AUTHORFAILED;
		pAutoHttpClient->uninit();	
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "OnAuthorize()[%s]"), strErrMsg.c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "AAAQuery", retCode, "%s", strErrMsg.c_str());
		return retCode;
	}

	pAutoHttpClient->getContent(strRC);
	strResponseConent += strRC;
	pAutoHttpClient->uninit();

	sprintf(buf, AAAFmt(AAAQuery, "authorize response "));
	MLOG.hexDump(ZQ::common::Log::L_INFO, strResponseConent.c_str(), strResponseConent.size(), buf, true);

	ResponseInfo responseinfo;
	if(!parserAuthorResponse(strResponseConent, aaaInfo, responseinfo))
	{
		strErrMsg = std::string("failed to parser authorize request response");
		retCode = IAAA::AUTHORFAILED;
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "OnAuthorize()[%s]"), strErrMsg.c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "AAAQuery", retCode, "%s", strErrMsg.c_str());
		return retCode;
	}

	if(responseinfo.retCode != 0)
	{
		strErrMsg = std::string("authorize failed");
		retCode = IAAA::AUTHORFAILED;
		aaaInfo.errorCode  = responseinfo.retCode;
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "OnAuthorize()%s: %d  %s"), strErrMsg.c_str(), responseinfo.retCode, responseinfo.errormsg.c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "AAAQuery", retCode, "%d %s", responseinfo.retCode, responseinfo.errormsg.c_str());
		return retCode; 
	}
	aedata.aeList.clear();
	TianShanIce::StrValues::iterator itorplitem;
	for(itorplitem = responseinfo.playlistItems.begin(); itorplitem != responseinfo.playlistItems.end(); itorplitem++)
	{
		std::string& playlistitem = *itorplitem;
		TianShanIce::StrValues strplItem;
		strplItem = ZQ::common::stringHelper::split(playlistitem, ' ');

		if(strplItem.size() < 3)
			continue;
		AEInfo aeInfo;
		aeInfo.aeUID = strplItem[0];

		aeInfo.bandWidth = responseinfo.maxBitrate;
		aeInfo.cueIn = 0;
		aeInfo.cueOut = 0;
		aeInfo.attributes.clear();

		if(itorplitem == responseinfo.playlistItems.begin())
		{
			int npos = contentIdRegex.find("${0}");
			if(npos >= 0)
			{
				contentIdRegex.replace(npos,  4, strplItem[0]);
			}
           
			npos = contentIdRegex.find("${1}");
			if(npos >= 0)
			{
				contentIdRegex.replace(npos,  4, strplItem[1]);
			} 
			
			aeInfo.attributes[PD_KEY_ContentID] =  contentIdRegex;
		}

		for(int i = 1; i < strplItem.size(); i++)
		{
			if(strplItem[i] == "vod" ||strplItem[i] =="ad" || strplItem[i] == "npvr")
			{
			  if(strplItem[i] == "ad" )
                aeInfo.attributes[ADM_ISAD] = "1";
			}
			else if(strplItem[i].find_first_not_of("0123456789") < 0)
			{
				aeInfo.bandWidth = _atoi64(strplItem[i].c_str());	
			}
		   else if(strplItem[i].find_first_not_of("FRP") < 0)
		   {
              for(int j = 0; j < strplItem[i].size(); ++j)
			  {
				  char ch = strplItem[i][j];
				  if(strplItem[i][j] == 'F')
				  {
					  aeInfo.attributes[ADM_FORBIDFF] = "1";
				  }
				  else  if(strplItem[i][j] == 'R')
				  {
					  aeInfo.attributes[ADM_FORBIDREW] = "1";
				  }
				  else  if(strplItem[i][j] == 'P')
				  {
					  aeInfo.attributes[ADM_FORBIDPAUSE] = "1";
				  }
			  }
		   }
		   else 
		   { 
			   int npos = strplItem[i].find('-');
			   if(npos >= 0)
			   {
                  aeInfo.cueIn = atoi(strplItem[i].substr(0, npos).c_str()) * 1000;
				  aeInfo.cueOut = atoi(strplItem[i].substr(npos + 1).c_str()) * 1000;
			   }
		   }
		}
		//aeInfo.name = assetinfo.name;
		aeInfo.nasUrls.clear();
		aeInfo.volumeList.clear();
		aedata.aeList.push_back(aeInfo);
	}

	if(aedata.aeList.size() <1 )
	{
		strErrMsg = std::string("failed with error: no play list items");
		retCode = IAAA::AUTHORFAILED;
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "OnAuthorize() [%s]"), strErrMsg.c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "AAAQuery", retCode, "%s", strErrMsg.c_str());
		return retCode; 
	}
	MLOG(ZQ::common::Log::L_INFO, AAAFmt(AAAQuery, "finished authorize took %dms"), ZQTianShan::now() - lStart); 

    return retCode;
}
int AAAQuery::OnStatusNotice(AAAInfo& aaaInfo, const ::TianShanIce::Properties& prop)
{
	MLOG(ZQ::common::Log::L_DEBUG, AAAFmt(AAAQuery, "Entry OnStatusNotice() at endpoint [%s][%s]"),aaaInfo.endpoint.c_str(), aaaInfo.commmand.c_str());

	Ice::Long lStart = ZQTianShan::now();

	int retCode = IAAA::AAAQUERYSUCCESS;
	aaaInfo.errorCode = IAAA::AAAQUERYSUCCESS;

	std::string strErrMsg ="";

	std::string locateRequestIP = "";
	int nport = 0;
    int recvTimeout = 20;//second
	std::string netid, senderId, receiverId, transactionID;

	TianShanIce::Properties::iterator itor;
	itor = aaaInfo.prop.find(PD_KEY_LocalBind);
	if(itor != aaaInfo.prop.end())
		locateRequestIP = itor->second;

	itor = aaaInfo.prop.find(PD_KEY_Port);
	if(itor != aaaInfo.prop.end())
		nport = atoi(itor->second.c_str());

	itor = aaaInfo.prop.find(PD_KEY_ReceiverID);
	if(itor != aaaInfo.prop.end())
		receiverId = itor->second;

	itor = aaaInfo.prop.find(PD_KEY_SMNAME);
	if(itor != aaaInfo.prop.end())
		senderId = itor->second;

	itor = aaaInfo.prop.find(PD_KEY_TimeOut);
	if(itor != aaaInfo.prop.end())
		recvTimeout = atoi(itor->second.c_str());

	char strTemp[65] = "";
	{
		ZQ::common::MutexGuard guard(_mutex);
		_ltransactionID++;
		sprintf(strTemp, "_%lld\0", _ltransactionID);	
	}
	transactionID =  aaaInfo.ident.name + strTemp;

	//httpclient
	std::auto_ptr<ZQ::common::HttpClient>	pAutoHttpClient;
	ZQ::common::HttpClient* phttpclient = new ZQ::common::HttpClient();
	if(!phttpclient)
		return false; 
	pAutoHttpClient.reset(phttpclient); 
	pAutoHttpClient->init(ZQ::common::HttpClient::HTTP_IO_KEEPALIVE);
	pAutoHttpClient->setLog(&MLOG);
	pAutoHttpClient->setLocalHost(locateRequestIP, nport);

	pAutoHttpClient->setHeader(NULL,NULL);
	pAutoHttpClient->setHeader("Content-Type", "text/xml");

	ZQ::common::URLStr url(aaaInfo.endpoint.c_str());
//	url.setPath(URLKEY_STATUSNOTICE);

	std::string urlstr  = url.generate() ;//+ URLKEY_STATUSNOTICE;
	if (pAutoHttpClient->httpConnect(urlstr.c_str(), ZQ::common::HttpClient::HTTP_POST))
	{
		strErrMsg = "failed to connect to url " + urlstr + " with error: " + std::string(pAutoHttpClient->getErrorstr());
		retCode = IAAA::AUTHORFAILED;

		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "OnStatusNotice() at endpoint [%s][%s][%s]"),aaaInfo.endpoint.c_str(), aaaInfo.commmand.c_str(), strErrMsg.c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "AAAQuery", retCode, "%s", strErrMsg.c_str());
		return retCode;
	}

	std::string reason = "";
	if(aaaInfo.commmand == "Release")
		reason	= (!aaaInfo.tearDownReason.empty() ? aaaInfo.tearDownReason : aaaInfo.terminateReason);

	std::string strOnstatusRequest;
	try
	{
		std::string utctime;
		time_t currentTime;
		time(&currentTime);
		char buftime[65]="";
		TimeUtil::Time2Iso(currentTime, buftime, 64);
		
		char localTime[65]="";
		ZQ::common::TimeUtil::Iso2Local(buftime, localTime, sizeof(localTime) -1);
		utctime = localTime;

		std::ostringstream buf;
		buf << XML_HEADER ;
		buf << "<Message>\n";
		buf << "  <Header>\n" ;
		buf << "    <SenderID>" << senderId << "</SenderID>\n" ;
		buf << " 	<ReceiverID>"<< receiverId << "</ReceiverID>\n" ;
		buf << " 	<TransactionID>" << transactionID << "</TransactionID>\n" ;
		buf << " 	<Version>1.0</Version>\n" ;
		buf << " 	<Time> "<< utctime <<"</Time>\n" ;
		buf << " 	<OpCode>SM_AAA_S2_STATUSNOTICE_REQ</OpCode>\n" ;
		buf << " 	<MsgType>REQ</MsgType>\n" ;
		buf << " 	<ErrorMessage>" << reason<< "</ErrorMessage>\n" ;
		buf << "  </Header>\n" ;
		buf << "  <Body>\n" ;
		buf << "    <StatusNoticeInfo  \n";
		buf << " 	   command=\""  << aaaInfo.commmand    <<"\"\n";
		buf << "       timestamp=\""<< aaaInfo.timeStamp   << "\"\n";
		buf << " 	   sessionID=\"" << aaaInfo.sessionID  << "\"\n";
		buf << " 	   userID=\"" << aaaInfo.userID << "\"\n";
		buf << " 	   contentID=\"" << aaaInfo.contentId << "\"\n";

		// TODO: please verify the source of this stopNPT. Its output is in decimal sss.mmm
		//       if can identify, end-of-stream should be represented as -1
		char strStopNPT[128] = "";

		const float EPSINON = 0.00001;
		if ((aaaInfo.stopNPT - (-2) >= - EPSINON) && (aaaInfo.stopNPT - (-2) <= EPSINON))
			memset(strStopNPT, sizeof(strStopNPT), 0);
		else if((aaaInfo.stopNPT - (-1) >= - EPSINON) && (aaaInfo.stopNPT - (-1) <= EPSINON))
			memcpy(strStopNPT, "-1", sizeof("-1"));
		else
			snprintf(strStopNPT, sizeof(strStopNPT)-1, "%.3f", aaaInfo.stopNPT);
	 
		buf << " 	   stopNPT=\"" << strStopNPT<< "\"";
		  
		buf << ">\n";
		buf << " 	</StatusNoticeInfo> \n";
		buf << "  </Body>\n" ;
		buf << "</Message>\n" ;

		strOnstatusRequest = buf.str();
	}
	catch(...)
	{
		strErrMsg = std::string("composing on status notice caught exception:") + pAutoHttpClient->getErrorstr();
		retCode = IAAA::AUTHORFAILED;
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "OnStatusNotice() at endpoint [%s][%s][%s]"),aaaInfo.endpoint.c_str(), aaaInfo.commmand.c_str(), strErrMsg.c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "AAAQuery", retCode, "%s", strErrMsg.c_str());
		return retCode;
	}

	std::string strResponseConent;
	char buf[256]= "";
	sprintf(buf, AAAFmt(AAAQuery, "sending on status notice request"));
	MLOG.hexDump(ZQ::common::Log::L_INFO, strOnstatusRequest.c_str(), strOnstatusRequest.size(), buf, true);

	if(pAutoHttpClient->httpSendContent(strOnstatusRequest.c_str(), strOnstatusRequest.size()))
	{
		strErrMsg = std::string("send on status notice request with error:") + pAutoHttpClient->getErrorstr();
		retCode = IAAA::AUTHORFAILED;
		pAutoHttpClient->uninit();	
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "OnStatusNotice() at endpoint [%s][%s][%s]"),aaaInfo.endpoint.c_str(), aaaInfo.commmand.c_str(), strErrMsg.c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "AAAQuery", retCode, "%s", strErrMsg.c_str());
		return retCode;
	}
	if (pAutoHttpClient->httpEndSend() )
	{
		strErrMsg = std::string("httpEndSend on status notice request with error:") + pAutoHttpClient->getErrorstr();
		retCode = IAAA::AUTHORFAILED;
		pAutoHttpClient->uninit();		
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "AAAQuery", retCode, "%s", strErrMsg.c_str());
		return retCode;
	}

	pAutoHttpClient->setRecvTimeout(recvTimeout);
	if (pAutoHttpClient->httpBeginRecv())
	{
		strErrMsg = std::string("begin receive on status notice request response with error:") + pAutoHttpClient->getErrorstr();
		retCode = IAAA::AUTHORFAILED;
		pAutoHttpClient->uninit();	
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "OnStatusNotice() at endpoint [%s][%s][%s]"),aaaInfo.endpoint.c_str(), aaaInfo.commmand.c_str(), strErrMsg.c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "AAAQuery", retCode, "%s", strErrMsg.c_str());
		return retCode;
	}

	int status = pAutoHttpClient->getStatusCode();

	if(status != 200)
	{
		strErrMsg = std::string("send on status notice request with error:") + pAutoHttpClient->getMsg();
		retCode = IAAA::AUTHORFAILED;
		pAutoHttpClient->uninit();
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "OnStatusNotice() at endpoint [%s][%s][%s]"),aaaInfo.endpoint.c_str(), aaaInfo.commmand.c_str(), strErrMsg.c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "AAAQuery", retCode, "%s", strErrMsg.c_str());
		return retCode;
	}

	std::string strRC = "";
	while(!pAutoHttpClient->isEOF())
	{
		strRC.clear();
		if(pAutoHttpClient->httpContinueRecv())
		{
			strErrMsg = std::string("continue receiver on status notice request response with error:") + pAutoHttpClient->getErrorstr();
			retCode = IAAA::AUTHORFAILED;
			pAutoHttpClient->uninit();		
			MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "OnStatusNotice() at endpoint [%s][%s][%s]"),aaaInfo.endpoint.c_str(), aaaInfo.commmand.c_str(), strErrMsg.c_str());
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "AAAQuery", retCode, "%s", strErrMsg.c_str());
			return retCode;
		}
		pAutoHttpClient->getContent(strRC);
		strResponseConent += strRC;
	}

	if ( pAutoHttpClient->httpEndRecv() )
	{
		strErrMsg = std::string("finished receiver on status notice request response with error:")+ pAutoHttpClient->getErrorstr();
		retCode = IAAA::AUTHORFAILED;
		pAutoHttpClient->uninit();	
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "OnStatusNotice() at endpoint [%s][%s][%s]"),aaaInfo.endpoint.c_str(), aaaInfo.commmand.c_str(), strErrMsg.c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "AAAQuery", retCode, "%s", strErrMsg.c_str());
		return retCode;
	}

	pAutoHttpClient->getContent(strRC);
	strResponseConent += strRC;
	pAutoHttpClient->uninit();


	sprintf(buf, AAAFmt(AAAQuery, "status notice response "));
	MLOG.hexDump(ZQ::common::Log::L_INFO, strResponseConent.c_str(), strResponseConent.size(), buf, true);

	ResponseInfo responseinfo;
	if(!parserStatusNoticeResponse(strResponseConent, aaaInfo, responseinfo))
	{
		strErrMsg = std::string("failed to parser status notice request response");
		retCode = IAAA::AUTHORFAILED;
		pAutoHttpClient->uninit();	
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "OnStatusNotice() at endpoint [%s][%s][%s]"), aaaInfo.endpoint.c_str(), aaaInfo.commmand.c_str(), strErrMsg.c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "AAAQuery", retCode, "%s", strErrMsg.c_str());
		return retCode;
	}

	MLOG(ZQ::common::Log::L_INFO, AAAFmt(AAAQuery, "finished send status notice [%s]TransactionID[%s]Time[%s]ReturnCode[%d]errorMsg[%s]took %dms"), 
		aaaInfo.commmand.c_str(),responseinfo.transactionID.c_str(), responseinfo.time.c_str(), responseinfo.retCode, responseinfo.errormsg.c_str(),ZQTianShan::now() - lStart); 

	return retCode;
}
bool AAAQuery::parserAuthorResponse(std::string& strResponse, AAAInfo& aaaInfo, ResponseInfo& responseInfo)
{
	Ice::Long lStart =  ZQTianShan::now();

	char temp[513] = "";
	if (strResponse.size() < 1)
	{
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "parserAuthorResponse() Response message is NULL")); 
		return false;
	}

	int length =strResponse.size(); 
	ZQ::common::XMLPreferenceDocumentEx xmlDoc;
	if(!xmlDoc.read((void*)strResponse.c_str(), length, 1))//successful
	{
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "parserAuthorResponse() failed to init XMLdocument")); 
		return false;
	}
	ZQ::common::XMLPreferenceEx* pXMLRoot = xmlDoc.getRootPreference();
	if(NULL == pXMLRoot)
	{
		xmlDoc.clear();
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "parserAuthorResponse()failed to get root Preference")); 
		return false;
	}
	ZQ::common::XMLPreferenceEx* headpre = pXMLRoot->findSubPreference("Header");

	if(NULL == headpre)
	{
		pXMLRoot->free();
		xmlDoc.clear();
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "parserAuthorResponse()failed to get header Preference")); 
		return false;
	}

	ZQ::common::XMLPreferenceEx* returncodepre = headpre->findChild("ReturnCode");
	if(NULL == returncodepre)
	{
		pXMLRoot->free();
		xmlDoc.clear();
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "parserAuthorResponse()failed to get Header/ReturnCode child")); 
		return false;
	}
	memset(temp, 0, 513);
	returncodepre->getPreferenceText(temp, 512);
	responseInfo.retCode = atoi(temp);

	memset(temp, 0, 513);
	ZQ::common::XMLPreferenceEx* errormsgpre = headpre->findChild("ErrorMessage");
	if(NULL != errormsgpre)
	{
		memset(temp, 0, 513);
		errormsgpre->getPreferenceText(temp, 512);
		responseInfo.errormsg = temp;
	}

	if(responseInfo.retCode != 0)
	{
		pXMLRoot->free();
		xmlDoc.clear();
		return true;
	}

	ZQ::common::XMLPreferenceEx* transactionIdpre = headpre->findChild("TransactionID");
	if(NULL == transactionIdpre)
	{
		pXMLRoot->free();
		xmlDoc.clear();
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "parserAuthorResponse()failed to get Header/TransactionID child")); 
		return false;
	}
	memset(temp, 0, 513);
	transactionIdpre->getPreferenceText(temp, 512);
	responseInfo.transactionID = temp;

	ZQ::common::XMLPreferenceEx* timepre = headpre->findChild("Time");
	if(NULL == timepre)
	{
		pXMLRoot->free();
		xmlDoc.clear();
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "parserAuthorResponse()failed to get Header/Time child")); 
		return false;
	}
	memset(temp, 0, 513);
	timepre->getPreferenceText(temp, 512);
	responseInfo.time = temp;

	ZQ::common::XMLPreferenceEx* opCodepre = headpre->findChild("OpCode");
	if(NULL == opCodepre)
	{
		pXMLRoot->free();
		xmlDoc.clear();
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "parserAuthorResponse()failed to get Header/OpCode child")); 
		return false;
	}
	memset(temp, 0, 513);
	opCodepre->getPreferenceText(temp, 512);
	responseInfo.opCode = temp;

	ZQ::common::XMLPreferenceEx*msgTypepre = headpre->findChild("MsgType");
	if(NULL == msgTypepre)
	{
		pXMLRoot->free();
		xmlDoc.clear();
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "parserAuthorResponse()failed to get Header/MsgType child")); 
		return false;
	}
	memset(temp, 0, 513);
	msgTypepre->getPreferenceText(temp, 512);
	responseInfo.msgType = temp;

	ZQ::common::XMLPreferenceEx* bodypre = pXMLRoot->findSubPreference("Body");
	if(NULL == bodypre)
	{
		pXMLRoot->free();
		xmlDoc.clear();
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "parserAuthorResponse()failed to get Body Preference")); 
		return false;
	}

	ZQ::common::XMLPreferenceEx* authorpre = bodypre->findChild("AuthorizedResponseInfo");
	if(NULL == authorpre)
	{
		pXMLRoot->free();
		xmlDoc.clear();
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "parserAuthorResponse()failed to get Body/AuthorizedResponseInfo child")); 
		return false;
	}

	memset(temp, 0, 513);
	if(!authorpre->getAttributeValue("sessionID", temp))
	{
		pXMLRoot->free();
		xmlDoc.clear();
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "parserAuthorResponse()failed to get Body/AuthorizedResponseInfo/sessionID attribute")); 
		return false;
	}
	responseInfo.sessionId = temp;

	memset(temp, 0, 513);
	if(!authorpre->getAttributeValue("userID", temp))
	{
		pXMLRoot->free();
		xmlDoc.clear();
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "parserAuthorResponse()failed to get Body/AuthorizedResponseInfo/userID attribute")); 

		return false;
	}
	responseInfo.userId = temp;

	memset(temp, 0, 513);
	if(!authorpre->getAttributeValue("entitlementCode", temp))
	{
		MLOG(ZQ::common::Log::L_DEBUG, AAAFmt(AAAQuery, "parserAuthorResponse()missed Body/AuthorizedResponseInfo/entitlementCode attribute")); 
	}
	else
		responseInfo.entitlementCode = temp;

	memset(temp, 0, 513);
	if(!authorpre->getAttributeValue("breakpoint", temp))
	{
		MLOG(ZQ::common::Log::L_DEBUG, AAAFmt(AAAQuery, "parserAuthorResponse()missed Body/AuthorizedResponseInfo/breakpoint attribute")); 
	}
	else
	{
		sscanf(temp, "%.03f", &responseInfo.breakpoint);
	}

	ZQ::common::XMLPreferenceEx* maxbitratepre = authorpre->findChild("MaxBitrate");
	if(NULL == maxbitratepre)
	{
		pXMLRoot->free();
		xmlDoc.clear();
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "parserAuthorResponse()failed to get Body/MaxBitrate child")); 
		return false;
	}
	memset(temp, 0, 513);
	maxbitratepre->getPreferenceText(temp, 512);
	responseInfo.maxBitrate = _atoi64(temp);

	ZQ::common::XMLPreferenceEx* durationpre = authorpre->findChild("Duration");
	if(NULL == durationpre)
	{
		pXMLRoot->free();
		xmlDoc.clear();
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "parserAuthorResponse()failed to get Body/Duration child")); 
		return false;
	}
	memset(temp, 0, 513);
	durationpre->getPreferenceText(temp, 512);
	sscanf(temp, "%.03f", &responseInfo.duration);

	ZQ::common::XMLPreferenceEx* playTypepre = authorpre->findChild("PlayType");
	if(NULL == playTypepre)
	{
		pXMLRoot->free();
		xmlDoc.clear();
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "parserAuthorResponse()failed to get Body/PlayType child")); 
		return false;
	}
	memset(temp, 0, 513);
	playTypepre->getPreferenceText(temp, 512);
	responseInfo.playType = temp;

	ZQ::common::XMLPreferenceEx* playlistpre = authorpre->findSubPreference("Playlist");
	if(NULL == playlistpre)
	{
		pXMLRoot->free();
		xmlDoc.clear();
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "parserAuthorResponse()failed to get Body/AuthorizedResponseInfo/Playlist preference")); 
		return false;
	}
	ZQ::common::XMLPreferenceEx* playlistitempre = playlistpre->firstChild("PlaylistItem");
	while( NULL != playlistitempre)
	{
		memset(temp, 0, 513);
		playlistitempre->getPreferenceText(temp, 512);
		responseInfo.playlistItems.push_back(temp);

		playlistitempre = playlistpre->nextChild();
	}
/*	if(NULL == playlistitempre)
	{
		pXMLRoot->free();
		xmlDoc.clear();
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "parserAuthorResponse()failed to get Body/AuthorizedResponseInfo/Playlist/PlaylistItem child")); 
		return false;
	}
	memset(temp, 0, 513);
	playlistitempre->getPreferenceText(temp, 512);
	responseInfo.playlistItems.push_back(temp);

	while(playlistpre->hasNextChild())
	{
		playlistitempre = playlistpre->nextChild();
		memset(temp, 0, 513);
		playlistitempre->getPreferenceText(temp, 512);
		responseInfo.playlistItems.push_back(temp);
	}
*/
	pXMLRoot->free();
	xmlDoc.clear();
	MLOG(ZQ::common::Log::L_INFO,AAAFmt(AAAQuery, "parsed authorize response took %dms"), ZQTianShan::now() - lStart); 
	return true;
}
bool AAAQuery::parserStatusNoticeResponse(std::string& strResponse, AAAInfo& aaaInfo, ResponseInfo& responseInfo)
{
	Ice::Long lStart =  ZQTianShan::now();

	char temp[513] = "";
	if (strResponse.size() < 1)
	{
		return false;
	}

	int length =strResponse.size(); 
	ZQ::common::XMLPreferenceDocumentEx xmlDoc;
	if(!xmlDoc.read((void*)strResponse.c_str(), length, 1))//successful
	{
		return false;
	}
	ZQ::common::XMLPreferenceEx* pXMLRoot = xmlDoc.getRootPreference();
	if(NULL == pXMLRoot)
	{
		xmlDoc.clear();
		return false;
	}
	ZQ::common::XMLPreferenceEx* headpre = pXMLRoot->findSubPreference("Header");

	if(NULL == headpre)
	{
		pXMLRoot->free();
		xmlDoc.clear();
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "parserStatusNoticeResponse()failed to get Header Preference")); 
		return false;
	}

	ZQ::common::XMLPreferenceEx* transactionIdpre = headpre->findChild("TransactionID");
	if(NULL == transactionIdpre)
	{
		pXMLRoot->free();
		xmlDoc.clear();
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "parserStatusNoticeResponse() failed to get hHeader/TransactionID child")); 
		return false;
	}
	memset(temp, 0, 513);
	transactionIdpre->getPreferenceText(temp, 512);
	responseInfo.transactionID = temp;

	ZQ::common::XMLPreferenceEx* timepre = headpre->findChild("Time");
	if(NULL == timepre)
	{
		pXMLRoot->free();
		xmlDoc.clear();
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "parserStatusNoticeResponse()failed to get Header/Time child")); 
		return false;
	}
	memset(temp, 0, 513);
	timepre->getPreferenceText(temp, 512);
	responseInfo.time = temp;

	ZQ::common::XMLPreferenceEx* opCodepre = headpre->findChild("OpCode");
	if(NULL == opCodepre)
	{
		pXMLRoot->free();
		xmlDoc.clear();
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "parserStatusNoticeResponse()failed to get Header/OpCode child")); 
		return false;
	}
	memset(temp, 0, 513);
	opCodepre->getPreferenceText(temp, 512);
	responseInfo.opCode = temp;

	ZQ::common::XMLPreferenceEx*msgTypepre = headpre->findChild("MsgType");
	if(NULL == msgTypepre)
	{
		pXMLRoot->free();
		xmlDoc.clear();
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "parserStatusNoticeResponse()failed to get Header/MsgType child")); 
		return false;
	}
	memset(temp, 0, 513);
	msgTypepre->getPreferenceText(temp, 512);
	responseInfo.msgType = temp;

	ZQ::common::XMLPreferenceEx* returncodepre = headpre->findChild("ReturnCode");
	if(NULL == returncodepre)
	{
		pXMLRoot->free();
		xmlDoc.clear();
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "parserStatusNoticeResponse()failed to get Header/ReturnCode child")); 
		return false;
	}
	memset(temp, 0, 513);
	returncodepre->getPreferenceText(temp, 512);
	responseInfo.retCode = atoi(temp);

	ZQ::common::XMLPreferenceEx* errormsgpre = headpre->findChild("ErrorMessage");
	if(NULL == errormsgpre)
	{
		pXMLRoot->free();
		xmlDoc.clear();
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "parserStatusNoticeResponse()failed to get Header/ErrorMessage child")); 
		return false;
	}
	memset(temp, 0, 513);
	errormsgpre->getPreferenceText(temp, 512);
	responseInfo.errormsg = temp;

	pXMLRoot->free();
	xmlDoc.clear();
	MLOG(ZQ::common::Log::L_INFO,AAAFmt(AAAQuery, "parser status notice response took %dms"), ZQTianShan::now() - lStart); 
	return true;

}
}}}//end namespace