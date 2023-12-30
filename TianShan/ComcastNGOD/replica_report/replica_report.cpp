// replica_report.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <TianShanIce.h>

using namespace std;
using namespace TianShanIce;

struct Config {
	std::string		subscriberEndoint; //ReplicaSubscriber:tcp -h ip -p 11200
	//SEA80055-N0_SS_NC/Spigot00
	std::string		spigotId;//Spigot00
	std::string		nodeId;//SEA80055-N0_SS_NC	
};
Config conf;

void report( bool up) {
	try {
		int i = 0;
		Ice::CommunicatorPtr ic = Ice::initialize(i, 0);
		ReplicaSubscriberPrx sub =  ReplicaSubscriberPrx::uncheckedCast( ic->stringToProxy( conf.subscriberEndoint ) );
		if(!sub) {
			cerr<<"bad subscriber endpoint"<<endl;
			return;
		}
		Replica r;
		r.category = "Streamer";
		r.groupId = conf.nodeId;
		r.replicaId = conf.spigotId;
		r.replicaState = up ? stInService : stOutOfService;
		r.maxPrioritySeenInGroup = 0;
		r.priority	= 0;

		Replicas reps;reps.push_back(r);
		sub->updateReplica(reps);
		ic->destroy();
	} catch( const Ice::Exception& ex) {
		cerr <<"ice exception: " << ex.ice_name()<<endl;

	} catch( const std::exception& ex ) {
		cerr <<"std exception:" << ex.what()<<endl;
	}
}

int main(int argc, char* argv[])
{
	// test endpoint nodename spigotname
	if( argc < 4) {
		cerr<<"usage: "<<argv[0]<<" subscriber_endpoint nodename spigotname"<<endl;
		return -2;
	}
	conf.subscriberEndoint = argv[1];
	conf.nodeId = argv[2];
	conf.spigotId = argv[3];
	report( true );
	Sleep(500);
	report( false );
	return 0;
}

