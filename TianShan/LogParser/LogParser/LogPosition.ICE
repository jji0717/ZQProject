// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
#ifndef _ZQ_EVENTSINK_LOGRECORD_H_
#define _ZQ_EVENTSINK_LOGRECORD_H_

module EventSink
{
struct PositionData
{
	string source;
    string handler;
	long position;
    long stamp;
    long lastUpdated;
};


class LogPosition
{
    ["freeze:read","cpp:const"] string getSourceName();
    ["freeze:read","cpp:const"] string getHandlerName();
    ["freeze:read","cpp:const"] long getLastUpdatedTime();
    ["freeze:read","cpp:const"] void getData(out long position, out long stamp);
    ["freeze:write"] void updateData(long position, long stamp);
    PositionData data;
};

};
#endif //_ZQ_EVENTSINK_LOGRECORD_H_
