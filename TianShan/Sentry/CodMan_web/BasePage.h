#ifndef __CodWebPage_BasePage_H__
#define __CodWebPage_BasePage_H__

#define _WINSOCK2API_

//#define USE_OLD_NS  // The switch to use the old namespace

#ifdef USE_OLD_NS
#  include "ChannelOnDemand.h"
#  define  NS_PREFIX(_CLS) _CLS
#  define  CI_NS_PREFIX(_CLS) ChannelOnDemand::_CLS
#else
#  include "TsAppChOD.h"
#  define  NS_PREFIX(_CLS) TianShanIce::Application::_CLS
#  define  CI_NS_PREFIX(_CLS) TianShanIce::Application::_CLS
#endif // USE_OLD_NS

#include <string>
#include <Ice/Ice.h>
#include "../httpdInterface.h"
#include <Log.h>
#include "./stroprt.h"
#include "./ChannelOnDemandEx.h"
#include "./DataTypes.h"
#include "urlstr.h"

#define DebugLog ZQ::common::Log::L_DEBUG
#define InfoLog ZQ::common::Log::L_INFO
#define NoticeLog ZQ::common::Log::L_NOTICE
#define WarningLog ZQ::common::Log::L_WARNING
#define ErrorLog ZQ::common::Log::L_ERROR
#define CritLog ZQ::common::Log::L_CRIT
#define ALertLog ZQ::common::Log::L_ALERT
#define EmergLog ZQ::common::Log::L_EMERG

#define ChannelNameKey			"cod#chnl"
#define ItemNameKey				"cod#item"
#define PublisherKey			"cod#pubep"
#define TemplateKey				"#template"

#define CodMainPage				"CodMain.codweb.tswl"
#define ShowChannelPage			"ShowChannel.codweb.tswl"
#define AddChannelPage			"AddChannel.codweb.tswl"
#define EditChannelPage			"EditChannel.codweb.tswl"
#define RemoveChannelPage		"RemoveChannel.codweb.tswl"
#define PushItemPage			"PushItem.codweb.tswl"
#define InsertItemPage			"InsertItem.codweb.tswl"
#define ShowItemPage			"ShowItem.codweb.tswl"
#define RemoveItemPage			"RemoveItem.codweb.tswl"
#define EditItemPage			"EditItem.codweb.tswl"

#define SplitNetIdChar			";"

#define LinkToCodMainPageRight	\
					url.clear();\
					url.setPath(CodMainPage);\
					url.setVar(PublisherKey, _varMap[PublisherKey].c_str());\
					url.setVar(TemplateKey, _varMap[TemplateKey].c_str());\
					snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Main page</B></a>", String::getRightStr(url.generate(), "/", false).c_str());\
					responser<<szBuf;

#define LinkToCodMainPageError	\
					url.clear();\
					url.setPath(CodMainPage);\
					url.setVar(PublisherKey, _varMap[PublisherKey].c_str());\
					url.setVar(TemplateKey, _varMap[TemplateKey].c_str());\
					_snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Main page</B></a>", String::getRightStr(url.generate(), "/", false).c_str());\
					addToLastError(szBuf);

#define RedirectToCodMainPage \
					url.clear();\
					url.setPath(CodMainPage);\
					url.setVar(TemplateKey, _varMap[TemplateKey].c_str());\
					url.setVar(PublisherKey, _varMap[PublisherKey].c_str());\
					responser<<"<script language=\"javascript\">";\
					_snprintf(szBuf, sizeof(szBuf) - 1, "document.location.href=\"%s\"", String::getRightStr(url.generate(), "/", false).c_str());\
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
					_snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Return to channel: [%s]</B></a>", String::getRightStr(url.generate(), "/", false).c_str(), chnlName.c_str());\
					addToLastError(szBuf);

#define RedirectToShowChannelPage \
					url.clear();\
					url.setPath(ShowChannelPage);\
					url.setVar(TemplateKey, _varMap[TemplateKey].c_str());\
					url.setVar(PublisherKey, _varMap[PublisherKey].c_str());\
					url.setVar(ChannelNameKey, chnlName.c_str());\
					responser<<"<script language=\"javascript\">";\
					_snprintf(szBuf, sizeof(szBuf) - 1, "document.location.href=\"%s\"", String::getRightStr(url.generate(), "/", false).c_str());\
					responser<<szBuf;\
					responser<<"</script>";

#define LinkSpace responser<<" | ";
#define LinkSpaceError addToLastError(" | ");

namespace CodWebPage
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

		TianShanIce::Application::ChannelOnDemand::ChannelPublisherPrx chnlPub;

	};
}

#endif // __CodWebPage_BasePage_H__

