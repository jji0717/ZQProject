#ifndef __TELNETPARSER_H__
#define __TELNETPARSER_H__

//include std header
#include <string>
#include <list>

//include tianshan commonstlp header
#include "Log.h"

static const char *PLState		= "state :";
static const char *PLPosition	= "position :";
static const char *PLIndex		= "playlistindex :";
static const char *PLEOF		= "\r\n";
static const char *PLUnknownCMD	= "Unknown command";
static const char *PLWrongCMD	= "Wrong command syntax";
static const char *PLUnknown	= "Unknown";
static const char *PLStatePlay	= "playing";
static const char *PLStatePause	= "paused";
static const char *PLStateStop	= "stop";

namespace ZQ{

namespace common{

class TelnetParser
{
public:
	TelnetParser(ZQ::common::Log *ZQLog);
	~TelnetParser();

	bool getContentByHeader(const std::string &msg, const char *header, std::string &res);
	bool checkError(const std::string &msg);
	bool checkPLName(const std::string &msg, const char *plName);

public:
	/// add by zjm
	/// get list according list name from "show" command, "media" and "schedule" are legal name only
	/// that is to say, this method only can parse response message from "show", "show media", "show schedule"
	bool getList(const std::string &msg, std::list<std::string>& lst, const std::string strListName = "media");

private:
	ZQ::common::Log *_log;
};

#define TELNETLOG if (_log) (*_log)
}//namespace common

}//namespace ZQ
#endif __TELNETPARSER_H__