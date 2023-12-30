#ifndef __contentsession_BasePage_H__
#define __contentsession_BasePage_H__

#define _WINSOCK2API_

#include <Ice/Ice.h>
#include "../httpdInterface.h"
#include <Log.h>
#include <urlstr.h>
#include "stroprt.h"
#include <TianShanDefines.h>
#include "TsContentProv.h"

#define DebugLog ZQ::common::Log::L_DEBUG
#define InfoLog ZQ::common::Log::L_INFO
#define NoticeLog ZQ::common::Log::L_NOTICE
#define WarningLog ZQ::common::Log::L_WARNING
#define ErrorLog ZQ::common::Log::L_ERROR
#define CritLog ZQ::common::Log::L_CRIT
#define ALertLog ZQ::common::Log::L_ALERT
#define EmergLog ZQ::common::Log::L_EMERG

// up# means "url parameter"
#define TemplateKey							"#template"
#define contentprovisonAddressKey			"up#ContentProvisionAddress"
#define SessionNumberPerPageKey				"up#SessionNumberPerPage"
#define LinkNumberPerPageKey				"up#LinkNumberPerPage"
#define PageSequenceKey						"up#PageSequence"
#define SessionGroupKey						"up#SessionGroup"
#define StartId					            "up#StartId"
#define MaxSessionCout						"up#MaxSessionCout"

#define SessionPagePath						"session.cpemanweb.tswl"
#define MethodPagePath						"method.cpemanweb.tswl"


namespace ContentProvisionweb
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
		char szBuf[8 * 1024];
		ZQ::common::URLStr url;

		IHttpRequestCtx* _reqCtx;
		Ice::CommunicatorPtr _ic;
		TianShanIce::ContentProvision::ContentProvisionServicePrx _ps;

		IHttpRequestCtx::RequestVars _varMap;
	};
}
#endif // __contentsession_BasePage_H__

