#ifndef __ErmWebPage_EditQAM_H__
#define __ErmWebPage_EditQAM_H__

#include "BasePage.h"

namespace ErmWebPage
{
	class EditQAM : public BasePage
	{
	public: 
		EditQAM(IHttpRequestCtx* pHttpRequestCtx);
		virtual ~EditQAM();

	protected: 
		virtual bool get();
		virtual bool post();
	};
}

#endif // __ErmWebPage_EditQAM_H__

