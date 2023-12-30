#include "RemoveDevice.h"

namespace ErmWebPage
{

	RemoveDevice::RemoveDevice(IHttpRequestCtx* pHttpRequestCtx) : BasePage(pHttpRequestCtx)
	{
	}

	RemoveDevice::~RemoveDevice()
	{
	}

	bool RemoveDevice::get()
	{
		IHttpResponse& responser = _reqCtx->Response();

		std::string devName;
		devName = _varMap[DeviceNameKey];
		if (devName.empty())
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "No Device name<br>");
			setLastError(szBuf);
			responser.SetLastError(getLastError());
			return false;
		}

//		removeDevice(devName, DEVICE_SAFESTORE);

		// redirect page to show Device page
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
		return true;
	}

	bool RemoveDevice::post()
	{
		return false;
	}

} // namespace ErmWebPage

