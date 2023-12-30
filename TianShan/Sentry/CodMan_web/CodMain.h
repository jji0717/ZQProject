#ifndef __CodWebPage_CodMain_H__
#define __CodWebPage_CodMain_H__

#include "BasePage.h"

namespace CodWebPage
{
	class CodMain : public BasePage
	{
	public: 
		CodMain(IHttpRequestCtx* pHttpRequestCtx);
		virtual ~CodMain();

	protected: 
		virtual bool get();
		virtual bool post();
	};
}

#endif // __CodWebPage_CodMain_H__

