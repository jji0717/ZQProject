//#include "BaseService.hpp"

#include "DupMIBSvc.hpp"
#include "TreeDupMIBSvc.hpp"

#include "getopt.h"


int main(int argc, char* argv[])
{ 
	int ch = 0;
	std::string svcSelect("TreeDupMIBSvc");
	BaseService* pInstance;
	std::string opt("-p");
	for(int loop = 1; loop < argc; ++loop) 
	{
		if(0 == opt.compare(argv[loop]) ) 
		{
			svcSelect = argv[loop + 1];
		}
	}

	if (0 == svcSelect.compare("DupMIBSvc"))
	{
		pInstance = new DupMIBSvc;
	}else{
		pInstance = new TreeDupMIBSvc;
	}

	ErrorCode svcStatus = EXIT_FAILED;
	if(SUCCEED != pInstance->init(argc, argv))
	{
		delete pInstance;
		return svcStatus;
	}

	if (SUCCEED != pInstance->start())
	{		
		pInstance->unInit();
	}else{	
		pInstance->stop();
		pInstance->unInit();
		svcStatus = SUCCEED;
	}

	delete pInstance;
	return svcStatus;
}