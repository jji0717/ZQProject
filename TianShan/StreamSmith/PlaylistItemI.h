#ifndef _ZQ_STREAMSMITH_PlaylistItem_Implement_H__
#define	_ZQ_STREAMSMITH_PlaylistItem_Implement_H__

#include "StreamSmithadmin.h"
#include <IceUtil/IceUtil.h>

namespace ZQ
{
namespace StreamSmith
{

class PlaylistItemI:public TianShanIce::Streamer::PlaylistItem,public IceUtil::AbstractMutexWriteI<IceUtil::RWRecMutex>
{
public:
	PlaylistItemI();
	~PlaylistItemI();
public:
	void destroy(const ::Ice::Current& = ::Ice::Current());	
    
    ::TianShanIce::Streamer::PlaylistItemAttr getAttr(const ::Ice::Current& = ::Ice::Current()) const;
    
	void updateAttr(const ::TianShanIce::Streamer::PlaylistItemAttr& pliAttr, 
					const ::Ice::Current& = ::Ice::Current());    

private:
};
}
}
#endif