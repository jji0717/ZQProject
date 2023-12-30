#ifndef __BcastWebPage_BcastMain_H__
#define __BcastWebPage_BcastMain_H__

#include "BasePage.h"

namespace BcastWebPage
{
	class BcastMain : public BasePage
	{
	public: 
		BcastMain(IHttpRequestCtx* pHttpRequestCtx);
		virtual ~BcastMain();

	protected: 
		virtual bool get();
		virtual bool post();
	};
}

#endif // __BcastWebPage_BcastMain_H__