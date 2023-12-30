#include "EditRouteNames.h"
#include "DataTypes.h"
#include "strHelper.h"

namespace ErmWebPage
{
	EditRouteNames::EditRouteNames(IHttpRequestCtx* pHttpRequestCtx) : BasePage(pHttpRequestCtx)
	{
	}

	EditRouteNames::~EditRouteNames()
	{
	}

	bool EditRouteNames::get()
	{
		IHttpResponse& responser = _reqCtx->Response();
		show(responser);
		return true;
	}

	bool EditRouteNames::post()
	{
		IHttpResponse& responser = _reqCtx->Response();
		return show(responser);
	}

	bool EditRouteNames::show(IHttpResponse& responser)
	{
		std::string devName;
		devName = _varMap[DeviceNameKey];
		if (devName.empty())
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "No device name<br>");
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(EditRouteNames, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}

		std::string portId;
		portId = _varMap[EdgePortKey];
		if (portId.empty())
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "No port ID<br>");
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(EditRouteNames, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}

		const char* isAdd = _reqCtx->GetRequestVar("isAdd");
		if (isAdd)
		{
			
			TianShanIce::StrValues routes;
			routes.clear();

			bool bAdd = ((strcmp(isAdd, "1") == 0) ? true : false);
			if(bAdd)
			{	
				const char* isUpdate = _reqCtx->GetRequestVar("isUpdate");
				if(isUpdate)
				{
					std::string rn, freq;
					if(!_varMap["RouteNames"].empty())
					{
						rn = _varMap["RouteNames"];
						/*
						for(int i = 0; i < sg.size(); i++)
						{
							if(!isdigit(sg[i]))
							{
								snprintf(szBuf, sizeof(szBuf) - 1, "Invalid input for ServiceGroup<br>");
								setLastError(szBuf);
								glog(ErrorLog, CLOGFMT(EditRouteNames, "%s"), getLastError());
								responser.SetLastError(getLastError());
								return false;
							}
						}
						*/
					}
					if(!_varMap["Frequence"].empty())
						freq = _varMap["Frequence"];
					bool bUpdate = ((strcmp(isUpdate, "1") == 0) ? true : false);
					if(bUpdate)
					{
						routes.push_back(rn);
						if(unlink(responser, devName, portId, routes))
							link(responser, devName, portId, rn, freq);
					}
					else
					{
						link(responser, devName, portId, rn, freq);
					}
				}

			}
			else
			{
				const char* names = _reqCtx->GetRequestVar("RouteNamesSelected");
				splitString(routes, names, ";");
				unlink(responser, devName, portId, routes);
			}
		}

		TianShanIce::EdgeResource::EdgeDevicePrx devicePrx;
		::TianShanIce::EdgeResource::RoutesMap routes;
		try
		{
			devicePrx = _ERM->openDevice(devName);
			routes = devicePrx->getRoutesRestriction(atoi(portId.c_str()));		
		}
		catch (const TianShanIce::BaseException& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "get Route Names Restriction caught %s:%s<br>", 
				ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(EditRouteNames, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const Ice::Exception& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "get Route Names Restriction caught %s<br>", 
				ex.ice_name().c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(EditRouteNames, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}
		catch (...)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "get Route Names Restriction caught unknown error<br>");
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(EditRouteNames, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}

		url.clear();
		url.setPath(ShowEdgePortPage);
		url.setVar(TemplateKey, neighborLayout(_varMap[TemplateKey].c_str(), "LOCAL_ERM_PORT").c_str());
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(DeviceNameKey, devName.c_str());
		url.setVar(EdgePortKey, portId.c_str());
		// write page's functionality
		snprintf(szBuf, sizeof(szBuf) - 1, "<H2>RouteNames of RFPort <u><i><a href=\"%s\">%s/%s</a></i></u></H2>", String::getRightStr(url.generate(), "/", false).c_str(), devName.c_str(), portId.c_str());
		responser<<szBuf;

		url.clear();
		url.setPath(EditRouteNamesPage);
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(DeviceNameKey, devName.c_str());
		url.setVar(EdgePortKey, portId.c_str());
		snprintf(szBuf, sizeof(szBuf) - 1, "<form id=\"EditRouteNames\" method=\"post\" action=\"%s\" onsubmit=\"return true\">", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;

		responser<< "<script type='text/javascript'>\n";
		responser << "function updateRN(isAdd)\n"
			<< "{\n"
			<<      "var RouteNamesSelected=\"\";\n"
			<<      "var obj=document.getElementsByName(\"toSelect\");\n"
			<<      "if(isAdd==false)"
			<<      "{\n"
			<<      "for(var i=0;i<obj.length;i++)\n"
			<<      "{\n"
			<<           "if(obj[i].checked == true)\n"
			<<           "{\n"
			<<              "RouteNamesSelected+=obj[i].value;\n"
			<<              "RouteNamesSelected+=\";\";\n"
			<<           "}\n"
			<<      "}\n"
			<<      "}\n"
			<<      "document.getElementById(\"isAdd\").value=isAdd;\n"
			<<      "document.getElementById(\"RouteNamesSelected\").value=RouteNamesSelected;\n"
			<<      "document.getElementById(\"EditRouteNames\").submit();\n"
			<< "}\n";
		responser << "function selectAll()\n"
			<< "{\n"
			<<      "var all=document.getElementById(\"all\");\n"
			<<      "var obj=document.getElementsByName(\"toSelect\");\n"
			<<      "for(var i=0;i<obj.length;i++)\n"
			<<      "{\n"
			<<          "obj[i].checked=all.checked;\n"
			<<      "}\n"
			<< "}\n";
		responser << "function isSelected()\n"
			<< "{\n"
			<<      "var count=0;\n"
			<<      "var obj=document.getElementsByName(\"toSelect\");\n"
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

		responser << "function getFocus()\n"
			<< "{\n"
			<< "	document.getElementById(\"Frequence\").select();\n"
			<< "}\n";
		responser << "function setUpdate(isUpdate)\n"
			<< "{\n"
			<<      "document.getElementById(\"isAdd\").value=1;\n"
			<<      "document.getElementById(\"isUpdate\").value=isUpdate;\n"
			<<      "document.getElementById(\"EditRouteNames\").submit();\n"
			<< "}\n";
		responser << "function rowSelected()\n"
			<< "{\n"
			<<      "var   td = event.srcElement;\n"
			<<      "var   obj=document.getElementById(\"tableview\");\n"
			<<      "var   tempRow=obj.getElementsByTagName(\"TR\")[td.parentElement.rowIndex];\n"
			<<      "var   cellArray=tempRow.getElementsByTagName(\"TD\");\n"
			<<      "document.getElementById(\"RouteNames\").value=cellArray[1].innerHTML;\n"
			<<      "document.getElementById(\"Frequence\").value=cellArray[2].innerHTML;\n"
			<< "}\n";
		/*
		responser << "function checkNumber()\n"
			<< "{\n"
			<<      "var pattern=/^\\d+$/;\n"
			<<      "flag_num=pattern.test(document.all.ServiceGroup.value);\n"
			<<      "if(!flag_num)\n"
			<<      "{\n"
			<<           "alert(\"Invalid input\");\n"
			<<           "document.all.ServiceGroup.focus();\n"
			<<           "return false;\n"
			<<      "}\n"
			<< "}\n";
		*/
//		responser << "document.getElementById('toolbar').innerHTML = toolbar.build();\n";
		responser << "</script>\n";

		// write start table flag
		responser<<"<table cellpadding='0' cellspacing='0'>";

		// write table header line
		responser<<"<tr>";
		responser<<"	<td>RouteNames</td>";
		responser<<"	<td>&nbsp&nbsp:<input type='text' id='RouteNames' name='RouteNames' width=\"10%\"/>";
		responser<<"</tr>";
		responser<<"<tr>";
		responser<<"	<td>Frequence</td>";
		responser<<"	<td>&nbsp&nbsp:<input type='text' id='Frequence' name='Frequence' value='' onfocus='getFocus()' style=\"width:260px;\"/></td>";
		responser<<"</tr>";
		responser<<"<tr>";
		responser<<"	<td></td>";
		responser<<"	<td>&nbsp&nbsp<i>empty means all frequences<br>use ; to seperate or use ~ to represent a range</i></td>";
		responser<<"</tr>";

		responser<<"</table>";

		responser<< "<br><input type=\"button\" value=\"link\" onclick=\"setUpdate(0)\"/>\n";
		responser<< "<input type=\"button\" value=\"update\" onclick=\"setUpdate(1)\"/>\n";

		responser << "<hr/>\n";
		// write submit button
		responser<< "<br><input type=\"checkbox\" name=\"all\" id=\"all\" value=\"all\" onclick=\"selectAll()\"/>all&nbsp&nbsp\n";
		responser<< "<input type=\"button\" value=\"unlink\" onclick=\"updateRN(0)\"/>&nbsp&nbsp\n";
		responser<< "<br><input type=\"hidden\" name=\"isAdd\" id=\"isAdd\" value=\"\"/>\n";
		responser<< "<br><input type=\"hidden\" name=\"isUpdate\" id=\"isUpdate\" value=\"\"/>\n";
		responser<< "<br><input type=\"hidden\" name=\"RouteNamesSelected\" id=\"RouteNamesSelected\"/>\n";

		responser<<"<table id='tableview' class='listTable' onclick='rowSelected()'>";

		responser<<"<tr>";
		responser<<"	<th></th>";
		responser<<"	<th><center>Route Names</center></th>";
		responser<<"	<th><center>Freq Range</center></th>";
		responser<<"</tr>";
		
		for(::TianShanIce::EdgeResource::RoutesMap::iterator iter = routes.begin(); iter != routes.end(); iter++)
		{
			responser<<"<tr>";
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><input type='checkbox' name='toSelect' id='toSelect' onclick='isSelected()' value='%s' style='width:auto'></td>", iter->first.c_str());
			responser<<szBuf;

			url.clear();
			url.setPath(ShowRouteNamesPage);
			url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			url.setVar(DeviceNameKey, devName.c_str());
			url.setVar(EdgePortKey, portId.c_str());
			char buf[256] = {0};
			snprintf(buf, sizeof(buf), "%s", iter->first.c_str());
			url.setVar(RouteNamesKey, buf);

			// show a link to restrict service group
			snprintf(szBuf, sizeof(szBuf) - 1, "<td onmouseover=\"this.style.cursor='hand';\">%s</td>", iter->first.c_str());
			responser<<szBuf;

			//show freqs
			TianShanIce::Variant var_freqs = iter->second;
			if(var_freqs.bRange == true)
			{
				int low,high;
				if(var_freqs.lints.size() >=2 )
				{
					low = var_freqs.lints[0];
					high = var_freqs.lints[1];
				}
				snprintf(szBuf, sizeof(szBuf) - 1, "<td onmouseover=\"this.style.cursor='hand';\">%d~%d</td>", low, high);	
				responser<<szBuf;
			}
			else
			{
				responser<<"<td onmouseover=\"this.style.cursor='hand';\">";
				if(var_freqs.lints.size())
				{
					for (int i = 0; i < var_freqs.lints.size(); i++)
					{	
						snprintf(szBuf, sizeof(szBuf) - 1, "%d", var_freqs.lints[i]);
						responser<<szBuf;
						if(i != var_freqs.lints.size() - 1)
						{
							responser<<";";
						}
					}
				}
				responser<<"</td>";
			}
			responser<<"</tr>";
		}

		// write </form>
		responser<<"</form>";

		responser<<"</table>";

		return true;
	}

	bool EditRouteNames::link(IHttpResponse& responser, std::string devName, std::string portId, std::string routeName, std::string& freqs)
	{
		TianShanIce::EdgeResource::EdgeDevicePrx devicePrx;
		TianShanIce::IValues currentGroups;
		try
		{
			TianShanIce::Variant var_freqs;
			var_freqs.type = ::TianShanIce::vtLongs;
			var_freqs.bRange = false;
			for(int i = 0; i < freqs.size(); i++)
			{
				if(!isdigit(freqs[i]) && freqs[i] != ';' && freqs[i] != '~')
				{
					return false;
				}
				else if(freqs[i] == '~')
				{
					var_freqs.bRange = true;
				}
			}
			if(var_freqs.bRange)
			{
				TianShanIce::StrValues strVec = ZQ::common::stringHelper::split(freqs, '~');
				if(strVec.size() < 2)
				{
					return false;
				}
				else
				{
					if(atol(strVec[0].c_str()) >= atol(strVec[1].c_str()))
					{
						return false;
					}
					var_freqs.lints.push_back(atol(strVec[0].c_str()));
					var_freqs.lints.push_back(atol(strVec[1].c_str()));
				}
			}
			else
			{
				TianShanIce::StrValues strVec = ZQ::common::stringHelper::split(freqs, ';');
				for(TianShanIce::StrValues::iterator it = strVec.begin(); it != strVec.end(); it++)
				{
					if(it->size() <= 0)
						continue;
					if(atol(it->c_str()) > 0)
						var_freqs.lints.push_back(atol(it->c_str()));
				}
			}
			devicePrx = _ERM->openDevice(devName);
			devicePrx->linkRoutes(atoi(portId.c_str()), routeName, var_freqs);
		}
		catch (const TianShanIce::BaseException& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "link Route Name Restriction caught %s:%s<br>", 
				ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(EditRouteNames, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const Ice::Exception& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "link Route Name Restriction caught %s<br>", 
				ex.ice_name().c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(EditRouteNames, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}
		catch (...)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "link Route Name Restriction caught unknown error<br>");
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(EditRouteNames, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}

		return true;

	}

	bool EditRouteNames::unlink(IHttpResponse& responser, std::string devName, std::string portId, const TianShanIce::StrValues& routeNames)
	{
		TianShanIce::EdgeResource::EdgeDevicePrx devicePrx;
		TianShanIce::IValues currentGroups;
		try
		{
			devicePrx = _ERM->openDevice(devName);
		}
		catch (const TianShanIce::BaseException& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "get Route Name Restriction caught %s:%s<br>", 
				ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(EditRouteNames, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const Ice::Exception& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "get Route Name Restriction caught %s<br>", 
				ex.ice_name().c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(EditRouteNames, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}
		catch (...)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "get Route Name Restriction caught unknown error<br>");
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(EditRouteNames, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}

		for(TianShanIce::StrValues::const_iterator iterStr = routeNames.begin(); iterStr != routeNames.end(); iterStr++)
		{
			try
			{
				//unlunk service groups	
				devicePrx->unlinkRoutes(atoi(portId.c_str()), *iterStr);
			}
			catch (const TianShanIce::BaseException& ex)
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "unlink Route Name caught %s<br>", 
					ex.ice_name().c_str());
				setLastError(szBuf);
				glog(ErrorLog, CLOGFMT(EditRouteNames, "%s"), getLastError());
				responser.SetLastError(getLastError());
				return false;
			}
			catch (const Ice::Exception& ex)
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "unlink Route Name caught %s<br>", 
					ex.ice_name().c_str());
				setLastError(szBuf);
				glog(ErrorLog, CLOGFMT(EditRouteNames, "%s"), getLastError());
				responser.SetLastError(getLastError());
				return false;
			}
			catch (...)
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "unlink Route Name caught unknown error<br>");
				setLastError(szBuf);
				glog(ErrorLog, CLOGFMT(EditRouteNames, "%s"), getLastError());
				responser.SetLastError(getLastError());
				return false;
			}
		}

		return true;
	}

} // namespace ErmWebPage

