
#pragma once

#include <string>
#include <list>

typedef struct Message_Info
{
	// The message body was save into the file.
	::std::string fileName;

	//It denote a order number in the queue.
	//bForever=TRUE,means= forever;else has duration
	BOOL bForever;
	
   //each message deleteTime;
   	long  deleteTime;	

	// messageid and nDataType identify message
	::std::string sMessageID;
	
	int GroupId;
}ZQCMessageInfoTINF, *PZQCMessageInfoTINF;

typedef std::list<ZQCMessageInfoTINF> zqMessageList;
