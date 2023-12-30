// simpleRtsp.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "ScLog.h"

#include "RtspConnectionManager.h"

//////////////////////////////////////////////////////////////////////////
// rtsp settings
//////////////////////////////////////////////////////////////////////////
int		port		= 554;
DWORD	purid		= 101;
char	ip[20]		= "10.3.0.23";
char	appuid[12]	= "00080001";
char	astuid[12]	= "00080010";
//////////////////////////////////////////////////////////////////////////


int main(int argc, char* argv[])
{
	printf("Rtsp Test started\n");

	ZQ::common::ScLog rtsplog("D:\\RTSP.log", Log::L_DEBUG, 32*1024*1024);
	pGlog = &rtsplog;
	//////////////////////////////////////////////////////////////////////////

	char	ipstr[20];
	char	portstr[8];
	char	purstr[16];
	char	urlstr[256];

	strncpy(ipstr, ip, 20);
	sprintf(portstr, "%d", port);
	sprintf(purstr, "%010d", purid);
	sprintf(urlstr, "rtsp://%s/mediacluster?%s.%s", ip, appuid, astuid);
	
	RtspConnectionManager manager(10);
	manager.start();

	//////////////////////////////////////////////////////////////////////////
	

	std::string	tmpsession;
	bool status = false;
	//////////////////////////////////////////////////////////////////////////
	// create client
	RtspClient* testconn =manager.createClient(purid, ipstr, port, 480000);

	if(!testconn)
	{
		glog("Can not create rtsp connection!");
		manager.terminate();
		return 0;
	}

	RtspRequest reqSetup("SETUP",urlstr);
	RtspRequest reqPlay("PLAY","*");
	RtspRequest reqTeardown("TEARDOWN", "*");
	RtspResponse resBack;
	
	//////////////////////////////////////////////////////////////////////////
	
	// make up Transport header
	RtspMsgHeader transport(KEY_TRANSPORT);
	std::string multimac = RtspClient::getMulticastMac("225.12.12.126");
	transport.setSubHeaderField(" MP2T/AVP/UDP","");
	transport.setSubHeaderField("unicast","");
	transport.setSubHeaderField(KEY_DESTINATION,"225.12.12.126");
	transport.setSubHeaderField(KEY_CLIENTPORT,"8000");
	transport.setSubHeaderField(KEY_CLIENTMAC,multimac.c_str());

	// make up SeaChange-Server-Data header
	RtspMsgHeader serverdata(KEY_SEACHANGESERVERDATA);
	serverdata.setSubHeaderField(KEY_NODEGROUPID,"1");
	serverdata.setSubHeaderField(KEY_DEVICEID,multimac.c_str());
	
	// make up SeaChange-Mod-Data header
	RtspMsgHeader moddata(KEY_SEACHANGEMODDATA);
//	moddata.setSubHeaderField(KEY_PURCHASEID, purstr);
	moddata.setSubHeaderField(KEY_HOMEID, purstr);

	// form the whole SETUP request
	reqSetup.setHeaderField(KEY_SEACHANGEVERSION,"1");
	reqSetup.setHeaderField(transport.getName().c_str(),transport.toString().c_str());
	reqSetup.setHeaderField(KEY_SEACHANGEMAYNOTIFY," ");
	reqSetup.setHeaderField(serverdata.getName().c_str(),serverdata.toString().c_str());
	reqSetup.setHeaderField(moddata.getName().c_str(), moddata.toString().c_str());

	// send SETUP request & PLAY request
	status = testconn->sendMsg(reqSetup, resBack);
	tmpsession = resBack.getHeaderField(KEY_SESSION);
			
	if(!status)
	{
		manager.terminate();
		::Sleep(1000);
		return 0;
	}
	//////////////////////////////////////////////////////////////////////////
	
	resBack.clearMessage();
	// form the whole PLAY request
	reqPlay.setHeaderField(KEY_SCALE,"1.0");
	status = testconn->sendMsg(reqPlay, resBack);
	
	//////////////////////////////////////////////////////////////////////////
	
	::Sleep(30000);

	status = testconn->sendMsg(reqTeardown, resBack);
			
	//////////////////////////////////////////////////////////////////////////
	
	manager.removeClient(testconn);

	manager.terminate();

	printf("Rtsp Test finished\n");
	return 0;

}

