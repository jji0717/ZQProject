#ifndef _CONTENTSTORE_PROC_HEADER_
#define _CONTENTSTORE_PROC_HEADER_

#include "ContentStore.h"
#include "CPCImpl.h"

using namespace TianShanIce::Storage;
using namespace TianShanIce::ContentProvision;


class ContentProvisionWrapper: protected CPCImpl
{
public:
	typedef ::IceInternal::Handle<ContentProvisionWrapper> Ptr;
	friend class ::IceInternal::Handle<ContentProvisionWrapper>;

	ContentProvisionWrapper(ZQ::common::Log& log) : CPCImpl(log),_log(log) 
	{
	}

	// no throw
	bool init(Ice::CommunicatorPtr ic, ::TianShanIce::Storage::ContentStoreExPrx csPrx, const std::string& cpcEndPoint, int nRegisterInterval);

	void setTrickSpeeds(const TrickSpeeds& trickSpeeds);

	// no throw
	void unInit();

	void cancelProvision(const std::string& name, const std::string& provisionSessionId)
		throw (::TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt);

	TianShanIce::ContentProvision::ProvisionSessionPrx passiveProvision(
		::TianShanIce::Storage::ContentPrx	contentPrx,
		TianShanIce::ContentProvision::ProvisionContentKey	contentKey,
		const ::std::string& filePathName,
		const ::std::string& sourceType, 
		const ::std::string& startTimeUTC,
		const ::std::string& stopTimeUTC, 
		const int maxTransferBitrate,
		::std::string& pushUrl)
		throw (::TianShanIce::InvalidParameter, ::TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt);

	TianShanIce::ContentProvision::ProvisionSessionPrx activeProvision(
		::TianShanIce::Storage::ContentPrx	contentPrx,
		TianShanIce::ContentProvision::ProvisionContentKey	contentKey,
		const ::std::string& filePathName,
		const ::std::string& sourceUrl, 
		const ::std::string& sourceType,
		const ::std::string& startTimeUTC,
		const ::std::string& stopTimeUTC,
		const int maxTransferBitrate,
		bool bIsNPVRSession = false)
		throw (::TianShanIce::InvalidParameter, ::TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt);

	std::string getExposeUrl(const std::string& protocal, const TianShanIce::ContentProvision::ProvisionContentKey& contentkey, int transferBitrate, int& nTTL, int& permittedBitrate);

protected:

	Ice::CommunicatorPtr		_ic;	
	ZQ::common::Log&			_log;
	ZQADAPTER_DECLTYPE		_adapter;

	::TianShanIce::ContentProvision::ProvisionSessionBindPrx	_provisionEvent;	
};



#endif
