
#ifndef _ZQ_TianShan_StreamService_StreamBase_h__
#define _ZQ_TianShan_StreamService_StreamBase_h__

#include <ZQ_common_conf.h>
#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include "Playlist.h"
#include "SsEnvironment.h"


namespace ZQ
{
namespace StreamService
{

#define SEEK_FROM_BEGIN			1
#define SEEK_FROM_END			2
#define SEEK_FROM_CURRENT		0

class SsServiceImpl;

class SsStreamBase : public TianShanIce::Streamer::SsPlaylist ,public IceUtil::AbstractMutexI<IceUtil::RecMutex>
{
public:	
	SsStreamBase(SsServiceImpl& serviceImpl , SsEnvironment* environment );
	SsStreamBase(SsServiceImpl& serviceImpl , SsEnvironment* environment ,bool );
	virtual ~SsStreamBase( );

public:
	virtual ::TianShanIce::SRM::ResourceMap getResources( const Ice::Current& = ::Ice::Current() ) const;
	virtual ::TianShanIce::Transport::PathTicketPrx getPathTicket(const ::Ice::Current& = ::Ice::Current()) const ;

	typedef TianShanIce::Streamer::ItemSessionInfoS::const_iterator constIter;
	typedef TianShanIce::Streamer::ItemSessionInfoS::iterator		iter;

	/*
	----------------------------------------------------------
			|    |    |    |    |    |   |   |   |    |             
		x	|    |    |    |    |    |   |   |   |    |   x
			|    |    |    |    |    |   |   |   |    |             
	----------------------------------------------------------
	Begin    first                                last   end

	*/
	//beginItem and endItem are both invalid item used for identify the end of list
	inline iter						iterBeginItem( )
	{
		assert( itemCount() >= 0 );
		return items.begin();
	}
	inline constIter				iterBeginItem( ) const
	{
		assert( itemCount() >= 0 );
		return items.begin();
	}

	inline iter						iterEndItem( )
	{
		assert( itemCount() >= 0 );
		return items.end();
	}	
	inline constIter				iterEndItem( ) const
	{
		assert( itemCount() >= 0 );
		return items.end();
	}

	inline iter						iterFirstItem( )
	{
		assert( itemCount() >= 0 );
		return items.begin() + 1;		
	}
	inline constIter				iterFirstItem( ) const
	{
		assert( itemCount() >= 0 );
		return items.begin() + 1;		
	}

	inline iter						iterLastItem( )
	{
		assert( itemCount() >= 0 );
		return items.end() - 1;
	}
	inline constIter				iterLastItem( ) const
	{
		assert( itemCount() >= 0 );
		return items.end() - 1;
	}
	iter							iterEffective( )
	{
		return effectiveIter;
	}
	constIter						iterEffective( ) const
	{
		return effectiveIter;
	}

	iter							iterCurrentItem( );
	constIter						iterCurrentItem( ) const;

	iter							iterNextItem( );	
	constIter						iterNextItem( ) const;	
	

	inline bool						iterValid( constIter it ) const
	{
		assert( itemCount() >= 0 );
		return (it != iterBeginItem() && it != iterEndItem());
	}

	inline	size_t						itemCount( ) const
	{
		assert(items.size( ) >= 1 );
		return items.size( ) - 1;
	}

	bool						iterValid( iter it ) const
	{
		assert( itemCount() >= 0 );
		return (it != iterBeginItem() && it != iterEndItem());
	}

	iter						findItemWithUserCtrlNum( Ice::Int userCtrlNum )	;
	constIter					findItemWithUserCtrlNum( Ice::Int userCtrlNum ) const;

	bool						isItemStreaming( constIter it ) const;

	/// return true if current streaming direction is backward , vice versa
	/// NOTE : what current item is streaming or not , there is do have a direction for it
	inline bool					isReverseStreaming( ) const
	{
		return speed < -0.001f ;
	}

	///find item with time offset
	///offset : time offset , if from == SEEK_FROM_BEGIN , offset must be positive , 
	///			if from == SEEK_FROM_END offset must be negative
	///
	iter						findItemWithTimeOffset( INOUT Ice::Long offset , INOUT Ice::Short from  );
	constIter					findItemWithTimeOffset( INOUT Ice::Long offset , INOUT Ice::Short from  ) const;

// 	///find item with IRP structure
// 	///offset relative to the item/stream begin
// 	iter						findItemWithIrp( const IRP& irp , OUT Ice::Long& offset );
// 	constIter					findItemWithIrp( const IRP& irp , OUT Ice::Long& offset ) const;




	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	///																	  ///
	///																	  ///
	///use it at your own risk
	/// remember do it within a locker
	TianShanIce::Properties&	getStreamProperties( )
	{		
		return	property;
	}
	
	///																	  ///
	///																	  ///
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	bool						setEffectiveItem( Ice::Int userCtrlNum )
	{
		effectiveIter	= findItemWithUserCtrlNum(userCtrlNum);
		return iterValid(effectiveIter);
	}
	
	bool						setEffectiveItem( iter it )
	{
		effectiveIter = it;
		return iterValid(effectiveIter);
	}

	
	std::string					getPID( constIter it ) const;
	std::string					getPAID( constIter it ) const;
	std::string					getNextURL( constIter it ) ;
	std::string					getLastURL( constIter it ) const;
	bool						hasURL( constIter it ) const;

	//////////////////////////////////////////////////////////////////////////
	///implement ice definition
	virtual ::TianShanIce::Properties getProperties(const ::Ice::Current& = ::Ice::Current()) const ;
	virtual void setProperties(const ::TianShanIce::Properties&, const ::Ice::Current& = ::Ice::Current()) ;

protected:	

	bool						setCurrentItem( Ice::Int userCtrlNum );
	bool						setCurrentItem( iter it );
	bool						setNextItem(  Ice::Int userCtrlNum );
	bool						setNextItem( iter it );

	int32						getEOTSize( ) const;

	int32						getPreloadTime( ) const;
	
	/*void						setCurrentItem( const Ice::Int&  userCtrlNum );*/

protected:
	typedef struct _PlInstanceState
	{
		constIter								itItem;
		TianShanIce::Streamer::StreamState		state;
		Ice::Long								timeOffset;
	}PlInstanceState;

	PlInstanceState				mInstanceStateBeforeAction;
	PlInstanceState				mInstanceStateAfterAction;


	void						recordInstanceState( PlInstanceState& s );

	bool						initializeStream( );

protected:
	SsServiceImpl&				serviceImpl;
	SsEnvironment*				env;

protected:

	iter						currentIter;
	bool						bCurIterInited;
	iter						nextIter;
	bool						bNextIterInited;
	iter						effectiveIter;	
	unsigned long               stampSpeedChangeReq;
	unsigned long               stampStateChangeReq;

	
};

}
}

#endif//_ZQ_TianShan_StreamService_StreamBase_h__
