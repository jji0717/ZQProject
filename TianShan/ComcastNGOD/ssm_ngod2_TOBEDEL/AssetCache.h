// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// Ice version 3.1.0

// Freeze types in this file:
// name="com::izq::am::facade::servicesForIce::AssetCache", key="string", value="com::izq::am::facade::servicesForIce::AssetElementCollection"

#ifndef ____AssetCache_h__
#define ____AssetCache_h__

#include <Freeze/Map.h>
#include <LAMFacade.h>

#ifndef ICE_IGNORE_VERSION
#   if ICE_INT_VERSION / 100 != 301
#       error Ice version mismatch!
#   endif
#   if ICE_INT_VERSION % 100 < 0
#       error Ice patch level mismatch!
#   endif
#endif

namespace com
{

namespace izq
{

namespace am
{

namespace facade
{

namespace servicesForIce
{

class AssetCacheKeyCodec
{
public:

    static void write(const ::std::string&, Freeze::Key&, const ::Ice::CommunicatorPtr&);
    static void read(::std::string&, const Freeze::Key&, const ::Ice::CommunicatorPtr&);
    static const std::string& typeId();
};

class AssetCacheValueCodec
{
public:

    static void write(const ::com::izq::am::facade::servicesForIce::AssetElementCollection&, Freeze::Value&, const ::Ice::CommunicatorPtr&);
    static void read(::com::izq::am::facade::servicesForIce::AssetElementCollection&, const Freeze::Value&, const ::Ice::CommunicatorPtr&);
    static const std::string& typeId();
};

typedef Freeze::Map< ::std::string, ::com::izq::am::facade::servicesForIce::AssetElementCollection, AssetCacheKeyCodec, AssetCacheValueCodec, Freeze::IceEncodingCompare > AssetCache;

}

}

}

}

}

#endif
