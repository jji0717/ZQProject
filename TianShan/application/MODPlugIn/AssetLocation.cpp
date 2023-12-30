#include "StdAfx.h"
#include "AssetLocation.h"
#include "SystemUtils.h"
#include "TimeUtil.h"
#include "HttpClient.h"
#include "XMLPreferenceEx.h"
#include "ZQ_common_conf.h"
#include "strHelper.h"
#include "../MOD2/MODDefines.h"
#include "urlstr.h"
using namespace ZQ::common;
const std::string XML_HEADER = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n";
#define AssetLocationFmt(_C, _X) CLOGFMT(_C, "[%s][%s] " _X), alinfo.onDemandSessionId.c_str(), alinfo.ident.name.c_str()

extern ZQ::common::Log * PMHOlog;
#define MLOG (*PMHOlog)

namespace ZQTianShan {
	namespace Application{
		namespace MOD{
AssetLocation::AssetLocation(void)
{
}

AssetLocation::~AssetLocation(void)
{
}
int  AssetLocation::getAssetLocation(AssetLocationInfo& alinfo, AEReturnData& aedata, ::TianShanIce::ValueMap& privData)
{
	MLOG(ZQ::common::Log::L_DEBUG, AssetLocationFmt(AssetLocation, "Entry getAssetLocation() at endpoint [%s]"),alinfo.endpoint.c_str());
	Ice::Long lStart = ZQTianShan::now();

	int retCode = IAssetLocation::ALSUCCESS;

	std::string strErrMsg ="";

	std::string locateRequestIP = "";
	int nport = 0;
	int recvTimeout = 20;//second
  
	TianShanIce::Properties::iterator itor;
	itor = alinfo.prop.find(PD_KEY_LocalBind);
	if(itor != alinfo.prop.end())
      locateRequestIP = itor->second;
    
	itor = alinfo.prop.find(PD_KEY_Port);
	if(itor != alinfo.prop.end())
		nport = atoi(itor->second.c_str());

	itor = alinfo.prop.find(PD_KEY_TimeOut);
	if(itor != alinfo.prop.end())
		recvTimeout = atoi(itor->second.c_str());

	
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

	ZQ::common::URLStr url(alinfo.endpoint.c_str());

	std::string urlstr  = url.generate() ;
	if (pAutoHttpClient->httpConnect(urlstr.c_str(), ZQ::common::HttpClient::HTTP_POST))
	{
		strErrMsg = "failed to connect to url " + urlstr + " with error: " + pAutoHttpClient->getErrorstr();
		retCode = IAssetLocation::INTERNAL;
		MLOG(ZQ::common::Log::L_ERROR, AssetLocationFmt(AssetLocation, "getAssetLocation()[%s]"), strErrMsg.c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "AssetLocation", retCode, "%s", strErrMsg.c_str());
		return retCode;
	}
/*
	<?xml version="1.0" encoding="utf-8"?>
		<LocateAssets ODSessionID=¡± be074250-cc5a-11d9-8cd5-0800200c9a66¡±>
		<Asset providerID=¡±comcast.com¡± assetID=¡±CokeAd1¡±/>
		<Asset providerID=¡±comcast.com¡± assetID=¡±flyers20041103¡±/>
		<Asset providerID=¡±comcast.com¡± assetID=¡±ComcastPromo1¡±/>
		<Filter type=¡±ODRM_Domain¡±>
		Detroit.GrossePointe.Hub1,
		Detroit.GrossePointe
		</Filter>
		</LocateAssets>
*/
	std::map<std::string, std::string>pidpaidToAEUID;
	std::string strALRequest;
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
		buf << "<LocateAssets ODSessionID=\"" << alinfo.onDemandSessionId << "\" >\n";
		for(int i = 0; i < aedata.aeList.size(); i++){
			std::string pid, paid;
			if(aedata.aeList[i].attributes.find("PAID") == aedata.aeList[i].attributes.end())
			{

			}
			paid = aedata.aeList[i].attributes["PAID"];

			if(aedata.aeList[i].attributes.find("PID") == aedata.aeList[i].attributes.end())
			{

			}
			pid = aedata.aeList[i].attributes["PID"];

			pidpaidToAEUID[aedata.aeList[i].aeUID] = paid + pid;

			buf << "  <Asset providerID=\""<< pid << "\"  assetID=\""<< paid << "\" />\n"  ;
		}
		//buf << "  <Filter type=\"ODRM_Domain\">\n" ;
		//buf << "  </Filter>\n" ;
		buf << "</LocateAssets>\n" ;

		strALRequest = buf.str();
	}
	catch(...)
	{
		strErrMsg = std::string("composing asset location request caught exception:") + pAutoHttpClient->getErrorstr();
		retCode = IAssetLocation::INTERNAL;
		MLOG(ZQ::common::Log::L_ERROR, AssetLocationFmt(AssetLocation, "getAssetLocation()[%s]"), strErrMsg.c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "AssetLocation", retCode, "%s", strErrMsg.c_str());
		return retCode;
	}

	std::string strResponseConent;
	char buf[256]= "";
	sprintf(buf,  AssetLocationFmt(AssetLocation, "sending get AssetLocation request"));
	MLOG.hexDump(ZQ::common::Log::L_INFO, strALRequest.c_str(), strALRequest.size(), buf, true);

	if(phttpclient->httpSendContent(strALRequest.c_str(), strALRequest.size()))
	{
		strErrMsg = std::string("send authorize request with error:") + pAutoHttpClient->getErrorstr();
		retCode = IAssetLocation::INTERNAL;
		phttpclient->uninit();	
		MLOG(ZQ::common::Log::L_ERROR,  AssetLocationFmt(AssetLocation, "getAssetLocation()[%s]"), strErrMsg.c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "AssetLocation", retCode, "%s", strErrMsg.c_str());
		return retCode;
	}
	if (pAutoHttpClient->httpEndSend() )
	{
		strErrMsg = std::string("httpEndSend authorize request with error:") + pAutoHttpClient->getErrorstr();
		retCode = IAssetLocation::INTERNAL;
		pAutoHttpClient->uninit();	
		MLOG(ZQ::common::Log::L_ERROR,  AssetLocationFmt(AssetLocation, "getAssetLocation()[%s]"), strErrMsg.c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "AssetLocation", retCode, "%s", strErrMsg.c_str());
		return retCode;
	}

	pAutoHttpClient->setRecvTimeout(recvTimeout);
	if (pAutoHttpClient->httpBeginRecv())
	{
		strErrMsg = std::string("begin receive authorize request response with error:") + pAutoHttpClient->getErrorstr();
		retCode = IAssetLocation::INTERNAL;
		pAutoHttpClient->uninit();		
		MLOG(ZQ::common::Log::L_ERROR, AssetLocationFmt(AssetLocation, "getAssetLocation()[%s]"), strErrMsg.c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "AssetLocation", retCode, "%s", strErrMsg.c_str());
		return retCode;
	}

	int status = pAutoHttpClient->getStatusCode();

	if(status != 200)
	{
		strErrMsg = std::string("send authorize request with error:") + pAutoHttpClient->getMsg();
		retCode = IAssetLocation::INTERNAL;
		pAutoHttpClient->uninit();	
		MLOG(ZQ::common::Log::L_ERROR, AssetLocationFmt(AssetLocation, "getAssetLocation()[%s]"), strErrMsg.c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "AssetLocation", retCode, "%s", strErrMsg.c_str());
		return retCode;
	}

	std::string strRC = "";
	while(!pAutoHttpClient->isEOF())
	{
		strRC.clear();
		if(pAutoHttpClient->httpContinueRecv())
		{
			strErrMsg = std::string("continue receiver authorize request response with error:") + pAutoHttpClient->getErrorstr();
			retCode = IAssetLocation::INTERNAL;
			pAutoHttpClient->uninit();	
			MLOG(ZQ::common::Log::L_ERROR, AssetLocationFmt(AssetLocation, "getAssetLocation()[%s]"), strErrMsg.c_str());
			ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "AssetLocation", retCode, "%s", strErrMsg.c_str());
			return retCode;
		}
		pAutoHttpClient->getContent(strRC);
		strResponseConent += strRC;
	}

	if ( pAutoHttpClient->httpEndRecv() )
	{
		strErrMsg = std::string("finished receiver AssetLocation request response with error:")+ pAutoHttpClient->getErrorstr();
		retCode = IAssetLocation::INTERNAL;
		pAutoHttpClient->uninit();	
		MLOG(ZQ::common::Log::L_ERROR,  AssetLocationFmt(AssetLocation, "getAssetLocation()[%s]"), strErrMsg.c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "AssetLocation", retCode, "%s", strErrMsg.c_str());
		return retCode;
	}

	pAutoHttpClient->getContent(strRC);
	strResponseConent += strRC;
	pAutoHttpClient->uninit();

	sprintf(buf,  AssetLocationFmt(AssetLocation, "getAssetLocation response "));
	MLOG.hexDump(ZQ::common::Log::L_INFO, strResponseConent.c_str(), strResponseConent.size(), buf, true);

	ResponseInfo responseinfo;
	if(!parserAssetLocationResponse(strResponseConent, alinfo, responseinfo))
	{
		strErrMsg = std::string("failed to parser authorize request response");
		retCode = IAssetLocation::INTERNAL;
		MLOG(ZQ::common::Log::L_ERROR, AssetLocationFmt(AssetLocation, "getAssetLocation()[%s]"), strErrMsg.c_str());
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(MLOG, "AssetLocation", retCode, "%s", strErrMsg.c_str());
		return retCode;
	}

	for(int i = 0; i < aedata.aeList.size(); i++)
	{
		std::string pid, paid;
		if(aedata.aeList[i].attributes.find("PAID") == aedata.aeList[i].attributes.end())
		{
			continue;
		}
		paid = aedata.aeList[i].attributes["PAID"];

		if(aedata.aeList[i].attributes.find("PID") == aedata.aeList[i].attributes.end())
		{
			continue;
		}
		pid = aedata.aeList[i].attributes["PID"];

		if(responseinfo.assetInfos.find(paid + pid) != responseinfo.assetInfos.end())
			aedata.aeList[i].volumeList = responseinfo.assetInfos[paid + pid].volumes;
	}
	MLOG(ZQ::common::Log::L_INFO,  AssetLocationFmt(AssetLocation, "finished getAssetLocation took %dms"), ZQTianShan::now() - lStart); 

    return retCode;
}
/*
<?xml version="1.0" encoding="utf-8"?>
<LocateAssetsResponse ODSessionID=¡± be074250-cc5a-11d9-8cd5-0800200c9a66¡±>
	<Asset ProviderID=¡±comcast.com¡± AssetID=¡±CokeAd1¡±>
		<Location fileLocation=¡±/vodContent/CokeAd1.Mpg¡±
		volumeName=¡±detroit.GrossePointe.volume1¡±/>
	</Asset>
	<Asset ProviderID=¡±comcast.com¡± AssetID=¡±flyers20041103¡±>
		<Location fileLocation=¡°/vodContent/flyers20041103¡±
			volumeName=¡±detroit.GrossePointe.volume2¡±/>
		<Location fileLocation=¡°/vodContent/flyers20041103¡±
			volumeName=¡±detroit.GrossePointe.volume1¡±/>
	</Asset>
	<Asset ProviderID=¡±comcast.com¡± AssetID=¡±ComcastPromo¡±>
	</Asset>
</LocateAssetsResponse>
*/

bool AssetLocation::parserAssetLocationResponse(const std::string& strResponse, AssetLocationInfo& alinfo, ResponseInfo& responseInfo)
{
	Ice::Long lStart =  ZQTianShan::now();

	char temp[513] = "";
	if (strResponse.size() < 1)
	{
		MLOG(ZQ::common::Log::L_ERROR, AssetLocationFmt(AssetLocation, "parserAssetLocationResponse() Response message is NULL")); 
		return false;
	}

	int length =strResponse.size(); 
	ZQ::common::XMLPreferenceDocumentEx xmlDoc;
	try
	{
		if(!xmlDoc.read((void*)strResponse.c_str(), length, 1))//successful
		{
			MLOG(ZQ::common::Log::L_ERROR, AssetLocationFmt(AssetLocation, "parserAssetLocationResponse() failed to init XMLdocument")); 
			return false;
		}
	}
	catch (ZQ::common::XMLException* e)
	{  
		MLOG(ZQ::common::Log::L_ERROR, AssetLocationFmt(AssetLocation, "parserAssetLocationResponse() failed to init XMLdocument caught exception: %s"), e->getString()); 
		return false;
	}	
	catch (...)
	{
		MLOG(ZQ::common::Log::L_ERROR, AssetLocationFmt(AssetLocation, "parserAssetLocationResponse() failed to init XMLdocument caught unknown exception: %d"), SYS::getLastErr()); 
		return false;
	}

	ZQ::common::XMLPreferenceEx* pXMLRoot = xmlDoc.getRootPreference();
	if(NULL == pXMLRoot)
	{
		xmlDoc.clear();
		MLOG(ZQ::common::Log::L_ERROR, AssetLocationFmt(AssetLocation, "parserAssetLocationResponse()failed to get root Preference")); 

		return false;
	}
	memset(temp, 0, 513);
	if(!pXMLRoot->getAttributeValue("ODSessionID",temp))
	{
		MLOG(ZQ::common::Log::L_INFO, AssetLocationFmt(AssetLocation,"parsed asset location response missed ODSessionID attribute"));
		pXMLRoot->free();
		xmlDoc.clear();
		return false;
	}
	responseInfo.onDemandSessionID = temp;

	ZQ::common::XMLPreferenceEx* assetItemsPre = pXMLRoot->firstChild("Asset");
	while( NULL != assetItemsPre)
	{
		AssetInfo assetInfo;
		memset(temp, 0, 513);
		if(!assetItemsPre->getAttributeValue("ProviderID",temp))
		{
			MLOG(ZQ::common::Log::L_INFO, AssetLocationFmt(AssetLocation, "parsed asset location response missed ProviderID attribute")); 
			pXMLRoot->free();
			xmlDoc.clear();
			return false;
		}
		assetInfo.pid = temp;

		memset(temp, 0, 513);
		if(!assetItemsPre->getAttributeValue("AssetID",temp))
		{
			MLOG(ZQ::common::Log::L_INFO, AssetLocationFmt(AssetLocation,"parsed asset location response missed AssetID attribute"));
			pXMLRoot->free();
			xmlDoc.clear();
			return false;
		}
		assetInfo.assetId = temp;

		ZQ::common::XMLPreferenceEx* locationPre = assetItemsPre->firstChild("Location");
		while(locationPre != NULL)
		{
			memset(temp, 0, 513);
			if(!locationPre->getAttributeValue("volumeName",temp))
			{
				break;
			}
			assetInfo.volumes.push_back(temp);

			locationPre = assetItemsPre->nextChild();	
		}

		assetItemsPre = pXMLRoot->nextChild();		
		responseInfo.assetInfos[assetInfo.assetId + assetInfo.pid] = assetInfo;
	}

	pXMLRoot->free();
	xmlDoc.clear();
	MLOG(ZQ::common::Log::L_INFO, AssetLocationFmt(AssetLocation, "parsed asset location response took %dms"), ZQTianShan::now() - lStart); 
	return true;
}
}}}//end namespace