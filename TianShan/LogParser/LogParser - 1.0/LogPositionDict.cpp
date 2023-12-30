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

#include <Ice/BasicStream.h>
#include <LogPositionDict.h>

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

void
EventSink::logposmapKeyCodec::write(const ::std::string& v, Freeze::Key& bytes, const ::Ice::CommunicatorPtr& communicator)
{
    IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);
    IceInternal::BasicStream stream(instance.get());
    stream.write(v);
    ::std::vector<Ice::Byte>(stream.b.begin(), stream.b.end()).swap(bytes);
}

void
EventSink::logposmapKeyCodec::read(::std::string& v, const Freeze::Key& bytes, const ::Ice::CommunicatorPtr& communicator)
{
    IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);
    IceInternal::BasicStream stream(instance.get());
    stream.b.resize(bytes.size());
    ::memcpy(&stream.b[0], &bytes[0], bytes.size());
    stream.i = stream.b.begin();
    stream.read(v);
}

static const ::std::string __EventSink__logposmapKeyCodec_typeId = "string";

const ::std::string&
EventSink::logposmapKeyCodec::typeId()
{
    return __EventSink__logposmapKeyCodec_typeId;
}

void
EventSink::logposmapValueCodec::write(const ::EventSink::LogPositionPtr& v, Freeze::Value& bytes, const ::Ice::CommunicatorPtr& communicator)
{
    IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);
    IceInternal::BasicStream stream(instance.get());
    stream.startWriteEncaps();
    ::EventSink::__write(&stream, v);
    stream.writePendingObjects();
    stream.endWriteEncaps();
    ::std::vector<Ice::Byte>(stream.b.begin(), stream.b.end()).swap(bytes);
}

void
EventSink::logposmapValueCodec::read(::EventSink::LogPositionPtr& v, const Freeze::Value& bytes, const ::Ice::CommunicatorPtr& communicator)
{
    IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);
    IceInternal::BasicStream stream(instance.get());
    stream.sliceObjects(false);
    stream.b.resize(bytes.size());
    ::memcpy(&stream.b[0], &bytes[0], bytes.size());
    stream.i = stream.b.begin();
    stream.startReadEncaps();
    stream.read(::EventSink::__patch__LogPositionPtr, &v);
    stream.readPendingObjects();
    stream.endReadEncaps();
}

static const ::std::string __EventSink__logposmapValueCodec_typeId = "::EventSink::LogPosition";

const ::std::string&
EventSink::logposmapValueCodec::typeId()
{
    return __EventSink__logposmapValueCodec_typeId;
}
