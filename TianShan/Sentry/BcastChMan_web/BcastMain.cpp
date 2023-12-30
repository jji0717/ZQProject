#include "BcastMain.h"

namespace BcastWebPage
{	
	BcastMain::BcastMain(IHttpRequestCtx* pHttpRequestCtx) : BasePage(pHttpRequestCtx)
	{
	}

	BcastMain::~BcastMain()
	{
	}

	bool BcastMain::get()
	{
		IHttpResponse& responser = _reqCtx->Response();

		//style
		responser << "<style>";
		responser << "<!--";
		responser << "body,div,ul,li{";
		responser << "margin:0 5px 0 0;";
		responser << "padding:0;";
		responser << "}";
		responser << "body{";
		responser << "font:12px Verdana,Arial,Tahoma;";
		responser << "text-align:left;";
		responser << "}";
		responser << "a:link{";
//		responser << "color:#00F;";
		responser << "text-decoration:none;";
		responser << "}";
		responser << "a:visited {";
//		responser << "color: #00F;";
		responser << "text-decoration:none;";
		responser << "}";
		responser << "a:hover {";
		responser << "color: #c00;";
		responser << "text-decoration:underline;";
		responser << "}";
		responser << "ul{";
		responser << "list-style:none;";
		responser << "}";
		responser << ".main{";
		responser << "clear:both;";
		responser << "padding:8px;";
		responser << "text-align:left;";
		responser << "}";
		responser << "#tabs1{";
		responser << "text-align:left;";
		responser << "}";
		responser << ".menu1box{";
		responser << "position:relative;";
		responser << "overflow:hidden;";
		responser << "height:22px;";
		responser << "text-align:left;";
		responser << "}";
		responser << "#menu1{";
		responser << "position:absolute;";
		responser << "top:0;";
		responser << "left:0;";
		responser << "z-index:1;";
		responser << "}";
		responser << "#menu1 li{";
		responser << "float:left;";
		responser << "display:block;";
		responser << "cursor:pointer;";
		responser << "text-align:center;";
		responser << "line-height:21px;";
		responser << "height:21px;";
		responser << "}";
		responser << "#menu1 li.hover{";
		responser << "background:#fff;";
		responser << "border-left:1px solid #333;";
		responser << "border-top:1px solid #333;";
		responser << "border-right:1px solid #333;";
		responser << "}";
		responser << ".main1box{";
		responser << "clear:both;";
		responser << "margin-top:-1px;";
		responser << "border:1px solid #333;";
		responser << "}";
		responser << "#main1 ul{";
		responser << "display: none;";
		responser << "}";
		responser << "#main1 ul.block{";
		responser << "display: block;";
		responser << "}";
		responser << "-->";
		responser << "</style>";

		//tab
		responser << "<SCRIPT>"; 
		responser << "	function setTab(m,n)"; 
		responser << "{  \n"; 
		responser << "	var tli=document.getElementById(\"menu\"+m).getElementsByTagName(\"li\"); \n";  
		responser << "	var mli=document.getElementById(\"main\"+m).getElementsByTagName(\"ul\");\n"; 
		responser << "	for(i=0;i<tli.length;i++){\n"; 
		responser << "		tli[i].className=i==n?\"hover\":\"\"; \n"; 
		responser << "		mli[i].style.display=i==n?\"block\":\"none\";\n";
		responser << "		}\n";
		responser << "}  \n"; 
		responser << "</SCRIPT>  \n"; 

		std::string fun;
		fun = _varMap[FunctionKey];
		if (!fun.empty())
		{
			std::string chName;
			chName = _varMap[ChannelNameKey];
			::TianShanIce::Application::Broadcast::BcastPublishPointPrx bcastPubPrx = NULL;
			::TianShanIce::Application::PublishPointPrx pubPrx = NULL;
			if(fun.compare("START") == 0)
			{
				try
				{
					pubPrx = chnlPub->open(chName);
					bcastPubPrx = ::TianShanIce::Application::Broadcast::BcastPublishPointPrx::checkedCast(pubPrx);
					bcastPubPrx->start();
				}
				catch (const TianShanIce::BaseException& ex)
				{
					snprintf(szBuf, sizeof(szBuf) - 1, "start channel [%s] caught %s: %s", chName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
					setLastError(szBuf);
					glog(ErrorLog, CLOGFMT(BcastMain, "%s"), getLastError());
					responser.SetLastError(getLastError());
					return false;
				}
				catch (const Ice::Exception& ex)
				{
					snprintf(szBuf, sizeof(szBuf) - 1, "start channel [%s] caught %s", chName.c_str(), ex.ice_name().c_str());
					setLastError(szBuf);
					glog(ErrorLog, CLOGFMT(BcastMain, "%s"), getLastError());
					responser.SetLastError(getLastError());
					return false;
				}				
			}
			if(fun.compare("STOP") == 0)
			{
				try
				{
					pubPrx = chnlPub->open(chName);
					bcastPubPrx = ::TianShanIce::Application::Broadcast::BcastPublishPointPrx::checkedCast(pubPrx);
					bcastPubPrx->stop();
				}
				catch (const TianShanIce::BaseException& ex)
				{
					snprintf(szBuf, sizeof(szBuf) - 1, "stop channel [%s] caught %s: %s", chName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
					setLastError(szBuf);
					glog(ErrorLog, CLOGFMT(BcastMain, "%s"), getLastError());
					responser.SetLastError(getLastError());
					return false;
				}
				catch (const Ice::Exception& ex)
				{
					snprintf(szBuf, sizeof(szBuf) - 1, "stop channel [%s] caught %s", chName.c_str(), ex.ice_name().c_str());
					setLastError(szBuf);
					glog(ErrorLog, CLOGFMT(BcastMain, "%s"), getLastError());
					responser.SetLastError(getLastError());
					return false;
				}				
			}
		}

		unsigned int i = 0, count = 0;
		TianShanIce::Application::PublishPointInfos chnlInfos;
		TianShanIce::StrValues params;
		TianShanIce::Application::Broadcast::ChannnelItems channelItems;
		params.push_back("maxBitrate");
		params.push_back("desc");
		try
		{
			chnlInfos = chnlPub->listPublishPointInfo(params);
			channelItems = chnlPub->listFilterItems();
		}
		catch (const TianShanIce::BaseException& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "listChannelInfo caught %s:%s", ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(BcastMain, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const Ice::Exception& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "listChannelInfo caught %s", ex.ice_name().c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(BcastMain, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}

		// write page's functionality
		responser<<"<H2>Main page</H2>";

		responser<<"<div id=\"tabs1\">";
		responser<<"	<div class=\"menu1box\">";
		responser<<"	<ul id=\"menu1\">";
		if(_varMap[TabKey] == AddFilterItemPage)
		{
			responser<<"	<li onclick=\"setTab(1,0)\"><a href=\"#\">ChannelPublisher</a></li>";
			responser<<"	<li class=\"hover\" onclick=\"setTab(1,1)\"><a href=\"#\">Filter</a></li>";
		}
		else
		{
			responser<<"	<li class=\"hover\" onclick=\"setTab(1,0)\"><a href=\"#\">ChannelPublisher</a></li>";
			responser<<"	<li onclick=\"setTab(1,1)\"><a href=\"#\">Filter</a></li>";
		}
		responser<<"	</ul>";
		responser<<"	</div>";
		responser<<"	<div class=\"main1box\">";
		responser<<"	<div class=\"main\" id=\"main1\">";
		if(_varMap[TabKey] == AddFilterItemPage)
		{
			responser<<"	<ul class=\"none\">";
		}
		else
		{	
			responser<<"	<ul class=\"block\">";
		}

		// write start table flag
		responser<<"<table class='listTable'>";

		// write table header line
		responser<<"<tr class='heading'>";
//		responser<<"	<th>Edit</th>";
		responser<<"	<th><center>Start</th>";
		responser<<"	<th><center>Stop</center></th>";
		responser<<"	<th><center>Remove</center></th>";
		responser<<"	<th><center>ChannelName</center></th>";
		for (i = 0, count = params.size(); i < count; i ++)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "	<th>%s</th>", params[i].c_str());
			responser<<szBuf;
		}
		responser<<"	<th><center>Type</center></th>";
		responser<<"	<th><center>"RESKEY_Interval"</center></th>";
		responser<<"	<th><center>"RESKEY_Iterator"</center></th>";
		responser<<"	<th><center>"RESKEY_IpPort"</center></th>";
		responser<<"	<th><center>"RESKEY_UpTimpe"</center></th>";
		responser<<"</tr>";

		// write table data line
		for (i = 0, count = chnlInfos.size(); i < count; i ++)
		{
			responser<<"<tr>";
/*
			// a link to edit channel page
			url.clear();
			url.setPath(EditChannelPage);
			url.setVar(ChannelNameKey, chnlInfos[i].name.c_str());
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
			//			<Img src='img/xxx.gif' href='' alt='edit'>
			snprintf(szBuf, sizeof(szBuf) - 1, "		<td><a href=\"%s\">Edit</a></td>", String::getRightStr(url.generate(), "/", false).c_str());
			responser<<szBuf;
*/
			// start channel
			url.clear();
			url.setPath(BcastMainPage);
			url.setVar(ChannelNameKey, chnlInfos[i].name.c_str());
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
			url.setVar(FunctionKey, "START");
			snprintf(szBuf, sizeof(szBuf) - 1, "		<td><a href=\"%s\">Start</a></td>", String::getRightStr(url.generate(), "/", false).c_str());
			responser<<szBuf;

			// stop channel
			url.clear();
			url.setPath(BcastMainPage);
			url.setVar(ChannelNameKey, chnlInfos[i].name.c_str());
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
			url.setVar(FunctionKey, "STOP");
			snprintf(szBuf, sizeof(szBuf) - 1, "		<td><a href=\"%s\">Stop</a></td>", String::getRightStr(url.generate(), "/", false).c_str());
			responser<<szBuf;

			// a link to remove channel page
			url.clear();
			url.setPath(RemoveChannelPage);
			url.setVar(ChannelNameKey, chnlInfos[i].name.c_str());
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
			snprintf(szBuf, sizeof(szBuf) - 1, "		<td><a href=\"%s\">Remove</a></td>", String::getRightStr(url.generate(), "/", false).c_str());
			responser<<szBuf;

			// channel name
			url.clear();
			url.setPath(ShowChannelPage);
			url.setVar(ChannelNameKey, chnlInfos[i].name.c_str());
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
			snprintf(szBuf, sizeof(szBuf) - 1, "		<td><a href=\"%s\">%s</a></td>", String::getRightStr(url.generate(), "/", false).c_str(), chnlInfos[i].name.c_str());
			responser<<szBuf;

			TianShanIce::StrValues::const_iterator itor = params.begin();
			while (params.end() != itor)
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "		<td>%s</td>", chnlInfos[i].params[*itor].c_str());
				responser<<szBuf;
				itor ++;
			}
/*
			std::map<std::string, std::string>::const_iterator search_it;
			search_it = chnlInfos[i].params.find("type");
			if(search_it != chnlInfos[i].params.end())
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "		<td>%s</td>", chnlInfos[i].params["type"].c_str());
				responser<<szBuf;
			}
*/
			snprintf(szBuf, sizeof(szBuf) - 1, "		<td>%s</td>", chnlInfos[i].type.c_str());
			responser<<szBuf;

			snprintf(szBuf, sizeof(szBuf) - 1, "		<td>%s</td>", chnlInfos[i].params[RESKEY_Interval].c_str());
			responser<<szBuf;

			snprintf(szBuf, sizeof(szBuf) - 1, "		<td>%s</td>", chnlInfos[i].params[RESKEY_Iterator].c_str());
			responser<<szBuf;

			snprintf(szBuf, sizeof(szBuf) - 1, "		<td>%s</td>", chnlInfos[i].params[RESKEY_IpPort].c_str());
			responser<<szBuf;

			std::string strUpTime = chnlInfos[i].params[RESKEY_UpTimpe];
			if(!strUpTime.empty())
			{
				Ice::Long uptime = _atoi64(strUpTime.c_str());
				uptime = uptime/1000;
				Ice::Long hour = uptime/3600;
				Ice::Long minute = (uptime - hour*3600)/60;
				Ice::Long second = uptime - hour*3600 - minute*60;
				char time[40];
				snprintf(time, sizeof(time) - 1, "%4d:%02d:%02d", (int)hour, (int)minute, (int)second);
				snprintf(szBuf, sizeof(szBuf) - 1, "		<td>%s</td>", time);
				responser<<szBuf;
			}

			responser<<"</tr>";
		}

		// write end table flag
		responser<<"</table>";
		responser<<"</ul>";
		if(_varMap[TabKey] == AddFilterItemPage)
		{
			responser<<"<ul class=\"block\">";
		}
		else
		{
			responser<<"<ul>";
		}

		// write start filter table
		responser<<"<table class='listTable'>";

		// write table header line
		responser<<"<tr class='heading'>";
		responser<<"	<th><center>Remove</center></th>";
		responser<<"	<th><center>FilterItemName</center></th>";
		responser<<"	<th><center>BroadcastStart</th>";
		responser<<"	<th><center>Expiration</center></th>";
		responser<<"	<th><center>Playable</center></th>";
		responser<<"	<th><center>ForceNormalSpeed</center></th>";
		responser<<"	<th><center>InTimeOffset</center></th>";
		responser<<"	<th><center>OutTimeOffset</center></th>";
		responser<<"	<th><center>LastModified</center></th>";
		responser<<"</tr>";

		// write table data line
		for (i = 0, count = channelItems.size(); i < count; i ++)
		{
			responser<<"<tr>";

			// a link to remove channel page
			url.clear();
			url.setPath(RemoveFilterItemPage);
			url.setVar(FilterItemNameKey, channelItems[i].contentName.c_str());
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
			snprintf(szBuf, sizeof(szBuf) - 1, "		<td><a href=\"%s\">Remove</a></td>", String::getRightStr(url.generate(), "/", false).c_str());
			responser<<szBuf;

			snprintf(szBuf, sizeof(szBuf) - 1, "		<td>%s</td>", channelItems[i].contentName.c_str());
			responser<<szBuf;

			snprintf(szBuf, sizeof(szBuf) - 1, "		<td>%s</td>", channelItems[i].broadcastStart.c_str());
			responser<<szBuf;

			snprintf(szBuf, sizeof(szBuf) - 1, "		<td>%s</td>", channelItems[i].expiration.c_str());
			responser<<szBuf;

			snprintf(szBuf, sizeof(szBuf) - 1, "		<td>%d</td>", channelItems[i].playable);
			responser<<szBuf;

			snprintf(szBuf, sizeof(szBuf) - 1, "		<td>%d</td>", channelItems[i].forceNormalSpeed);
			responser<<szBuf;

			snprintf(szBuf, sizeof(szBuf) - 1, "		<td>%lld</td>", channelItems[i].inTimeOffset);
			responser<<szBuf;

			snprintf(szBuf, sizeof(szBuf) - 1, "		<td>%lld</td>", channelItems[i].outTimeOffset);
			responser<<szBuf;

			uint64 ltime;
			ltime = channelItems[i].lastModified;
			
#ifdef ZQ_OS_MSWIN
			ltime *= 10000;
			FILETIME filetime;
			memcpy(&filetime,&ltime,sizeof(filetime));
			SYSTEMTIME stUTC, stLocal;
			FileTimeToSystemTime(&filetime, &stUTC);

			TIME_ZONE_INFORMATION zinfo;
			GetTimeZoneInformation(&zinfo);

			SystemTimeToTzSpecificLocalTime(&zinfo,&stUTC, &stLocal);

			snprintf(szBuf, sizeof(szBuf) - 1, "		<td>%4d-%2d-%2dT%2d:%2d:%2d</td>", stLocal.wYear, stLocal.wMonth, stLocal.wDay, stLocal.wHour, stLocal.wMinute, stLocal.wSecond);
#else
			int64 sec  = ltime/1000;
			time_t tt;

			memcpy(&tt, &sec, sizeof(tt));
			struct tm *ptm = localtime(&tt);

			snprintf(szBuf, sizeof(szBuf) - 1, "		<td>%4d-%02d-%02dT%02d:%02d:%02d</td>", ptm->tm_year, ptm->tm_mon, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
#endif
			responser<<szBuf;

			responser<<"</tr>";
		}

		// write end table flag
		responser<<"</table>";
		responser<<"</ul>";
		responser<<"</div>";
		responser<<"</div>";
		responser<<"</div>";

		responser<<"<br>";
		// a link to add new channel page
		url.clear();
		url.setPath(AddChannelPage);
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>New channel</B></a>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;
		// a link to add new channel page
		LinkSpace;
		url.clear();
		url.setPath(BcastMainPage);
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Refresh</B></a>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;
		LinkSpace;
		// a link to add new filter page
		url.clear();
		url.setPath(AddFilterItemPage);
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>New Filter</B></a>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;

		return true;
	}

	bool BcastMain::post()
	{
		return true;
	}

}