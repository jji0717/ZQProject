// DataProfile.cpp: implementation of the DataProfile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DataProfile.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DataProfile::DataProfile()
{

}

DataProfile::~DataProfile()
{

}

bool DataProfile::retisterCategory(const std::string& name, 
	ProfileCategory& category)
{
	return 0;
}

bool DataProfile::unretisterCategory(const std::string& category)
{
	return 0;
}

ProfileCategory* getCategory(const std::string& category)
{
	return 0;
}

bool getCategoryNames(std::vector<std::string>& categoryNames)
{
	return 0;
}
