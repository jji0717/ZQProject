// Utils.h: interface for the Utils class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UTILS_H__DC95CA93_9DBE_4081_B667_D39B9922EE35__INCLUDED_)
#define AFX_UTILS_H__DC95CA93_9DBE_4081_B667_D39B9922EE35__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ZQ_common_conf.h"
#include "TianShanIce.h"

class Utils  
{
public:
	static int getBitPos(uint32 n);
	static std::string getFileNamePart(const std::string& path);

	static ::TianShanIce::Variant& getValueMapItem(
		::TianShanIce::ValueMap& PD, 
		const char* field);

	static void setVariantInt(::TianShanIce::Variant& var, int value);
	static void setVariantLongLong(::TianShanIce::Variant& var, int64 value);
	static void setVariantStr(::TianShanIce::Variant& var, 
		const std::string& value);
	static void setVariantFlt(::TianShanIce::Variant& var, float value);

	static long getVariantInt(::TianShanIce::Variant& var);
	static int64 getVariantLongLong(::TianShanIce::Variant& var);
	static std::string getVariantStr(::TianShanIce::Variant& var);
	static float getVariantFlt(::TianShanIce::Variant& var);

	static void createDir(const char* dirName);
};

#endif // !defined(AFX_UTILS_H__DC95CA93_9DBE_4081_B667_D39B9922EE35__INCLUDED_)
