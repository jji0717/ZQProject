#pragma once
#ifndef _INITINFOLINUX_H
#define _INITINFOLINUX_H

#include <map>
#include <vector>
#include <string>

class InitInfoLinux
{
public:
	InitInfoLinux(void);
	~InitInfoLinux(void);
public:
	bool initConfig (const char* szFilename);
	void getValue (const char* keyValue, std::string& value);
	void getStreamers(std::vector<std::string >& streamers);
protected:
	int									_currentPos;
	std::map<std::string, std::string >	_confgInfo;
	std::vector<std::string >			_streamers;

private:
	void setConfig (std::string& line);
	void doString (std::string& str);
};


#endif