#ifndef __CodManWeb_DataTypes_H__
#define __CodManWeb_DataTypes_H__

#include <string>
#include <vector>

struct ChannelInfo
{
	std::string name;
	std::string onDmdName;
	Ice::Int maxBit;
	std::string desc;
	std::vector<std::string> netIds;
};

#endif // __CodManWeb_DataTypes_H__

