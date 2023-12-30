#ifndef _SSM_NGOD_R2C1_ICE_
#define _SSM_NGOD_R2C1_ICE_

#include <Ice/Identity.ICE>

module NGODr2c1
{
	struct ctxData
	{
		Ice::Identity ident;
		string weiwooFullID;
		string onDemandID;
		string streamFullID;
		string streamShortID;
		string normalURL;
		string resourceURL;
		string connectID;
		string groupID;
		long expiration;
		int announceSeq;
	};

	class Context
	{
		Ice::Identity ident;
		string weiwooFullID;
		string onDemandID;
		string streamFullID;
		string streamShortID;
		string normalURL;
		string resourceURL;
		string connectID;
		string groupID;
		long expiration;
		int announceSeq;
		ctxData getState();
		void renew(long ttl);
		void increaseAnnSeq();
		void onTimer();
	};
};

#endif