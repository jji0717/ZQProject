#include "NTP.h"
#include "FileLog.h"
#include "SystemUtils.h"
int main(void)
{
	ZQ::common::FileLog svcLog(".\\NTPSvc.log",7);
	ZQ::common::NTPServer *theSvc=new ZQ::common::NTPServer(&svcLog,"192.168.81.119",5689);
	//theSvc.start();
	theSvc->start();
	SYS::sleep(300000);
	theSvc->stop();
	delete theSvc;
	return 0;
}
