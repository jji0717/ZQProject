#include "ShowVolume.h"

namespace ClibWebPage
{
	ShowVolume::ShowVolume(IHttpRequestCtx* pHttpRequestCtx) : BasePage(pHttpRequestCtx)
	{
	}

	ShowVolume::~ShowVolume()
	{
	}

	bool ShowVolume::get()
	{
		IHttpResponse& responser = _reqCtx->Response();

		AllInfos.clear();
		_volumeNumberPerPage = CountPerPage;

		TianShanIce::StrValues expectMetaData;
		expectMetaData.push_back("netId");
		expectMetaData.push_back("volumeName");
		expectMetaData.push_back("FreeSpace");
		expectMetaData.push_back("TotalSpace");
		TianShanIce::Properties searchForMetaData;
		::TianShanIce::Repository::MetaDataValue metaData;
		metaData.hintedType = ::TianShanIce::vtStrings;
		// NetId specified
		std::string netId = "";
		if(_varMap.find(StoreReplicaKey) != _varMap.end())
		{
			netId = _varMap[StoreReplicaKey];
			searchForMetaData.insert(TianShanIce::Properties::value_type("netId", netId));
		}
		searchForMetaData.insert(TianShanIce::Properties::value_type("objectType", "MetaVolume"));
		try
		{
//			AllInfos = _lib->locateContent(searchForMetaData, expectMetaData, 0, false);
			AllInfos = _lib->locateVolumesByNetID(netId, expectMetaData);
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
		url.setPath(ShowVolumePage);
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());		
		snprintf(szBuf, sizeof(szBuf) - 1, "<form id='ShowVolume' method='post' action='%s'><br>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;

		snprintf(szBuf, sizeof(szBuf) - 1, "<B>Volume</B><input type='text' id='volume_name' name='volume_name' value='%s'/>&nbsp&nbsp", "");
		responser<<szBuf;

		snprintf(szBuf, sizeof(szBuf) - 1, "<B>NetID</B><input type='text' id='net_id' name='net_id' value='%s'/>&nbsp&nbsp", netId.c_str());
		responser<<szBuf;

		// the submit button
		responser<<"	<input type='submit' value='search'/>&nbsp&nbsp\n";
		responser<<"</form>";

		int contentCount = AllInfos.size();

		// 计算分页数量
		_pageCount = contentCount / _volumeNumberPerPage;
		if (contentCount % _volumeNumberPerPage != 0)
			_pageCount ++;

		responser<<"<script type=text/javascript>";
		snprintf(szBuf, sizeof(szBuf) - 1, "var _curPageId = 'tb0';");
		responser<<szBuf;
		responser<<"function showPage(idx)";
		responser<<"{";
		responser<<"if(idx > " << (int)_pageCount << ") idx = " << (int)_pageCount << ";";
		responser<<"if(idx < 1) idx = 1;";
		responser<<"var pgId = 'tb' + (idx-1);";
		responser<<"if(document.getElementById(_curPageId))"; 
		responser<<"document.getElementById(_curPageId).style.display='none';";
		responser<<"if(document.getElementById(pgId))";
		responser<<"document.getElementById(pgId).style.display='block';";
		responser<<"_curPageId = pgId; return idx;";
		responser<<"}";
		responser
			<<"function updateStatus(){"
			<<"document.getElementById('cur-page').innerHTML=document.getElementById('page-idx').value;}\n";
		responser<<" </script>";

		TianShanIce::Repository::MetaObjectInfos::iterator it = AllInfos.begin();
		for(unsigned int i = 0; i < _pageCount; i++)
		{
			if(i == 0)
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "<table class='listTable' style='display:block' id=tb%d>", i);
				responser<<szBuf;
			}
			else
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "<table class='listTable' style='display:none' id=tb%d>", i);
				responser<<szBuf;
			}

			// write table header line
			responser<<"<tr>";
			responser<<"	<th><center>VolumeName</center></th>";
			responser<<"	<th><center>NetId</center></th>";
//			responser<<"	<th><center>State</center></th>";
			responser<<"	<th><center>Total Size(MB)</center></th>";
			responser<<"	<th><center>Free Size(MB)</center></th>";
			responser<<"</tr>";

			// write table data line
			for(unsigned int j = 0; j < _volumeNumberPerPage && it < AllInfos.end(); j++)
			{
				responser<<"<tr>";

				url.clear();
				url.setPath(ShowContentPage);
				url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
				url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
				url.setVar(StoreReplicaKey, it->id.substr(0, it->id.find("$")).c_str());
				url.setVar(MetaVolumeKey, it->id.substr(it->id.find("$")+1, it->id.size()).c_str());
				snprintf(szBuf, sizeof(szBuf) - 1, "<td><a href=\"%s\"><B>%s</B></a></td>", String::getRightStr(url.generate(), "/", false).c_str(), it->id.c_str());
				responser<<szBuf;

				snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", it->metaDatas["netId"].value.c_str());
				responser<<szBuf;

//				snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", it->metaDatas["State"].value.c_str());
//				responser<<szBuf;

				snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", it->metaDatas["TotalSpace"].value.c_str());
				responser<<szBuf;

				snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", it->metaDatas["FreeSpace"].value.c_str());
				responser<<szBuf;

				responser<<"</tr>";
				it++;
			}

			// write end table flag
			responser<<"</table>";
		}
		responser<<"<br>";

		if(_pageCount <= 0)
		{
			responser
				<< "<span style='width:100%'>[ no volume found ]</span>"
				<< "<form style='display:inline'>";
		}
		else
		{
			responser
				<< "<span style='width:100%'>[<span id='cur-page'>1</span> / " << (int)_pageCount << "]</span>"
				<< "<form style='display:inline'>";
			responser << "<input id=\"page-idx\" value='1' type='hidden'>";
			responser << "<span class=\"lnk\" onclick=\"document.getElementById('page-idx').value=showPage(1); updateStatus();\"><img src='img/first_page.gif' alt='first'></span>&nbsp;&nbsp;";
			responser << "<span class=\"lnk\" onclick=\"var obj = document.getElementById('page-idx'); obj.value=showPage(parseInt(obj.value) - 1);updateStatus();\"><img src='img/previous_page.gif' alt='previous'></span>&nbsp;&nbsp;";
			responser << "<span class=\"lnk\" onclick=\"var obj = document.getElementById('page-idx'); obj.value=showPage(parseInt(obj.value) + 1);updateStatus();\"><img src='img/next_page.gif' alt='next'></span>&nbsp;&nbsp;";
			responser << "<span class=\"lnk\" onclick=\"document.getElementById('page-idx').value=showPage(" << (int)_pageCount << ");updateStatus();\"><img src='img/last_page.gif' alt='last'></span>";

		}

		responser << "</form>";

		return true;
	}

	bool ShowVolume::post()
	{
		IHttpResponse& responser = _reqCtx->Response();

		AllInfos.clear();
		_volumeNumberPerPage = CountPerPage;

		TianShanIce::StrValues expectMetaData;
		expectMetaData.push_back("netId");
		expectMetaData.push_back("volumeName");
		expectMetaData.push_back("FreeSpace");
		expectMetaData.push_back("TotalSpace");
		TianShanIce::Properties searchForMetaData;
		::TianShanIce::Repository::MetaDataValue metaData;
		metaData.hintedType = ::TianShanIce::vtStrings;
		// NetId specified
		std::string netId = "";
		std::string volumeName = "";
		if(_varMap.find("net_id") != _varMap.end() && String::getTrimRight(_varMap["net_id"]) != "")
		{
			netId = _varMap["net_id"];
			searchForMetaData.insert(TianShanIce::Properties::value_type("netId", netId));
		}
		if(_varMap.find("volume_name") != _varMap.end() && String::getTrimRight(_varMap["volume_name"]) != "")
		{
			volumeName = _varMap["volume_name"];
			searchForMetaData.insert(TianShanIce::Properties::value_type("volumeName", volumeName));
		}
		searchForMetaData.insert(TianShanIce::Properties::value_type("objectType", "MetaVolume"));
		try
		{
//			AllInfos = _lib->locateContent(searchForMetaData, expectMetaData, 0, false);
			AllInfos = _lib->locateVolumesByNetID(netId, expectMetaData);
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
		url.setPath(ShowVolumePage);
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());		
		snprintf(szBuf, sizeof(szBuf) - 1, "<form id='ShowVolume' method='post' action='%s'><br>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;

		snprintf(szBuf, sizeof(szBuf) - 1, "<B>Volume</B><input type='text' id='volume_name' name='volume_name' value='%s'/>&nbsp&nbsp", volumeName.c_str());
		responser<<szBuf;

		snprintf(szBuf, sizeof(szBuf) - 1, "<B>NetID</B><input type='text' id='net_id' name='net_id' value='%s'/>&nbsp&nbsp", netId.c_str());
		responser<<szBuf;

		// the submit button
		responser<<"	<input type='submit' value='search'/>&nbsp&nbsp\n";
		responser<<"</form>";

		int contentCount = AllInfos.size();

		// 计算分页数量
		_pageCount = contentCount / _volumeNumberPerPage;
		if (contentCount % _volumeNumberPerPage != 0)
			_pageCount ++;

		responser<<"<script type=text/javascript>";
		snprintf(szBuf, sizeof(szBuf) - 1, "var _curPageId = 'tb0';");
		responser<<szBuf;
		responser<<"function showPage(idx)";
		responser<<"{";
		responser<<"if(idx > " << (int)_pageCount << ") idx = " << (int)_pageCount << ";";
		responser<<"if(idx < 1) idx = 1;";
		responser<<"var pgId = 'tb' + (idx-1);";
		responser<<"if(document.getElementById(_curPageId))"; 
		responser<<"document.getElementById(_curPageId).style.display='none';";
		responser<<"if(document.getElementById(pgId))";
		responser<<"document.getElementById(pgId).style.display='block';";
		responser<<"_curPageId = pgId; return idx;";
		responser<<"}";
		responser
			<<"function updateStatus(){"
			<<"document.getElementById('cur-page').innerHTML=document.getElementById('page-idx').value;}\n";
		responser<<" </script>";

		TianShanIce::Repository::MetaObjectInfos::iterator it = AllInfos.begin();
		for(unsigned int i = 0; i < _pageCount; i++)
		{
			if(i == 0)
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "<table class='listTable' style='display:block' id=tb%d>", i);
				responser<<szBuf;
			}
			else
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "<table class='listTable' style='display:none' id=tb%d>", i);
				responser<<szBuf;
			}

			// write table header line
			responser<<"<tr>";
			responser<<"	<th><center>VolumeName</center></th>";
			responser<<"	<th><center>NetId</center></th>";
//			responser<<"	<th><center>State</center></th>";
			responser<<"	<th><center>Total Size(MB)</center></th>";
			responser<<"	<th><center>Free Size(MB)</center></th>";
			responser<<"</tr>";

			// write table data line
			for(unsigned int j = 0; j < _volumeNumberPerPage && it < AllInfos.end(); j++)
			{
				if(volumeName != "" && volumeName != it->id.substr(it->id.find("$")+1, it->id.size()))
					continue;

				responser<<"<tr>";

				url.clear();
				url.setPath(ShowContentPage);
				url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
				url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
				url.setVar(StoreReplicaKey, it->id.substr(0, it->id.find("$")).c_str());
				url.setVar(MetaVolumeKey, it->id.substr(it->id.find("$")+1, it->id.size()).c_str());
				snprintf(szBuf, sizeof(szBuf) - 1, "<td><a href=\"%s\"><B>%s</B></a></td>", String::getRightStr(url.generate(), "/", false).c_str(), it->id.c_str());
				responser<<szBuf;

				snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", it->metaDatas["netId"].value.c_str());
				responser<<szBuf;

				snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", it->metaDatas["TotalSpace"].value.c_str());
				responser<<szBuf;

				snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", it->metaDatas["FreeSpace"].value.c_str());
				responser<<szBuf;

				responser<<"</tr>";
				it++;
			}

			// write end table flag
			responser<<"</table>";
		}
		responser<<"<br>";

		if(_pageCount <= 0)
		{
			responser
				<< "<span style='width:100%'>[ no volume found ]</span>"
				<< "<form style='display:inline'>";
		}
		else
		{
			responser
				<< "<span style='width:100%'>[<span id='cur-page'>1</span> / " << (int)_pageCount << "]</span>"
				<< "<form style='display:inline'>";
			responser << "<input id=\"page-idx\" value='1' type='hidden'>";
			responser << "<span class=\"lnk\" onclick=\"document.getElementById('page-idx').value=showPage(1); updateStatus();\"><img src='img/first_page.gif' alt='first'></span>&nbsp;&nbsp;";
			responser << "<span class=\"lnk\" onclick=\"var obj = document.getElementById('page-idx'); obj.value=showPage(parseInt(obj.value) - 1);updateStatus();\"><img src='img/previous_page.gif' alt='previous'></span>&nbsp;&nbsp;";
			responser << "<span class=\"lnk\" onclick=\"var obj = document.getElementById('page-idx'); obj.value=showPage(parseInt(obj.value) + 1);updateStatus();\"><img src='img/next_page.gif' alt='next'></span>&nbsp;&nbsp;";
			responser << "<span class=\"lnk\" onclick=\"document.getElementById('page-idx').value=showPage(" << (int)_pageCount << ");updateStatus();\"><img src='img/last_page.gif' alt='last'></span>";

		}

		responser << "</form>";

		return true;
	}
}
