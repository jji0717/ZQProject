
#include "Log.h"
#include "TianShanDefines.h"
#include "ContentProvisionWrapper.h"
#include "CPHInc.h"
#include "urlstr.h"
#include "strHelper.h"
#include "ProvEventSink.h"


using namespace TianShanIce;
using namespace TianShanIce::Storage;
using namespace TianShanIce::ContentProvision;

#define MOLOG	(_log)


bool ContentProvisionWrapper::init(Ice::CommunicatorPtr ic, ::TianShanIce::Storage::ContentStoreExPrx csPrx, const std::string& cpcEndPoint, int nRegisterInterval, D4Speaker* d4Speaker)
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

TianShanIce::ContentProvision::ProvisionSessionPrx ContentProvisionWrapper::passiveProvision(
					  ::TianShanIce::Storage::ContentPrx	contentPrx,
					  TianShanIce::ContentProvision::ProvisionContentKey	contentKey,
					  const ::std::string& filePathName,
					  const ::std::string& sourceType, 
					  const ::std::string& startTimeUTC,
					  const ::std::string& stopTimeUTC, 
					  const int maxTransferBitrate,
					  ::std::string& pushUrl)
//                      throw (::TianShanIce::InvalidParameter, ::TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt)
{
	std::string strProvisionMethod;
	if (sourceType == ctH264)
		strProvisionMethod = METHODTYPE_RTFRDSH264VSVSTRM;
	else
		strProvisionMethod = METHODTYPE_RTFRDSVSVSTRM;

#ifdef CDNCS_SERVICE
	// throw, unsupported provision source protocol and type
	ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(
		_log,
		"ContentStorI",
		csexpInvalidSourceURL,
		"content[%s] unsupported passiveProvision",
		contentKey.content.c_str()			
		);
#endif
    ::TianShanIce::Properties prop;
	TianShanIce::ContentProvision::ProvisionSessionPrx pPrx = provision(strProvisionMethod, "", contentKey,
		filePathName, startTimeUTC, stopTimeUTC, maxTransferBitrate, _provisionEvent, contentPrx,prop);

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
					 ::TianShanIce::Properties& prop,
					 bool bIsNPVRSession)
//                     throw (::TianShanIce::InvalidParameter, ::TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt)
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
    
	if(strSourceType == ctH264)
		strSubType = subctVV2;
	else if(strSourceType == ctMPEG2TS)
		strSubType = subctVVX;
	else if(strSourceType == ctCSI)
		strSubType = "unkonwn";

//	strSubType = (strSourceType == ctH264) ? subctVV2 : subctVVX;

	std::string strProvisionMethod;
	bool  audioType = false;

#ifndef CDNCS_SERVICE
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
			if (strSourceType == ctMPEG2TS || strSourceType == ctAudioTS)
			{
				if (strSourceType == ctAudioTS)
					audioType = true;
				strProvisionMethod = METHODTYPE_NTFSRTFVSVSTRM;
			}
			else if (strSourceType == ctH264)
			{
				strProvisionMethod = METHODTYPE_NTFSRTFH264VSVSTRM;
			}	
			else if (strSourceType == ctCSI)
			{
				strProvisionMethod = METHODTYPE_CSI;
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
			else if (strSourceType == ctCSI)
			{
				strProvisionMethod = METHODTYPE_CSI;
			}
		}
		else if (!stricmp(proto, "udp"))
		{
			if ("NativeCS" == strSourceType)
			{
				strProvisionMethod = METHODTYPE_RTIRAW;
			}
			else if (strSourceType == ctMPEG2TS)
			{
				strProvisionMethod = METHODTYPE_RTIVSVSTRM;
			}
			else if (strSourceType == ctH264)
			{
				strProvisionMethod = METHODTYPE_RTIH264VSVSTRM;
			}
			else if (strSourceType == ctCSI)
			{
				strProvisionMethod = METHODTYPE_CSI;
			}
		}
		else if(!stricmp(proto, "c2http"))
		{
			strProvisionMethod = METHODTYPE_CDN_HTTPPropagation;
		}
		else if(!stricmp(proto, "c2pull"))
		{
			if (strSourceType == ctMPEG2TS)
			{
				strProvisionMethod = METHODTYPE_CDN_C2Pull;
			}
			else if (strSourceType == ctH264)
			{
				strProvisionMethod = METHODTYPE_CDN_C2PullH264;
			}
		}
	}

	if (bIsNPVRSession)
	{
		strProvisionMethod = METHODTYPE_NPVRVSVSTRM;
	}
#else
	if (!stricmp(proto, potoFTP.c_str()))
	{
		if (isFileSet)
			strProvisionMethod = METHODTYPE_CDN_FTPPropagation;
		else if (strSourceType == ctH264)
		{
			strProvisionMethod = METHODTYPE_CDN_FTPRTFH264;
		}
		else
			strProvisionMethod = METHODTYPE_CDN_FTPRTF;
	}
	else if(!stricmp(proto, "udp"))
	{
		if ("NativeCS" == strSourceType)
		{
			strProvisionMethod = METHODTYPE_RTIRAW;
		}
		else if (strSourceType == ctMPEG2TS)
		{
			strProvisionMethod = METHODTYPE_RTIVSVSTRM;
		}
		else if (strSourceType == ctH264)
		{
			strProvisionMethod = METHODTYPE_RTIH264VSVSTRM;
		}
	}
	else if(!stricmp(proto, "file") || !stricmp(proto, potoCIFS.c_str()))
	{
		if (strSourceType == ctMPEG2TS)
		{
			strProvisionMethod = METHODTYPE_CDN_NTFSRTF;
		}
		else if (strSourceType == ctH264)
		{
			strProvisionMethod = METHODTYPE_CDN_NTFSRTFH264;
		}
	}
	else if(!stricmp(proto, "c2http"))
	{
		strProvisionMethod = METHODTYPE_CDN_HTTPPropagation;
	}
	else if(!stricmp(proto, "c2pull"))
	{
		if (strSourceType == ctMPEG2TS)
		{
			strProvisionMethod = METHODTYPE_CDN_C2Pull;
		}
		else if (strSourceType == ctH264)
		{
			strProvisionMethod = METHODTYPE_CDN_C2PullH264;
		}
	}
#endif
	if (strProvisionMethod.empty())
	{
		// throw, unsupported provision source protocol and type
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

	TianShanIce::ContentProvision::ProvisionSessionPrx pPrx = provision(strProvisionMethod, sourceUrl, contentKey, filePathName, startTimeUTC, stopTimeUTC, maxTransferBitrate, _provisionEvent, contentPrx, prop, audioType);

	if (strProvisionMethod == METHODTYPE_CDN_HTTPPropagation || strProvisionMethod == METHODTYPE_CDN_FTPPropagation)
	{
		::TianShanIce::Storage::UnivContentPrx uniContent = ::TianShanIce::Storage::UnivContentPrx::uncheckedCast(contentPrx);
		TianShanIce::Properties metaData;
		metaData["METADATA_MonoProvision"] = "1";
		try
		{
			uniContent->setMetaData(metaData);
			MOLOG(ZQ::common::Log::L_DEBUG, LOGFMT("[%s] set content metadata METADATA_MonoProvision = [1]"), contentKey.content.c_str());
		}
		catch (const Ice::Exception& ex) 
		{
		}
		catch (...) 
		{
		}
	}

	return pPrx;
}

void ContentProvisionWrapper::cancelProvision(const std::string& name, const std::string& provisionSessionId  )
//    throw (::TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt)
{
	_log(ZQ::common::Log::L_INFO, CLOGFMT(ContentProvisionWrapper, "cancel provision of content (%s)"), name.c_str());

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


std::string ContentProvisionWrapper::getExposeUrl(const std::string& protocal, const ::TianShanIce::ContentProvision::ProvisionContentKey& contentkey, int transferBitrate, int& nTTL, int& permittedBitrate)
{
	return CPCImpl::getExposeUrl(protocal, contentkey, transferBitrate, nTTL, permittedBitrate);
}

void ContentProvisionWrapper::setTrickSpeeds( const TrickSpeeds& trickSpeeds )
{
	CPCImpl::setTrickSpeeds(trickSpeeds);
}
void ContentProvisionWrapper::setNoTrickSpeedFileRegex(bool enable, const TianShanIce::StrValues& fileRegexs )
{
	CPCImpl::setNoTrickSpeedFileRegex(enable,fileRegexs);
}
