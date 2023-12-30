// NSSTestClient.cpp : Defines the entry point for the console application.
//

#include <winsock2.h>
#include "NSSClient.h"
#include "PathManagerImpl.h"
#include "PathSvcEnv.h"
#include "FileLog.h"
#include "NativeThreadPool.h"
#include "Guid.h"
#include <iostream>
using namespace std;

int main(int argc, char* argv[])
{
	int status = 0;
	Ice::CommunicatorPtr ic;
	try {
		Ice::PropertiesPtr props = Ice::createProperties();
		props->setProperty("Ice.Trace.Network", "1");
		props->setProperty("Ice.ThreadPool.Server.Size", "5");
		props->setProperty("Ice.ThreadPool.Client.Size", "5");

		ic = Ice::initializeWithProperties(argc, argv, props);
		Ice::ObjectPrx base = ic->stringToProxy("NSS:tcp -h 192.168.81.103 -p 5735");

		//Ice::ObjectAdapterPtr adapter = ic->createObjectAdapterWithEndpoints("Pathticket", "default -p 3333");


		::TianShanIce::Streamer::NGODStreamServer::NGODStreamServicePrx ngodStreamServicePrx 
			= ::TianShanIce::Streamer::NGODStreamServer::NGODStreamServicePrx::checkedCast(base);

		if (!ngodStreamServicePrx)
			throw "Invalid proxy";

		ZQ::common::FileLog _log("c:\\testlog.txt", ZQ::common::Log::L_DEBUG, 10240000);
		ZQ::common::NativeThreadPool _pool(5);
		::ZQTianShan::AccreditedPath::PathSvcEnv _env(_log, _pool, ic, "tcp -h 192.168.81.103 -p 5738", "C:\\TianShan\\data"); 
		_env._adapter->activate();

		::ZQTianShan::AccreditedPath::ADPathTicketImpl::Ptr ticket = new ::ZQTianShan::AccreditedPath::ADPathTicketImpl(_env);
		char tmpGuid[128];
		tmpGuid[127] = 0;
		::ZQ::common::Guid _guid;
		_guid.create();
		_guid.toString(tmpGuid, 127);
		ticket->ident.name = tmpGuid;
		ticket->ident.category = DBFILENAME_PathTicket;
		_env._ePathTicket->add(ticket, ticket->ident);

		::Ice::ObjectPrx objPrx = _env._adapter->createProxy((const ::Ice::Identity) ticket->ident);
		

		::TianShanIce::Transport::PathTicketPrx ticketPrx = 
			::TianShanIce::Transport::PathTicketPrx::checkedCast(
				objPrx, "");

		cout << _env._communicator->proxyToString(ticketPrx) << endl;

		::std::string tmpProxStr = ic->proxyToString(objPrx);
		::TianShanIce::Streamer::StreamPrx streamPrx = 
			ngodStreamServicePrx->createStream(ticketPrx); 
	} catch (const Ice::Exception& ex) {
		cerr << ex << endl;
		status = 1;
	} catch (const char* msg) {
		cerr << msg << endl;
		status = 1;
	} catch (const ::TianShanIce::ServerError& ex){
		cerr << "1." << ex << endl;
		return -1;
	} catch (const ::TianShanIce::InvalidParameter& ex){
		cerr << "2." << ex << endl;
		return -1;
	} catch (const ::TianShanIce::InvalidStateOfArt& ex){
		cerr << "3." <<  ex << endl;
		return -1;
	} catch (const ::TianShanIce::BaseException &ex){
		cerr << ex << endl;
		return -1;
	} catch(std::exception &ex){
		cerr << ex.what() << endl;
		return -2;
	}

	if (ic)
		ic->destroy();
	system("pause");
	return status;
}
