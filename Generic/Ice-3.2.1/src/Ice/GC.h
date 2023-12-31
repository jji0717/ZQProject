// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GC_H
#define ICE_GC_H

#include <Ice/Config.h>
#include <IceUtil/Thread.h>
#include <IceUtil/Monitor.h>
#include <IceUtil/Mutex.h>

namespace IceInternal
{

struct GCStats
{
    int examined;
    int collected;
    IceUtil::Time time;
};

class GC : public ::IceUtil::Thread, public ::IceUtil::Monitor< ::IceUtil::Mutex>
{
public:

    typedef void (*StatsCallback)(const ::IceInternal::GCStats&);

    GC(int, StatsCallback);
    virtual ~GC();
    virtual void run();
    void stop();
    void collectGarbage();

private:

    enum State { NotStarted, Started, Stopping, Stopped };
    State _state;
    bool _collecting;
    int _interval;
    StatsCallback _statsCallback;
};

}

#endif
