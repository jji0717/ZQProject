#include "MRTClient.h"
#include "TimeUtil.h"
namespace ZQMRT
{ 
	MRTClient::MRTClient(ZQ::common::Log& log): _log(log)
	{
	}

	MRTClient::~MRTClient(void)
	{
	}
	void MRTClient::logSoapErrorMsg(const soapMRT& soapMRTClient, std::string& errorMsg)
	{
		if(soapMRTClient.soap->error)
		{ 
			const char **s;
			if (!*soap_faultcode(soapMRTClient.soap))
				soap_set_fault(soapMRTClient.soap);

			char* soapError;
			s = soap_faultdetail(soapMRTClient.soap);
			if (s && *s)
			{	
				soapError = (char*)*s;
			}
			char buf[2048]="";
			snprintf(buf, sizeof(buf) -1, "SOAP Error: SocketErrNo=[%d] ErrorCode=[%d] FaultCode=[%s] FaultString=[%s] Detail Error: [%s]", 
				soapMRTClient.soap->errnum, soapMRTClient.soap->error, *soap_faultcode(soapMRTClient.soap), *soap_faultstring(soapMRTClient.soap), soapError);
			errorMsg = buf;
		}
		else
		{
			errorMsg = "unknown soap error";
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(MRTClient, "unknown soap error"));
		}
		_log.flush();
	}
	bool MRTClient::setup(const MRTEndpointInfo& mrtEndpintInfo, const std::string& AssetName, const std::string& destIp, int destPort, int bitrate, const std::string& srmSessionId,
		std::string& mrtSessinId, std::string& errorMessage)
	{
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(MRTClient, "setup() enter"));

		int64 lstart = ZQ::common::now();
		try
		{
			glog(ZQ::common::Log::L_DEBUG,  CLOGFMT(MRTClient, "[%s]setup() setup mrt stream with url[%s], AssetId[%s], destIp[%s],destPort[%d]"),
				srmSessionId.c_str(),mrtEndpintInfo.mrtEndpoint.c_str(), AssetName.c_str(), destIp.c_str(), destPort);

			soapMRT soapMRTClient;

			soapMRTClient.endpoint = mrtEndpintInfo.mrtEndpoint.c_str();

			soapMRTClient.soap->connect_timeout = mrtEndpintInfo.connectTimeout;
			soapMRTClient.soap->send_timeout = mrtEndpintInfo.sendTimeout;
			soapMRTClient.soap->recv_timeout = mrtEndpintInfo.receiverTimeout;

			ZQ2__setupInfo setupInfo;
			setupInfo.asset = (char*)AssetName.c_str();
			setupInfo.resource = new ZQ2__map();
			setupInfo.resource->size = 0;
			setupInfo.params = new ZQ2__map();
			//add destHost info
			{
				ZQ2__pair *pair = new ZQ2__pair();
				pair->key= "destHost";
				pair->value = (char*)destIp.c_str();
				setupInfo.resource->ptr.push_back(pair);
				setupInfo.resource->size++;
			}

			char buf[65];
			//add destPort
			{	
				memset(buf, 0, sizeof(buf));
				itoa(destPort, buf, 10);
				ZQ2__pair *pair = new ZQ2__pair();
				pair->key= "destPort";
				pair->value = buf;
				setupInfo.resource->ptr.push_back(pair);
				setupInfo.resource->size++;
			}
			//add bitrate info
			{	
				memset(buf, 0, sizeof(buf));
				itoa(bitrate, buf, 10);
				ZQ2__pair *pair = new ZQ2__pair();
				pair->key= "bitrate";
				pair->value = buf;
				setupInfo.resource->ptr.push_back(pair);
				setupInfo.resource->size++;
			}

			ZQ2__setupResponse setupResponse;
			if(soapMRTClient.ZQ2__setup(&setupInfo, setupResponse) == SOAP_OK  && setupResponse.ret == true)
			{
				mrtSessinId = setupResponse.sessionId;
			}
			else
			{
				// soap level error
				logSoapErrorMsg(soapMRTClient, errorMessage);
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(MRTClient, "[%s]setup mrt stream with error[%s]"),srmSessionId.c_str(), errorMessage.c_str());
				return false;
			}
			glog(ZQ::common::Log::L_INFO, CLOGFMT(MRTClient, "[%s]setup mrt stream with url[%s], AssetId[%s], destIp[%s],destPort[%d],SessionId[%s] success took %dms"),
				srmSessionId.c_str(), mrtEndpintInfo.mrtEndpoint.c_str(), AssetName.c_str(), destIp.c_str(), destPort, mrtSessinId.c_str(), (int)(ZQ::common::now() - lstart));
		}
		catch (...)
		{
			errorMessage = "failed to setup mrt stream caught unknown exception";
			glog(ZQ::common::Log::L_ERROR,  CLOGFMT(MRTClient, "[%s]failed to setup mrt stream caught unknown exception"), srmSessionId.c_str());
			return false;
		}
		return true;
	}
	bool MRTClient::getStatus(const MRTEndpointInfo& mrtEndpintInfo, const std::string& mrtSessinId,const std::string& srmSessionId, std::string& errorMessage)
	{
		glog(ZQ::common::Log::L_DEBUG,  CLOGFMT(MRTClient, "[%s]getStatus() enter, mrt endpoint[%s] mrtSessionId[%s]"),
												srmSessionId.c_str(), mrtEndpintInfo.mrtEndpoint.c_str(),mrtSessinId.c_str());

		int64  lstart = ZQ::common::now();
		soapMRT soapMRTClient;

		soapMRTClient.endpoint = mrtEndpintInfo.mrtEndpoint.c_str();
		soapMRTClient.soap->connect_timeout = mrtEndpintInfo.connectTimeout;
		soapMRTClient.soap->send_timeout = mrtEndpintInfo.sendTimeout;
		soapMRTClient.soap->recv_timeout = mrtEndpintInfo.receiverTimeout;

		bool bret = false;
		ZQ2__getStatusResponse getStatusRes;
		if(soapMRTClient.ZQ2__getStatus((char*)mrtSessinId.c_str(), getStatusRes) != SOAP_OK)
		{
			// soap level error
			logSoapErrorMsg(soapMRTClient, errorMessage);

			glog(ZQ::common::Log::L_ERROR, CLOGFMT(MRTClient, "[%s] failed to get mrt stream statusfrom mrt server[%s] with sessionId[%s]  errormsg[%s] took %dms "),
				srmSessionId.c_str(),soapMRTClient.endpoint, mrtSessinId.c_str(), errorMessage.c_str(), (int)(ZQ::common::now() - lstart));

			return false;
		}
		glog(ZQ::common::Log::L_INFO, CLOGFMT(MRTClient, "[%s]get mrt stream status [%s] retCode[%d] from mrt server[%s] with sessionId[%s] took %dms "),
			srmSessionId.c_str(), getStatusRes.state.c_str() , getStatusRes.ret, soapMRTClient.endpoint, mrtSessinId.c_str(), (int)(ZQ::common::now() - lstart));

		return getStatusRes.ret;
	}
	bool MRTClient::teardown(const MRTEndpointInfo& mrtEndpintInfo, const std::string& mrtSessinId, const std::string& srmSessionId, std::string& errorMessage)
	{
		glog(ZQ::common::Log::L_DEBUG,  CLOGFMT(MRTClient, "[%s]teardown() enter, mrt endpoint[%s] mrtSessionId[%s]"),
											srmSessionId.c_str(), mrtEndpintInfo.mrtEndpoint.c_str(),mrtSessinId.c_str());

		soapMRT soapMRTClient;
		soapMRTClient.endpoint = mrtEndpintInfo.mrtEndpoint.c_str();
		soapMRTClient.soap->connect_timeout = mrtEndpintInfo.connectTimeout;
		soapMRTClient.soap->send_timeout = mrtEndpintInfo.sendTimeout;
		soapMRTClient.soap->recv_timeout = mrtEndpintInfo.receiverTimeout;

		int64 lstart = ZQ::common::now();
		bool bret = true;
		if(soapMRTClient.ZQ2__teardown((char*)mrtSessinId.c_str(), bret) != SOAP_OK  ||  bret == false)
		{
			// soap level error
			logSoapErrorMsg(soapMRTClient, errorMessage);
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(MRTClient, "[%s]teardown mrt stream[%s] with error[%s]"), srmSessionId.c_str(), mrtSessinId.c_str(), errorMessage.c_str());
		}
		else
			glog(ZQ::common::Log::L_INFO, CLOGFMT(MRTClient, "[%s]teardown mrt stream [%s]successfully took %dms"), srmSessionId.c_str(), mrtSessinId.c_str(), (int)(ZQ::common::now() - lstart));
		return bret;
	}
}