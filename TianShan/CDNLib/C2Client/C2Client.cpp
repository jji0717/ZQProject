// C2Client.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include "HttpProtocol.h"
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/ref.hpp>
#include <HttpSession.h>
#include <HttpDialog.h>
#include <ConnectionFactory.h>
#include <ClientCommand.h>

#ifdef ZQ_OS_LINUX
#include <sys/resource.h>
#endif

void testClient()
{
	ZQ::common::Log* clientLog = new ZQ::common::Log();
	ZQ::common::setGlogger(clientLog);
	ClientCommand cm;
	cm.process();
	ZQ::common::setGlogger(NULL);
	delete clientLog;
	clientLog = NULL;
}
int main(int argc, char* argv[])
{
#ifdef ZQ_OS_LINUX
	struct rlimit rlim;
	rlim.rlim_cur = 64*1024;
	rlim.rlim_max = 640*1024;
	setrlimit( RLIMIT_NOFILE, &rlim);
#endif
	testClient();
	return 0;
}

