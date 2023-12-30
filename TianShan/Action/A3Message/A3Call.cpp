#include "A3Call.h"
#include "A3HttpReq.h"
#include "strHelper.h"
#include "TianShanDefines.h"
#include "EventRuleEngine.h"
/*
Action& A3Call::operator() (Context& ctx, const Properties& input, Properties& output)
{
	ctx.statusCode = aSucceedQuit;

	//verify concerned subtype
	Properties::const_iterator iter = input.find("SubType");
	Properties::const_iterator md_iter = ctx.metaData.find("Event.SubType");
	if(iter != input.end() && iter->second != "")
	{
		if(md_iter != ctx.metaData.end() && md_iter->second != iter->second)
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3Call, "[%s] Not concerned subtype[%s]"), md_iter->second.c_str(), iter->second.c_str());
			return *this;
		}
	}

	std::string ContentInterfaceMode;
	std::string sourceType;
	iter = input.find("ContentInterfaceMode");
	if(iter != input.end())
		ContentInterfaceMode = iter->second;
	iter = input.find("sourceType");
	if(iter != input.end())
		sourceType = iter->second;

	// step 1. GetContentInfo
	std::string _providerID;
	std::string _assetID;
	md_iter = ctx.metaData.find("Event.ProviderId");
	if(md_iter != ctx.metaData.end())
		_providerID = md_iter->second;
	md_iter = ctx.metaData.find("Event.ProviderAssetId");
	if(md_iter != ctx.metaData.end())
		_assetID = md_iter->second;
	if(_assetID.empty() || _providerID.empty())
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3Call, "No AssetID[%s] or ProviderID[%s] given"), _assetID.c_str(), _providerID.c_str());
		return *this;
	}

	std::string _volName;
	iter = input.find("VolumeName");
	if(iter != input.end())
		_volName = iter->second;
	if(_volName.empty())
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3Call, "No VolumeName[%s] given"), _volName.c_str());
		return *this;
	}

	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(A3Call, "quering GetContentInfo of volume[%s]"), _volName.c_str());

	A3Request::MessageCtx msgCInfo;
	msgCInfo.params["providerID"]	= _providerID;
	msgCInfo.params["assetID"]		= _assetID;
	msgCInfo.params["volumeName"]	= _volName;

	// step 1.1 GetContentInfo query
	std::string A3NationalUrl;
	std::string A3RegionalUrl;
	if(input.find("NationalIP") != input.end())
		A3NationalUrl = input.find("NationalIP")->second + ":";
	if(input.find("NationalPort") != input.end())
		A3NationalUrl += input.find("NationalPort")->second;
	if(input.find("NationalPath") != input.end())
	A3NationalUrl += "/" + input.find("NationalPath")->second;

	if(input.find("RegionalIP") != input.end())
		A3RegionalUrl = input.find("RegionalIP")->second + ":";
	if(input.find("RegionalPort") != input.end())
		A3RegionalUrl += input.find("RegionalPort")->second;
	if(input.find("RegionalPath") != input.end())
		A3RegionalUrl += "/" + input.find("RegionalPath")->second;

	A3Request A3Req(A3RegionalUrl, &_log);
	int errorCode = A3Req.request(A3Request::A3_GetContentInfo, msgCInfo);
	if( errorCode < 200 || errorCode >= 300)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3Call, "GetContentInfo for volume[%s] failed: %s"), _volName.c_str(), A3Req.getStatusMessage().c_str());
		if(errorCode == 404)
		{
			// step 2. ExposeContent
			std::string url;
			A3Request::MessageCtx msgData;

			msgData.params["assetID"] = _assetID;
			msgData.params["providerID"] = _providerID;
			msgData.params["volumeName"] = _volName;

			size_t pos = msgData.params["volumeName"].length() -1;
			if (msgData.params["volumeName"][pos] == FNSEPC)
				msgData.params["volumeName"] = msgData.params["volumeName"].substr(0, pos);

			std::string transferBitRate;
			std::string transferProtocol;
			iter = input.find("transferProtocol");
			if(iter != input.end())
				transferProtocol = iter->second;
			iter = input.find("transferBitRate");
			if(iter != input.end())
				transferBitRate = iter->second;
			msgData.params["transferBitRate"] = transferBitRate;
			msgData.params["protocol"] = transferProtocol;

			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(A3Call, "getExportURL() querying SS[%s] for PID[%s] PAID[%s]"), A3NationalUrl.c_str(), msgData.params["providerID"].c_str(), msgData.params["assetID"].c_str());

			A3Request req(A3NationalUrl, &_log);
			int errorCode = req.request(A3Request::A3_ExposeContent, msgData);	
			std::string statusMsg = req.getStatusMessage();
			url = msgData.params["URL"];
			transferBitRate = msgData.params["transferBitRate"];
			std::string userName = msgData.params["userName"];
			std::string password = msgData.params["password"];

			if (errorCode >=200 && errorCode < 300) //ok, fill the attribute
			{
				_log(ZQ::common::Log::L_INFO, CLOGFMT(A3Call, "getExportURL() succeeded: %s"), statusMsg.c_str());

				// step 3. TransferContent
				char stampBuf[64] = {0};
				std::string startTimeUTC = ZQTianShan::TimeToUTC(ZQTianShan::now(), stampBuf, sizeof(stampBuf));
				memset(stampBuf,0,sizeof(stampBuf));
				::Ice::Long interval = 36000000;
				std::string stopTimeUTC = ZQTianShan::TimeToUTC(ZQTianShan::now() + interval, stampBuf, sizeof(stampBuf));

				A3Request::MessageCtx msgData;
				msgData.params["assetID"] = _assetID;
				msgData.params["providerID"] = _providerID;

				std::string strvn = _volName;
//				std::string strvn = "SEAC00005/$";
				size_t sL = strvn.length()-1;
				if(strvn[sL] == FNSEPC)
					strvn[sL] = '\0';

				msgData.params["volumeName"] = strvn;

				{
					char buf[1024] = {0};
					snprintf(buf, sizeof(buf) -2, "http://%s:%s/", input.find("NationalIP")->second.c_str(), input.find("NationalPort")->second.c_str());
					msgData.params["responseURL"] = buf;
				}

				//source URL parse
				msgData.params["sourceURL"] = url;
				msgData.params["userName"] = userName;
				msgData.params["password"] = password;
				//test
// 				msgData.params["sourceURL"] = "ftp://10.50.12.22:14149/170201_20100104102056_2575.recrpt";
// 				msgData.params["userName"] = "administrator";
// 				msgData.params["password"] = "itv";
				_log(ZQ::common::Log::L_DEBUG, CLOGFMT(A3Call, "submitProvision() parsed sourceUrl[%s]: url[%s] user[%s] passwd[%s]"), url.c_str(), msgData.params["sourceURL"].c_str(), msgData.params["userName"].c_str(), msgData.params["password"].c_str());

				msgData.params["captureStart"] = startTimeUTC;
				msgData.params["captureEnd"] = stopTimeUTC;
				msgData.params["transferBitRate"] = transferBitRate;

				if (0 == ContentInterfaceMode.compare("SeaChange"))
				{
					::TianShanIce::Properties props;
					// processing SeaChange's customized Content metadata: usefileset, cscontenttype and cscontenttype
					if (::std::string::npos == sourceType.find(":"))
						props["cscontenttype"] = sourceType;
					else
					{
						props["usefileset"] = "TRUE";
						::TianShanIce::StrValues strValue = ::ZQ::common::stringHelper::split(sourceType, ':');
						if (strValue.size() == 2)
						{
							if (0 == strValue[1].compare("VVX"))
								props["cscontenttype"] = "MPEG2TS";
							else if (0 == strValue[1].compare("VV2"))
								props["cscontenttype"] = "H264";
						}
					}
					msgData.table.push_back(props);
				}


				A3Request req(A3RegionalUrl, &_log);
				int errorCode = req.request(A3Request::A3_TransferContent, msgData);
				std::string statusMsg = req.getStatusMessage();

				if (errorCode >=200 && errorCode< 300) //ok
				{
					_log(ZQ::common::Log::L_INFO, CLOGFMT(A3Call, "submitProvision() provideID[%s] assetID[%s] volumeName[%s] captureStart[%s] captureEnd[%s] transferBitRate[%s] sourceURL[%s] responseURL[%s] ,provisioning submitted [%s]"), 
						msgData.params["providerID"].c_str(), msgData.params["assetID"].c_str(), msgData.params["volumeName"].c_str(), msgData.params["captureStart"].c_str(), msgData.params["captureEnd"].c_str(), msgData.params["transferBitRate"].c_str(),
						msgData.params["sourceURL"].c_str(), msgData.params["responseURL"].c_str(), statusMsg.c_str());
					return *this;
				}

				_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3Call, "provision failed: %s for PID[%s] PAID[%s]"), statusMsg.c_str(), _providerID.c_str(), _assetID.c_str());
			}
			else
			{
				_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3Call, "getExportURL() failed: %s for PID[%s] PAID[%s]"), statusMsg.c_str(), _providerID.c_str(), _assetID.c_str());
			}
		}
	}
	else
	{
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(A3Call, "Content exist, quit"));
		return *this;
	}

	return *this;
}
*/
Action& A3GetContentInfo::operator() (Context& ctx, const Properties& input, Properties& output)
{
	ctx.statusCode = aFailedQuit;

	//verify concerned subtype
	Properties::const_iterator input_iter = input.find("SubType");
	Properties::const_iterator md_iter = ctx.metaData.find("Event.SubType");
	if(input_iter != input.end() && input_iter->second != "")
	{
		if(md_iter != ctx.metaData.end() && md_iter->second != input_iter->second)
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3GetContentInfo, "[%s] Not concerned subtype[%s]"), md_iter->second.c_str(), input_iter->second.c_str());	
			return *this;
		}
	}

	// GetContentInfo
	std::string _providerID;
	std::string _assetID;
	md_iter = ctx.metaData.find("Event.ProviderId");
	if(md_iter != ctx.metaData.end())
		_providerID = md_iter->second;
	md_iter = ctx.metaData.find("Event.ProviderAssetId");
	if(md_iter != ctx.metaData.end())
		_assetID = md_iter->second;

	std::string _volName;
	input_iter = input.find("VolumeName");
	if(input_iter != input.end())
		_volName = input_iter->second;
	if(_volName.empty())
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3GetContentInfo, "No VolumeName[%s] given"), _volName.c_str());
		return *this;
	}

	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(A3GetContentInfo, "quering GetContentInfo of volume[%s]"), _volName.c_str());

	A3Request::MessageCtx msgCInfo;
	msgCInfo.params["providerID"]	= _providerID;
	msgCInfo.params["assetID"]		= _assetID;
	msgCInfo.params["volumeName"]	= _volName;

	// GetContentInfo query
	std::string A3RegionalUrl;
	if(input.find("RegionalIP") != input.end())
		A3RegionalUrl = input.find("RegionalIP")->second + ":";
	if(input.find("RegionalPort") != input.end())
		A3RegionalUrl += input.find("RegionalPort")->second;
	if(input.find("RegionalPath") != input.end())
		A3RegionalUrl += "/" + input.find("RegionalPath")->second;

	A3Request A3Req(A3RegionalUrl, &_log);
	int errorCode = A3Req.request(A3Request::A3_GetContentInfo, msgCInfo);
	if( errorCode < 200 || errorCode >= 300)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3GetContentInfo, "GetContentInfo for volume[%s] failed: %s"), _volName.c_str(), A3Req.getStatusMessage().c_str());
		if(errorCode == 404)
		{
			ctx.statusCode = aSucceed;
			STL_MAPSET(Properties, output, "volume", _volName);
			return *this;
		}
	}
	return *this;
}

Action& A3ExposeContent::operator() (Context& ctx, const Properties& input, Properties& output)
{
	ctx.statusCode = aFailedQuit;
	// ExposeContent
	std::string _providerID;
	std::string _assetID;
	Properties::const_iterator md_iter = ctx.metaData.find("Event.ProviderId");
	if(md_iter != ctx.metaData.end())
		_providerID = md_iter->second;
	md_iter = ctx.metaData.find("Event.ProviderAssetId");
	if(md_iter != ctx.metaData.end())
		_assetID = md_iter->second;
	if(_assetID.empty() || _providerID.empty())
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3ExposeContent, "No AssetID[%s] or ProviderID[%s] given"), _assetID.c_str(), _providerID.c_str());
		return *this;
	}

	std::string _volName;
	Properties::const_iterator input_iter = input.find("VolumeName");
	if(input_iter != input.end())
		_volName = input_iter->second;
	if(_volName.empty())
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3ExposeContent, "No VolumeName[%s] given"), _volName.c_str());
		return *this;
	}

	std::string url;
	A3Request::MessageCtx msgData;
	msgData.params["assetID"] = _assetID;
	msgData.params["providerID"] = _providerID;
	msgData.params["volumeName"] = _volName;

	size_t pos = msgData.params["volumeName"].length() -1;
	if (msgData.params["volumeName"][pos] == FNSEPC)
		msgData.params["volumeName"] = msgData.params["volumeName"].substr(0, pos);

	std::string transferBitRate;
	std::string transferProtocol;
	input_iter = input.find("transferProtocol");
	if(input_iter != input.end())
		transferProtocol = input_iter->second;
	input_iter = input.find("transferBitRate");
	if(input_iter != input.end())
		transferBitRate = input_iter->second;
	msgData.params["transferBitRate"] = transferBitRate;
	msgData.params["protocol"] = transferProtocol;


	std::string A3NationalUrl;
	if(input.find("NationalIP") != input.end())
		A3NationalUrl = input.find("NationalIP")->second + ":";
	if(input.find("NationalPort") != input.end())
		A3NationalUrl += input.find("NationalPort")->second;
	if(input.find("NationalPath") != input.end())
		A3NationalUrl += "/" + input.find("NationalPath")->second;

	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(A3ExposeContent, "getExportURL() querying SS[%s] for PID[%s] PAID[%s]"), A3NationalUrl.c_str(), msgData.params["providerID"].c_str(), msgData.params["assetID"].c_str());

	A3Request req(A3NationalUrl, &_log);
	int errorCode = req.request(A3Request::A3_ExposeContent, msgData);	
	std::string statusMsg = req.getStatusMessage();
	url = msgData.params["URL"];
	transferBitRate = msgData.params["transferBitRate"];
	std::string userName = msgData.params["userName"];
	std::string password = msgData.params["password"];

	if (errorCode >=200 && errorCode < 300) //ok, fill the attribute
	{
		ctx.statusCode = aSucceed;
		STL_MAPSET(Properties, output, "url", url);
		STL_MAPSET(Properties, output, "userName", userName);
		STL_MAPSET(Properties, output, "password", password);
		STL_MAPSET(Properties, output, "transferBitRate", transferBitRate);
		_log(ZQ::common::Log::L_INFO, CLOGFMT(A3ExposeContent, "getExportURL() succeeded: %s"), statusMsg.c_str());
	}
	else
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3ExposeContent, "getExportURL() failed: %s for PID[%s] PAID[%s]"), statusMsg.c_str(), _providerID.c_str(), _assetID.c_str());
	}
	return *this;
}

Action& A3TransferContent::operator() (Context& ctx, const Properties& input, Properties& output)
{
	ctx.statusCode = aFailedQuit;
	// TransferContent
	std::string _providerID;
	std::string _assetID;
	Properties::const_iterator md_iter = ctx.metaData.find("Event.ProviderId");
	if(md_iter != ctx.metaData.end())
		_providerID = md_iter->second;
	md_iter = ctx.metaData.find("Event.ProviderAssetId");
	if(md_iter != ctx.metaData.end())
		_assetID = md_iter->second;
	if(_assetID.empty() || _providerID.empty())
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3TransferContent, "No AssetID[%s] or ProviderID[%s] given"), _assetID.c_str(), _providerID.c_str());
		return *this;
	}

	std::string _volName;
	Properties::const_iterator input_iter = input.find("VolumeName");
	if(input_iter != input.end())
		_volName = input_iter->second;
	if(_volName.empty())
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3TransferContent, "No VolumeName[%s] given"), _volName.c_str());
		return *this;
	}

	char stampBuf[64] = {0};
	std::string startTimeUTC = ZQTianShan::TimeToUTC(ZQTianShan::now(), stampBuf, sizeof(stampBuf));
	memset(stampBuf,0,sizeof(stampBuf));
	::Ice::Long interval = 36000000;
	std::string stopTimeUTC = ZQTianShan::TimeToUTC(ZQTianShan::now() + interval, stampBuf, sizeof(stampBuf));

	A3Request::MessageCtx msgData;
	msgData.params["assetID"] = _assetID;
	msgData.params["providerID"] = _providerID;

	std::string strvn = _volName;
	//				std::string strvn = "SEAC00005/$";
	size_t sL = strvn.length()-1;
	if(strvn[sL] == FNSEPC)
		strvn[sL] = '\0';

	msgData.params["volumeName"] = strvn;

	//source URL parse
	md_iter = ctx.metaData.find("url");
	if(md_iter != ctx.metaData.end())
		msgData.params["sourceURL"] = md_iter->second;

	md_iter = ctx.metaData.find("userName");
	if(md_iter != ctx.metaData.end())
		msgData.params["userName"] = md_iter->second;

	md_iter = ctx.metaData.find("password");
	if(md_iter != ctx.metaData.end())
		msgData.params["password"] = md_iter->second;
	// 				msgData.params["sourceURL"] = "ftp://10.50.12.22:14149/170201_20100104102056_2575.recrpt";
	// 				msgData.params["userName"] = "administrator";
	// 				msgData.params["password"] = "itv";

	std::string responseURL;
	input_iter = input.find("responseURL");
	if(input_iter == input.end())
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3TransferContent, "No responseURL given"));
		return *this;
	}
	msgData.params["responseURL"] = input_iter->second;

	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(A3TransferContent, "submitProvision() : url[%s] user[%s] passwd[%s]"), msgData.params["sourceURL"].c_str(), msgData.params["userName"].c_str(), msgData.params["password"].c_str());

	msgData.params["captureStart"] = startTimeUTC;
	msgData.params["captureEnd"] = stopTimeUTC;

	md_iter = ctx.metaData.find("transferBitRate");
	if(md_iter != ctx.metaData.end())
		msgData.params["transferBitRate"] = md_iter->second;

	std::string ContentInterfaceMode;
	std::string sourceType;
	input_iter = input.find("ContentInterfaceMode");
	if(input_iter != input.end())
		ContentInterfaceMode = input_iter->second;
	input_iter = input.find("sourceType");
	if(input_iter != input.end())
		sourceType = input_iter->second;

	if (0 == ContentInterfaceMode.compare("SeaChange"))
	{
		::TianShanIce::Properties props;
		// processing SeaChange's customized Content metadata: usefileset, cscontenttype and cscontenttype
		if (::std::string::npos == sourceType.find(":"))
			props["cscontenttype"] = sourceType;
		else
		{
			props["usefileset"] = "TRUE";
			::TianShanIce::StrValues strValue = ::ZQ::common::stringHelper::split(sourceType, ':');
			if (strValue.size() == 2)
			{
				if (0 == strValue[1].compare("VVX"))
					props["cscontenttype"] = "MPEG2TS";
				else if (0 == strValue[1].compare("VV2"))
					props["cscontenttype"] = "H264";
			}
		}
		msgData.table.push_back(props);
	}

	std::string A3RegionalUrl;
	if(input.find("RegionalIP") != input.end())
		A3RegionalUrl = input.find("RegionalIP")->second + ":";
	if(input.find("RegionalPort") != input.end())
		A3RegionalUrl += input.find("RegionalPort")->second;
	if(input.find("RegionalPath") != input.end())
		A3RegionalUrl += "/" + input.find("RegionalPath")->second;

	A3Request req(A3RegionalUrl, &_log);
	int errorCode = req.request(A3Request::A3_TransferContent, msgData);
	std::string statusMsg = req.getStatusMessage();

	if (errorCode >=200 && errorCode< 300) //ok
	{
		ctx.statusCode = aSucceedQuit;
		_log(ZQ::common::Log::L_INFO, CLOGFMT(A3TransferContent, "submitProvision() provideID[%s] assetID[%s] volumeName[%s] captureStart[%s] captureEnd[%s] transferBitRate[%s] sourceURL[%s] responseURL[%s] ,provisioning submitted [%s]"), 
			msgData.params["providerID"].c_str(), msgData.params["assetID"].c_str(), msgData.params["volumeName"].c_str(), msgData.params["captureStart"].c_str(), msgData.params["captureEnd"].c_str(), msgData.params["transferBitRate"].c_str(),
			msgData.params["sourceURL"].c_str(), msgData.params["responseURL"].c_str(), statusMsg.c_str());
	}
	else
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3TransferContent, "provision failed: %s for PID[%s] PAID[%s]"), statusMsg.c_str(), _providerID.c_str(), _assetID.c_str());
	return *this;
}

Action& A3DeleteContent::operator() (Context& ctx, const Properties& input, Properties& output)
{
	ctx.statusCode = aFailedQuit;
/*
	EventRuleEngine* pEventRuleEngine = dynamic_cast<EventRuleEngine*>(&_engine);
	if(!pEventRuleEngine)
	{
		return *this;
	}
*/
	std::string _providerID;
	std::string _assetID;
	Properties::const_iterator md_iter = ctx.metaData.find("Event.ProviderId");
	if(md_iter != ctx.metaData.end())
		_providerID = md_iter->second;
	md_iter = ctx.metaData.find("Event.ProviderAssetId");
	if(md_iter != ctx.metaData.end())
		_assetID = md_iter->second;
	if(_assetID.empty() || _providerID.empty())
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3TransferContent, "No AssetID[%s] or ProviderID[%s] given"), _assetID.c_str(), _providerID.c_str());
		return *this;
	}

	std::string _volName;
	Properties::const_iterator input_iter = input.find("VolumeName");
	if(input_iter != input.end())
		_volName = input_iter->second;
	if(_volName.empty())
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3TransferContent, "No VolumeName[%s] given"), _volName.c_str());
		return *this;
	}

	A3Request::MessageCtx msgData;
	msgData.params["assetID"] = _assetID;
	msgData.params["providerID"] = _providerID;
	msgData.params["volumeName"]	= _volName;

	input_iter = input.find("ReasonCode");
	if(input_iter != input.end())
		msgData.params["reasonCode"] = input_iter->second;

	std::string A3RegionalUrl;
	if(input.find("RegionalIP") != input.end())
		A3RegionalUrl = input.find("RegionalIP")->second + ":";
	if(input.find("RegionalPort") != input.end())
		A3RegionalUrl += input.find("RegionalPort")->second;
	if(input.find("RegionalPath") != input.end())
		A3RegionalUrl += "/" + input.find("RegionalPath")->second;

	A3Request req(A3RegionalUrl, &_log);
	int errorCode = req.request(A3Request::A3_DeleteContent, msgData);
	std::string statusMsg = req.getStatusMessage();

	if (errorCode == 200) //ok
	{
		ctx.statusCode = aSucceed;
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(A3DeleteContent, "delete content successfully for PID[%s] PAID[%s]"), _providerID.c_str(), _assetID.c_str());
	}
	else
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3DeleteContent, "delete content failed: %s for PID[%s] PAID[%s]"), statusMsg.c_str(), _providerID.c_str(), _assetID.c_str());
	return *this;

}

Action& A3GetVolumeInfo::operator() (Context& ctx, const Properties& input, Properties& output)
{
	ctx.statusCode = aFailedQuit;

	std::string _volName;
	Properties::const_iterator input_iter = input.find("VolumeName");
	if(input_iter != input.end())
		_volName = input_iter->second;
	if(_volName.empty())
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3TransferContent, "No VolumeName[%s] given"), _volName.c_str());
		return *this;
	}

	A3Request::MessageCtx msgData;
	msgData.params["volumeName"]	= _volName;

	std::string A3RegionalUrl;
	if(input.find("RegionalIP") != input.end())
		A3RegionalUrl = input.find("RegionalIP")->second + ":";
	if(input.find("RegionalPort") != input.end())
		A3RegionalUrl += input.find("RegionalPort")->second;
	if(input.find("RegionalPath") != input.end())
		A3RegionalUrl += "/" + input.find("RegionalPath")->second;

	A3Request req(A3RegionalUrl, &_log);
	int errorCode = req.request(A3Request::A3_GetVolumeInfo, msgData);
	std::string statusMsg = req.getStatusMessage();

	if (errorCode == 200) //ok
	{
		ctx.statusCode = aSucceed;
		std::string volumeSize = msgData.params["volumeSize"]; // MB
		std::string freeSize = msgData.params["freeSize"]; // MB
		std::string state = msgData.params["state"]; // MB
		double freerate = atof(freeSize.c_str()) * 100 / atof(volumeSize.c_str());
		char buf[20] = {0};
		snprintf(buf, sizeof(buf), "%f", freerate);
		STL_MAPSET(Properties, output, "freespace", buf);
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(A3GetVolumeInfo, "get volume info successfully for [%s]"), _volName.c_str());
	}
	else
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3GetVolumeInfo, "get volume info failed: %s for [%s]"), statusMsg.c_str(), _volName.c_str());
	return *this;

}

Action& A3CancelTransfer::operator() (Context& ctx, const Properties& input, Properties& output)
{
	ctx.statusCode = aFailedQuit;

	std::string _volName;
	Properties::const_iterator input_iter = input.find("VolumeName");
	if(input_iter != input.end())
		_volName = input_iter->second;
	if(_volName.empty())
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3CancelTransfer, "No VolumeName[%s] given"), _volName.c_str());
		return *this;
	}

	std::string _providerID;
	std::string _assetID;
	Properties::const_iterator md_iter = ctx.metaData.find("Event.ProviderId");
	if(md_iter != ctx.metaData.end())
		_providerID = md_iter->second;
	md_iter = ctx.metaData.find("Event.ProviderAssetId");
	if(md_iter != ctx.metaData.end())
		_assetID = md_iter->second;
	if(_assetID.empty() || _providerID.empty())
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3CancelTransfer, "No AssetID[%s] or ProviderID[%s] given"), _assetID.c_str(), _providerID.c_str());
		return *this;
	}

	A3Request::MessageCtx msgData;
	msgData.params["assetID"] = _assetID;
	msgData.params["providerID"] = _providerID;
	msgData.params["volumeName"]	= _volName;

	input_iter = input.find("ReasonCode");
	if(input_iter != input.end())
		msgData.params["reasonCode"] = input_iter->second;

	std::string A3RegionalUrl;
	if(input.find("RegionalIP") != input.end())
		A3RegionalUrl = input.find("RegionalIP")->second + ":";
	if(input.find("RegionalPort") != input.end())
		A3RegionalUrl += input.find("RegionalPort")->second;
	if(input.find("RegionalPath") != input.end())
		A3RegionalUrl += "/" + input.find("RegionalPath")->second;

	A3Request req(A3RegionalUrl, &_log);
	int errorCode = req.request(A3Request::A3_CancelTransfer, msgData);
	std::string statusMsg = req.getStatusMessage();

	if (errorCode == 200) //ok
	{
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(A3CancelTransfer, "cancel tramsfer successfully for [%s][%s][%s]"), _volName.c_str(), _assetID.c_str(), _providerID.c_str());
	}
	else
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3CancelTransfer, "cancel tramsfer info failed: %s for [%s][%s][%s]"), statusMsg.c_str(), _volName.c_str(), _assetID.c_str(), _providerID.c_str());
	return *this;

}

Action& A3GetTransferStatus::operator() (Context& ctx, const Properties& input, Properties& output)
{
	ctx.statusCode = aFailedQuit;

	std::string _volName;
	Properties::const_iterator input_iter = input.find("VolumeName");
	if(input_iter != input.end())
		_volName = input_iter->second;
	if(_volName.empty())
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3GetTransferStatus, "No VolumeName[%s] given"), _volName.c_str());
		return *this;
	}

	std::string _providerID;
	std::string _assetID;
	Properties::const_iterator md_iter = ctx.metaData.find("Event.ProviderId");
	if(md_iter != ctx.metaData.end())
		_providerID = md_iter->second;
	md_iter = ctx.metaData.find("Event.ProviderAssetId");
	if(md_iter != ctx.metaData.end())
		_assetID = md_iter->second;
	if(_assetID.empty() || _providerID.empty())
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3GetTransferStatus, "No AssetID[%s] or ProviderID[%s] given"), _assetID.c_str(), _providerID.c_str());
		return *this;
	}

	A3Request::MessageCtx msgData;
	msgData.params["assetID"] = _assetID;
	msgData.params["providerID"] = _providerID;
	msgData.params["volumeName"]	= _volName;

	std::string A3RegionalUrl;
	if(input.find("RegionalIP") != input.end())
		A3RegionalUrl = input.find("RegionalIP")->second + ":";
	if(input.find("RegionalPort") != input.end())
		A3RegionalUrl += input.find("RegionalPort")->second;
	if(input.find("RegionalPath") != input.end())
		A3RegionalUrl += "/" + input.find("RegionalPath")->second;

	A3Request req(A3RegionalUrl, &_log);
	int errorCode = req.request(A3Request::A3_GetTransferStatus, msgData);
	std::string statusMsg = req.getStatusMessage();

	if (errorCode == 200) //ok
	{
		ctx.statusCode = aSucceed;
		std::string state = msgData.params["State"]; 
		std::string reasonCode = msgData.params["reasonCode"]; 
		std::string percentComplete = msgData.params["percentComplete"]; 
		std::string contentSize = msgData.params["contentSize"]; 
		std::string supportFileSize = msgData.params["supportFileSize"]; 
		std::string md5Checksum = msgData.params["md5Checksum"]; 
		std::string md5DateTime = msgData.params["md5DateTime"]; 
		std::string bitrate = msgData.params["bitrate"]; 
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(A3GetTransferStatus, "get tramsfer info successfully for [%s][%s][%s], [State:%s] [PercentComplete:%s] [ContentSize:%s] [SupportFileSize:%s] [MD5Checksum:%s] [Md5DateTime:%s] [Bitrate:%s]")
		, _volName.c_str(), _assetID.c_str(), _providerID.c_str(), state.c_str(), percentComplete.c_str(), contentSize.c_str(), supportFileSize.c_str(), md5Checksum.c_str(), md5DateTime.c_str(), bitrate.c_str());
	}
	else
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3GetTransferStatus, "get tramsfer info failed: %s for [%s][%s][%s]"), statusMsg.c_str(), _volName.c_str(), _assetID.c_str(), _providerID.c_str());
	return *this;

}

Action& A3GetContentChecksum::operator() (Context& ctx, const Properties& input, Properties& output)
{
	ctx.statusCode = aFailedQuit;

	std::string _volName;
	Properties::const_iterator input_iter = input.find("VolumeName");
	if(input_iter != input.end())
		_volName = input_iter->second;
	if(_volName.empty())
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(GetContentChecksum, "No VolumeName[%s] given"), _volName.c_str());
		return *this;
	}

	std::string _providerID;
	std::string _assetID;
	Properties::const_iterator md_iter = ctx.metaData.find("Event.ProviderId");
	if(md_iter != ctx.metaData.end())
		_providerID = md_iter->second;
	md_iter = ctx.metaData.find("Event.ProviderAssetId");
	if(md_iter != ctx.metaData.end())
		_assetID = md_iter->second;
	if(_assetID.empty() || _providerID.empty())
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(GetContentChecksum, "No AssetID[%s] or ProviderID[%s] given"), _assetID.c_str(), _providerID.c_str());
		return *this;
	}

	A3Request::MessageCtx msgData;
	msgData.params["assetID"] = _assetID;
	msgData.params["providerID"] = _providerID;
	msgData.params["volumeName"]	= _volName;

	input_iter = input.find("responseURL");
	if(input_iter == input.end())
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3TransferContent, "No responseURL given"));
		return *this;
	}
	msgData.params["responseURL"] = input_iter->second;

	std::string A3RegionalUrl;
	if(input.find("RegionalIP") != input.end())
		A3RegionalUrl = input.find("RegionalIP")->second + ":";
	if(input.find("RegionalPort") != input.end())
		A3RegionalUrl += input.find("RegionalPort")->second;
	if(input.find("RegionalPath") != input.end())
		A3RegionalUrl += "/" + input.find("RegionalPath")->second;

	A3Request req(A3RegionalUrl, &_log);
	int errorCode = req.request(A3Request::A3_GetContentChecksum, msgData);
	std::string statusMsg = req.getStatusMessage();

	if (errorCode == 200) //ok
	{
		ctx.statusCode = aSucceed;
		std::string md5Checksum = msgData.params["md5Checksum"]; 
		std::string md5DateTime = msgData.params["md5DateTime"]; 
		std::string resultCode = msgData.params["resultCode"]; 
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(GetContentChecksum, "get content checksum successfully for [%s][%s][%s], [MD5Checksum:%s] [Md5DateTime:%s] [ResultCode:%s]")
			, _volName.c_str(), _assetID.c_str(), _providerID.c_str(), md5Checksum.c_str(), md5DateTime.c_str(), resultCode.c_str());
	}
	else
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(GetContentChecksum, "get content checksum failed: %s for [%s][%s][%s]"), statusMsg.c_str(), _volName.c_str(), _assetID.c_str(), _providerID.c_str());
	return *this;

}