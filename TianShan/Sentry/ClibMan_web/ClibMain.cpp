#include "ClibMain.h"

namespace ClibWebPage
{

	ClibMain::ClibMain(IHttpRequestCtx* pHttpRequestCtx) : BasePage(pHttpRequestCtx)
	{
	}
	ClibMain::~ClibMain()
	{
	}

	bool ClibMain::get()
	{
		IHttpResponse& responser = _reqCtx->Response();

		TianShanIce::StrValues expectMetaData;
		TianShanIce::Properties searchForMetaData;
		searchForMetaData.insert(TianShanIce::Properties::value_type("objectType", "ContentStoreReplica"));
		expectMetaData.push_back("endpoint");
		expectMetaData.push_back("netId");
		TianShanIce::Repository::MetaObjectInfos infos;
		try
		{
			infos = _lib->locateContent(searchForMetaData, expectMetaData, 0, false);
		}
		catch (Ice::Exception& ex) 
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ClibMain, "locateContent() ice exception[%s]"), ex.ice_name().c_str());
		}
		catch (...)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ClibMain, "locateContent() unknown exception"));
		}
		
		// write start table flag
		responser<<"<table class='listTable'>";

		// write table header line
		responser<<"<tr>";
		responser<<"	<th><center>NetId</center></th>";
		responser<<"	<th><center>Endpoint</center></th>";
		responser<<"</tr>";

		for(TianShanIce::Repository::MetaObjectInfos::iterator it = infos.begin(); it != infos.end(); it++)
		{
			responser<<"<tr>";

			url.clear();
			url.setPath(ShowVolumePage);
			url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			url.setVar(StoreReplicaKey, it->id.c_str());
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><a href=\"%s\"><B>%s</B></a></td>", String::getRightStr(url.generate(), "/", false).c_str(), it->id.c_str());
			responser<<szBuf;

			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", it->metaDatas["endpoint"].value.c_str());
			responser<<szBuf;

			responser<<"</tr>";
		}

		// write end table flag
		responser<<"</table>";
		responser<<"<br>";

		url.clear();
		url.setPath(ClibMainPage);
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Refresh</B></a>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;

		return true;
	}

	bool ClibMain::post()
	{
		return true;
	}

} // namespace ClibWebPage
