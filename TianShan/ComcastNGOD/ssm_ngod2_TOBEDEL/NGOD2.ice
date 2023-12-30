#ifndef __TIANSHAN__NGOD2_ICE_INTERFACE__
#define __TIANSHAN__NGOD2_ICE_INTERFACE__

#include <Ice/Identity.ICE>
module NGOD2
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
		nonmutating ctxData getState();
		void renew(long ttl);
		void increaseAnnSeq();
		void onTimer();
	};
};

#endif//__TIANSHAN__NGOD2_ICE_INTERFACE__