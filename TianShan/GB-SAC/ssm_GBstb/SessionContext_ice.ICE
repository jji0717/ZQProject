#ifndef __TianShanS1_SessionContext_ICE__
#define __TianShanS1_SessionContext_ICE__

// build command
// $(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)\slice -I$(ZQProjsPath)\TianShan\ice --output-dir . SessionContext_ice.ICE  
// $(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)\slice -I$(ZQProjsPath)\TianShan\ice --index "TianShanS1::StreamIdx, TianShanS1::SessionContext, streamID" StreamIdx --output-dir . SessionContext_ice.ICE

// SessionContext_ice.h
// SessionContext_ice.cpp
// StreamIdx.h
// StreamIdx.cpp

#include <Ice/Identity.ice>
#include "TianShanIce.ICE"

module TianShanS1 {

const string PropKeyLastScaleChange = "prop#lsc";

	exception ErrorBase
	{
		string message;
	};

	struct SessionData 
	{

		Ice::Identity ident; // index, include session id
		string streamID; // index
		string streamPrxID;
		string purchasePrxID;
		string srvrSessID;
		string srvrSessPrxID;
		string rangePrefix;
		string url;
		int requestType; // 1 SeaChange, 2 TianShan.
		int announceSeq; // initial value is 1
		TianShanIce::Properties props;

	};

	dictionary <string, string> StringMap;
	
	["freeze:write"]
	class SessionContext 
	{

		idempotent SessionData getSessionData();
		// increace the announce sequence number and return the increaced value
		int					addAnnounceSeq();
		void				setRangePrefix(string rngPf);
		idempotent void		onTimer();
		idempotent bool		canSendScaleChange(string curSeq);
		["cpp:const","freeze:read"]		 string				getProperty(string key);
		
		void				updateProperty(string key , string value);

		Ice::Identity ident; // index, include session id
		string streamID; // index
		string streamPrxID;
		string purchasePrxID;
		string srvrSessID;
		string srvrSessPrxID;
		string rangePrefix;
		string url;
		int requestType; // 1 SeaChange, 2 TianShan.
		int announceSeq; // initial value is 1
		TianShanIce::Properties props;

	};
	
	sequence<SessionData> SessionDatas;

	["freeze:write"]
	class SessionView
	{
		// ident, the client's identity which will be used in getRange.
		int getAllContext(int iIdent, out int oIdent);
		// return SessionDatas between from an to, from starts from 1.
		["cpp:const", "freeze:read"] SessionDatas getRange(int from, int to, int ident)
			throws ErrorBase;
		void unregister(int iIdent)
			throws ErrorBase;
	};

};

#endif // __TianShanS1_SessionContext_ICE__
