// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// Ice version 3.2.1

// Freeze types in this file:
// name="EventSink::logposmap", key="string", value="EventSink::LogPosition"

#ifndef ____LogPositionDict_h__
#define ____LogPositionDict_h__

#include <Freeze/Map.h>
#include <./LogPosition.h>

#ifndef ICE_IGNORE_VERSION
#   if ICE_INT_VERSION / 100 != 302
#       error Ice version mismatch!
#   endif
#   if ICE_INT_VERSION % 100 > 50
#       error Beta header file detected
#   endif
#   if ICE_INT_VERSION % 100 < 1
#       error Ice patch level mismatch!
#   endif
#endif

namespace EventSink
{

class logposmapKeyCodec
{
public:

    static void write(const ::std::string&, Freeze::Key&, const ::Ice::CommunicatorPtr&);
    static void read(::std::string&, const Freeze::Key&, const ::Ice::CommunicatorPtr&);
    static const std::string& typeId();
};

class logposmapValueCodec
{
public:

    static void write(const ::EventSink::LogPositionPtr&, Freeze::Value&, const ::Ice::CommunicatorPtr&);
    static void read(::EventSink::LogPositionPtr&, const Freeze::Value&, const ::Ice::CommunicatorPtr&);
    static const std::string& typeId();
};

typedef Freeze::Map< ::std::string, ::EventSink::LogPositionPtr, logposmapKeyCodec, logposmapValueCodec, Freeze::IceEncodingCompare > logposmap;

}

#endif
