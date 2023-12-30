#ifndef _A3CONTENTSTORE_PROC_HEADER_
#define _A3CONTENTSTORE_PROC_HEADER_

#include "ContentStore.h"
#include "CPCImpl.h"

using namespace TianShanIce::Storage;
using namespace TianShanIce::ContentProvision;

class D4Speaker;

namespace CRM
{
	namespace A3Message
	{
class A3MsgEnv;
class ContentProvisionWrapper: protected CPCImpl
{
public:
	typedef ::IceInternal::Handle<ContentProvisionWrapper> Ptr;
	friend class ::IceInternal::Handle<ContentProvisionWrapper>;

	ContentProvisionWrapper(ZQ::common::Log& log, A3MsgEnv& env) : CPCImpl(log), _log(log), _env(env) 	{
	}

	// no throw
	bool init(Ice::CommunicatorPtr ic, const std::string& cpcEndPoint, int nRegisterInterval,D4Speaker *pD4Speaker=NULL);

	void setTrickSpeeds(const TrickSpeeds& trickSpeeds);

	// no throw
	void unInit();

	TianShanIce::ContentProvision::ProvisionSessionPrx activeProvision(
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
		bool bIsNPVRSession = false);
//		throw (::TianShanIce::InvalidParameter, ::TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt);

	std::string getExposeUrl(const std::string& protocal, const ::TianShanIce::ContentProvision::ProvisionContentKey& contentkey, int transferBitrate, int& nTTL, int& permittedBitrate);

protected:

	ZQ::common::Log&			_log;
	Ice::CommunicatorPtr		_ic;	
	ZQADAPTER_DECLTYPE		_adapter;
	A3MsgEnv&		          _env;

	::TianShanIce::ContentProvision::ProvisionSessionBindPrx	_provisionEvent;	
};


}}

#endif

