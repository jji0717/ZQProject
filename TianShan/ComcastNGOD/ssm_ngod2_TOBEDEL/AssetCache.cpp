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

#include <Ice/BasicStream.h>
#include <AssetCache.h>

#ifndef ICE_IGNORE_VERSION
#   if ICE_INT_VERSION / 100 != 301
#       error Ice version mismatch!
#   endif
#   if ICE_INT_VERSION % 100 < 0
#       error Ice patch level mismatch!
#   endif
#endif

void
com::izq::am::facade::servicesForIce::AssetCacheKeyCodec::write(const ::std::string& v, Freeze::Key& bytes, const ::Ice::CommunicatorPtr& communicator)
{
    IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);
    IceInternal::BasicStream stream(instance.get());
    stream.write(v);
    ::std::vector<Ice::Byte>(stream.b.begin(), stream.b.end()).swap(bytes);
}

void
com::izq::am::facade::servicesForIce::AssetCacheKeyCodec::read(::std::string& v, const Freeze::Key& bytes, const ::Ice::CommunicatorPtr& communicator)
{
    IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);
    IceInternal::BasicStream stream(instance.get());
    stream.b.resize(bytes.size());
    ::memcpy(&stream.b[0], &bytes[0], bytes.size());
    stream.i = stream.b.begin();
    stream.read(v);
}

static const ::std::string __com__izq__am__facade__servicesForIce__AssetCacheKeyCodec_typeId = "string";

const ::std::string&
com::izq::am::facade::servicesForIce::AssetCacheKeyCodec::typeId()
{
    return __com__izq__am__facade__servicesForIce__AssetCacheKeyCodec_typeId;
}

void
com::izq::am::facade::servicesForIce::AssetCacheValueCodec::write(const ::com::izq::am::facade::servicesForIce::AssetElementCollection& v, Freeze::Value& bytes, const ::Ice::CommunicatorPtr& communicator)
{
    IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);
    IceInternal::BasicStream stream(instance.get());
    stream.startWriteEncaps();
    if(v.size() == 0)
    {
	stream.writeSize(0);
    }
    else
    {
	::com::izq::am::facade::servicesForIce::__write(&stream, &v[0], &v[0] + v.size(), ::com::izq::am::facade::servicesForIce::__U__AssetElementCollection());
    }
    stream.endWriteEncaps();
    ::std::vector<Ice::Byte>(stream.b.begin(), stream.b.end()).swap(bytes);
}

void
com::izq::am::facade::servicesForIce::AssetCacheValueCodec::read(::com::izq::am::facade::servicesForIce::AssetElementCollection& v, const Freeze::Value& bytes, const ::Ice::CommunicatorPtr& communicator)
{
    IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);
    IceInternal::BasicStream stream(instance.get());
    stream.b.resize(bytes.size());
    ::memcpy(&stream.b[0], &bytes[0], bytes.size());
    stream.i = stream.b.begin();
    stream.startReadEncaps();
    ::com::izq::am::facade::servicesForIce::__read(&stream, v, ::com::izq::am::facade::servicesForIce::__U__AssetElementCollection());
    stream.endReadEncaps();
}

static const ::std::string __com__izq__am__facade__servicesForIce__AssetCacheValueCodec_typeId = "::com::izq::am::facade::servicesForIce::AssetElementCollection";

const ::std::string&
com::izq::am::facade::servicesForIce::AssetCacheValueCodec::typeId()
{
    return __com__izq__am__facade__servicesForIce__AssetCacheValueCodec_typeId;
}
