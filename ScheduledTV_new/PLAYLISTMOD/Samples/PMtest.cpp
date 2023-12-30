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
		
		STVChannel::setBasePath("D:\\ITVPlayback\\DB\\");
		
		glog(ZQ::common::Log::L_NOTICE, "PM started");
		mgm.start();
		
		///*
		
		ZQ::common::XMLPrefDoc doc1(STVPlaylistManager::_coInit);
		ZQ::common::XMLPrefDoc doc2(STVPlaylistManager::_coInit);
		ZQ::common::XMLPrefDoc doc3(STVPlaylistManager::_coInit);
		if (!doc1.open("D:\\channel.xml"))
			return;
		if (!doc2.open("D:\\normal.xml"))
			return;
		if (!doc3.open("D:\\barker.xml"))
			return;
		ZQ::common::IPreference* root1 = doc1.root();
		ZQ::common::IPreference* root2 = doc2.root();
		ZQ::common::IPreference* root3 = doc3.root();
		

		status=mgm.OnNewPlayList("1000#0", root1, root2, LISTTYPE_NORMAL);
		status=mgm.OnNewPlayList("1001#0", root1, root3, LISTTYPE_BARKER);

		//*/
		root1->free();
		root2->free();
		root3->free();
		//*/
		
		Sleep(1800000);
		mgm.terminate();
		glog(ZQ::common::Log::L_NOTICE, "PM stopped");

		delete pGlog;
	}
	catch (ZQ::common::Exception excp) {
		printf("%s\n", excp.getString());
	}
}