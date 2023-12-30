// TimeOutServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include "TimeoutI.h"


class ServerI : public Ice::Application
{
public:
	ServerI(){}
	~ServerI(){};
	virtual int run(int, _TCHAR*[]) 
	{
		Ice::PropertiesPtr prop =  communicator()->getProperties();
		prop->setProperty("Ice.ThreadPool.Client.Size","10");
		prop->setProperty("Ice.ThreadPool.Server.Size","100");
		prop->setProperty("Ice.Trace.Network","1");
		prop->setProperty("Ice.Trace.Retry","1");
		prop->setProperty("Ice.Warn.Connections","1");
		prop->setProperty("Server.ThreadPool.Size","4");

		_timeoutServer = new TimeoutI( 100 * 1000 );
		_adapter = communicator()->createObjectAdapterWithEndpoints("Server","default -p 12345");
		Ice::Identity id;
		id.name = "TimeoutServer";
		id.category = "";
		_adapter->add( _timeoutServer , id );
		_adapter->activate();
		printf("\'TimeoutServer:default -p 12345\'   started\n");
		communicator()->waitForShutdown( );
		
		return 0;
	}
private:
	Ice::ObjectAdapterPtr	_adapter;
	TimeoutI::Ptr			_timeoutServer;
};


int _tmain(int argc, _TCHAR* argv[])
{
	ServerI app;
	return app.main(argc, argv);
}

