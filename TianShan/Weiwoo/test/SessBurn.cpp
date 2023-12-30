#include "../../common/TianShanDefines.h"
#include "../WeiwooAdmin.h"
#include "../../Ice/TsStreamer.h"
#include "Log.h"
#include "getopt.h"

extern "C"
{
#include <time.h>
#include <stdio.h>
}

BOOL WINAPI ConsoleHandler(DWORD CEvent);
static bool bQuit = false;

#define DEFAULT_BURN_URI	"rtsp://hotel/testapp?item0=1222"
#define DEFAULT_DEST_ADDR	"224.1.1.1"
#define DEFAULT_DEST_PORT	10012
#define DEFAULT_SVCGRP		1

void usage()
{
	printf("Usage: SessBurn [-e <endpoint>] [-s <eventchannel endpoint>] [-b <round>]");
	printf("                [-g <servicegroup>] [-u <uri>] [-a <daddr>] [-p <dport>]\n");
	printf("       SessBurn -h\n");
	printf("Session burning tool.\n");
	printf("options:\n");
	printf("\t-e        the endpoint to connect to Weiwoo server\n");
	printf("\t-s        the endpoint to connect to Event Channel\n");
	printf("\t-b <n>    specify the round of burning\n");
	printf("\t-g <n>    specify destination service group, default: %d\n", DEFAULT_SVCGRP);
	printf("\t-u <uri>  specify the uri to order, default:\"%s\"\n", DEFAULT_BURN_URI);
	printf("\t-a <addr> specify destination address, default: %s\n", DEFAULT_DEST_ADDR);
	printf("\t-p <n>    specify destination port, default: %d\n", DEFAULT_DEST_PORT);
	printf("\t-h        display this help\n");
	printf("\n");
}

int main(int argc, char* argv[])
{
	int ch;
	std::string endpoint = DEFAULT_ENDPOINT_Weiwoo, epIceStorm = "default -h 192.168.80.49 -p 10000";

	bool bBurn= false, bFeed=false;
	int trace = ZQ::common::Log::L_DEBUG;
	int round = 1000, mt =0, intv=0, svcgrp =DEFAULT_SVCGRP, dport = DEFAULT_DEST_PORT;
	std::string uri2order = DEFAULT_BURN_URI, dAddr= DEFAULT_DEST_ADDR;

	while((ch = getopt(argc, argv, "hb:fd:e:t:i:g:u:p:a:")) != EOF)
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

		case 'i':
			intv = atoi(optarg);
			break;

		case 'g':
			svcgrp = atoi(optarg);
			break;

		case 'u':
			uri2order = optarg;
			break;

		case 'p':
			dport = atoi(optarg);
			break;

		case 'a':
			dAddr = optarg;
			break;

		default:
			printf("Error: unknown option %c specified\n", ch);
			exit(1);
		}
	}

	if (uri2order.empty())
		uri2order = DEFAULT_BURN_URI;

	int i =0;
	Ice::CommunicatorPtr ic = Ice::initialize(i, NULL);

	trace = (trace>7) ? 7 : ((trace<0) ? 0 : trace);
	glog.setVerbosity(trace);

    printf("Connect to " SERVICE_NAME_SessionManager " at \"%s\"\n", endpoint.c_str());
	TianShanIce::Weiwoo::SessionAdminPrx sessmgr = TianShanIce::Weiwoo::SessionAdminPrx::checkedCast(ic->stringToProxy(std::string(SERVICE_NAME_SessionManager ":") + endpoint));

	::TianShanIce::Weiwoo::Resource uri;

	uri.attr = TianShanIce::Weiwoo::raMandatoryNonNegotiable;
	uri.status = TianShanIce::Weiwoo::rsRequested;
	::TianShanIce::Variant val;
	val.bRange = false;
	val.type = TianShanIce::vtStrings;
	val.strs.push_back(uri2order);
	uri.resourceData["uri"] = val;
	val.strs.clear();


	float avarageLatency=0;
	int minlatency=3600*1000, maxlatency=0, maxTicket=0;

	for(i=0; !bQuit && i<round; i++)
	{
		__int64 stamp = ZQTianShan::now();
		
		try {
			::TianShanIce::Weiwoo::SessionPrx sess = sessmgr->createSession(uri);
			
			::std::string id = ic->proxyToString(sess);
			id = sess->getId();
			
			try {
				// set the service group id to 1
				::TianShanIce::ValueMap svcgrpdata;
				val.type = TianShanIce::vtInts;
				val.ints.clear();
				val.ints.push_back(1);
				svcgrpdata["id"] = val;
				val.ints.clear();
				sess->addResource(::TianShanIce::Weiwoo::rtServiceGroup, TianShanIce::Weiwoo::raMandatoryNonNegotiable, svcgrpdata);
				
				// set a bw for test
				::TianShanIce::ValueMap bwdata;
				val.type = TianShanIce::vtLongs;
				val.lints.clear();
				val.lints.push_back(4000000);
				bwdata["bandwidth"] = val;
				val.lints.clear();
				sess->addResource(::TianShanIce::Weiwoo::rtTsDownstreamBandwidth, TianShanIce::Weiwoo::raMandatoryNonNegotiable, bwdata);
				
				::TianShanIce::ValueMap ip;
				val.type = TianShanIce::vtStrings;
				val.strs.clear();
				val.strs.push_back(dAddr);
				ip["destAddr"] = val;
				val.strs.clear();
				val.type = TianShanIce::vtInts;
				val.ints.clear();
				val.ints.push_back(dport);
				ip["destPort"] = val;
				val.ints.clear();

				sess->addResource(::TianShanIce::Weiwoo::rtIP, TianShanIce::Weiwoo::raMandatoryNonNegotiable, ip);

				::TianShanIce::Weiwoo::ResourceMap resmap = sess->getReources();
				ZQTianShan::dumpResourceMap(resmap);
				printf("Session %s created\n", id.c_str());

				sess->provision();
				printf("Session %s provisioned\n", id.c_str());

// continue;
				sess->serve();
				printf("Session %s served\n", id.c_str());

				::TianShanIce::Streamer::StreamPrx stream = sess->getStream();
				stream->play();
				printf("Session %s stream start playing\n", id.c_str());

				sess->renew(30000);
			}
			catch(const ::TianShanIce::BaseException& e)
			{
				printf("TianShan exception caught: %s\n", e.message.c_str());
			}
			catch(const ::Ice::Exception& e)
			{
				printf("Ice exception caught: %s\n", e.ice_name().c_str());
			}
			catch(...)
			{
				printf("unknown exception caught\n");
			}
#if 0
			sess->destroy();
			printf("Session %s destroyed\n", id.c_str());
#endif
		}
		catch(const ::TianShanIce::BaseException& e)
		{
			printf("TianShan exception caught: %s\n", e.message.c_str());
		}
		catch(const ::Ice::Exception& e)
		{
			printf("Ice exception caught: %s\n", e.ice_name().c_str());
		}
		catch(...)
		{
			printf("unknown exception caught\n");
		}

		int latency =(int) (ZQTianShan::now() -stamp);

		minlatency = (minlatency >latency) ? latency : minlatency;
		maxlatency = (maxlatency >latency) ? maxlatency : latency;
		avarageLatency = ((float) avarageLatency * i + latency) / (i +1);

		if (intv >0) 
			::Sleep(intv);
	}

	printf("end of session burn, %d rounds; latency: min=%dms, max=%dms, avg=%.2fms\n",
		   round, minlatency, maxlatency, avarageLatency);

/*
	for(i=0; i<1000; i++)
	{
		ZQ::common::Guid id;
		id.create();
		char buf[100];
		id.toString(buf, sizeof(buf));
		printf("%s\t%s\n", ZQTianShan::Weiwoo::SessionImpl::generateSessionID().c_str(), buf);
	}

	TianShanIce::Weiwoo::BusinessAdminPrx client = TianShanIce::Weiwoo::BusinessAdminPrx::checkedCast(ic->stringToProxy("BusinessRouter:" DEFAULT_Weiwoo_Endpoint));
	for(i=0; i<10; i++)
	{
		char buf[16];
		sprintf(buf, "path%02d", i);
		client->mountApplication("test", buf, "nPVR");
	}
*/
	if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler,TRUE)==FALSE)
	{
		printf("Unable to install handler!                      \n");
		return -1;
	}

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

/*
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
		client->updateStreamer(netid1, "Spigot", "StreamSmith:default -p 10000", desc);
		sprintf(desc, "ServiceGroup %d", i);
		client->updateServiceGroup(i, desc);
	}
	
	glog(ZQ::common::Log::L_INFO, LOGFMT("link data"));
	
	::TianShanIce::ValueMap linkPd;
	::TianShanIce::Variant  val;
	val.bRange = false;
	val.type   = ::TianShanIce::vtLongs;
	val.lints.push_back(3000);
	linkPd["TotalBandwidth"] = val;

	for (i=0; i<20; i++)
	{
		try
		{
			sprintf(netid1, "%05d", i%7 +10000);
			sprintf(netid2, "%05d", i%3 +10000);
			::TianShanIce::AccreditedPath::StorageLinkPrx link = client->linkStorage(netid1, netid2, STORLINK_TYPE_RAID5SQL, linkPd);
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
*/