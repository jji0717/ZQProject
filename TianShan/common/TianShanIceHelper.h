
#ifndef	_ZQ_TianShan_UTILITY_HEADER_FILE_H__
#define _ZQ_TianShan_UTILITY_HEADER_FILE_H__

#include "ZQ_common_conf.h"
#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>

#include <TsStreamer.h>
#include <TsSRM.h>

#include <vector>
#include <list>
#include <set>

#include "Log.h"
#include <TimeUtil.h>


#define		VALUE_WANTED_ITEM_NPT			( 1 << 0 )
#define		VALUE_WANTED_ITEM_DURATION		( 1 << 1 )
#define		VALUE_WANTED_STREAM_NPT			( 1 << 2 )
#define		VALUE_WANTED_STREAM_DURATION	( 1 << 3 )
#define		VALUE_WANTED_STREAM_SPEED		( 1 << 4 )
#define		VALUE_WANTED_STREAM_STATE		( 1 << 5 )
#define		VALUE_WANTED_USER_CTRLNUM		( 1 << 6 )



namespace ZQTianShan 
{
namespace Util
{

void			updateStreamInfoValue( TianShanIce::StrValues& value , const Ice::Int mask );

void			getStreamInfoValue( Ice::Int& wanted , const TianShanIce::StrValues& value );

std::string		dumpStreamInfoValues( const TianShanIce::StrValues& value ,const std::string& delimiter = " " );

const char*		dumpTianShanStreamState( const TianShanIce::Streamer::StreamState& state );

const char*		resourceTypeToString( const TianShanIce::SRM::ResourceType& type );

///retrieve data from property
void			getPropertyData( const TianShanIce::Properties& props , const std::string& key , Ice::Int& valueOut ) throw (::TianShanIce::InvalidParameter);
void			getPropertyData( const TianShanIce::Properties& props , const std::string& key , Ice::Float& valueOut ) throw (::TianShanIce::InvalidParameter);
void			getPropertyData( const TianShanIce::Properties& props , const std::string& key , Ice::Long& valueOut ) throw (::TianShanIce::InvalidParameter);
void			getPropertyData( const TianShanIce::Properties& props , const std::string& key , std::string& valueOut ) throw (::TianShanIce::InvalidParameter);

void			getPropertyDataWithDefault( const TianShanIce::Properties& props , const std::string& key , const Ice::Int& defaultValue, Ice::Int& valueOut );
void			getPropertyDataWithDefault( const TianShanIce::Properties& props , const std::string& key , const Ice::Float& defaultValue, Ice::Float& valueOut );
void			getPropertyDataWithDefault( const TianShanIce::Properties& props , const std::string& key , const Ice::Long& defaultValue, Ice::Long& valueOut );
void			getPropertyDataWithDefault( const TianShanIce::Properties& props , const std::string& key , const std::string defaultValue, std::string& valueOut );

template<class T> void	updatePropertyData( TianShanIce::Properties& props , const std::string& key , const T& value )
{
	if(key.empty()) return;
	std::ostringstream oss;
	oss<<std::fixed;
	oss<<value;
	props[key] = oss.str();
}
// void			updatePropertyData( TianShanIce::Properties& props , const std::string& key , const Ice::Float& value );
// void			updatePropertyData( TianShanIce::Properties& props , const std::string& key , const Ice::Long& value );
// void			updatePropertyData( TianShanIce::Properties& props , const std::string& key , const std::string& value );

///retrieve data from ValueMap
void			getValueMapData( const TianShanIce::ValueMap& value , const std::string& key , Ice::Byte& valueOut, size_t pos = 0 );
void			getValueMapData( const TianShanIce::ValueMap& value , const std::string& key , std::vector<Ice::Byte>& valueOut ,bool& bRange );

//@return false if not found and default value is taken
bool			getValueMapDataWithDefault( const TianShanIce::ValueMap& value , const std::string& key , const Ice::Byte& valueDefault , Ice::Byte& valueOut , size_t pos = 0);

void			updateValueMapData( TianShanIce::ValueMap& value , const std::string& key , const Ice::Byte& valueIn );
void			updateValueMapData( TianShanIce::ValueMap& value , const std::string& key , const  std::vector<Ice::Byte>& valueIn , bool bRange );

void			getValueMapData( const TianShanIce::ValueMap& value , const std::string& key , Ice::Int& valueOut, size_t pos = 0 );
void			getValueMapData( const TianShanIce::ValueMap& value , const std::string& key , std::vector<Ice::Int>& valueOut ,bool& bRange );

//@return false if not found and default value is taken
bool			getValueMapDataWithDefault( const TianShanIce::ValueMap& value , const std::string& key , const Ice::Int& valueDefault , Ice::Int& valueOut , size_t pos = 0 );

void			updateValueMapData( TianShanIce::ValueMap& value , const std::string& key , const Ice::Int& valueIn );
void			updateValueMapData( TianShanIce::ValueMap& value , const std::string& key , const  std::vector<Ice::Int>& valueIn , bool bRange );

void			getValueMapData( const TianShanIce::ValueMap& value , const std::string& key , Ice::Long& valueOut, size_t pos = 0 );
void			getValueMapData( const TianShanIce::ValueMap& value , const std::string& key , std::vector<Ice::Long>& valueOut ,bool& bRange );

//@return false if not found and default value is taken
bool			getValueMapDataWithDefault( const TianShanIce::ValueMap& value , const std::string& key , const Ice::Long& valueDefault , Ice::Long& valueOut , size_t pos = 0);

void			updateValueMapData( TianShanIce::ValueMap& value , const std::string& key , const  Ice::Long& valueIn );
void			updateValueMapData( TianShanIce::ValueMap& value , const std::string& key , const  std::vector<Ice::Long>& valueIn , bool bRange );

void			getValueMapData( const TianShanIce::ValueMap& value , const std::string& key , Ice::Float& valueOut , size_t pos = 0);
void			getValueMapData( const TianShanIce::ValueMap& value , const std::string& key , std::vector<Ice::Float>& valueOut ,bool& bRange );

//@return false if not found and default value is taken
bool			getValueMapDataWithDefault( const TianShanIce::ValueMap& value , const std::string& key , const Ice::Float& valueDefault , Ice::Float& valueOut , size_t pos = 0 );

void			updateValueMapData( TianShanIce::ValueMap& value , const std::string& key , const  Ice::Float& valueIn );
void			updateValueMapData( TianShanIce::ValueMap& value , const std::string& key , const  std::vector<Ice::Float>& valueIn , bool bRange );

void			getValueMapData( const TianShanIce::ValueMap& value , const std::string& key , std::string& valueOut, size_t pos = 0 );
void			getValueMapData( const TianShanIce::ValueMap& value , const std::string& key , std::vector<std::string>& valueOut ,bool& bRange );

//@return false if not found and default value is taken
bool			getValueMapDataWithDefault( const TianShanIce::ValueMap& value , const std::string& key ,const std::string& valueDefault , std::string& valueOut , size_t pos = 0 );

void			updateValueMapData( TianShanIce::ValueMap& value , const std::string& key , const  std::string& valueIn );
void			updateValueMapData( TianShanIce::ValueMap& value , const std::string& key , const  std::vector<std::string>& valueIn , bool bRange );


TianShanIce::Properties getValueMapAsProperties( const  TianShanIce::ValueMap& vm );

///retrieve data from TianShanIce resource
void			getResourceData( const TianShanIce::SRM::ResourceMap& value , const TianShanIce::SRM::ResourceType& type , const std::string& key , Ice::Byte& valueOut , size_t pos = 0 );
void			getResourceData( const TianShanIce::SRM::ResourceMap& value , const TianShanIce::SRM::ResourceType& type , const std::string& key , std::vector<Ice::Byte>& valueOut ,bool & bRange );
void			getResourceData( const TianShanIce::SRM::ResourceMap& value , const TianShanIce::SRM::ResourceType& type , const std::string& key , Ice::Int& valueOut , size_t pos = 0 );
void			getResourceData( const TianShanIce::SRM::ResourceMap& value , const TianShanIce::SRM::ResourceType& type , const std::string& key , std::vector<Ice::Int>& valueOut ,bool & bRange);
void			getResourceDataWithDefault(  const TianShanIce::SRM::ResourceMap& value , const TianShanIce::SRM::ResourceType& type , const std::string& key , const Ice::Int& valueDefault ,  Ice::Int& valueOut , size_t pos = 0);

void			getResourceData( const TianShanIce::SRM::ResourceMap& value , const TianShanIce::SRM::ResourceType& type , const std::string& key , Ice::Long& valueOut ,size_t pos = 0);
void			getResourceData( const TianShanIce::SRM::ResourceMap& value , const TianShanIce::SRM::ResourceType& type , const std::string& key , std::vector<Ice::Long>& valueOut ,bool& bRange );
void			getResourceDataWithDefault(  const TianShanIce::SRM::ResourceMap& value , const TianShanIce::SRM::ResourceType& type , const std::string& key , const Ice::Long& valueDefault ,  Ice::Long& valueOut , size_t pos = 0 );

void			getResourceData( const TianShanIce::SRM::ResourceMap& value , const TianShanIce::SRM::ResourceType& type , const std::string& key , Ice::Float& valueOut , size_t pos = 0 );
void			getResourceData( const TianShanIce::SRM::ResourceMap& value , const TianShanIce::SRM::ResourceType& type , const std::string& key , std::vector<Ice::Float>& valueOut , bool& bRange);
void			getResourceDataWithDefault(  const TianShanIce::SRM::ResourceMap& value , const TianShanIce::SRM::ResourceType& type , const std::string& key , const Ice::Float& valueDefault ,  Ice::Float& valueOut , size_t pos = 0 );

void			getResourceData( const TianShanIce::SRM::ResourceMap& value , const TianShanIce::SRM::ResourceType& type , const std::string& key , std::string& valueOut , size_t pos = 0 );
void			getResourceData( const TianShanIce::SRM::ResourceMap& value , const TianShanIce::SRM::ResourceType& type , const std::string& key , std::vector<std::string>& valueOut ,bool bRange);
void			getResourceDataWithDefault(  const TianShanIce::SRM::ResourceMap& value , const TianShanIce::SRM::ResourceType& type , const std::string& key , const std::string& valueDefault ,  std::string& valueOut , size_t pos = 0 );

void			mergeValueMap( TianShanIce::ValueMap& mapA , const TianShanIce::ValueMap& mapB, bool bForce = true );

template<typename T> void updateResourceData( TianShanIce::SRM::ResourceMap& resMap , const TianShanIce::SRM::ResourceType& type, const std::string& key , const T& valueIn )
{
	TianShanIce::SRM::Resource res;
	res.status	=	TianShanIce::SRM::rsRequested;
	res.attr	=	TianShanIce::SRM::raMandatoryNonNegotiable;

	updateValueMapData( res.resourceData , key ,  valueIn  );

	TianShanIce::SRM::ResourceMap::iterator it = resMap.find( type );
	if( it == resMap.end() )
	{
		resMap.insert( TianShanIce::SRM::ResourceMap::value_type( type, res) );
	}
	else
	{
		mergeValueMap( it->second.resourceData , res.resourceData );
	}
}

template<typename T> void updateResourceData( TianShanIce::SRM::ResourceMap& resMap , const TianShanIce::SRM::ResourceType& type, const std::string& key , const std::vector<T>& valueIn ,bool bRange )
{
	TianShanIce::SRM::Resource res;
	res.status	=	TianShanIce::SRM::rsRequested;
	res.attr	=	TianShanIce::SRM::raMandatoryNonNegotiable;

	updateValueMapData( res.resourceData , key ,  valueIn ,bRange );

	TianShanIce::SRM::ResourceMap::iterator it = resMap.find( type );
	if( it == resMap.end() )
	{
		resMap.insert( TianShanIce::SRM::ResourceMap::value_type( type, res) );
	}
	else
	{
		mergeValueMap( it->second.resourceData , res.resourceData );
	}
}



TianShanIce::Properties getResourceMapAsProperties( const TianShanIce::SRM::ResourceMap& value );

void			mergeResourceMap( TianShanIce::SRM::ResourceMap& mapA, const TianShanIce::SRM::ResourceMap& mapB , bool bForece = true );
void			mergeProperty( TianShanIce::Properties& propsA , const TianShanIce::Properties& propsB, bool bForce = true );

///dump variant
void			dumpTianShanVariant( const TianShanIce::Variant& var , ZQ::common::Log& logger ,const std::string& hint = "" );
void			dumpTianShanVariant( const TianShanIce::Variant& var , char*& pBuffer , size_t& bufferSize ,const std::string& hint ="");

///dump value map
void			dumpTianShanValueMap( const TianShanIce::ValueMap& value , ZQ::common::Log& logger ,const std::string& hint ="" );
void			dumpTianShanValueMap( const TianShanIce::ValueMap& value , char*& pBuffer , size_t& bufferSize , const std::string& hint = "");

///dump resource
void			dumpTianShanResource( const TianShanIce::SRM::Resource& res , ZQ::common::Log& logger , const std::string& hint ="" );
void			dumpTianShanResource( const TianShanIce::SRM::Resource& res , char*& pBuffer , size_t& bufferSize ,const std::string& hint ="" );

//dump resource map
void			dumpTianShanResourceMap( const TianShanIce::SRM::ResourceMap& resMap , ZQ::common::Log& logger ,const std::string& hint =""  );
//void	dumpTianShanResourceMap( const TianShanIce::SRM::ResourceMap& resMap , char* pBuffer , size_t& bufferSize ,const std::string& hint ="" );

//////////////////////////////////////////////////////////////////////////
///get information from TianShanIce::Streamer::StreamInfo
///get speed from TianShanIce::Streamer::StreamInfo
Ice::Float		getSpeedFromStreamInfo( const TianShanIce::Streamer::StreamInfo& info );
Ice::Int		getUserCtrlNumFromStreamInfo(  const TianShanIce::Streamer::StreamInfo& info );
Ice::Long		getItemTimeOffset( const TianShanIce::Streamer::StreamInfo& info );
Ice::Long		getItemTotalDuration( const TianShanIce::Streamer::StreamInfo& info );
Ice::Long		getStreamTimeOffset( const TianShanIce::Streamer::StreamInfo& info );
Ice::Long		getStreamTotalDuration( const TianShanIce::Streamer::StreamInfo& info );

void			updateSpeedToStreamInfo( TianShanIce::Streamer::StreamInfo& info , const Ice::Float& fSpeed );
void			updateUserCtrlNumToStreamInfo( TianShanIce::Streamer::StreamInfo& info , const Ice::Int& userCtrlNum );
void			updateItemTimeOffsetToStreamInfo( TianShanIce::Streamer::StreamInfo& info , const Ice::Long& timeOffset );
void			updateItemTotalDurationToStreamInfo(  TianShanIce::Streamer::StreamInfo& info , const Ice::Long& timeOffset  );
void			updateStreamTimeOffsetToStreamInfo(  TianShanIce::Streamer::StreamInfo& info , const Ice::Long& timeOffset );
void			updateStreamTotalDurationToStreamInfo(  TianShanIce::Streamer::StreamInfo& info , const Ice::Long& timeOffset );

const char*		dumpStreamInfo( const TianShanIce::Streamer::StreamInfo& info , char* buffer ,size_t bufSize );

///convert stream state into string
const char* 	convertStreamStateToString( const TianShanIce::Streamer::StreamState& state);

///convert item flag into string
std::string		convertItemFlagToStr( Ice::Long flags );

///convert critical start time into string
std::string		convertCriticalStartToStr( const Ice::Long& startTime );

std::string		dumpPlaylistItemSetupInfo( const TianShanIce::Streamer::PlaylistItemSetupInfo& info);

std::string		dumpTianShanIceIValues( const TianShanIce::IValues& values, const std::string& delimiter=" " );

std::string		dumpTianShanIceStrValues(  const TianShanIce::StrValues& values ,const std::string& delimiter=" ");
std::string		dumpStringSets(  const std::set<std::string>& values ,const std::string& delimiter=" ");
std::string		dumpStringMap( const std::map<std::string,std::string>& values );

template<class Ty>
void			copyListToVector( const std::list<Ty>& l, std::vector<Ty>& v )
{
	v.clear();
	typename std::list<Ty>::const_iterator it = l.begin();
	for( ; it != l.end() ; it ++ )
	{
		v.push_back(*it);
	}
}

template<class Ty>
void			copyVectorToList( const std::vector<Ty>& v, std::list<Ty>& l )
{
	l.clear();
	typename std::vector<Ty>::const_iterator it = v.begin();
	for( ; it != v.end() ; it ++ )
	{
		l.push_back(*it);
	}
}


template<class Ty>
void			copyVectorToSet( const std::vector<Ty>& v, std::set<Ty>& l )
{
	l.clear();
	typename std::vector<Ty>::const_iterator it = v.begin();
	for( ; it != v.end() ; it ++ )
	{
		l.insert(*it);
	}
}

template<class Ty>
void			copySetToVector( const std::set<Ty>& v, std::vector<Ty>& l )
{
	l.clear();
	typename std::set<Ty>::const_iterator it = v.begin();
	for( ; it != v.end() ; it ++ )
	{
		l.push_back(*it);
	}
}


///calculate greatest common divisor of two number
template<typename T>
T				calculateGCD(T a, T b)
{
	if( a == 0 || b == 0 )
		return 0;
	T c;
	if( a > b )
	{
		c = a;
		a = b;
		b =c;
	}
	while( a!= 0 )
	{
		c = b % a;
		b = a;
		a = c;
	}
	return b;
}

//////////////////////////////////////////////////////////////////////////
//OS independent FS helper function

std::string			fsFixupPath( const std::string& strPath );

std::string			fsConcatPath( const std::string& parentPath , const std::string& subPath );

bool				fsCreatePath( const std::string& strPath );

bool				fsCreateFile( const std::string& strFile );

std::string			fsGetParentFolderPath( const std::string& strPath );

//////////////////////////////////////////////////////////////////////////
//time helper
std::string			getNowOfUTC( );


//////////////////////////////////////////////////////////////////////////
//suicide
void				suicide( );


class BufferMarshal
{
public:
	BufferMarshal( void* buffer = NULL , size_t size = 0 );
	virtual ~BufferMarshal();

	BufferMarshal&	operator<<( int8 value );
	BufferMarshal&	operator<<( uint8 value );
	BufferMarshal&	operator<<( int16 value );
	BufferMarshal&	operator<<( uint16 value );
	BufferMarshal&	operator<<( int32 value );
	BufferMarshal&	operator<<( uint32 value );
	BufferMarshal&	operator<<( int64 value );
	BufferMarshal&	operator<<( uint64 value );
	BufferMarshal&	operator<<( float value );
	BufferMarshal&	operator<<( double value );
	BufferMarshal&	operator<<( const std::string& value );
	BufferMarshal&	operator<<( const char* value );
	
	BufferMarshal&	operator>>( int8& value );
	BufferMarshal&	operator>>( uint8& value );
	BufferMarshal&	operator>>( int16& value );
	BufferMarshal&	operator>>( uint16& value );
	BufferMarshal&	operator>>( int32& value );
	BufferMarshal&	operator>>( uint32& value );
	BufferMarshal&	operator>>( int64& value );
	BufferMarshal&	operator>>( uint64& value );
	BufferMarshal&	operator>>( float& value );
	BufferMarshal&	operator>>( double& value );
	BufferMarshal&	operator>>( std::string& value );	

	void*		getBuffer( size_t& size);

	void		attachBuffer( void* buffer , size_t size );

protected:

	void	incMemory( size_t newSize = 0 );

	void	putData( const void* data , size_t size );

	enum BufferMarshalDataType
	{
		MARSHAL_TYPE_INTEGER,
		MARSHAL_TYPE_STRING
	};

	void	setDataType(const BufferMarshalDataType& type, size_t size );

	bool	getDataType( BufferMarshalDataType& type , size_t& size);

	bool	getData( const void*& data , size_t size);

private:
	
	void	clear();

private:
	void*			mMemory;
	size_t			mMemSize;
	size_t			mDataSize;
	bool			mbNoMemory;
	bool			mbNeedFree;
};

class TimeSpan
{
public:
	TimeSpan()
	{
		mStart = 0;
		mDelta = 0 ;
	}
	~TimeSpan(){}
	
	void start()
	{
		mStart = ZQ::common::now();
	}
	int64 stop()
	{
		mDelta = ZQ::common::now() - mStart;
		return span();
	}
	int64 span() const
	{
		return mDelta;
	}

private:
	int64	mDelta;
	int64	mStart;
};

template<typename T> // , typename IceStream_t=IceInternal::BasicStream >
size_t marshal(const T& data, TianShanIce::BValues& binstrm, const Ice::CommunicatorPtr& ic)
{
#if ICE_INT_VERSION / 100 >= 306
	Ice::EncodingVersion temp = {3,6};
	IceInternal::BasicStream stream(IceInternal::getInstance(ic).get(),temp);  //IceStream_t 
    Ice::StreamWriter< T, IceInternal::BasicStream>::write(&stream, data);	
    //data.__writeImpl(&stream);
#else
	IceInternal::BasicStream stream(IceInternal::getInstance(ic).get());  //IceStream_t 
	data.__write(&stream);
#endif
	TianShanIce::BValues(stream.b.begin(), stream.b.end()).swap(binstrm);
	return binstrm.size();
}

template<typename T > // , typename IceStream_t=IceInternal::BasicStream >
size_t unmarshal(T& data, const TianShanIce::BValues& binstrm, const Ice::CommunicatorPtr& ic)
{
#if ICE_INT_VERSION / 100 >= 306
	Ice::EncodingVersion temp = {3,6};
	IceInternal::BasicStream stream(IceInternal::getInstance(ic).get(),temp);  //IceStream_t 
	stream.b.resize(binstrm.size());
	memcpy(&stream.b[0], &binstrm[0], stream.b.size());
	// ?? binstrm.swap(stream.b);
	stream.i = stream.b.begin();
//	data.__readImpl(&stream);
    Ice::StreamReader< T, IceInternal::BasicStream>::read(&stream, data);
#else
	IceInternal::BasicStream stream(IceInternal::getInstance(ic).get());  //IceStream_t 
	stream.b.resize(binstrm.size());
	memcpy(&stream.b[0], &binstrm[0], stream.b.size());
	// ?? binstrm.swap(stream.b);
	stream.i = stream.b.begin();
	data.__read(&stream);
#endif
	return stream.b.size();
}


}}//

#endif //_ZQ_TianShan_UTILITY_HEADER_FILE_H__

