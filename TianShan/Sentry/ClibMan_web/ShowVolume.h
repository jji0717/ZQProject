#ifndef __ClibWebPage_ShowVolume_H__
#define __ClibWebPage_ShowVolume_H__

#include "BasePage.h"

namespace ClibWebPage
{

	class ShowVolume: public BasePage
	{
	public:
		ShowVolume(IHttpRequestCtx* pHttpRequestCtx);
		virtual ~ShowVolume();
	protected: 
		virtual bool get();
		virtual bool post();
	private:
		TianShanIce::Repository::MetaObjectInfos AllInfos;
		unsigned int _pageCount;
		unsigned int _volumeNumberPerPage;
	};

}

#endif // __ClibWebPage_ShowVolume_H__
