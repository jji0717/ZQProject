// DummySS.cpp : Defines the entry point for the console application.
//

#include "StdAfx.h"
#include "DummyStreamSmith.h"

#ifdef ZQ_OS_MSWIN
	#include <conio.h>
	#include<direct.h>
	#include "InitInfo.h"
#else
	#include <stdio.h>
	#include <unistd.h>
	#include <stdlib.h>
	#include <termios.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include <termios.h> 
	#include <unistd.h>   
	#include "InitInfoLinux.h"
#endif

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <getopt.h>

// #if ICE_INT_VERSION/100 < 303
// 	#include <ice\IdentityUtil.h>
// #endif

#include <FileLog.h>

#ifdef max
	#undef max
#endif//max

#ifdef min
	#undef min
#endif//min

#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))

int	pauseMax = 2000;
int pauseMin = 500;

class IceLogger:public Ice::Logger
{
public:
	IceLogger(ZQ::common::Log& log):_logger(log)
	{		
	}
	~IceLogger()
	{	
	}
	void print(const ::std::string& message)
	{
		ZQ::common::MutexGuard gd(_locker);
		_logger(ZQ::common::Log::L_DEBUG,message.c_str());
	}
	void trace(const ::std::string& category, const ::std::string& message)
	{
		ZQ::common::MutexGuard gd(_locker);
		_logger(ZQ::common::Log::L_DEBUG,"catagory %s,message %s",category.c_str(),message.c_str());
	}
	void warning(const ::std::string& message)
	{
		ZQ::common::MutexGuard gd(_locker);
		_logger(ZQ::common::Log::L_WARNING,message.c_str());
		_logger.flush ();
	}
	void error(const ::std::string& message)
	{
		ZQ::common::MutexGuard gd(_locker);
		_logger(ZQ::common::Log::L_ERROR,message.c_str());
		_logger.flush ();
	}
	virtual ::std::string getPrefix() {return "";}
	virtual ::Ice::LoggerPtr cloneWithPrefix(const ::std::string&){return NULL;}
private:
	ZQ::common::Log& _logger;
	ZQ::common::Mutex _locker;
};

DummyService*	_serviceInstance;
//ZQ::common::Log* ZQ::common::pGlog=NULL;

std::string getWorkingDir( )
{
	//get current dir
	/*char	szCurrrent[1024];
	memset(szCurrrent, 0 , sizeof(szCurrrent));
#ifdef ZQ_OS_MSWIN
	ZeroMemory(szCurrrent,sizeof(szCurrrent));
	GetModuleFileNameA(NULL,szCurrrent,sizeof(szCurrrent)-1);
#else
	
#endif
	int iCount = strlen(szCurrrent)-1;
	while (szCurrrent[iCount]!='\\') 
		iCount--;
	szCurrrent[iCount+1] = '\0';*/
	char	szCurrrent[1024];
	memset(szCurrrent, 0 , sizeof(szCurrrent));
	if(getcwd(szCurrrent, sizeof(szCurrrent)) == NULL)
		return "";
	printf("%s\n", szCurrrent);
	int iCount = strlen(szCurrrent) ;
	szCurrrent[iCount] = '\\';
	szCurrrent[iCount+1] = '\0';
	return std::string(szCurrrent);
}
void initIceProps( Ice::PropertiesPtr proper )
{
	proper->setProperty("Dummy.ThreadPool.Size","80");
	proper->setProperty("Dummy.Ice.Trace.Network","1");
	proper->setProperty("Dummy.Ice.Trace.Protocol", "0");
	proper->setProperty("Dummy.Ice.Trace.Retry", "1");
	proper->setProperty("Dummy.Ice.Trace.Slicing", "0");
	proper->setProperty("Dummy.Ice.Warn.Connections", "1");
	proper->setProperty("Dummy.Freeze.Warn.Deadlocks", "1");
	proper->setProperty("Dummy.Freeze.Trace.Map", "0");
	proper->setProperty("Dummy.Freeze.Trace.DbEnv", "2");
	proper->setProperty("Dummy.Freeze.Trace.Evictor", "0");
	proper->setProperty("Ice.ThreadPool.Server.Size","60");
}

#ifdef  ZQ_OS_LINUX
	int kbhit(void)
	{
		struct termios oldt, newt;
		int ch;
		int oldf;

		tcgetattr(STDIN_FILENO, &oldt);
		newt = oldt;
		newt.c_lflag &= ~(ICANON | ECHO);
		tcsetattr(STDIN_FILENO, TCSANOW, &newt);
		oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
		fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

		ch = getchar();

		tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
		fcntl(STDIN_FILENO, F_SETFL, oldf);

		if(ch != EOF)
		{
			ungetc(ch, stdin);
			return 1;
		}
		return 0;
	}

	int getch (void){   
		int ch;    struct termios oldt, newt;// get terminal input's attribute   
		tcgetattr(STDIN_FILENO, &oldt);    
		newt = oldt;    //set termios' local mode    
		newt.c_lflag &= ~(ECHO|ICANON);    
		tcsetattr(STDIN_FILENO, TCSANOW, &newt);    
		ch = getchar();    //recover terminal's attribute    
		tcsetattr(STDIN_FILENO, TCSANOW, &oldt);      
		return ch;
	}
#endif

int main(int argc, char* argv[])
{

	std::string strRoot = getWorkingDir();
	std::string strLogPath = strRoot + "..\\logs\\dummyssice.log";	
	std::string strDbPath = strRoot + "..\\data\\";
	std::string replicaSubscriberEndpoint;
	std::string bindEndpoint = "default -p 21000";
	std::string netId = "DummyNetId";
	std::string eventChannelEndpoint = "";
	std::vector<std::string> streamers;
	
	if( argc < 2 )
	{
		pauseMin	= 10;
		pauseMax	= 100;
		streamers.push_back("STREAMER");
	}
	else
	{
#ifdef ZQ_OS_MSWIN
		InitInfo info;
		if(!info.init(argv[1]))
		{
			printf("failed to open conf file at [%s]\n",argv[1]);
			return -1;
		}
		info.setCurrent("general");
		info.getValue("netId",netId);
		info.getValue("endpoint",bindEndpoint);
		info.getValue("dbPath",strDbPath);
		info.getValue("logPath",strLogPath);
		info.getValue("pauseMin",pauseMin);
		info.getValue("pauseMax",pauseMax);
		info.getValue("replicaSubscriberEndpoint",replicaSubscriberEndpoint);
		info.getValue("eventChannel",eventChannelEndpoint);
		info.setCurrent("streamers");
		int tmp = 0;
		char szTmp[128];
		while(true)
		{
			sprintf(szTmp,"streamer%d",tmp++);
			std::string strStreamer;
			info.getValue(szTmp,strStreamer);
			if( strStreamer.empty())
				break;
			streamers.push_back( strStreamer);
		}
#else
		InitInfoLinux info;
		if(!info.initConfig(argv[1]))
		{
			printf("failed to open conf file at [%s]\n",argv[1]);
			return -1;
		}
		std::string strPauseMin, strPauseMax;
		info.getValue("netId",netId);
		info.getValue("endpoint",bindEndpoint);
		info.getValue("dbPath",strDbPath);
		info.getValue("logPath",strLogPath);
		info.getValue("pauseMin",strPauseMin);
		info.getValue("pauseMax",strPauseMax);
		info.getValue("replicaSubscriberEndpoint",replicaSubscriberEndpoint);
		info.getValue("eventChannel",eventChannelEndpoint);
		info.getStreamers(streamers);
		pauseMin = atoi(strPauseMin.c_str());
		pauseMax = atoi(strPauseMax.c_str());
#endif
	}


	ZQ::common::FileLog* dummySSLog = new ZQ::common::FileLog( strLogPath.c_str(), 7);
	ZQ::common::setGlogger(dummySSLog);
	IceUtil::Handle<IceLogger> m_iceLogger=new IceLogger(* ZQ::common::getGlogger());
	int i=0;
	::Ice::InitializationData initData;
	initData.properties = Ice::createProperties(i, NULL);
	initData.logger = m_iceLogger;
	#if  ICE_INT_VERSION / 100 >= 306
		::Ice::CommunicatorPtr ic = ::Ice::initialize(i,NULL,initData);	
	#else
		::Ice::CommunicatorPtr ic=::Ice::initializeWithLogger(i,NULL,m_iceLogger);
	#endif
	initIceProps( ic->getProperties() );

	Ice::ObjectAdapterPtr  objAdapter;

	try
	{	
		objAdapter=ic->createObjectAdapterWithEndpoints( "Dummy" , bindEndpoint );
		printf("bind service at [%s]\n", bindEndpoint.c_str());
	}
	catch ( const Ice::Exception& ex ){
		printf("caught exception when create object adapter at [%s]:[%s]\n",bindEndpoint.c_str() , ex.ice_name().c_str() );
		return -1;			 
	}

	_serviceInstance = new DummyService( objAdapter, strDbPath, netId, streamers );
	_serviceInstance->listenerEndpoint = replicaSubscriberEndpoint;
	_serviceInstance->updateInterval = 60*1000;
	TianShanIce::Streamer::StreamServicePtr _service;
	_service=_serviceInstance;

	if( !eventChannelEndpoint.empty() ) {
		_serviceInstance->connectToEventChannel(eventChannelEndpoint);
	}
	
	ReplicaUpdater  replicaUpdater(*_serviceInstance);
	if( !replicaSubscriberEndpoint.empty() )
	{
		printf("use replica subscriber endpoint[%s]\n",replicaSubscriberEndpoint.c_str());
		replicaUpdater.start();
	}
	printf("service attr: pauseMin[%d], pauseMax[%d], netId[%s]\n",
		pauseMin,pauseMax,netId.c_str());

	
	objAdapter->add(_service,ic->stringToIdentity("DummySS"));	
	
	objAdapter->activate();
	printf("\npress 'q' to exit\n");
	do 
	{
		if(kbhit()&&getch()=='q')
			break;
		SYS::sleep(1000);
	} while(1);
	if( !replicaSubscriberEndpoint.empty() )
		replicaUpdater.stop();

	objAdapter->deactivate();
	_service=NULL;
	ic->destroy();
	if (dummySSLog != NULL)
	{
		delete dummySSLog;
		dummySSLog = NULL;
	}
	return 0;
}

