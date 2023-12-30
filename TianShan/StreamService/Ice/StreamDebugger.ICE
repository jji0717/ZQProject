
#ifndef __ZQ_TianShanIce_StreamDebugger_ICE__
#define __ZQ_TianShanIce_StreamDebugger_ICE__

#include <StreamSmithPlaylist.ice>

module TianShanIce
{
module Streamer
{

///debug monitor should implement this
interface DebugEventObserver
{
	["ami","amd"]	onPlaylistEvent( Identity id ,  SsPlaylist* playlist , int eventId , ValueMap data );
	
};

sequence<DebugEventObserver*>	DebugEventObserverS;
struct serviceDebugInfo
{
	
};


const	int				loadTimeCostMin					=	1;
const	int				loadTimeCostMax					=	2;
const	int				loadTimeCostAvg					=	3;
	
const	int				playTimeCostMin					=	11;
const	int				playTimeCostMax					=	12;
const	int				playTimeCostAvg					=	13;
	
const	int				pauseTimeCostMin				=	21;
const	int				pauseTimeCostMax				=	22;
const	int				pauseTimeCostAvg				=	23;
	
const	int				changeSpeedTimeCostMin			=	31;
const	int				changeSpeedTimeCostMax			=	32;
const	int				changeSpeedTimeCostAvg			=	33;
	
const	int				unloadTimeCostMin				=	41;
const	int				unloadTimeCostMax				=	42;
const	int				unloadTimeCostAvg				=	43;

const	int				resumeTimeCostMin				=	51;
const	int				resumeTimeCostMax				=	52;
const	int				resumeTimeCostAvg				=	53;




///
///for every single playlist , record every single operation if profile is enabled
///
["freeze:write"] 
class  playlistDebug extends SsPlaylist
{	
	bool									bEnableProfile;
	
	void														enableProfile( bool bEnable );
	
	///reset profile record , any previous record will be removed
	void														resetProfile( );
	
	["cpp:const", "freeze:read"]	OperationProfileS			getPlaylistProfile( );
	
	["cpp:const", "freeze:read"]	OperationProfileS			getItemProfile( );
	
	["cpp:const", "freeze:read"]	bool						isProfileEnable( );
}


class  StreamSmithServiceDebug extends StreamSmithAdmin
{	
	///service debug information
	serviceDebugInfo				debugInfo;
	
	///registered event observers	
	DebugEventObserverS				regsiteredObservers;
	
	OperationProfileS				opProfiles;
	
	void							registerDebugObserver( DebugEventObserver* observer ) throws InvalidParamater,ServerError;
	
};


};
};


#endif//__ZQ_TianShanIce_StreamDebugger_ICE__
