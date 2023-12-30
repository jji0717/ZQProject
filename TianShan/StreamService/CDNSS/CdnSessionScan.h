
#ifndef _cdn_session_scanner_header_file_h__
#define _cdn_session_scanner_header_file_h__

#include <NativeThread.h>
#include <Locks.h>
#include <vector>
#include <list>
#include <SsServiceImpl.h>
#include "C2StreamerLib.h"

namespace ZQ
{
namespace StreamService
{
typedef C2Streamer::SessionStatusInfo SessionAttr;
typedef std::vector<SessionAttr> SessionAttrS;
class CdnSsEnvironment;

class CdnSessionScaner : public ZQ::common::NativeThread
{
public:
	CdnSessionScaner( CdnSsEnvironment* environment , SsServiceImpl* serviceInstance );
	virtual ~CdnSessionScaner();
public:

	bool		init(void);
	void		stop( );

	int			run( );

protected:

	void		onSessionNew( SessionAttrS::const_iterator itNew );
	void		onSessionExpired( SessionAttrS::const_iterator itOld );
	void		onSessionStateChange( SessionAttrS::const_iterator itOld , SessionAttrS::const_iterator itNew  );
	
private:

	void		getSessionData( );

private:
	SessionAttrS	mSessionsOld;
	SessionAttrS	mSessionsNew;
	bool					mbQuit;
	CdnSsEnvironment*		env;
	SsServiceImpl*			ss;
	ZQ::common::Semaphore	mSem;
};

}}


#endif//_cdn_session_scanner_header_file_h__
