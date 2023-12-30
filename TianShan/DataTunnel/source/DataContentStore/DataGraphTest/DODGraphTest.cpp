// DODContentStore.cpp : Defines the entry point for the console application.
//

// #include "stdafx.h"
#include "FileLog.h"
#include "DODGraphFactory.h"

using namespace ZQ::Content::Process;


int main(int argc, char* argv[])
{
	ZQ::common::FileLog* flog = new ZQ::common::FileLog("DODGraphTest.log", ZQ::common::Log::L_DEBUG);

	std::string home = "C:\\DataOnDemand\\home";
	DODGraphFactory dodFactory(1, flog, home, 50);
	GraphPool gpool(dodFactory, flog, true);


	for(int i=0; i<1; i++)
	{
//		std::string srcURL = "file://127.0.0.1/c$/DODServer/DODCS/input"; //file://127.0.0.1/c$/DODServer/DCA/Port_TELNAV";
    	std::string srcURL = "file://127.0.0.1/c$/test";

		char desFile[64];
		sprintf(desFile, "TestOutput_%d.out", i+1);

		DODProvisionRequest* request = new DODProvisionRequest(flog, gpool, true, srcURL, desFile, 1000);

	//  std::string srcURL2 = "file://127.0.0.1/c$/DODServer/DCA/Port_TELNAV";
	//	std::string desFile2 = "C:\\DODServer\\DODCS\\output\\port_telnav2.out";
	//
	//	DODProvisionRequest* request2 = new DODProvisionRequest(flog, gpool, true, srcURL2, desFile2);
		std::string nav = "MSG";
/*		DWORD dwNav = 0;
		int nsize = nav.size();
		for(int i=0; i<nsize; i++)
		{
			dwNav |= (nav[i] << (8*(i+1)));
		}*/
		DWORD dwNav = 0;
		unsigned char strTemp;
		for(int i = 0; i<3; i++)
		{	    
			strTemp = nav[i];
			dwNav |= strTemp; 
			dwNav = dwNav << 8;
		}

		request->setParameters(141, 248, 0, 0, dwNav, 1);

	// 	request2->setParameters(100, 1, 5, 1, "NAV", 1);

		DWORD dwStart = GetTickCount();
		request->start();
	// 	request2->start();

		std::string err;
		bool ret = true;
		
		ret = request->waitForCompletion(err);

		DWORD dwEnd = GetTickCount();
		if(!ret)
		{
			printf("Data wrapping failed with reason: %s", err.c_str());
		}
		else
		{
			printf("[%d] Data wrapping succeed, spend %d ms\n", i+1, dwEnd-dwStart);
			
			std::string fullname = home + std::string("\\") + desFile;
//			DeleteFile(fullname.c_str());
		}
		Sleep(500);
	}
//	ret = request2->waitForCompletion(err);
//	if(!ret)
//	{
//		printf("Data wrapping failed with reason: %s", err.c_str());
//	}
//	else
//	{
//		printf("Data wrapping succeed");
//	}
	
	if(flog != NULL)
	{
		delete flog;
	}

	return 0;
}
