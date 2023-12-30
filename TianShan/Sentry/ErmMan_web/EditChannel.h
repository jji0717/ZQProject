#ifndef __ErmWebPage_EditChannel_H__
#define __ErmWebPage_EditChannel_H__

#include "BasePage.h"

namespace ErmWebPage
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

#endif // __ErmWebPage_EditChannel_H__

