// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceStorm/TraceLevels.h>

#include <Ice/Properties.h>

using namespace std;
using namespace IceStorm;

TraceLevels::TraceLevels(const string name, const Ice::PropertiesPtr& properties, const Ice::LoggerPtr& theLogger) :
    topicMgr(0),
    topicMgrCat("TopicManager"),
    topic(0),
    topicCat("Topic"),
    subscriber(0),
    subscriberCat("Subscriber"),
    election(0),
    electionCat("Election"),
    replication(0),
    replicationCat("Replication"),
    traceEventPublish(0),
    traceEventPublishCat("EventPublishTrace"),
    traceEventFlush(0),
    traceEventFlushCat("EventFlushTrace"),
    logger(theLogger)
{
    const string keyBase = name + ".Trace.";
    const_cast<int&>(topicMgr) = properties->getPropertyAsInt(keyBase + topicMgrCat);
    const_cast<int&>(topic) = properties->getPropertyAsInt(keyBase + topicCat);
    const_cast<int&>(subscriber) = properties->getPropertyAsInt(keyBase + subscriberCat);
    const_cast<int&>(election) = properties->getPropertyAsInt(keyBase + electionCat);
    const_cast<int&>(traceEventPublish) = properties->getPropertyAsInt(keyBase + traceEventPublishCat);
    const_cast<int&>(traceEventFlush) = properties->getPropertyAsInt(keyBase + traceEventFlushCat);
}

TraceLevels::~TraceLevels()
{
}
