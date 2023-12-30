#ifndef __CodWebPage_EditChannel_H__
#define __CodWebPage_EditChannel_H__

#include "BasePage.h"

namespace CodWebPage
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

#endif // __CodWebPage_EditChannel_H__

