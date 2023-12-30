#include "ShowRouteNames.h"
#include "DataTypes.h"
#include "EdgeRM.h"
#include "TsEdgeResource.h"

namespace ErmWebPage
{
	ShowRouteNames::ShowRouteNames(IHttpRequestCtx* pHttpRequestCtx) : BasePage(pHttpRequestCtx)
	{
	}

	ShowRouteNames::~ShowRouteNames()
	{
	}

	bool ShowRouteNames::get()
	{
		IHttpResponse& responser = _reqCtx->Response();

		url.clear();
		url.setPath(ShowRouteNamesPage);
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		snprintf(szBuf, sizeof(szBuf) - 1, "<br><a href=\"%s\"><B>Refresh</B></a>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;

		// write start table flag
		responser<<"<table class='listTable'>";

		// write table header line
		responser<<"<tr>";
		responser<<"	<th><center>RouteNames</center></th>";
		responser<<"	<th><center>RF Port</center></th>";
		responser<<"</tr>";

		TianShanIce::EdgeResource::ObjectInfos objInfos;

		try
		{
			objInfos = _ERM->listRouteNames();
		}
		catch (const TianShanIce::BaseException& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "list Route Names caught %s:%s<br>", 
				ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(ShowDevice, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const Ice::Exception& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "list Route Names caught %s<br>", 
				ex.ice_name().c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(ShowDevice, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}
		// write table data line
		for (TianShanIce::EdgeResource::ObjectInfos::iterator it = objInfos.begin(); it != objInfos.end(); it++)
		{

			std::string routeName; 
			routeName = _varMap[RouteNamesKey];
			if(!routeName.empty() && it->ident.name!=routeName)
				continue;

			responser<<"<tr>";

			// show zone
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", it->ident.name.c_str());
			responser<<szBuf;

			// show a link to list port
			url.clear();
			url.setPath(ShowEdgePortPage);
			url.setVar(TemplateKey, neighborLayout(_varMap[TemplateKey].c_str(), "LOCAL_ERM_PORT").c_str());
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			url.setVar(RouteNamesKey, it->ident.name.c_str());
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><a href=\"%s\">%s</a></center></td>", String::getRightStr(url.generate(), "/", false).c_str(), it->props["RFPortCount"].c_str());
			responser<<szBuf;

			responser<<"</tr>";
		}

		// write end table flag
		responser<<"</table>";
		responser<<"<br>";

		return true;
	}

	bool ShowRouteNames::post()
	{
		return true;
	}
} // namespace ErmWebPage

