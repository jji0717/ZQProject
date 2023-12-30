#include <iostream>
#include "NTP.h"
#include "FileLog.h"
using namespace  ZQ::common;
int main(void)
{
	ZQ::common::FileLog clientLog(".\\NTPClient.log",7);
	ZQ::common::NTPClient theClient(&clientLog,"202.120.2.101");
	int32 timeOffset=0;
	NTPClient::Txn txnPacket;
	memset(&txnPacket,0,sizeof(txnPacket));
	if(theClient.getServerTime(txnPacket,"192.168.81.119",5689) >0)
	{
		std::cout<<"the server is 192.168.81.119"<<std::endl;
		timeOffset=NTPClient::readOffset(txnPacket);
	}else if (theClient.getServerTime(txnPacket,"time-a.timefreq.bldrdoc.gov") > 0)
	{
		std::cout<<"the server is time-a.timefreq.bldrdoc.gov"<<std::endl;
		timeOffset=NTPClient::readOffset(txnPacket);
	}else if (theClient.getServerTime(txnPacket,"10.15.10.50",5689) >0)
	{
		std::cout<<"the server is 10.15.10.50"<<std::endl;
		timeOffset=NTPClient::readOffset(txnPacket);
	}else if(theClient.getServerTime(txnPacket,"202.120.2.101",123) >0)
	{
		std::cout<<"the server is 202.120.2.101"<<std::endl;
		timeOffset=NTPClient::readOffset(txnPacket);
	}else
	{
		std::cout<<"getSeverTime Error at all server"<<std::endl;
		return -1;
	}
	if (timeOffset <0)
	{
		timeOffset=-timeOffset;
		std::cout<<"the local machine's clock is ahead "<<timeOffset<<" MS"<<std::endl;
	}
	else{
		std::cout<<"the local machine's clock is behind "<<timeOffset<<" MS"<<std::endl;
	}
	std::cout<<"the Delay Time is "<<NTPClient::readDelay(txnPacket)<<" MS"<<std::endl;
	return 0;
}
