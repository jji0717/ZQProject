#ifndef __tianshan_siteadmin_txn_transfer_header_file_h__
#define __tianshan_siteadmin_txn_transfer_header_file_h__

#include <list>
#include <Locks.h>
#include <NativeThread.h>
#include "SystemUtils.h"

class SiteAdminEnv;
class SiteAdminDb;

class TxnTransfer : public ZQ::common::NativeThread
{
public:
	TxnTransfer( SiteAdminEnv& env  , SiteAdminDb& db );
	virtual ~TxnTransfer( );

	void	pushSess( const std::string& sessId  );

	void	stop( );

protected:
	
	int		run( );

private:
	SiteAdminEnv&			mEnv;
	SiteAdminDb&			mDb;
	ZQ::common::Mutex		mLocker;
	std::list<std::string>	mSessIds;
	bool					mbQuit;
	SYS::SingleObject		mEvent;
};

#endif//__tianshan_siteadmin_txn_transfer_header_file_h__
