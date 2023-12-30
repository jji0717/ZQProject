#include "RemoveChannel.h"

namespace BcastWebPage
{

	RemoveChannel::RemoveChannel(IHttpRequestCtx* pHttpRequestCtx) : BasePage(pHttpRequestCtx)
	{
	}

	RemoveChannel::~RemoveChannel()
	{
	}

	bool RemoveChannel::get()
	{
		IHttpResponse& responser = _reqCtx->Response();

		std::string chnlName;
		chnlName = _varMap[ChannelNameKey];
		if (chnlName.empty())
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "No channel name<br>");
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(InsertItem, "%s"), getLastError());
			LinkToBcastMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}

		TianShanIce::Application::PublishPointPrx pubPrx = NULL;
		try
		{
			pubPrx = chnlPub->open(chnlName);
			pubPrx->destroy();
		}
		catch (const TianShanIce::InvalidParameter& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "remove channel [%s] caught %s:%s<br>", 
				chnlName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(RemoveChannel, "%s"), getLastError());
			LinkToBcastMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const TianShanIce::BaseException& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "remove channel [%s] caught %s:%s<br>", 
				chnlName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(RemoveChannel, "%s"), getLastError());
			LinkToBcastMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const Ice::Exception& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "remove channel [%s] caught %s<br>", 
				chnlName.c_str(), ex.ice_name().c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(RemoveChannel, "%s"), getLastError());
			LinkToBcastMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}

		RedirectToBcastMainPage;

		return true;
	}

	bool RemoveChannel::post()
	{
		return false;
	}

} 

