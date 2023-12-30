//DSMCC Gateway session ice definition file
#include <Ice/Identity.ice>
#include <TsSRM.ICE>
#include <TsStreamer.ICE>
#include <TsApplication.ICE>

module TianShanIce
{
module ClientRequest
{

interface Gateway
{


};


["freeze:write"]
class Session
{	
	string				sessionId;
	string				clientId;
	long				timeoutTarget;

	SRM::Session*		weiwooSession;
	Streamer::Stream*	streamSession;
	Application::Purchase* appSession;
	
	TianShanIce::Properties sessProps;
	TianShanIce::State	sessState;

	//void				touch();// notify ice runtime that current session's data has been modified

	void				destroy( );//destroy current session, will retry if not succeed

	void				attachWeiwooSession( SRM::Session* sess );
	void				attachStreamSession( Streamer::Stream* sess ,string streamSessId );
	void				attachPurchaseSession( Application::Purchase* sess );
	void				setProperty( string key , string value);
	void				setProperties( TianShanIce::Properties props );
	void				removeProperty( string key );

	void				updateTimer( long interval );
	
	["Freeze:read","cpp:const"]
	SRM::Session*	getWeiwooSession( );
	
	["Freeze:read","cpp:const"]
	Streamer::Stream*		getStreamSession( );

	["Freeze:read","cpp:const"]
	Application::Purchase*	getPurchaseSession( );

	["Freeze:read","cpp:const"]
	string					getProperty( string key );

	["Freeze:read","cpp:const"]
	TianShanIce::Properties	getProperties( );

	["Freeze:read","cpp:const"]
	TianShanIce::State		getState( );

	["Freeze:read","cpp:const"]
	string					getSessId( );

	["Freeze:read","cpp:const"]
	string					getClientId( );

	void					onRestore( );
};


};
};
