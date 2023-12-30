#ifndef __BcastWebPage_BasePage_H__
#define __BcastWebPage_BasePage_H__

#define _WINSOCK2API_

#include "TsAppBcast.h"

#include <string>
#include <Ice/Ice.h>
#include "../httpdInterface.h"
#include <Log.h>
#include "./stroprt.h"
#include "./BcastChannelEx.h"
#include "./DataTypes.h"
#include "urlstr.h"
#include "BcastChDef.h"

#define DebugLog ZQ::common::Log::L_DEBUG
#define InfoLog ZQ::common::Log::L_INFO
#define NoticeLog ZQ::common::Log::L_NOTICE
#define WarningLog ZQ::common::Log::L_WARNING
#define ErrorLog ZQ::common::Log::L_ERROR
#define CritLog ZQ::common::Log::L_CRIT
#define ALertLog ZQ::common::Log::L_ALERT
#define EmergLog ZQ::common::Log::L_EMERG

#define ChannelNameKey			"bcast#chnl"
#define ItemNameKey				"bcast#item"
#define FilterItemNameKey		"bcast#filteritem"
#define PublisherKey			"bcast#pubep"
#define TemplateKey				"#template"
#define FunctionKey				"#function"
#define TabKey					"#tab"

#define BcastMainPage			"BcastMain.bcastweb.tswl"
#define ShowChannelPage			"ShowChannel.bcastweb.tswl"
#define AddChannelPage			"AddChannel.bcastweb.tswl"
#define EditChannelPage			"EditChannel.bcastweb.tswl"
#define RemoveChannelPage		"RemoveChannel.bcastweb.tswl"
#define PushItemPage			"PushItem.bcastweb.tswl"
#define InsertItemPage			"InsertItem.bcastweb.tswl"
#define ShowItemPage			"ShowItem.bcastweb.tswl"
#define RemoveItemPage			"RemoveItem.bcastweb.tswl"
#define EditItemPage			"EditItem.bcastweb.tswl"
#define AddFilterItemPage		"AddFilterItem.bcastweb.tswl"
#define RemoveFilterItemPage	"RemoveFilterItem.bcastweb.tswl"

#define SplitNetIdChar			";"

#define LinkToBcastMainPageRight	\
					url.clear();\
					url.setPath(BcastMainPage);\
					url.setVar(PublisherKey, _varMap[PublisherKey].c_str());\
					url.setVar(TemplateKey, _varMap[TemplateKey].c_str());\
					snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Main page</B></a>", String::getRightStr(url.generate(), "/", false).c_str());\
					responser<<szBuf;

#define LinkToBcastMainPageError	\
					url.clear();\
					url.setPath(BcastMainPage);\
					url.setVar(PublisherKey, _varMap[PublisherKey].c_str());\
					url.setVar(TemplateKey, _varMap[TemplateKey].c_str());\
					snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Main page</B></a>", String::getRightStr(url.generate(), "/", false).c_str());\
					addToLastError(szBuf);

#define RedirectToBcastMainPage \
					url.clear();\
					url.setPath(BcastMainPage);\
					url.setVar(TemplateKey, _varMap[TemplateKey].c_str());\
					url.setVar(PublisherKey, _varMap[PublisherKey].c_str());\
					responser<<"<script language=\"javascript\">";\
					snprintf(szBuf, sizeof(szBuf) - 1, "document.location.href=\"%s\"", String::getRightStr(url.generate(), "/", false).c_str());\
					responser<<szBuf;\
					responser<<"</script>";

#define LinkToShowChannelPageRight	\
					url.clear();\
					url.setPath(ShowChannelPage);\
					url.setVar(PublisherKey, _varMap[PublisherKey].c_str());\
					url.setVar(TemplateKey, _varMap[TemplateKey].c_str());\
					url.setVar(ChannelNameKey, chnlName.c_str());\
					snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Return to channel: [%s]</B></a>", String::getRightStr(url.generate(), "/", false).c_str(), chnlName.c_str());\
					responser<<szBuf;

#define LinkToShowChannelPageError	\
					url.clear();\
					url.setPath(ShowChannelPage);\
					url.setVar(PublisherKey, _varMap[PublisherKey].c_str());\
					url.setVar(TemplateKey, _varMap[TemplateKey].c_str());\
					url.setVar(ChannelNameKey, chnlName.c_str());\
					snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Return to channel: [%s]</B></a>", String::getRightStr(url.generate(), "/", false).c_str(), chnlName.c_str());\
					addToLastError(szBuf);

#define RedirectToShowChannelPage \
					url.clear();\
					url.setPath(ShowChannelPage);\
					url.setVar(TemplateKey, _varMap[TemplateKey].c_str());\
					url.setVar(PublisherKey, _varMap[PublisherKey].c_str());\
					url.setVar(ChannelNameKey, chnlName.c_str());\
					responser<<"<script language=\"javascript\">";\
					snprintf(szBuf, sizeof(szBuf) - 1, "document.location.href=\"%s\"", String::getRightStr(url.generate(), "/", false).c_str());\
					responser<<szBuf;\
					responser<<"</script>";

#define LinkSpace responser<<" | ";
#define LinkSpaceError addToLastError(" | ");

namespace BcastWebPage
{
	class BasePage
	{
	public: 
		BasePage(IHttpRequestCtx* pHttpRequestCtx);
		virtual ~BasePage();

	public: 
		const char* getLastError() const;
		void setLastError(const char* error);
		void addToLastError(const char* error);
		bool process();

	protected:
		bool isPostBack() const;
		virtual bool get() = 0;
		virtual bool post() = 0;

	protected: 
		std::string _lastError;
		char szBuf[1024];
		ZQ::common::URLStr url;

		::Ice::CommunicatorPtr _gComm;

		IHttpRequestCtx* _reqCtx;
		IHttpRequestCtx::RequestVars _varMap;		

		TianShanIce::Application::Broadcast::BcastPublisherPrx chnlPub;

	};
}

#endif // __BcastWebPage_BasePage_H__

