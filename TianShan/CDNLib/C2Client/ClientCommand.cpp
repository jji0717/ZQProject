
#include <iostream>
#include <fstream>
#include <boost/bind.hpp>
#include "ClientCommand.h"
#include "SessionRecordAnalyzer.h"
#include <TimeUtil.h>

#ifdef ZQ_OS_LINUX
	#include <stdio.h>
	#include <readline/readline.h>
	#include <readline/history.h>
#endif //ZQ_OS_LINUX

StatisticsRunner::StatisticsRunner( ClientCommand& cc)
:mCC(cc)
{
	reset();
	mbQuit = true;
}

StatisticsRunner::~StatisticsRunner()
{
	stop( );
}

void StatisticsRunner::stop( )
{
	if( !mbQuit )
	{
		mbQuit = true;
		mSem.post();
		waitHandle( 10 * 1000 );
	}
}

void StatisticsRunner::reset( )
{
	lastBytes = 0;
	lastTime = 0;
	totalBytes = 0 ;
	totalTime = 0;
}
void StatisticsRunner::startCounting( HttpSessionFactoryPtr sessfac )
{
	{
		ZQ::common::MutexGuard gd(mMutex);
		mSessfac = sessfac;
		mbCouting = true;
		reset();
	}	
	mSem.post();
}

void StatisticsRunner::stopCouting( )
{
	{
		ZQ::common::MutexGuard gd(mMutex);
		mSessfac = NULL;
		mbCouting = false;
	}	
}

int StatisticsRunner::run()
{
	mbQuit = false;
	while ( !mbQuit )
	{
		mSem.timedWait( 1000 );//wait for 1 second
		if(mbQuit ) break;
		if(!mbCouting) continue;
		HttpSessionFactoryPtr sessfac = NULL;
		{
			ZQ::common::MutexGuard gd(mMutex);
			if(!mSessfac)
				continue;
			sessfac = mSessfac;
		}
		if( lastTime == 0 )
		{
			lastTime = ZQ::common::now();
			lastBytes = sessfac->getTotalBytes();
		}
		else
		{
			int64 curTime = ZQ::common::now();
			int64 deltaTime = curTime - lastTime;			
			totalTime += deltaTime;
			int64 curByte = sessfac->getTotalBytes();
			int64 deltaBytes = curByte - lastBytes;
			totalBytes += deltaBytes;
			if( totalTime <= 0 || deltaTime <= 0 || !mbCouting)
				continue;
			{
				char strNow[256];				
				mCC.output("[%s] instantRate[%lld]\tavgRate[%lld]",
					ZQ::common::TimeUtil::TimeToUTC( curTime, strNow, 255 , true ) , 
					deltaBytes * 8000 / deltaTime ,
					totalBytes * 8000 /totalTime );
			}
			lastBytes = curByte;
			lastTime = curTime;
		}
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////

ClientCommand::ClientCommand(void)
:mClientService(mNullogger),
mStatRunner(*this)
{
	mNullogger.setVerbosity(ZQ::common::Log::L_ERROR);
	initialize();
	mPromotion = "init>>";
	initCallback();
	mStatRunner.start();
}

ClientCommand::~ClientCommand(void)
{
	mStatRunner.stop();
}

void ClientCommand::initCallback()
{
	HttpSessionFactoryPtr sessFac = mClientService.getDialogfactory()->getSessionFactory();
	sessFac->registerEventHandler( boost::bind(&ClientCommand::onCycleStart,this) , 
									boost::bind(&ClientCommand::onCycleStop,this) );

}

static ZQ::common::Mutex outputMutex;

void ClientCommand::output( const char* fmt , ... )
{
	
	ZQ::common::MutexGuard gd(outputMutex);
	va_list args;
	va_start(args, fmt);
	vprintf(fmt,args);
	va_end(args);
	printf("\n");
}

void ClientCommand::output( const std::string& str , ... )
{
	va_list args;
	va_start(args, str );
	output( str.c_str() , args );
}

void ClientCommand::initialize()
{
	mClientService.startService();

	{
		OptDescription opt;
		opt.addOptions("get file from c2 server")					
			("hostip","10.15.10.50","peer host ip address","hostip", true)
			("hostport","12000","peer host port","hostport",true)			
			("file","","which file do you want to get","filename",true)
			("li","0.0.0.0","local bind ip","localip",false)
			("lp","0","local bind port","localport",false)
			("r","3750000","transfer bitrate","bitrate",false)
			("i","10000000000","ingress capacity","ic",false)
			("t","1000","time out","timeout",false)
			("d","-100","transfer delay","delay",false)
			("c","1","concurrency count","count",false)
			("f","file name list","file that contain a name list",false)
			("g","-","request range","range",false);
		mCmdRunner.regVerb( "getc2", opt , boost::bind( &ClientCommand::createC2Session, this , _1 ) );
	}
	{
		OptDescription opt;
		opt.addOptions("get file from http server such as apache")					
			("hostip","10.15.10.50","peer host ip address","hostip", true)
			("hostport","80","peer host port","hostport",true)			
			("file","","which file do you want to get","filename",true)
			("c","1","concurrency count","count",false)
			("url","/","url used to download file","url",false);
		mCmdRunner.regVerb( "get", opt , boost::bind( &ClientCommand::createHttp, this , _1 ) );
	}
	{
		OptDescription opt;
		opt.addOptions("result analyze");			
		mCmdRunner.regVerb( "show", opt , boost::bind( &ClientCommand::show, this , _1 ) );
	}
	{
		OptDescription opt;
		opt.addOptions("quit process");			
		mCmdRunner.regVerb( "quit", opt , boost::bind( &ClientCommand::quit, this , _1 ) );
	}
}
int ClientCommand::quit( OptResult& opts )
{
	return CMD_ERR_QUIT;
}
int ClientCommand::process( )
{	
	int ret = 0;
	std::string line;
	do 
	{
		
#ifdef ZQ_OS_LINUX
		char* newcmd = readline(mPromotion.c_str());
		if(!newcmd || !(*newcmd))
			continue;
		add_history(newcmd);
		line = newcmd;	
#else
		{
			ZQ::common::MutexGuard gd(outputMutex);
			printf(mPromotion.c_str());
		}
		std::getline( std::cin , line);
		if(std::cin.eof())
			break;
#endif//ZQ_OS

		ret = mCmdRunner.run( line );

	} while ( ret != CMD_ERR_QUIT);

	return 0;
}

void ClientCommand::onCycleStart( )
{
	mPromotion = "testing>>";
	//output("test started");
	HttpSessionFactoryPtr sessfac = mClientService.getDialogfactory()->getSessionFactory();
	if( sessfac )
	{
		sessfac->resetTotalBytes();
		mStatRunner.startCounting( sessfac );
	}
}

void ClientCommand::onCycleStop( )
{
	mStatRunner.stopCouting();	
	{
		ZQ::common::MutexGuard gd(outputMutex);
		mPromotion = "done>>";
		printf(mPromotion.c_str());
	}	
}

int ClientCommand::show( OptResult& opts )
{
	SessionDataRecorder& rec = mClientService.getDialogfactory()->getSessionFactory()->getRecorder();
	SessionRecordAnalyzer a(rec);
	a.show();
	return 0;
}

int ClientCommand::createHttp( OptResult& opts )
{
	int count = opts.as<int>("c");
	std::string ip = opts.as<std::string>("hostip");
	std::string port = opts.as<std::string>("hostport");
	std::string url = std::string("/")+opts.as<std::string>("file");
	std::string tmpUrl = opts.as<std::string>("url");
	if( tmpUrl != "/")
		url = tmpUrl;

	for(int i = 0 ;i < count ;i ++ )
	{
		ZQ::DataPostHouse::ASocketPtr s = mClientService.connect( ip , port );
		if(!s)
		{
			output("failed to connect to %s:%s",ip.c_str() , port.c_str() );
			return -1;
		}
		HttpMessage msg;
		msg.updateMethod("get");
		msg.updateProtocol("http/1.1");
		msg.updateUrl(url);		
		msg.updateHeader("User-Agent","Mozilla/5.0 (Windows; U; Windows NT 5.1; zh-CN; rv:1.9.2.10) Gecko/20100914 Firefox/3.6.10 GTB7.1");
		msg.updateHeader("Accept","text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
		msg.updateHeader("Connection","keep-alive");
		msg.updateHeader("Accept-Language","en-us,zh-cn;q=0.7,zh;q=0.3");
		msg.updateHeader("Accept-Encoding","gzip,deflate");
		msg.updateHeader("Keep-Alive","115");
		msg.updateHeader("Host",ip);
		
		std::string strMsg = msg.toString();const char* p = strMsg.c_str();
		s->write( strMsg.c_str() , strMsg.length() );
		output(url);
		if( i + 1 < count )
		{
			ZQ::common::delay(3);
		}
	}
	return 0;
}
void readfilenamelist( const std::string& path , std::vector<std::string>& namelist )
{
	std::ifstream inf;
	inf.open(path.c_str());
	if(!inf) return;
	std::string name;
	while(getline(inf,name))
	{
		namelist.push_back( name );
	}
}
int ClientCommand::createC2Session( OptResult& opts )
{	
	int count = opts.as<int>("c");
	std::string localip = opts.as<std::string>("li");
	std::string localport = opts.as<std::string>("lp");
	std::string namelistfile= opts.as<std::string>("f");
	std::vector<std::string> filenames;
	if( !namelistfile.empty() )
	{
		readfilenamelist( namelistfile, filenames );
	}
	int nameindex = 0 ;
	for( int i = 0 ;i < count ; i ++ )
	{
		std::string ip = opts.as<std::string>("hostip");
		std::string port = opts.as<std::string>("hostport");
		std::string filename = opts.as<std::string>("file");
		if( filenames.size() > 0 )
		{
			filename = filenames[nameindex];
			nameindex++;
			if( nameindex>= (int)filenames.size() )
				nameindex = 0;
		}

		std::string url = std::string("/scs/getfile?file=")+
			filename+
			"&ic="+opts.as<std::string>("i")+
			"&rate="+opts.as<std::string>("r")+
			"&delay="+opts.as<std::string>("d")+
			"&range="+opts.as<std::string>("g")+
			"&timeout="+opts.as<std::string>("t");

		ZQ::DataPostHouse::ASocketPtr s = mClientService.connect( ip , port , localip, localport );
		if(!s)
		{
			output("failed to connect to %s:%s",ip.c_str() , port.c_str() );
			return -1;
		}
		HttpMessage msg;
		msg.updateMethod("get");
		msg.updateProtocol("http/1.1");
		msg.updateUrl(url);
		msg.updateHeader("cseq","1");
		std::string strMsg = msg.toString();
		s->write( strMsg.c_str() , strMsg.length() );
		output(url);
	}
	return 0;
}
