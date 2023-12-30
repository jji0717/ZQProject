#include "PathManagerImpl.h"
#include "getopt.h"
#include "Log.h"
#include "../pho/IpEdgePHO.h"
#include "../pho/Raid5sqrPHO.h"

extern "C"
{
#include <time.h>
#include <stdio.h>
}

BOOL WINAPI ConsoleHandler(DWORD event);
void feed(TianShanIce::AccreditedPath::PathAdminPrx& client);
void burn(TianShanIce::AccreditedPath::PathAdminPrx& client, int round, int mt=0);
bool bQuit = false;

void usage()
{
	printf("Usage: PathSvc [-e \"<endpoint>\"]\n");
	printf("       PathSvc -h\n");
	printf("PathSvc console mode server demo.\n");
	printf("options:\n");
	printf("\t-e       the endpoint to connect, default %s\n", DEFAULT_ENDPOINT_PathManager);
	printf("\t-b <n>   burn the database\n");
	printf("\t-f       feed the database\n");
	printf("\t-m <n>   max tickets for each allocation, default: 0-unlimited\n");

#ifdef WITH_ICESTORM
	printf("\t-t   the IceStorm endpoint to publish events\n");
#endif // WITH_ICESTORM
	printf("\t-h   display this help\n");
}

int main(int argc, char* argv[])
{
	int ch;
	std::string endpoint = DEFAULT_ENDPOINT_PathManager, epIceStorm = "default -h 192.168.80.49 -p 10000";

	bool bBurn= false, bFeed=false;
	int trace = ZQ::common::Log::L_ERROR;
	int round = 1000, mt =0;

	while((ch = getopt(argc, argv, "hb:fd:e:t:m:")) != EOF)
	{
		switch (ch)
		{
		case '?':
		case 'h':
			usage();
			exit(0);

		case 'e':
			endpoint = optarg;
			break;

		case 'd':
			trace = atoi(optarg);
			break;

		case 't':
			epIceStorm = optarg;
			break;

		case 'b':
			round = atoi(optarg);
			break;

		case 'm':
			mt = atoi(optarg);
			break;

		case 'f':
			bFeed = true;
			break;

		default:
			printf("Error: unknown option %c specified\n", ch);
			exit(1);
		}
	}

	int i =0;
	Ice::CommunicatorPtr ic = Ice::initialize(i, NULL);

    printf("Connect to " ADAPTER_NAME_PathManager " at \"%s\"\n", endpoint.c_str());
	
	trace = (trace>7) ? 7 : ((trace<0) ? 0 : trace);
	glog.setVerbosity(trace);

	TianShanIce::AccreditedPath::PathAdminPrx client = TianShanIce::AccreditedPath::PathAdminPrx::checkedCast(ic->stringToProxy(std::string(ADAPTER_NAME_PathManager) + ":" + endpoint));
	{
		printf("\nStorageLink types: ");
		::TianShanIce::StrValues linktypes = client->listSupportedStorageLinkTypes();
		for (::TianShanIce::StrValues::iterator it =linktypes.begin(); it < linktypes.end(); it++)
		{
			printf("%s; ", it->c_str());
			::TianShanIce::PDSchema schema =client->getStorageLinkSchema(*it);
		}
		printf("\nStreamLink types: ");
		linktypes = client->listSupportedStreamLinkTypes();
		for (it =linktypes.begin(); it < linktypes.end(); it++)
		{
			printf("%s; ", it->c_str());
			::TianShanIce::PDSchema schema =client->getStreamLinkSchema(*it);
		}
		printf("\n");
	}

	if (bFeed)
		feed(client);
		
	burn(client, round, mt);

	printf("burning " ADAPTER_NAME_PathManager " finished");
	ic->destroy();

	return 0;
}

BOOL WINAPI ConsoleHandler(DWORD CEvent)
{
    switch(CEvent)
    {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
		bQuit = true;
        break;
    }
    return TRUE;
}

void feed(TianShanIce::AccreditedPath::PathAdminPrx& client)
{
	char netid1[16], netid2[16];
	int i;
	
	if (!client)
	{
		printf("null client to feed\n");
		return;
	}
	
	glog(ZQ::common::Log::L_INFO, LOGFMT("feed data"));
	
	for (i =0; i< 10; i++)
	{
		char desc[32];
		sprintf(netid1, "%05d", i+10000);
		sprintf(desc, "ContentStore %s", netid1);
		client->updateStorage(netid1, "MediaCluster", "ContentStore:default -p 10000", desc);
		sprintf(desc, "Spigot %s", netid1);
		client->updateStreamer(netid1, "Spigot", "StreamSmith:default -h 10.15.10.250 -p 10000", desc);
		sprintf(desc, "ServiceGroup %d", i);
		client->updateServiceGroup(i, desc);
	}
	
	glog(ZQ::common::Log::L_INFO, LOGFMT("link data"));
	
	::TianShanIce::ValueMap linkPd;
	::TianShanIce::Variant  val;
	val.bRange = false;
	val.type   = ::TianShanIce::vtLongs;
	val.lints.push_back(10*1000);
	linkPd["TotalBandwidth"] = val;

	for (i=0; i<20; i++)
	{
		try
		{
			sprintf(netid1, "%05d", i%7 +10000);
			sprintf(netid2, "%05d", i%3 +10000);
			::TianShanIce::AccreditedPath::StorageLinkPrx link = client->linkStorage(netid1, netid2, STORLINK_TYPE_RAID5SQR, linkPd);
			//				std::string id = ic->proxyToString(link);
			::TianShanIce::AccreditedPath::Streamer st = link->getStreamerInfo();
		} 
		catch(const ::TianShanIce::ServerError& ex)
		{
			glog(ZQ::common::Log::L_ERROR, LOGFMT("NULL link"));
			break;;
		}

		try
		{
			sprintf(netid1, "%05d", (9-i)%10 +10000);
			::TianShanIce::AccreditedPath::StreamLinkPrx link = client->linkStreamer(1, netid1, STRMLINK_TYPE_IPEDGE_IP, linkPd);
			//				std::string id = ic->proxyToString(link);
			::TianShanIce::AccreditedPath::Streamer st = link->getStreamerInfo();
		} 
		catch(const ::TianShanIce::ServerError& ex)
		{
			glog(ZQ::common::Log::L_ERROR, LOGFMT("NULL link"));
			break;;
		}
	}
	
	glog(ZQ::common::Log::L_INFO, LOGFMT("end of feeding"));
}

void burn(TianShanIce::AccreditedPath::PathAdminPrx& client, int round, int mt)
{
	char netid1[16];
	int i;

	if (!client)
	{
		printf("null client to burn\n");
		return;
	}

	if (mt<0)
		mt =0;

	glog(ZQ::common::Log::L_INFO, LOGFMT("burn path reservation"));

	::TianShanIce::AccreditedPath::Storages stores = client->listStorages();
	int size = stores.size();
	stores.clear();
	
	::TianShanIce::AccreditedPath::Streamers streamers = client->listStreamers();
	size = streamers.size();
	streamers.clear();
	
	::TianShanIce::Variant val;
	val.type = ::TianShanIce::vtStrings;
	val.bRange = false;
	val.strs.push_back("test value");
	
	{
		sprintf(netid1, "%05d", 3 +10000);
		::TianShanIce::ValueMap vals = client->getStoragePrivateData(netid1);
		
		::TianShanIce::ValueMap::iterator it = vals.find("content00001");
		if (vals.end() != it)
			val = it->second;
	}

	float avarageLatency=0, avarageTicket=0;
	int minlatency=3600*1000, maxlatency=0, maxTicket=0;

	::TianShanIce::Weiwoo::SessionPrx sess=NULL;
	
	for(int jjj=0; jjj<round; jjj++, Sleep((jjj%3+1)*200))
	{
		for (i=0; i<5; i++)
		{
			sprintf(netid1, "%05d", i +10000);
			val.type = ::TianShanIce::vtStrings;
			val.bRange = false;
			val.strs.push_back(netid1);
			
			glog(ZQ::common::Log::L_WARNING, LOGFMT(">> find path from store %s to service group %d at BW %d"), netid1, 1, 10000);
			
			try {
				__int64 stamp = ZQTianShan::now();
				TianShanIce::AccreditedPath::PathTickets tickets = client->reservePathsByStorage(val, 1, 500, 0, mt, 3800, sess);
				
				int latency =(int) (ZQTianShan::now() -stamp);
				int ticketcount = tickets.size();
				glog(ZQ::common::Log::L_WARNING, LOGFMT(">> !! %d tickets - %dms"), ticketcount, latency);
				
				minlatency = (minlatency >latency) ? latency : minlatency;
				maxlatency = (maxlatency >latency) ? maxlatency : latency;
				avarageLatency = ((float) avarageLatency * (jjj *5 + i) + latency) / (jjj *5 + i +1);
				maxTicket  = (maxTicket >ticketcount) ? maxTicket : ticketcount;
				avarageTicket = ((float) avarageTicket * (jjj *5 + i) + ticketcount) / (jjj *5 + i +1);
				
				for (::TianShanIce::AccreditedPath::PathTickets::iterator itTicket =tickets.begin(); itTicket < tickets.end(); itTicket++)
				{
					::TianShanIce::AccreditedPath::PathTicketPrx ticket = (*itTicket);
					
					if (!ticket)
						continue;
					
					TianShanIce::ValueMap ticketPD = ticket->getPrivateData();
					::TianShanIce::Variant& field = ::ZQTianShan::PDField(ticketPD, PD_FIELD(PathTicket, bandwidth));
					ASSETVARIANT(field, ::TianShanIce::vtLongs);
					::Ice::Long bw = field.lints[0];
					
					::TianShanIce::AccreditedPath::StorageLinkPrx storelink = ticket->getStorageLink();
					::TianShanIce::AccreditedPath::StreamLinkPrx streamlink = ticket->getStreamLink();
					if (!storelink || !streamlink)
					{
						glog(ZQ::common::Log::L_ERROR, LOGFMT("Null link in the path"));
						continue;
					}
					
					::TianShanIce::AccreditedPath::Storage store = storelink->getStorageInfo();
					::TianShanIce::AccreditedPath::Streamer streamer = streamlink->getStreamerInfo();
					::TianShanIce::AccreditedPath::ServiceGroup sgroup = streamlink->getServiceGroupInfo();
					
					glog(ZQ::common::Log::L_WARNING, LOGFMT(">> Path allocated: sto[%s]->strm[%s]->svcgrp[%d]: cost=%d, lLeft=%dms; bw=%d"),
						store.netId.c_str(), streamer.netId.c_str(), sgroup.id, ticket->getCost(), ticket->getLeaseLeft(), bw);
				}
			}
			catch(...)
			{
				glog(ZQ::common::Log::L_ERROR, LOGFMT("exception occurs, yield for 2sec"));
				::Sleep(2000);
			}
		}
		
	}

	glog(ZQ::common::Log::L_INFO, LOGFMT("end of burning"));
	printf("end of burning, %d rounds; tickets: max=%d, avg=%.2f; latency: min=%dms, max=%dms, avg=%.2fms\n",
		   round*5, maxTicket, avarageTicket, minlatency, maxlatency, avarageLatency);
}
