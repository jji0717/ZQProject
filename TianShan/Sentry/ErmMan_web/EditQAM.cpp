#include "EditQAM.h"


namespace ErmWebPage
{
#define PostBackEditChannelPath ""

	EditQAM::EditQAM(IHttpRequestCtx* pHttpRequestCtx) : BasePage(pHttpRequestCtx)
	{
	}

	EditQAM::~EditQAM()
	{
	}

	bool EditQAM::get()
	{
		IHttpResponse& responser = _reqCtx->Response();

		std::string tempName = "Temp";
/*		tempName = _varMap[TemplateKey];
		if (tempName.empty())
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "No temp name<br>");
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(Edit QAM, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}
*/
		// write page's functionality
		snprintf(szBuf, sizeof(szBuf) - 1, "<H2>Edit QAM: [%s]</H2>", tempName.c_str());
		responser<<szBuf;

		url.clear();
		url.setPath(EditQAMPage);;
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		snprintf(szBuf, sizeof(szBuf) - 1, "<form id=\"EditQAM\" method=\"post\" action=\"%s\">", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;

		responser<<"<fieldset>";
		responser<<"<legend>On Demand Name</legend>";
		responser<<"<label for=\"onDemandName\">OnDemandName:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"onDemandName\" name=\"onDemandName\" value=\"%s\"/>", "");
		responser<<szBuf;
		responser<<"<font color=red>Use Channel's name if left empty</font>";
		responser<<"</fieldset>";

		responser<<"<fieldset>";
		responser<<"	<legend>Max Bitrates</legend>";
		responser<<"	<label for=\"maxBitrates\">MaxBitrates:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"maxBitrates\" name=\"maxBitrates\" value=\"%d\"/>", 0);
		responser<<szBuf;
		responser<<"</fieldset>";

		responser<<"<fieldset>";
		responser<<"	<legend>Description</legend>";
		responser<<"	<label for=\"description\">Description:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"description\" name=\"description\" value=\"%s\"/>", "");
		responser<<szBuf;
		responser<<"</fieldset>";

		responser<<"<fieldset>";
		responser<<"	<label for=\"restriction\">Restriction:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"restriction\" name=\"restriction\" value=\"%s\"/>", "");
		responser<<szBuf;
		responser<<"</fieldset>";

		// write submit button
		responser<<"<input type=\"submit\" value=\"Update\"/>";

		// write </form>
		responser<<"</form>";

		return true;
	}

	bool EditQAM::post()
	{
		IHttpResponse& responser = _reqCtx->Response();

		// get input values
		std::string onDmdName = _varMap["onDemandName"];
		std::string desc = _varMap["description"];
		std::string netId = _varMap["restriction"];
		std::string maxBit = _varMap["maxBitrates"];
		String::trimAll(onDmdName);
		String::trimAll(desc);
		String::trimAll(netId);
		String::trimAll(maxBit);



		// redirect page to show channel page
		// <script language="javascript">
		//	document.location.href="http://www.cctv.com"
		// </script>
		url.clear();
		url.setPath(ShowChannelPage);
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		responser<<"<script language=\"javascript\">";
		snprintf(szBuf, sizeof(szBuf) - 1, "document.location.href=\"%s\"", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;
		responser<<"</script>";

		return true;
	}

} // namespace ErmWebPage

