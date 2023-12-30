#include "VLCCmd.h"

std::string NewPlayList(const std::string &strPLName, const std::string &strType, VLCProps &props)
{
	std::string cmd;

	//add playlist
	cmd += strVLCNew + strBlank + strPLName + strBlank + strType + strBlank + "enabled";

	for (VLCProps::iterator iter = props.begin(); iter != props.end(); iter++)
	{
		cmd += strBlank + iter->prop;
	}

	return cmd;
}

std::string SetupPlayList(const std::string &strPLName, VLCProps &props)
{
	std::string cmd;

	//add playlist
	cmd += strVLCSetup + strBlank + strPLName;

	//add input
	for (VLCProps::iterator iter = props.begin(); iter != props.end(); iter++)
		cmd += strBlank + iter->prop;

	//add output
	//if (!playlist._ouputItem._access.empty() && !playlist._ouputItem._mux.empty() && !playlist._ouputItem._dstIp.empty() && !playlist._ouputItem._dstPort > 0)
	//{
	//	cmd += strBlank + strVLCOutput;
	//	cmd += strBlank + strOutputBegin + "access=" + playlist._ouputItem._access;
	//	cmd	+= ",mux=" + playlist._ouputItem._mux;
	//	std::stringstream ss;
	//	ss << playlist._ouputItem._dstPort;
	//	cmd += ",dst=" + playlist._ouputItem._dstIp + ":" + ss.str();
	//	cmd += strOutputEnd;
	//}

	return cmd;
}

std::string DelPlayList(const std::string &strPLName)
{
	std::string cmd;
	cmd += strVLCDel + strBlank + strPLName;
	return cmd;
}

std::string ShowPlayList(const std::string &strPLName)
{
	std::string cmd;
	cmd += strVLCShow + strBlank + strPLName;
	return cmd;
}

std::string ControlPlayList(const std::string &strPLName, std::string &strCmd)
{
	std::string cmd;
	cmd += strVLCControl + strBlank + strPLName + strBlank + strCmd;
	return cmd;
}

std::string SetProps(const std::string &strCmd, CommonVLCPlaylist &pl)
{
	std::string cmd;
	if (strCmd.compare(strVLCInput) == 0)
	{
		for (VLCPlayItemList::iterator iter = pl._inputItem.begin(); iter != pl._inputItem.end(); iter++)
			cmd += strBlank + strCmd + strBlank + iter->_path;
	}
	else if (strCmd.compare(strVLCOutput) == 0)
	{
		cmd += strCmd;
		if ((!pl._ouputItem._access.empty()) && (!pl._ouputItem._mux.empty()) && (!pl._ouputItem._dstIp.empty()) && (pl._ouputItem._dstPort > 0))
		{
			cmd += strBlank + strOutputBegin + "access=" + pl._ouputItem._access;
			cmd	+= ",mux=" + pl._ouputItem._mux;
			std::stringstream ss;
			ss << pl._ouputItem._dstPort;
			cmd += ",dst=" + pl._ouputItem._dstIp + ":" + ss.str();
			cmd += strOutputEnd;
		}
	}
	else if (strCmd.compare(strVLCInputdel) == 0)
	{
		for (VLCInputDelItemList::iterator iter = pl._inputDelItem.begin(); iter != pl._inputDelItem.end(); iter++)
			cmd += strBlank + strCmd + strBlank + iter->_path;
	}
	else
	{
		return strNull;
	}
	return cmd;
}