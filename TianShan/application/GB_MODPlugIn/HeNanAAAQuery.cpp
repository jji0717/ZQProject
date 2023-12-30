#include "StdAfx.h"
#include "HeNanAAAQuery.h"
#include "SystemUtils.h"
#include "TimeUtil.h"
#include "HttpClient.h"
#include "XMLPreferenceEx.h"
#include "ZQ_common_conf.h"
#include "strHelper.h"
#include "../GB_MOD2/MODDefines.h"
#include "urlstr.h"
using namespace ZQ::common;
const std::string XML_HEADER = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>";

#define URI_PLAYLIST   "requestPlaylist"
#define URI_SESSIONSTART   "sessionStart"
#define URI_SESSIONSTOP    "sessionStop"
#define URI_SESSIONFAIL    "sessionFail"

#define AAAFmt(_C, _X) CLOGFMT(_C, "[%s][%s] " _X), aaaInfo.sessionID.c_str(), aaaInfo.ident.name.c_str()

extern ZQ::common::Log * PMHOlog;
#define MLOG (*PMHOlog)

namespace ZQTianShan {
	namespace Application{
		namespace MOD{
HeNanAAAQuery::HeNanAAAQuery(void)
{
	_ltransactionID = 0;
}

HeNanAAAQuery::~HeNanAAAQuery(void)
{
}
int HeNanAAAQuery::OnAuthorize(AAAInfo& aaaInfo, AEReturnData& aedata, ::TianShanIce::ValueMap& privData)
{
	MLOG(ZQ::common::Log::L_DEBUG, AAAFmt(HeNanAAAQuery, "Entry OnAuthorize() at endpoint [%s]"),aaaInfo.endpoint.c_str());
	Ice::Long lStart = ZQTianShan::now();

	int retCode = IAAA::AAAQUERYSUCCESS;
	aaaInfo.errorCode = IAAA::AAAQUERYSUCCESS;

	std::string strErrMsg ="";

	std::string locateRequestIP = "";
	int nport = 0;
	int recvTimeout = 20;//second

	std::string smname;
   
	TianShanIce::Properties::iterator itor;
	itor = aaaInfo.prop.find(PD_KEY_LocalBind);
	if(itor != aaaInfo.prop.end())
      locateRequestIP = itor->second;
    
	itor = aaaInfo.prop.find(PD_KEY_Port);
	if(itor != aaaInfo.prop.end())
		nport = atoi(itor->second.c_str());

	itor = aaaInfo.prop.find(PD_KEY_TimeOut);
	if(itor != aaaInfo.prop.end())
		recvTimeout = atoi(itor->second.c_str());

	itor = aaaInfo.prop.find(PD_KEY_SMNAME);
	if(itor != aaaInfo.prop.end())
		smname = itor->second;

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

	std::string tempURL = aaaInfo.endpoint;
	if(!tempURL.empty() && tempURL[tempURL.length() -1] != '/')
		tempURL+= "/";
	tempURL += URI_PLAYLIST;
    
	ZQ::common::URLStr url(tempURL.c_str());
	std::string urlstr  = url.generate() ;
	if (pAutoHttpClient->httpConnect(urlstr.c_str(), ZQ::common::HttpClient::HTTP_POST))
	{
		strErrMsg = "failed to connect to url " + urlstr + " with error: " + pAutoHttpClient->getErrorstr();
		retCode = IAAA::AUTHORFAILED;
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(HeNanAAAQuery, "OnAuthorize()[%s]"), strErrMsg.c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "HeNanAAAQuery", retCode, "%s", strErrMsg.c_str());
		return retCode;
	}

	std::string strAuthorRequest;
	try
	{
		/*
		<?xml version="1.0" encoding="UTF-8" standalone="yes"?> 
		< RequestPlaylist 
		SMName = ¡°sm.ngod.henancatv.com¡±
		SessionID=¡±[32-hex OSD ID]¡±
		DeviceID=¡±[MAC addr]¡±
		Locality=¡±25503¡±
		Usage=¡± start¡±
		PT=¡± [32-hex ID]¡±
		/>
    	*/
		std::string sessionId;
		for(int pos = 0; pos < aaaInfo.ident.name.length(); pos++)
		{
			if(aaaInfo.ident.name[pos] != '-')
				sessionId.push_back(aaaInfo.ident.name[pos]);
		}
		std::ostringstream buf;
		buf << XML_HEADER ;
		buf << "<RequestPlaylist \n";
		buf << "SMName=\"" << smname << "\" \n";
		buf << "SessionID=\"" << sessionId << "\" \n";
		buf << "DeviceID=\"" << aaaInfo.deviceId << "\" \n";
		buf << "Locality=\"" << aaaInfo.locality << "\" \n";
		buf << "Usage=\"" << aaaInfo.usage << "\" \n";
		buf << "PT=\"" << aaaInfo.entitlementCode << "\"\n";
		buf << "/>\n" ;

		strAuthorRequest = buf.str();
	}
	catch(...)
	{
		strErrMsg = std::string("composing authorize request caught exception:") + pAutoHttpClient->getErrorstr();
		retCode = IAAA::AUTHORFAILED;
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(HeNanAAAQuery, "OnAuthorize()[%s]"), strErrMsg.c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "HeNanAAAQuery", retCode, "%s", strErrMsg.c_str());
		return retCode;
	}

	std::string strResponseConent;
	char buf[256]= "";
	sprintf(buf, AAAFmt(HeNanAAAQuery, "sending authorize request"));
	MLOG.hexDump(ZQ::common::Log::L_INFO, strAuthorRequest.c_str(), strAuthorRequest.size(), buf, true);

	if (pAutoHttpClient->httpSendContent(strAuthorRequest.c_str(), strAuthorRequest.size()))
	{
		strErrMsg = std::string("send authorize request with error:") + pAutoHttpClient->getErrorstr();
		retCode = IAAA::AUTHORFAILED;
		pAutoHttpClient->uninit();	
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(HeNanAAAQuery, "OnAuthorize()[%s]"), strErrMsg.c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "HeNanAAAQuery", retCode, "%s", strErrMsg.c_str());
		return retCode;
	}
	if (pAutoHttpClient->httpEndSend() )
	{
		strErrMsg = std::string("httpEndSend authorize request with error:") + pAutoHttpClient->getErrorstr();
		retCode = IAAA::AUTHORFAILED;
		pAutoHttpClient->uninit();	
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(HeNanAAAQuery, "OnAuthorize()[%s]"), strErrMsg.c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "HeNanAAAQuery", retCode, "%s", strErrMsg.c_str());
		return retCode;
	}

	pAutoHttpClient->setRecvTimeout(recvTimeout);
	if (pAutoHttpClient->httpBeginRecv())
	{
		strErrMsg = std::string("begin receive authorize request response with error:") + pAutoHttpClient->getErrorstr();
		retCode = IAAA::AUTHORFAILED;
		pAutoHttpClient->uninit();		
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(HeNanAAAQuery, "OnAuthorize()[%s]"), strErrMsg.c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "HeNanAAAQuery", retCode, "%s", strErrMsg.c_str());
		return retCode;
	}


	int status = pAutoHttpClient->getStatusCode();

	if (status != 200)
	{
		strErrMsg = std::string("send authorize request with error:") + pAutoHttpClient->getMsg();
		retCode = IAAA::AUTHORFAILED;
		pAutoHttpClient->uninit();	
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(HeNanAAAQuery, "OnAuthorize()[%s: %d]"), strErrMsg.c_str(), status);
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "HeNanAAAQuery", retCode, "%s,%d", strErrMsg.c_str(), status);
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
			MLOG(ZQ::common::Log::L_ERROR, AAAFmt(HeNanAAAQuery, "OnAuthorize()[%s]"), strErrMsg.c_str());
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "HeNanAAAQuery", retCode, "%s", strErrMsg.c_str());
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
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(HeNanAAAQuery, "OnAuthorize()[%s]"), strErrMsg.c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "HeNanAAAQuery", retCode, "%s", strErrMsg.c_str());
		return retCode;
	}

	pAutoHttpClient->getContent(strRC);
	strResponseConent += strRC;
	pAutoHttpClient->uninit();

	sprintf(buf, AAAFmt(HeNanAAAQuery, "authorize response "));
	MLOG.hexDump(ZQ::common::Log::L_INFO, strResponseConent.c_str(), strResponseConent.size(), buf, true);

	ResponseInfo responseinfo;
	if(!parserAuthorResponse(strResponseConent, aaaInfo, responseinfo))
	{
		strErrMsg = std::string("failed to parser authorize request response");
		retCode = IAAA::AUTHORFAILED;
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(HeNanAAAQuery, "OnAuthorize()[%s]"), strErrMsg.c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "HeNanAAAQuery", retCode, "%s", strErrMsg.c_str());
		return retCode;
	}

	aedata.aeList.clear();
	contentRefs::iterator itorplitem;
	for(itorplitem = responseinfo.playlistItems.begin(); itorplitem != responseinfo.playlistItems.end(); itorplitem++)
	{
		contentRef& content = *itorplitem;
		AEInfo aeInfo;
		aeInfo.aeUID = content.assetID + "_" + content.providerID;
		aeInfo.bandWidth = responseinfo.bitrate;
		aeInfo.cueIn = atoi(content.startNPT.c_str()) * 1000;
		aeInfo.cueOut = atoi(content.endNPT.c_str()) * 1000;
		aeInfo.attributes.clear();

		//aeInfo.name = assetinfo.name;
		aeInfo.nasUrls.clear();
		aeInfo.volumeList.clear();
		aedata.aeList.push_back(aeInfo);
	}

	if(aedata.aeList.size() <1 )
	{
		strErrMsg = std::string("failed with error: no play list items");
		retCode = IAAA::AUTHORFAILED;
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(HeNanAAAQuery, "OnAuthorize() [%s]"), strErrMsg.c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "HeNanAAAQuery", retCode, "%s", strErrMsg.c_str());
		return retCode; 
	}
	TianShanIce::Variant varPL;
	varPL.bRange = false;
	varPL.type = TianShanIce::vtStrings;
	varPL.strs.push_back(responseinfo.playlistId);
	MAPSET(TianShanIce::ValueMap, privData, PD_KEY_PLAYLISTID, varPL);
	MLOG(ZQ::common::Log::L_INFO, AAAFmt(HeNanAAAQuery, "finished authorize took %dms"), ZQTianShan::now() - lStart); 

    return retCode;
}
int HeNanAAAQuery::OnStatusNotice(AAAInfo& aaaInfo, const ::TianShanIce::Properties& prop)
{
	MLOG(ZQ::common::Log::L_DEBUG, AAAFmt(HeNanAAAQuery, "Entry OnStatusNotice() at endpoint [%s][%s]"),aaaInfo.endpoint.c_str(), aaaInfo.commmand.c_str());

	Ice::Long lStart = ZQTianShan::now();

	int retCode = IAAA::AAAQUERYSUCCESS;
	aaaInfo.errorCode = IAAA::AAAQUERYSUCCESS;

	std::string strErrMsg ="";

	std::string locateRequestIP = "";
	int nport = 0;
    int recvTimeout = 20;//second
	std::string smname;

	TianShanIce::Properties::iterator itor;
	itor = aaaInfo.prop.find(PD_KEY_LocalBind);
	if(itor != aaaInfo.prop.end())
		locateRequestIP = itor->second;

	itor = aaaInfo.prop.find(PD_KEY_Port);
	if(itor != aaaInfo.prop.end())
		nport = atoi(itor->second.c_str());

	itor = aaaInfo.prop.find(PD_KEY_SMNAME);
	if(itor != aaaInfo.prop.end())
		smname = itor->second;

	itor = aaaInfo.prop.find(PD_KEY_TimeOut);
	if(itor != aaaInfo.prop.end())
		recvTimeout = atoi(itor->second.c_str());

	std::string sessionId;
	for(int pos = 0; pos < aaaInfo.ident.name.length(); pos++)
	{
		if(aaaInfo.ident.name[pos] != '-')
			sessionId.push_back(aaaInfo.ident.name[pos]);
	}
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


	std::string tempURL = aaaInfo.endpoint;
	if(!tempURL.empty() && tempURL[tempURL.length() -1] != '/')
		tempURL+= "/";

	if(aaaInfo.commmand == "Setup")
		tempURL += URI_SESSIONSTART;
	else if (aaaInfo.commmand == "Release" && aaaInfo.terminateReason.empty())
		tempURL += URI_SESSIONSTOP;
	else 
		tempURL += URI_SESSIONFAIL;

	ZQ::common::URLStr url(tempURL.c_str());
//	url.setPath(URLKEY_STATUSNOTICE);

	std::string urlstr  = url.generate() ;//+ URLKEY_STATUSNOTICE;
	if (pAutoHttpClient->httpConnect(urlstr.c_str(), ZQ::common::HttpClient::HTTP_POST))
	{
		strErrMsg = "failed to connect to url " + urlstr + " with error: " + std::string(pAutoHttpClient->getErrorstr());
		retCode = IAAA::AUTHORFAILED;

		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(HeNanAAAQuery, "OnStatusNotice() at endpoint [%s][%s][%s]"),tempURL.c_str(), aaaInfo.commmand.c_str(), strErrMsg.c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "HeNanAAAQuery", retCode, "%s", strErrMsg.c_str());
		return retCode;
	}

	std::string strOnstatusRequest;
	try
	{
		std::string utctime;
		time_t currentTime;
		time(&currentTime);
		char buftime[65]="";
		TimeUtil::Time2Iso(currentTime, buftime, 64);
		
/*		char localTime[65]="";
		ZQ::common::TimeUtil::Iso2Local(buftime, localTime, sizeof(localTime) -1);
		utctime = localTime;
*/
		utctime = buftime;
		std::ostringstream buf;
		if(aaaInfo.commmand == "Setup")
		{
			/*
			<?xml version="1.0" encoding="UTF-8" standalone="yes"?> 
			< SessionStart
			SMName =¡±255¡±
			SessionID=¡±[32-hex OSD ID]¡±
			playlistID =¡± [32-hex ID]¡±
			deviceID=¡±[MAC addr]¡±//the stb that name the selection
			locality=¡±25503¡±// VOD ServerID
			startDateTime=¡±2005-09-09T10:30:305Z¡±
			/>
			*/
			buf << XML_HEADER ;
			buf << "<SessionStart \n";
			buf << "SMName=\"" << smname << "\" \n";
			buf << "SessionID=\"" << sessionId << "\" \n";
			buf << "PlaylistID=\"" << aaaInfo.playListId << "\" \n";
			buf << "DeviceID=\"" << aaaInfo.deviceId << "\" \n";
			buf << "Locality=\"" << aaaInfo.locality << "\" \n";
			buf << "StartDateTime=\"" << utctime << "\"\n";
			buf << "/>\n" ;
		}
		else if(aaaInfo.commmand == "Release")
		{
			/*
			<?xml version="1.0" encoding="UTF-8" standalone="yes"?> 
			< SessionStop	
			SRM_ID =¡±255¡±
			purchaseToken=¡± [32-hex ID]¡±
			SessionID=¡±1c5771c0-c84e-11dd-99af-001c2312554e¡±
			stopNPT=¡°467189¡°  //ms
			stopAssetIndex=¡°3¡°
			deviceID=¡±[MAC addr]¡±//the stb that name the selection
			locality=¡±25503¡±// 
			stopDateTime=¡±2005-09-09T10:30:305Z¡±
			/>
			*/
			if(aaaInfo.terminateReason.empty())
			{
				// TODO: please verify the source of this stopNPT. Its output is in decimal sss.mmm
				//       if can identify, end-of-stream should be represented as -1
				char strStopNPT[128] = "";

				const float EPSINON = 0.00001;
				if ((aaaInfo.stopNPT - (-2) >= - EPSINON) && (aaaInfo.stopNPT - (-2) <= EPSINON))
					memset(strStopNPT, sizeof(strStopNPT), 0);
				else if((aaaInfo.stopNPT - (-1) >= - EPSINON) && (aaaInfo.stopNPT - (-1) <= EPSINON))
					memcpy(strStopNPT, "-1", sizeof("-1"));
				//			else
				//				snprintf(strStopNPT, sizeof(strStopNPT)-1, "%.3f", aaaInfo.stopNPT);

				buf << XML_HEADER ;
				buf << "<SessionStop \n";
				buf << "SMName=\"" << smname << "\" \n";
				buf << "PlaylistID=\"" << aaaInfo.playListId<< "\" \n";
				buf << "SessionID=\"" << sessionId << "\" \n";
				buf << "StopNPT=\"" << strStopNPT << "\" \n";
				buf << "StopAssetIndex=\"" << aaaInfo.stopAssetIndex << "\" \n";
				buf << "DeviceID=\"" << aaaInfo.deviceId << "\" \n";
				buf << "Locality=\"" << aaaInfo.locality << "\" \n";
				buf << "StopDateTime=\"" << utctime << "\"\n";
				buf << "/>\n" ;
			}
			else
			{
				/*
				<?xml version="1.0" encoding="UTF-8" standalone="yes"?> 
				<SessionFail
				SMName =¡±255¡±
				SessionID=¡±1c5771c0-c84e-11dd-99af-001c2312554e¡±
				DeviceID=¡±[MAC addr]¡±//the stb that name the selection
				ReasonCode=¡±400¡±
				ReasonMessage=¡±Not Found¡±
				/>
				*/
				int reasonCode = 0;
				std::string reasonMsg =  aaaInfo.terminateReason;
				int npos = aaaInfo.terminateReason.find(' ');
				if(npos > 0)
				{
					reasonCode = atoi((aaaInfo.terminateReason.substr(0, npos)).c_str());
					reasonMsg = aaaInfo.terminateReason.substr(npos +1);
				}
				buf << "<SessionFail \n";
				buf << "SMName=\"" << smname << "\" \n";
				buf << "SessionID=\"" << sessionId << "\" \n";
				buf << "DeviceID=\"" << aaaInfo.deviceId << "\" \n";
				buf << "ReasonCode=\"" << reasonCode << "\" \n";
				buf << "ReasonMessage=\"" << reasonMsg << "\"\n";
				buf << "/>\n" ;
			}
		}
		  
		strOnstatusRequest = buf.str();
	}
	catch(...)
	{
		strErrMsg = std::string("composing on status notice caught exception:") + pAutoHttpClient->getErrorstr();
		retCode = IAAA::AUTHORFAILED;
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(HeNanAAAQuery, "OnStatusNotice() at endpoint [%s][%s][%s]"),tempURL.c_str(), aaaInfo.commmand.c_str(), strErrMsg.c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "HeNanAAAQuery", retCode, "%s", strErrMsg.c_str());
		return retCode;
	}

	std::string strResponseConent;
	char buf[256]= "";
	sprintf(buf, AAAFmt(HeNanAAAQuery, "sending on status notice request"));
	MLOG.hexDump(ZQ::common::Log::L_INFO, strOnstatusRequest.c_str(), strOnstatusRequest.size(), buf, true);

	if(pAutoHttpClient->httpSendContent(strOnstatusRequest.c_str(), strOnstatusRequest.size()))
	{
		strErrMsg = std::string("send on status notice request with error:") + pAutoHttpClient->getErrorstr();
		retCode = IAAA::AUTHORFAILED;
		pAutoHttpClient->uninit();	
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(HeNanAAAQuery, "OnStatusNotice() at endpoint [%s][%s][%s]"),tempURL.c_str(), aaaInfo.commmand.c_str(), strErrMsg.c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "HeNanAAAQuery", retCode, "%s", strErrMsg.c_str());
		return retCode;
	}
	if (pAutoHttpClient->httpEndSend() )
	{
		strErrMsg = std::string("httpEndSend on status notice request with error:") + pAutoHttpClient->getErrorstr();
		retCode = IAAA::AUTHORFAILED;
		pAutoHttpClient->uninit();		
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "HeNanAAAQuery", retCode, "%s", strErrMsg.c_str());
		return retCode;
	}

	pAutoHttpClient->setRecvTimeout(recvTimeout);
	if (pAutoHttpClient->httpBeginRecv())
	{
		strErrMsg = std::string("begin receive on status notice request response with error:") + pAutoHttpClient->getErrorstr();
		retCode = IAAA::AUTHORFAILED;
		pAutoHttpClient->uninit();	
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(HeNanAAAQuery, "OnStatusNotice() at endpoint [%s][%s][%s]"),tempURL.c_str(), aaaInfo.commmand.c_str(), strErrMsg.c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "HeNanAAAQuery", retCode, "%s", strErrMsg.c_str());
		return retCode;
	}

	int status = pAutoHttpClient->getStatusCode();

	if(status != 200)
	{
		strErrMsg = std::string("send on status notice request with error:") + pAutoHttpClient->getMsg();
		retCode = IAAA::AUTHORFAILED;
		pAutoHttpClient->uninit();
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(HeNanAAAQuery, "command[%s][%s:%d]"),aaaInfo.commmand.c_str(), strErrMsg.c_str(), status);
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "HeNanAAAQuery", retCode, "%s:%d", strErrMsg.c_str(), status);
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
			MLOG(ZQ::common::Log::L_ERROR, AAAFmt(HeNanAAAQuery, "OnStatusNotice() at endpoint [%s][%s][%s]"),tempURL.c_str(), aaaInfo.commmand.c_str(), strErrMsg.c_str());
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "HeNanAAAQuery", retCode, "%s", strErrMsg.c_str());
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
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(HeNanAAAQuery, "OnStatusNotice() at endpoint [%s][%s][%s]"),tempURL.c_str(), aaaInfo.commmand.c_str(), strErrMsg.c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "HeNanAAAQuery", retCode, "%s", strErrMsg.c_str());
		return retCode;
	}

	pAutoHttpClient->getContent(strRC);
	strResponseConent += strRC;
	pAutoHttpClient->uninit();

	MLOG(ZQ::common::Log::L_INFO, AAAFmt(HeNanAAAQuery, "finished send status notice [%s]took %dms"), 
		aaaInfo.commmand.c_str(), ZQTianShan::now() - lStart); 

	return retCode;
}
bool HeNanAAAQuery::parserAuthorResponse(std::string& strResponse, AAAInfo& aaaInfo, ResponseInfo& responseInfo)
{
	Ice::Long lStart =  ZQTianShan::now();

	char temp[513] = "";
	if (strResponse.size() < 1)
	{
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(HeNanAAAQuery, "parserAuthorResponse() Response message is NULL")); 
		return false;
	}

	int length =strResponse.size(); 
	ZQ::common::XMLPreferenceDocumentEx xmlDoc;
	if(!xmlDoc.read((void*)strResponse.c_str(), length, 1))//successful
	{
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(HeNanAAAQuery, "parserAuthorResponse() failed to init XMLdocument")); 
		return false;
	}

	/*
	<?xml version="1.0" encoding="UTF-8" standalone="yes"?> 
	<PhysicalPlaylist  playlistID=¡± [32-hex ID]¡± bitRate=¡±3750000¡±  startIndex =¡±1¡± startNPT =¡±0.0¡±>
	<ContentRef  providerID=¡±hncatv¡±  
	assetID =¡± hnca1234567890123456¡±
	startNPT=¡±0.0¡±   endNpt=¡±¡± />
	<ContentRef  providerID=¡± hncatv¡± 
	assetID =¡± hnca1234567890123457¡±  
	startNPT=¡±0.0¡±   endNpt=¡±¡± />
	</PhysicalPlaylist>
	*/
	ZQ::common::XMLPreferenceEx* pXMLRoot = xmlDoc.getRootPreference();
	if(NULL == pXMLRoot)
	{
		xmlDoc.clear();
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(HeNanAAAQuery, "parserAuthorResponse()failed to get root Preference")); 
		return false;
	}

	bool bret = pXMLRoot->getPreferenceName(temp, 513);
	if(!bret || stricmp(temp, "PhysicalPlaylist") != 0)
	{
		pXMLRoot->free();
		xmlDoc.clear();
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(HeNanAAAQuery, "parserAuthorResponse()failed to get PhysicalPlaylist Preference")); 
		return false;
	}

	memset(temp, 0, 513);
	if(!pXMLRoot->getAttributeValue("playlistID", temp))
	{
		pXMLRoot->free();
		xmlDoc.clear();
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "parserAuthorResponse()failed to getPhysicalPlaylist/playlistID attribute")); 
		return false;
	}
	responseInfo.playlistId = temp;

	memset(temp, 0, 513);
	if(!pXMLRoot->getAttributeValue("bitRate", temp))
	{
		pXMLRoot->free();
		xmlDoc.clear();
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "parserAuthorResponse()failed to getPhysicalPlaylist/bitRate attribute")); 
		return false;
	}
	responseInfo.bitrate = atoi(temp);


	memset(temp, 0, 513);
	if(!pXMLRoot->getAttributeValue("startIndex", temp))
	{
		pXMLRoot->free();
		xmlDoc.clear();
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "parserAuthorResponse()failed to getPhysicalPlaylist/startIndex attribute")); 
		return false;
	}
	responseInfo.startIndex = atoi(temp);

	memset(temp, 0, 513);
	if(!pXMLRoot->getAttributeValue("startNPT", temp))
	{
		pXMLRoot->free();
		xmlDoc.clear();
		MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "parserAuthorResponse()failed to getPhysicalPlaylist/startNPT attribute")); 
		return false;
	}
	responseInfo.startNPT = std::string(temp);

	ZQ::common::XMLPreferenceEx* playlistitempre = pXMLRoot->firstChild("ContentRef");

	while(NULL != playlistitempre)
	{
        contentRef content;
		memset(temp, 0, 513);
		if(!playlistitempre->getAttributeValue("providerID", temp))
		{
			pXMLRoot->free();
			xmlDoc.clear();
			MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "parserAuthorResponse()failed to getPhysicalPlaylist/ContentRef/providerIDf attribute")); 
			return false;
		}
        content.providerID = temp;

		memset(temp, 0, 513);
		if(!playlistitempre->getAttributeValue("assetID", temp))
		{
			pXMLRoot->free();
			xmlDoc.clear();
			MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "parserAuthorResponse()failed to getPhysicalPlaylist/ContentRef/providerIDf attribute")); 
			return false;
		}
		content.assetID = temp;

		memset(temp, 0, 513);
		if(!playlistitempre->getAttributeValue("startNPT", temp))
		{
			pXMLRoot->free();
			xmlDoc.clear();
			MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "parserAuthorResponse()failed to getPhysicalPlaylist/ContentRef/startNPT attribute")); 
			return false;
		}
		content.startNPT = temp;

		memset(temp, 0, 513);
		if(!playlistitempre->getAttributeValue("endNpt", temp))
		{
			pXMLRoot->free();
			xmlDoc.clear();
			MLOG(ZQ::common::Log::L_ERROR, AAAFmt(AAAQuery, "parserAuthorResponse()failed to getPhysicalPlaylist/ContentRef/endNpt attribute")); 
			return false;
		}
		content.endNPT = temp;
        responseInfo.playlistItems.push_back(content);
/*		if(pXMLRoot->hasNextChild())
			playlistitempre = pXMLRoot->nextChild();
		else
			playlistitempre = NULL;*/

		playlistitempre = pXMLRoot->nextChild();
	}

	pXMLRoot->free();
	xmlDoc.clear();
	MLOG(ZQ::common::Log::L_INFO,AAAFmt(HeNanAAAQuery, "parsed authorize response took %dms"), ZQTianShan::now() - lStart); 
	return true;
}
}}}//end namespace