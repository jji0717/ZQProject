#include "ShowContent.h"

namespace ClibWebPage
{
	ShowContent::ShowContent(IHttpRequestCtx* pHttpRequestCtx) : BasePage(pHttpRequestCtx)
	{
	}

	ShowContent::~ShowContent()
	{
	}

	bool ShowContent::get()
	{
		IHttpResponse& responser = _reqCtx->Response();

		AllInfos.clear();
		_contentNumberPerPage = CountPerPage;
		int totalCount;

		TianShanIce::StrValues expectMetaData;
		// 		expectMetaData.push_back("netId");
		// 		expectMetaData.push_back("volumeName");
		// 		expectMetaData.push_back("contentState");
		// 		expectMetaData.push_back("user.ProviderId");
		// 		expectMetaData.push_back("user.ProviderAssetId");
		TianShanIce::Properties searchForMetaData;
		::TianShanIce::Repository::MetaDataValue metaData;
		metaData.hintedType = ::TianShanIce::vtStrings;
		std::string netId = "";
		std::string volumeName = "";
		// NetId specified
		if(_varMap.find(StoreReplicaKey) != _varMap.end())
		{
			netId = _varMap[StoreReplicaKey];
			searchForMetaData.insert(TianShanIce::Properties::value_type("netId", netId));
		}
		if(_varMap.find(MetaVolumeKey) != _varMap.end())
		{
			volumeName = _varMap[MetaVolumeKey];
			searchForMetaData.insert(TianShanIce::Properties::value_type("volumeName", volumeName));
		}
		try
		{
			AllInfos = _lib->locateContentByNetIDAndVolume(netId, volumeName, expectMetaData, 0, _contentNumberPerPage, totalCount);
		}
		catch (Ice::Exception& ex) 
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ClibMain, "locateContent() ice exception[%s]"), ex.ice_name().c_str());
		}
		catch (...)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ClibMain, "locateContent() unknown exception"));
		}

		url.clear();
		url.setPath(ShowContentPage);
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());		
		snprintf(szBuf, sizeof(szBuf) - 1, "<form id='ShowContent' method='post' action='%s'><br>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;

		snprintf(szBuf, sizeof(szBuf) - 1, "<B>NetID</B><input type='text' id='net_id' name='net_id' value='%s'/>&nbsp&nbsp", netId.c_str());
		responser<<szBuf;

		snprintf(szBuf, sizeof(szBuf) - 1, "<B>Volume</B><input type='text' id='volume_name' name='volume_name' value='%s'/>&nbsp&nbsp", volumeName.c_str());
		responser<<szBuf;

		responser<<"<br>";

		snprintf(szBuf, sizeof(szBuf) - 1, "<B>AssetID</B><input type='text' id='asset_id' name='asset_id' value='%s'/>&nbsp&nbsp", "");
		responser<<szBuf;

		snprintf(szBuf, sizeof(szBuf) - 1, "<B>ProviderID</B><input type='text' id='provider_id' name='provider_id' value='%s'/>&nbsp&nbsp", "");
		responser<<szBuf;

		responser << " <select name=\"content_state\" size=\"1\" >";
		responser << "	<option value=\"asset_state\" selected>Asset State";
		responser << "	<option value=\"Pending\">Pending";
		responser << "	<option value=\"Complete\">Complete";
		responser << "	<option value=\"Canceled\">Canceled";
		responser << "	<option value=\"Failed\">Failed";
		responser << " </select>";

		// the submit button
		responser<<"	<input type='submit' value='search'/>&nbsp&nbsp\n";
		responser<<"</form>";

		int contentCount = totalCount;

		// 计算分页数量
		_pageCount = contentCount / _contentNumberPerPage;
		if (contentCount % _contentNumberPerPage != 0)
			_pageCount ++;

		url.clear();
		url.setPath(ShowContentPage);
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());	
		if(_varMap.find(StoreReplicaKey) != _varMap.end())
		{
			url.setVar("net_id", _varMap[StoreReplicaKey].c_str());
		}
		if(_varMap.find(MetaVolumeKey) != _varMap.end())
		{
			url.setVar("volume_name", _varMap[MetaVolumeKey].c_str());
		}
		std::string actionURL = String::getRightStr(url.generate(), "/", false);
		responser<<"<script type=text/javascript>";
		responser<<"function showPage(idx)";
		responser<<"{";
		responser<<"if(idx > " << (int)_pageCount << ") idx = " << (int)_pageCount << ";";
		responser<<"if(idx < 1) idx = 1;";;
		responser<<"return idx;";
		responser<<"}";
		responser<<"function showPage2(idx)";
		responser<<"{";
		responser<<"if(idx > " << (int)_pageCount << ") idx = " << (int)_pageCount << ";";
		responser<<"if(idx < 1) idx = 1;";
		responser<<"document.write('<form name=myForm>');\n";
		responser<<"var myForm=document.forms['myForm'];\n";
		responser<<"myForm.action= '";
		responser<<actionURL;
		responser<<"&currentPage='";
		responser<<"+idx;";
		responser<<"myForm.method='POST'; \n";  
		responser<<"myForm.submit(); \n";
		responser<<"}";
		responser<<"function check()";
		responser<<"{";
		responser<<"if(!(parseInt(document.getElementById('page_number').value) > 0) || (parseInt(document.getElementById('page_number').value) > " << (int)_pageCount << ") )";
		responser<<"{";
		responser<<"document.getElementById('page_number').value = '';";
		responser<<"document.getElementById('page_number').focus();";
		responser<<"return false;";
		responser<<"}";
		responser<<"return true;";
		responser<<"}";
		responser<<" </script>";

		// write start table flag
		responser<<"<table class='listTable'>";

		// write table header line
		responser<<"<tr>";
		responser<<"	<th><center>ContentName</center></th>";
		responser<<"	<th><center>NetId</center></th>";
		responser<<"	<th><center>VolumeName</center></th>";
		responser<<"	<th><center>State</center></th>";
		responser<<"	<th><center>ProviderId</center></th>";
		responser<<"	<th><center>ProviderAssetId</center></th>";
		responser<<"</tr>";

		// write table data line
		TianShanIce::Repository::MetaObjectInfos::iterator it = AllInfos.begin();
		for(unsigned int j = 0; j < _contentNumberPerPage && it < AllInfos.end(); j++)
		{
			::TianShanIce::Repository::MetaDataMap metaDataMap;
			std::string fullName = it->id;
			std::string contentName =  fullName.substr(0, fullName.find("@"));
			std::string strNetId = fullName.substr(fullName.find("@") + 1, fullName.find_first_of("$") - fullName.find("@") - 1);
			std::string volName = fullName.substr(fullName.find_first_of("$") + 1);

			try
			{
				::TianShanIce::Repository::ContentReplicaPrx contentPrx = _lib->toContentReplica(strNetId + "$" + volName + "/" + contentName);
				metaDataMap = ::TianShanIce::Repository::ContentReplicaExPrx::checkedCast(contentPrx)->getMetaDataMap();
			}
			catch (...)
			{
				it++;
				continue;
			}

			responser<<"<tr>";

			url.clear();
			url.setPath(ContentDetailPage);
			url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			url.setVar(StoreReplicaKey, strNetId.c_str());
			url.setVar(MetaVolumeKey, volName.c_str());
			url.setVar(ContentReplicaKey, it->id.c_str());
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><a href=\"%s\"><B>%s</B></a></td>", String::getRightStr(url.generate(), "/", false).c_str(), it->id.c_str());
			responser<<szBuf;

			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", strNetId.c_str());
			responser<<szBuf;

			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", volName.c_str());
			responser<<szBuf;

			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", metaDataMap["contentState"].value.c_str());
			responser<<szBuf;

			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", metaDataMap["user.ProviderId"].value.c_str());
			responser<<szBuf;

			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", metaDataMap["user.ProviderAssetId"].value.c_str());
			responser<<szBuf;

			responser<<"</tr>";
			it++;
		}

		// write end table flag
		responser<<"</table>";

		responser<<"<br>";

		if(_pageCount <= 0)
		{
			responser
				<< "<span style='width:100%'>[ no content found ]</span>"
				<< "<form style='display:inline'>";
		}
		else
		{
			responser<< "<span style='width:100%'>[<span id='cur-page'>1</span> / " << (int)_pageCount << "]</span>";

			url.clear();
			url.setPath(ShowContentPage);
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			url.setVar(TemplateKey, _varMap[TemplateKey].c_str());	
			if(_varMap.find(StoreReplicaKey) != _varMap.end())
			{
				url.setVar("net_id", _varMap[StoreReplicaKey].c_str());
			}
			if(_varMap.find(MetaVolumeKey) != _varMap.end())
			{
				url.setVar("volume_name", _varMap[MetaVolumeKey].c_str());
			}
			snprintf(szBuf, sizeof(szBuf) - 1, "<form style='display:inline' method='post' action='%s'><br>", String::getRightStr(url.generate(), "/", false).c_str());
			responser<<szBuf;
			responser << "<input id=\"page-idx\" value='1' type='hidden'>";
			// the go to page button
			snprintf(szBuf, sizeof(szBuf) - 1, "<input type='text' id='page_number' name='page_number' value='%s'/>", "");
			responser<<szBuf;
			responser<<"	<input type='submit' value='Go' onclick='return check()'/>&nbsp&nbsp\n";
			responser<<"<br>";
			responser << "<span class=\"lnk\" onclick=\"var obj = document.getElementById('page-idx'); obj.value=showPage(1); showPage2(1);\"><img src='img/first_page.gif' alt='first'></span>&nbsp;&nbsp;";
			responser << "<span class=\"lnk\" onclick=\"var obj = document.getElementById('page-idx'); obj.value=showPage(parseInt(obj.value) - 1); showPage2(parseInt(obj.value));\"><img src='img/previous_page.gif' alt='previous'></span>&nbsp;&nbsp;";
			responser << "<span class=\"lnk\" onclick=\"var obj = document.getElementById('page-idx'); obj.value=showPage(parseInt(obj.value) + 1); showPage2(parseInt(obj.value));\"><img src='img/next_page.gif' alt='next'></span>&nbsp;&nbsp;";
			responser << "<span class=\"lnk\" onclick=\"var obj = document.getElementById('page-idx'); obj.value=showPage(" << (int)_pageCount << "); showPage2(" << (int)_pageCount << ");\"><img src='img/last_page.gif' alt='last'></span>";
		}

		responser << "</form>";
		return true;
	}

	bool ShowContent::post()
	{
		IHttpResponse& responser = _reqCtx->Response();

		AllInfos.clear();
		_contentNumberPerPage = CountPerPage;
		int totalCount;

		unsigned int currentPage = 1;
		if(_varMap.find("currentPage") != _varMap.end())
		{
			currentPage = atoi(_varMap["currentPage"].c_str());
		}
		if(_varMap.find("page_number") != _varMap.end() && String::getTrimRight(_varMap["page_number"])!= "")
		{
			currentPage = atoi(_varMap["page_number"].c_str());
		}

		TianShanIce::StrValues expectMetaData;
		std::string asset_id="";
		std::string provider_id="";
		TianShanIce::Properties searchForMetaData;
		::TianShanIce::Repository::MetaDataValue metaData;
		metaData.hintedType = ::TianShanIce::vtStrings;
		std::string netId = "";
		std::string volumeName = "";
		// NetId specified
		if(_varMap.find("net_id") != _varMap.end())
		{
			netId = _varMap["net_id"];
			searchForMetaData.insert(TianShanIce::Properties::value_type("netId", netId));
		}
		if(_varMap.find("volume_name") != _varMap.end())
		{
			volumeName = _varMap["volume_name"];
			searchForMetaData.insert(TianShanIce::Properties::value_type("volumeName", volumeName));
		}
		if(_varMap.find("asset_id") != _varMap.end() && String::getTrimRight(_varMap["asset_id"]) != "")
		{
			asset_id = _varMap["asset_id"];
			searchForMetaData.insert(TianShanIce::Properties::value_type("user.ProviderAssetId", asset_id));
		}
		if(_varMap.find("provider_id") != _varMap.end() && String::getTrimRight(_varMap["provider_id"]) != "")
		{
			provider_id = _varMap["provider_id"];
			searchForMetaData.insert(TianShanIce::Properties::value_type("user.ProviderId", provider_id));
		}
		if(_varMap.find("content_state") != _varMap.end() && String::getTrimRight(_varMap["content_state"]) != "asset_state")
		{
			searchForMetaData.insert(TianShanIce::Properties::value_type("contentState", _varMap["content_state"]));
		}
//		searchForMetaData.insert(TianShanIce::Properties::value_type("objectType", "ContentReplica"));
		bool bByBatch = false;
		try
		{
//			AllInfos = _lib->locateContent(searchForMetaData, expectMetaData, 0, false);
			if(asset_id != "" && provider_id != "" && (_varMap.find("content_state") == _varMap.end() || String::getTrimRight(_varMap["content_state"]) == "asset_state" ))
			{
				AllInfos = _lib->locateContentByPIDAndPAID(netId, volumeName, provider_id, asset_id, expectMetaData);
			}
			else if ( asset_id == "" && provider_id == "" && (_varMap.find("content_state") == _varMap.end() || String::getTrimRight(_varMap["content_state"]) == "asset_state" ))
			{
				AllInfos = _lib->locateContentByNetIDAndVolume(netId, volumeName, expectMetaData, (currentPage-1)*_contentNumberPerPage, _contentNumberPerPage, totalCount);
				bByBatch = true;
			}
			else // if( (netId == "" && volumeName =="") && (asset_id != "" || provider_id != "") )
			{
				AllInfos = _lib->locateContent(searchForMetaData, expectMetaData, 0, false);
			}
		}
		catch (Ice::Exception& ex) 
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ClibMain, "locateContent() ice exception[%s]"), ex.ice_name().c_str());
		}
		catch (...)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ClibMain, "locateContent() unknown exception"));
		}

		url.clear();
		url.setPath(ShowContentPage);
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());		
		snprintf(szBuf, sizeof(szBuf) - 1, "<form id='ShowContent' method='post' action='%s'><br>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;

		snprintf(szBuf, sizeof(szBuf) - 1, "<B>NetID</B><input type='text' id='net_id' name='net_id' value='%s'/>&nbsp&nbsp", netId.c_str());
		responser<<szBuf;

		snprintf(szBuf, sizeof(szBuf) - 1, "<B>Volume</B><input type='text' id='volume_name' name='volume_name' value='%s'/>&nbsp&nbsp", volumeName.c_str());
		responser<<szBuf;

		responser<<"<br>";

		snprintf(szBuf, sizeof(szBuf) - 1, "<B>AssetID</B><input type='text' id='asset_id' name='asset_id' value='%s'/>&nbsp&nbsp", asset_id.c_str());
		responser<<szBuf;

		snprintf(szBuf, sizeof(szBuf) - 1, "<B>ProviderID</B><input type='text' id='provider_id' name='provider_id' value='%s'/>&nbsp&nbsp", provider_id.c_str());
		responser<<szBuf;

		if(_varMap.find("content_state") != _varMap.end() && _varMap["content_state"] != "asset_state")
		{
			if(_varMap["content_state"] == "Pending")
			{
				responser << " <select name=\"content_state\" size=\"1\" >";
				responser << "	<option value=\"asset_state\">Asset State";
				responser << "	<option value=\"Pending\" selected>Pending";
				responser << "	<option value=\"Complete\">Complete";
				responser << "	<option value=\"Canceled\">Canceled";
				responser << "	<option value=\"Failed\">Failed";
				responser << " </select>";
			}
			else if(_varMap["content_state"] == "Complete")
			{
				responser << " <select name=\"content_state\" size=\"1\" >";
				responser << "	<option value=\"asset_state\">Asset State";
				responser << "	<option value=\"Pending\">Pending";
				responser << "	<option value=\"Complete\" selected>Complete";
				responser << "	<option value=\"Canceled\">Canceled";
				responser << "	<option value=\"Failed\">Failed";
				responser << " </select>";
			}
			else if(_varMap["content_state"] == "Canceled")
			{
				responser << " <select name=\"content_state\" size=\"1\" >";
				responser << "	<option value=\"asset_state\">Asset State";
				responser << "	<option value=\"Pending\">Pending";
				responser << "	<option value=\"Complete\">Complete";
				responser << "	<option value=\"Canceled\" selected>Canceled";
				responser << "	<option value=\"Failed\">Failed";
				responser << " </select>";
			}
			else if(_varMap["content_state"] == "Failed")
			{
				responser << " <select name=\"content_state\" size=\"1\" >";
				responser << "	<option value=\"asset_state\">Asset State";
				responser << "	<option value=\"Pending\">Pending";
				responser << "	<option value=\"Complete\">Complete";
				responser << "	<option value=\"Canceled\">Canceled";
				responser << "	<option value=\"Failed\" selected>Failed";
				responser << " </select>";
			}
		}
		else
		{
			responser << " <select name=\"content_state\" size=\"1\" >";
			responser << "	<option value=\"asset_state\" selected>Asset State";
			responser << "	<option value=\"Pending\">Pending";
			responser << "	<option value=\"Complete\">Complete";
			responser << "	<option value=\"Canceled\">Canceled";
			responser << "	<option value=\"Failed\">Failed";
			responser << " </select>";
		}

		// the submit button
		responser<<"	<input type='submit' value='search'/>&nbsp&nbsp\n";
		responser<<"</form>";

		int contentCount;
		if (!bByBatch)
		{
			contentCount = AllInfos.size();
		}
		else
		{
			contentCount = totalCount;
		}

		// 计算分页数量
		_pageCount = contentCount / _contentNumberPerPage;
		if (contentCount % _contentNumberPerPage != 0)
			_pageCount ++;

		if(currentPage == 0)
			currentPage = 1;
		if(currentPage > _pageCount)
			currentPage = _pageCount;

		url.clear();
		url.setPath(ShowContentPage);
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());	
		if(_varMap.find("net_id") != _varMap.end())
		{
			url.setVar("net_id", _varMap["net_id"].c_str());
		}
		if(_varMap.find("volume_name") != _varMap.end())
		{
			url.setVar("volume_name", _varMap["volume_name"].c_str());
		}
		if(_varMap.find("asset_id") != _varMap.end() && String::getTrimRight(_varMap["asset_id"]) != "")
		{
			url.setVar("asset_id", _varMap["asset_id"].c_str());
		}
		if(_varMap.find("provider_id") != _varMap.end() && String::getTrimRight(_varMap["provider_id"]) != "")
		{
			url.setVar("provider_id", _varMap["provider_id"].c_str());
		}
		if(_varMap.find("content_state") != _varMap.end() && String::getTrimRight(_varMap["content_state"]) != "")
		{
			url.setVar("content_state", _varMap["content_state"].c_str());
		}
		std::string actionURL = String::getRightStr(url.generate(), "/", false);
		responser<<"<script type=text/javascript>";
		responser<<"function showPage(idx)";
		responser<<"{";
		responser<<"if(idx > " << (int)_pageCount << ") idx = " << (int)_pageCount << ";";
		responser<<"if(idx < 1) idx = 1;";
		responser<<"return idx;";
		responser<<"}";
		responser<<"function showPage2(idx)";
		responser<<"{";
		responser<<"if(idx > " << (int)_pageCount << ") idx = " << (int)_pageCount << ";";
		responser<<"if(idx < 1) idx = 1;";
		responser<<"document.write('<form name=myForm>');\n";
		responser<<"var myForm=document.forms['myForm'];\n";
		responser<<"myForm.action= '";
		responser<<actionURL;
		responser<<"&currentPage='";
		responser<<"+idx;";
		responser<<"myForm.method='POST'; \n";  
		responser<<"myForm.submit(); \n";
		responser<<"}";
		responser<<"function check()";
		responser<<"{";
		responser<<"if(!(parseInt(document.getElementById('page_number').value) > 0) || (parseInt(document.getElementById('page_number').value) > " << (int)_pageCount << ") )";
		responser<<"{";
		responser<<"document.getElementById('page_number').value = '';";
		responser<<"document.getElementById('page_number').focus();";
		responser<<"return false;";
		responser<<"}";
		responser<<"return true;";
		responser<<"}";
		responser<<" </script>";

		// write start table flag
		responser<<"<table class='listTable'>";

		// write table header line
		responser<<"<tr>";
		responser<<"	<th><center>ContentName</center></th>";
		responser<<"	<th><center>NetId</center></th>";
		responser<<"	<th><center>VolumeName</center></th>";
		responser<<"	<th><center>State</center></th>";
		responser<<"	<th><center>ProviderId</center></th>";
		responser<<"	<th><center>ProviderAssetId</center></th>";
		responser<<"</tr>";

		// write table data line
		TianShanIce::Repository::MetaObjectInfos::iterator it = AllInfos.begin();
		if (!bByBatch && currentPage > 1)
		{
			it = it + _contentNumberPerPage*(currentPage-1);
		}
		for(unsigned int j = 0; j < _contentNumberPerPage && it < AllInfos.end(); j++)
		{
			::TianShanIce::Repository::MetaDataMap metaDataMap;
			std::string fullName = it->id;
			std::string contentName =  fullName.substr(0, fullName.find("@"));
			std::string strNetId = fullName.substr(fullName.find("@") + 1, fullName.find_first_of("$") - fullName.find("@") - 1);
			std::string volName = fullName.substr(fullName.find_first_of("$") + 1);

			try
			{
				::TianShanIce::Repository::ContentReplicaPrx contentPrx = _lib->toContentReplica(strNetId + "$" + volName + "/" + contentName);
				metaDataMap = ::TianShanIce::Repository::ContentReplicaExPrx::checkedCast(contentPrx)->getMetaDataMap();
			}
			catch (...)
			{
				it++;
				continue;
			}

			responser<<"<tr>";

			url.clear();
			url.setPath(ContentDetailPage);
			url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			url.setVar(StoreReplicaKey, strNetId.c_str());
			url.setVar(MetaVolumeKey, volName.c_str());
			url.setVar(ContentReplicaKey, it->id.c_str());
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><a href=\"%s\"><B>%s</B></a></td>", String::getRightStr(url.generate(), "/", false).c_str(), it->id.c_str());
			responser<<szBuf;

			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", strNetId.c_str());
			responser<<szBuf;

			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", volName.c_str());
			responser<<szBuf;

			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", metaDataMap["contentState"].value.c_str());
			responser<<szBuf;

			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", metaDataMap["user.ProviderId"].value.c_str());
			responser<<szBuf;

			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", metaDataMap["user.ProviderAssetId"].value.c_str());
			responser<<szBuf;

			responser<<"</tr>";
			it++;
		}

		// write end table flag
		responser<<"</table>";

		responser<<"<br>";

		if(_pageCount <= 0)
		{
			responser
				<< "<span style='width:100%'>[ no content found ]</span>"
				<< "<form style='display:inline'>";
		}
		else
		{
			responser
				<< "<span style='width:100%'>[<span id='cur-page'>" 
				<< currentPage
				<< "</span> / " << (int)_pageCount << "]</span>";
			url.clear();
			url.setPath(ShowContentPage);
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			url.setVar(TemplateKey, _varMap[TemplateKey].c_str());	
			if(_varMap.find("net_id") != _varMap.end())
			{
				url.setVar("net_id", _varMap["net_id"].c_str());
			}
			if(_varMap.find("volume_name") != _varMap.end())
			{
				url.setVar("volume_name", _varMap["volume_name"].c_str());
			}
			if(_varMap.find("asset_id") != _varMap.end() && String::getTrimRight(_varMap["asset_id"]) != "")
			{
				url.setVar("asset_id", _varMap["asset_id"].c_str());
			}
			if(_varMap.find("provider_id") != _varMap.end() && String::getTrimRight(_varMap["provider_id"]) != "")
			{
				url.setVar("provider_id", _varMap["provider_id"].c_str());
			}
			if(_varMap.find("content_state") != _varMap.end() && String::getTrimRight(_varMap["content_state"]) != "")
			{
				url.setVar("content_state", _varMap["content_state"].c_str());
			}
			snprintf(szBuf, sizeof(szBuf) - 1, "<form style='display:inline' method='post' action='%s'><br>", String::getRightStr(url.generate(), "/", false).c_str());
			responser<<szBuf;
			responser << "<input id=\"page-idx\" value='";
			responser << currentPage;
			responser << "' type='hidden'>";
			// the go to page button
			snprintf(szBuf, sizeof(szBuf) - 1, "<input type='text' id='page_number' name='page_number' value='%s'/>", "");
			responser<<szBuf;
			responser<<"	<input type='submit' value='Go' onclick='return check()'/>&nbsp&nbsp\n";
			responser<<"<br>";
			responser << "<span class=\"lnk\" onclick=\"var obj = document.getElementById('page-idx'); obj.value=showPage(1); showPage2(1);\"><img src='img/first_page.gif' alt='first'></span>&nbsp;&nbsp;";
			responser << "<span class=\"lnk\" onclick=\"var obj = document.getElementById('page-idx'); obj.value=showPage(parseInt(obj.value) - 1); showPage2(parseInt(obj.value));\"><img src='img/previous_page.gif' alt='previous'></span>&nbsp;&nbsp;";
			responser << "<span class=\"lnk\" onclick=\"var obj = document.getElementById('page-idx'); obj.value=showPage(parseInt(obj.value) + 1); showPage2(parseInt(obj.value));\"><img src='img/next_page.gif' alt='next'></span>&nbsp;&nbsp;";
			responser << "<span class=\"lnk\" onclick=\"var obj = document.getElementById('page-idx'); obj.value=showPage(" << (int)_pageCount << "); showPage2(" << (int)_pageCount << ");\"><img src='img/last_page.gif' alt='last'></span>";
		}
/*
		else
		{
			responser
				<< "<span style='width:100%'>[<span id='cur-page'>" 
				<< currentPage
				<< "</span> / " << (int)_pageCount << "]</span>"
				<< "<form style='display:inline'>";
			responser << "<input id=\"page-idx\" value='";
			responser << currentPage;
			responser << "' type='hidden'>";
			// the go to page button
			snprintf(szBuf, sizeof(szBuf) - 1, "<input type='text' id='page_number' name='page_number' value='%s'/>", "");
			responser<<szBuf;
			responser<<"	<input type='submit' value='Go'/>&nbsp&nbsp\n";
			responser<<"<br>";
			responser << "<span class=\"lnk\" onclick=\"document.getElementById('page-idx').value=showPage(1);\"><img src='img/first_page.gif' alt='first'></span>&nbsp;&nbsp;";
			responser << "<span class=\"lnk\" onclick=\"var obj = document.getElementById('page-idx'); obj.value=showPage(parseInt(obj.value) - 1);\"><img src='img/previous_page.gif' alt='previous'></span>&nbsp;&nbsp;";
			responser << "<span class=\"lnk\" onclick=\"var obj = document.getElementById('page-idx'); obj.value=showPage(parseInt(obj.value) + 1);\"><img src='img/next_page.gif' alt='next'></span>&nbsp;&nbsp;";
			responser << "<span class=\"lnk\" onclick=\"document.getElementById('page-idx').value=showPage(" << (int)_pageCount << ");\"><img src='img/last_page.gif' alt='last'></span>";
		}
*/

		responser << "</form>";

		return true;
	}
}
