#include "AddDevice.h"
#include "DataTypes.h"
#include "EdgeRM.h"

namespace ErmWebPage
{
	AddDevice::AddDevice(IHttpRequestCtx* pHttpRequestCtx) : BasePage(pHttpRequestCtx)
	{
	}

	AddDevice::~AddDevice()
	{
	}

	bool AddDevice::get()
	{
		IHttpResponse& responser = _reqCtx->Response();

		// write page's functionality
		responser<<"<H2>New device</H2>";

		// write <form id="", method="" action="">
		url.clear();
		url.setPath(AddDevicePage);
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		snprintf(szBuf, sizeof(szBuf) - 1, "<form id='AddDevice' method='post' action='%s' onsubmit='return checkIP()'><br>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;

		responser<< "<script type='text/javascript'>";
		responser << "function checkIP()\n"
			<< "{\n"
			<<      "var pattern=/^(\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])\\.(\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])\\.(\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])\\.(\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])$/;\n"
			<<      "flag_ip=pattern.test(document.all.address.value);\n"
			<<      "if(!flag_ip)\n"
			<<      "{\n"
			<<           "alert(\"Invalid IP Format\");\n"
			<<           "document.all.address.focus();\n"
			<<           "return false;\n"
			<<      "}\n"
			<< "}\n";
		responser << "function getFocus()\n"
			<< "{\n"
			<< "	document.getElementById(\"address\").select();\n"
			<< "}\n";
		responser << "</script>\n";

		responser<<"<table cellpadding=\"0\" cellspacing=\"0\">";

		responser<<"<tr>";

		responser<< "<td><center>Zone</center></td>";

		snprintf(szBuf, sizeof(szBuf) - 1, "<td><input type=\"text\" id=\"zone\" name=\"zone\" value=\"%s\"/></td>", "");
		responser<<szBuf;

		responser<<"</tr>";
		responser<<"<tr>";

		responser<< "<td><center>Name</center></td>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<td><input type=\"text\" id=\"device\" name=\"device\" value=\"%s\"/></td>", "");
		responser<<szBuf;

		responser<<"</tr>";
		responser<<"<tr>";

		responser<< "<td><center>Address</center></td>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<td><input type=\"text\" id=\"address\" name=\"address\" value=\"%s\" onfocus='getFocus()'/></td>", "*.*.*.*");
		responser<<szBuf;

		responser<<"</tr>";
		responser<<"<tr>";

		responser<< "<td><center>MacAddress</center></td>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<td><input type=\"text\" id=\"macaddress\" name=\"macaddress\" value=\"%s\"/></td>", "");
		responser<<szBuf;

		responser<<"</tr>";
		responser<<"<tr>";

		responser<< "<td><center>Path</center></td>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<td><input type=\"file\" id=\"path\" name=\"path\" value=\"%s\"/></td>", "");
		responser<<szBuf;

		responser<<"</tr>";
		responser<<"<tr>";
		
		responser<<"<td><INPUT name=\"compressed\" type=\"radio\" value=\"Y\"> Compressed</td>";
		responser<<"<td><INPUT CHECKED name=\"compressed\" type=\"radio\" value=\"N\"> Not Compressed</td>";

		responser<<"</table>";
		responser<<"<br>";
		// write submit button
		responser<<"<input type=\"submit\" value=\"Update\"/>";

		// write </form>
		responser<<"</form>";

		return true;
	}

	bool AddDevice::post()
	{
		IHttpResponse& responser = _reqCtx->Response();

		// get input values
		std::string devName = _varMap["device"];
		std::string zone = _varMap["zone"];
		std::string path = _varMap["path"];
		std::string compressed = _varMap["compressed"];
		std::string address = _varMap["address"];
		std::string macaddress = _varMap["macaddress"];

		String::trimAll(zone);
		String::trimAll(devName);
		String::trimAll(path);
		String::trimAll(address);
		String::trimAll(macaddress);

		if(zone.empty() || devName.empty() || path.empty())
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "Imcomplete information<br>");
			setLastError(szBuf);
			responser.SetLastError(getLastError());
			return false;
		}

		if (devName.empty())
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "DeviceName is empty<br>");
			setLastError(szBuf);
			responser.SetLastError(getLastError());
			return false;
		}

		try
		{
			bool bCompress = 0;
			if(!compressed.empty() && compressed == "Y")
				bCompress = 1;
			TianShanIce::Properties privateData;
			privateData["DeviceIP/Address"] = address;
			privateData["DeviceIP/MacAddress"] = macaddress;
			TianShanIce::EdgeResource::EdgeRMPrx erm = TianShanIce::EdgeResource::EdgeRMPrx::uncheckedCast(_ERM);
			erm->importDevice(devName, zone, path, bCompress, privateData);
		}
		catch (const TianShanIce::BaseException& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "import device caught %s:%s<br>", 
				ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const Ice::Exception& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "import device caught %s<br>", 
				ex.ice_name().c_str());
			setLastError(szBuf);
			responser.SetLastError(getLastError());
			return false;
		}

		// redirect page to show channel page
		// <script language="javascript">
		//	document.location.href="http://www.cctv.com"
		// </script>
		url.clear();
		url.setPath(ShowDevicePage);
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
//		url.setVar(DeviceNameKey, devName.c_str());
		url.setVar(RouteNamesKey, zone.c_str());
		responser<<"<script language=\"javascript\">";
		snprintf(szBuf, sizeof(szBuf) - 1, "document.location.href=\"%s\"", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;
		responser<<"</script>";

		return true;
	}

} // namespace ErmWebPage

