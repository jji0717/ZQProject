#include "ScheduleTV.h"
#include "../ini.h"
#include "../../Common/comextra/ZqStringConv.h"

#include <signal.h>
BOOL bTerminated = FALSE;

void signal_handler(int code)
{
	printf("Do you really want to Quit? [y/n] :");
	char c;
	c = getchar();
	if (c == 'y' || c == 'Y')
		bTerminated = TRUE;
}
// global STV Main Control object

using namespace ZQ::comextra;
ScheduleTV gSTV;

void _tmain() 
{
	CIni mainIni;
	if(!mainIni.Open("Mainctrl.ini")) {
		printf("Can not find Mainctrl.ini file!\n");
		return;
	}

	// install Ctrl-C signal
	signal(SIGINT, signal_handler);
	
	wcscpy(gSTV._wcsVer, L"0.1.8");
	// init attribute members
//	gSTV.m_dwSMPort = 4444;
	gSTV.m_dwRtspNsec = 5000;
	gSTV.m_dwRtspHostPort = 554;
	gSTV.m_dwListenerFrequence = 500;
	gSTV.m_dwClientID = 1;
	gSTV.m_dwType = ITV_TYPE_PRIMARY_ZQ_SCHEDULEDTV;
	gSTV.m_dwInst = 1;
	gSTV.m_dwFillLength = DEFAULT_FILLLENGTH;
	gSTV.m_dwMaxSubChannel = MAX_SUBCHNL;

	//PM
	wcscpy(gSTV.m_wszMirrorPath, As2Ws(mainIni.ReadText("PM","wszMirrorPath")).c_str() );

	//SM
	wcscpy(gSTV.m_wszBindIP, As2Ws(mainIni.ReadText("SM","wszBindIP")).c_str() );
	wcscpy(gSTV.m_wszSMServerIP, As2Ws(mainIni.ReadText("SM","wszSMServerIP")).c_str() );
	gSTV.m_dwSMPort = mainIni.ReadInt("SM", "dwSMPort");
	//RTSP
	wcscpy(gSTV.m_wszRtspHostIP, As2Ws(mainIni.ReadText("RTSP","wszRtspHostIP")).c_str() );
	wcscpy(gSTV.m_wszRtspURL, As2Ws(mainIni.ReadText("RTSP","wszRtspURL")).c_str() );
	gSTV.m_dwRtspNsec = mainIni.ReadInt("RTSP", "dwRtspNsec");

	//ISS
	gSTV.m_dwAppUID = mainIni.ReadInt("ISS", "dwAppUID");
	//MAIN
	wcscpy(gSTV.m_wszLogPath, As2Ws(mainIni.ReadText("Main","wszLogPath")).c_str() );
	
	//wchar_t ScheduleTV::m_wszBindIP [] = L"192.168.80.72";
//	wcscpy(gSTV.m_wszMirrorPath, L"C:\\TestDB\\");
//
//	// for 10.3 machine
//	wcscpy(gSTV.m_wszBindIP, L"10.3.0.22");
//	wcscpy(gSTV.m_wszRtspHostIP, L"10.3.0.23");
//	wcscpy(gSTV.m_wszRtspURL, L"rtsp://10.3.0.23/mediacluster?00080002.00080001");
//	wcscpy(gSTV.m_wszSMServerIP, L"10.3.0.72");

	// for 192.168 machine
//	wcscpy(gSTV.m_wszBindIP, L"192.168.12.12");
//	wcscpy(gSTV.m_wszRtspHostIP, L"192.168.12.12");
//	wcscpy(gSTV.m_wszRtspURL, L"rtsp://192.168.12.12/mediacluster?0560004.0560001");
//	wcscpy(gSTV.m_wszSMServerIP, L"192.168.12.176");

//	wcscpy(gSTV.m_wszLogPath, L"C:\\TestDB\\STV.log");



	// init SM  information

	gSTV.OnInit();


	gSTV.OnStart();

//	::Sleep(1000);
//
//	int status;
//	ZQ::common::ComInitializer init;
//
//	ZQ::common::XMLPrefDoc doc1(init);
//	ZQ::common::XMLPrefDoc doc2(init);
//	ZQ::common::XMLPrefDoc doc3(init);
//	ZQ::common::XMLPrefDoc doc4(init);
//	if (!doc1.open("D:\\channel.xml"))
//		return;
//	if (!doc2.open("D:\\list.xml"))
//		return;
//	if (!doc3.open("D:\\filler.xml"))
//		return;
//	if (!doc4.open("D:\\configure.xml"))
//		return;
//	ZQ::common::IPreference* root1 = doc1.root();
//	ZQ::common::IPreference* root2 = doc2.root();
//	ZQ::common::IPreference* root3 = doc3.root();
//	ZQ::common::IPreference* root4 = doc4.root();
		
//	status=gSTV.OnConfigration(root4);
//
//	gSTV.OnNewPlayList(root1, root2, LISTTYPE_PLAYLIST);
	

//	root1->free();
//	root2->free();
//	root3->free();
//	root4->free();

	printf("Type Ctrl-C to terminate the program\n");
	while (!bTerminated) Sleep(1000);


	gSTV.OnClose();
	gSTV.OnUnInit();

/*	while(1)
	{
		::Sleep(5000);
	}*/
}