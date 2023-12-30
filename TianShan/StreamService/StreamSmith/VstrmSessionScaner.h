
#ifndef _zq_StreamService_Vstrm_Session_scaner_header_file_h__
#define _zq_StreamService_Vstrm_Session_scaner_header_file_h__

#include <NativeThread.h>
#include <SsEnvironment.h>
#include "VstrmStreamerManager.h"

namespace ZQ
{
namespace StreamService
{

class VstrmSessionScaner : public ZQ::common::NativeThread
{
public:
	VstrmSessionScaner( SsEnvironment* environment , VstrmStreamerManager& manager );
	virtual ~VstrmSessionScaner( );

public:

	void		attachServiceInstance( SsServiceImpl* s );

	void		stop( );

protected:	

	int			run( );

	void		scanSessions( );

private:

	friend  static VSTATUS vstrmFOR_EACH_SESSION_CB( HANDLE,PVOID,ESESSION_CHARACTERISTICS*,ULONG,ULONG,ULONG);
	static VSTATUS vstrmFOR_EACH_SESSION_CB( HANDLE vstrmClassHandle, 
											PVOID cbParm,
											ESESSION_CHARACTERISTICS* sessionChars, 
											ULONG sizeofSessionChars, 
											ULONG currentSessionInstance, 
											ULONG totalSessions );

	void			checkVstrmPort( );


protected:

	
private:

	
	SsEnvironment*				env;	
	VstrmStreamerManager&		vstrmManager;
	typedef std::vector<ESESSION_CHARACTERISTICS>	SESSIONS;
	SESSIONS			oldSessionInfo;
	SESSIONS			newSessionInfo;
	SESSIONS			expiredSessionInfo;

	bool				mbQuit;	

	SsServiceImpl*		ss;

private:
	
	void				notifyStateChange(  ULONG sessionId , UCHAR newState , Ice::Long timeStamp );
	void				notifySpeedChange(  ULONG sessionId , SPEED_IND oldSpeed , SPEED_IND newSpeed ,  Ice::Long timeStamp  );
	void				notifyNewSession(   ULONG sessionId  ,  Ice::Long timeStamp  );
	void				notifyExpiredSession( ULONG sessionId , Ice::Long timeStamp  );
	void				notifyProgress( ULONG sessionId , ULONG curOffset, ULONG totalDuration );

	TianShanIce::Streamer::StreamState convertVstrmStateToTianShanStreamState( UCHAR state );
};


}}

#endif//_zq_StreamService_Vstrm_Session_scaner_header_file_h__
