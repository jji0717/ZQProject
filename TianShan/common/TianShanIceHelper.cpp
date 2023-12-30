
#include "TianShanDefines.h"
#include <assert.h>
#include "TianShanIceHelper.h"
#include <TimeUtil.h>

#ifdef ZQ_OS_LINUX
#include <sys/stat.h>
#include <sys/types.h>
#endif


namespace ZQTianShan
{
namespace Util
{

#define STRSWITCH() if(0){
#define STRCASE(x)	} else if(::strncmp( it->c_str() , x ,strlen(x) ) == 0 ){
#define STRENDCASE() }

void	updateStreamInfoValue( TianShanIce::StrValues& value , const Ice::Int mask )
{
	value.clear();
	if ( mask & VALUE_WANTED_ITEM_NPT  )			{value.push_back("ITEM_CURRENTPOS");}
	if ( mask & VALUE_WANTED_ITEM_DURATION )		{value.push_back("ITEM_TOTALPOS"); }
	if ( mask & VALUE_WANTED_STREAM_NPT )			{value.push_back("CURRENTPOS");}
	if ( mask & VALUE_WANTED_STREAM_DURATION )		{value.push_back("TOTALPOS");}
	if ( mask & VALUE_WANTED_STREAM_SPEED )			{value.push_back("SPEED");}
	if ( mask & VALUE_WANTED_STREAM_STATE )			{value.push_back("STATE");}
	if ( mask & VALUE_WANTED_USER_CTRLNUM )			{value.push_back("USERCTRLNUM");}
}
const char* resourceTypeToString( const TianShanIce::SRM::ResourceType& type )
{
	using namespace TianShanIce::SRM;
	switch( type )
	{
		case rtURI:	return "rtURI";
		case rtStorage: return "rtStorage";
		case rtStreamer: return "rtStreamer";
		case rtServiceGroup: return "rtServiceGroup";
		case rtProvisionBandwidth: return "rtProvisionBandwidth";
		case rtMpegProgram: return "rtMpegProgram";
		case rtTsDownstreamBandwidth: return "rtTsDownstreamBandwidth";
		case rtIP: return "rtIP";
		case rtEthernetInterface: return "rtEthernetInterface";
		case rtPhysicalChannel: return "rtPhysicalChannel";
		case rtAtscModulationMode: return "rtAtscModulationMode";
		case rtHeadendId: return "rtHeadendId";
		case rtClientConditionalAccess: return "rtClientConditionalAccess";
		case rtServerConditionalAccess: return "rtServerConditionalAccess";
		case rtContentProvision: return "rtContentProvision";
		case rtTsUpstreamBandwidth: return "rtTsUpstreamBandwidth";
		default:	return "unknown";
	}
}
std::string dumpStreamInfoValues( const TianShanIce::StrValues& value ,const std::string& delimiter )
{
	std::string strRet ;
	TianShanIce::StrValues::const_iterator it = value.begin( );
	for( ; it != value.end() ; it ++ )
	{
		strRet = strRet + *it;
		strRet = strRet + delimiter;
	}
	return strRet;
}
std::string		dumpTianShanIceStrValues(  const TianShanIce::StrValues& values ,const std::string& delimiter )
{
	return dumpStreamInfoValues( values , delimiter );
}
std::string		dumpStringSets(  const std::set<std::string>& values ,const std::string& delimiter )
{
	std::string strRet ;
	std::set<std::string>::const_iterator it = values.begin( );
	for( ; it != values.end() ; it ++ )
	{
		strRet = strRet + *it;
		strRet = strRet + delimiter;
	}
	return strRet;
}

std::string		dumpStringMap( const std::map<std::string,std::string>& values )
{
	std::ostringstream oss;
	std::map<std::string , std::string>::const_iterator it = values.begin();
	for( ; it != values.end() ; it ++ )
	{
		oss<<"["<<it->first<<":"<<it->second<<"] ";
	}
	return oss.str();
}

void	getStreamInfoValue( Ice::Int& mask , const TianShanIce::StrValues& value )
{
	mask = 0;
	TianShanIce::StrValues::const_iterator it = value.begin();
	for( ; it != value.end() ; it ++ )
	{
		STRSWITCH()
			STRCASE("ITEM_CURRENTPOS")		mask |= VALUE_WANTED_ITEM_NPT;
			STRCASE("ITEM_TOTALPOS")		mask |= VALUE_WANTED_ITEM_DURATION;
			STRCASE("CURRENTPOS")			mask |= VALUE_WANTED_STREAM_NPT;
			STRCASE("TOTALPOS")				mask |= VALUE_WANTED_STREAM_DURATION;
			STRCASE("SPEED")				mask |= VALUE_WANTED_STREAM_SPEED;
			STRCASE("STATE")				mask |= VALUE_WANTED_STREAM_STATE;
			STRCASE("USERCTRLNUM")			mask |= VALUE_WANTED_USER_CTRLNUM;
		STRENDCASE()
	}
}

const char*	dumpTianShanStreamState( const TianShanIce::Streamer::StreamState& state )
{
	switch( state )
	{
	case TianShanIce::Streamer::stsSetup:
		return "stsSetup";
	case TianShanIce::Streamer::stsStreaming:
		return "stsStreaming";
	case TianShanIce::Streamer::stsPause:
		return "stsPause";
	case TianShanIce::Streamer::stsStop:
		return "stsStop";
	default:
		return "Unknown State";
	}
}

void	getPropertyData( const TianShanIce::Properties& props , const std::string& key , Ice::Int& valueOut ) throw (::TianShanIce::InvalidParameter)
{
	TianShanIce::Properties::const_iterator it = props.find( key );
	if( it == props.end() || it->second.empty() )
	{
		throw TianShanIce::InvalidParameter(); 
	}
	sscanf(it->second.c_str(),"%d",&valueOut );
}
void	getPropertyData( const TianShanIce::Properties& props , const std::string& key , Ice::Float& valueOut ) throw (::TianShanIce::InvalidParameter)
{
	TianShanIce::Properties::const_iterator it = props.find( key );
	if( it == props.end() || it->second.empty() )
	{
		throw TianShanIce::InvalidParameter(); 
	}
	sscanf(it->second.c_str(),"%f",&valueOut );
}
void	getPropertyData( const TianShanIce::Properties& props , const std::string& key , Ice::Long& valueOut )  throw (::TianShanIce::InvalidParameter)
{
	TianShanIce::Properties::const_iterator it = props.find( key );
	if( it == props.end() || it->second.empty() )
	{
		throw TianShanIce::InvalidParameter(); 
	}
	sscanf(it->second.c_str(),FMT64,&valueOut );
}
void	getPropertyData( const TianShanIce::Properties& props , const std::string& key , std::string& valueOut ) throw (::TianShanIce::InvalidParameter)
{
	TianShanIce::Properties::const_iterator it = props.find( key );
	if( it == props.end() || it->second.empty() )
	{
		throw TianShanIce::InvalidParameter(); 
	}
	valueOut = it->second;
}

void	getPropertyDataWithDefault( const TianShanIce::Properties& props , const std::string& key , const Ice::Int& defaultValue, Ice::Int& valueOut )
{
	TianShanIce::Properties::const_iterator it = props.find( key );
	if( it == props.end() || it->second.empty() )
	{
		valueOut = defaultValue ;
	}
	else
	{
		sscanf(it->second.c_str(),"%d",&valueOut );
	}
}
void	getPropertyDataWithDefault( const TianShanIce::Properties& props , const std::string& key , const Ice::Float& defaultValue, Ice::Float& valueOut )
{
	TianShanIce::Properties::const_iterator it = props.find( key );
	if( it == props.end() || it->second.empty() )
	{
		valueOut = defaultValue ;
	}
	else
	{
		sscanf(it->second.c_str(),"%f",&valueOut );
	}
}
void	getPropertyDataWithDefault( const TianShanIce::Properties& props , const std::string& key , const Ice::Long& defaultValue, Ice::Long& valueOut )
{
	TianShanIce::Properties::const_iterator it = props.find( key );
	if( it == props.end() || it->second.empty() )
	{
		valueOut = defaultValue ;
	}
	else
	{
		sscanf(it->second.c_str(),FMT64,&valueOut );
	}
}
void	getPropertyDataWithDefault( const TianShanIce::Properties& props , const std::string& key , const std::string defaultValue, std::string& valueOut )
{
	TianShanIce::Properties::const_iterator it = props.find( key );
	if( it == props.end() || it->second.empty() )
	{
		valueOut = defaultValue ;
	}
	else
	{
		valueOut = it->second;
	}
}

TianShanIce::Properties getResourceMapAsProperties( const TianShanIce::SRM::ResourceMap& resMap )
{
	TianShanIce::Properties props;
	TianShanIce::SRM::ResourceMap::const_iterator it = resMap.begin();
	for( ; it != resMap.end() ; it ++ )
	{
		TianShanIce::Properties p = getValueMapAsProperties( it->second.resourceData );
		
		if( p.size() > 0 )
		{
			TianShanIce::Properties pv ;
			pv[ ResourceTypeStr( it->first ) ] = p.begin()->second;
			mergeProperty( props , pv );
		}
	}
	return props;
}

TianShanIce::Properties getValueMapAsProperties( const  TianShanIce::ValueMap& vm )
{
	TianShanIce::Properties props;
	std::ostringstream oss;
	TianShanIce::ValueMap::const_iterator it = vm.begin();
	for ( ; it != vm.end() ; it ++ )
	{
		oss.str("");
		switch( it->second.type )
		{
		case TianShanIce::vtInts:
			{
				if( it->second.ints.size()  == 1 )
				{
					oss << it->second.ints[0];
				}
				else if( it->second.ints.size() > 1 )
				{
					oss << it->second.ints[0]<<"-"<<it->second.ints[1];
				}
			}
			break;
		case TianShanIce::vtLongs:
			{
				if( it->second.lints.size()  == 1 )
				{
					oss << it->second.lints[0];
				}
				else if( it->second.lints.size() > 1 )
				{
					oss << it->second.lints[0]<<"-"<<it->second.lints[1];
				}
			}
			break;
		case TianShanIce::vtFloats:
			{
				if( it->second.floats.size() == 1 )
				{
					oss << it->second.floats[0];
				}
				else if( it->second.floats.size() > 1 )
				{
					oss << it->second.floats[0]<<"-"<<it->second.floats[1];
				}
			}
			break;
		case TianShanIce::vtStrings:
			{
				if( it->second.strs.size() == 1 )
				{
					oss << it->second.strs[0];
				}
				else if( it->second.strs.size() > 1 )
				{
					oss << it->second.strs[0]<<"-"<<it->second.strs[1];
				}
			}
			break;
		default:
			{
				
			}
			break;
		}
		props[it->first] = oss.str();
	}

	return props;
}

void	getValueMapData( const TianShanIce::ValueMap& value , const std::string& key , Ice::Byte& valueOut, size_t pos )
{
	TianShanIce::ValueMap::const_iterator it = value.find( key );
	if( it == value.end() )
	{
		throw TianShanIce::InvalidParameter("ValueMap", 404, key + " not found"); 
	}
	const TianShanIce::Variant& var = it->second;
	if( var.type != TianShanIce::vtBin )
	{
		throw TianShanIce::InvalidParameter("ValueMap", 417, key + " is non-byte"); 
	}
	if( var.bin.size( ) < (pos+1) )
	{
		throw TianShanIce::InvalidParameter("ValueMap", 416, key + " unexpected size");  
	}
	valueOut = var.bin[pos];
}

void	updateValueMapData( TianShanIce::ValueMap& value , const std::string& key , const  Ice::Byte& valueIn )
{
	TianShanIce::ValueMap::iterator it = value.find(key);
	if( it == value.end() )
	{
		TianShanIce::Variant var;
		var.type	=	TianShanIce::vtBin;
		var.bRange	=	false;
		var.bin.clear();
		var.bin.push_back(valueIn);
		value[key]	=	var;
	}
	else
	{
		TianShanIce::Variant& var = it->second;
		var.type	=	TianShanIce::vtBin;
		var.bin.clear();
		var.bin.push_back(valueIn);
	}
}

void	getValueMapData( const TianShanIce::ValueMap& value , const std::string& key , std::vector<Ice::Byte>& valueOut, bool& bRange )
{
	TianShanIce::ValueMap::const_iterator it = value.find( key );
	if( it == value.end() )
	{
		throw TianShanIce::InvalidParameter("ValueMap", 404, key + " not found");
	}
	const TianShanIce::Variant& var = it->second;
	if( var.type != TianShanIce::vtBin )
	{
		throw TianShanIce::InvalidParameter("ValueMap", 417, key + " is non-bytes");  
	}
	if( var.bin.size( ) <= 1 )
	{
		throw TianShanIce::InvalidParameter("ValueMap", 416, key + " unexpected size"); 
	}
	valueOut	= var.bin;
	bRange		= var.bRange;
}
void	updateValueMapData( TianShanIce::ValueMap& value , const std::string& key , const  std::vector<Ice::Byte>& valueIn , bool bRange )
{
	TianShanIce::ValueMap::iterator it = value.find(key);
	if( it == value.end() )
	{
		TianShanIce::Variant var;
		var.type	=	TianShanIce::vtBin;
		var.bin	=	valueIn;
		var.bRange	=	bRange;
		value[key]	=	var;
	}
	else
	{
		TianShanIce::Variant& var = it->second;
		var.type	=	TianShanIce::vtBin;
		var.bin	=	valueIn;
		var.bRange	=	bRange;
	}
}

void	getValueMapData( const TianShanIce::ValueMap& value , const std::string& key , Ice::Int& valueOut, size_t pos )
{
	TianShanIce::ValueMap::const_iterator it = value.find( key );
	if( it == value.end() )
	{
		throw TianShanIce::InvalidParameter("ValueMap", 404, key + " not found");
	}
	const TianShanIce::Variant& var = it->second;
	if( var.type != TianShanIce::vtInts )
	{
		throw TianShanIce::InvalidParameter("ValueMap", 417, key + " is non-int");
	}
	if( var.ints.size( ) < (pos+1) )
	{
		throw TianShanIce::InvalidParameter("ValueMap", 416, key + " unexpected size");
	}
	valueOut = var.ints[pos];
}
void	updateValueMapData( TianShanIce::ValueMap& value , const std::string& key , const  Ice::Int& valueIn )
{
	TianShanIce::ValueMap::iterator it = value.find(key);
	if( it == value.end() )
	{
		TianShanIce::Variant var;
		var.type	=	TianShanIce::vtInts;
		var.bRange	=	false;
		var.ints.clear();
		var.ints.push_back(valueIn);
		value[key]	=	var;
	}
	else
	{
		TianShanIce::Variant& var = it->second;
		var.type	=	TianShanIce::vtInts;
		var.ints.clear();
		var.ints.push_back(valueIn);
	}
}

void	getValueMapData( const TianShanIce::ValueMap& value , const std::string& key , std::vector<Ice::Int>& valueOut,bool& bRange )
{
	TianShanIce::ValueMap::const_iterator it = value.find( key );
	if( it == value.end() )
	{
		throw TianShanIce::InvalidParameter("ValueMap", 404, key + " not found");
	}
	const TianShanIce::Variant& var = it->second;
	if( var.type != TianShanIce::vtInts )
	{
		throw TianShanIce::InvalidParameter("ValueMap", 417, key + " is non-int");
	}
	if( var.ints.size( ) <= 1 )
	{
		throw TianShanIce::InvalidParameter("ValueMap", 416, key + " unexpected size");
	}
	valueOut	= var.ints;
	bRange		= var.bRange;
}
void	updateValueMapData( TianShanIce::ValueMap& value , const std::string& key , const  std::vector<Ice::Int>& valueIn , bool bRange )
{
	TianShanIce::ValueMap::iterator it = value.find(key);
	if( it == value.end() )
	{
		TianShanIce::Variant var;
		var.type	=	TianShanIce::vtInts;
		var.ints	=	valueIn;
		var.bRange	=	bRange;
		value[key]	=	var;
	}
	else
	{
		TianShanIce::Variant& var = it->second;
		var.type	=	TianShanIce::vtInts;
		var.ints	=	valueIn;
		var.bRange	=	bRange;
	}
}

void	getValueMapData( const TianShanIce::ValueMap& value , const std::string& key , Ice::Long& valueOut , size_t pos )
{
	TianShanIce::ValueMap::const_iterator it = value.find( key );
	if( it == value.end() )
	{
		throw TianShanIce::InvalidParameter("ValueMap", 404, key + " not found"); 
	}
	const TianShanIce::Variant& var = it->second;
	if( var.type != TianShanIce::vtLongs )
	{
		throw TianShanIce::InvalidParameter("ValueMap", 417, key + " is non-long");
	}
	if( var.lints.size( ) < (pos+1) )
	{
		throw TianShanIce::InvalidParameter("ValueMap", 416, key + " unexpected size"); 
	}
	valueOut = var.lints[pos];
}
void	updateValueMapData( TianShanIce::ValueMap& value , const std::string& key , const  Ice::Long& valueIn )
{
	TianShanIce::ValueMap::iterator it = value.find(key);
	if( it == value.end() )
	{
		TianShanIce::Variant var;
		var.type	=	TianShanIce::vtLongs;
		var.lints.clear();
		var.lints.push_back(valueIn);
		value[key]	=	var;
	}
	else
	{
		TianShanIce::Variant& var = it->second;
		var.type	=	TianShanIce::vtLongs;
		var.lints.clear();
		var.lints.push_back(valueIn);
	}
}

void	getValueMapData( const TianShanIce::ValueMap& value , const std::string& key , std::vector<Ice::Long>& valueOut ,bool& bRange )
{
		TianShanIce::ValueMap::const_iterator it = value.find( key );
	if( it == value.end() )
	{
		throw TianShanIce::InvalidParameter("ValueMap", 404, key + " not found");
	}
	const TianShanIce::Variant& var = it->second;
	if( var.type != TianShanIce::vtLongs )
	{
		throw TianShanIce::InvalidParameter("ValueMap", 417, key + " is non-long");
	}
	if( var.lints.size( ) <= 1 )
	{
		throw TianShanIce::InvalidParameter("ValueMap", 416, key + " unexpected size");
	}
	valueOut	= var.lints;
	bRange		= var.bRange;
}

void	updateValueMapData( TianShanIce::ValueMap& value , const std::string& key , const  std::vector<Ice::Long>& valueIn , bool bRange )
{
	TianShanIce::ValueMap::iterator it = value.find(key);
	if( it == value.end() )
	{
		TianShanIce::Variant var;
		var.bRange	= bRange;
		var.type	= TianShanIce::vtLongs;
		var.lints	= valueIn;
		value[key]	= var;
	}
	else
	{
		TianShanIce::Variant& var = it->second;
		var.bRange	= bRange;
		var.type	= TianShanIce::vtLongs;
		var.lints	= valueIn;
	}
}


void	getValueMapData( const TianShanIce::ValueMap& value , const std::string& key , Ice::Float& valueOut ,size_t pos )
{
	TianShanIce::ValueMap::const_iterator it = value.find( key );
	if( it == value.end() )
	{
		throw TianShanIce::InvalidParameter("ValueMap", 404, key + " not found");
	}
	const TianShanIce::Variant& var = it->second;
	if( var.type != TianShanIce::vtFloats )
	{
		throw TianShanIce::InvalidParameter("ValueMap", 417, key + " is non-float");
	}
	if( var.floats.size( ) < (pos+1) )
	{
		throw TianShanIce::InvalidParameter("ValueMap", 416, key + " unexpected size");
	}
	valueOut = var.floats[pos];
}

void	updateValueMapData(  TianShanIce::ValueMap& value , const std::string& key , const  Ice::Float& valueIn )
{
	TianShanIce::ValueMap::iterator it = value.find(key);
	if( it == value.end() )
	{
		TianShanIce::Variant var;
		var.type	=	TianShanIce::vtFloats;
		var.floats.clear();
		var.floats.push_back(valueIn);
		value[key]	=	var;
	}
	else
	{
		TianShanIce::Variant& var = it->second;
		var.type	=	TianShanIce::vtFloats;
		var.floats.clear();
		var.floats.push_back(valueIn);
	}
}

void	getValueMapData( const TianShanIce::ValueMap& value , const std::string& key , std::vector<Ice::Float>& valueOut ,bool& bRange )
{
	TianShanIce::ValueMap::const_iterator it = value.find( key );
	if( it == value.end() )
	{
		throw TianShanIce::InvalidParameter("ValueMap", 404, key + " not found"); 
	}
	const TianShanIce::Variant& var = it->second;
	if( var.type != TianShanIce::vtFloats )
	{
		throw TianShanIce::InvalidParameter("ValueMap", 417, key + " is non-float");
	}
	if( var.floats.size( ) <= 1 )
	{
		throw TianShanIce::InvalidParameter("ValueMap", 416, key + " unexpected size");
	}
	valueOut	= var.floats;
	bRange		= var.bRange;
}

void updateValueMapData( TianShanIce::ValueMap& value , const std::string& key , const  std::vector<Ice::Float>& valueIn , bool bRange )
{
	TianShanIce::ValueMap::iterator it = value.find(key);
	if( it == value.end() )
	{
		TianShanIce::Variant var;
		var.type	=	TianShanIce::vtFloats;
		var.floats	=	valueIn;
		var.bRange	=	bRange;
		value[key]	=	var;
	}
	else
	{
		TianShanIce::Variant& var = it->second;
		var.type	=	TianShanIce::vtFloats;
		var.floats	=	valueIn;
		var.bRange	=	bRange;
	}
}

void	getValueMapData( const TianShanIce::ValueMap& value , const std::string& key , std::string& valueOut , size_t pos )
{
	TianShanIce::ValueMap::const_iterator it = value.find( key );
	if( it == value.end() )
	{
		throw TianShanIce::InvalidParameter("ValueMap", 404, key + " not found");
	}
	const TianShanIce::Variant& var = it->second;
	if( var.type != TianShanIce::vtStrings )
	{
		throw TianShanIce::InvalidParameter("ValueMap", 417, key + " is non-string");
	}
	if( var.strs.size( ) < (pos+1) )
	{
		throw TianShanIce::InvalidParameter("ValueMap", 416, key + " unexpected size");
	}
	valueOut = var.strs[pos];
}

void	updateValueMapData( TianShanIce::ValueMap& value , const std::string& key , const  std::string& valueIn )
{
	TianShanIce::ValueMap::iterator it = value.find(key);
	if( it == value.end() )
	{
		TianShanIce::Variant var;
		var.type	=	TianShanIce::vtStrings;
		var.bRange	=	false;
		var.strs.clear();
		var.strs.push_back(valueIn);
		value[key]	=	var;
	}
	else
	{
		TianShanIce::Variant& var = it->second;
		var.type	=	TianShanIce::vtStrings;
		var.bRange	=	false;
		var.strs.clear();
		var.strs.push_back(valueIn);
	}
}

void	getValueMapData( const TianShanIce::ValueMap& value , const std::string& key , std::vector<std::string>& valueOut ,bool& bRange )
{
	TianShanIce::ValueMap::const_iterator it = value.find( key );
	if( it == value.end() )
	{
		throw TianShanIce::InvalidParameter("ValueMap", 404, key + " not found");
	}
	const TianShanIce::Variant& var = it->second;
	if( var.type != TianShanIce::vtStrings )
	{
		throw TianShanIce::InvalidParameter("ValueMap", 417, key + " is non-string");
	}
	if( var.strs.size( ) < 1 )
	{
		throw TianShanIce::InvalidParameter("ValueMap", 416, key + " unexpected size");
	}
	valueOut	= var.strs;
	bRange		= var.bRange;
}

void	updateValueMapData( TianShanIce::ValueMap& value , const std::string& key ,  const std::vector<std::string>& valueIn , bool bRange )
{
	TianShanIce::ValueMap::iterator it = value.find(key);
	if( it ==value.end() )
	{
		TianShanIce::Variant var;
		var.type	=	TianShanIce::vtStrings;
		var.bRange	=	bRange;
		var.strs	=	valueIn;
		value[key]	=	var;
	}
	else
	{
		TianShanIce::Variant& var = it->second;
		var.type	=	TianShanIce::vtStrings;
		var.bRange	=	bRange;
		var.strs	=	valueIn;
	}
}

bool getValueMapDataWithDefault( const TianShanIce::ValueMap& value , const std::string& key , const Ice::Byte& valueDefault , Ice::Byte& valueOut , size_t pos )
{
	TianShanIce::ValueMap::const_iterator it = value.find( key );
	if( it == value.end() )
	{
		valueOut = valueDefault;
		return false;
	}
	const TianShanIce::Variant& var = it->second;
	if( var.type != TianShanIce::vtBin )
	{
		valueOut = valueDefault;
		return false;		
	}
	if( var.bin.size( ) < 1 )
	{
		valueOut = valueDefault;
		return false;
	}

	if( var.bin.size() >= (pos+1) )
	{
		valueOut	= var.bin[pos];
	}
	else
	{
		valueOut =valueDefault;
	}

	return true;
}

bool getValueMapDataWithDefault( const TianShanIce::ValueMap& value , const std::string& key , const Ice::Int& valueDefault , Ice::Int& valueOut, size_t pos )
{
	TianShanIce::ValueMap::const_iterator it = value.find( key );
	if( it == value.end() )
	{
		valueOut = valueDefault;
		return false;
	}

	const TianShanIce::Variant& var = it->second;
	if( var.type != TianShanIce::vtInts )
	{
		valueOut = valueDefault;
		return false;		
	}

	if( var.ints.size( ) < 1 )
	{
		valueOut = valueDefault;
		return false;
	}	

	if( var.ints.size() >= (pos+1) )
	{
		valueOut	= var.ints[pos];
	}
	else
	{
		valueOut = valueDefault;
	}

	return true;
}

bool getValueMapDataWithDefault( const TianShanIce::ValueMap& value , const std::string& key , const Ice::Long& valueDefault , Ice::Long& valueOut, size_t pos )
{
	TianShanIce::ValueMap::const_iterator it = value.find( key );
	if( it == value.end() )
	{
		valueOut = valueDefault;
		return false;
	}
	const TianShanIce::Variant& var = it->second;
	if( var.type != TianShanIce::vtLongs )
	{
		valueOut = valueDefault;
		return false;		
	}
	if( var.lints.size( ) < 1 )
	{
		valueOut = valueDefault;
		return false;
	}	
	else
	{
		if( var.lints.size() >= (pos+1) )
		{
			valueOut	= var.lints[pos];
		}
		else
		{
			valueOut = valueDefault;
		}
	}
	
	return true;
}

bool getValueMapDataWithDefault( const TianShanIce::ValueMap& value , const std::string& key , const Ice::Float& valueDefault , Ice::Float& valueOut , size_t pos )
{
	TianShanIce::ValueMap::const_iterator it = value.find( key );
	if( it == value.end() )
	{
		valueOut = valueDefault;
		return false;
	}
	const TianShanIce::Variant& var = it->second;
	if( var.type != TianShanIce::vtFloats )
	{
		valueOut = valueDefault;
		return false;		
	}

	if( var.floats.size( ) < 1 )
	{
		valueOut = valueDefault;
		return false;
	}

	if( var.floats.size() >= (pos+1) )
	{
		valueOut	= var.floats[pos];
	}
	else
	{
		valueOut = valueDefault;
	}

	return true;
}

bool getValueMapDataWithDefault( const TianShanIce::ValueMap& value , const std::string& key ,const std::string& valueDefault , std::string& valueOut , size_t pos )
{
	TianShanIce::ValueMap::const_iterator it = value.find( key );
	if( it == value.end() )
	{
		valueOut = valueDefault;
		return false;
	}

	const TianShanIce::Variant& var = it->second;
	if( var.type != TianShanIce::vtStrings )
	{
		valueOut = valueDefault;
		return false;		
	}

	if( var.strs.size( ) < 1 )
	{
		valueOut = valueDefault;
		return false;
	}	

	if( var.strs.size() >= (pos+1) )
	{
		valueOut	= var.strs[pos];
	}
	else
	{
		valueOut = valueDefault;
	}

	return true;
}

void	getResourceData(  const TianShanIce::SRM::ResourceMap& value , const TianShanIce::SRM::ResourceType& type , const std::string& key , Ice::Byte& valueOut , size_t pos )
{
	TianShanIce::SRM::ResourceMap::const_iterator it = value.find( type );
	if( it == value.end() )
	{
		throw TianShanIce::InvalidParameter(); 
	}
	const TianShanIce::SRM::Resource& res = it->second;
	getValueMapData( res.resourceData , key , valueOut , pos );
}

void	getResourceData( const TianShanIce::SRM::ResourceMap& value , const TianShanIce::SRM::ResourceType& type , const std::string& key , std::vector<Ice::Byte>& valueOut ,bool& bRange )
{
	TianShanIce::SRM::ResourceMap::const_iterator it = value.find( type );
	if( it == value.end() )
	{
		throw TianShanIce::InvalidParameter(); 
	}
	const TianShanIce::SRM::Resource& res = it->second;
	getValueMapData( res.resourceData , key , valueOut ,bRange );
}

void	getResourceData(  const TianShanIce::SRM::ResourceMap& value , const TianShanIce::SRM::ResourceType& type , const std::string& key , Ice::Int& valueOut ,size_t pos )
{
	TianShanIce::SRM::ResourceMap::const_iterator it = value.find( type );
	if( it == value.end() )
	{
		throw TianShanIce::InvalidParameter(); 
	}
	const TianShanIce::SRM::Resource& res = it->second;
	getValueMapData( res.resourceData , key , valueOut , pos );
}

void	getResourceData( const TianShanIce::SRM::ResourceMap& value , const TianShanIce::SRM::ResourceType& type , const std::string& key , std::vector<Ice::Int>& valueOut ,bool& bRange )
{
	TianShanIce::SRM::ResourceMap::const_iterator it = value.find( type );
	if( it == value.end() )
	{
		throw TianShanIce::InvalidParameter(); 
	}
	const TianShanIce::SRM::Resource& res = it->second;
	getValueMapData( res.resourceData , key , valueOut ,bRange );
}

void	getResourceData(  const TianShanIce::SRM::ResourceMap& value , const TianShanIce::SRM::ResourceType& type , const std::string& key , Ice::Long& valueOut , size_t pos )
{
	TianShanIce::SRM::ResourceMap::const_iterator it = value.find( type );
	if( it == value.end() )
	{
		throw TianShanIce::InvalidParameter(); 
	}
	const TianShanIce::SRM::Resource& res = it->second;
	getValueMapData( res.resourceData , key , valueOut , pos );
}

void	getResourceData(  const TianShanIce::SRM::ResourceMap& value , const TianShanIce::SRM::ResourceType& type , const std::string& key , std::vector<Ice::Long>& valueOut ,bool& bRange )
{
	TianShanIce::SRM::ResourceMap::const_iterator it = value.find( type );
	if( it == value.end() )
	{
		throw TianShanIce::InvalidParameter(); 
	}
	const TianShanIce::SRM::Resource& res = it->second;
	getValueMapData( res.resourceData , key , valueOut ,bRange);
}

void	getResourceData( const TianShanIce::SRM::ResourceMap& value , const TianShanIce::SRM::ResourceType& type , const std::string& key , Ice::Float& valueOut , size_t pos )
{
	TianShanIce::SRM::ResourceMap::const_iterator it = value.find( type );
	if( it == value.end() )
	{
		throw TianShanIce::InvalidParameter(); 
	}
	const TianShanIce::SRM::Resource& res = it->second;
	getValueMapData( res.resourceData , key , valueOut ,pos );
}
void	getResourceData( const TianShanIce::SRM::ResourceMap& value , const TianShanIce::SRM::ResourceType& type , const std::string& key , std::vector<Ice::Float>& valueOut , bool& bRange )
{
	TianShanIce::SRM::ResourceMap::const_iterator it = value.find( type );
	if( it == value.end() )
	{
		throw TianShanIce::InvalidParameter(); 
	}
	const TianShanIce::SRM::Resource& res = it->second;
	getValueMapData( res.resourceData , key , valueOut, bRange );
}

void	getResourceData( const TianShanIce::SRM::ResourceMap& value , const TianShanIce::SRM::ResourceType& type , const std::string& key , std::string& valueOut ,size_t pos )
{
	TianShanIce::SRM::ResourceMap::const_iterator it = value.find( type );
	if( it == value.end() )
	{
		throw TianShanIce::InvalidParameter(); 
	}
	const TianShanIce::SRM::Resource& res = it->second;
	getValueMapData( res.resourceData , key , valueOut ,pos );
}
void	getResourceData( const TianShanIce::SRM::ResourceMap& value , const TianShanIce::SRM::ResourceType& type , const std::string& key , std::vector<std::string>& valueOut ,bool& bRange)
{
	TianShanIce::SRM::ResourceMap::const_iterator it = value.find( type );
	if( it == value.end() )
	{
		throw TianShanIce::InvalidParameter(); 
	}
	const TianShanIce::SRM::Resource& res = it->second;
	getValueMapData( res.resourceData , key , valueOut ,bRange );
}

void	getResourceDataWithDefault(  const TianShanIce::SRM::ResourceMap& value , const TianShanIce::SRM::ResourceType& type , const std::string& key , const Ice::Int& valueDefault ,  Ice::Int& valueOut ,size_t pos )
{
	TianShanIce::SRM::ResourceMap::const_iterator it = value.find( type );
	if( it == value.end()) {
		valueOut = valueDefault;
		return;
	}
	getValueMapDataWithDefault(it->second.resourceData,key,valueDefault,valueOut,pos);
}
void	getResourceDataWithDefault(  const TianShanIce::SRM::ResourceMap& value , const TianShanIce::SRM::ResourceType& type , const std::string& key , const Ice::Long& valueDefault ,  Ice::Long& valueOut ,size_t pos )
{
	TianShanIce::SRM::ResourceMap::const_iterator it = value.find( type );
	if( it == value.end()) {
		valueOut = valueDefault;
		return;
	}
	getValueMapDataWithDefault(it->second.resourceData,key,valueDefault,valueOut,pos);
}

void	getResourceDataWithDefault(  const TianShanIce::SRM::ResourceMap& value , const TianShanIce::SRM::ResourceType& type , const std::string& key , const Ice::Float& valueDefault ,  Ice::Float& valueOut ,size_t pos )
{
	TianShanIce::SRM::ResourceMap::const_iterator it = value.find( type );
	if( it == value.end()) {
		valueOut = valueDefault;
		return;
	}
	getValueMapDataWithDefault(it->second.resourceData,key,valueDefault,valueOut,pos);
}

void	getResourceDataWithDefault(  const TianShanIce::SRM::ResourceMap& value , const TianShanIce::SRM::ResourceType& type , const std::string& key , const std::string& valueDefault ,  std::string& valueOut ,size_t pos )
{
	TianShanIce::SRM::ResourceMap::const_iterator it = value.find( type );
	if( it == value.end()) {
		valueOut = valueDefault;
		return;
	}
	getValueMapDataWithDefault(it->second.resourceData,key,valueDefault,valueOut,pos);
}

void	dumpTianShanVariant( const TianShanIce::Variant& var , ZQ::common::Log& logger ,const std::string& hint)
{
	const char* pType = NULL;
	switch ( var.type )
	{
	case TianShanIce::vtInts:
		{
			pType = "vtInts";
			::TianShanIce::IValues::const_iterator itFirst = var.ints.begin();
			if( var.ints.size() >=2 )
				logger(ZQ::common::Log::L_DEBUG,"%s:<Variant>:%s:%s:[%d-%d]",hint.c_str(), pType , var.bRange ? "R" : "E" , (*itFirst) , *(itFirst+1) );
			else if( var.ints.size() == 1 )
				logger(ZQ::common::Log::L_DEBUG,"%s:<Variant>:%s:%s:[%d]",hint.c_str(),pType, var.bRange ? "R" :"E" , (*itFirst) );
			else
				logger(ZQ::common::Log::L_DEBUG,"%s:<Variant>:Invalid",hint.c_str());
		}
		break;
	case TianShanIce::vtLongs:
		{
			pType = "vtLongs";
			::TianShanIce::LValues::const_iterator itFirst = var.lints.begin();
			if( var.lints.size() >=2 )
				logger(ZQ::common::Log::L_DEBUG,"%s:<Variant>:%s:%s:[%lld-%lld]",hint.c_str(),pType , var.bRange ? "R" : "E" , (*itFirst) , *(itFirst+1) );
			else if( var.lints.size() == 1) 
				logger(ZQ::common::Log::L_DEBUG,"%s:<Variant>:%s:%s:[%lld]",hint.c_str(),pType, var.bRange ? "R" : "E" , (*itFirst) );
			else
				logger(ZQ::common::Log::L_DEBUG,"%s:<Variant>:Invalid",hint.c_str());
		}
		break;
	case TianShanIce::vtFloats:
		{
			pType = "vtFloats";
			::TianShanIce::FValues::const_iterator itFirst = var.floats.begin();
			if( var.floats.size() >=2 )
				logger(ZQ::common::Log::L_DEBUG,"%s:<Variant>:%s:%s:[%f-%f]",hint.c_str(),pType , var.bRange ? "R" : "E" ,(*itFirst) , *(itFirst+1) );
			else if( var.floats.size() == 1) 
				logger(ZQ::common::Log::L_DEBUG,"%s:<Variant>:%s:%s:[%f]",hint.c_str(),pType, var.bRange ? "R" : "E" , (*itFirst) );
			else
				logger(ZQ::common::Log::L_DEBUG,"%s:<Variant>:Invalid",hint.c_str());
		}
		break;
	case TianShanIce::vtStrings:
		{
			pType = "vtStrings";
			::TianShanIce::StrValues::const_iterator itFirst = var.strs.begin();
			if( var.strs.size() >=2 )
				logger(ZQ::common::Log::L_DEBUG,"%s:<Variant>:%s:%s:[%s-%s]",hint.c_str(),pType , var.bRange ? "R" : "E" , (*itFirst).c_str() , (*(itFirst+1)).c_str() );
			else if( var.strs.size() == 1) 
				logger(ZQ::common::Log::L_DEBUG,"%s:<Variant>:%s:%s:[%s]",hint.c_str(),pType, var.bRange ? "R" : "E" ,  (*itFirst).c_str() );
			else
				logger(ZQ::common::Log::L_DEBUG,"%s:<Variant>:Invalid",hint.c_str());

		}
		break;
	default:
		pType = "Unknown";
		logger(ZQ::common::Log::L_DEBUG,"Unknown variant");
		break;
	}

}

void	dumpTianShanVariant( const TianShanIce::Variant& var , char*& pBuffer , size_t& bufferSize  ,const std::string& hint)
{
	const char* pType = NULL;
	size_t pos = 0;
	switch ( var.type )
	{
	case TianShanIce::vtInts:
		{
			pType = "vtInts";
			::TianShanIce::IValues::const_iterator itFirst = var.ints.begin();
			if( var.ints.size() >=2 )
				pos = snprintf( pBuffer , bufferSize , "%s:<Variant>:%s:%s:[%d-%d]",hint.c_str(),pType , var.bRange ? "R" : "E" , (*itFirst) , *(itFirst+1) );
			else if( var.ints.size() == 1) 
				pos = snprintf( pBuffer , bufferSize , "%s:<Variant>:%s:%s:[%d]",hint.c_str(),pType, var.bRange ? "R" : "E" , (*itFirst) );
			else
				pos = snprintf( pBuffer , bufferSize , "%s:<Variant>:Invalid",hint.c_str());
		}
		break;
	case TianShanIce::vtLongs:
		{
			pType = "vtLongs";
			::TianShanIce::LValues::const_iterator itFirst = var.lints.begin();
			if( var.lints.size() >=2 )
				pos = snprintf( pBuffer , bufferSize , "%s:<Variant>:%s:%s:["FMT64"-"FMT64"]",hint.c_str(),pType , var.bRange ? "R" : "E" , (*itFirst) , *(itFirst+1) );
			else if( var.lints.size() == 1) 
				pos = snprintf( pBuffer , bufferSize , "%s:<Variant>:%s:%s:["FMT64"]",hint.c_str(),pType, var.bRange ? "R" : "E" , (*itFirst) );
			else
				pos = snprintf( pBuffer , bufferSize , "%s:<Variant>:Invalid",hint.c_str());
		}
		break;
	case TianShanIce::vtFloats:
		{
			pType = "vtFloats";
			::TianShanIce::FValues::const_iterator itFirst = var.floats.begin();
			if( var.floats.size() >=2 )
				pos = snprintf( pBuffer , bufferSize , "%s:<Variant>:%s:%s:[%f-%f]",hint.c_str(),pType , var.bRange ? "R" : "E" ,(*itFirst) , *(itFirst+1) );
			else if( var.floats.size() == 1) 
				pos = snprintf( pBuffer , bufferSize , "%s:<Variant>:%s:%s:[%f]",hint.c_str(),pType, var.bRange ? "R" : "E" , (*itFirst) );
			else
				pos = snprintf( pBuffer , bufferSize , "%s:<Variant>:Invalid",hint.c_str());
		}
		break;
	case TianShanIce::vtStrings:
		{
			pType = "vtStrings";
			::TianShanIce::StrValues::const_iterator itFirst = var.strs.begin();
			if( var.strs.size() >=2 )
				pos = snprintf( pBuffer , bufferSize , "%s:<Variant>:%s:%s:[%s-%s]",hint.c_str(),pType , var.bRange ? "R" : "E" , (*itFirst).c_str() , (*(itFirst+1)).c_str() );
			else if( var.strs.size() == 1) 
				pos = snprintf( pBuffer , bufferSize , "%s:<Variant>:%s:%s:[%s]",hint.c_str(),pType, var.bRange ? "R" : "E" ,  (*itFirst).c_str() );
			else
				pos = snprintf( pBuffer , bufferSize , "%s:<Variant>:Invalid",hint.c_str());

		}
		break;
	default:
		pType = "Unknown";
		pos = snprintf( pBuffer , bufferSize , "Unknown variant");
		break;
	}
	bufferSize	-= pos;
	pBuffer		+= pos;
}

void	dumpTianShanValueMap( const TianShanIce::ValueMap& value , ZQ::common::Log& logger ,const std::string& hint )
{
	char	localBuffer[2048];	
	size_t	bufferSize = sizeof(localBuffer);
	localBuffer[ bufferSize-1 ] = 0;
	char* pBuffer = localBuffer;
	TianShanIce::ValueMap::const_iterator it = value.begin();
	for( ; it != value.end() ; it ++ )
	{
		int pos = snprintf( pBuffer , bufferSize , "<Key>[%s] " , it->first.c_str() );
		bufferSize -= pos;
		pBuffer += pos;
		dumpTianShanVariant( it->second , pBuffer , bufferSize , "");
		logger( ZQ::common::Log::L_DEBUG, "%s%s", hint.c_str() , localBuffer);
	}
}
void	dumpTianShanValueMap( const TianShanIce::ValueMap& value , char*& pBuffer , size_t& bufferSize , const std::string& hint )
{
	TianShanIce::ValueMap::const_iterator it = value.begin();
	int pos = 0;
	for( ; it != value.end() ; it ++ )
	{
		pos = snprintf( pBuffer , bufferSize , "<Key>[%s] " , it->first.c_str() );
		bufferSize -= pos;
		pBuffer += pos;
		dumpTianShanVariant( it->second , pBuffer , bufferSize , "" );
	
		TianShanIce::ValueMap::const_iterator itNext = it;
		if(  ++itNext  != value.end() )
		{
			pos = snprintf( pBuffer , bufferSize , " || " );
			pBuffer += pos;
			bufferSize -= pos;
		}
	}
}

void	dumpTianShanResource( const TianShanIce::SRM::Resource& res , ZQ::common::Log& logger  ,const std::string& hint )
{
	char	localBuffer[2048];	
	size_t	bufferSize = sizeof(localBuffer);
	localBuffer[ bufferSize-1 ] = 0;

	const TianShanIce::ValueMap& value = res.resourceData;
	dumpTianShanValueMap(value,logger,hint);
}

void	dumpTianShanResource( const TianShanIce::SRM::Resource& res , char* pBuffer , size_t& bufferSize ,const std::string& hint )
{
	dumpTianShanValueMap( res.resourceData , pBuffer , bufferSize , hint );
}

void	dumpTianShanResourceMap( const TianShanIce::SRM::ResourceMap& resMap , ZQ::common::Log& logger ,const std::string& hint )
{
	char	localBuffer[2048];	
	size_t	bufferSize = sizeof(localBuffer);
	localBuffer[ bufferSize-1 ] = 0;
	TianShanIce::SRM::ResourceMap::const_iterator it = resMap.begin();
	for( ; it != resMap.end() ; it ++ )
	{	
		int pos = snprintf( localBuffer , bufferSize , "<Key>[%d] " , it->first );
		bufferSize -= pos;
		dumpTianShanResource( it->second , &localBuffer[pos] , bufferSize ,"" );
		logger( ZQ::common::Log::L_DEBUG, "%s%s", hint.c_str() , localBuffer);
	}
}
void mergeProperty( TianShanIce::Properties& propsA , const TianShanIce::Properties& propsB, bool bForce )
{
	TianShanIce::Properties::const_iterator it = propsB.begin();
	for ( ; it != propsB.end() ; it ++ )
	{
		if( !bForce )
		{
			if( propsA.find(it->first) != propsA.end())
			{
				continue;
			}
		}
		propsA[it->first]	=	it->second;
	}
}
void mergeValueMap( TianShanIce::ValueMap& mapA , const TianShanIce::ValueMap& mapB, bool bForce )
{
	TianShanIce::ValueMap::const_iterator itMap = mapB.begin();
	for( ; itMap != mapB.end() ; itMap ++ )
	{
		if( !bForce )
		{
			if ( mapA.find(itMap->first) != mapA.end() )
			{
				continue;
			}
		}		
		mapA[itMap->first] =  itMap->second;		
	}
}

void mergeResourceMap( TianShanIce::SRM::ResourceMap& mapA, const TianShanIce::SRM::ResourceMap& mapB , bool bForece )
{
	TianShanIce::SRM::ResourceMap::const_iterator itMap = mapB.begin();
	for ( ; itMap != mapB.end() ; itMap ++ )
	{
		if(!bForece)
		{
			if( mapA.find( itMap->first ) != mapA.end( ) )
			{
				continue;
			}
		}
		mapA[itMap->first] = itMap->second;
	}
}


const char* convertStreamStateToString( const TianShanIce::Streamer::StreamState& state)
{
	switch (state)
	{
	case TianShanIce::Streamer::stsSetup:
		return "stsSetup";
		break;
	case TianShanIce::Streamer::stsPause:
		return "stsPause";
		break;
	case TianShanIce::Streamer::stsStreaming:
		return "Streaming";
		break;
	case TianShanIce::Streamer::stsStop:
		return "stop";
		break;
	default:
		return "unknown";
		break;
	}
	return "";
}

std::string convertItemFlagToStr( Ice::Long flags )
{
	std::string strRet;
	strRet.reserve( 256 );
	//check the flags	
	if ( flags & TianShanIce::Streamer::PLISFlagNoPause ) 
	{
		strRet += "NoPause ";
	}
	if ( flags & TianShanIce::Streamer::PLISFlagNoFF ) 
	{
		strRet += "NoFF ";
	}
	if ( flags & TianShanIce::Streamer::PLISFlagNoRew ) 
	{
		strRet += "NoRew ";
	}
	if ( flags & TianShanIce::Streamer::PLISFlagNoSeek ) 
	{
		strRet += "NoSeek ";
	}
	if ( flags & TianShanIce::Streamer::PLISFlagSkipAtFF )
	{
		strRet += "SkipAtFF ";
	}
	if ( flags & TianShanIce::Streamer::PLISFlagSkipAtRew )
	{
		strRet += "SkipAtRew ";
	}

	return strRet;
}

Ice::Float	getSpeedFromStreamInfo( const TianShanIce::Streamer::StreamInfo& info )
{
	TianShanIce::Properties::const_iterator it = info.props.find("SPEED");
	if( it == info.props.end() )
	{
		throw TianShanIce::InvalidParameter(); 
		return 0.0f; //make complier happy
	}
	else
	{
		Ice::Float f;
		sscanf( it->second.c_str() , "%f" , &f );
		return f;
	}
}

Ice::Int	getUserCtrlNumFromStreamInfo(  const TianShanIce::Streamer::StreamInfo& info )
{
	TianShanIce::Properties::const_iterator it = info.props.find("USERCTRLNUM");
	if( it == info.props.end() )
	{
		throw TianShanIce::InvalidParameter(); 
		return 0; //make complier happy
	}
	else
	{
		Ice::Int d;
		sscanf( it->second.c_str() , "%d" , &d );
		return d;
	}
}

Ice::Long	getItemTimeOffset( const TianShanIce::Streamer::StreamInfo& info )
{
	TianShanIce::Properties::const_iterator it = info.props.find("ITEM_CURRENTPOS");
	if( it == info.props.end() )
	{
		throw TianShanIce::InvalidParameter(); 
		return 0; //make complier happy
	}
	else
	{
		Ice::Long d;
		sscanf( it->second.c_str() , FMT64 , &d );
		return d;
	}
}

Ice::Long	getItemTotalDuration( const TianShanIce::Streamer::StreamInfo& info )
{
	TianShanIce::Properties::const_iterator it = info.props.find("ITEM_TOTALPOS");
	if( it == info.props.end() )
	{
		throw TianShanIce::InvalidParameter(); 
		return 0; //make complier happy
	}
	else
	{
		Ice::Long d;
		sscanf( it->second.c_str() , FMT64 , &d );
		return d;
	}
}

Ice::Long	getStreamTimeOffset( const TianShanIce::Streamer::StreamInfo& info )
{
	TianShanIce::Properties::const_iterator it = info.props.find("CURRENTPOS");
	if( it == info.props.end() )
	{
		throw TianShanIce::InvalidParameter(); 
		return 0; //make complier happy
	}
	else
	{
		Ice::Long d;
		sscanf( it->second.c_str() , FMT64 , &d );
		return d;
	}
}

Ice::Long	getStreamTotalDuration( const TianShanIce::Streamer::StreamInfo& info )
{
	TianShanIce::Properties::const_iterator it = info.props.find("TOTALPOS");
	if( it == info.props.end() )
	{
		throw TianShanIce::InvalidParameter(); 
		return 0; //make complier happy
	}
	else
	{
		Ice::Long d;
		sscanf( it->second.c_str() , FMT64 , &d );
		return d;
	}
}

void		updateSpeedToStreamInfo( TianShanIce::Streamer::StreamInfo& info , const Ice::Float& fSpeed )
{
	char szBuffer[32];
	sprintf(szBuffer,"%f",fSpeed );
	info.props["SPEED"] = szBuffer;
}
void		updateUserCtrlNumToStreamInfo( TianShanIce::Streamer::StreamInfo& info , const Ice::Int& userCtrlNum )
{
	char szBuffer[32];
	sprintf(szBuffer,"%d",userCtrlNum );
	info.props["USERCTRLNUM"] = szBuffer;
}
void		updateItemTimeOffsetToStreamInfo( TianShanIce::Streamer::StreamInfo& info , const Ice::Long& timeOffset )
{
	char szBuffer[32];
	sprintf(szBuffer,FMT64,timeOffset );
	info.props["ITEM_CURRENTPOS"] = szBuffer;
}
void		updateItemTotalDurationToStreamInfo(  TianShanIce::Streamer::StreamInfo& info , const Ice::Long& timeOffset  )
{
	char szBuffer[32];
	sprintf(szBuffer,FMT64,timeOffset );
	info.props["ITEM_TOTALPOS"] = szBuffer;
}
void		updateStreamTimeOffsetToStreamInfo(  TianShanIce::Streamer::StreamInfo& info , const Ice::Long& timeOffset )
{
	char szBuffer[32];
	sprintf(szBuffer,FMT64,timeOffset );
	info.props["CURRENTPOS"] = szBuffer;
}
void		updateStreamTotalDurationToStreamInfo(  TianShanIce::Streamer::StreamInfo& info , const Ice::Long& timeOffset )
{
	char szBuffer[32];
	sprintf(szBuffer,FMT64,timeOffset );
	info.props["TOTALPOS"] = szBuffer;
}

const char*	dumpStreamInfo( const TianShanIce::Streamer::StreamInfo& info , char* buffer ,size_t bufSize )
{
	assert( buffer != NULL );
	assert( bufSize > 0 );
	int		iPos = 0;
	int		iSize = static_cast<int>(bufSize);
	
	buffer[ bufSize-1 ] = 0;


	char* pTemp = buffer;
	const TianShanIce::Properties& props = info.props;
	TianShanIce::Properties::const_iterator it = props.begin();
	for( ; it != props.end() ; it ++ )
	{
		iPos = snprintf( pTemp  , iSize , " [%s]:[%s] " , it->first.c_str( ),it->second.c_str() );
		iSize -= iPos;
		pTemp += iPos;
		if( iSize <= 0 )
			break;
	}
	return buffer;
}

#ifdef ZQ_OS_MSWIN
#define		PATHDELIMETER '\\'

void		replaceCharacter( int iLen , char* p , const char* pSrc , int& iPos, bool& bLastSlash )
{
	for ( int i = 0 ;i < iLen ; i ++ )
	{
		if( pSrc[i] == '/' )
		{
			if( bLastSlash	)
			{
				continue;			
			}
			else
			{
				p[iPos++] = '\\';
			}
			bLastSlash = true;
		}
		else if ( bLastSlash && pSrc[i] == '\\'  )
		{
			continue;
		}
		else
		{
			bLastSlash = pSrc[i] == '\\';
			p[iPos++]=pSrc[i];
		}
	}
}

bool				fsCreatePath( const std::string& strPath )
{
	if( !CreateDirectoryA(fsFixupPath(strPath).c_str(),NULL) )
	{
		if ( ERROR_ALREADY_EXISTS == GetLastError() )
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	return true;
}
bool				fsCreateFile( const std::string& strFile )
{
	return false;
}

#endif

#ifdef ZQ_OS_LINUX
#define		PATHDELIMETER '/'
void		replaceCharacter( int iLen , char* p , const char* pSrc , int& iPos, bool& bLastSlash )
{
	for ( int i = 0 ;i < iLen ; i ++ )
	{
		if( pSrc[i] == '\\' )
		{
			if( bLastSlash	)
			{
				continue;			
			}
			else
			{
				p[iPos++] = '/';
			}
			bLastSlash = true;
		}
		else if ( bLastSlash && pSrc[i] == '/'  )
		{
			continue;
		}
		else
		{
			bLastSlash = pSrc[i] == '/';
			p[iPos++]=pSrc[i];
		}
	}
}
bool				fsCreatePath( const std::string& strPath )
{
	bool isSuccess = true;
	if(!(0 == mkdir(fsFixupPath(strPath).c_str(), 0755))
		&& EEXIST != errno)
		isSuccess = false;

	return isSuccess;
}
bool				fsCreateFile( const std::string& strFile )
{
	return false;
}
#endif

std::string			fsFixupPath( const std::string& strPath )
{
	int iLen = static_cast<int>(strPath.length());
	const char* pSrc = strPath.c_str();
	char* p = new char[iLen + 1 ];
	p[iLen] = 0;
	assert( p != NULL );
	bool bLastSlash = false;
	int iPos = 0;
	replaceCharacter(iLen,p,pSrc,iPos, bLastSlash);
	p[iPos] = 0;
	std::string result(p);
	delete[] p;
	return result;
}
std::string			fsConcatPath( const std::string& parentPath , const std::string& subPath )
{
	int iLen = static_cast<int>(parentPath.length());
	const char* pSrc = parentPath.c_str();
	char* p = new char[parentPath.length() + subPath.length() + 3 ];
	p[iLen] = 0;
	assert( p != NULL );
	bool bLastSlash = false;
	int iPos = 0;
	replaceCharacter( iLen , p , pSrc , iPos , bLastSlash );
	pSrc = subPath.c_str() ;
	iLen = static_cast<int>(subPath.length());
	if( pSrc[0] != '/' || pSrc[0] != '\\')
	{
		if( !bLastSlash )
			p[iPos++] = PATHDELIMETER;
		bLastSlash = true;
	}
	replaceCharacter( iLen, p, pSrc, iPos ,bLastSlash );
	p[iPos] = 0;
	std::string result(p);
	delete[] p;
	return result;
}

std::string			fsGetParentFolderPath( const std::string& strPath )
{
	std::string pathString = fsFixupPath(strPath);
	std::string::size_type posSlash = pathString.find_last_of(PATHDELIMETER);
	if( posSlash != std::string::npos )
	{
		if( posSlash == (pathString.length() -1) )
		{
			posSlash = pathString.rfind( PATHDELIMETER , posSlash - 1);
			if( posSlash == std::string::npos )
				return pathString;
		}
		pathString = pathString.substr( 0 ,posSlash );
	}
	return pathString;
}

std::string			getNowOfUTC( )
{
	time_t	now;
	now = time(&now);
	char nowStr[256];
	ZQ::common::TimeUtil::Time2Iso(now,nowStr,sizeof(nowStr));
	return std::string(nowStr);
}
std::string		dumpPlaylistItemSetupInfo( const TianShanIce::Streamer::PlaylistItemSetupInfo& info)
{
	std::ostringstream oss;
	oss<<"NAME["<<info.contentName<<"] InOffset["<<info.inTimeOffset<<"] OutOffset["<<
		info.outTimeOffset<<"] spliceIn["<<info.spliceIn <<"] spliceOut["<<info.spliceOut<<"]";
	return oss.str();
}

std::string		convertCriticalStartToStr( const Ice::Long&	tt )
{
	char UTCTimeBuffer[256];
	memset(UTCTimeBuffer, '\0', sizeof(UTCTimeBuffer));
	time_t t = static_cast<time_t>(tt);
	tm* tLocal = localtime (&t) ;
	strftime(UTCTimeBuffer,sizeof(UTCTimeBuffer)-1,"%Y:%m:%d-%H:%M:%S",tLocal);
	return std::string(UTCTimeBuffer);
}



/////////////////////////////
void suicide()
{
#ifdef ZQ_OS_MSWIN
	RaiseException(EXCEPTION_ACCESS_VIOLATION, 0, 0, NULL);
#else

#endif
}


//////////////////////////////////////////////////////////////////////////


BufferMarshal::BufferMarshal( void* data , size_t size )
:mMemory(NULL),
mMemSize(0),
mDataSize(0),
mbNoMemory(false),
mbNeedFree(false)
{
	if( data)
	{
		attachBuffer(data,size);
	}
	else
	{
		incMemory(256);
	}
}
BufferMarshal::~BufferMarshal()
{
	clear();
}
void BufferMarshal::clear()
{
	if( mMemory && mbNeedFree )
	{
		free(mMemory);
		mMemory = NULL;
	}
}
void* BufferMarshal::getBuffer( size_t& size)
{
	size = mDataSize;
	return mMemory;
}

void BufferMarshal::attachBuffer( void* buffer , size_t size )
{
	clear();
	mbNeedFree = false;
	mbNoMemory = false;
	mMemory = buffer;
	mMemSize = size;
	mDataSize = 0;
}

inline void throwBadDataException()
{
	throw "bad data exception";
}

BufferMarshal&	BufferMarshal::operator<<( int8 value )
{
	setDataType( MARSHAL_TYPE_INTEGER , sizeof( value ) );
	putData( &value , sizeof( value ) );
	return *this;
}
BufferMarshal&	BufferMarshal::operator<<( uint8 value )
{
	setDataType( MARSHAL_TYPE_INTEGER , sizeof( value ) );
	putData( &value , sizeof( value ) );
	return *this;
}
BufferMarshal&	BufferMarshal::operator<<( int16 value )
{
	setDataType( MARSHAL_TYPE_INTEGER , sizeof( value ) );
	putData( &value , sizeof( value ) );
	return *this;
}
BufferMarshal&	BufferMarshal::operator<<( uint16 value )
{
	setDataType( MARSHAL_TYPE_INTEGER , sizeof( value ) );
	putData( &value , sizeof( value ) );
	return *this;
}
BufferMarshal&	BufferMarshal::operator<<( int32 value )
{
	setDataType( MARSHAL_TYPE_INTEGER , sizeof( value ) );
	putData( &value , sizeof( value ) );
	return *this;
}
BufferMarshal&	BufferMarshal::operator<<( uint32 value )
{
	setDataType( MARSHAL_TYPE_INTEGER , sizeof( value ) );
	putData( &value , sizeof( value ) );
	return *this;
}
BufferMarshal&	BufferMarshal::operator<<( int64 value )
{
	setDataType( MARSHAL_TYPE_INTEGER , sizeof( value ) );
	putData( &value , sizeof( value ) );
	return *this;
}
BufferMarshal&	BufferMarshal::operator<<( uint64 value )
{
	setDataType( MARSHAL_TYPE_INTEGER , sizeof( value ) );
	putData( &value , sizeof( value ) );
	return *this;
}
BufferMarshal&	BufferMarshal::operator<<( float value )
{
	setDataType( MARSHAL_TYPE_INTEGER , sizeof( value ) );
	putData( &value , sizeof( value ) );
	return *this;
}
BufferMarshal&	BufferMarshal::operator<<( double value )
{
	setDataType( MARSHAL_TYPE_INTEGER , sizeof( value ) );
	putData( &value , sizeof( value ) );
	return *this;
}
BufferMarshal&	BufferMarshal::operator<<( const std::string& value )
{
	setDataType( MARSHAL_TYPE_STRING , value.length() );
	putData( value.c_str() , value.length() );
	return *this;
}
BufferMarshal&	BufferMarshal::operator<<( const char* value )
{
	size_t len = 0;
	if(value) len = strlen(value);
	setDataType( MARSHAL_TYPE_STRING , len );
	putData( value , len );
	return *this;
}

#define CHECKDATATYPE(x) { BufferMarshalDataType type = MARSHAL_TYPE_INTEGER; \
							if(!getDataType(type,size) || type != x )		throwBadDataException(); \
							if(!getData( (const void*&)data,size)) throwBadDataException();}
BufferMarshal&	BufferMarshal::operator>>( int8& value )
{
	size_t size = 0 ;
	const void* data = NULL;
	CHECKDATATYPE(MARSHAL_TYPE_INTEGER)

	value = *(int8*)data;

	return *this;
}
BufferMarshal&	BufferMarshal::operator>>( uint8& value )
{
	size_t size = 0 ;
	const void* data = NULL;
	CHECKDATATYPE(MARSHAL_TYPE_INTEGER)

	value = *(uint8*)data;

	return *this;
}
BufferMarshal&	BufferMarshal::operator>>( int16& value )
{
	size_t size = 0 ;
	const void* data = NULL;
	CHECKDATATYPE(MARSHAL_TYPE_INTEGER)

	value = *(int16*)data;

	return *this;
}
BufferMarshal&	BufferMarshal::operator>>( uint16& value )
{
	size_t size = 0 ;
	const void* data = NULL;
	CHECKDATATYPE(MARSHAL_TYPE_INTEGER)

	value = *(uint16*)data;

	return *this;
}
BufferMarshal&	BufferMarshal::operator>>( int32& value )
{
	size_t size = 0 ;
	const void* data = NULL;
	CHECKDATATYPE(MARSHAL_TYPE_INTEGER)

	value = *(int32*)data;

	return *this;
}
BufferMarshal&	BufferMarshal::operator>>( uint32& value )
{
	size_t size = 0 ;
	const void* data = NULL;
	CHECKDATATYPE(MARSHAL_TYPE_INTEGER)

	value = *(uint32*)data;

	return *this;
}
BufferMarshal&	BufferMarshal::operator>>( int64& value )
{
	size_t size = 0 ;
	const void* data = NULL;
	CHECKDATATYPE(MARSHAL_TYPE_INTEGER)

	value = *(int64*)data;

	return *this;
}
BufferMarshal&	BufferMarshal::operator>>( uint64& value )
{
	size_t size = 0 ;
	const void* data = NULL;
	CHECKDATATYPE(MARSHAL_TYPE_INTEGER)

	value = *(uint64*)data;

	return *this;
}
BufferMarshal&	BufferMarshal::operator>>( float& value )
{
	size_t size = 0 ;
	const void* data = NULL;
	CHECKDATATYPE(MARSHAL_TYPE_INTEGER)

	value = *(float*)data;

	return *this;
}
BufferMarshal&	BufferMarshal::operator>>( double& value )
{
	size_t size = 0 ;
	const void* data = NULL;
	CHECKDATATYPE(MARSHAL_TYPE_INTEGER)

	value = *(double*)data;

	return *this;
}
BufferMarshal&	BufferMarshal::operator>>( std::string& value )
{
	size_t size = 0 ;
	const void* data = NULL;
	CHECKDATATYPE(MARSHAL_TYPE_STRING)

	value.assign((char*)data,size);

	return *this;
}
void BufferMarshal::incMemory( size_t size )
{
	size_t newSize = size;
	if( newSize == 0 )
	{
		assert( mMemSize != 0 );
		newSize = mMemSize * 2;
	}
	char* pNewMem =  (char*)malloc(newSize);
	if(!pNewMem)
	{
		mbNoMemory = true;
		return;
	}
	mbNoMemory = false;
	mbNeedFree = true;
	memcpy(pNewMem,mMemory,mDataSize);
	free(mMemory);
	mMemSize = newSize;
	mMemory = pNewMem;
}

void BufferMarshal::setDataType(const BufferMarshalDataType& type, size_t size )
{
	uint16 flag = ((uint16)type)<<14;
	size = size & 16383;
	flag |= size;
	putData( (const char*)&flag , sizeof(flag) );
}

bool BufferMarshal::getDataType( BufferMarshalDataType& type , size_t& size)
{
	uint16 flag = 0;
	const void* pFlag = NULL;
	if( !getData( pFlag , sizeof(flag)) )
	{
		throwBadDataException();
	}
	flag = *(uint16*)pFlag;
	type = (BufferMarshalDataType)(flag>>14);
	size = flag & 16383;
	return true;
}

bool BufferMarshal::getData( const void*& data , size_t size)
{
	if( (mDataSize + size) > mMemSize )
		return false;
	data = (const void*)((char*)mMemory + mDataSize);
	mDataSize += size;	
	return true;
}

void BufferMarshal::putData( const void* data , size_t size )
{
	if( (mDataSize + size) > mMemSize )
	{
		incMemory();		
	}
	if( mbNoMemory )	return;
	memcpy((char*)mMemory+mDataSize,data,size);
	mDataSize += size;
}



}}//namespace ZQ::StreamSmith
