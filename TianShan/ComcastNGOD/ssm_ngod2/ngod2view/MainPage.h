#ifndef __ngod2view_MainPage_H__
#define __ngod2view_MainPage_H__

#include "BasePage.h"

#define ClientIdKey "up#ClientId"
#define SessionCountKey "up#SessionCount"

namespace ngod2view
{
	class MainPage : public BasePage
	{
	public: 
		MainPage(IHttpRequestCtx* pHttpRequestCtx);
		virtual ~MainPage();

	protected: 
		virtual bool get();
		virtual bool post();

		Ice::Int _sessCount;
		unsigned int _curPage;
		unsigned int _pageCount;
		unsigned int _sessionNumberPerPage;
		unsigned int _linkNumberPerPage;
		::Ice::Int _clientId;

	};
}

#endif // __ngod2view_MainPage_H__

