#include "MainPage.h"


namespace ngod2view
{

MainPage::MainPage(IHttpRequestCtx* pHttpRequestCtx) : BasePage(pHttpRequestCtx)
{
}

MainPage::~MainPage()
{
}

bool MainPage::get()
{
	IHttpResponse& responser = _reqCtx->Response();

	char buff[MAX_PATH];
	memset(buff, 0, sizeof(buff));

	// --------------------------------------------------------------------
	// 取得配置中的值
	// --------------------------------------------------------------------
	_sessionNumberPerPage = atoi(_varMap[SessionNumberPerPageKey].c_str());
	if (_sessionNumberPerPage == 0)
	{
		// 如果没有该key，则默认每页显示50条记录
		_sessionNumberPerPage = 50;
	}
#ifndef _DEBUG
	if (_sessionNumberPerPage < 5)
	{
		// 如果配置小于5，调整为5
		_sessionNumberPerPage = 5;
	}
#endif
	_linkNumberPerPage = atoi(_varMap[LinkNumberPerPageKey].c_str());
	if (_linkNumberPerPage == 0)
	{
		// 如果没有该key，则默认显示10个页面链接
		_linkNumberPerPage = 10;
	}
#ifndef _DEBUG
	if (_linkNumberPerPage < 5)
	{
		// 如果配置小于5，调整为5
		_linkNumberPerPage = 5;
	}
#endif

	// --------------------------------------------------------------------
	// 取得页面自身保存的参数，第一次访问该页面时，这些值肯定没有。
	// --------------------------------------------------------------------
	_clientId	= atoi(_varMap[ClientIdKey].c_str());
	_sessCount	= atoi(_varMap[SessionCountKey].c_str());
	_curPage	= atoi(_varMap[PageSequenceKey].c_str());

	NGOD::CtxDatas data;
	int32 iTimeout; // 存放session的timeout值
	try
	{
		// 活动服务器端session的timeout值，供显示session状态使用
		iTimeout = _viewPrx->getTimeoutValue();

		// --------------------------------------------------------------------
		// 如果_curPage == 0，说明是第一次访问该页面
		// 因为该页面生成的PageSequence从1开始
		// --------------------------------------------------------------------
		if (_curPage == 0)
		{
			// 活动当前所有session的快照
			// 并获得clientId
			_curPage = 1;
			_sessCount = _viewPrx->getAllContexts(_clientId);
			if (_sessCount <= 0)
			{
				sprintf(buff, "There is no sessions now, please refresh this page by clicking the page tab.");
				responser<<buff;
				return true;
			}
		}

		// --------------------------------------------------------------------
		// 如果请求里面有SessionGroup且不为空
		// --------------------------------------------------------------------
		if (_varMap[SessionGroupKey].size())
		{
			::Ice::Context ctx;
			memset(buff, 0, sizeof(buff));
			sprintf(buff, "%d", _clientId);
			ctx["ctx#ClientId"] = buff;
			_sessCount = _viewPrx->getContextsBySG(_varMap[SessionGroupKey], ctx);
			if (_sessCount <= 0)
			{
				sprintf(buff, "There is no sessions under SessionGroup[%s] in current snapshot", _varMap[SessionGroupKey].c_str());
				responser<<buff;
				// show a link for return
				url.clear();
				url.setPath(MainPagePath);
				url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
				url.setVar(SessionNumberPerPageKey, _varMap[SessionNumberPerPageKey].c_str());
				url.setVar(LinkNumberPerPageKey, _varMap[LinkNumberPerPageKey].c_str());
				url.setVar(Ngod2BindAddressKey, _varMap[Ngod2BindAddressKey].c_str());
				url.setVar(SessionGroupKey, ""); // 注意这里SessionGroup为空，也可以省略该句
				url.setVar(PageSequenceKey, "1");
				url.setVar(ClientIdKey, _varMap[ClientIdKey].c_str());
				url.setVar(SessionCountKey, _varMap[SessionCountKey].c_str());
				SNPRINTF(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\">Return</a>", ZQ::StringOperation::getRightStr(url.generate(), "/", false).c_str());
				responser<<szBuf;
				return true;
			}
		}

		// 计算分页数量
		_pageCount = _sessCount / _sessionNumberPerPage;
		if (_sessCount % _sessionNumberPerPage != 0)
			_pageCount ++;

		// 如果_curPage不再合理返回，调整之
		if (_curPage < 1)
			_curPage = 1;
		if (_curPage > _pageCount)
			_curPage = _pageCount;

		::Ice::Context ctx;
		memset(buff, 0, sizeof(buff));
		sprintf(buff, "%d", _clientId);
		ctx["ctx#ClientId"] = buff;
		if (_varMap[SessionGroupKey].size())
		{
			data = _viewPrx->getRangeBySG((_curPage - 1) * _sessionNumberPerPage + 1, _curPage * _sessionNumberPerPage, _varMap[SessionGroupKey], ctx);
		}
		else 
		{
			data = _viewPrx->getRange((_curPage - 1) * _sessionNumberPerPage + 1, _curPage * _sessionNumberPerPage, ctx);
		}
	}
	catch (const TianShanIce::ClientError& ex)
	{
		SNPRINTF(szBuf, sizeof(szBuf) - 1, "Get session list caught %s: %s<br>", ex.ice_name().c_str(), ex.message.c_str());
		setLastError(szBuf);//
		SNPRINTF(szBuf, sizeof(szBuf) - 1, "Refresh page. if this error occurs again, there is must be something wrong with the server");
		addToLastError(szBuf);
		glog(ErrorLog, CLOGFMT(MainPage, "%s"), getLastError());
		responser.SetLastError(getLastError());
		return false;
	}
	catch (const Ice::Exception& ex)
	{
		SNPRINTF(szBuf, sizeof(szBuf) - 1, "Get session list caught %s<br>", ex.ice_name().c_str());
		setLastError(szBuf);
		glog(ErrorLog, CLOGFMT(MainPage, "%s"), getLastError());
		responser.SetLastError(getLastError());
		return false;
	}

	// write page's functionality
	responser<<"<H2>Sessions List</H2>";
	responser<<"<table class='listTable'>";

	// write table header line
	responser<<"<tr class='heading'>";
	responser<<"	<th>SessionID</th>";
	responser<<"	<th>Status</th>";
	responser<<"	<th>UsedBandWidth</th>";
	responser<<"	<th>SessionGroup</th>";
	responser<<"	<th>Expiration</th>";
	responser<<"	<th>Stream</th>";
	responser<<"</tr>";

	// write table data line
	unsigned int i, count;
	for (i = 0, count = data.size(); i < count; i ++)
	{
		responser<<"<tr>";

		SNPRINTF(szBuf, sizeof(szBuf) - 1, "		<td>%s</td>", data[i].ident.name.c_str());
		responser<<szBuf;

		Ice::Long lDuration = (data[i].expiration - ZQTianShan::now()) / 1000;
		if (data[i].expiration - ZQTianShan::now() % 1000 != 0)
			lDuration ++;
		if (lDuration > 0)
			SNPRINTF(szBuf, sizeof(szBuf) - 1, "		<td>live</td>", data[i].ident.name.c_str());
		else if (lDuration > -2 * iTimeout)
			SNPRINTF(szBuf, sizeof(szBuf) - 1, "		<td>live(timeout)</td>", data[i].ident.name.c_str());
		else 
			SNPRINTF(szBuf, sizeof(szBuf) - 1, "		<td>terminated</td>", data[i].ident.name.c_str());
		responser<<szBuf;

		SNPRINTF(szBuf, sizeof(szBuf) - 1, "		<td>%lld</td>", data[i].usedBandwidth);
		responser<<szBuf;

		SNPRINTF(szBuf, sizeof(szBuf) - 1, "		<td>%s</td>", data[i].groupID.c_str());
		responser<<szBuf;

		SNPRINTF(szBuf, sizeof(szBuf) - 1, "		<td>%lld</td>", (data[i].expiration - ZQTianShan::now()) / 1000);
		responser<<szBuf;

		SNPRINTF(szBuf, sizeof(szBuf) - 1, "		<td>%s</td>", data[i].streamFullID.c_str());
		responser<<szBuf;

		responser<<"</tr>";
	}

	// write end table flag
	responser<<"</table>";
	responser<<"<br>";

	unsigned int fromPage = _curPage, toPage = _pageCount;
	// 调整范围
	if (_pageCount - _curPage <= _linkNumberPerPage - 1)
	{
		fromPage = _curPage;
		toPage = _pageCount;
		while (toPage - fromPage < _linkNumberPerPage - 1 && fromPage > 1)
		{
			fromPage --;
		}
	}
	else 
	{
		fromPage = _curPage;
		toPage = _curPage + _linkNumberPerPage - 1;
	}

	// show a link to first page
	url.clear();
	url.setPath(MainPagePath);
	url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
	url.setVar(SessionNumberPerPageKey, _varMap[SessionNumberPerPageKey].c_str());
	url.setVar(LinkNumberPerPageKey, _varMap[LinkNumberPerPageKey].c_str());
	url.setVar(Ngod2BindAddressKey, _varMap[Ngod2BindAddressKey].c_str());
	url.setVar(SessionGroupKey, _varMap[SessionGroupKey].c_str());
	url.setVar(PageSequenceKey, "1");
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "%d", _clientId);
	url.setVar(ClientIdKey, buff);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "%d", _sessCount);
	url.setVar(SessionCountKey, buff);
	if (_curPage == 1)
		SNPRINTF(szBuf, sizeof(szBuf) - 1, "First | ");
	else 
		SNPRINTF(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\">First</a> | ", ZQ::StringOperation::getRightStr(url.generate(), "/", false).c_str());
	responser<<szBuf;

	// show previous page
	unsigned int pre_Page = _curPage;
	if (_curPage != 1)
		pre_Page --;
	url.clear();
	url.setPath(MainPagePath);
	url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
	url.setVar(SessionNumberPerPageKey, _varMap[SessionNumberPerPageKey].c_str());
	url.setVar(LinkNumberPerPageKey, _varMap[LinkNumberPerPageKey].c_str());
	url.setVar(Ngod2BindAddressKey, _varMap[Ngod2BindAddressKey].c_str());
	url.setVar(SessionGroupKey, _varMap[SessionGroupKey].c_str());
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "%d", pre_Page);
	url.setVar(PageSequenceKey, buff);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "%d", _clientId);
	url.setVar(ClientIdKey, buff);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "%d", _sessCount);
	url.setVar(SessionCountKey, buff);
	if (_curPage == 1)
		SNPRINTF(szBuf, sizeof(szBuf) - 1, "Previous | ");
	else 
		SNPRINTF(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\">Previous</a> | ", ZQ::StringOperation::getRightStr(url.generate(), "/", false).c_str());
	responser<<szBuf;

	// show current page links
	unsigned int cur_fromPage = fromPage;
	unsigned int cur_toPage = toPage;
	while (cur_fromPage <= cur_toPage)
	{
		url.clear();
		url.setPath(MainPagePath);
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		url.setVar(SessionNumberPerPageKey, _varMap[SessionNumberPerPageKey].c_str());
		url.setVar(LinkNumberPerPageKey, _varMap[LinkNumberPerPageKey].c_str());
		url.setVar(Ngod2BindAddressKey, _varMap[Ngod2BindAddressKey].c_str());
		url.setVar(SessionGroupKey, _varMap[SessionGroupKey].c_str());
		memset(buff, 0, sizeof(buff));
		sprintf(buff, "%d", cur_fromPage);
		url.setVar(PageSequenceKey, buff);
		memset(buff, 0, sizeof(buff));
		sprintf(buff, "%d", _clientId);
		url.setVar(ClientIdKey, buff);
		memset(buff, 0, sizeof(buff));
		sprintf(buff, "%d", _sessCount);
		url.setVar(SessionCountKey, buff);
		if (_curPage == cur_fromPage)
			SNPRINTF(szBuf, sizeof(szBuf) - 1, "%d | ", cur_fromPage);
		else 
			SNPRINTF(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\">%d</a> | ", ZQ::StringOperation::getRightStr(url.generate(), "/", false).c_str(), cur_fromPage);
		responser<<szBuf;
		cur_fromPage ++;
	}

	// show next page
	unsigned int next_Page = _curPage;
	if (_curPage != _pageCount)
		next_Page ++;
	url.clear();
	url.setPath(MainPagePath);
	url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
	url.setVar(SessionNumberPerPageKey, _varMap[SessionNumberPerPageKey].c_str());
	url.setVar(LinkNumberPerPageKey, _varMap[LinkNumberPerPageKey].c_str());
	url.setVar(Ngod2BindAddressKey, _varMap[Ngod2BindAddressKey].c_str());
	url.setVar(SessionGroupKey, _varMap[SessionGroupKey].c_str());
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "%d", next_Page);
	url.setVar(PageSequenceKey, buff);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "%d", _clientId);
	url.setVar(ClientIdKey, buff);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "%d", _sessCount);
	url.setVar(SessionCountKey, buff);
	if (_curPage == _pageCount)
		SNPRINTF(szBuf, sizeof(szBuf) - 1, "Next | ");
	else 
		SNPRINTF(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\">Next</a> | ", ZQ::StringOperation::getRightStr(url.generate(), "/", false).c_str());
	responser<<szBuf;

	// show a link to last page
	url.clear();
	url.setPath(MainPagePath);
	url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
	url.setVar(SessionNumberPerPageKey, _varMap[SessionNumberPerPageKey].c_str());
	url.setVar(LinkNumberPerPageKey, _varMap[LinkNumberPerPageKey].c_str());
	url.setVar(Ngod2BindAddressKey, _varMap[Ngod2BindAddressKey].c_str());
	url.setVar(SessionGroupKey, _varMap[SessionGroupKey].c_str());
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "%d", _pageCount);
	url.setVar(PageSequenceKey, buff);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "%d", _clientId);
	url.setVar(ClientIdKey, buff);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "%d", _sessCount);
	url.setVar(SessionCountKey, buff);
	if (_curPage == _pageCount)
		SNPRINTF(szBuf, sizeof(szBuf) - 1, "Last");
	else 
	{
		SNPRINTF(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\">Last</a>", ZQ::StringOperation::getRightStr(url.generate(), "/", false).c_str());
	}
	responser<<szBuf;

	// show a test control to allow reposition page by inputting a number
	// RTSP, redirect to special page
	url.setPath(MainPagePath);
	url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
	url.setVar(SessionNumberPerPageKey, _varMap[SessionNumberPerPageKey].c_str());
	url.setVar(LinkNumberPerPageKey, _varMap[LinkNumberPerPageKey].c_str());
	url.setVar(Ngod2BindAddressKey, _varMap[Ngod2BindAddressKey].c_str());
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "%d", _clientId);
	url.setVar(ClientIdKey, buff);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "%d", _sessCount);
	url.setVar(SessionCountKey, buff);
	SNPRINTF(szBuf, sizeof(szBuf) - 1, "<form id=\"RTSP\" method=\"post\" action=\"%s\">", ZQ::StringOperation::getRightStr(url.generate(), "/", false).c_str());
	responser<<szBuf;
	SNPRINTF(szBuf, sizeof(szBuf) - 1, "SessionGroup: <input type=\"text\" id=\"SessionGroupText\" name=\"SessionGroupText\" value=\"%s\" />", _varMap[SessionGroupKey].c_str());
	responser<<szBuf;
	SNPRINTF(szBuf, sizeof(szBuf) - 1, "<br>Page: <input type=\"text\" id=\"PageNumberText\" name=\"PageNumberText\" value=\"%d\" />", _curPage);
	responser<<szBuf;
	responser<<"<input type=\"submit\" value=\"Go\"/>";
	responser<<"</form>";

	return true;
}

bool MainPage::post()
{
	IHttpResponse& responser = _reqCtx->Response();

	std::string sessionGroup = _varMap["SessionGroupText"];
	ZQ::StringOperation::trimAll(sessionGroup);

	std::string pageNumber = _varMap["PageNumberText"];
	ZQ::StringOperation::trimAll(pageNumber);

	url.clear();
	url.setPath(MainPagePath);
	url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
	url.setVar(SessionNumberPerPageKey, _varMap[SessionNumberPerPageKey].c_str());
	url.setVar(LinkNumberPerPageKey, _varMap[LinkNumberPerPageKey].c_str());
	url.setVar(Ngod2BindAddressKey, _varMap[Ngod2BindAddressKey].c_str());
	url.setVar(ClientIdKey, _varMap[ClientIdKey].c_str());
	url.setVar(SessionCountKey, _varMap[SessionCountKey].c_str());
	// DO: 取导航到的page的值
	url.setVar(PageSequenceKey, pageNumber.c_str());
	// DO: 取显示指定SessionGroup的值
	url.setVar(SessionGroupKey, sessionGroup.c_str());
	responser<<"<script language=\"javascript\">";
	SNPRINTF(szBuf, sizeof(szBuf) - 1, "document.location.href=\"%s\"", ZQ::StringOperation::getRightStr(url.generate(), "/", false).c_str());
	responser<<szBuf;
	responser<<"</script>";
	return true;
}

} // namespace CodWebPage

