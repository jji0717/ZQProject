// Utils.cpp: implementation of the Utils class.
//
//////////////////////////////////////////////////////////////////////

#ifdef ZQ_OS_MSWIN
#include "StdAfx.h"
#endif
#include "Utils.h"
#include <math.h>
#include "TianShanDefines.h"
#ifdef ZQ_OS_LINUX
extern "C"
{
#include <sys/stat.h>
#include <sys/types.h>
}
#endif

int Utils::getBitPos(uint32 n)
{
	int pos = -1;

	for (int i = 0; n && i < 32; i ++) {
		pos ++;
		if (n & 1) {

			if (n != 1)
				pos = -1;
			
			break;
		}
		
		n >>= 1;
	}

	return pos;
}

std::string Utils::getFileNamePart(const std::string& path)
{
	
	std::string::const_iterator it = path.end();
	while(it != path.begin()) {
		it --;
		if (*it == '\\' || *it == '/') {
			return std::string(it + 1, path.end());
		}
	}

	return std::string();
}

::TianShanIce::Variant& Utils::getValueMapItem(::TianShanIce::ValueMap& PD, 
									   const char* field)
{
	if (NULL == field || *field ==0x00) {
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("Utils", 0,
			"NULL field to access private data");
	}

	::TianShanIce::ValueMap::iterator it = PD.find(field);

	if (PD.end() == it) {
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("Utils", 0,
			"private data field is not found");
	}
	
	return it->second;
}

void Utils::setVariantInt(::TianShanIce::Variant& var, int value)
{
	var.type = TianShanIce::vtInts;
	var.ints.clear();
	var.ints.push_back(value);
}

void Utils::setVariantLongLong(::TianShanIce::Variant& var, int64 value)
{
	var.type = TianShanIce::vtLongs;
	var.lints.clear();
	var.lints.push_back(value);
}

void Utils::setVariantStr(::TianShanIce::Variant& var, 
						  const std::string& value)
{
	var.type = TianShanIce::vtStrings;
	var.strs.clear();
	var.strs.push_back(value);
}

void Utils::setVariantFlt(::TianShanIce::Variant& var, float value)
{
	var.type = TianShanIce::vtFloats;
	var.floats.clear();
	var.floats.push_back(value);
}

long Utils::getVariantInt(::TianShanIce::Variant& var)
{
	if (var.type != TianShanIce::vtInts || !var.ints.size()) {
		throw ::TianShanIce::InvalidParameter();
	}

	return var.ints[0];
}

int64 Utils::getVariantLongLong(::TianShanIce::Variant& var)
{
	if (var.type != TianShanIce::vtLongs || !var.strs.size()) {
		throw ::TianShanIce::InvalidParameter();
	}

	return var.lints[0];
}

std::string Utils::getVariantStr(::TianShanIce::Variant& var)
{
	if (var.type != TianShanIce::vtStrings || !var.strs.size()) {
		throw ::TianShanIce::InvalidParameter();
	}

	return var.strs[0];
}

float Utils::getVariantFlt(::TianShanIce::Variant& var)
{
	if (var.type != TianShanIce::vtFloats || !var.floats.size()) {
		throw ::TianShanIce::InvalidParameter();
	}

	return var.floats[0];
}

void Utils::createDir(const char* dirName)
{
	if(strlen(dirName)<1)
		return;

	char pathName[MAX_PATH];
	const char* c = dirName;
#ifdef ZQ_OS_MSWIN
	while (*c) {
		if (*c == FNSEPC) {
			size_t len = c - dirName;
			strncpy(pathName, dirName, len);
			pathName[len] = 0;
			CreateDirectoryA(pathName, NULL);
		}

		c ++;
	}

	CreateDirectoryA(dirName, NULL);
#else
	c++;
	while (*c) 
	{
		if (*c == FNSEPC)
		{
			size_t len = c - dirName;
			strncpy(pathName, dirName, len);
			pathName[len] = 0;
			mkdir(pathName, 0755);
		}

		c ++;
	}
	mkdir(dirName,0755);
#endif
}

