
#include "VstrmSessionCallbackManager.h"
#include <SsEnvironment.h>

#include <memoryDebug.h>

#undef ENVLOG
#define ENVLOG	( *(getSsEnv()->getMainLogger()) )

namespace ZQ
{
namespace StreamService
{

VstrmSessionCallbackManager::VstrmSessionCallbackManager( )
{
	mCallBackId = 0;
}
VstrmSessionCallbackManager::~VstrmSessionCallbackManager( )
{
}

std::string VstrmSessionCallbackManager::getPlId( uint32 id ) const
{	
	ZQ::common::MutexGuard gd(mMapMutex);

	STREAM2PLMAP::const_iterator it = mId2PlMap.find( id );
	if( it != mId2PlMap.end() )
	{
		return it->second;
	}
	else
	{
		return "";
	}
}

uint32 VstrmSessionCallbackManager::registerPlaylist( const std::string& pl )
{
	ZQ::common::MutexGuard gd(mMapMutex);
	PL2STREAMMAP::const_iterator itPl2Id = mPl2IdMap.find( pl );	
	if( itPl2Id != mPl2IdMap.end() )
		return itPl2Id->second;
	do
	{
		++mCallBackId;
		STREAM2PLMAP::const_iterator itId2Pl = mId2PlMap.find( mCallBackId );
		if( itId2Pl == mId2PlMap.end() )
		{
			break;
		}
	}while( true);
	mId2PlMap.insert( STREAM2PLMAP::value_type( mCallBackId , pl ) );
	mPl2IdMap.insert( PL2STREAMMAP::value_type( pl,mCallBackId ) );
#if defined _DEBUG || defined DEBUG
	ENVLOG(ZQ::common::Log::L_DEBUG,
		CLOGFMT(VstrmSessionCallbackManager,"registerPlaylist with PL[%s] and ID[%u] current total callback count[%u][%u] "),
		pl.c_str(),	mCallBackId,
		mId2PlMap.size() ,
		mPl2IdMap.size() );
#endif
	return mCallBackId;
}

void VstrmSessionCallbackManager::unregisterPlaylist( const uint32 id )
{
	ZQ::common::MutexGuard gd(mMapMutex);
	STREAM2PLMAP::iterator it = mId2PlMap.find( id );	
	if( it != mId2PlMap.end() )
	{
		mPl2IdMap.erase( it->second );
		mId2PlMap.erase( it );
	}
#if defined _DEBUG || defined DEBUG
	ENVLOG(ZQ::common::Log::L_DEBUG,
		CLOGFMT(VstrmSessionCallbackManager,"unregisterPlaylist with ID[%u] current total callback count[%u][%u] "),
		id,
		mId2PlMap.size() ,
		mPl2IdMap.size() );
#endif
}

void VstrmSessionCallbackManager::unregisterPlaylist( const std::string& pl )
{
	ZQ::common::MutexGuard gd(mMapMutex);
	PL2STREAMMAP::iterator it = mPl2IdMap.find( pl );	
	if( it != mPl2IdMap.end() )
	{
		mId2PlMap.erase( it->second );
		mPl2IdMap.erase( it );
	}
#if defined _DEBUG || defined DEBUG
	ENVLOG(ZQ::common::Log::L_DEBUG,
		CLOGFMT(VstrmSessionCallbackManager,"unregisterPlaylist with PL[%s] current total callback count[%u][%u] "),
		pl.c_str(),
		mId2PlMap.size(),
		mPl2IdMap.size() );
#endif
}

}}
