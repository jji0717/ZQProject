#if !defined __EVENTSENDERCFG_H__
#define __EVENTSENDERCFG_H__

#include <ConfigHelper.h>
#include <string>
#include <vector>
using namespace ZQ::common;

struct EventSender
{
	//log item
	std::string		logPath;
	int32			logSize;
	int32			logLevel;
	int32			logNumber;

	//icesender item
	std::string		endPoint;
	int32			timeout;

	//textwrite item
	std::string		recvFilePath;

	static void structure(Config::Holder< EventSender > &holder)
    {
		//log detail
        holder.addDetail("EventSender/Log", "logPath", &EventSender::logPath, NULL, Config::optReadOnly);
        holder.addDetail("EventSender/Log", "logFileSize", &EventSender::logSize, "10240000", Config::optReadOnly);
		holder.addDetail("EventSender/Log", "logLevel", &EventSender::logLevel, "7", Config::optReadOnly);
		holder.addDetail("EventSender/Log", "logNumber", &EventSender::logNumber, "2", Config::optReadOnly);

		//ice detail
		holder.addDetail("EventSender/IceSender/Basic", "endPoint", &EventSender::endPoint, NULL, Config::optReadOnly);
		holder.addDetail("EventSender/IceSender/Basic", "timeout", &EventSender::timeout, NULL, Config::optReadOnly);
		
        //textwriter detail
		holder.addDetail("EventSender/TextWriter/Basic", "receiveFile", &EventSender::recvFilePath, NULL,Config::optReadOnly);		

    }
};


#endif//__EVENTSENDERCFG_H__
