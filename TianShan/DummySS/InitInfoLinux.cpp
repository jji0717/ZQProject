
#include <stdio.h>
#include <string.h>
#include "InitInfoLinux.h"

InitInfoLinux::InitInfoLinux(void)
{
	_currentPos = 0;
}

InitInfoLinux::~InitInfoLinux(void)
{
}

bool InitInfoLinux::initConfig(const char* szFilename)
{
	FILE * fTemp = fopen(szFilename,"r");
	if( !fTemp)
		return false;
	char* buffer = new char [1024];
	memset(buffer, 0, sizeof(buffer));
	size_t len = sizeof(buffer);
	while(getline(&buffer, &len, fTemp) != -1)
	{
		std::string line (buffer);
		if ( line.find("[general]") != std::string::npos)
			_currentPos = 0;
		else if ( line.find("[streamers]") != std::string::npos)
			_currentPos = 1;
		else
			setConfig(line);
		memset(buffer, 0, sizeof(buffer) );
	}
	delete[] buffer;
	fclose(fTemp);
	return true;
}

void InitInfoLinux::setConfig(std::string& line)
{
	size_t lPos = line.find('=');
	if (lPos == std::string::npos)
		return;
	std::string name, value;
	name = line.substr(0, lPos);
	value = line.substr(lPos+1, line.length() - lPos -1);
	doString(name);
	doString(value);
	if (0 == _currentPos)
	{
		_confgInfo[name] = value;
	}
	else if (1 == _currentPos)
	{
		_streamers.push_back(value);
	}
}

void InitInfoLinux::doString(std::string& str)
{
	while( str[0] == ' ')
		str.erase(0);
	while( str[str.length() -1 ] == ' ')
		str.erase(str.length() - 1);
}
void InitInfoLinux::getValue (const char* keyValue, std::string& value)
{
	std::map<std::string, std::string >::iterator mapIter;
	mapIter = _confgInfo.find(keyValue);
	if (mapIter == _confgInfo.end() || mapIter->second.empty())
		return;
	value = mapIter->second;
}
void InitInfoLinux::getStreamers(std::vector<std::string >& streamers)
{
	streamers = _streamers;
}