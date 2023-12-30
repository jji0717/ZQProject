#ifndef __VLCCMD_H__
#define __VLCCMD_H__

#include "ZQ_common_conf.h"
#include <iostream>
#include <string>
#include <list>
#include <sstream>

#include "TelnetParser.h"

typedef struct VLCPlayItem
{
	std::string	_path;		//file path
	int32		_length;	//file time length
	int32		_size;		//file size
	int32		_index;		//play item index in this playlist
	int32		_userCtrlNum;
}VLCPlayItem;
typedef std::list<VLCPlayItem> VLCPlayItemList;
class FindByPLCtrlNum
{
public:
	FindByPLCtrlNum(int32 iUserCtrlNum):_userCtrlNum(iUserCtrlNum){}

	bool operator ()(VLCPlayItem &item)
	{
		if (_userCtrlNum == item._userCtrlNum)
			return true;
		else
			return false;
	}
private:
	int32 _userCtrlNum;
};

typedef struct VLCOutputItem
{
	std::string _mux;		//mux coding type, default is ts
	std::string _access;	//stream out type, default is udp
	std::string _dstIp;		//destination ip address
	int32		_dstPort;	//destination port
}VLCOutputItem;

typedef struct VLCInputDelItem
{
	std::string _path;
}VLCInputDelItem;
typedef std::list<VLCInputDelItem> VLCInputDelItemList;

typedef struct CommonVLCPlaylist
{
	std::string _name;	//play list name, used in control and show command
	std::string _type;	//play list type, default is broadcast
	VLCPlayItemList	_inputItem;	//input item list to read
	VLCOutputItem	_ouputItem;	//output item
	VLCInputDelItemList	_inputDelItem;	//the item show be remove from current 
}CommonVLCPlaylist;

typedef struct VLCProp
{
	std::string prop;
}VLCProp;

typedef std::list<VLCProp> VLCProps;

static std::string strNull			= "";
static std::string strBlank			= " ";
static std::string strOutputBegin	= "#standard{";
static std::string strOutputEnd		= "}";
//VLC command
static std::string strVLCNew		= "new";
static std::string strVLCDel		= "del";
static std::string strVLCSetup		= "setup";
static std::string strVLCShow		= "show";
static std::string strVLCControl	= "control";

//VLC properties
static std::string strVLCInput		= "input";
static std::string strVLCInputdel	= "inputdel";
static std::string strVLCInputdeln	= "inputdeln";
static std::string strVLCOutput		= "output";
static std::string strVLCEnabled	= "enabled";
static std::string strVLCDisabled	= "Disabled";
static std::string strVLCLoop		= "loop";
static std::string strVLCUnloop		= "unloop";

//VLC control command
static std::string strVLCPlay		= "play";
static std::string strVLCPause		= "pause";
static std::string strVLCSeek		= "seek";
static std::string strVLCStop		= "stop";

std::string NewPlayList(const std::string &strPLName, const std::string &strType, VLCProps &props);
std::string SetupPlayList(const std::string &strPLName, VLCProps &props);
std::string DelPlayList(const std::string &strPLName);
std::string ShowPlayList(const std::string &strPLName);
std::string ControlPlayList(const std::string &strPLName, std::string &strCmd);

std::string SetProps(const std::string &strCmd, CommonVLCPlaylist &pl);

#endif