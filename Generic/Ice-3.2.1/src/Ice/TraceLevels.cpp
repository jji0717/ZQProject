// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/TraceLevels.h>
#include <Ice/Properties.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(TraceLevels* p) { p->__incRef(); }
#ifdef __BCPLUSPLUS__
ICE_API
#endif
void IceInternal::decRef(TraceLevels* p) { p->__decRef(); }

IceInternal::TraceLevels::TraceLevels(const PropertiesPtr& properties) :
    network(0),
    networkCat("Network"),
    protocol(0),
    protocolCat("Protocol"),
    retry(0),
    retryCat("Retry"),
    location(0),
    locationCat("Location"),
    slicing(0),
    slicingCat("Slicing"),
    gc(0),
    gcCat("GC")
{
    const string keyBase = "Ice.Trace.";
    const_cast<int&>(network) = properties->getPropertyAsInt(keyBase + networkCat);
    const_cast<int&>(protocol) = properties->getPropertyAsInt(keyBase + protocolCat);
    const_cast<int&>(retry) = properties->getPropertyAsInt(keyBase + retryCat);
    const_cast<int&>(location) = properties->getPropertyAsInt(keyBase + locationCat);
    const_cast<int&>(slicing) = properties->getPropertyAsInt(keyBase + slicingCat);
    const_cast<int&>(gc) = properties->getPropertyAsInt(keyBase + gcCat);
}
