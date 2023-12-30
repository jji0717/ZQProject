#include "CodMain.h"


namespace CodWebPage
{

	CodMain::CodMain(IHttpRequestCtx* pHttpRequestCtx) : BasePage(pHttpRequestCtx)
	{
	}

	CodMain::~CodMain()
	{
	}

	bool CodMain::get()
	{
		IHttpResponse& responser = _reqCtx->Response();

//		TianShanIce::StrValues chnlList;
//		try
//		{
//			chnlList = chnlPub->list();
//		}
//		catch (const TianShanIce::ServerError& ex)
//		{
//			_snprintf(szBuf, sizeof(szBuf) - 1, "list channels caught %s:%s", ex.ice_name().c_str(), ex.message.c_str());
//			setLastError(szBuf);
//			glog(ErrorLog, CLOGFMT(CodMain, "%s"), getLastError());
//			responser.SetLastError(getLastError());
//			return false;
//		}
//		catch (const Ice::Exception& ex)
//		{
//			_snprintf(szBuf, sizeof(szBuf) - 1, "list channels caught %s", ex.ice_name().c_str());
//			setLastError(szBuf);
//			glog(ErrorLog, CLOGFMT(CodMain, "%s"), getLastError());
//			responser.SetLastError(getLastError());
//			return false;
//		}

		unsigned int i = 0, count = 0;
		TianShanIce::Application::PublishPointInfos chnlInfos;
		TianShanIce::StrValues params;
		params.push_back("maxBitrate");
		params.push_back("desc");
		try
		{
			chnlInfos = chnlPub->listOnDemandPointInfo("*", params);
		}
		catch (const TianShanIce::BaseException& ex)
		{
			_snprintf(szBuf, sizeof(szBuf) - 1, "listChannelInfo caught %s:%s", ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(CodMain, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const Ice::Exception& ex)
		{
			_snprintf(szBuf, sizeof(szBuf) - 1, "listChannelInfo caught %s", ex.ice_name().c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(CodMain, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}

		// write page's functionality
		responser<<"<H2>Main page</H2>";

//		<table width="80%" border="1">
//			<tr> 
//				<th>www.dreamdu.com</th>
//				<th>.com域名的数量</th>
//				<th>.cn域名的数量</th>
//				<th>.net域名的数量</th>
//			</tr>
//			<tr>
//				<td>2003年</td>
//				<td>1000</td>
//				<td>2000</td>
//				<td>3000</td>
//			</tr>
//		</table>

		// write start table flag
		responser<<"<table class='listTable'>";

		// write table header line
		responser<<"<tr class='heading'>";
		responser<<"	<th>Edit</th>";
		responser<<"	<th>Remove</th>";
		responser<<"	<th>ChannelName</center></th>";
		responser<<"	<th>OnDemandName</center></th>";
		for (i = 0, count = params.size(); i < count; i ++)
		{
			_snprintf(szBuf, sizeof(szBuf) - 1, "	<th>%s</th>", params[i].c_str());
			responser<<szBuf;
		}
		responser<<"</tr>";

		// write table data line
		for (i = 0, count = chnlInfos.size(); i < count; i ++)
		{
			responser<<"<tr>";

			// a link to edit channel page
			url.clear();
			url.setPath(EditChannelPage);
			url.setVar(ChannelNameKey, chnlInfos[i].name.c_str());
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
//			<Img src='img/xxx.gif' href='' alt='edit'>
			_snprintf(szBuf, sizeof(szBuf) - 1, "		<td><a href=\"%s\">Edit</a></td>", String::getRightStr(url.generate(), "/", false).c_str());
			responser<<szBuf;

			// a link to remove channel page
			url.clear();
			url.setPath(RemoveChannelPage);
			url.setVar(ChannelNameKey, chnlInfos[i].name.c_str());
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
			_snprintf(szBuf, sizeof(szBuf) - 1, "		<td><a href=\"%s\">Remove</a></td>", String::getRightStr(url.generate(), "/", false).c_str());
			responser<<szBuf;

			// channel name
			url.clear();
			url.setPath(ShowChannelPage);
			url.setVar(ChannelNameKey, chnlInfos[i].name.c_str());
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
			_snprintf(szBuf, sizeof(szBuf) - 1, "		<td><a href=\"%s\">%s</a></td>", String::getRightStr(url.generate(), "/", false).c_str(), chnlInfos[i].name.c_str());
			responser<<szBuf;

			// on demand name
			_snprintf(szBuf, sizeof(szBuf) - 1, "		<td>%s</td>", chnlInfos[i].type.c_str());
			responser<<szBuf;

			TianShanIce::StrValues::const_iterator itor = params.begin();
			while (params.end() != itor)
			{
				_snprintf(szBuf, sizeof(szBuf) - 1, "		<td>%s</td>", chnlInfos[i].params[*itor].c_str());
				responser<<szBuf;
				itor ++;
			}

			responser<<"</tr>";
		}

		// write end table flag
		responser<<"</table>";
		responser<<"<br>";

		// a link to add new channel page
		url.clear();
		url.setPath(AddChannelPage);
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		_snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>New channel</B></a>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;
		// a link to add new channel page
		LinkSpace;
		url.clear();
		url.setPath(CodMainPage);
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		_snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Refresh</B></a>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;

		return true;
	}

	bool CodMain::post()
	{
		return true;
	}

} // namespace CodWebPage

