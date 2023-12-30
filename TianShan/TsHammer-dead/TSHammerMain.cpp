// FileName : TSHammerMain.cpp
// Author   : Zheng Junming
// Date     : 2009-11
// Desc     : 

#include <iostream>
#include "./ZQResource.h"
#include "RtspSessionManager.h"

int main(int argc, char* argv[])
{
	ZQHammer::RtspSessionManager rtspSessionManager;
	rtspSessionManager.start(argc, argv);
	return 0;
}
