#include "ErmMain.h"

namespace ErmWebPage
{

	ErmMain::ErmMain(IHttpRequestCtx* pHttpRequestCtx) : BasePage(pHttpRequestCtx)
	{
	}
	ErmMain::~ErmMain()
	{
	}

	bool ErmMain::get()
	{
		IHttpResponse& responser = _reqCtx->Response();

		// write page's functionality
		responser<<"<H2>Main page</H2>";

		responser<<"<br>";

		url.clear();
		url.setPath(ShowDevicePage);
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Show Device</B></a>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;

		LinkSpace;
		url.clear();
		url.setPath(ShowChannelPage);
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Show Channel</B></a>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;

		LinkSpace;
		url.clear();
		url.setPath(ShowAllocationPage);
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Show Allocation</B></a>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;

		LinkSpace;
		url.clear();
		url.setPath(ShowRouteNamesPage);
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Show RouteNames</B></a>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;


		LinkSpace;
		url.clear();
		url.setPath(ErmMainPage);
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Refresh</B></a>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;

		return true;
	}

	bool ErmMain::post()
	{
		return true;
	}

} // namespace ErmWebPage