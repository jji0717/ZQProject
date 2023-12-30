#ifndef __ClibWebPage_ShowContent_H__
#define __ClibWebPage_ShowContent_H__

#include "BasePage.h"

namespace ClibWebPage
{

	class ShowContent: public BasePage
	{
	public:
		ShowContent(IHttpRequestCtx* pHttpRequestCtx);
		virtual ~ShowContent();
	protected: 
		virtual bool get();
		virtual bool post();
	private:
		TianShanIce::Repository::MetaObjectInfos AllInfos;
		unsigned int _pageCount;
		unsigned int _contentNumberPerPage;
	};

}

#endif // __ClibWebPage_ShowContent_H__
