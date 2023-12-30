#include "EditDevice.h"
#include "DataTypes.h"

namespace ErmWebPage
{
	EditDevice::EditDevice(IHttpRequestCtx* pHttpRequestCtx) : BasePage(pHttpRequestCtx)
	{
	}

	EditDevice::~EditDevice()
	{
	}

	bool EditDevice::get()
	{
		IHttpResponse& responser = _reqCtx->Response();

		std::string devName;
		devName = _varMap[DeviceNameKey];
		if (devName.empty())
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "No device name<br>");
			setLastError(szBuf);
			responser.SetLastError(getLastError());
			return false;
		}

		std::string zone;
		std::string vendor;
		std::string model;
		std::string desc;
		std::string tftp;
		std::string telnet;

		TianShanIce::EdgeResource::EdgeDeviceInfos deviceInfos;
//		readDevices(deviceInfos, DEVICE_SAFESTORE);
		for (DeviceInfos_iter it = deviceInfos.begin(); it != deviceInfos.end(); it++)
		{
			if(it->ident.name == devName)
			{
				zone = it->props[Zone];
				vendor = it->props[Vendor];
				model = it->props[Model];
				desc = it->props[Description];
				tftp = it->props[TFTP];
				telnet = it->props[AdminUrl];
				break;
			}
		}

		// write page's functionality
		snprintf(szBuf, sizeof(szBuf) - 1, "<H2>Edit device: [%s]</H2>", devName.c_str());
		responser<<szBuf;

		url.clear();
		url.setPath(EditDevicePage);
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(DeviceNameKey, devName.c_str());
		snprintf(szBuf, sizeof(szBuf) - 1, "<form id=\"EditDevice\" method=\"post\" action=\"%s\">", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;

		responser<<"<fieldset>";
		responser<<"<legend>Zone</legend>";
		responser<<"<label for=\"zone\">Zone:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"zone\" name=\"zone\" value=\"%s\"/>", zone.c_str());
		responser<<szBuf;
		responser<<"</fieldset>";

		responser<<"<fieldset>";
		responser<<"<legend>Device</legend>";
		responser<<"<label for=\"device\">Device:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"device\" name=\"device\" value=\"%s\"/>", devName.c_str());
		responser<<szBuf;
		responser<<"</fieldset>";


		responser<<"<fieldset>";
		responser<<"	<legend>Vendor</legend>";
		responser<<"	<label for=\"vendor\">Vendor:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"vendor\" name=\"vendor\" value=\"%s\"/>", vendor.c_str());
		responser<<szBuf;
		responser<<"</fieldset>";

		responser<<"<fieldset>";
		responser<<"	<legend>Modell</legend>";
		responser<<"	<label for=\"model\">Model:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"model\" name=\"model\" value=\"%s\"/>", model.c_str());
		responser<<szBuf;
		responser<<"</fieldset>";

		responser<<"<fieldset>";
		responser<<"	<legend>Description</legend>";
		responser<<"	<label for=\"desc\">Description:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"textarea\" clos=\"20\" rows=\"5\" id=\"desc\" name=\"desc\" value=\"%s\"/>", desc.c_str());
		responser<<szBuf;
		responser<<"</fieldset>";

		responser<<"<fieldset>";
		responser<<"	<legend>TFTP</legend>";
		responser<<"	<label for=\"tftp\">TFTP:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"tftp\" name=\"tftp\" value=\"%s\"/>", tftp.c_str());
		responser<<szBuf;
		responser<<"</fieldset>";

		responser<<"<fieldset>";
		responser<<"	<legend>Telnet</legend>";
		responser<<"	<label for=\"telnet\">Telnet:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"telnet\" name=\"telnet\" value=\"%s\"/>", telnet.c_str());
		responser<<szBuf;
		responser<<"</fieldset>";

		// write submit button
		responser<<"<input type=\"submit\" value=\"Update\"/>";

		// write </form>
		responser<<"</form>";

		return true;
	}

	bool EditDevice::post()
	{
		IHttpResponse& responser = _reqCtx->Response();

		std::string devName;
		devName = _varMap[DeviceNameKey];
		if (devName.empty())
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "No device name<br>");
			setLastError(szBuf);
			responser.SetLastError(getLastError());
			return false;
		}

		// get input values
		std::string zone = _varMap["zone"];
		std::string vendor = _varMap["vendor"];
		std::string model = _varMap["model"];
		std::string desc = _varMap["desc"];
		std::string tftp = _varMap["tftp"];
		std::string telnet = _varMap["telnet"];

		String::trimAll(zone);
		String::trimAll(vendor);
		String::trimAll(model);
		String::trimAll(desc);
		String::trimAll(tftp);
		String::trimAll(tftp);
		String::trimAll(telnet);

		if (devName.empty())
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "DeviceName is empty<br>");
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(EditDevice, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}

		EdgeDeviceInfo deviceInfo;
		deviceInfo.ident.name = devName;
		deviceInfo.props[Zone] = zone;
		deviceInfo.props[Vendor] = vendor;
		deviceInfo.props[Model] = model;
		deviceInfo.props[Description] = desc;
		deviceInfo.props[TFTP] = tftp;
		deviceInfo.props[AdminUrl] = telnet;
//		modifyDevice(deviceInfo, DEVICE_SAFESTORE);

		// redirect page to show device page
		// <script language="javascript">
		//	document.location.href="http://www.cctv.com"
		// </script>
		url.clear();
		url.setPath(ShowDevicePage);
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(DeviceNameKey, devName.c_str());
		responser<<"<script language=\"javascript\">";
		snprintf(szBuf, sizeof(szBuf) - 1, "document.location.href=\"%s\"", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;
		responser<<"</script>";

		return true;
	}

} // namespace ErmWebPage

