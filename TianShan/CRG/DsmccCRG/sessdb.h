#ifndef __zq_dsmcc_gateway_session_database_header_file_h__
#define __zq_dsmcc_gateway_session_database_header_file_h__

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>

#include <TianShanDefines.h>

#include "clientidx.h"
#include "clientrequest.h"


namespace ZQ { namespace CLIENTREQUEST{

class Environment;

class SessionDatabase
{
public:
	SessionDatabase( Environment& env );
	virtual ~SessionDatabase(void);

	bool	openDB( const std::string& dbpath , bool enableClientIdIndex, ZQADAPTER_DECLTYPE objadapter );

	void	closeDB( );

	TianShanIce::ClientRequest::SessionPrx	openSession( const std::string& sessId );
	std::vector<TianShanIce::ClientRequest::SessionPrx> findSessionByClient( const std::string& clientId );

	bool	addSession( TianShanIce::ClientRequest::SessionPtr sess );
	void	removeSession( const std::string& sessId );
	std::vector<std::string> loadAllSessionIds( );
protected:
	
	void updateIceProperty( Ice::PropertiesPtr iceProperty , const std::string& key ,const std::string& value );	

private:
	Environment&								mEnv;

	TianShanIce::ClientRequest::ClientIdxPtr	mClientIdx;	
	Freeze::EvictorPtr							mEvictor;

	ZQADAPTER_DECLTYPE							mAdapter;
};

}}//namespace ZQ::DSMCC

#endif//__zq_dsmcc_gateway_session_database_header_file_h__
