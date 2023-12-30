#include "ShowDevice.h"
#include "DataTypes.h"
#include "EdgeRM.h"
#include "TsEdgeResource.h"

namespace ErmWebPage
{
	ShowDevice::ShowDevice(IHttpRequestCtx* pHttpRequestCtx) : BasePage(pHttpRequestCtx)
	{
	}

	ShowDevice::~ShowDevice()
	{
	}

	bool ShowDevice::get()
	{
		IHttpResponse& responser = _reqCtx->Response();

		const char* isenable = _reqCtx->GetRequestVar("isDeviceEnable");
		if (isenable)
		{
			const char* names = _reqCtx->GetRequestVar("DeviceNames");
			TianShanIce::StrValues deviceName;
			splitString(deviceName, names, ";");
			bool bEnable = ((strcmp(isenable, "1") == 0) ? true : false);
			enableDevice(deviceName, bEnable);
		}

		const char* isdestroy = _reqCtx->GetRequestVar("isDestroy");
		if (isdestroy)
		{
			const char* names = _reqCtx->GetRequestVar("DeviceNames");
			TianShanIce::StrValues deviceName;
			splitString(deviceName, names, ";");
			destroy(deviceName);
		}

// 		url.clear();
// 		url.setPath(ShowDevicePage);
// 		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
// 		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
// 		snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Show Device</B></a>", String::getRightStr(url.generate(), "/", false).c_str());
// 		responser<<szBuf;
// 
// 		LinkSpace;
// 		url.clear();
// 		url.setPath(ShowEdgePortPage);
// 		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
// 		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
// 		snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Show RF Port</B></a>", String::getRightStr(url.generate(), "/", false).c_str());
// 		responser<<szBuf;
// 
// 		LinkSpace;
// 		url.clear();
// 		url.setPath(ShowChannelPage);
// 		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
// 		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
// 		snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Show Channel</B></a>", String::getRightStr(url.generate(), "/", false).c_str());
// 		responser<<szBuf;
// 
// 		LinkSpace;
// 		url.clear();
// 		url.setPath(ShowAllocationPage);
// 		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
// 		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
// 		snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Show Allocation</B></a>", String::getRightStr(url.generate(), "/", false).c_str());
// 		responser<<szBuf;
// 
// 		LinkSpace;
// 		url.clear();
// 		url.setPath(ShowServiceGroupPage);
// 		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
// 		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
// 		snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Show ServiceGroup</B></a>", String::getRightStr(url.generate(), "/", false).c_str());
// 		responser<<szBuf;

		TianShanIce::EdgeResource::EdgeDeviceInfos deviceInfos;
		TianShanIce::StrValues expectedMetaData;
		expectedMetaData.push_back(Zone);
		expectedMetaData.push_back(Vendor);
		expectedMetaData.push_back(Model);
		expectedMetaData.push_back(Description);
		expectedMetaData.push_back(TFTP);
		expectedMetaData.push_back(AdminUrl);
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

		std::string defaultZone = "";
		if(!_varMap[RouteNamesKey].empty())
			defaultZone = _varMap[RouteNamesKey];
		else
		{
			if(deviceInfos.size() && !deviceInfos[0].props[Zone].empty())
				defaultZone = deviceInfos[0].props[Zone];
		}

		url.clear();
		url.setPath(ShowDevicePage);
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());		
		snprintf(szBuf, sizeof(szBuf) - 1, "<form id='ShowDevice' method='post' action='%s'><br>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;

		responser<< "<script type='text/javascript'>";
		responser << "function enableDevices(isEnable)\n"
			<< "{\n"
			<<      "var DeviceNames=\"\";\n"
			<<      "var obj=document.getElementsByName(\"enableDevice\");\n"
			<<      "for(var i=0;i<obj.length;i++)\n"
			<<      "{\n"
			<<           "if(obj[i].checked == true)\n"
			<<           "{\n"
			<<              "DeviceNames+=obj[i].value;\n"
			<<              "DeviceNames+=\";\";\n"
			<<           "}\n"
			<<      "}\n"
			<<      "document.getElementById(\"isDeviceEnable\").value=isEnable;\n"
			<<      "document.getElementById(\"DeviceNames\").value=DeviceNames;\n"
			//<<      "alert(DeviceNames);\n"
			<<      "document.getElementById(\"ShowDevice\").submit();\n"
			<< "}\n";
		responser << "function destroy()\n"
			<< "{\n"
			<<      "var DeviceNames=\"\";\n"
			<<      "var obj=document.getElementsByName(\"enableDevice\");\n"
			<<      "for(var i=0;i<obj.length;i++)\n"
			<<      "{\n"
			<<           "if(obj[i].checked == true)\n"
			<<           "{\n"
			<<              "DeviceNames+=obj[i].value;\n"
			<<              "DeviceNames+=\";\";\n"
			<<           "}\n"
			<<      "}\n"
			<<      "document.getElementById(\"isDestroy\").value=true;\n"
			<<      "document.getElementById(\"DeviceNames\").value=DeviceNames;\n"
			<<      "document.getElementById(\"ShowDevice\").submit();\n"
			<< "}\n";
		responser << "function selectAll()\n"
			<< "{\n"
			<<      "var all=document.getElementById(\"all\");\n"
			<<      "var obj=document.getElementsByName(\"enableDevice\");\n"
			<<      "for(var i=0;i<obj.length;i++)\n"
			<<      "{\n"
			<<          "obj[i].checked=all.checked"
			<<      "}\n"
			<< "}\n";
		responser << "function isEnableAll()\n"
			<< "{\n"
			<<      "var count=0;\n"
			<<      "var obj=document.getElementsByName(\"enableDevice\");\n"
			// <<      "alert(obj.length);\n"
			<<      "for(var i=0;i<obj.length;i++)\n"
			<<      "{\n"
			<<          "if (obj[i].checked==true)\n"
			<<          "{\n"
			<<              "count++;\n"
			<<          "}\n"
			<<      "}\n"
			<<      "if(count!=obj.length)\n"
			<<      "{\n"
			<<           "document.getElementById(\"all\").checked=false;\n"
			<<      "}\n"
			<<       "else\n"
			<<      "{\n"
			<<           "document.getElementById(\"all\").checked=true;\n"
			<<      "}\n"
			<< "}\n";
//		responser << "document.getElementById('toolbar').innerHTML = toolbar.build();\n";
		responser << "</script>\n";
		
		responser << " <select name=\"search_type\" size=\"1\" >";
		responser << "	<option value=\"zone\" selected>Zone";
		responser << "	<option value=\"vendor\">Vendor";
		responser << " </select>";

		snprintf(szBuf, sizeof(szBuf) - 1, "<input type='text' id='search_value' name='search_value' value='%s'/>", defaultZone.c_str());
		responser<<szBuf;

		// the submit button
		responser<<"	<input type='submit' value='search'/>&nbsp&nbsp\n";
//		responser<<"    <input type=\"checkbox\" name=\"all\" id=\"all\" value=\"all\" onclick=\"selectAll()\"/>all&nbsp&nbsp\n"
//			<< "<input type=\"button\" value=\"enable\" onclick=\"enableDevices(1)\"/>&nbsp&nbsp\n"
//			<< "<input type=\"button\" value=\"disable\" onclick=\"enableDevices(0)\"/>&nbsp&nbsp\n"
//			<< "<input type=\"button\" value=\"destroy\" onclick=\"destroy()\"/>\n"
//			<< "<input type=\"hidden\" name=\"isDeviceEnable\" id=\"isDeviceEnable\" value=\"\"/>\n"
//			<< "<input type=\"hidden\" name=\"DeviceNames\" id=\"DeviceNames\"/>\n";
//			<< "<input type=\"hidden\" name=\"isDestroy\" id=\"isDestroy\" value=\"\"/>\n";

// 		url.clear();
// 		url.setPath(ShowDevicePage);
// 		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
// 		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
// 		snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Refresh</B></a>", String::getRightStr(url.generate(), "/", false).c_str());
// 		responser<<szBuf;

		// write </form>
		responser<<"</form>";

		// write start table flag
		responser<<"<table class='listTable'>";

		// write table header line
		responser<<"<tr>";
		responser<<"	<th><center>Zone</center></th>";
		responser<<"	<th><center>Device</center></th>";
		responser<<"	<th><center>Vendor</center></th>";
		responser<<"	<th><center>Model</center></th>";
		responser<<"	<th><center>Description</center></th>";
		responser<<"	<th><center>SessionEP</center></th>";
		responser<<"	<th><center>AdminEP</center></th>";
		responser<<"	<th><center>Active</center></th>";
		responser<<"	<th><center>RF-Port</center></th>";
		responser<<"</tr>";

		// write table data line
		for (DeviceInfos_iter it = deviceInfos.begin(); it != deviceInfos.end(); it++)
		{
			EdgeDeviceInfo deviceInfo = *it;
			
			std::string devName; 
			devName = _varMap[DeviceNameKey];
			if(!devName.empty() && deviceInfo.ident.name!=devName)
				continue;
			else
			{
				if(deviceInfo.props[Zone] != defaultZone)
					continue;
			}

			//to get device enable info
			std::string strEnable = "0";
			bool bFound = false;
			int portCount = 0;
			try
			{
				TianShanIce::EdgeResource::EdgeDevicePrx devicePrx; 
				TianShanIce::EdgeResource::EdgePortInfos  portInfos;
				devicePrx = _ERM->openDevice(deviceInfo.ident.name);
				portInfos = devicePrx->listEdgePorts();
				portCount =portInfos.size();
				for (PortInfos_iter port_it = portInfos.begin(); port_it != portInfos.end(); port_it++)
				{
					TianShanIce::StrValues expectedMetaData;
					expectedMetaData.push_back(Enabled);
					TianShanIce::EdgeResource::EdgeChannelInfos channelInfos;
					channelInfos = devicePrx->listChannels(port_it->Id, expectedMetaData, false);
					for (ChannelInfos_iter it = channelInfos.begin(); it != channelInfos.end(); it++)
					{
						if(it->props[Enabled] == "1")
						{
							strEnable = "1";
							bFound = true;
							break;
						}
					}
					if(bFound)
						break;
				}

			}
			catch (const TianShanIce::BaseException& ex)
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "list device enable caught %s:%s<br>", 
					ex.ice_name().c_str(), ex.message.c_str());
				setLastError(szBuf);
				glog(ErrorLog, CLOGFMT(ShowDevice, "%s"), getLastError());
				responser.SetLastError(getLastError());
				continue;
			}
			catch (const Ice::Exception& ex)
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "list device enable caught %s<br>", 
					ex.ice_name().c_str());
				setLastError(szBuf);
				glog(ErrorLog, CLOGFMT(ShowDevice, "%s"), getLastError());
				responser.SetLastError(getLastError());
				continue;
			}

			responser<<"<tr>";

			// show zone
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", deviceInfo.props[Zone].c_str());
			responser<<szBuf;

			// show device name
// 			snprintf(szBuf, sizeof(szBuf) - 1, "<td><input type='checkbox' name='enableDevice' id='enableDevice' onclick='isEnableAll()' value='%s' style='width:auto'>%s</td>", deviceInfo.ident.name.c_str(), deviceInfo.ident.name.c_str());
// 			responser<<szBuf;

			// show device name
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", deviceInfo.ident.name.c_str());
			responser<<szBuf;

			// show Vendor
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", deviceInfo.props[Vendor].c_str());
			responser<<szBuf;

			// show Model
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", deviceInfo.props[Model].c_str());
			responser<<szBuf;

			// show Description
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", deviceInfo.props[Description].c_str());
			responser<<szBuf;

			// show TFTP
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", deviceInfo.props[TFTP].c_str());
			responser<<szBuf;

			// show Telnet
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", deviceInfo.props[AdminUrl].c_str());
			responser<<szBuf;

			// show Enable
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", strEnable.c_str());
			responser<<szBuf;
/*
			// show a link to edit item page
			url.clear();
			url.setPath(EditDevicePage);
			url.setVar(DeviceNameKey, deviceInfo.ident.name.c_str());
			url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center><a href=\"%s\">Edit</a>", String::getRightStr(url.generate(), "/", false).c_str());
			responser<<szBuf;

			// show a link to delete item page
			url.clear();
			url.setPath(RemoveDevicePage);
			url.setVar(DeviceNameKey, deviceInfo.ident.name.c_str());
			url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			snprintf(szBuf, sizeof(szBuf) - 1, "<br><a href=\"%s\">Remove</a></center></td>", String::getRightStr(url.generate(), "/", false).c_str());
			responser<<szBuf;
*/
			// show a link to list port
			url.clear();
			url.setPath(ShowEdgePortPage);
			url.setVar(DeviceNameKey, deviceInfo.ident.name.c_str());
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			url.setVar(TemplateKey, neighborLayout(_varMap[TemplateKey].c_str(), "LOCAL_ERM_PORT").c_str());
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><a href=\"%s\">%d</a></center></td>", String::getRightStr(url.generate(), "/", false).c_str(), portCount);
			responser<<szBuf;

			responser<<"</tr>";
		}

		// write end table flag
		responser<<"</table>";
		responser<<"<br>";
/*
		url.clear();
		url.setPath(AddDevicePage);
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Add</B></a>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;
*/
		return true;
	}

	bool ShowDevice::post()
	{
		IHttpResponse& responser = _reqCtx->Response();

		const char* isenable = _reqCtx->GetRequestVar("isDeviceEnable");
		if (isenable)
		{
			const char* names = _reqCtx->GetRequestVar("DeviceNames");
			TianShanIce::StrValues deviceName;
			splitString(deviceName, names, ";");
			bool bEnable = ((strcmp(isenable, "1") == 0) ? true : false);
			enableDevice(deviceName, bEnable);
		}

		const char* isdestroy = _reqCtx->GetRequestVar("isDestroy");
		if (isdestroy)
		{
			const char* names = _reqCtx->GetRequestVar("DeviceNames");
			TianShanIce::StrValues deviceName;
			splitString(deviceName, names, ";");
			destroy(deviceName);
		}

		url.clear();
		url.setPath(ShowDevicePage);
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());		
		snprintf(szBuf, sizeof(szBuf) - 1, "<form id='ShowDevice' method='post' action='%s'><br>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;

		responser<< "<script type='text/javascript'>";
		responser << "function enableDevices(isEnable)\n"
			<< "{\n"
			<<      "var DeviceNames=\"\";\n"
			<<      "var obj=document.getElementsByName(\"enableDevice\");\n"
			<<      "for(var i=0;i<obj.length;i++)\n"
			<<      "{\n"
			<<           "if(obj[i].checked == true)\n"
			<<           "{\n"
			<<              "DeviceNames+=obj[i].value;\n"
			<<              "DeviceNames+=\";\";\n"
			<<           "}\n"
			<<      "}\n"
			<<      "document.getElementById(\"isDeviceEnable\").value=isEnable;\n"
			<<      "document.getElementById(\"DeviceNames\").value=DeviceNames;\n"
			//<<      "alert(DeviceNames);\n"
			<<      "document.getElementById(\"ShowDevice\").submit();\n"
			<< "}\n";
		responser << "function destroy()\n"
			<< "{\n"
			<<      "var DeviceNames=\"\";\n"
			<<      "var obj=document.getElementsByName(\"enableDevice\");\n"
			<<      "for(var i=0;i<obj.length;i++)\n"
			<<      "{\n"
			<<           "if(obj[i].checked == true)\n"
			<<           "{\n"
			<<              "DeviceNames+=obj[i].value;\n"
			<<              "DeviceNames+=\";\";\n"
			<<           "}\n"
			<<      "}\n"
			<<      "document.getElementById(\"isDestroy\").value=true;\n"
			<<      "document.getElementById(\"DeviceNames\").value=DeviceNames;\n"
			<<      "document.getElementById(\"ShowDevice\").submit();\n"
			<< "}\n";
		responser << "function selectAll()\n"
			<< "{\n"
			<<      "var all=document.getElementById(\"all\");\n"
			<<      "var obj=document.getElementsByName(\"enableDevice\");\n"
			<<      "for(var i=0;i<obj.length;i++)\n"
			<<      "{\n"
			<<          "obj[i].checked=all.checked"
			<<      "}\n"
			<< "}\n";
		responser << "function isEnableAll()\n"
			<< "{\n"
			<<      "var count=0;\n"
			<<      "var obj=document.getElementsByName(\"enableDevice\");\n"
			// <<      "alert(obj.length);\n"
			<<      "for(var i=0;i<obj.length;i++)\n"
			<<      "{\n"
			<<          "if (obj[i].checked==true)\n"
			<<          "{\n"
			<<              "count++;\n"
			<<          "}\n"
			<<      "}\n"
			<<      "if(count!=obj.length)\n"
			<<      "{\n"
			<<           "document.getElementById(\"all\").checked=false;\n"
			<<      "}\n"
			<<       "else\n"
			<<      "{\n"
			<<           "document.getElementById(\"all\").checked=true;\n"
			<<      "}\n"
			<< "}\n";
//		responser << "document.getElementById('toolbar').innerHTML = toolbar.build();\n";
		responser << "</script>\n";

		responser << " <select name=\"search_type\" size=\"1\" >";
		if(_varMap["search_type"] == "vendor")
		{
			responser << "	<option value=\"zone\">Zone";
			responser << "	<option value=\"vendor\" selected>Vendor";
		}
		if(_varMap["search_type"] == "zone")
		{
			responser << "	<option value=\"zone\" selected>Zone";
			responser << "	<option value=\"vendor\">Vendor";
		}
		responser << " </select>";

		snprintf(szBuf, sizeof(szBuf) - 1, "	<input type='text' id='search_value' name='search_value' value='%s'/>", _varMap["search_value"].c_str());
		responser<<szBuf;

		// the submit button
		responser<<"	<input type='submit' value='search'/>&nbsp&nbsp\n";
//		responser<<"    <input type=\"checkbox\" name=\"all\" id=\"all\" value=\"all\" onclick=\"selectAll()\"/>all&nbsp&nbsp\n"
//			<< "<input type=\"button\" value=\"enable\" onclick=\"enableDevices(1)\"/>&nbsp&nbsp\n"
//			<< "<input type=\"button\" value=\"disable\" onclick=\"enableDevices(0)\"/>&nbsp&nbsp\n"
//			<< "<input type=\"button\" value=\"destroy\" onclick=\"destroy()\"/>\n"
//			<< "<input type=\"hidden\" name=\"isDeviceEnable\" id=\"isDeviceEnable\" value=\"\"/>\n"
//			<< "<input type=\"hidden\" name=\"isDestroy\" id=\"isDestroy\" value=\"\"/>\n"
//			<< "<input type=\"hidden\" name=\"DeviceNames\" id=\"DeviceNames\"/>\n";

		// write </form>
		responser<<"</form>";

		// write start table flag
		responser<<"<table class='listTable'>";

		// write table header line
		responser<<"<tr>";
		responser<<"	<th><center>Zone</center></th>";
		responser<<"	<th><center>Device</center></th>";
		responser<<"	<th><center>Vendor</center></th>";
		responser<<"	<th><center>Model</center></th>";
		responser<<"	<th><center>Description</center></th>";
		responser<<"	<th><center>TFTP</center></th>";
		responser<<"	<th><center>Telnet</center></th>";
		responser<<"	<th><center>Enable</center></th>";
		responser<<"	<th><center>RF Port</center></th>";
		responser<<"</tr>";

		TianShanIce::EdgeResource::EdgeDeviceInfos deviceInfos;
		//		readDevices(deviceInfos, DEVICE_SAFESTORE);	

		TianShanIce::StrValues expectedMetaData;
		expectedMetaData.push_back(Zone);
		expectedMetaData.push_back(Vendor);
		expectedMetaData.push_back(Model);
		expectedMetaData.push_back(Description);
		expectedMetaData.push_back(TFTP);
		expectedMetaData.push_back(AdminUrl);
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

			//to get device enable info
			std::string strEnable = "0";
			bool bFound = false;
			int portCount = 0;
			try
			{
				TianShanIce::EdgeResource::EdgeDevicePrx devicePrx; 
				TianShanIce::EdgeResource::EdgePortInfos  portInfos;
				devicePrx = _ERM->openDevice(deviceInfo.ident.name);
				portInfos = devicePrx->listEdgePorts();
				portCount = portInfos.size();
				for (PortInfos_iter port_it = portInfos.begin(); port_it != portInfos.end(); port_it++)
				{
					TianShanIce::StrValues expectedMetaData;
					expectedMetaData.push_back(Enabled);
					TianShanIce::EdgeResource::EdgeChannelInfos channelInfos;
					channelInfos = devicePrx->listChannels(port_it->Id, expectedMetaData, false);
					for (ChannelInfos_iter it = channelInfos.begin(); it != channelInfos.end(); it++)
					{
						if(it->props[Enabled] == "1")
						{
							strEnable = "1";
							bFound = true;
							break;
						}
					}
					if(bFound)
						break;
				}

			}
			catch (const TianShanIce::BaseException& ex)
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "list device enable caught %s:%s<br>", 
					ex.ice_name().c_str(), ex.message.c_str());
				setLastError(szBuf);
				glog(ErrorLog, CLOGFMT(ShowDevice, "%s"), getLastError());
				responser.SetLastError(getLastError());
				continue;
			}
			catch (const Ice::Exception& ex)
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "list device enable caught %s<br>", 
					ex.ice_name().c_str());
				setLastError(szBuf);
				glog(ErrorLog, CLOGFMT(ShowDevice, "%s"), getLastError());
				responser.SetLastError(getLastError());
				continue;
			}

			if(_varMap["search_type"] == "vendor" && !_varMap["search_value"].empty())
			{
				if(deviceInfo.props[Vendor] != _varMap["search_value"])
					continue;
			}

			if(_varMap["search_type"] == "zone" && !_varMap["search_value"].empty())
			{
				if(deviceInfo.props[Zone] != _varMap["search_value"])
					continue;
			}

			responser<<"<tr>";

			// show zone
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", deviceInfo.props[Zone].c_str());
			responser<<szBuf;

			// show device name
// 			snprintf(szBuf, sizeof(szBuf) - 1, "<td><input type='checkbox' name='enableDevice' id='enableDevice' onclick='isEnableAll()' value='%s' style='width:auto'>%s</td>", deviceInfo.ident.name.c_str(), deviceInfo.ident.name.c_str());
// 			responser<<szBuf;

			// show device name
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", deviceInfo.ident.name.c_str());
			responser<<szBuf;

			// show Vendor
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", deviceInfo.props[Vendor].c_str());
			responser<<szBuf;

			// show Model
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", deviceInfo.props[Model].c_str());
			responser<<szBuf;

			// show Description
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", deviceInfo.props[Description].c_str());
			responser<<szBuf;

			// show TFTP
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", deviceInfo.props[TFTP].c_str());
			responser<<szBuf;

			// show Telnet
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", deviceInfo.props[AdminUrl].c_str());
			responser<<szBuf;

			// show Enable
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", strEnable.c_str());
			responser<<szBuf;
			/*
			// show a link to edit item page
			url.clear();
			url.setPath(EditDevicePage);
			url.setVar(DeviceNameKey, deviceInfo.ident.name.c_str());
			url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center><a href=\"%s\">Edit</a>", String::getRightStr(url.generate(), "/", false).c_str());
			responser<<szBuf;

			// show a link to delete item page
			url.clear();
			url.setPath(RemoveDevicePage);
			url.setVar(DeviceNameKey, deviceInfo.ident.name.c_str());
			url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			snprintf(szBuf, sizeof(szBuf) - 1, "<br><a href=\"%s\">Remove</a></center></td>", String::getRightStr(url.generate(), "/", false).c_str());
			responser<<szBuf;
			*/
			// show a link to list port
			url.clear();
			url.setPath(ShowEdgePortPage);
			url.setVar(DeviceNameKey, deviceInfo.ident.name.c_str());
//			url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
			url.setVar(TemplateKey, neighborLayout(_varMap[TemplateKey].c_str(), "LOCAL_ERM_PORT").c_str());
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><a href=\"%s\">%d</a></center></td>", String::getRightStr(url.generate(), "/", false).c_str(), portCount);
			responser<<szBuf;

			responser<<"</tr>";
		}

		// write end table flag
		responser<<"</table>";
		responser<<"<br>";
		/*
		url.clear();
		url.setPath(AddDevicePage);
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Add</B></a>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;
		*/

		return true;
	}

	bool ShowDevice::enableDevice(TianShanIce::StrValues& deviceNames, bool bEnable)
	{
/*
		if(deviceNames.size() <= 0)
			return false;
		for(int i=0; i<deviceNames.size(); i++)
		{
			try
			{
				TianShanIce::EdgeResource::EdgeDevicePrx devicePrx; 
				TianShanIce::EdgeResource::EdgePortInfos  portInfos;
				devicePrx = _ERM->openDevice(deviceNames[i]);
				portInfos = devicePrx->listEdgePorts();
				for (PortInfos_iter port_it = portInfos.begin(); port_it != portInfos.end(); port_it++)
				{
					TianShanIce::StrValues expectedMetaData;
					expectedMetaData.push_back(Enabled);
					TianShanIce::EdgeResource::EdgeChannelInfos channelInfos;
					channelInfos = devicePrx->listChannels(port_it->Id, expectedMetaData, false);
					for (ChannelInfos_iter it = channelInfos.begin(); it != channelInfos.end(); it++)
					{
						TianShanIce::EdgeResource::EdgeRMPrx edgeRM = TianShanIce::EdgeResource::EdgeRMPrx::uncheckedCast(_ERM);
						TianShanIce::EdgeResource::EdgeChannelPrx channelPrx = edgeRM->openChannel(it->ident.name);
						channelPrx->enable(bEnable);						
					}
				}
			}
			catch (const Ice::Exception& ex)
			{
				continue;
			}
		}
*/
		return true;
	}

	bool ShowDevice::destroy(TianShanIce::StrValues& deviceNames)
	{
		if(deviceNames.size() <= 0)
			return false;
		for(int i=0; i<deviceNames.size(); i++)
		{
			try
			{
				TianShanIce::EdgeResource::EdgeDevicePrx devicePrx; 
				devicePrx = _ERM->openDevice(deviceNames[i]);
				devicePrx->destroy();
			}
			catch (const Ice::Exception& ex)
			{
				continue;
			}
		}

		return true;
	}

} // namespace ErmWebPage

