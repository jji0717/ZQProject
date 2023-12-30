// IceTimeOut.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include "TimeoutCall.h"




CRITICAL_SECTION outputCS;

void output( const char* fmt , ... )
{
	va_list arglist;
	va_start( arglist, fmt );
	EnterCriticalSection(&outputCS);
	_vprintf_s_l(fmt,NULL,arglist);
	printf("\n");
	LeaveCriticalSection(&outputCS);
}

test::timeoutPrx changeTimeout( test::timeoutPrx prx,Ice::Int timeoutValue)
{
	return test::timeoutPrx::uncheckedCast( prx->ice_timeout( timeoutValue ) );
}

class SyncCallThread
{
public:
	SyncCallThread( Ice::Int clientId , Ice::Int serverPauseInterval , test::timeoutPrx prx )
		:mClientId(clientId),
		mServerPauseInterval(serverPauseInterval),
		mPrx(prx),
		mThreadHandle(0)
	{
	}
	SyncCallThread::~SyncCallThread( )
	{
		if( mThreadHandle)
			CloseHandle(mThreadHandle);
	}
	void run( )
	{
		try
		{
			//output("[%s] CLIENT[%02d] begin call with PauseInterval[%04d]", IceUtil::Time::now().toDateTime().c_str(), mClientId, mServerPauseInterval);
			int i = mPrx->call(mClientId,mServerPauseInterval);
			output("[%s] CLIENT[%02d] end call with PauseInterval[%04d],result [%d]", IceUtil::Time::now().toDateTime().c_str(), mClientId, mServerPauseInterval, i);
		}
		catch( const Ice::Exception& ex )
		{
			output("[%s] CLIENT[%02d] with PauseInterval[%04d] caught exception:[%s]", 
				IceUtil::Time::now().toDateTime().c_str(), mClientId, mServerPauseInterval,
				ex.ice_name().c_str() );
		}
	}
	void start( )
	{
		mThreadHandle = CreateThread(NULL,0,ThreadProc,this,0,0);		
	}
	void final( )
	{
		delete this;
	}
	static DWORD WINAPI ThreadProc( LPVOID lpPara)
	{
		SyncCallThread* pThis = (SyncCallThread*)lpPara;
		pThis->run();
		pThis->final();
		return 0;
	}
private:
	Ice::Int			mClientId;
	Ice::Int			mServerPauseInterval;
	test::timeoutPrx	mPrx;
private:
	HANDLE				mThreadHandle;
};

class AsyncCallback : public test::AMI_timeout_call
{
public:
	AsyncCallback( const Ice::Int& clientId , const Ice::Int& serverPauseInterval )
		:mClientId(clientId),
		mServerPauseInterval(serverPauseInterval)
	{
	}
	~AsyncCallback( )
	{
	}
public:
	virtual void ice_response( int i) 
	{
		output("[%s] CLIENT[%02d] ASYNC end call with PauseInterval[%04d],result [%d]", IceUtil::Time::now().toDateTime().c_str(), mClientId, mServerPauseInterval,i);
	}
	virtual void ice_exception(const ::Ice::Exception& ex) 
	{
		output("[%s] CLIENT[%02d] ASYNC with PauseInterval[%04d] caught exception:[%s]", 
			IceUtil::Time::now().toDateTime().c_str(), mClientId, mServerPauseInterval,
			ex.ice_name().c_str() );
	}
private:
	Ice::Int		mClientId;
	Ice::Int		mServerPauseInterval;
};

class TimeoutCallTest : public Ice::Application
{
public:
	TimeoutCallTest(){}
	~TimeoutCallTest(){}

	int test2( int count )
	{
		test::timeoutPrx prx = NULL;
		Ice::PropertiesPtr prop =  communicator()->getProperties();
		prop->setProperty( "Ice.ThreadPool.Client.Size" , "10" );
		prop->setProperty( "Ice.ThreadPool.Server.Size" , "10" );
		prop->setProperty( "Ice.Override.Timeout" , "1000" );
		try
		{
			prx = test::timeoutPrx::uncheckedCast( communicator()->stringToProxy("TimeoutServer:default -p 12345 -t 10000") );
			output("connected to [%s]",communicator()->proxyToString(prx).c_str() );
		}
		catch(const Ice::Exception& ex)
		{	
			output("failed to connect to server: [%s]",ex.ice_name().c_str() );
			return -1;
		}
		Ice::Int clientId = 1;

		for ( int i = 0 ; i < count ; i++ )
		{
			//output("[%s] CLIENT[%02d] SYNC begin call with PauseInterval[%04d]", IceUtil::Time::now().toDateTime().c_str(), clientId,  i % 200);
			SyncCallThread( clientId, i % 50+10, prx ).run();			
			clientId ++;
		}
		return 0;
	}

	virtual int run(int argc, _TCHAR* argv[]) 
	{
		int count = 200;
		if( argc > 1)
		{
			count = atoi(argv[1]);
		}
		count = count < 100 ? 100 : count;
		return test2(count);	

		test::timeoutPrx prx = NULL;
		Ice::PropertiesPtr prop =  communicator()->getProperties();
		prop->setProperty("Ice.ThreadPool.Client.Size","10");
		prop->setProperty("Ice.ThreadPool.Server.Size","10");
		prop->setProperty("Ice.Override.Timeout","1000");
		try
		{
			prx = test::timeoutPrx::uncheckedCast( communicator()->stringToProxy("TimeoutServer:default -p 12345 -t 10000") );
			output("connected to [%s]",communicator()->proxyToString(prx).c_str() );
		}
		catch(const Ice::Exception& ex)
		{	
			output("failed to connect to server: [%s]",ex.ice_name().c_str() );
			return -1;
		}
		Ice::Int clientId = 1;

		//////////////////////////////////////////////////////////////////////////
		//sync call
		output("SYNC call test");
		prx = changeTimeout( prx , 10000 );
		output("changed timeout value of the proxy:[%s]",communicator()->proxyToString(prx).c_str() );
		
		SyncCallThread( clientId, 9000, prx ).run();
		clientId++;
		SyncCallThread( clientId, 11000, prx ).run();
		clientId++;

		for( int i = 3 ; i < 13 ; i++ )
		{
			SyncCallThread* pSync =  new SyncCallThread( clientId, 6000 + (i-3)*1000 , prx );
			clientId++;
			pSync->start();
		}
		
		Sleep(1 * 1000);

		output("ASYNC call test");

		output("[%s] CLIENT[%02d] ASYNC begin call with PauseInterval[%04d]", IceUtil::Time::now().toDateTime().c_str(), clientId, 9000);
		prx->call_async( new AsyncCallback( clientId , 9000 ), clientId , 9000 );
		clientId++;
		
		output("[%s] CLIENT[%02d] ASYNC begin call with PauseInterval[%04d]", IceUtil::Time::now().toDateTime().c_str(), clientId, 11000);
		prx->call_async( new AsyncCallback( clientId , 11000 ), clientId , 11000 );
		clientId++;
		Sleep( 10 * 1000);
		
		for( int i = 0 ; i< 10000 ; i ++ )
		{
			Ice::Int pauseInterval = 1000 + (i % 10) * 1000;
			output("[%s] CLIENT[%02d] ASYNC begin call with PauseInterval[%04d]", IceUtil::Time::now().toDateTime().c_str(), clientId, pauseInterval);
			prx->call_async( new AsyncCallback( clientId , pauseInterval ), clientId , pauseInterval );
			clientId++;
		}
		Sleep(5 * 1000);

		return 0;
	}
};


class A : public Ice::LocalObject
{
public:
	A(){}
	~A(){};
};
typedef IceUtil::Handle<A> APtr;
int _tmain(int argc, _TCHAR* argv[])
{
	APtr a = new A();
	APtr b = new A();
	a =b;
	InitializeCriticalSection(&outputCS);
	TimeoutCallTest app;
	int ret = app.main(argc, argv);
	DeleteCriticalSection(&outputCS);
	return ret;
}