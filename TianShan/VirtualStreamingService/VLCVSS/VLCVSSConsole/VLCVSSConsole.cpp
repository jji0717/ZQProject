// VLCVSS.cpp : 定义控制台应用程序的入口点。
//

#include "IceLog.h"
#include "VLCStreamserviceImpl.h"
#include "VLCVSSCfgLoader.h"
#include <iostream>
using namespace std;

#define HSNVSSService "VLCVSS"
#define HSNVSSProduct "TianShan"

char *DefaultConfigPath = "C:\\TianShan\\etc\\VLCVSS.xml";
ZQ::common::Config::Loader< ::ZQTianShan::VSS::VLC::VLCVSSCfg > pConfig(DefaultConfigPath);

Ice::CommunicatorPtr	_ic;

::TianShanIce::common::IceLogIPtr _iceLog;

::ZQTianShan::VSS::VLC::VLCStreamServiceImplPtr			 _cps;
::ZQTianShan::ContentStore::ContentStoreImpl::Ptr _contentStore;

BOOL WINAPI ConsoleHandler(DWORD event);
bool bQuit = false;

::ZQ::common::FileLog _fileLog;
::ZQ::common::FileLog _fileLog_CS;
::ZQ::common::FileLog _fileLog_CSEvent;

#define TMPBUFFERSIZE 4096
#define RECVTIMEOUT 5

void testSocket(int argc, char* argv[])
{
	sockaddr_in sockAddr;
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(4212);
	sockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	int serverLen = sizeof(sockAddr);
	SOCKET so = socket(AF_INET, SOCK_STREAM, 0);

	int rc = connect(so, (struct sockaddr *)&sockAddr, serverLen);
	//fail to connect server
	if (rc == SOCKET_ERROR)
	{
		struct timeval timeout;
		fd_set fd;
		FD_ZERO(&fd);   
		FD_SET(so, &fd);
		timeout.tv_sec	= 5;
		timeout.tv_usec	= 0;
		rc = select(0, NULL, &fd, NULL, &timeout);
		if (rc <= 0)
		{
			cout << "fail to connect to server" << endl;
			return;
		}
	}
	
	char pBuffer[1024];
	bQuit = true;
	int iMode = 1;
	//rc = ioctlsocket(so, FIONBIO, (u_long FAR*) &iMode);
	if (rc == 0)
		cout << "success set socket to un-block model" << endl;
	while (bQuit)
	{
		struct timeval timeout;
		fd_set fd;
		FD_ZERO(&fd);   
		FD_SET(so, &fd);
		timeout.tv_sec	= 5;
		timeout.tv_usec	= 0;
		rc = select(0, NULL, &fd, NULL, &timeout);
		cout << "select result=" << rc << endl;
		if (rc > 0)
		{
			int recvLen = recv(so, (char *)pBuffer,(int)(1024), 0);
			cout << "recv len=" << recvLen << endl;
		}
		Sleep(10);
	}
}

void testTCPSocket(int argc, char* argv[])
{
	ZQ::common::InetAddress addr;
	addr.setAddress("127.0.0.1");
	//::ZQ::common::TCPSocket telnet_socket(addr, 4212);

	::ZQ::common::TCPSocket telnet_socket;
	bool b = telnet_socket.connect(5);
	if (b)
	{
		std::string strPWD = "admin\r\n";
		int ret = telnet_socket.send(strPWD.c_str(), strPWD.length());
		
		char chRecv[TMPBUFFERSIZE];
		memset(chRecv,0,TMPBUFFERSIZE);
		ret = telnet_socket.receive(chRecv, TMPBUFFERSIZE, ZQ::common::BLOCK);
		if (ret == SOCKET_ERROR)
			cout << "recv socket error" << endl;
		else
			cout << "recv socket message(len=" << ret << ")\n" << chRecv << endl;

		strPWD = "help\r\nhelp\r\n";
		ret = telnet_socket.send(strPWD.c_str(), strPWD.length());

		memset(chRecv,0,TMPBUFFERSIZE);
		ret = telnet_socket.receive(chRecv, TMPBUFFERSIZE, ZQ::common::BLOCK);
		if (ret == SOCKET_ERROR)
			cout << "recv socket error" << endl;
		else
			cout << "recv socket message(len=" << ret << ")\n" << chRecv << endl;
	}
	else
		cout << "connect to telnet server error" << endl;
}

void testGetList()
{
	ZQ::common::InetAddress addr;
	addr.setAddress("127.0.0.1");
	::ZQ::common::Telnet telnet_socket(addr, 4212);

	CommonVLCPlaylist pl;
	pl._name = "test";
	pl._type = "broadcast";
	VLCPlayItem item;
	item._path = "D:\\temp\\1.mpg";
	pl._inputItem.push_back(item);
	item._path = "D:\\temp\\2.mpg";
	pl._inputItem.push_back(item);
	pl._ouputItem._access = "udp";
	pl._ouputItem._mux = "ts";
	pl._ouputItem._dstIp = "127.0.0.1";
	pl._ouputItem._dstPort = 1234;

	std::string strCmd;
	telnet_socket.setPWD("admin");
	bool b = telnet_socket.connectToServer(5);
	if (b)
	{
		std::string strRecvBuf;

		//create new play list
		VLCProps vlcProps;
		VLCProp vlcProp;

		//set input
		vlcProp.prop = SetProps(strVLCInput, pl);
		vlcProps.push_back(vlcProp);

		//set output
		vlcProp.prop = SetProps(strVLCOutput, pl);
		vlcProps.push_back(vlcProp);

		std::string strPWD = NewPlayList(pl._name, pl._type, vlcProps);
		cout << "send command: " << strPWD << endl;
		if (telnet_socket.sendCMD(strPWD.c_str(), strRecvBuf, RECVTIMEOUT) == false)
			cout << "recv socket error" << endl;
		else
			cout << "recv socket message(len=" << strRecvBuf.length() << ")\n" << strRecvBuf << endl;
		ZQ::common::TelnetParser parser(NULL);
		std::list<std::string> lst;
		strPWD = ShowPlayList("");
		if (telnet_socket.sendCMD(strPWD.c_str(), strRecvBuf, 1000))
		{
			parser.getList(strRecvBuf, lst, "media");
			std::list<std::string>::iterator p = lst.begin();
			cout << "---------show media from all  ----------------------------" <<endl;
			while (p != lst.end())
			{
				cout<< *p<<endl;
				p++;
			}
			cout << "---------show media from all ----------------------------" <<endl;

			parser.getList(strRecvBuf, lst, "schedule");
			cout << "---------show schedule from all  ----------------------------" <<endl;
			p = lst.begin();
			while (p != lst.end())
			{
				cout<< *p<<endl;
				p++;
			}
			cout << "---------show schedule from all ----------------------------" <<endl;

		}
		strPWD = ShowPlayList("media");
		// may be have dirty data, recv: password:?
		if (telnet_socket.sendCMD(strPWD.c_str(), strRecvBuf, 1000))
		{
			parser.getList(strRecvBuf, lst, "media");
			std::list<std::string>::iterator p = lst.begin();
			cout << "---------show media ----------------------------" <<endl;
			while (p != lst.end())
			{
				cout<< *p<<endl;
				p++;
			}
			cout << "---------show media ----------------------------" <<endl;
		}
		strPWD = ShowPlayList("schedule");
		if (telnet_socket.sendCMD(strPWD.c_str(), strRecvBuf, 1000))
		{
			parser.getList(strRecvBuf, lst, "schedule");
			std::list<std::string>::iterator p = lst.begin();
			cout << "---------show schedule ----------------------------" <<endl;
			while (p != lst.end())
			{
				cout<< *p<<endl;
				p++;
			}
			cout << "---------show schedule ----------------------------" <<endl;
		}
		system("pause");

	}
}

void testTelnet(int argc, char* argv[])
{
	ZQ::common::InetAddress addr;
	addr.setAddress("127.0.0.1");
	::ZQ::common::Telnet telnet_socket(addr, 4212);

	CommonVLCPlaylist pl;
	pl._name = "test";
	pl._type = "broadcast";
	VLCPlayItem item;
	item._path = "D:\\seachange_tabletennis\\practise\\M2U00034.MPG";
	pl._inputItem.push_back(item);
	item._path = "D:\\seachange_tabletennis\\practise\\M2U00029.MPG";
	pl._inputItem.push_back(item);
	pl._ouputItem._access = "udp";
	pl._ouputItem._mux = "ts";
	pl._ouputItem._dstIp = "127.0.0.1";
	pl._ouputItem._dstPort = 1234;

	std::string strCmd;
	telnet_socket.setPWD("admin");
	bool b = telnet_socket.connectToServer();
	if (b)
	{
		std::string strRecvBuf;

		//create new play list
		VLCProps vlcProps;
		VLCProp vlcProp;

		//set input
		vlcProp.prop = SetProps(strVLCInput, pl);
		vlcProps.push_back(vlcProp);

		//set output
		vlcProp.prop = SetProps(strVLCOutput, pl);
		vlcProps.push_back(vlcProp);

		std::string strPWD = NewPlayList(pl._name, pl._type, vlcProps);
		cout << "send command: " << strPWD << endl;
		if (telnet_socket.sendCMD(strPWD.c_str(), strRecvBuf, RECVTIMEOUT) == false)
			cout << "recv socket error" << endl;
		else
			cout << "recv socket message(len=" << strRecvBuf.length() << ")\n" << strRecvBuf << endl;

		strPWD = ShowPlayList(pl._name);
		cout << "send command: " << strPWD << endl;
		if (telnet_socket.sendCMD(strPWD.c_str(), strRecvBuf, RECVTIMEOUT) == false)
			cout << "recv socket error" << endl;
		else
			cout << "recv socket message(len=" << strRecvBuf.length() << ")\n" << strRecvBuf << endl;

		std::string strPos;
		std::string strPlIdx;
		strPWD = ControlPlayList(pl._name, std::string("play 1 100s"));
		cout << "send command: " << strPWD << endl;
		if (telnet_socket.sendCMD(strPWD.c_str(), strRecvBuf, RECVTIMEOUT) == false)
			cout << "recv socket error" << endl;
		else
			cout << "recv socket message(len=" << strRecvBuf.length() << ")\n" << strRecvBuf << endl;

		strPWD = ControlPlayList(pl._name, std::string("seek 100s"));
		if (telnet_socket.sendCMD(strPWD.c_str(), strRecvBuf, RECVTIMEOUT) == false)
			cout << "recv socket error" << endl;
		else
			cout << "recv socket message(len=" << strRecvBuf.length() << ")\n" << strRecvBuf << endl;

		strPWD = ShowPlayList(pl._name);
		cout << "send command: " << strPWD << endl;
		if (telnet_socket.sendCMD(strPWD.c_str(), strRecvBuf, RECVTIMEOUT) == false)
			cout << "recv socket error" << endl;
		else
			cout << "recv socket message(len=" << strRecvBuf.length() << ")\n" << strRecvBuf << endl;
		//try to parse
		std::string strStatus;
		ZQ::common::TelnetParser parser(NULL);
		parser.getContentByHeader(strRecvBuf, "state :", strStatus);
		cout << "playitem status: " << strStatus << endl;

		strPWD = DelPlayList(pl._name);
		if (telnet_socket.sendCMD(strPWD.c_str(), strRecvBuf, RECVTIMEOUT) == false)
			cout << "recv socket error" << endl;
		else
			cout << "recv socket message(len=" << strRecvBuf.length() << ")\n" << strRecvBuf << endl;
	}
	else
		cout << "connect to telnet server error" << endl;
	system("pause");
}


void initWithConfig(Ice::PropertiesPtr proper )
{
	proper->setProperty("Ice.ThreadPool.Client.Size","5");
	proper->setProperty("Ice.ThreadPool.Client.SizeMax","10");
	proper->setProperty("Ice.ThreadPool.Server.Size","5");
	proper->setProperty("Ice.ThreadPool.Server.SizeMax","10");

	for (ZQTianShan::VSS::VLC::IceProperties::props::iterator iceIter = pConfig._iceProperties._props.begin();
		iceIter != pConfig._iceProperties._props.end(); iceIter++)
	{
		proper->setProperty((*iceIter).name.c_str(), (*iceIter).value.c_str());
		_fileLog(::ZQ::common::Log::L_INFO, CLOGFMT(VLCVSSConsole, "Set ICE Property %s=%s"), (*iceIter).name.c_str(), (*iceIter).value.c_str());			
	}
}

void ConfigTest()
{
	::ZQTianShan::VSS::VLC::VLCVSSConfig  vssConfig("C:\\TianShan\\etc\\VLCVSS.xml");
	vssConfig.ConfigLoader();
	cout << "load config" << endl;
	cout << "<TianShan>" << endl;

	cout << "  <Definitions src=\"TianShanDef.xml\">" << endl;
	cout << "  </Definitions>" << endl;

	cout << "  <default>" << endl;

	cout << "    "
		<< "<CrashDump path=\"" << vssConfig._crashDump.path 
		<< "\" enabled=\"" << vssConfig._crashDump.enabled 
		<< "\" />" << endl;

	cout << "    "
		<< "<IceTrace enabled=\"" << vssConfig._iceTrace.enabled
		<< "\" level=\"" << vssConfig._iceTrace.level
		<< "\" size=\"" << vssConfig._iceTrace.size 
		<< "\"/>" << endl;

	cout << "    "
		<< "<EventChannel endPoint=\"" << vssConfig._iceStorm.endPoint <<"\" />" << endl;

	cout << "    " << "<IceProperties>" << endl;
	for (::ZQTianShan::VSS::VLC::IceProperties::props::iterator iter = vssConfig._iceProperties._props.begin(); iter != vssConfig._iceProperties._props.end(); iter++)
		cout << "      " << "<prop name=\"" << iter->name << "\" value=\"" << iter->value << "\" />" << endl;
	cout << "    " << "</IceProperties>" << endl;

	cout << "    " << "<PublishedLogs>" << endl;
	for (std::vector<::ZQTianShan::VSS::VLC::PublishedLogs::PublishLogHolder>::iterator iter = vssConfig._publishedLogs._logDatas.begin(); iter != vssConfig._publishedLogs._logDatas.end(); iter++)
		cout << "      " << "<Log  path=\"" << iter->_path <<"\" syntax=\"" << iter->_syntax << "\" />" << endl;
	cout << "    " << "</PublishedLogs>" << endl;

	cout << "    " << "<Database path=\"" << vssConfig._dataBase.path <<"\" runtimePath=\"" << vssConfig._dataBase.runtimePath << "\" />" << endl;

	cout << "  </default>" << endl;

	cout << "  <VLCVSS>" << endl;

	cout << "    " << "<LogFile path=\"" << vssConfig._cLogFile.path
		<< "\" level=\"" << vssConfig._cLogFile.level
		<< "\" maxCount=\"" << vssConfig._cLogFile.maxCount
		<< "\" size=\"" << vssConfig._cLogFile.size
		<< "\" buffer=\"" << vssConfig._cLogFile.bufferSize
		<< "\" flushTimeout=\"" << vssConfig._cLogFile.flushTimeout
		<< "\" />" << endl;

	cout << "    " << "<Bind endPoint=\"" << vssConfig._bind.endPoint
		<< "\" dispatchSize=\"" << vssConfig._bind.dispatchSize
		<< "\" dispatchMax=\"" << vssConfig._bind.dispatchMax
		<< "\" evictorSize=\"" << vssConfig._bind.evictorSize
		<< "\" threadPoolSize=\"" << vssConfig._bind.threadPoolSize
		<< "\"/>" << endl;

	cout << "    " << "<TelnetProp threadPoolSize=\"" << vssConfig._telnetProp.threadPoolSize
		<< "\" timeOut=\"" << vssConfig._telnetProp.timeOut
		<< "\" bufferMaxSize=\"" << vssConfig._telnetProp.bufferMaxSize
		<< "\"/>" << endl;

	cout << "    " << "<IceLog path=\"" << vssConfig._VLCVSSIceLog.path
		<< "\" level=\"" << vssConfig._VLCVSSIceLog.level
		<< "\" maxCount=\"" << vssConfig._VLCVSSIceLog.maxCount
		<< "\" size=\"" << vssConfig._VLCVSSIceLog.size
		<< "\" buffer=\"" << vssConfig._VLCVSSIceLog.bufferSize
		<< "\" flushTimeout=\"" << vssConfig._VLCVSSIceLog.flushTimeout
		<< "\"/> " << endl;

	cout << "    " << "<TelnetServerInfo ip=\"" << vssConfig._telnetServerInfo.ip
		<< "\" port=\"" << vssConfig._telnetServerInfo.port
		<< "\" password=\"" << vssConfig._telnetServerInfo.password
		<< "\"/> " << endl;

	cout << "    " << "<VolumnInfo>" << endl;
	for (::ZQTianShan::VSS::VLC::VolumnInfo::Volumns::iterator iter = vssConfig._volumnInfo._volumns.begin(); iter != vssConfig._volumnInfo._volumns.end(); iter++)
		cout << "      " << "<Volume  name=\"" << iter->name <<"\" path=\"" << iter->path << "\" />" << endl;
	cout << "    " << "</VolumnInfo>" << endl;


	cout << "  </VLCVSS>" << endl;
	cout << "</TianShan>" << endl;
	system("PAUSE");
}

int main(int argc, char* argv[])
{
	//ConfigTest();
	//testTelnet(argc, argv);
	//testGetList();
	//testSocket(argc, argv);
	//return 1;

	pConfig.load(DefaultConfigPath, true);
	if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE) ConsoleHandler, TRUE)==FALSE)
	{
		printf("Unable to install handler!                      \n");
		return -1;
	}

	//initialize ice log
	::ZQ::common::FileLog _iceFileLog(pConfig._VLCVSSIceLog.path.c_str(), pConfig._VLCVSSIceLog.level, pConfig._VLCVSSIceLog.maxCount, pConfig._VLCVSSIceLog.size, pConfig._VLCVSSIceLog.bufferSize, pConfig._VLCVSSIceLog.flushTimeout);

	//initialize VSS file log
	_fileLog.open(pConfig._logFile.path.c_str(), pConfig._logFile.level, pConfig._logFile.maxCount,pConfig._logFile.size, pConfig._logFile.bufferSize, pConfig._logFile.flushTimeout);

	std::string strCSLog = pConfig._logFile.path + "_CS";
	_fileLog_CS.open(strCSLog.c_str(), pConfig._logFile.level, pConfig._logFile.maxCount,pConfig._logFile.size, pConfig._logFile.bufferSize, pConfig._logFile.flushTimeout);
	strCSLog += "Event";
	_fileLog_CSEvent.open(strCSLog.c_str(), pConfig._logFile.level, pConfig._logFile.maxCount,pConfig._logFile.size, pConfig._logFile.bufferSize, pConfig._logFile.flushTimeout);


	//initialize thread pool
	::ZQ::common::NativeThreadPool _pVSSThreadPool(10);
	_pVSSThreadPool.resize( pConfig._telnetProp.threadPoolSize> 10 ? pConfig._telnetProp.threadPoolSize : 10 );

	//initialize service
	_iceLog = new ::TianShanIce::common::IceLogI(&_iceFileLog);

	int i=0;

#if ICE_INT_VERSION / 100 >= 303

	::Ice::InitializationData initData;
	initData.properties = Ice::createProperties(i, NULL);
	initWithConfig(initData.properties);

	initData.logger = _iceLog;

	_ic = Ice::initialize(initData);

#else
	Ice::PropertiesPtr proper = Ice::createProperties(i,NULL);
	initWithConfig(proper);	

	_ic=Ice::initializeWithPropertiesAndLogger(i,NULL,proper, _iceLog);
#endif // ICE_INT_VERSION

	std::string	strDbPath ;
	strDbPath = pConfig._dataBase.path;


	//get media server config
	std::string strServerPath = pConfig._telnetServerInfo.ip;
	uint16 uServerPort = pConfig._telnetServerInfo.port;
	std::string strPassword = pConfig._telnetServerInfo.password;
	uint16 uTelnetPoolSize = pConfig._telnetProp.threadPoolSize;

	ZQTianShan::VSS::VLC::VLCVSSEnv *_pVSSEnv = new ZQTianShan::VSS::VLC::VLCVSSEnv(_fileLog, _pVSSThreadPool,
		strServerPath, uServerPort, strPassword, uTelnetPoolSize,
		_ic,
		pConfig._iceStorm.endPoint.c_str(),
		pConfig._VLCServiceProp.szServiceName, pConfig._VLCServiceProp.synInterval,
		pConfig._bind.endPoint.c_str(),
		pConfig._dataBase.path.c_str(),
		pConfig._dataBase.runtimePath.c_str());

	_cps = new ::ZQTianShan::VSS::VLC::VLCStreamServiceImpl(_fileLog, _ic, pConfig._bind.evictorSize, *_pVSSEnv);
	_cps->start();

	::std::string csDataPath = pConfig._dataBase.path + FNSEPS + HSNVSSService;
	_contentStore = new ZQTianShan::ContentStore::ContentStoreImpl(_fileLog_CS,_fileLog_CSEvent, _pVSSThreadPool, _pVSSEnv->_adapter, csDataPath.c_str());

	try{
		::std::string CSPath =  std::string("VLCCS") + FNSEPS;
		_contentStore->initializeContentStore();
		if (pConfig._volumnInfo._volumns.empty())
			_contentStore->mountStoreVolume("70001", CSPath.c_str(), true);
		else
		{
			for (::ZQTianShan::VSS::VLC::VolumnInfo::Volumns::iterator iter = pConfig._volumnInfo._volumns.begin(); iter != pConfig._volumnInfo._volumns.end(); iter++)
				_contentStore->mountStoreVolume(iter->name, iter->path);
		}
		_pVSSEnv->connectToContentStore();

		//initialize content store basic infor
		//config for contentstore information
		_contentStore->_netId = pConfig._storeInfo.netId;
		if(_contentStore->_netId.length() == 0)
		{
			char chHost[256] = {0};
			gethostname(chHost,sizeof(chHost));
			_contentStore->_netId = chHost;
		}
		_contentStore->_storeType = pConfig._storeInfo.type;
		_contentStore->_streamableLength = atol(pConfig._storeInfo.streamableLength.c_str());

		_contentStore->_replicaGroupId = pConfig._storeInfo.groupId;
		_contentStore->_replicaId = pConfig._storeInfo.replicaId;
		if(_contentStore->_replicaId.length() == 0)
			_contentStore->_replicaId = _contentStore->_netId;

		_contentStore->_replicaPriority = pConfig._storeInfo.replicaPriority;
		_contentStore->_replicaTimeout = pConfig._storeInfo.timeout;
		_contentStore->_contentEvictorSize = pConfig._storeInfo.contentSize;
		_contentStore->_volumeEvictorSize = pConfig._storeInfo.volumeSize;

	}
	catch(...)
	{
		_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(VLCVSS, "mount store volume name (%s) path (%s) catch an exception\n"), "70001", "VLCCS");
		return -1;
	}

	_fileLog(::ZQ::common::Log::L_INFO, CLOGFMT(VLCVSS, "VLC Stream Service and ContentStore Service created"));
	while (!bQuit)
	{
		static const char* chs="-\\|/";
		static int chi=0;
		chi = ++chi %4;
		//printf("\rTMVSS is now listening %c", chs[chi]);
		printf("\nVLCVSS is now listening");
		Sleep(5000);
	}

	ZQ::common::setGlogger(NULL);
	_iceLog = NULL;
	printf("\rVLCVSS Console is quiting                   \r");

	_cps->uninitializeService();
	_cps = NULL;
	_contentStore->unInitializeContentStore();
	_contentStore = NULL;

	_pVSSThreadPool.stop();
	delete _pVSSEnv;
	_pVSSEnv = NULL;
	//_pCVSSEnv->_adapter->deactivate();
	_ic->shutdown();
	_ic->destroy();
	_ic = NULL;

	Sleep(1000);

	printf("\rVLCVSS stopped                    \n");
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
