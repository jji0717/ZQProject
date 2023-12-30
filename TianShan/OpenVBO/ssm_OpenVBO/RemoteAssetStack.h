
#ifndef _tianshan_ngod_selection_asset_stack_header_file_h__
#define _tianshan_ngod_selection_asset_stack_header_file_h__

#include <ZQ_common_conf.h>
#include <Locks.h>
#include <NativeThread.h>


class SelectionEnv;
struct ElementInfo;
typedef std::vector<ElementInfo> ElementInfoS;
struct ResourceStreamerAttrEx;
typedef std::vector< ResourceStreamerAttrEx > ResourceStreamerAttrExS;

class RemoteAssetStack 
{
public:
	RemoteAssetStack( SelectionEnv& env );
	virtual ~RemoteAssetStack(void);

public:
	struct AssetKey 
	{
		std::string			pid;
		std::string			paid;
		mutable int         hitCount;
		mutable int64       timeEdge;
		AssetKey ()
		{
			hitCount = 0;
		}
		bool operator<( const AssetKey& b ) const
		{
			if( pid < b.pid )			{	return true;			}
			else if( pid == b.pid )		{	return paid < b.paid ;	}
			else						{	return true;			}
		}
	};

	typedef std::set<AssetKey>	RemoteAssetS;


	bool			adjustWeight( const ElementInfoS& elements , ResourceStreamerAttrExS& streamers );

	void			registerSession( const ElementInfoS& elements , const std::string& streamerNetId  );
	void			registerSession( const RemoteAssetS& assets , const std::string& streamerNetId  );

	void			unregisterSession( const RemoteAssetS& assets , const std::string& streamerNetId );

protected:

	int32			calcPopularity(  int64& timeEdge , int32 reqCount ,bool bQuery = false );

	int32			findAssetPopularity( const std::string& streamer ,  const ElementInfoS& elements );

private:

	SelectionEnv&			mEnv;	

		
	typedef std::map< std::string , RemoteAssetS > AssetStackMap;
	//                streamerZone , assets info

	AssetStackMap				mAssetStack;
	ZQ::common::Mutex			mMutex;
	ZQ::common::Semaphore		mSem;	
};


#endif//_tianshan_ngod_selection_asset_stack_header_file_h__


