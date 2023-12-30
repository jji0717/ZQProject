#ifndef ZQ_CPE_COMMON_DEFINITION_H
#define ZQ_CPE_COMMON_DEFINITION_H
#include "BaseClass.h"
struct SubFileInfo
{
	char	ext[64];
	long	numerator;
	long	denominator;
	long	direction;	
	int64   firstOffset;
	int64   finalOffset;
	int64   totalFilesize;
	bool    bIsSparseFile;
};
class ParseIndexFileInfo
{
public:
	ParseIndexFileInfo(){};
	~ParseIndexFileInfo(){};
	static bool getIdxSubFileInfo(const char* szIdxFile, std::string  strIndexExt, std::vector<SubFileInfo>& subFiles, ZQTianShan::ContentProvision::MediaInfo& info);
};

#endif