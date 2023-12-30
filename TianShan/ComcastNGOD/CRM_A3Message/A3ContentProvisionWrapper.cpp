
#include "Log.h"
#include "TianShanDefines.h"
#include "A3ContentProvisionWrapper.h"
#include "CPHInc.h"
#include "urlstr.h"
#include "strHelper.h"
#include "A3ProvEventSink.h"
#include "A3Config.h"
extern ZQ::common::Config::Loader< A3MessageCfg > _A3Config;

using namespace TianShanIce;
using namespace TianShanIce::Storage;
using namespace TianShanIce::ContentProvision;

#define MOLOG	(_log)

namespace CRM
{
	namespace A3Message
	{
bool ContentProvisionWrapper::init(Ice::CommunicatorPtr ic, const std::string& cpcEndPoint, int nRegisterInterval, D4Speaker* d4Speaker)
{
	_ic = ic;

	if (!CPCImpl::init(ic, nRegisterInterval, d4Speaker))
		return false;

	try
	{
		_adapter = ZQADAPTER_CREATE(ic, ADAPTER_NAME_CPC, cpcEndPoint.c_str(), MOLOG);
	}
	catch(Ice::Exception& ex)
	{
		_log(ZQ::common::Log::L_ERROR,LOGFMT("Create adapter failed with endpoint=%s and exception is %s"),
			cpcEndPoint.c_str(), ex.ice_name().c_str());
		return false;
	}
	try 
	{
		ProvisionEventSink::Ptr provisionEventPtr = new ProvisionEventSink(MOLOG, _env);	
		Ice::Identity strIdent = ic->stringToIdentity(A3_CS_PROVISION_EVENT_SINK);
		_adapter->ZQADAPTER_ADD(ic, provisionEventPtr, A3_CS_PROVISION_EVENT_SINK);
		_provisionEvent = ::TianShanIce::ContentProvision::ProvisionSessionBindPrx::
			uncheckedCast(_adapter->createProxy(strIdent));		

		_adapter->ZQADAPTER_ADD(ic, this, SERVICE_NAME_ContentProvisionCluster);	
		_adapter->activate();
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, LOGFMT("add adapter caught exception[%s]: %s"), ex.ice_name().c_str(), ex.message.c_str());
		return false;
	}
	catch(const ::Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, LOGFMT("add adapter caught exception[%s]"), ex.ice_name().c_str());
		return false;
	}

	return true;
}

void ContentProvisionWrapper::unInit()
{
	CPCImpl::uninit();
	_adapter = NULL;
	_provisionEvent = NULL;
	_ic = NULL;	
}

TianShanIce::ContentProvision::ProvisionSessionPrx ContentProvisionWrapper::activeProvision(
					 ::TianShanIce::Storage::ContentPrx	contentPrx,
					 TianShanIce::ContentProvision::ProvisionContentKey	contentKey,
					 const ::std::string& filePathName,
					 const ::std::string& sourceUrl, 
					 const ::std::string& sourceType,
					 const ::std::string& startTimeUTC,
					 const ::std::string& stopTimeUTC,
					 const int maxTransferBitrate,
					 ::TianShanIce::Properties& props,
					 ::TianShanIce::Properties& outProps,
					 bool bIsNPVRSession)
//                     throw (::TianShanIce::InvalidParameter, ::TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt)
{
	ZQ::common::URLStr src(sourceUrl.c_str());
	const char* proto = src.getProtocol();

	if(!proto) 
	{
		MOLOG(ZQ::common::Log::L_ERROR, LOGFMT("failed to parse source url (%s) for (%s)"), sourceUrl.c_str(), contentKey.content.c_str());
        return NULL;
	}

	std::string strSourceType;
	bool isFileSet = false;

	/* file set provision detect */
	std::vector<std::string> res = ZQ::common::stringHelper::split(sourceType, ':');		
	if(res.size() >= 2) 
	{
		isFileSet = true;
		strSourceType = res.at(0);
	}
	else
	{
		strSourceType = sourceType;
	}

 /*   
	if(strSourceType == ctH264)
		strSubType = subctVV2;
#ifdef ZQ_OS_MSWIN
	else if(strSourceType == ctMPEG2TS)
		strSubType = subctVVX;
	else if(strSourceType == ctH265)
		strSubType = subctVV2;
#else
	else if(strSourceType == ctMPEG2TS)
		strSubType = "VVC";
	else if(strSourceType == ctH265)
		strSubType = "VVC";
#endif
	else if(strSourceType == ctCSI)
		strSubType = "unkonwn";
*/

	if(strSourceType == ctH265 || !stricmp(proto, "aqua"))
		props[CPHPM_NOTRICKSPEEDS] = "1";

	std::string strProvisionMethod;
	bool  audioType = false;

	if(strSourceType == ctCSI)
	{
		if(!stricmp(proto, "aqua"))
			strProvisionMethod = METHODTYPE_AQUA_INDEX;
		else
			strProvisionMethod = METHODTYPE_AQUA_CSI;
	}
	else if (!stricmp(proto, potoFTP.c_str()))
	{
		if ( strSourceType == ctMPEG2TS)
		{
			strProvisionMethod = METHODTYPE_AQUA_FTPRTF;
		}
		else if (strSourceType == ctH264)
			strProvisionMethod = METHODTYPE_AQUA_FTPRTFH264;
		else if(strSourceType == ctH265)
			strProvisionMethod = METHODTYPE_AQUA_FTPRTFH265;
	}
	else if(!stricmp(proto, "udp"))
	{
		if (strSourceType == ctMPEG2TS)
		{
			strProvisionMethod = METHODTYPE_AQUA_RTI;
		}
		else if (strSourceType == ctH264)
		{
			strProvisionMethod = METHODTYPE_AQUA_RTIH264;
		}
		else if (strSourceType == ctH265)
		{
			strProvisionMethod = METHODTYPE_AQUA_RTIH265;
		}
	}
	else if(!stricmp(proto, "file") || !stricmp(proto, potoCIFS.c_str()))
	{
		if (strSourceType == ctMPEG2TS)
		{
			strProvisionMethod = METHODTYPE_AQUA_NTFSRTF;
		}
		else if (strSourceType == ctH264)
		{
			strProvisionMethod = METHODTYPE_AQUA_NTFSRTFH264;
		}
		else if (strSourceType == ctH265)
		{
			strProvisionMethod = METHODTYPE_AQUA_NTFSRTFH265;
		}
	}
	else if(!stricmp(proto, "aqua"))
	{
		if (strSourceType == ctMPEG2TS)
		{
			strProvisionMethod = METHODTYPE_AQUA_INDEX;
		}
		else if (strSourceType == ctH264)
		{
			strProvisionMethod = METHODTYPE_AQUA_INDEXH264;
		}
		else if (strSourceType == ctH264)
		{
			strProvisionMethod = METHODTYPE_AQUA_INDEXH265;
		}
	}
	else if(!stricmp(proto, "raw"))
	{
		strProvisionMethod = METHODTYPE_RTIRAW;
	}

	MOLOG(ZQ::common::Log::L_INFO, LOGFMT("activeProvision()sourceUrl[%s], contentKey[%s], sourceType[%s], methodtype[%s]"),
		sourceUrl.c_str(), contentKey.content.c_str(), strSourceType.c_str(), strProvisionMethod.c_str());
	if (strProvisionMethod.empty())
	{
		MOLOG(ZQ::common::Log::L_ERROR, LOGFMT("content[%s] unsupported provision source url[%s] and content type[%s]"), contentKey.content.c_str(), sourceUrl.c_str(),strSourceType.c_str());
		return NULL;
	}

	TianShanIce::ContentProvision::ProvisionSessionPrx pPrx = provision(strProvisionMethod, sourceUrl, contentKey, filePathName, startTimeUTC, stopTimeUTC, maxTransferBitrate, _provisionEvent, contentPrx, props, audioType);

	if (strProvisionMethod == METHODTYPE_CDN_HTTPPropagation || strProvisionMethod == METHODTYPE_CDN_FTPPropagation)
	{
		outProps["METADATA_MonoProvision"] = "1";
	}

	return pPrx;
}
std::string ContentProvisionWrapper::getExposeUrl(const std::string& protocal, const ::TianShanIce::ContentProvision::ProvisionContentKey& contentkey, int transferBitrate, int& nTTL, int& permittedBitrate)
{
	return CPCImpl::getExposeUrl(protocal, contentkey, transferBitrate, nTTL, permittedBitrate);
}

void ContentProvisionWrapper::setTrickSpeeds( const TrickSpeeds& trickSpeeds )
{
	CPCImpl::setTrickSpeeds(trickSpeeds);
}
}}