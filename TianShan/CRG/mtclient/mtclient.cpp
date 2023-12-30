#include "wtconfig.h"
#include "NativeThread.h"
#include "command.h"
#include "getopt.h"
#include "tlock.h"
#include "wtsocket.h"
#include "wtlog.h"
#include "wtguuid.h"
#include "wtusocket.h"
#include "../DsmccCRG/DsmccDefine.h"

#define MILLIS_TO_NANOS 1000
#define MAINFUNIDIFIER main 

Mutex g_Lock;
CritSect sect;


std::string g_sessionID,g_locIP;
unsigned long g_ltimeout;
char* g_pmv_resourse;
CritSect g_sleeptime;
ZQ::common::Log  *PLOG;
void  usages_printf()
{
	fprintf(stderr,"Usage:%s [--ms ] [--confile ] [--logfile ] [--resourse ] [--threads 10] [--timeout ] [--suffix ] [--ffsflag] [--protocol]\n", "mtclient");
	fprintf(stderr,"Usage:%s [--ms 100] [--confile ./data/configure] [--logfile ./mtclient.log] [--resourse dsmcctest0001] [--threads 10] [--timeout 150000] [--suffix 0] [--ffsflag 0] [--protocol 0]\n","mtclient");
	exit(-1);
}
//temporarily 
unsigned long  msintime=0,lthreads=0,ltimeout=0;
static bool g_bflag_suffix,g_bflag_ffsession=0,g_protocolflag; 
char *pconfile=NULL,*plogfile=NULL,*ptemp=NULL;
ZQ::DSMCC::ProtocolType protocol_type = ZQ::DSMCC::Protocol_MOTO;
 

COMMANDLIST  sdlins2;
PROPERTIESMAP  g_propertyMap;
std::string g_setuptime;

bool gethostIP()
{
	char host_name[255] = {0}; 
	if (gethostname(host_name, sizeof(host_name)) == SOCKET_ERROR) { 
		printf("Error %d when getting local host name.\n", WSAGetLastError()); 
		return 1; 
	} 
	//printf("Host name is: %s\n", host_name); 
	struct hostent *phe = gethostbyname(host_name); 
	if (phe == 0) { 
		printf("Yow! Bad host lookup."); 
		return 1; 
	} 
	//for (int i = 0; phe->h_addr_list[i] != 0; ++i) { 
	struct in_addr addr; 
	memcpy(&addr, phe->h_addr_list[0], sizeof(struct in_addr)); 
	//printf("Address %d : %s\n" , 0, inet_ntoa(addr)); 
	g_locIP=inet_ntoa(addr);
	//} 
	return true; 
}
void parse_parameter(int argc, char *argv[])
{
	if (argc<3)
		usages_printf();
	int option_index = 0,c;
	static struct option long_options[] = {{"ms",    1, 0, 'm'},
	{"confile", 1, 0, 'C'},
	{"logfile", 1, 0, 'L'},
	{"resourse", 1, 0, 'R'},
	{"threads",1, 0, 'T'},
	{"timeout", 1, 0, 't'},
	{"file",   1, 0, 0},
	{"suffix", 1, 0, 'S'},
	{"ffsflag",1, 0, 'f'},
	{"protocol",1, 0, 'p'},
	{0,        0, 0, 0}
	};

	while (-1 != (c = getopt_long(argc, argv, "mC:L:TtSfp",long_options, &option_index))) 
	{
		if (c == -1)
			usages_printf();
		switch (c) 
		{
		case 'm':
			ptemp=optarg;
			if(ptemp)
				msintime=atol(ptemp);
			else
				msintime=0;
			break;
		case 'C':
			ptemp=optarg;
			pconfile=ptemp;
			break;
		case 'L':
			ptemp=optarg;
			plogfile=ptemp;
			break;
		case 'R':
			ptemp = optarg;
			g_pmv_resourse=ptemp;
			break;
		case 'T':
			ptemp=optarg;
			if(ptemp)
				lthreads=atol(ptemp);
			else
				lthreads=20;
			break;
		case 't':
			ptemp=optarg;
			if(ptemp)
				ltimeout=atol(ptemp);
			else
				ltimeout=150000;
			break;
		case 'S':
			ptemp=optarg;
			if ( 0 == strcmp("1",ptemp))
				g_bflag_suffix=1;
			else
				g_bflag_suffix=0;
			break;
		case 'f':
			ptemp=optarg;
			if ( 0 == strcmp("1",ptemp))
				g_bflag_ffsession=1;
			else
				g_bflag_ffsession=0;
			break;
		case 'p':
			ptemp=optarg;
			if ( 0 == strcmp("1",ptemp))
				protocol_type = ZQ::DSMCC::Protocol_Tangberg;
			else
				protocol_type = ZQ::DSMCC::Protocol_MOTO;
			break;
		case '?':
			usages_printf();
		default:
			usages_printf();
		}
	}
	if(0 != msintime)
		Sleep(msintime);
	g_ltimeout=ltimeout;
	CWTCONFIG  config_insobj;

	config_insobj.loadConfigureFile(pconfile);
	config_insobj.parse();
	config_insobj.loadProperyFile("");
	sdlins2=config_insobj._sendlist;
	//define macro protocol flags   
	COMMANDLIST::iterator citor;
	if (sdlins2.size() > 0)
	{
		citor=sdlins2.begin();
		if (0 == _stricmp("tcp",citor->sprotocol.c_str()) )
			g_protocolflag=1;
		else
			g_protocolflag=0;
	}
}

class ClientThread:public ZQ::common::NativeThread
{
public:

	ClientThread(std::string n, COMMANDLIST& sdl):name(n),_threadsendlist(sdl.begin(),sdl.end())
	{}
	virtual int  run();
public:
	COMMANDLIST::iterator itor;
	COMMANDLIST   _threadsendlist;
private:
	C_COMMANDS ccobject;
	std::string name;
};
//
class ClientThreadU:public ZQ::common::NativeThread
{
public:

	ClientThreadU(std::string n, COMMANDLIST& sdl):name(n),_threadsendlist(sdl.begin(),sdl.end())
	{}
	virtual int  run();
public:
	COMMANDLIST::iterator itor;
	COMMANDLIST   _threadsendlist;
private:
	C_COMMANDS ccobject;
	std::string name;
};

ClientThread** g_pthreads;
ClientThreadU** g_upthreads;
int ClientThread::run()
{
	//(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(ClientThread,"invoke run()"));
	SocketClient tp;

	CWTGUUID  cguid(g_bflag_suffix,g_bflag_ffsession);//cguid.set_suffix(g_bflag_suffix);cguid.set_ffsession(g_bflag_ffsession);
	cguid.decimalSerialN2();
	itor=_threadsendlist.begin();
	while(itor != _threadsendlist.end())
	{
		if ( 0== (*itor).scommand.compare("setup"))
		{
			cout<<"setup  "<<endl;
			ccobject.cSutUp(tp,cguid,(*itor));
			(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(ClientThread,"setup ."));
			if(!ccobject.get_setupflag())
				return -1;
			goto LOOPHUMP;
		}	
		
		if ( 0== (*itor).scommand.compare("play"))
		{
			cout<<"play  "<<endl;
			ccobject.cPlay(tp,cguid,(*itor));
			(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(ClientThread,"play ."));
			goto LOOPHUMP;
		}	
			
		if ( 0== (*itor).scommand.compare("status"))
		{
			cout<<"status  "<<endl;
			ccobject.cstatus(tp,cguid,(*itor));
			(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(ClientThread,"status ."));
			goto LOOPHUMP;
		}

		if ( 0== (*itor).scommand.compare("pause"))
		{
			cout<<"pause  "<<endl;
			ccobject.cpause(tp,cguid,(*itor));
			(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(ClientThread,"pause ."));
			goto LOOPHUMP;
		}

		if ( 0== (*itor).scommand.compare("close"))
		{
			cout<<"close  "<<endl;
			ccobject.cClose(tp,cguid,(*itor));
			(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(ClientThread,"close ."));
			goto LOOPHUMP;
		}

LOOPHUMP:
		itor++;
		//_threadsendlist.pop_front();
	}

	(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(ClientThread,"ClientThread run() exit....."));
	return 0;
}
//udp 
int ClientThreadU::run()
{
	//(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(ClientThread,"invoke run()"));
	SocketClientU tp;

	CWTGUUID  cguid(g_bflag_suffix,g_bflag_ffsession);//cguid.set_suffix(g_bflag_suffix);cguid.set_ffsession(g_bflag_ffsession);
	cguid.decimalSerialN2();
	itor=_threadsendlist.begin();
	while(itor != _threadsendlist.end())
	{
		if ( 0== (*itor).scommand.compare("setup"))
		{
			cout<<"setup  "<<endl;
			ccobject.cSutUp(tp,cguid,(*itor));
			(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(ClientThread,"setup ."));
			if(!ccobject.get_setupflag())
				return -1;
			goto LOOPHUMP;
		}	
		if ( 0== (*itor).scommand.compare("play"))
		{
			cout<<"play  "<<endl;
			ccobject.cPlay(tp,cguid,(*itor));
			(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(ClientThread,"play ."));
			goto LOOPHUMP;
		}	

		if ( 0== (*itor).scommand.compare("status"))
		{
			cout<<"status  "<<endl;
			ccobject.cstatus(tp,cguid,(*itor));
			(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(ClientThread,"status ."));
			goto LOOPHUMP;
		}

		if ( 0== (*itor).scommand.compare("pause"))
		{
			cout<<"pause  "<<endl;
			ccobject.cpause(tp,cguid,(*itor));
			(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(ClientThread,"pause ."));
			goto LOOPHUMP;
		}

		if ( 0== (*itor).scommand.compare("close"))
		{
			cout<<"close  "<<endl;
			ccobject.cClose(tp,cguid,(*itor));
			(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(ClientThread,"close ."));
			goto LOOPHUMP;
		}

LOOPHUMP:
		itor++;
		//_threadsendlist.pop_front();
	}

	(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(ClientThread,"ClientThread run() exit....."));
	return 0;
}
bool   _CtrlHandle(DWORD   _dwCtrlType) 
{ 
	int i;
	if(_dwCtrlType==CTRL_C_EVENT||_dwCtrlType==CTRL_BREAK_EVENT) 
	{
		system("taskkill \/F \/IM mtclient.exe >> systemcall.log");
		for(i=0; i<lthreads; ++i) 
		{		
		 	//TerminateThread(g_pthreads[i]._thrdID,89);
			if (g_protocolflag)
			{
				if (g_pthreads[i])
					delete [] g_pthreads[i];
			}
			else
			{
				if (g_upthreads[i])
					delete [] g_pthreads[i];
			}
				
		}
		//system("taskkill \/F \/IM mtclient.exe >> systemcall.log");
	}
	return   true; 
} 
void   _ConsoleInit() 
{ 
	HANDLE hStdIn=GetStdHandle(STD_INPUT_HANDLE); 
	HANDLE hStdOut=GetStdHandle(STD_OUTPUT_HANDLE); 

	SetConsoleCtrlHandler((PHANDLER_ROUTINE)&_CtrlHandle,TRUE); 
	return; 
} 

int main(int argc, char *argv[]) 
{
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	char starttimebuf[64]={0};
	sprintf(starttimebuf,"%d-%d %02d:%02d:%02d.%d",sys.wMonth,sys.wDay,sys.wHour,sys.wMinute,sys.wSecond,sys.wMilliseconds);
	_ConsoleInit();
	WSADATA info;  
	if (WSAStartup(MAKEWORD(2,2), &info)) 
	{  
		throw "Could not start WSA ,initlization WSAStartup failed";  
	}  
	parse_parameter(argc,argv);
	CWTLOG    clog(plogfile);
	PLOG=clog.traceFileLog;
	// --ms 160 --confile ./data/configurefile --logfile romalin99.log --threads 20  --timeout 150000 --suffix 0 --ffsflag 0 --resourse dsmcctest001
	(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(MAINFUNIDIFIER,
		"PID[%05d],startNOW[%s],start delay[%d],configurefile[%s],logfilename[%s],threads[%d],socket timeout[%ld],resouce suffix[%d], sessionID prefix[%d],reource name[%s]"),
		getpid(),starttimebuf,msintime,pconfile,plogfile,lthreads,ltimeout,g_bflag_suffix,g_bflag_ffsession,g_pmv_resourse);

	//get local IP
	gethostIP();

	int i = 0,threadintervalms=0;
	PROPERTIESMAP::iterator proitor;
	if (0 == lthreads )
	{
		proitor=g_propertyMap.find("threadcount");
		if (g_propertyMap.end() == proitor)
			lthreads = 10;
		else
			lthreads = proitor->second.empty()==1 ? 10 :atol(proitor->second.c_str());
	}
	
	(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(MAINFUNIDIFIER,"parameter threads:%ld"),lthreads);

	//std::auto_ptr<ClientThread>* pthreads  = new ClientThread*[lthreads];
	proitor= g_propertyMap.find("threadinterval");
	if (g_propertyMap.end() == proitor)
		threadintervalms=30;
	else
		threadintervalms = proitor->second.empty()==1 ? 30 : atol(proitor->second.c_str());

    if (g_protocolflag)
    {
		ClientThread** pthreads = new ClientThread*[lthreads];
		g_pthreads = pthreads;
		for(i=0; i<lthreads; ++i) 
		{
			char tname[50]={0};
			sprintf(tname,"Thread-%d",i);
			pthreads[i] = new ClientThread(tname,sdlins2);

		}
		(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(MAINFUNIDIFIER,"create all threads OK!,threadinterval:%sms. "),proitor->second.c_str());

		for(i=0; i<lthreads; ++i) 
		{		
			Sleep(threadintervalms);
			bool bf=pthreads[i]->start();
			if (false == bf)
				(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(MAINFUNIDIFIER,"create thread failed!"));
		}

		Sleep(120* 1000);

		for(i=0; i<lthreads; ++i) 
		{
			if (pthreads[i])
			{
				pthreads[i]->waitHandle(30000*1000);
				delete pthreads[i];
			}

		}
    }
	else
	{
		ClientThreadU** pthreads = new ClientThreadU*[lthreads];
		g_upthreads = pthreads;
		for(i=0; i<lthreads; ++i) 
		{
			char tname[50]={0};
			sprintf(tname,"Thread-%d",i);
			pthreads[i] = new ClientThreadU(tname,sdlins2);

		}
		(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(MAINFUNIDIFIER,"create all threads OK!,threadinterval:%sms. "),proitor->second.c_str());

		for(i=0; i<lthreads; ++i) 
		{		
			Sleep(threadintervalms);
			bool bf=pthreads[i]->start();
			if (false == bf)
				(*PLOG)(ZQ::common::Log::L_INFO,CCLOGFMT(MAINFUNIDIFIER,"create thread failed!"));
		}

		Sleep(120* 1000);

		for(i=0; i<lthreads; ++i) 
		{
			if (pthreads[i])
			{
				pthreads[i]->waitHandle(30000*1000);
				delete pthreads[i];
			}

		}
	}

	return 0; 
} 

