#ifndef __ClibWebPage_Main_H__
#define __ClibWebPage_Main_H__

#include "BasePage.h"

namespace ClibWebPage
{
	class ClibMain: public BasePage
	{
	public:
		ClibMain(IHttpRequestCtx* pHttpRequestCtx);
		virtual ~ClibMain();
	protected: 
		virtual bool get();
		virtual bool post();
	};
}

#endif // __ClibWebPage_Main_H__

