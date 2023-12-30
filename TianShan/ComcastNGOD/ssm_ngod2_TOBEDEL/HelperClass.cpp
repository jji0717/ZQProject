
#include "HelperClass.h"
#include <TianShandefines.h>
#include <TimeUtil.h>

void	HelperClass::getValueMapData( const TianShanIce::ValueMap& value , const std::string& key , Ice::Int& valueOut )
{
	TianShanIce::ValueMap::const_iterator it = value.find( key );
	if( it == value.end() )
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("StreamSmith",0 , "can't find data with key name[%s]" , key.c_str() );
	}
	const TianShanIce::Variant& var = it->second;
	if( var.type != TianShanIce::vtInts )
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("StreamSmith",0 , "data with key name[%s] do not have vtInt type" , key.c_str() );
	}
	if( var.ints.size( ) == 0 )
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("StreamSmith",0 , "data with key name[%s] do not have vtInt data" , key.c_str() );
	}
	valueOut = var.ints[0];
}
void	HelperClass::getValueMapData( const TianShanIce::ValueMap& value , const std::string& key , std::vector<Ice::Int>& valueOut,bool& bRange )
{
	TianShanIce::ValueMap::const_iterator it = value.find( key );
	if( it == value.end() )
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("StreamSmith",0 , "can't find data with key name[%s]" , key.c_str() );
	}
	const TianShanIce::Variant& var = it->second;
	if( var.type != TianShanIce::vtInts )
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("StreamSmith",0 , "data with key name[%s] do not have vtInt type" , key.c_str() );
	}
	if( var.ints.size( ) <= 1 )
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("StreamSmith",0 , "data with key name[%s] do not have enough vtInt data" , key.c_str() );
	}
	valueOut	= var.ints;
	bRange		= var.bRange;
}

void	HelperClass::getValueMapData( const TianShanIce::ValueMap& value , const std::string& key , Ice::Long& valueOut )
{
	TianShanIce::ValueMap::const_iterator it = value.find( key );
	if( it == value.end() )
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("StreamSmith",0 , "can't find data with key name[%s]" , key.c_str() );
	}
	const TianShanIce::Variant& var = it->second;
	if( var.type != TianShanIce::vtLongs )
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("StreamSmith",0 , "data with key name[%s] do not have vtLongs type" , key.c_str() );
	}
	if( var.lints.size( ) == 0 )
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("StreamSmith",0 , "data with key name[%s] do not have vtLongs data" , key.c_str() );
	}
	valueOut = var.lints[0];
}
void	HelperClass::getValueMapData( const TianShanIce::ValueMap& value , const std::string& key , std::vector<Ice::Long>& valueOut ,bool& bRange )
{
	TianShanIce::ValueMap::const_iterator it = value.find( key );
	if( it == value.end() )
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("StreamSmith",0 , "can't find data with key name[%s]" , key.c_str() );
	}
	const TianShanIce::Variant& var = it->second;
	if( var.type != TianShanIce::vtLongs )
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("StreamSmith",0 , "data with key name[%s] do not have vtLongs type" , key.c_str() );
	}
	if( var.lints.size( ) <= 1 )
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("StreamSmith",0 , "data with key name[%s] do not have vtLongs data" , key.c_str() );
	}
	valueOut	= var.lints;
	bRange		= var.bRange;
}

void	HelperClass::getValueMapData( const TianShanIce::ValueMap& value , const std::string& key , Ice::Float& valueOut )
{
	TianShanIce::ValueMap::const_iterator it = value.find( key );
	if( it == value.end() )
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("StreamSmith",0 , "can't find data with key name[%s]" , key.c_str() );
	}
	const TianShanIce::Variant& var = it->second;
	if( var.type != TianShanIce::vtFloats )
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("StreamSmith",0 , "data with key name[%s] do not have vtFloats type" , key.c_str() );
	}
	if( var.floats.size( ) == 0 )
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("StreamSmith",0 , "data with key name[%s] do not have vtFloats data" , key.c_str() );
	}
	valueOut = var.floats[0];
}
void	HelperClass::getValueMapData( const TianShanIce::ValueMap& value , const std::string& key , std::vector<Ice::Float>& valueOut ,bool& bRange )
{
	TianShanIce::ValueMap::const_iterator it = value.find( key );
	if( it == value.end() )
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("StreamSmith",0 , "can't find data with key name[%s]" , key.c_str() );
	}
	const TianShanIce::Variant& var = it->second;
	if( var.type != TianShanIce::vtFloats )
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("StreamSmith",0 , "data with key name[%s] do not have vtFloats type" , key.c_str() );
	}
	if( var.floats.size( ) <= 1 )
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("StreamSmith",0 , "data with key name[%s] do not have vtFloats data" , key.c_str() );
	}
	valueOut	= var.floats;
	bRange		= var.bRange;
}

void	HelperClass::getValueMapData( const TianShanIce::ValueMap& value , const std::string& key , std::string& valueOut )
{
	TianShanIce::ValueMap::const_iterator it = value.find( key );
	if( it == value.end() )
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("StreamSmith",0 , "can't find data with key name[%s]" , key.c_str() );
	}
	const TianShanIce::Variant& var = it->second;
	if( var.type != TianShanIce::vtStrings )
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("StreamSmith",0 , "data with key name[%s] do not have vtStrings type" , key.c_str() );
	}
	if( var.strs.size( ) == 0 )
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("StreamSmith",0 , "data with key name[%s] do not have vtStrings data" , key.c_str() );
	}
	valueOut = var.strs[0];
}
void	HelperClass::getValueMapData( const TianShanIce::ValueMap& value , const std::string& key , std::vector<std::string>& valueOut ,bool& bRange )
{
	TianShanIce::ValueMap::const_iterator it = value.find( key );
	if( it == value.end() )
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("StreamSmith",0 , "can't find data with key name[%s]" , key.c_str() );
	}
	const TianShanIce::Variant& var = it->second;
	if( var.type != TianShanIce::vtStrings )
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("StreamSmith",0 , "data with key name[%s] do not have vtStrings type" , key.c_str() );
	}
	if( var.strs.size( ) <= 1 )
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("StreamSmith",0 , "data with key name[%s] do not have vtStrings data" , key.c_str() );
	}
	valueOut	= var.strs;
	bRange		= var.bRange;
}

std::string NgodUtilsClass::generatorISOTime()
{
	SYSTEMTIME curTime;
	GetSystemTime(&curTime);
	char buf[256];
	ZQ::common::TimeUtil::Time2Iso(curTime, buf, sizeof(buf) - 1);
	std::string strISOTime = buf;
	return strISOTime;
}

std::string NgodUtilsClass::generatorNoticeString(const std::string strNoticeCode, 
												  const std::string strNoticeString, 
												  const std::string strNpt)
{
	std::string notice_str;
	notice_str = strNoticeCode + " \"" + strNoticeString + "\" " + "event-date=";
	SYSTEMTIME time;
	GetLocalTime(&time);
	char t[50];
	memset(t, 0, 50);
	snprintf(t, 49, "%04d%02d%02dT%02d%02d%02d.%03dZ npt=",time.wYear,time.wMonth,time.wDay,
		time.wHour,time.wMinute,time.wSecond,time.wMilliseconds);
	notice_str += t;
	notice_str += strNpt;
	return notice_str;
}