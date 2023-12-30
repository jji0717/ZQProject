
#ifndef __ZQ_TianShanIce_ICE__
#define __ZQ_TianShanIce_ICE__

#include <ZQIceVersion.ICE>
#include <Ice/Identity.ice>

/// TianShan represents an open architecture of VOD integration
module TianShanIce
{

sequence <string>	StrValues;

class BaseClass
{
	int i;
	int j;
	string a;
};

class TestClass extends BaseClass
{
	string          id;
	bool			bRange;
	StrValues       strs;
};

};

#endif // __ZQ_TianShanIce_ICE__

