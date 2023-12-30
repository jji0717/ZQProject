#include "Log.h"
#include "TianShanDefines.h"
#include "ProvWrapper.h"
#include "CPHInc.h"
#include "urlstr.h"
#include "strHelper.h"
#include "ProvEventSink.h"


using namespace ZQ::common;
using namespace TianShanIce;
using namespace TianShanIce::Storage;
using namespace TianShanIce::ContentProvision;

#define MOLOG	(_log)

#ifdef ZQ_OS_LINUX
#define stricmp strcasecmp
#endif

bool ContentProvisionWrapper::init(Ice::CommunicatorPtr ic, ::TianShanIce::Storage::ContentStoreExPrx csPrx, const std::string& cpcEndPoint, int nRegisterInterval)
{
	_ic = ic;

	if (!CPCImpl::init(ic, nRegisterInterval))
		return false;

 //   printf("ProvWrapper: %p\n", &_log);
	try 
	{	
		_adapter = ZQADAPTER_CREATE(ic, ADAPTER_NAME_CPC, cpcEndPoint.c_str(), MOLOG);

		ProvisionEventSink::Ptr provisionEventPtr = new ProvisionEventSink(csPrx, MOLOG);	
		Ice::Identity strIdent = ic->stringToIdentity(CS_PROVISION_EVENT_SINK);
		_adapter->ZQADAPTER_ADD(ic, provisionEventPtr, CS_PROVISION_EVENT_SINK);
		_provisionEvent = ::TianShanIce::ContentProvision::ProvisionSessionBindPrx::
			uncheckedCast(_adapter->createProxy(strIdent));		

		_adapter->ZQADAPTER_ADD(ic, this, SERVICE_NAME_ContentProvisionCluster);	
		_adapter->activate();
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPC, "caught exception[%s]: %s"), ex.ice_name().c_str(), ex.message.c_str());
		return false;
	}
	catch(const ::Ice::Exception& ex)
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPC, "caught exception[%s]"), ex.ice_name().c_str());
		return false;
	}

	return true;
}

void ContentProvisionWrapper::unInit()
{
	CPCImpl::uninit();
  
    _adapter->remove(_ic->stringToIdentity(CS_PROVISION_EVENT_SINK));
    _adapter->remove(_ic->stringToIdentity(SERVICE_NAME_ContentProvisionCluster));

	_adapter->deactivate();
//	_ic = 0;	
}

TianShanIce::ContentProvision::ProvisionSessionPrx ContentProvisionWrapper::passiveProvision(
					  ::TianShanIce::Storage::ContentPrx	contentPrx,
					  TianShanIce::ContentProvision::ProvisionContentKey	contentKey,
					  const ::std::string& filePathName,
					  const ::std::string& sourceType, 
					  const ::std::string& startTimeUTC,
					  const ::std::string& stopTimeUTC, 
					  const int maxTransferBitrate,
					  ::std::string& pushUrl)
		throw (::TianShanIce::InvalidParameter, ::TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt)
{
	std::string strProvisionMethod;
	strProvisionMethod = METHODTYPE_RTFRDSVSVSTRM;

	TianShanIce::ContentProvision::ProvisionSessionPrx pPrx = provision(strProvisionMethod, "", contentKey,
		filePathName, startTimeUTC, stopTimeUTC, maxTransferBitrate, _provisionEvent, contentPrx);

	//get upload url	
	::TianShanIce::Properties propers = pPrx->getProperties();
	::TianShanIce::Properties::const_iterator it = propers.find(PROPTY_PUSHURL);
	if (it!=propers.end())
	{
		pushUrl = it->second;
	}

	MOLOG(ZQ::common::Log::L_INFO, LOGFMT("[%s|%s|%s] passiveProvision return url [%s]"), 
		contentKey.content.c_str(), contentKey.contentStoreNetId.c_str(), contentKey.volume.c_str(), pushUrl.c_str());
	
	return pPrx;
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
					 bool bIsNPVRSession)
		throw (TianShanIce::InvalidParameter, ::TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt)
{
	ZQ::common::URLStr src(sourceUrl.c_str());
	const char* proto = src.getProtocol();

	if(!proto) 
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(
			_log,
			"ContentStorI",
			csexpInvalidSourceURL,
			"failed to parse source url (%s) for (%s)",
			sourceUrl.c_str(),
			contentKey.content.c_str()
			);
	}

	std::string strSourceType, strSubType;
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
	strSubType = (strSourceType == ctH264) ? subctVV2 : subctVVX;

	std::string strProvisionMethod;
	if (isFileSet)
	{
		if (!stricmp(proto, "file") || !stricmp(proto, potoCIFS.c_str()))
		{
			strProvisionMethod = METHODTYPE_NASCOPYVSVSTRM;
		}
		else if (!stricmp(proto, potoFTP.c_str()))
		{
			strProvisionMethod = METHODTYPE_FTPPropagation;
		}
	}
	else
	{
		if (!stricmp(proto, "file") || !stricmp(proto, potoCIFS.c_str()))
		{
			if (strSourceType == ctMPEG2TS)
			{
//				strProvisionMethod = METHODTYPE_NTFSRTFVSVSTRM;
				strProvisionMethod = METHODTYPE_COPYDEMO;
			}
			else if (strSourceType == ctH264)
			{
				strProvisionMethod = METHODTYPE_NTFSRTFH264VSVSTRM;
			}			
		}
		else if (!stricmp(proto, potoFTP.c_str()))
		{
			if (strSourceType == ctMPEG2TS)
			{
				strProvisionMethod = METHODTYPE_FTPRTFVSVSTRM;
			}
			else if (strSourceType == ctH264)
			{
				strProvisionMethod = METHODTYPE_FTPRTFH264VSVSTRM;
			}			
		}
		else if (!stricmp(proto, "udp"))
		{
			if (strSourceType == ctMPEG2TS)
			{
				strProvisionMethod = METHODTYPE_RTIVSVSTRM;
			}
			else if (strSourceType == ctH264)
			{
				strProvisionMethod = METHODTYPE_RTIH264VSVSTRM;
			}
		}
	}

	if (strProvisionMethod.empty()) {
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(
			_log,
			"ContentStorI",
			csexpInvalidSourceURL,
			"content[%s] unsupported provision source url[%s] and content type[%s]",
			contentKey.content.c_str(), 
			sourceUrl.c_str(),
			strSourceType.c_str()			
        );
	}

    TianShanIce::ContentProvision::ProvisionSessionPrx pPrx = 0;
    try {
         pPrx = provision(strProvisionMethod, sourceUrl, contentKey, filePathName, 
                          startTimeUTC, stopTimeUTC, maxTransferBitrate, _provisionEvent, contentPrx);
    }
    catch(const TianShanIce::Storage::NoResourceException& ex) {
        ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (
            _log,
            "ContentStorI",
            csexpNoResource,
            "method type [%s] not supported",
            strProvisionMethod.c_str()			
        );
    }

	return pPrx;
}

void ContentProvisionWrapper::cancelProvision(const std::string& name, const std::string& provisionSessionId  )
		throw (::TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt)
{
	_log(ZQ::common::Log::L_INFO, LOGFMT("cancel provision of content (%s)"), name.c_str());

	if (provisionSessionId.empty())
	{
		ZQTianShan::_IceThrow<InvalidStateOfArt>(
			"ContentI",
			csexpContentIsReady,
			"no provision session information for content (%s)",
			name.c_str()
			);
	}

	try 
	{
		ProvisionSessionPrx session = ProvisionSessionPrx::uncheckedCast(
			_ic->stringToProxy(provisionSessionId));

		session->cancel(0, "");
	}
	catch(const IceUtil::NullHandleException&) 
	{
		ZQTianShan::_IceThrow<ServerError>(
			_log,
			EXPFMT(ContentStoreI, csexpContentNotFound, "provision session[%s] not found for conetnt(%s) while cancelProvision()"), 
			provisionSessionId.c_str(), name.c_str());

	}
	catch(const Ice::Exception& ex) 
	{
		ZQTianShan::_IceThrow<ServerError>(
			_log,
			EXPFMT(ContentStoreI, csexpInternalError, "failed to cancel provision session[%s] for content(%s): (%s)"),
			provisionSessionId.c_str(), name.c_str(), ex.ice_name().c_str());
	}	
};


std::string ContentProvisionWrapper::getExposeUrl(const std::string& protocal, const TianShanIce::ContentProvision::ProvisionContentKey& contentkey, int transferBitrate, int& nTTL, int& permittedBitrate)
{
	return CPCImpl::getExposeUrl(protocal, contentkey, transferBitrate, nTTL, permittedBitrate);
}

void ContentProvisionWrapper::setTrickSpeeds( const TrickSpeeds& trickSpeeds )
{
	CPCImpl::setTrickSpeeds(trickSpeeds);
}
