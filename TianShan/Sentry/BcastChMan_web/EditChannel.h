#ifndef __BcastWebPage_EditChannel_H__
#define __BcastWebPage_EditChannel_H__

#include "BasePage.h"

namespace BcastWebPage
{
	class EditChannel : public BasePage
	{
	public: 
		EditChannel(IHttpRequestCtx* pHttpRequestCtx);
		virtual ~EditChannel();

	protected: 
		virtual bool get();
		virtual bool post();
	};
}

#endif // __BcastWebPage_EditChannel_H__