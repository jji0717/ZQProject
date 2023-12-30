#include "STVPlaylistManager.h"
//#include <stdlib.h>

using namespace ZQ::common;
//extern ZQ::common::Log* pGlog;
void main()
{
	try
	{
		pGlog = new ScLog("d:\\playlistMod.log", Log::L_DEBUG);
		STVPlaylistManager mgm;
		///*
		
		int status;
		glog(Log::L_DEBUG, "HAHA");
		
		mgm.restore();
		mgm.start();
		
		///*
		ZQ::common::ComInitializer init;

		ZQ::common::XMLPrefDoc doc1(init);
		ZQ::common::XMLPrefDoc doc2(init);
		ZQ::common::XMLPrefDoc doc3(init);
		ZQ::common::XMLPrefDoc doc4(init);
		if (!doc1.open("D:\\channel.xml"))
			return;
		if (!doc2.open("D:\\list.xml"))
			return;
		if (!doc3.open("D:\\filler.xml"))
			return;
		if (!doc4.open("D:\\configure.xml"))
			return;
		ZQ::common::IPreference* root1 = doc1.root();
		ZQ::common::IPreference* root2 = doc2.root();
		ZQ::common::IPreference* root3 = doc3.root();
		ZQ::common::IPreference* root4 = doc4.root();
		

		//mgm.OnConfigration(root3);

		status=mgm.OnConfigration(root4);
		status=mgm.OnNewPlayList(root1, root2, LISTTYPE_BARKER);
		status=mgm.OnNewPlayList(root1, root3, LISTTYPE_FILLER);

//		status=babylist->create(root1, root2, LISTTYPE_PLAYLIST);
//		status=babyfill->create(root1, root3, LISTTYPE_FILLER);
//		//babylist.updateToDB("d:\\testdb\\mirrored.xml");
//		//ASSETS As;
//		mgm.reg(babylist);
//		mgm.reg(babyfill);
//		//mgm.OnIDSquery(&babylist);
//		//bool bchnl=babylist.isMyChannel("1","10.0.1.2","12345");
		
		//*/
		///*
		
		
		//*/
		std::vector<STVPlaylist*> wowlist;
		int sizewow = mgm.getPlaylists(wowlist);

		wowlist[0]->setPLStatus(PLSTAT_PLAYING);
		wowlist[0]->setPlayingAsset(0);

		SSAENotification msg;
		msg.dwAeUID		=3311;
		msg.wOperation	= SAENO_STOP;
		msg.dwStatus	= SAENS_SUCCEED;
		char streambuff[4096];
		mgm.OnSetStreamStatus(wowlist[0]->getPLchannel(), msg, streambuff);

		printf("xml status stream is %d long\n\n", strlen(streambuff));
		printf("%s", streambuff);
		//::Sleep(30000);
		AELIST outAE;
		mgm.OnIDSquery(wowlist[0]);
		mgm.OnGetAElist(101,outAE);
		printf("AE contains\n");
		for(int i=0; i<outAE.AECount; i++) {
			printf("ID: %-8dStart: %-8dEnd: %-8dBitRate: %-8d\n", ((AELEMENT*)outAE.AELlist)[i].AEUID, ((AELEMENT*)outAE.AELlist)[i].StartPos.seconds, ((AELEMENT*)outAE.AELlist)[i].EndPos.seconds, ((AELEMENT*)outAE.AELlist)[i].BitRate);
		}
	
		//*/
		root1->free();
		root2->free();
		root3->free();
		root4->free();
		//*/
			
		mgm.terminate();
	}
	catch (ZQ::common::Exception excp) {
		printf("%s\n", excp.getString());
	}
}