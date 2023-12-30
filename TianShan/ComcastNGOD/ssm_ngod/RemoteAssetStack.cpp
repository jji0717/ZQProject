
#include <ZQ_common_conf.h>
#include <TianShanDefines.h>
#include "SelectionCommand.h"
#include "RemoteAssetStack.h"


RemoteAssetStack::RemoteAssetStack( SelectionEnv& env )
:mEnv(env)
{
}

RemoteAssetStack::~RemoteAssetStack(void)
{
}

std::string getStreamerZone( const std::string& streamerNetId )
{
	std::string::size_type pos = streamerNetId.find('/');
	if ( pos != std::string::npos )
	{
		std::string tmp = streamerNetId.substr( 0 , pos );;
		return streamerNetId.substr( 0 , pos );
	}
	else
	{
		return streamerNetId;
	}
}
void RemoteAssetStack::registerSession( const ElementInfoS& elements , const std::string& streamerNetId  )
{
	RemoteAssetS assets;
	ElementInfoS::const_iterator itElement = elements.begin();
	for( ; itElement != elements.end() ; itElement ++ )
	{
		AssetKey k;
		k.pid		= itElement->pid;
		k.paid		= itElement->paid;
		assets.insert( k );
	}
	registerSession( assets,  streamerNetId );
}
void RemoteAssetStack::registerSession( const RemoteAssetS& assets , const std::string& streamerNetId )
{
	if( !mEnv.mbEnableAssetStack )
		return;
	std::string streamerZone = getStreamerZone( streamerNetId );
	ZQ::common::MutexGuard gd(mMutex);
	
	AssetStackMap::iterator it = mAssetStack.find( streamerZone );
	if( it == mAssetStack.end() )
	{//just add new record
		RemoteAssetS stackAssets = assets;
		RemoteAssetS::iterator itAsset = stackAssets.begin();
		for( ; itAsset != stackAssets.end() ; itAsset++ )
		{
			itAsset->hitCount = 1;
			itAsset->timeEdge = ZQTianShan::now();
		}
		mAssetStack.insert( AssetStackMap::value_type( streamerZone , stackAssets ) );
	}
	else
	{
		RemoteAssetS& stackAssets = it->second;
		RemoteAssetS::const_iterator itAsset = assets.begin();
		for ( ; itAsset != assets.end() ; itAsset ++ )
		{
			RemoteAssetS::iterator itStackAsset = stackAssets.find(*itAsset);
			if( itStackAsset != stackAssets.end() )
			{
				itStackAsset->hitCount = calcPopularity( itStackAsset->timeEdge , itStackAsset->hitCount );
				if( itStackAsset->hitCount <= 0 )
				{
					stackAssets.erase( itStackAsset );
				}
			}
			else
			{
				AssetKey key = *itAsset;
				key.hitCount = 1;
				key.timeEdge = ZQTianShan::now();
				stackAssets.insert( key );
			}
		}
	}
}

void RemoteAssetStack::unregisterSession( const RemoteAssetS& assets , const std::string& streamerNetId )
{
	if( !mEnv.mbEnableAssetStack )
		return;
	std::string streamerZone = getStreamerZone( streamerNetId );
	ZQ::common::MutexGuard gd(mMutex);
	AssetStackMap::iterator it = mAssetStack.find( streamerZone );
	if( it == mAssetStack.end() )
		return;
	RemoteAssetS& stackAssets = it->second;
	RemoteAssetS::const_iterator itAsset = assets.begin();
	for( ; itAsset != assets.end() ; itAsset++ )
	{
		RemoteAssetS::iterator itStackAsset = stackAssets.find( *itAsset );
		if( itStackAsset == stackAssets.end() )
			continue;
		
		itStackAsset->hitCount -- ;
		if( itStackAsset->hitCount <= 0)
			stackAssets.erase( itStackAsset );
	}
	if( stackAssets.empty() )
		mAssetStack.erase( streamerZone );
}

int32 RemoteAssetStack::calcPopularity( int64& timeEdge ,int32 reqCount , bool bQuery )
{
	assert( mEnv.mAssetStackTimeShiftWindow >= 0 );
	if(  mEnv.mAssetStackTimeShiftWindow <= 0 )
	{//invalid window size
		return 0;
	}

	int64 stampPrev = timeEdge;
	if (stampPrev <=0)
		return 0;

	timeEdge = ZQTianShan::now();
	
	if(!bQuery)
		++reqCount;//increase reqCount due to new request come here

	int step =(int) ( timeEdge - stampPrev ) * 1 / mEnv.mAssetStackTimeShiftWindow;
	
	reqCount -= step;
	
	reqCount = max( 0 , reqCount );

	return reqCount;
}

int32 RemoteAssetStack::findAssetPopularity( const std::string& streamer ,  const ElementInfoS& elements )
{//instance should be locked out side of this function
	AssetStackMap::iterator it = mAssetStack.find( getStreamerZone(streamer) );
	if( it == mAssetStack.end() )
		return 0;
	RemoteAssetS& stackAssets = it->second;
	ElementInfoS::const_iterator itElement = elements.begin();
	for( ; itElement != elements.end() ; itElement ++ )
	{
		AssetKey k;
		k.pid	= itElement->pid;
		k.paid	= itElement->paid;
		RemoteAssetS::iterator itStackAsset = stackAssets.find(k);
		if( itStackAsset != stackAssets.end() )
		{
			itStackAsset->hitCount = calcPopularity( itStackAsset->timeEdge , itStackAsset->hitCount , true );
			if( itStackAsset->hitCount <= 0 )
			{
				stackAssets.erase( itStackAsset );
			}
			else
			{
				return k.hitCount;//found one , return 1
			}
		}
	}
	return 0;
}

bool RemoteAssetStack::adjustWeight( const ElementInfoS& elements , ResourceStreamerAttrExS& streamers )
{
	if( !mEnv.mbEnableAssetStack )
		return true;

	bool bAdusted =false;

	ZQ::common::MutexGuard gd(mMutex);
	
	ResourceStreamerAttrExS::iterator itStreamer = streamers.begin();
	
	for( ; itStreamer != streamers.end() ; itStreamer++ )
	{
		if( findAssetPopularity( itStreamer->netId , elements ) >  0 )
		{
			itStreamer->streamerWeight += mEnv.mAssetStackAdjustWeight; //higher its weight
			bAdusted = true;
		}
	}

	return true;
}
