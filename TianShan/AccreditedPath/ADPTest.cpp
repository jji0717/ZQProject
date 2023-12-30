#include "ADPIceImpl.h"
extern "C"
{
#include <stdio.h>
}

ZQTianShan::AccreditedPath::TicketHelper gTicketHelper;

#define FEED_DATA

void main()
{
	int i =0;
	Ice::CommunicatorPtr ic = Ice::initialize(i, NULL);
    Ice::ObjectAdapterPtr adapter = ic->createObjectAdapterWithEndpoints("AccreditedPath", "default -p 10000");
	TianShanIce::AccreditedPath::PathManagerPtr ADPaths = new ZQTianShan::AccreditedPath::AccreditedPathsImpl(adapter, gTicketHelper);
    adapter->add(ADPaths, Ice::stringToIdentity("PathManager"));

	TianShanIce::AccreditedPath::PathAdminPrx client = TianShanIce::AccreditedPath::PathAdminPrx::checkedCast(ic->stringToProxy("PathManager:default -p 10000"));

#ifdef FEED_DATA

	TRACE("feed data");

	for (i =0; i< 10; i++)
	{
		char desc[32], netid[16];
		sprintf(netid, "%05d", i+10000);
		sprintf(desc, "ContentStore %s", netid);
		client->updateStorage(i, "MediaCluster", netid, "ContentStore:default -p 10000", desc);
		sprintf(desc, "Spigot %s", netid);
		client->updateStreamer(i, "Spigot", netid, "StreamSmith:default -p 10000", desc);
		sprintf(desc, "ServiceGroup %d", i);
		client->updateServiceGroup(i, desc);
	}

	TRACE("link data");
	for (i=0; i<20; i++)
	{
		try
		{
			::TianShanIce::AccreditedPath::StorageLinkPrx link = client->linkStorage(i%7, i%3,"IP",3000);
			std::string id = ic->proxyToString(link);
			::TianShanIce::AccreditedPath::Streamer st = link->getStreamerInfo();
		} 
		catch(TianShanIce::AccreditedPath::IOException ex)
		{
			TRACE("NULL link");
			break;;
		}

		try
		{
			::TianShanIce::AccreditedPath::StreamLinkPrx link = client->linkStreamer((9-i)%10, 1,"IP", 3000);
			std::string id = ic->proxyToString(link);
			::TianShanIce::AccreditedPath::Streamer st = link->getStreamerInfo();
		} 
		catch(TianShanIce::AccreditedPath::IOException ex)
		{
			TRACE("NULL link");
			break;;
		}
	}
#endif //FEED_DATA

	::TianShanIce::AccreditedPath::Storages stores = client->listStorages();
	int size = stores.size();
	stores.clear();
	
	::TianShanIce::AccreditedPath::Streamers streamers = client->listStreamers();
	size = streamers.size();
	streamers.clear();

	::TianShanIce::VariantPtr val = new ::TianShanIce::Variant;
	val->type = ::TianShanIce::vtStrings;
	val->bRange = false;
	val->strs.push_back("test value");

	{
		::TianShanIce::ValueMap vals = client->getStoragePrivateData(3);
		
		::TianShanIce::ValueMap::iterator it = vals.find("content00001");
		if (vals.end() != it)
			val = it->second;
	}

	for(int jjj=0; jjj<1000; jjj++, Sleep((jjj%3+1)*200))
	for (i=0; i<5; i++)
	{
		::TianShanIce::VariantPtr val = new ::TianShanIce::Variant;
		val->type = ::TianShanIce::vtInts;
		val->bRange = false;
		val->ints.push_back(i);

		printf(">> find path from store %d to service group %d at BW %d:",i, 1, 10000);

		__int64 stamp = ZQTianShan::AccreditedPath::now();
		TianShanIce::AccreditedPath::PathTickets tickets = client->reservePathsByStorage(val, 1, 1000, 0, 0, 3800);
		printf(" %d tickets - %lldms\n", tickets.size(), ZQTianShan::AccreditedPath::now() -stamp);

		for (::TianShanIce::AccreditedPath::PathTickets::iterator itTicket =tickets.begin(); itTicket < tickets.end(); itTicket++)
		{
			::TianShanIce::AccreditedPath::PathTicketPrx ticket = (*itTicket);

			if (!ticket)
				continue;

			::Ice::Long bw = ticket->getBandwidth();

			::TianShanIce::AccreditedPath::StorageLinkPrx storelink = ticket->getStorageLink();
			::TianShanIce::AccreditedPath::StreamLinkPrx streamlink = ticket->getStreamLink();
			if (!storelink || !streamlink)
			{
				TRACE("Null link in the path");
				continue;
			}
			
			::TianShanIce::AccreditedPath::Storage store = storelink->getStorageInfo();
			::TianShanIce::AccreditedPath::Streamer streamer = streamlink->getStreamerInfo();
			::TianShanIce::AccreditedPath::ServiceGroup sgroup = streamlink->getServiceGroupInfo();

//			printf("  Path allocated: sto[%d]->strm[%d]->svcgrp[%d]: cost=%d, lLeft=%dms; bw=%d\n",
//				      store.id, streamer.id, sgroup.id, ticket->getCost(), ticket->getLeaseLeft(), bw);
		}
	}

	TRACE("end of program");
//	Sleep(5000);

	ic->destroy();
	Sleep(1000);
}
