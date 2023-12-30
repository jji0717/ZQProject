// simpleRtsp.cpp : Defines the entry point for the console application.
//
#include "InetAddr.h"
#include "UDPSocket.h"
#include "stdafx.h"
#include "Log.h"

#include "RtspConnectionManager.h"
#pragma comment (lib, "ScThreadPool_d.lib")

int _tmain(int argc, _TCHAR* argv[])
{
	printf("Rtsp Test started");
	pGlog = new ZQ::common::ScLog("D:\\Rtsptest\\RTSP.log", Log::L_DEBUG);
	char* testmsgstring = "SETUP rtsp://10.0.0.24/mediacluster?80001.8000f RTSP/1.0\r\n"
		"CSeq:1\r\n"
		"SeaChange-Version:0.05\r\n"
		"Transport:MP2T/DVBC/QAM;unicast\r\n"
		"SeaChange-MayNotify:\r\n"
		"SeaChange-Server-Data:node-group-id=1234567890;smartcard-id=1234567890\r\n"
		"\r\n";

	char* string2 ="RTSP/1.0 200 OK\r\n"
		"CSeq:1\r\n"
		"Session:123456789abcdef1234;timeout=60\r\n"
		"Transport:MP2T/DVBC/QAM;unicast\r\n"
		"SeaChange-Transport:transport-stream-id=12345;program-number=12345;frequency=123456789;qam-mode=123\r\n"
		"SeaChange-Server-Data:node-group-id=1234567890;smartcard-id=1234567890\r\n"
		"SeaChange-Entitlement:purchase-id=1234567890\r\n"
		"SeaChange-MOD-Data:billing-id=1234567890;purchase-time=1234567890;time-remaining=1234567890;home-id=1234567890;smartcard-id=1234567890;purchase-id=1234567890\r\n"
		"SeaChange-CAS:emm=123456789abcdef;expires=20010101T120000.0Z\r\n"
		"\r\n";

	
		//////////////////////////////////////////////////////////////////////////

	WORD wVersionRequested;
	WSADATA wsaData;

	wVersionRequested = MAKEWORD( 2, 2 );

//	if ( WSAStartup( wVersionRequested, &wsaData ) == 0 ) {
		//SOCKET sd;

		DWORD purid=101;
		char purstr[13];
		sprintf(purstr, "%.12x",purid);
		
		RtspConnectionManager manager;
		manager.start();
		

		RtspResponse back;
		std::string tmpsession;
		try{
//			RtspClientConnection* testconn =manager.createConn("10.7.0.23", "554", purid, 3000);
			RtspClientConnection* testconn =manager.createConn("192.168.12.12", "554", purid, 3000);
			//RtspClientConnection* testconn2 =manager.createConn("192.168.12.12", "554", purid+1, 200);
			if(!testconn) {
				printf("Can not create rtsp connection!\n");
				return -1;
			}
//			RtspRequest request1("SETUP","rtsp://10.7.0.23/mediacluster?00080001.00080001");
			RtspRequest request1("SETUP","rtsp://192.168.12.12/mediacluster?5600001.5600320");
			
			// make up Transport header
			// request1.setHeaderField(KEY_TRANSPORT," MP2T/AVP/UDP;unicast;destination=192.168.80.72;client_port=257;client_mac=000ea672b06c");
			RtspMsgHeader transport(KEY_TRANSPORT);
			transport.setSubHeaderField(" MP2T/AVP/UDP","");
			transport.setSubHeaderField("unicast","");
			transport.setSubHeaderField(KEY_DESTINATION,"225.12.12.126");
			transport.setSubHeaderField(KEY_CLIENTPORT,"257");
			std::string multimac = RtspClientConnection::getDynamicMac("225.12.12.126");
			transport.setSubHeaderField(KEY_CLIENTMAC,multimac);

			// make up SeaChange-Server-Data header
			RtspMsgHeader serverdata(KEY_SEACHANGESERVERDATA);
			serverdata.setSubHeaderField(KEY_NODEGROUPID,"0000000001");
			serverdata.setSubHeaderField(KEY_SMARTCARDID,"0000000001");
			serverdata.setSubHeaderField(KEY_DEVICEID,multimac);
			serverdata.setSubHeaderField(KEY_PURCHASEID, purstr);
			
			RtspMsgHeader moddata(KEY_SEACHANGEMODDATA);
			moddata.setSubHeaderField(KEY_PURCHASEID, purstr);

			RtspMsgHeader entitlement(KEY_SEACHANGEENTITLEMENT);
			entitlement.setSubHeaderField(KEY_PURCHASEID, purstr);
			
			// form the whole request
			request1.setHeaderField(KEY_SEACHANGEVERSION,"1");
			request1.setHeaderField(transport.getName(),transport.toString());
			request1.setHeaderField(KEY_SEACHANGEMAYNOTIFY," ");
			request1.setHeaderField(serverdata.getName(),serverdata.toString());
			//request1.setHeaderField(moddata.getName(), moddata.toString());
			//request1.setHeaderField(entitlement.getName(), entitlement.toString());
			
			bool status = testconn->sendMSG(request1,back);	// !!SETUP
			tmpsession = back.getHeaderField(KEY_SESSION);
			//////////////////////////////////////////////////////////////////////////
			
//			RtspRequest request2("PLAY","rtsp://10.7.0.23/mediacluster?00080001.00080001");
			RtspRequest request2("PLAY","rtsp://192.168.12.12/mediacluster?5600001.5600320");
			
			request2.setHeaderField(KEY_SCALE,"2");

			status = testconn->sendMSG(request2,back);		// !!PLAY
			////////////////////////////////////////////////////////////////////////////

			//transport.setSubHeaderField(KEY_CLIENTPORT,"258");
			//request1.setHeaderField(transport.getName(),transport.toString());
			//
			//status = testconn2->sendMSG(request1,back);	// !!SETUP
			//
			////////////////////////////////////////////////////////////////////////////
			//
			//request2.setHeaderField(KEY_SCALE, "2");

			//status = testconn2->sendMSG(request2, back);	// !!PLAY
			////////////////////////////////////////////////////////////////////////////
			::Sleep(10000);

			RtspRequest request3("TEARDOWN", "*");
			status = testconn->sendMSG(request3, back);

			printf("OK finished\n");

			//status = testconn->sendMSG(request1,back);
//			while (1)
//			{
//				::Sleep(5000);
//			}
			//::Sleep(20000);
			manager.terminate();

			/*status = testconn.sendGET_PARAMETER("*","RTSP/1.0",tmpsession,"text/parameters","presentation_state position scale",back);

			status = testconn.sendPAUSE("*","RTSP/1.0",tmpsession,back);*/
			

			//status = testconn.sendTEARDOWN("rtsp://192.168.12.12/mediacluster?5600001.560000f","RTSP/1.0",tmpsession,"2000","User Requested Teardown",back);
		}
		catch( ZQ::common::Exception errstr)
		{
			printf("%s\n",errstr.getString());
		}
		catch( int errnum) {
			printf("\nerror code: %d\n",errnum);
		}

		

		manager.removeConn( tmpsession);
		//printf("%s",back.toString().c_str());
//	}
	
	return 0;

}

