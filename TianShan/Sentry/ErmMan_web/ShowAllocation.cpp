#include "ShowAllocation.h"
#include "DataTypes.h"

namespace ErmWebPage
{
	ShowAllocation::ShowAllocation(IHttpRequestCtx* pHttpRequestCtx) : BasePage(pHttpRequestCtx)
	{
	}

	ShowAllocation::~ShowAllocation()
	{
	}

	bool ShowAllocation::get()
	{
		IHttpResponse& responser = _reqCtx->Response();

		AllInfos.clear();
		_sessionNumberPerPage = 30;

		TianShanIce::EdgeResource::EdgeDeviceInfos deviceInfos;
		TianShanIce::StrValues expectedMetaData;

		if (_varMap[ChannelNameKey].empty())
		{
			try
			{
				deviceInfos = _ERM->listDevices(expectedMetaData);
			}
			catch (const TianShanIce::BaseException& ex)
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "list device caught %s:%s<br>", 
					ex.ice_name().c_str(), ex.message.c_str());
				setLastError(szBuf);
				glog(ErrorLog, CLOGFMT(ShowDevice, "%s"), getLastError());
				responser.SetLastError(getLastError());
				return false;
			}
			catch (const Ice::Exception& ex)
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "list device caught %s<br>", 
					ex.ice_name().c_str());
				setLastError(szBuf);
				glog(ErrorLog, CLOGFMT(ShowDevice, "%s"), getLastError());
				responser.SetLastError(getLastError());
				return false;
			}
		}

		url.clear();
		url.setPath(ShowAllocationPage);
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());		
		snprintf(szBuf, sizeof(szBuf) - 1, "<form id='ShowAllocation' method='post' action='%s'><br>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;


		if(!_varMap[ChannelNameKey].empty())
		{
			responser << " <select name=\"search_type\" size=\"1\" >";
			responser << "	<option value=\"device\">Device";
			responser << "	<option value=\"channel\" selected>Channel";
			responser << " </select>";

			snprintf(szBuf, sizeof(szBuf) - 1, "	<input type='text' id='search_value' name='search_value' value='%s'/>", _varMap[ChannelNameKey].c_str());
			responser<<szBuf;
		}
		else
		{
			responser << " <select name=\"search_type\" size=\"1\" >";
			responser << "	<option value=\"device\" selected>Device";
			responser << "	<option value=\"channel\">Channel";
			responser << " </select>";

			if(deviceInfos.size())
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "	<input type='text' id='search_value' name='search_value' value='%s'/>", deviceInfos[0].ident.name.c_str());
				responser<<szBuf;
			}
			else
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "	<input type='text' id='search_value' name='search_value' value=''/>");
				responser<<szBuf;
			}
		}


		// the submit button
		responser<<"	<input type='submit' value='search'/>";

// 		url.clear();
// 		url.setPath(ShowAllocationPage);
// 		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
// 		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
// 		snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Refresh</B></a>", String::getRightStr(url.generate(), "/", false).c_str());
// 		responser<<szBuf;

		// write </form>
		responser<<"</form>";

		std::string chName; 
		chName = _varMap[ChannelNameKey];
		if (chName.empty())
		{
			if(deviceInfos.size())
				showAllocationInfo(responser, deviceInfos[0].ident.name);
		}
		else
		{
			std::string devName = _varMap[DeviceNameKey];
			short portNum = 0;
			short chNum = 0;
			std::string temp = chName.substr(chName.find_last_of('/')+1, chName.find_last_of('.')-chName.find_last_of('/'));
			portNum = atoi(temp.c_str());
			temp = chName.substr(chName.find_last_of('.')+1, chName.size());
			chNum = atoi(temp.c_str());
			showAllocationInfo(responser, devName, portNum, chNum);
		}

		int sesscount = AllInfos.size();

		// 计算分页数量
		_pageCount = sesscount / _sessionNumberPerPage;
		if (sesscount % _sessionNumberPerPage != 0)
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

		TianShanIce::EdgeResource::AllocationPrx allocationPrx;
		TianShanIce::EdgeResource::EdgeChannelPrx channelPrx;
		TianShanIce::State state;
		std::string chnlName;
		// write table data line

		AllocationInfos_iter it = AllInfos.begin();
		for(int i = 0; i < _pageCount; i++)
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
			//		responser<<"	<th><center>Video Module<br>Address</center></th>";
			//		responser<<"	<th><center>Owner</center></th>";
			responser<<"	<th>Output<br>QAM</center></th>";
			responser<<"	<th>Program<br>Number</center></th>";
			responser<<"	<th>Source IP</center></th>";
			responser<<"	<th>UDP<br>Port</center></th>";
			responser<<"	<th>Bandwidth<br>(Kbps)</center></th>";
			responser<<"	<th>State</center></th>";
			//		responser<<"	<th><center>Status</center></th>";
//			responser<<"	<th>Maximum<br>Jitter</center></th>";
			responser<<"	<th>Created</center></th>";
			responser<<"	<th>Provisioned</center></th>";
			responser<<"	<th>Committed</center></th>";
			responser<<"	<th>Expiration</center></th>";
			responser<<"</tr>";

			// write table data line
			for(int j = 0; j < _sessionNumberPerPage && it < AllInfos.end(); j++)
			{
				try
				{ 
					allocationPrx = _ERM->openAllocation((*it).ident.name);
					if(allocationPrx)
					{
						state = allocationPrx->getState();
						channelPrx = allocationPrx->getChannel();
						if(channelPrx)
						{
							chnlName = channelPrx->getId();
						}
					}
				}
				catch (const Ice::ObjectNotExistException&)
				{
					continue;
				}
				catch (const TianShanIce::BaseException& ex)
				{
					snprintf(szBuf, sizeof(szBuf) - 1, "list channel caught %s:%s<br>", 
						ex.ice_name().c_str(), ex.message.c_str());
					setLastError(szBuf);
					responser.SetLastError(getLastError());
					return false;
				}
				catch (const Ice::Exception& ex)
				{
					snprintf(szBuf, sizeof(szBuf) - 1, "list channel caught %s<br>", 
						ex.ice_name().c_str());
					setLastError(szBuf);
					responser.SetLastError(getLastError());
					return false;
				}

				responser<<"<tr>";

				// show channel 
				snprintf(szBuf, sizeof(szBuf) - 1, "<td>%s</td>", chnlName.c_str());
				responser<<szBuf;

				// show program number
				snprintf(szBuf, sizeof(szBuf) - 1, "<td>%s</td>", (*it).props[ProgramNumber].c_str());
				responser<<szBuf;

// 				url.clear();
// 				url.setPath(ChannelDetailPage);
// 				url.setVar(ChannelNameKey, chnlName.c_str());
// 				url.setVar(DeviceNameKey, devName.c_str());
// 				url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
// 				url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
// 				snprintf(szBuf, sizeof(szBuf) - 1, "<td><center><a href=\"%s\">Edit</a>", String::getRightStr(url.generate(), "/", false).c_str());
// 				responser<<szBuf;

				// show SourceIP
				snprintf(szBuf, sizeof(szBuf) - 1, "<td>%s</td>", (*it).props[SourceIP].c_str());
				responser<<szBuf;

				// show UDP port
				snprintf(szBuf, sizeof(szBuf) - 1, "<td>%s</td>", (*it).props[UDP].c_str());
				responser<<szBuf;

				// show BandWidth
				snprintf(szBuf, sizeof(szBuf) - 1, "<td>%lld</td>", _atoi64((*it).props[BandWidth].c_str())/1000);
				responser<<szBuf;

				// show state
				snprintf(szBuf, sizeof(szBuf) - 1, "<td>%s</td>", switchState(state).c_str());
				responser<<szBuf;

				// show MaximumJitter
// 				snprintf(szBuf, sizeof(szBuf) - 1, "<td>%s</td>", (*it).props[MaximumJitter].c_str());
// 				responser<<szBuf;

				// show create time
				char timeBuffer[128];
				memset(timeBuffer,'\0',sizeof(timeBuffer));
				ZQ::common::TimeUtil::TimeToUTC(_atoi64((*it).props[StpCreated].c_str()),timeBuffer,sizeof(timeBuffer),true);
				snprintf(szBuf, sizeof(szBuf) - 1, "<td>%s</td>", timeBuffer);
				responser<<szBuf;

				// show provision time
				memset(timeBuffer,'\0',sizeof(timeBuffer));
				ZQ::common::TimeUtil::TimeToUTC(_atoi64((*it).props[StpProvisioned].c_str()),timeBuffer,sizeof(timeBuffer),true);
				snprintf(szBuf, sizeof(szBuf) - 1, "<td>%s</td>", timeBuffer);
				responser<<szBuf;

				// show commit time
				memset(timeBuffer,'\0',sizeof(timeBuffer));
				ZQ::common::TimeUtil::TimeToUTC(_atoi64((*it).props[StpCommitted].c_str()),timeBuffer,sizeof(timeBuffer),true);
				snprintf(szBuf, sizeof(szBuf) - 1, "<td>%s</td>", timeBuffer);
				responser<<szBuf;

				// show expiration time
				memset(timeBuffer,'\0',sizeof(timeBuffer));
				ZQ::common::TimeUtil::TimeToUTC(_atoi64((*it).props[Expire].c_str()),timeBuffer,sizeof(timeBuffer),true);
				snprintf(szBuf, sizeof(szBuf) - 1, "<td>%s</td>", timeBuffer);
				responser<<szBuf;

				responser<<"</tr>";
				it++;
			}
			responser<<"</table>";
		}

		// write end table flag
//		responser<<"</table>";
		responser<<"<br>";

		if(_pageCount <= 0)
		{
			responser
				<< "<span style='width:100%'>[ no record found ]</span>"
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
/*
		url.clear();
		url.setPath(AddAllocationPage);
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Add</B></a>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;
*/
		return true;
	}

	bool ShowAllocation::post()
	{
		IHttpResponse& responser = _reqCtx->Response();

		AllInfos.clear();
		_sessionNumberPerPage = 30;

		url.clear();
		url.setPath(ShowAllocationPage);
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());		
		snprintf(szBuf, sizeof(szBuf) - 1, "<form id='ShowAllocation' method='post' action='%s'><br>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;

		if(_varMap["search_type"] == "device")
		{
			responser << " <select name=\"search_type\" size=\"1\" >";
			responser << "	<option value=\"device\" selected>Device";
			responser << "	<option value=\"channel\">Channel";
			responser << " </select>";
		}
		else
		{
			responser << " <select name=\"search_type\" size=\"1\" >";
			responser << "	<option value=\"device\">Device";
			responser << "	<option value=\"channel\" selected>Channel";
			responser << " </select>";
		}


		snprintf(szBuf, sizeof(szBuf) - 1, "	<input type='text' id='search_value' name='search_value' value='%s'/>", _varMap["search_value"].c_str());
		responser<<szBuf;

		// the submit button
		responser<<"	<input type='submit' value='search'/>";

		// write </form>
		responser<<"</form>";

		if(_varMap["search_value"].empty())
		{
			TianShanIce::EdgeResource::EdgeDeviceInfos deviceInfos;
			TianShanIce::StrValues expectedMetaData;
			try
			{
				deviceInfos = _ERM->listDevices(expectedMetaData);
			}
			catch (const TianShanIce::BaseException& ex)
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "list device caught %s:%s<br>", 
					ex.ice_name().c_str(), ex.message.c_str());
				setLastError(szBuf);
				glog(ErrorLog, CLOGFMT(ShowDevice, "%s"), getLastError());
				responser.SetLastError(getLastError());
				return false;
			}
			catch (const Ice::Exception& ex)
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "list device caught %s<br>", 
					ex.ice_name().c_str());
				setLastError(szBuf);
				glog(ErrorLog, CLOGFMT(ShowDevice, "%s"), getLastError());
				responser.SetLastError(getLastError());
				return false;
			}
			// write table data line
			for (DeviceInfos_iter it = deviceInfos.begin(); it != deviceInfos.end(); it++)
			{
				EdgeDeviceInfo deviceInfo = *it;
				showAllocationInfo(responser, deviceInfo.ident.name);
			}		
		}

		std::string devName; 
		if(_varMap["search_type"] == "device")
		{
			devName = _varMap["search_value"];
			if (!devName.empty())
				showAllocationInfo(responser, devName);
		}
		else
		{
			std::string chName = _varMap["search_value"];
			if (!chName.empty())
			{
				std::string devName = chName.substr(0, chName.find_first_of('/'));
				short portNum = 0;
				short chNum = 0;
				std::string temp = chName.substr(chName.find_last_of('/')+1, chName.find_last_of('.')-chName.find_last_of('/'));
				portNum = atoi(temp.c_str());
				temp = chName.substr(chName.find_last_of('.')+1, chName.size());
				chNum = atoi(temp.c_str());
				showAllocationInfo(responser, devName, portNum, chNum);
			}
		}

		int sesscount = AllInfos.size();

		// 计算分页数量
		_pageCount = sesscount / _sessionNumberPerPage;
		if (sesscount % _sessionNumberPerPage != 0)
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

		TianShanIce::EdgeResource::AllocationPrx allocationPrx;
		TianShanIce::EdgeResource::EdgeChannelPrx channelPrx;
		TianShanIce::State state;
		std::string chnlName;
		// write table data line

		AllocationInfos_iter it = AllInfos.begin();
		for(int i = 0; i < _pageCount; i++)
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
			responser<<"	<th>Output<br>QAM</center></th>";
			responser<<"	<th>Program<br>Number</center></th>";
			responser<<"	<th>Source IP</center></th>";
			responser<<"	<th>UDP<br>Port</center></th>";
			responser<<"	<th>Bandwidth<br>(Kbps)</center></th>";
			responser<<"	<th>State</center></th>";
//			responser<<"	<th>Maximum<br>Jitter</center></th>";
			responser<<"	<th>Created</center></th>";
			responser<<"	<th>Provisioned</center></th>";
			responser<<"	<th>Committed</center></th>";
			responser<<"	<th>Expiration</center></th>";
			responser<<"</tr>";

			// write table data line
			for(int j = 0; j < _sessionNumberPerPage && it < AllInfos.end(); j++)
			{
				responser<<"<tr>";

				try
				{ 
					allocationPrx = _ERM->openAllocation((*it).ident.name);
					if(allocationPrx)
					{
						state = allocationPrx->getState();
						channelPrx = allocationPrx->getChannel();
						if(channelPrx)
						{
							chnlName = channelPrx->getId();
						}
					}
				}
				catch (const TianShanIce::BaseException& ex)
				{
					snprintf(szBuf, sizeof(szBuf) - 1, "list channel caught %s:%s<br>", 
						ex.ice_name().c_str(), ex.message.c_str());
					setLastError(szBuf);
					responser.SetLastError(getLastError());
					return false;
				}
				catch (const Ice::Exception& ex)
				{
					snprintf(szBuf, sizeof(szBuf) - 1, "list channel caught %s<br>", 
						ex.ice_name().c_str());
					setLastError(szBuf);
					responser.SetLastError(getLastError());
					return false;
				}

				// show channel 
				snprintf(szBuf, sizeof(szBuf) - 1, "<td>%s</td>", chnlName.c_str());
				responser<<szBuf;

				// show program number
				snprintf(szBuf, sizeof(szBuf) - 1, "<td>%s</td>", (*it).props[ProgramNumber].c_str());
				responser<<szBuf;

				// show SourceIP
				snprintf(szBuf, sizeof(szBuf) - 1, "<td>%s</td>", (*it).props[SourceIP].c_str());
				responser<<szBuf;

				// show UDP port
				snprintf(szBuf, sizeof(szBuf) - 1, "<td>%s</td>", (*it).props[UDP].c_str());
				responser<<szBuf;

				// show BandWidth
				snprintf(szBuf, sizeof(szBuf) - 1, "<td>%lld</td>", _atoi64((*it).props[BandWidth].c_str())/1000);
				responser<<szBuf;

				// show state
				snprintf(szBuf, sizeof(szBuf) - 1, "<td>%s</td>", switchState(state).c_str());
				responser<<szBuf;

				// show MaximumJitter
// 				snprintf(szBuf, sizeof(szBuf) - 1, "<td>%s</td>", (*it).props[MaximumJitter].c_str());
// 				responser<<szBuf;

				// show create time
				char timeBuffer[128];
				memset(timeBuffer,'\0',sizeof(timeBuffer));
				ZQ::common::TimeUtil::TimeToUTC(_atoi64((*it).props[StpCreated].c_str()),timeBuffer,sizeof(timeBuffer),true);
				snprintf(szBuf, sizeof(szBuf) - 1, "<td>%s</td>", timeBuffer);
				responser<<szBuf;

				// show provision time
				memset(timeBuffer,'\0',sizeof(timeBuffer));
				ZQ::common::TimeUtil::TimeToUTC(_atoi64((*it).props[StpProvisioned].c_str()),timeBuffer,sizeof(timeBuffer),true);
				snprintf(szBuf, sizeof(szBuf) - 1, "<td>%s</td>", timeBuffer);
				responser<<szBuf;

				// show commit time
				memset(timeBuffer,'\0',sizeof(timeBuffer));
				ZQ::common::TimeUtil::TimeToUTC(_atoi64((*it).props[StpCommitted].c_str()),timeBuffer,sizeof(timeBuffer),true);
				snprintf(szBuf, sizeof(szBuf) - 1, "<td>%s</td>", timeBuffer);
				responser<<szBuf;

				// show expiration time
				memset(timeBuffer,'\0',sizeof(timeBuffer));
				ZQ::common::TimeUtil::TimeToUTC(_atoi64((*it).props[Expire].c_str()),timeBuffer,sizeof(timeBuffer),true);
				snprintf(szBuf, sizeof(szBuf) - 1, "<td>%s</td>", timeBuffer);
				responser<<szBuf;

				responser<<"</tr>";
				it++;
			}
			responser<<"</table>";
		}

		// write end table flag
		//		responser<<"</table>";
		responser<<"<br>";

		if(_pageCount <= 0)
		{
			responser
				<< "<span style='width:100%'>[ no record found ]</span>"
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

	bool ShowAllocation::showAllocationInfo(IHttpResponse& responser, std::string devName)
	{
		std::string edgePort; 
		edgePort = _varMap[EdgePortKey];
		TianShanIce::EdgeResource::EdgePortInfos  portInfos;
		if (edgePort.empty())
		{
			try
			{
				TianShanIce::EdgeResource::EdgeDevicePrx devicePrx; 
				devicePrx = _ERM->openDevice(devName);
				portInfos = devicePrx->listEdgePorts();
			}
			catch (const TianShanIce::BaseException& ex)
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "list edge port caught %s:%s<br>", 
					ex.ice_name().c_str(), ex.message.c_str());
				setLastError(szBuf);
				glog(ErrorLog, CLOGFMT(ShowAllocation, "%s"), getLastError());
				responser.SetLastError(getLastError());
				return false;
			}
			catch (const Ice::Exception& ex)
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "list edge port caught %s<br>", 
					ex.ice_name().c_str());
				setLastError(szBuf);
				glog(ErrorLog, CLOGFMT(ShowAllocation, "%s"), getLastError());
				responser.SetLastError(getLastError());
				return false;
			}
			for (PortInfos_iter it = portInfos.begin(); it != portInfos.end(); it++)
			{
				showAllocationInfo(responser, devName, it->Id);
			}
		}
		else
		{
			int port = atoi(edgePort.c_str());
			showAllocationInfo(responser, devName, port);
		}
		

		return true;
	}

	bool ShowAllocation::showAllocationInfo(IHttpResponse& responser, std::string devName, short port)
	{
		std::string chNum; 
		chNum = _varMap[ChannelNumberKey];
		if(chNum.empty())
		{
			TianShanIce::StrValues expectedMetaData;
			expectedMetaData.push_back(RF);
			expectedMetaData.push_back(TSID);
			expectedMetaData.push_back(PAT_Interval);
			expectedMetaData.push_back(PMT_Interval);
			expectedMetaData.push_back(LastUpdated);
			expectedMetaData.push_back(NITPID);
			expectedMetaData.push_back(StartUDP);
			expectedMetaData.push_back(UdpSBP);
			expectedMetaData.push_back(PN);
			expectedMetaData.push_back(MaxSessions);
			expectedMetaData.push_back(LBandWidth);
			expectedMetaData.push_back(HBandWidth);
			expectedMetaData.push_back(Enabled);
			TianShanIce::EdgeResource::EdgeDevicePrx devicePrx;
			TianShanIce::EdgeResource::EdgeChannelInfos channelInfos;
			try
			{ 
				devicePrx = _ERM->openDevice(devName);
				channelInfos = devicePrx->listChannels(port, expectedMetaData, false);
			}
			catch (const TianShanIce::BaseException& ex)
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "list channel caught %s:%s<br>", 
					ex.ice_name().c_str(), ex.message.c_str());
				setLastError(szBuf);
				responser.SetLastError(getLastError());
				return false;
			}
			catch (const Ice::Exception& ex)
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "list channel caught %s<br>", 
					ex.ice_name().c_str());
				setLastError(szBuf);
				responser.SetLastError(getLastError());
				return false;
			}
			for (ChannelInfos_iter it = channelInfos.begin(); it != channelInfos.end(); it++)
			{
				EdgeChannelInfo channelInfo = *it;
				short chNum = 0;
				string cn = channelInfo.ident.name;
				cn = cn.substr(cn.find_last_of('.')+1, cn.size());
				chNum = atoi(cn.c_str());
				showAllocationInfo(responser, devName, port, chNum);
			}
		}
		else
		{
			int num = atoi(chNum.c_str());
			showAllocationInfo(responser, devName, port, num);
		}
		

		return true;
	}

	bool ShowAllocation::showAllocationInfo(IHttpResponse& responser, std::string devName, short port, short chNum)
	{
		TianShanIce::EdgeResource::AllocationInfos allocationInfos;
		//		readAllocations(allocationInfos, ALLOCATION_SAFESTORE);	

		TianShanIce::StrValues expectedMetaData;
		expectedMetaData.push_back(OwnerKey);
		expectedMetaData.push_back(UDP);
		expectedMetaData.push_back(ProgramNumber);
		expectedMetaData.push_back(SourceIP);
		expectedMetaData.push_back(BandWidth);
//		expectedMetaData.push_back(Status);
		expectedMetaData.push_back(MaximumJitter);
		expectedMetaData.push_back(StpCreated);
		expectedMetaData.push_back(StpProvisioned);
		expectedMetaData.push_back(StpCommitted);
		expectedMetaData.push_back(Expire);
		try
		{
			allocationInfos = _ERM->listAllocations(devName, port, chNum, expectedMetaData);
			for (int i=0; i<allocationInfos.size(); i++)
			{
				AllInfos.push_back(allocationInfos[i]);
			}
		}
		catch (const TianShanIce::BaseException& ex)
		{
// 			snprintf(szBuf, sizeof(szBuf) - 1, "list allocation caught %s:%s<br>", 
// 				ex.ice_name().c_str(), ex.message.c_str());
// 			setLastError(szBuf);
// 			glog(ErrorLog, CLOGFMT(ShowAllocation, "%s"), getLastError());
// 			responser.SetLastError(getLastError());
			return false;
		}
		catch (const Ice::Exception& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "list allocation caught %s<br>", 
				ex.ice_name().c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(ShowAllocation, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}
		catch (...)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "list allocation caught unknown exception<br>");
			setLastError(szBuf);
			responser.SetLastError(getLastError());
			return false;
		}

		return true;
	}
} // namespace ErmWebPage

