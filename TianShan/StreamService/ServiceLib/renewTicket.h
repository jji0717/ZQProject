
#ifndef _ZQ_StreamService_Renew_Ticket_Command_header_file_h__
#define _ZQ_StreamService_Renew_Ticket_Command_header_file_h__

#include <map>
#include <list>
#include <NativeThreadPool.h>
#include <Locks.h>
#include <TsTransport.h>
#include "Scheduler.h"
#include "Playlist.h"
#include "SsEnvironment.h"
#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>


namespace ZQ
{
namespace StreamService
{

class RenewTicketCenter;
class TicketRenewRunner : public ZQ::common::ThreadRequest
{
public:
	TicketRenewRunner( SsEnvironment* environment, RenewTicketCenter& rtCenter , ZQ::common::NativeThreadPool& pool , const Ice::Identity& id);
	virtual ~TicketRenewRunner();
protected:
	int	run( );
	void final(int retcode /* =0 */, bool bCancelled /* =false */);
private:
	RenewTicketCenter&		mRenewCenter;
	Ice::Identity			mId;
	SsEnvironment*			env;
};

class RenewTicketTask : public ScheduleTask
{
public:
	
	RenewTicketTask( RenewTicketCenter& rtCenter ,const Ice::Identity& id );

	virtual ~RenewTicketTask( );

protected:
	
	virtual void	runTask( ) ;

private:
	
	RenewTicketCenter&						center;
	Ice::Identity							plId;	
	SsEnvironment*							env;
};


class SsServiceImpl;
class RenewTicketCenter : public ZQ::common::NativeThread
{
public:
	RenewTicketCenter( SsEnvironment* environment , SsServiceImpl& svc );
	virtual ~RenewTicketCenter( );
	friend class RenewTicketTask;
public:

	bool			start( );
	
	void			doRegister( const Ice::Identity& id , TianShanIce::Transport::PathTicketPrx ticket );

	void			doUnregister( const Ice::Identity& id );

	void			stop( );

	TianShanIce::Transport::PathTicketPrx getTicket( const Ice::Identity& id ) const;

protected:

	void			notify( const Ice::Identity& id );

protected:

	int				run( );
public:
	void			scheduleRenewCommand( const Ice::Identity& id  , int multi = 1 );

private:	
	SsEnvironment*			env;	
	typedef std::map< Ice::Identity , TianShanIce::Transport::PathTicketPrx >	ID2TICKETMAP;
	typedef std::list< Ice::Identity > TICKETLIST;
	ID2TICKETMAP			mIdTicketMap;	
	TICKETLIST				mTickets;
	ZQ::common::Mutex		mMutex;
	ZQ::common::Semaphore	mSemaphore;
	bool					mbQuit;
	SsServiceImpl&			mSvcImpl;
};

}}
#endif//_ZQ_StreamService_Renew_Ticket_Command_header_file_h__
