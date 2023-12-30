     // SMInterface.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "smconnector.h"

extern SITV sitv;
int main(int argc, char* argv[])
{
	printf("in!\n");

	CSMConnector connector;


//	connector.SetParam(&sitv,"192.168.80.143","192.168.80.236", 4444);
	
	connector.SetParam(&sitv, 1, "192.168.80.143","192.168.80.143", 4444);
//	connector.SetParam(&sitv, 1, "192.168.80.143", "192.168.80.157", 61232);
	

	connector.start();

/*	while(1)
	{
	Sleep(5000);
	connector.SendEnquireSchedule("<ScheduleInformation>\r\n\
<Port DeviceID=\"1\" PortID=\"1\"/>\r\n\
<Port DeviceID=\"2\" PortID=\"1\"/>\r\n\
</ScheduleInformation>");

	}
*/
	Sleep(5000);
	connector.SendStatusFeedback("<Port ad=\"33\"/>");
	Sleep(5000);
	connector.SendEnquireSchedule("<Port value =\"fad\"/>");
//	connector.SendStatusFeedback(1,2,"192.168.0.1", 5678, "20040910 09:14:25", 1,2,3,4,1,0);

//	Sleep(5000);
//	connector.SendEnquireSchedule(0,1,"192.168.80.143", 4);

//	Sleep(5000);
//	connector.SendQueryFillerList(0, 1);

	connector.wait();
	
//	connector.close();

	return 0;
}

