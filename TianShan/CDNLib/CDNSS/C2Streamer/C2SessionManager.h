#ifndef _tianshan_cdnss_c2streamer_session_manager_header_file_h__
#define _tianshan_cdnss_c2streamer_session_manager_header_file_h__

#include <Locks.h>
#include <NativeThread.h>
#include <LRUMap.h>
#include <DataPostHouse/common_define.h> //use ObjectHandle
#include <map>

#undef max
#undef min

#include "AioFile.h"

namespace C2Streamer
{

class C2StreamerEnv;
class C2Session;
class C2Service;

typedef ZQ::DataPostHouse::ObjectHandle<C2Session> C2SessionPtr;

class C2ClientManager
{
public:
	C2ClientManager( C2StreamerEnv& env );
	virtual ~C2ClientManager();

public:
	
	struct sessionAttr 
	{
		std::string		sessionId;
		int64			usedBW;
	};

	typedef std::map< std::string , sessionAttr> SESSIONATTRS;
	/*                sessionId ,    sessionAttr*/

	struct ClientAttr 
	{
		std::string			clientIp;
		int64				capcacity;
		int64				usedBW;
		SESSIONATTRS		sessInfos;
		ClientAttr()
		{
			capcacity	= 0;
			usedBW		= 0;
		}
	};

	typedef std::map< std::string , ClientAttr > ClientMap;

	bool			registerSession( const std::string& clientIp , const std::string& sessId , int64 requestBW );

	void			unregisterSession( const std::string& clientIp , const std::string& sessId );

	void			updateClient( const std::string& clientIp , int64 capacity );

	bool			getClientAttr( const std::string& clientIp , ClientAttr& attr ) const;

	void			getClientsAttr( ClientMap& attrs ) const;

private:
	C2StreamerEnv&			mEnv;	
	ClientMap				mClients;
	ZQ::common::Mutex		mMutex;
};

class C2SessionManager : public ZQ::common::NativeThread
{
public:
	C2SessionManager( C2StreamerEnv& env ,C2Service& svc );
	virtual ~C2SessionManager(void);

public:

	C2SessionPtr		createSession( const std::string& sessId );

	void				destroySession( const std::string& sessionId );
	
	void				removeSession( const std::string& sessionId );

	C2SessionPtr		findSession( const std::string& sessionId ) const;

	void				schedulerSession( const std::string& sessionId , int64 timeInterval );
	
	void				unScheduleSession( const std::string& sessionId );
	
	///stop sesson manager and clear all sessions
	void				stop( );

	AssetAttribute::Ptr	getAssetAttribute( const std::string& filename );

protected:

	int					run( );

protected:
	C2StreamerEnv&			mEnv;
	C2Service&				mSvc;
	ZQ::common::Mutex		mMutex;
	typedef std::map<std::string , C2SessionPtr> SessMap;
	SessMap					mSessions;
	bool					mbRunning;

	typedef ZQ::common::LRUMap<std::string, AssetAttribute::Ptr > AssetAttributeMap;
	AssetAttributeMap 		mAssetAttrs;
};

}

#endif//_tianshan_cdnss_c2streamer_session_manager_header_file_h__

