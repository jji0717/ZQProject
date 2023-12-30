#include "ContentDetail.h"


namespace ClibWebPage
{
	ContentDetail::ContentDetail(IHttpRequestCtx* pHttpRequestCtx) : BasePage(pHttpRequestCtx)
	{
	}
	ContentDetail::~ContentDetail()
	{
	}
	bool ContentDetail::get()
	{
		IHttpResponse& responser = _reqCtx->Response();

		std::string netId = "";
		std::string volumeName = "";
		std::string contentName = "";
		if(_varMap.find(StoreReplicaKey) != _varMap.end())
		{
			netId = _varMap[StoreReplicaKey];
		}
		if(_varMap.find(MetaVolumeKey) != _varMap.end())
		{
			volumeName = _varMap[MetaVolumeKey];
		}
		if(_varMap.find(ContentReplicaKey) != _varMap.end())
		{
			contentName = _varMap[ContentReplicaKey].substr(0, _varMap[ContentReplicaKey].find("@"));
		}

		::TianShanIce::Repository::MetaDataMap metaDataMap;
		try
		{
			::TianShanIce::Repository::ContentReplicaPrx contentPrx = _lib->toContentReplica(netId + "$" + volumeName + "/" + contentName);
			metaDataMap = ::TianShanIce::Repository::ContentReplicaExPrx::checkedCast(contentPrx)->getMetaDataMap();
		}
		catch (...)
		{
			return false;
		}

		// write page's functionality
		snprintf(szBuf, sizeof(szBuf) - 1, "<H2>Asset Detail [%s$%s/%s]</H2>", netId.c_str(), volumeName.c_str(), contentName.c_str());
		responser<<szBuf;

		responser<<"<fieldset>";

		responser<<"	<label for=\"ProviderId\">ProviderId:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", metaDataMap["user.ProviderId"].value.c_str());
		responser<<szBuf;
		responser<<"<BR>";

		responser<<"	<label for=\"ProviderAssetId\">ProviderAssetId:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", metaDataMap["user.ProviderAssetId"].value.c_str());
		responser<<szBuf;
		responser<<"<BR>";

		responser<<"	<label for=\"nPVRCopy\">nPVRCopy:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", metaDataMap["user.nPVRCopy"].value.c_str());
		responser<<szBuf;
		responser<<"<BR>";

		responser<<"	<label for=\"SubscriberId\">SubscriberId:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", metaDataMap["user.SubscriberId"].value.c_str());
		responser<<szBuf;
		responser<<"<BR>";

		responser<<"	<label for=\"ResponseURL\">ResponseURL:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", metaDataMap["user.ResponseURL"].value.c_str());
		responser<<szBuf;
		responser<<"<BR>";

		responser<<"	<label for=\"SourceUrl\">SourceUrl:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", metaDataMap["sys.SourceUrl"].value.c_str());
		responser<<szBuf;
		responser<<"<BR>";

		responser<<"	<label for=\"ParentName\">ParentName:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", metaDataMap["sys.ParentName"].value.c_str());
		responser<<szBuf;
		responser<<"<BR>";

		responser<<"	<label for=\"Comment\">Comment:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", metaDataMap["sys.Comment"].value.c_str());
		responser<<szBuf;
		responser<<"<BR>";

		responser<<"	<label for=\"FileSize\">FileSize:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", metaDataMap["sys.FileSize"].value.c_str());
		responser<<szBuf;
		responser<<"<BR>";

		responser<<"	<label for=\"SupportFileSize\">SupportFileSize:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", metaDataMap["sys.SupportFileSize"].value.c_str());
		responser<<szBuf;
		responser<<"<BR>";

		responser<<"	<label for=\"PixelHorizontal\">PixelHorizontal:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", metaDataMap["sys.PixelHorizontal"].value.c_str());
		responser<<szBuf;
		responser<<"<BR>";

		responser<<"	<label for=\"PixelVertical\">PixelVertical:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", metaDataMap["sys.PixelVertical"].value.c_str());
		responser<<szBuf;
		responser<<"<BR>";

		responser<<"	<label for=\"BitRate\">BitRate:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", metaDataMap["sys.BitRate"].value.c_str());
		responser<<szBuf;
		responser<<"<BR>";

		responser<<"	<label for=\"PlayTime\">PlayTime:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", metaDataMap["sys.PlayTime"].value.c_str());
		responser<<szBuf;
		responser<<"<BR>";

		responser<<"	<label for=\"FrameRate\">FrameRate:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", metaDataMap["sys.FrameRate"].value.c_str());
		responser<<szBuf;
		responser<<"<BR>";

		responser<<"	<label for=\"SourceType\">SourceType:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", metaDataMap["sys.SourceType"].value.c_str());
		responser<<szBuf;
		responser<<"<BR>";

		responser<<"	<label for=\"LocalType\">LocalType:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", metaDataMap["sys.LocalType"].value.c_str());
		responser<<szBuf;
		responser<<"<BR>";

		responser<<"	<label for=\"SubType\">SubType:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", metaDataMap["sys.SubType"].value.c_str());
		responser<<szBuf;
		responser<<"<BR>";

		responser<<"	<label for=\"MD5CheckSum\">MD5CheckSum:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", metaDataMap["sys.MD5CheckSum"].value.c_str());
		responser<<szBuf;
		responser<<"<BR>";

		responser<<"	<label for=\"ScheduledProvisonStart\">ScheduledProvisonStart:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", metaDataMap["sys.ScheduledProvisonStart"].value.c_str());
		responser<<szBuf;
		responser<<"<BR>";

		responser<<"	<label for=\"ScheduledProvisonEnd\">ScheduledProvisonEnd:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", metaDataMap["sys.ScheduledProvisonEnd"].value.c_str());
		responser<<szBuf;
		responser<<"<BR>";

		responser<<"	<label for=\"MaxProvisonBitRate\">MaxProvisonBitRate:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", metaDataMap["sys.MaxProvisonBitRate"].value.c_str());
		responser<<szBuf;
		responser<<"<BR>";

		responser<<"	<label for=\"nPVRLeadCopy\">nPVRLeadCopy:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", metaDataMap["sys.nPVRLeadCopy"].value.c_str());
		responser<<szBuf;
		responser<<"<BR>";

		responser<<"	<label for=\"StampLastUpdated\">StampLastUpdated:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", metaDataMap["sys.StampLastUpdated"].value.c_str());
		responser<<szBuf;
		responser<<"<BR>";

		responser<<"	<label for=\"StampCreated\">StampCreated:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", metaDataMap["sys.StampCreated"].value.c_str());
		responser<<szBuf;
		responser<<"<BR>";

		responser<<"	<label for=\"State\">State:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", metaDataMap["sys.State"].value.c_str());
		responser<<szBuf;
		responser<<"<BR>";

		responser<<"</fieldset>";

		url.clear();
		url.setPath(ShowContentPage);
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		if(_varMap.find(StoreReplicaKey) != _varMap.end())
		{
			url.setVar(StoreReplicaKey, _varMap[StoreReplicaKey].c_str());
		}
		if(_varMap.find(MetaVolumeKey) != _varMap.end())
		{
			url.setVar(MetaVolumeKey, _varMap[MetaVolumeKey].c_str());
		}
		snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Back to Content List</B></a>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;

		return true;
	}

	bool ContentDetail::post()
	{
		return true;
	}
}
