#include "EditChannel.h"
#include "DataTypes.h"

namespace ErmWebPage
{
	EditChannel::EditChannel(IHttpRequestCtx* pHttpRequestCtx) : BasePage(pHttpRequestCtx)
	{
	}

	EditChannel::~EditChannel()
	{
	}

	bool EditChannel::get()
	{
		IHttpResponse& responser = _reqCtx->Response();

		std::string chnlName;
		chnlName = _varMap[ChannelNameKey];
		if (chnlName.empty())
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "No channel name<br>");
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(InsertItem, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}

		std::string qam;
		std::string adminState;
		std::string rf;
		std::string powerLevel;
		std::string modulation;
		std::string level;
		std::string mode;
		std::string tsid;
		std::string PAT;
		std::string PMT;

		TianShanIce::EdgeResource::EdgeChannelInfos channelInfos;
//		readChannels(channelInfos, CHANNEL_SAFESTORE);
		for (ChannelInfos_iter it = channelInfos.begin(); it != channelInfos.end(); it++)
		{
			if(it->ident.name == chnlName)
			{
				qam = it->props[Qam];
				adminState = it->props[Admin];
				rf = it->props[Freq];
				powerLevel = it->props[Power];
				modulation = it->props[Modulation];
				level = it->props[Level];
				mode = it->props[Mode];
				tsid = it->props[TSID];
				PAT = it->props[PAT_Interval];
				PMT = it->props[PMT_Interval];
				break;
			}
		}

		// write page's functionality
		snprintf(szBuf, sizeof(szBuf) - 1, "<H2>Edit channel: [%s]</H2>", chnlName.c_str());
		responser<<szBuf;

		url.clear();
		url.setPath(EditChannelPage);
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(ChannelNameKey, chnlName.c_str());
		snprintf(szBuf, sizeof(szBuf) - 1, "<form id=\"EditChannel\" method=\"post\" action=\"%s\">", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;

		responser<<"<fieldset>";
		responser<<"<legend>QAM</legend>";
		responser<<"<label for=\"qam\">QAM:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"qam\" name=\"qam\" value=\"%s\"/>", qam.c_str());
		responser<<szBuf;
		responser<<"</fieldset>";


		responser<<"<fieldset>";
		responser<<"<legend>Admin<br>State</legend>";
		responser<<"<label for=\"adminState\">Admin State:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"adminState\" name=\"adminState\" value=\"%s\"/>", adminState.c_str());
		responser<<szBuf;
		responser<<"</fieldset>";

		responser<<"<fieldset>";
		responser<<"	<legend>RF Freq<br>(Khz)</legend>";
		responser<<"	<label for=\"RF\">RF:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"RF\" name=\"RF\" value=\"%s\"/>", rf.c_str());
		responser<<szBuf;
		responser<<"</fieldset>";

		responser<<"<fieldset>";
		responser<<"	<legend>Power<br>Level</legend>";
		responser<<"	<label for=\"powerLevel\">Power Level:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"powerLevel\" name=\"powerLevel\" value=\"%s\"/>", powerLevel.c_str());
		responser<<szBuf;
		responser<<"</fieldset>";

		responser<<"<fieldset>";
		responser<<"	<legend>Modulation<br>Format</legend>";
		responser<<"	<label for=\"modulation\">Modulation Format:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"modulation\" name=\"modulation\" value=\"%s\"/>", modulation.c_str());
		responser<<szBuf;
		responser<<"</fieldset>";

		responser<<"<fieldset>";
		responser<<"	<legend>InterLeaver<br>Level</legend>";
		responser<<"	<label for=\"level\">InterLeaver Level:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"level\" name=\"level\" value=\"%s\"/>", level.c_str());
		responser<<szBuf;
		responser<<"</fieldset>";

		responser<<"<fieldset>";
		responser<<"	<legend>InterLeaver<br>Mode</legend>";
		responser<<"	<label for=\"mode\">InterLeaver Mode:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"mode\" name=\"mode\" value=\"%s\"/>", mode.c_str());
		responser<<szBuf;
		responser<<"</fieldset>";

		responser<<"<fieldset>";
		responser<<"	<legend>TSID</legend>";
		responser<<"	<label for=\"TSID\">TSID:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"TSID\" name=\"TSID\" value=\"%s\"/>", tsid.c_str());
		responser<<szBuf;
		responser<<"</fieldset>";

		responser<<"<fieldset>";
		responser<<"	<legend>PAT</legend>";
		responser<<"	<label for=\"PAT\">PAT:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"PAT\" name=\"PAT\" value=\"%s\"/>", PAT.c_str());
		responser<<szBuf;
		responser<<"</fieldset>";

		responser<<"<fieldset>";
		responser<<"	<legend>PMT</legend>";
		responser<<"	<label for=\"PMT\">PMT:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"PMT\" name=\"PMT\" value=\"%s\"/>", PMT.c_str());
		responser<<szBuf;
		responser<<"</fieldset>";

		// write submit button
		responser<<"<input type=\"submit\" value=\"Update\"/>";

		// write </form>
		responser<<"</form>";

		return true;
	}

	bool EditChannel::post()
	{
		IHttpResponse& responser = _reqCtx->Response();

		std::string chnlName;
		chnlName = _varMap[ChannelNameKey];
		if (chnlName.empty())
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "No channel name<br>");
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(InsertItem, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}

		// get input values
		std::string qam = _varMap["qam"];
		std::string adminState = _varMap["adminState"];
		std::string rf = _varMap["RF"];
		std::string powerLevel = _varMap["powerLevel"];
		std::string modulation = _varMap["modulation"];
		std::string level = _varMap["level"];
		std::string mode = _varMap["mode"];
		std::string tsid = _varMap["TSID"];
		std::string PAT = _varMap["PAT"];
		std::string PMT = _varMap["PMT"];

		String::trimAll(qam);
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
		channelInfo.ident.name = chnlName + RF;
		channelInfo.props[Qam] = qam;
		channelInfo.props[Admin] = adminState;
		channelInfo.props[Freq] = rf;
		channelInfo.props[Power] = powerLevel;
		channelInfo.props[Modulation] = modulation;
		channelInfo.props[Level] = level;
		channelInfo.props[Mode] = mode;
		channelInfo.props[TSID] = tsid;
		channelInfo.props[PAT_Interval] = PAT;
		channelInfo.props[PMT_Interval] = PMT;
//		modifyChannel(channelInfo, CHANNEL_SAFESTORE);

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

