/*
#include "ShowServiceGroup.h"
#include "DataTypes.h"
#include "EdgeRM.h"
#include "TsEdgeResource.h"
namespace ErmWebPage
{
	ShowServiceGroup::ShowServiceGroup(IHttpRequestCtx* pHttpRequestCtx) : BasePage(pHttpRequestCtx)
	{
	}

	ShowServiceGroup::~ShowServiceGroup()
	{
	}

	bool ShowServiceGroup::get()
	{
		IHttpResponse& responser = _reqCtx->Response();

		url.clear();
		url.setPath(ShowServiceGroupPage);
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		snprintf(szBuf, sizeof(szBuf) - 1, "<br><a href=\"%s\"><B>Refresh</B></a>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;

		// write start table flag
		responser<<"<table class='listTable'>";

		// write table header line
		responser<<"<tr>";
		responser<<"	<th><center>ServiceGroup</center></th>";
		responser<<"	<th><center>RF Port</center></th>";
		responser<<"</tr>";

		TianShanIce::EdgeResource::ObjectInfos objInfos;

		try
		{
			objInfos = _ERM->listServiceGroups();
		}
		catch (const TianShanIce::BaseException& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "list Service Group caught %s:%s<br>", 
				ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(ShowDevice, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const Ice::Exception& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "list Service Group caught %s<br>", 
				ex.ice_name().c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(ShowDevice, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}
		// write table data line
		for (TianShanIce::EdgeResource::ObjectInfos::iterator it = objInfos.begin(); it != objInfos.end(); it++)
		{

			std::string serviceGroup; 
			serviceGroup = _varMap[ServiceGroupKey];
			if(!serviceGroup.empty() && it->ident.name!=serviceGroup)
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
			url.setVar(ServiceGroupKey, it->ident.name.c_str());
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><a href=\"%s\">%s</a></center></td>", String::getRightStr(url.generate(), "/", false).c_str(), it->props["RFPortCount"].c_str());
			responser<<szBuf;

			responser<<"</tr>";
		}

		// write end table flag
		responser<<"</table>";
		responser<<"<br>";

		return true;
	}

	bool ShowServiceGroup::post()
	{
		return true;
	}
} // namespace ErmWebPage
*/
