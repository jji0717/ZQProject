#ifndef __ClibWebPage_BasePage_H__
#define __ClibWebPage_BasePage_H__

#define _WINSOCK2API_

#include <string>
#include <Ice/Ice.h>
#include "../httpdInterface.h"
#include <Log.h>
#include "stroprt.h"
#include "urlstr.h"
#include "TianShanDefines.h"
#include "ContentReplicaEx.h"

#define DebugLog ZQ::common::Log::L_DEBUG
#define InfoLog ZQ::common::Log::L_INFO
#define NoticeLog ZQ::common::Log::L_NOTICE
#define WarningLog ZQ::common::Log::L_WARNING
#define ErrorLog ZQ::common::Log::L_ERROR
#define CritLog ZQ::common::Log::L_CRIT
#define ALertLog ZQ::common::Log::L_ALERT
#define EmergLog ZQ::common::Log::L_EMERG

#define MetaVolumeKey			"clib#volume"
#define ContentReplicaKey		"clib#content"
#define StoreReplicaKey			"clib#store"
#define PublisherKey			"clib#pubep"
#define TemplateKey				"#template"
#define FunctionKey				"#function"
#define TabKey					"#tab"

#define ClibMainPage			"ClibMain.clibweb.tswl"
#define ShowContentPage			"ShowContent.clibweb.tswl"
#define ShowVolumePage			"ShowVolume.clibweb.tswl"
#define ContentDetailPage		"ContentDetail.clibweb.tswl"

#define SplitNetIdChar			";"

#define LinkSpace responser<<" | ";
#define LinkSpaceError addToLastError(" | ");

#define CountPerPage 30

namespace ClibWebPage
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

		::TianShanIce::Repository::ContentLibPrx _lib;
	};
}

#endif // __ClibWebPage_BasePage_H__

