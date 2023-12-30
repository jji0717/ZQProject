// File Name : SetupHandler.h

#ifndef __EVENT_IS_VODI5_SETUP_HANDLER_H__
#define __EVENT_IS_VODI5_SETUP_HANDLER_H__

#include "RequestHandler.h"

#include "SelectionCommand.h"

namespace GBss
{

class SetupHandler : public RequestHandler
{
public:
	typedef IceUtil::Handle<SetupHandler> Ptr;

	SetupHandler(ZQ::common::Log& fileLog, Environment& env, 
		IStreamSmithSite* pSite, IClientRequestWriter* pReq);

	~SetupHandler();

	virtual RequestProcessResult doContentHandler();

private:
	/// get play list items from request
	bool getAssetsInfo(SelectIntentionParam& selectIntentionParam);
	bool getAssetInfo(const std::string& strItem, SelectIntentionParam::PlaylistItemInfo& info);

	/// create stream instance 
	bool createStream( TianShanIce::Streamer::StreamSmithAdminPrx streamServiceAdminPrx, 
		TianShanIce::SRM::ResourceMap& resMap, std::string globalSessId);

	/// render play list
	bool renderPlaylist(StreamerSelection& streamerSelection);

	/// use map to storage transport details;
	bool getTransportDetail(const std::string& strTransportdata, std::map<std::string, std::string>& transportMap);

private:
	std::string _strOriginalURI;
	TianShanIce::SRM::ResourceMap _resourceMap;
	SsmGBss::PlaylistItemSetupInfos _setupInfos;
	std::vector<std::string> _requestItems;
};

} // end GBss

#endif // end __EVENT_IS_VODI5_SETUP_HANDLER_H__
