#ifndef __ZQ_TianShanIce_StreamDebugger_ICE__
#define __ZQ_TianShanIce_StreamDebugger_ICE__

#include <Playlist.ice>

module TianShanIce
{
module Streamer
{

///debug monitor should implement this
interface DebugEventObserver
{
	["ami","amd"]	void onDebugEvent( SsPlaylist* playlist , int eventId , ValueMap data );	
};

sequence<DebugEventObserver*>	DebugEventObserverS;

struct serviceDebugInfo
{
	int			infoType;//place holder
};


const	int				dbgLoadTimeCostMin					=	1;
const	int				dbgLoadTimeCostMax					=	2;
const	int				dbgLoadTimeCostAvg					=	3;
	
const	int				dbgPlayTimeCostMin					=	11;
const	int				dbgPlayTimeCostMax					=	12;
const	int				dbgPlayTimeCostAvg					=	13;
	
const	int				dbgPauseTimeCostMin					=	21;
const	int				dbgPauseTimeCostMax					=	22;
const	int				dbgPauseTimeCostAvg					=	23;
	
const	int				dbgChangeScaleTimeCostMin			=	31;
const	int				dbgChangeScaleTimeCostMax			=	32;
const	int				dbgChangeScaleTimeCostAvg			=	33;
	
const	int				dbgUnloadTimeCostMin				=	41;
const	int				dbgUnloadTimeCostMax				=	42;
const	int				dbgUnloadTimeCostAvg				=	43;

const	int				dbgResumeTimeCostMin				=	51;
const	int				dbgResumeTimeCostMax				=	52;
const	int				dbgResumeTimeCostAvg				=	53;

const	int				dbgRepositionTimeCostMin			=	61;
const	int				dbgRepositionTimeCostMax			=	62;
const	int				dbgRepositionTimeCostAvg			=	63;

const	int				dbgGetInfoTimeCostMin				=	71;
const	int				dbgGetInfoTimeCostMax				=	72;
const	int				dbgGetInfoTimeCostAvg				=	73;

///
///for every single playlist , record every single operation if profile is enabled
///
["freeze:write"] 
class  playlistDebug
{	
	bool														bEnableProfile;
	
	["cpp:const", "freeze:read"]	void						enableProfile( bool bEnable );
	
	///reset profile record , any previous record will be removed
	["cpp:const", "freeze:read"]	void						resetProfile( );
	
	["cpp:const", "freeze:read"]	OperationProfileS			getPlaylistProfile( );
	
	["cpp:const", "freeze:read"]	OperationProfileS			getItemProfile( );
	
	["cpp:const", "freeze:read"]	bool						isProfileEnable( );
};

class  StreamServiceDebug extends StreamSmithAdmin
{	
	///service debug information
	serviceDebugInfo				debugInfo;
	
	///registered event observers	
	//DebugEventObserverS				regsiteredObservers;
	DebugEventObserver				registeredObserver;//only one observer is supported
	
	OperationProfileS				opProfiles;
	
	void							registerDebugObserver( DebugEventObserver* observer ) throws InvalidParameter,ServerError;
	
};


};
};


#endif//__ZQ_TianShanIce_StreamDebugger_ICE__
