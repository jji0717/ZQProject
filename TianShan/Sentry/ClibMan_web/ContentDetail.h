#ifndef __ClibWebPage_ContentDetail_H__
#define __ClibWebPage_ContentDetail_H__

#include "BasePage.h"

namespace ClibWebPage
{
	class ContentDetail: public BasePage
	{
	public:
		ContentDetail(IHttpRequestCtx* pHttpRequestCtx);
		virtual ~ContentDetail();
	protected: 
		virtual bool get();
		virtual bool post();
	};
}

#endif // __ClibWebPage_ContentDetail_H__

