#include "AddChannel.h"
#include "DataTypes.h"

namespace ErmWebPage
{
#define PostBackAddChannelPath ""

	AddChannel::AddChannel(IHttpRequestCtx* pHttpRequestCtx) : BasePage(pHttpRequestCtx)
	{
	}

	AddChannel::~AddChannel()
	{
	}

	bool AddChannel::get()
	{
		IHttpResponse& responser = _reqCtx->Response();

		// write page's functionality
		responser<<"<H2>New channel</H2>";

		// write <form id="", method="" action="">
		url.clear();
		url.setPath(AddChannelPage);
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		snprintf(szBuf, sizeof(szBuf) - 1, "<form id='AddChannel' method='post' action='%s'><i>* required fields</i><br>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;

		responser<<"<fieldset>";
		responser<<"<legend>QAM<br>Channel</legend>";
		responser<<"<label for=\"chnlName\">QAM Channel:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"chnlName\" name=\"chnlName\" value=\"%s\"/>", "");
		responser<<szBuf;
		responser<<"</fieldset>";

		responser<<"<fieldset>";
		responser<<"<legend>Admin<br>State</legend>";
		responser<<"<label for=\"adminState\">Admin State:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"adminState\" name=\"adminState\" value=\"%s\"/>", "");
		responser<<szBuf;
		responser<<"</fieldset>";

		responser<<"<fieldset>";
		responser<<"	<legend>RF Freq<br>(Khz)</legend>";
		responser<<"	<label for=\"RF\">RF:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"RF\" name=\"RF\" value=\"%.1f\"/>", 100.0);
		responser<<szBuf;
		responser<<"</fieldset>";

		responser<<"<fieldset>";
		responser<<"	<legend>Power<br>Level</legend>";
		responser<<"	<label for=\"powerLevel\">Power Level:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"powerLevel\" name=\"powerLevel\" value=\"%d\"/>", 50);
		responser<<szBuf;
		responser<<"</fieldset>";
/*
		responser<<"<fieldset>";
		responser<<"	<legend>Modulation<br>Format</legend>";
		responser<<"	<label for=\"modulation\">Modulation Format:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"modulation\" name=\"modulation\" value=\"%s\"/>", "");
		responser<<szBuf;
		responser<<"</fieldset>";
*/
		responser<<"<fieldset>";
		responser<<"	<legend>Modulation<br>Format</legend>";
		responser<<"	<label for=\"modulation\">Modulation Format:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<select name=\"modulation\" size=\"1\"><option value=\"unknown\">Unknown<option value=\"qam256\">QAM 256<option value=\"qam128\">QAM 128<option value=\"qam64\" selected>QAM 64<option value=\"qam32\">QAM 32<option value=\"qam16\">QAM 16</select>");
		responser<<szBuf;
		responser<<"</fieldset>";
/*			
		responser<<"<fieldset>";
		responser<<"	<legend>InterLeaver<br>Level</legend>";
		responser<<"	<label for=\"level\">InterLeaver Level:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"level\" name=\"level\" value=\"%s\"/>", "Level1");
		responser<<szBuf;
		responser<<"</fieldset>";
*/
		responser<<"<fieldset>";
		responser<<"	<legend>InterLeaver<br>Level</legend>";
		responser<<"	<label for=\"level\">InterLeaver Level:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<select name=\"level\" size=\"1\"><option value=\"Level1\"  selected>Level1<option value=\"Level2\">Level2</select>");
		responser<<szBuf;
		responser<<"</fieldset>";
/*
		responser<<"<fieldset>";
		responser<<"	<legend>InterLeaver<br>Mode</legend>";
		responser<<"	<label for=\"mode\">InterLeaver Mode:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"mode\" name=\"mode\" value=\"%s\"/>", "");
		responser<<szBuf;
		responser<<"</fieldset>";
*/
		responser<<"<fieldset>";
		responser<<"	<legend>InterLeaver<br>Mode</legend>";
		responser<<"	<label for=\"mode\">InterLeaver Mode:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<select name=\"mode\" size=\"1\"><option value=\"fecI128J1\" selected>fecI128J1<option value=\"fecI128J2\">fecI128J2<option value=\"fecI64J2\">fecI64J2<option value=\"fecI128J3\">fecI128J3<option value=\"fecI32J4\">fecI32J4<option value=\"fecI128J4\">fecI128J4<option value=\"fecI16J8\">fecI16J8<option value=\"fecI128J5\">fecI128J5<option value=\"fecI8J16\">fecI8J16<option value=\"fecI128J6\">fecI128J6<option value=\"fecI128J7\">fecI128J7<option value=\"fecI128J8\">fecI128J8</select>");
		responser<<szBuf;
		responser<<"</fieldset>";

		responser<<"<fieldset>";
		responser<<"	<legend>TSID</legend>";
		responser<<"	<label for=\"TSID\">TSID:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"TSID\" name=\"TSID\" value=\"%d\"/>", 200);
		responser<<szBuf;
		responser<<"</fieldset>";

		responser<<"<fieldset>";
		responser<<"	<legend>PAT</legend>";
		responser<<"	<label for=\"PAT\">PAT:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"PAT\" name=\"PAT\" value=\"%d\"/>", 44);
		responser<<szBuf;
		responser<<"</fieldset>";

		responser<<"<fieldset>";
		responser<<"	<legend>PMT</legend>";
		responser<<"	<label for=\"PMT\">PMT:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"PMT\" name=\"PMT\" value=\"%d\"/>", 444);
		responser<<szBuf;
		responser<<"</fieldset>";

		// write submit button
		responser<<"<input type=\"submit\" value=\"Update\"/>";

		// write </form>
		responser<<"</form>";

		return true;
	}

	bool AddChannel::post()
	{
		IHttpResponse& responser = _reqCtx->Response();

		// get input values
		std::string chnlName = _varMap["chnlName"];
		std::string adminState = _varMap["adminState"];
		std::string rf = _varMap["RF"];
		std::string powerLevel = _varMap["powerLevel"];
		std::string modulation = _varMap["modulation"];
		std::string level = _varMap["level"];
		std::string mode = _varMap["mode"];
		std::string tsid = _varMap["TSID"];
		std::string PAT = _varMap["PAT"];
		std::string PMT = _varMap["PMT"];

		String::trimAll(adminState);
		String::trimAll(rf);
		String::trimAll(powerLevel);
		String::trimAll(modulation);
		String::trimAll(level);
		String::trimAll(mode);
		String::trimAll(tsid);
		String::trimAll(PAT);
		String::trimAll(PMT);

		if (chnlName.empty())
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "ChannelName is empty<br>");
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(EditChannel, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}

		EdgeChannelInfo channelInfo;
		bool NotExist = true;

		TianShanIce::EdgeResource::EdgeChannelInfos channelInfos;
//		readChannels(channelInfos, CHANNEL_SAFESTORE);
		for (ChannelInfos_iter it = channelInfos.begin(); it != channelInfos.end(); it++)
		{
			channelInfo = *it;
			if(channelInfo.ident.name == chnlName + RF)
			{
				NotExist = false;
				break;
			}
		}

		if(NotExist)
		{
			channelInfo.ident.name = chnlName + RF;
			channelInfo.props[Qam] = chnlName;
			channelInfo.props[Admin] = adminState;
			channelInfo.props[Freq] = rf;
			channelInfo.props[Power] = powerLevel;
			channelInfo.props[Modulation] = modulation;
			channelInfo.props[Level] = level;
			channelInfo.props[Mode] = mode;
			channelInfo.props[TSID] = tsid;
			channelInfo.props[PAT_Interval] = PAT;
			channelInfo.props[PMT_Interval] = PMT;
//			appendChannel(channelInfo, CHANNEL_SAFESTORE);
		}


		// redirect page to show channel page
		// <script language="javascript">
		//	document.location.href="http://www.cctv.com"
		// </script>
		url.clear();
		url.setPath(ShowChannelPage);
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(ChannelNameKey, chnlName.c_str());
		responser<<"<script language=\"javascript\">";
		snprintf(szBuf, sizeof(szBuf) - 1, "document.location.href=\"%s\"", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;
		responser<<"</script>";

		return true;
	}

} // namespace ErmWebPage

