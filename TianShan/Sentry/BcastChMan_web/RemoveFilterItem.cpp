#include "RemoveFilterItem.h"

namespace BcastWebPage
{

	RemoveFilterItem::RemoveFilterItem(IHttpRequestCtx* pHttpRequestCtx) : BasePage(pHttpRequestCtx)
	{
	}

	RemoveFilterItem::~RemoveFilterItem()
	{
	}

	bool RemoveFilterItem::get()
	{
		IHttpResponse& responser = _reqCtx->Response();

		std::string chnlName, itemName;
		chnlName = "Filter";
		itemName = _varMap[FilterItemNameKey];
		if (itemName.empty())
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "No filter item name<br>");
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(RemoveFilterItem, "%s"), getLastError());
			LinkSpaceError;
			LinkToBcastMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}

		try
		{
			chnlPub->removeFilterItem(itemName);
		}
		catch (const TianShanIce::InvalidParameter& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "remove [%s#%s] caught %s:%s<br>", 
				chnlName.c_str(), itemName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(RemoveFilterItem, "%s"), getLastError());
			LinkSpaceError;
			LinkToBcastMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const TianShanIce::BaseException& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "remove [%s#%s] caught %s:%s<br>", 
				chnlName.c_str(), itemName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(RemoveFilterItem, "%s"), getLastError());
			LinkSpaceError;
			LinkToBcastMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const Ice::Exception& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "remove [%s#%s] caught %s<br>", 
				chnlName.c_str(), itemName.c_str(), ex.ice_name().c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(RemoveFilterItem, "%s"), getLastError());
			LinkSpaceError;
			LinkToBcastMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}

		// redirect page to show item page
		RedirectToBcastMainPage;

		return true;
	}

	bool RemoveFilterItem::post()
	{
		return true;
	}

} // namespace CodWebPage

