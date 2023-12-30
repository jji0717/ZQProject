#ifndef __ErmWebPage_CodMain_H__
#define __ErmWebPage_CodMain_H__

#include "BasePage.h"

namespace ErmWebPage
{

	class ErmMain: public BasePage
	{
	public:
		ErmMain(IHttpRequestCtx* pHttpRequestCtx);
		virtual ~ErmMain();

	protected: 
		virtual bool get();
		virtual bool post();
	};

}

#endif // __ErmWebPage_CodMain_H__