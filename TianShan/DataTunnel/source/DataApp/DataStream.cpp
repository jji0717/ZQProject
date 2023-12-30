// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// Ice version 3.2.1
// Generated from file `DataStream.ice'

#include <DataStream.h>
#include <Ice/LocalException.h>
#include <Ice/ObjectFactory.h>
#include <Ice/BasicStream.h>
#include <Ice/Object.h>
#include <IceUtil/Iterator.h>
#include <IceUtil/ScopedArray.h>

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

static const ::std::string __TianShanIce__Streamer__DataOnDemand__MuxItem__getName_name = "getName";

static const ::std::string __TianShanIce__Streamer__DataOnDemand__MuxItem__notifyFullUpdate_name = "notifyFullUpdate";

static const ::std::string __TianShanIce__Streamer__DataOnDemand__MuxItem__notifyFileAdded_name = "notifyFileAdded";

static const ::std::string __TianShanIce__Streamer__DataOnDemand__MuxItem__notifyFileDeleted_name = "notifyFileDeleted";

static const ::std::string __TianShanIce__Streamer__DataOnDemand__MuxItem__getInfo_name = "getInfo";

static const ::std::string __TianShanIce__Streamer__DataOnDemand__MuxItem__destroy_name = "destroy";

static const ::std::string __TianShanIce__Streamer__DataOnDemand__MuxItem__setProperties_name = "setProperties";

static const ::std::string __TianShanIce__Streamer__DataOnDemand__MuxItem__getProperties_name = "getProperties";

static const ::std::string __TianShanIce__Streamer__DataOnDemand__DataStream__getName_name = "getName";

static const ::std::string __TianShanIce__Streamer__DataOnDemand__DataStream__getInfo_name = "getInfo";

static const ::std::string __TianShanIce__Streamer__DataOnDemand__DataStream__control_name = "control";

static const ::std::string __TianShanIce__Streamer__DataOnDemand__DataStream__createMuxItem_name = "createMuxItem";

static const ::std::string __TianShanIce__Streamer__DataOnDemand__DataStream__getMuxItem_name = "getMuxItem";

static const ::std::string __TianShanIce__Streamer__DataOnDemand__DataStream__listMuxItems_name = "listMuxItems";

static const ::std::string __TianShanIce__Streamer__DataOnDemand__DataStream__ping_name = "ping";

static const ::std::string __TianShanIce__Streamer__DataOnDemand__DataStreamService__createStreamByApp_name = "createStreamByApp";

static const ::std::string __TianShanIce__Streamer__DataOnDemand__DataStreamService__getStream_name = "getStream";

static const ::std::string __TianShanIce__Streamer__DataOnDemand__DataStreamService__listStreams_name = "listStreams";

static const ::std::string __TianShanIce__Streamer__DataOnDemand__DataStreamService__clear_name = "clear";

static const ::std::string __TianShanIce__Streamer__DataOnDemand__DataStreamService__destroy_name = "destroy";

static const ::std::string __TianShanIce__Streamer__DataOnDemand__DataStreamService__ping_name = "ping";

void
IceInternal::incRef(::TianShanIce::Streamer::DataOnDemand::MuxItem* p)
{
    p->__incRef();
}

void
IceInternal::decRef(::TianShanIce::Streamer::DataOnDemand::MuxItem* p)
{
    p->__decRef();
}

void
IceInternal::incRef(::IceProxy::TianShanIce::Streamer::DataOnDemand::MuxItem* p)
{
    p->__incRef();
}

void
IceInternal::decRef(::IceProxy::TianShanIce::Streamer::DataOnDemand::MuxItem* p)
{
    p->__decRef();
}

void
IceInternal::incRef(::TianShanIce::Streamer::DataOnDemand::DataStream* p)
{
    p->__incRef();
}

void
IceInternal::decRef(::TianShanIce::Streamer::DataOnDemand::DataStream* p)
{
    p->__decRef();
}

void
IceInternal::incRef(::IceProxy::TianShanIce::Streamer::DataOnDemand::DataStream* p)
{
    p->__incRef();
}

void
IceInternal::decRef(::IceProxy::TianShanIce::Streamer::DataOnDemand::DataStream* p)
{
    p->__decRef();
}

void
IceInternal::incRef(::TianShanIce::Streamer::DataOnDemand::DataStreamService* p)
{
    p->__incRef();
}

void
IceInternal::decRef(::TianShanIce::Streamer::DataOnDemand::DataStreamService* p)
{
    p->__decRef();
}

void
IceInternal::incRef(::IceProxy::TianShanIce::Streamer::DataOnDemand::DataStreamService* p)
{
    p->__incRef();
}

void
IceInternal::decRef(::IceProxy::TianShanIce::Streamer::DataOnDemand::DataStreamService* p)
{
    p->__decRef();
}

void
TianShanIce::Streamer::DataOnDemand::__write(::IceInternal::BasicStream* __os, const ::TianShanIce::Streamer::DataOnDemand::MuxItemPrx& v)
{
    __os->write(::Ice::ObjectPrx(v));
}

void
TianShanIce::Streamer::DataOnDemand::__read(::IceInternal::BasicStream* __is, ::TianShanIce::Streamer::DataOnDemand::MuxItemPrx& v)
{
    ::Ice::ObjectPrx proxy;
    __is->read(proxy);
    if(!proxy)
    {
        v = 0;
    }
    else
    {
        v = new ::IceProxy::TianShanIce::Streamer::DataOnDemand::MuxItem;
        v->__copyFrom(proxy);
    }
}

void
TianShanIce::Streamer::DataOnDemand::__write(::IceInternal::BasicStream* __os, const ::TianShanIce::Streamer::DataOnDemand::MuxItemPtr& v)
{
    __os->write(::Ice::ObjectPtr(v));
}

void
TianShanIce::Streamer::DataOnDemand::__write(::IceInternal::BasicStream* __os, const ::TianShanIce::Streamer::DataOnDemand::DataStreamPrx& v)
{
    __os->write(::Ice::ObjectPrx(v));
}

void
TianShanIce::Streamer::DataOnDemand::__read(::IceInternal::BasicStream* __is, ::TianShanIce::Streamer::DataOnDemand::DataStreamPrx& v)
{
    ::Ice::ObjectPrx proxy;
    __is->read(proxy);
    if(!proxy)
    {
        v = 0;
    }
    else
    {
        v = new ::IceProxy::TianShanIce::Streamer::DataOnDemand::DataStream;
        v->__copyFrom(proxy);
    }
}

void
TianShanIce::Streamer::DataOnDemand::__write(::IceInternal::BasicStream* __os, const ::TianShanIce::Streamer::DataOnDemand::DataStreamPtr& v)
{
    __os->write(::Ice::ObjectPtr(v));
}

void
TianShanIce::Streamer::DataOnDemand::__write(::IceInternal::BasicStream* __os, const ::TianShanIce::Streamer::DataOnDemand::DataStreamServicePrx& v)
{
    __os->write(::Ice::ObjectPrx(v));
}

void
TianShanIce::Streamer::DataOnDemand::__read(::IceInternal::BasicStream* __is, ::TianShanIce::Streamer::DataOnDemand::DataStreamServicePrx& v)
{
    ::Ice::ObjectPrx proxy;
    __is->read(proxy);
    if(!proxy)
    {
        v = 0;
    }
    else
    {
        v = new ::IceProxy::TianShanIce::Streamer::DataOnDemand::DataStreamService;
        v->__copyFrom(proxy);
    }
}

void
TianShanIce::Streamer::DataOnDemand::__write(::IceInternal::BasicStream* __os, const ::TianShanIce::Streamer::DataOnDemand::DataStreamServicePtr& v)
{
    __os->write(::Ice::ObjectPtr(v));
}

TianShanIce::Streamer::DataOnDemand::DataStreamError::DataStreamError(const ::std::string& __ice_category, ::Ice::Int __ice_errorCode, const ::std::string& __ice_message) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ServerError(__ice_category, __ice_errorCode, __ice_message)
#else
    ::TianShanIce::ServerError(__ice_category, __ice_errorCode, __ice_message)
#endif
{
}

TianShanIce::Streamer::DataOnDemand::DataStreamError::~DataStreamError() throw()
{
}

static const char* __TianShanIce__Streamer__DataOnDemand__DataStreamError_name = "TianShanIce::Streamer::DataOnDemand::DataStreamError";

::std::string
TianShanIce::Streamer::DataOnDemand::DataStreamError::ice_name() const
{
    return __TianShanIce__Streamer__DataOnDemand__DataStreamError_name;
}

::Ice::Exception*
TianShanIce::Streamer::DataOnDemand::DataStreamError::ice_clone() const
{
    return new DataStreamError(*this);
}

void
TianShanIce::Streamer::DataOnDemand::DataStreamError::ice_throw() const
{
    throw *this;
}

void
TianShanIce::Streamer::DataOnDemand::DataStreamError::__write(::IceInternal::BasicStream* __os) const
{
    __os->write(::std::string("::TianShanIce::Streamer::DataOnDemand::DataStreamError"), false);
    __os->startWriteSlice();
    __os->endWriteSlice();
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ServerError::__write(__os);
#else
    ::TianShanIce::ServerError::__write(__os);
#endif
}

void
TianShanIce::Streamer::DataOnDemand::DataStreamError::__read(::IceInternal::BasicStream* __is, bool __rid)
{
    if(__rid)
    {
        ::std::string myId;
        __is->read(myId, false);
    }
    __is->startReadSlice();
    __is->endReadSlice();
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    ServerError::__read(__is, true);
#else
    ::TianShanIce::ServerError::__read(__is, true);
#endif
}

void
TianShanIce::Streamer::DataOnDemand::DataStreamError::__write(const ::Ice::OutputStreamPtr&) const
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "exception TianShanIce::Streamer::DataOnDemand::DataStreamError was not generated with stream support";
    throw ex;
}

void
TianShanIce::Streamer::DataOnDemand::DataStreamError::__read(const ::Ice::InputStreamPtr&, bool)
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "exception TianShanIce::Streamer::DataOnDemand::DataStreamError was not generated with stream support";
    throw ex;
}

struct __F__TianShanIce__Streamer__DataOnDemand__DataStreamError : public ::IceInternal::UserExceptionFactory
{
    virtual void
    createAndThrow()
    {
        throw ::TianShanIce::Streamer::DataOnDemand::DataStreamError();
    }
};

static ::IceInternal::UserExceptionFactoryPtr __F__TianShanIce__Streamer__DataOnDemand__DataStreamError__Ptr = new __F__TianShanIce__Streamer__DataOnDemand__DataStreamError;

const ::IceInternal::UserExceptionFactoryPtr&
TianShanIce::Streamer::DataOnDemand::DataStreamError::ice_factory()
{
    return __F__TianShanIce__Streamer__DataOnDemand__DataStreamError__Ptr;
}

class __F__TianShanIce__Streamer__DataOnDemand__DataStreamError__Init
{
public:

    __F__TianShanIce__Streamer__DataOnDemand__DataStreamError__Init()
    {
        ::IceInternal::factoryTable->addExceptionFactory("::TianShanIce::Streamer::DataOnDemand::DataStreamError", ::TianShanIce::Streamer::DataOnDemand::DataStreamError::ice_factory());
    }

    ~__F__TianShanIce__Streamer__DataOnDemand__DataStreamError__Init()
    {
        ::IceInternal::factoryTable->removeExceptionFactory("::TianShanIce::Streamer::DataOnDemand::DataStreamError");
    }
};

static __F__TianShanIce__Streamer__DataOnDemand__DataStreamError__Init __F__TianShanIce__Streamer__DataOnDemand__DataStreamError__i;

#ifdef __APPLE__
extern "C" { void __F__TianShanIce__Streamer__DataOnDemand__DataStreamError__initializer() {} }
#endif

TianShanIce::Streamer::DataOnDemand::NameDupException::NameDupException(const ::std::string& __ice_category, ::Ice::Int __ice_errorCode, const ::std::string& __ice_message) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    DataStreamError(__ice_category, __ice_errorCode, __ice_message)
#else
    ::TianShanIce::Streamer::DataOnDemand::DataStreamError(__ice_category, __ice_errorCode, __ice_message)
#endif
{
}

TianShanIce::Streamer::DataOnDemand::NameDupException::~NameDupException() throw()
{
}

static const char* __TianShanIce__Streamer__DataOnDemand__NameDupException_name = "TianShanIce::Streamer::DataOnDemand::NameDupException";

::std::string
TianShanIce::Streamer::DataOnDemand::NameDupException::ice_name() const
{
    return __TianShanIce__Streamer__DataOnDemand__NameDupException_name;
}

::Ice::Exception*
TianShanIce::Streamer::DataOnDemand::NameDupException::ice_clone() const
{
    return new NameDupException(*this);
}

void
TianShanIce::Streamer::DataOnDemand::NameDupException::ice_throw() const
{
    throw *this;
}

void
TianShanIce::Streamer::DataOnDemand::NameDupException::__write(::IceInternal::BasicStream* __os) const
{
    __os->write(::std::string("::TianShanIce::Streamer::DataOnDemand::NameDupException"), false);
    __os->startWriteSlice();
    __os->endWriteSlice();
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    DataStreamError::__write(__os);
#else
    ::TianShanIce::Streamer::DataOnDemand::DataStreamError::__write(__os);
#endif
}

void
TianShanIce::Streamer::DataOnDemand::NameDupException::__read(::IceInternal::BasicStream* __is, bool __rid)
{
    if(__rid)
    {
        ::std::string myId;
        __is->read(myId, false);
    }
    __is->startReadSlice();
    __is->endReadSlice();
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    DataStreamError::__read(__is, true);
#else
    ::TianShanIce::Streamer::DataOnDemand::DataStreamError::__read(__is, true);
#endif
}

void
TianShanIce::Streamer::DataOnDemand::NameDupException::__write(const ::Ice::OutputStreamPtr&) const
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "exception TianShanIce::Streamer::DataOnDemand::NameDupException was not generated with stream support";
    throw ex;
}

void
TianShanIce::Streamer::DataOnDemand::NameDupException::__read(const ::Ice::InputStreamPtr&, bool)
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "exception TianShanIce::Streamer::DataOnDemand::NameDupException was not generated with stream support";
    throw ex;
}

struct __F__TianShanIce__Streamer__DataOnDemand__NameDupException : public ::IceInternal::UserExceptionFactory
{
    virtual void
    createAndThrow()
    {
        throw ::TianShanIce::Streamer::DataOnDemand::NameDupException();
    }
};

static ::IceInternal::UserExceptionFactoryPtr __F__TianShanIce__Streamer__DataOnDemand__NameDupException__Ptr = new __F__TianShanIce__Streamer__DataOnDemand__NameDupException;

const ::IceInternal::UserExceptionFactoryPtr&
TianShanIce::Streamer::DataOnDemand::NameDupException::ice_factory()
{
    return __F__TianShanIce__Streamer__DataOnDemand__NameDupException__Ptr;
}

class __F__TianShanIce__Streamer__DataOnDemand__NameDupException__Init
{
public:

    __F__TianShanIce__Streamer__DataOnDemand__NameDupException__Init()
    {
        ::IceInternal::factoryTable->addExceptionFactory("::TianShanIce::Streamer::DataOnDemand::NameDupException", ::TianShanIce::Streamer::DataOnDemand::NameDupException::ice_factory());
    }

    ~__F__TianShanIce__Streamer__DataOnDemand__NameDupException__Init()
    {
        ::IceInternal::factoryTable->removeExceptionFactory("::TianShanIce::Streamer::DataOnDemand::NameDupException");
    }
};

static __F__TianShanIce__Streamer__DataOnDemand__NameDupException__Init __F__TianShanIce__Streamer__DataOnDemand__NameDupException__i;

#ifdef __APPLE__
extern "C" { void __F__TianShanIce__Streamer__DataOnDemand__NameDupException__initializer() {} }
#endif

void
TianShanIce::Streamer::DataOnDemand::__write(::IceInternal::BasicStream* __os, ::TianShanIce::Streamer::DataOnDemand::CacheType v)
{
    __os->write(static_cast< ::Ice::Byte>(v));
}

void
TianShanIce::Streamer::DataOnDemand::__read(::IceInternal::BasicStream* __is, ::TianShanIce::Streamer::DataOnDemand::CacheType& v)
{
    ::Ice::Byte val;
    __is->read(val);
    v = static_cast< ::TianShanIce::Streamer::DataOnDemand::CacheType>(val);
}

bool
TianShanIce::Streamer::DataOnDemand::MuxItemInfo::operator==(const MuxItemInfo& __rhs) const
{
    return !operator!=(__rhs);
}

bool
TianShanIce::Streamer::DataOnDemand::MuxItemInfo::operator!=(const MuxItemInfo& __rhs) const
{
    if(this == &__rhs)
    {
        return false;
    }
    if(name != __rhs.name)
    {
        return true;
    }
    if(streamId != __rhs.streamId)
    {
        return true;
    }
    if(streamType != __rhs.streamType)
    {
        return true;
    }
    if(bandWidth != __rhs.bandWidth)
    {
        return true;
    }
    if(tag != __rhs.tag)
    {
        return true;
    }
    if(expiration != __rhs.expiration)
    {
        return true;
    }
    if(repeatTime != __rhs.repeatTime)
    {
        return true;
    }
    if(ctype != __rhs.ctype)
    {
        return true;
    }
    if(cacheAddr != __rhs.cacheAddr)
    {
        return true;
    }
    if(encryptMode != __rhs.encryptMode)
    {
        return true;
    }
    if(subchannelCount != __rhs.subchannelCount)
    {
        return true;
    }
    return false;
}

bool
TianShanIce::Streamer::DataOnDemand::MuxItemInfo::operator<(const MuxItemInfo& __rhs) const
{
    if(this == &__rhs)
    {
        return false;
    }
    if(name < __rhs.name)
    {
        return true;
    }
    else if(__rhs.name < name)
    {
        return false;
    }
    if(streamId < __rhs.streamId)
    {
        return true;
    }
    else if(__rhs.streamId < streamId)
    {
        return false;
    }
    if(streamType < __rhs.streamType)
    {
        return true;
    }
    else if(__rhs.streamType < streamType)
    {
        return false;
    }
    if(bandWidth < __rhs.bandWidth)
    {
        return true;
    }
    else if(__rhs.bandWidth < bandWidth)
    {
        return false;
    }
    if(tag < __rhs.tag)
    {
        return true;
    }
    else if(__rhs.tag < tag)
    {
        return false;
    }
    if(expiration < __rhs.expiration)
    {
        return true;
    }
    else if(__rhs.expiration < expiration)
    {
        return false;
    }
    if(repeatTime < __rhs.repeatTime)
    {
        return true;
    }
    else if(__rhs.repeatTime < repeatTime)
    {
        return false;
    }
    if(ctype < __rhs.ctype)
    {
        return true;
    }
    else if(__rhs.ctype < ctype)
    {
        return false;
    }
    if(cacheAddr < __rhs.cacheAddr)
    {
        return true;
    }
    else if(__rhs.cacheAddr < cacheAddr)
    {
        return false;
    }
    if(encryptMode < __rhs.encryptMode)
    {
        return true;
    }
    else if(__rhs.encryptMode < encryptMode)
    {
        return false;
    }
    if(subchannelCount < __rhs.subchannelCount)
    {
        return true;
    }
    else if(__rhs.subchannelCount < subchannelCount)
    {
        return false;
    }
    return false;
}

void
TianShanIce::Streamer::DataOnDemand::MuxItemInfo::__write(::IceInternal::BasicStream* __os) const
{
    __os->write(name);
    __os->write(streamId);
    __os->write(streamType);
    __os->write(bandWidth);
    __os->write(tag);
    __os->write(expiration);
    __os->write(repeatTime);
    ::TianShanIce::Streamer::DataOnDemand::__write(__os, ctype);
    __os->write(cacheAddr);
    __os->write(encryptMode);
    __os->write(subchannelCount);
}

void
TianShanIce::Streamer::DataOnDemand::MuxItemInfo::__read(::IceInternal::BasicStream* __is)
{
    __is->read(name);
    __is->read(streamId);
    __is->read(streamType);
    __is->read(bandWidth);
    __is->read(tag);
    __is->read(expiration);
    __is->read(repeatTime);
    ::TianShanIce::Streamer::DataOnDemand::__read(__is, ctype);
    __is->read(cacheAddr);
    __is->read(encryptMode);
    __is->read(subchannelCount);
}

void
TianShanIce::Streamer::DataOnDemand::__addObject(const MuxItemPtr& p, ::IceInternal::GCCountMap& c)
{
    p->__addObject(c);
}

bool
TianShanIce::Streamer::DataOnDemand::__usesClasses(const MuxItemPtr& p)
{
    return p->__usesClasses();
}

void
TianShanIce::Streamer::DataOnDemand::__decRefUnsafe(const MuxItemPtr& p)
{
    p->__decRefUnsafe();
}

void
TianShanIce::Streamer::DataOnDemand::__clearHandleUnsafe(MuxItemPtr& p)
{
    p.__clearHandleUnsafe();
}

bool
TianShanIce::Streamer::DataOnDemand::StreamInfo::operator==(const StreamInfo& __rhs) const
{
    return !operator!=(__rhs);
}

bool
TianShanIce::Streamer::DataOnDemand::StreamInfo::operator!=(const StreamInfo& __rhs) const
{
    if(this == &__rhs)
    {
        return false;
    }
    if(name != __rhs.name)
    {
        return true;
    }
    if(totalBandwidth != __rhs.totalBandwidth)
    {
        return true;
    }
    if(destAddress != __rhs.destAddress)
    {
        return true;
    }
    if(pmtPid != __rhs.pmtPid)
    {
        return true;
    }
    return false;
}

bool
TianShanIce::Streamer::DataOnDemand::StreamInfo::operator<(const StreamInfo& __rhs) const
{
    if(this == &__rhs)
    {
        return false;
    }
    if(name < __rhs.name)
    {
        return true;
    }
    else if(__rhs.name < name)
    {
        return false;
    }
    if(totalBandwidth < __rhs.totalBandwidth)
    {
        return true;
    }
    else if(__rhs.totalBandwidth < totalBandwidth)
    {
        return false;
    }
    if(destAddress < __rhs.destAddress)
    {
        return true;
    }
    else if(__rhs.destAddress < destAddress)
    {
        return false;
    }
    if(pmtPid < __rhs.pmtPid)
    {
        return true;
    }
    else if(__rhs.pmtPid < pmtPid)
    {
        return false;
    }
    return false;
}

void
TianShanIce::Streamer::DataOnDemand::StreamInfo::__write(::IceInternal::BasicStream* __os) const
{
    __os->write(name);
    __os->write(totalBandwidth);
    __os->write(destAddress);
    __os->write(pmtPid);
}

void
TianShanIce::Streamer::DataOnDemand::StreamInfo::__read(::IceInternal::BasicStream* __is)
{
    __is->read(name);
    __is->read(totalBandwidth);
    __is->read(destAddress);
    __is->read(pmtPid);
}

void
TianShanIce::Streamer::DataOnDemand::__addObject(const DataStreamPtr& p, ::IceInternal::GCCountMap& c)
{
    p->__addObject(c);
}

bool
TianShanIce::Streamer::DataOnDemand::__usesClasses(const DataStreamPtr& p)
{
    return p->__usesClasses();
}

void
TianShanIce::Streamer::DataOnDemand::__decRefUnsafe(const DataStreamPtr& p)
{
    p->__decRefUnsafe();
}

void
TianShanIce::Streamer::DataOnDemand::__clearHandleUnsafe(DataStreamPtr& p)
{
    p.__clearHandleUnsafe();
}

void
TianShanIce::Streamer::DataOnDemand::__addObject(const DataStreamServicePtr& p, ::IceInternal::GCCountMap& c)
{
    p->__addObject(c);
}

bool
TianShanIce::Streamer::DataOnDemand::__usesClasses(const DataStreamServicePtr& p)
{
    return p->__usesClasses();
}

void
TianShanIce::Streamer::DataOnDemand::__decRefUnsafe(const DataStreamServicePtr& p)
{
    p->__decRefUnsafe();
}

void
TianShanIce::Streamer::DataOnDemand::__clearHandleUnsafe(DataStreamServicePtr& p)
{
    p.__clearHandleUnsafe();
}

::std::string
IceProxy::TianShanIce::Streamer::DataOnDemand::MuxItem::getName(const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__TianShanIce__Streamer__DataOnDemand__MuxItem__getName_name);
            __delBase = __getDelegate();
            ::IceDelegate::TianShanIce::Streamer::DataOnDemand::MuxItem* __del = dynamic_cast< ::IceDelegate::TianShanIce::Streamer::DataOnDemand::MuxItem*>(__delBase.get());
            return __del->getName(__ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapper(__delBase, __ex);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

void
IceProxy::TianShanIce::Streamer::DataOnDemand::MuxItem::notifyFullUpdate(const ::std::string& fileName, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __delBase = __getDelegate();
            ::IceDelegate::TianShanIce::Streamer::DataOnDemand::MuxItem* __del = dynamic_cast< ::IceDelegate::TianShanIce::Streamer::DataOnDemand::MuxItem*>(__delBase.get());
            __del->notifyFullUpdate(fileName, __ctx);
            return;
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapper(__delBase, __ex);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

void
IceProxy::TianShanIce::Streamer::DataOnDemand::MuxItem::notifyFileAdded(const ::std::string& fileName, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __delBase = __getDelegate();
            ::IceDelegate::TianShanIce::Streamer::DataOnDemand::MuxItem* __del = dynamic_cast< ::IceDelegate::TianShanIce::Streamer::DataOnDemand::MuxItem*>(__delBase.get());
            __del->notifyFileAdded(fileName, __ctx);
            return;
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapper(__delBase, __ex);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

void
IceProxy::TianShanIce::Streamer::DataOnDemand::MuxItem::notifyFileDeleted(const ::std::string& fileName, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __delBase = __getDelegate();
            ::IceDelegate::TianShanIce::Streamer::DataOnDemand::MuxItem* __del = dynamic_cast< ::IceDelegate::TianShanIce::Streamer::DataOnDemand::MuxItem*>(__delBase.get());
            __del->notifyFileDeleted(fileName, __ctx);
            return;
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapper(__delBase, __ex);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

::TianShanIce::Streamer::DataOnDemand::MuxItemInfo
IceProxy::TianShanIce::Streamer::DataOnDemand::MuxItem::getInfo(const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__TianShanIce__Streamer__DataOnDemand__MuxItem__getInfo_name);
            __delBase = __getDelegate();
            ::IceDelegate::TianShanIce::Streamer::DataOnDemand::MuxItem* __del = dynamic_cast< ::IceDelegate::TianShanIce::Streamer::DataOnDemand::MuxItem*>(__delBase.get());
            return __del->getInfo(__ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapper(__delBase, __ex);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

void
IceProxy::TianShanIce::Streamer::DataOnDemand::MuxItem::destroy(const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__TianShanIce__Streamer__DataOnDemand__MuxItem__destroy_name);
            __delBase = __getDelegate();
            ::IceDelegate::TianShanIce::Streamer::DataOnDemand::MuxItem* __del = dynamic_cast< ::IceDelegate::TianShanIce::Streamer::DataOnDemand::MuxItem*>(__delBase.get());
            __del->destroy(__ctx);
            return;
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapper(__delBase, __ex);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

void
IceProxy::TianShanIce::Streamer::DataOnDemand::MuxItem::setProperties(const ::TianShanIce::Properties& props, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __delBase = __getDelegate();
            ::IceDelegate::TianShanIce::Streamer::DataOnDemand::MuxItem* __del = dynamic_cast< ::IceDelegate::TianShanIce::Streamer::DataOnDemand::MuxItem*>(__delBase.get());
            __del->setProperties(props, __ctx);
            return;
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapper(__delBase, __ex);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

::TianShanIce::Properties
IceProxy::TianShanIce::Streamer::DataOnDemand::MuxItem::getProperties(const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__TianShanIce__Streamer__DataOnDemand__MuxItem__getProperties_name);
            __delBase = __getDelegate();
            ::IceDelegate::TianShanIce::Streamer::DataOnDemand::MuxItem* __del = dynamic_cast< ::IceDelegate::TianShanIce::Streamer::DataOnDemand::MuxItem*>(__delBase.get());
            return __del->getProperties(__ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapper(__delBase, __ex);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

const ::std::string&
IceProxy::TianShanIce::Streamer::DataOnDemand::MuxItem::ice_staticId()
{
    return ::TianShanIce::Streamer::DataOnDemand::MuxItem::ice_staticId();
}

::IceInternal::Handle< ::IceDelegateM::Ice::Object>
IceProxy::TianShanIce::Streamer::DataOnDemand::MuxItem::__createDelegateM()
{
    return ::IceInternal::Handle< ::IceDelegateM::Ice::Object>(new ::IceDelegateM::TianShanIce::Streamer::DataOnDemand::MuxItem);
}

::IceInternal::Handle< ::IceDelegateD::Ice::Object>
IceProxy::TianShanIce::Streamer::DataOnDemand::MuxItem::__createDelegateD()
{
    return ::IceInternal::Handle< ::IceDelegateD::Ice::Object>(new ::IceDelegateD::TianShanIce::Streamer::DataOnDemand::MuxItem);
}

bool
IceProxy::TianShanIce::Streamer::DataOnDemand::operator==(const ::IceProxy::TianShanIce::Streamer::DataOnDemand::MuxItem& l, const ::IceProxy::TianShanIce::Streamer::DataOnDemand::MuxItem& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) == static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::TianShanIce::Streamer::DataOnDemand::operator!=(const ::IceProxy::TianShanIce::Streamer::DataOnDemand::MuxItem& l, const ::IceProxy::TianShanIce::Streamer::DataOnDemand::MuxItem& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) != static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::TianShanIce::Streamer::DataOnDemand::operator<(const ::IceProxy::TianShanIce::Streamer::DataOnDemand::MuxItem& l, const ::IceProxy::TianShanIce::Streamer::DataOnDemand::MuxItem& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) < static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::TianShanIce::Streamer::DataOnDemand::operator<=(const ::IceProxy::TianShanIce::Streamer::DataOnDemand::MuxItem& l, const ::IceProxy::TianShanIce::Streamer::DataOnDemand::MuxItem& r)
{
    return l < r || l == r;
}

bool
IceProxy::TianShanIce::Streamer::DataOnDemand::operator>(const ::IceProxy::TianShanIce::Streamer::DataOnDemand::MuxItem& l, const ::IceProxy::TianShanIce::Streamer::DataOnDemand::MuxItem& r)
{
    return !(l < r) && !(l == r);
}

bool
IceProxy::TianShanIce::Streamer::DataOnDemand::operator>=(const ::IceProxy::TianShanIce::Streamer::DataOnDemand::MuxItem& l, const ::IceProxy::TianShanIce::Streamer::DataOnDemand::MuxItem& r)
{
    return !(l < r);
}

::std::string
IceProxy::TianShanIce::Streamer::DataOnDemand::DataStream::getName(const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__TianShanIce__Streamer__DataOnDemand__DataStream__getName_name);
            __delBase = __getDelegate();
            ::IceDelegate::TianShanIce::Streamer::DataOnDemand::DataStream* __del = dynamic_cast< ::IceDelegate::TianShanIce::Streamer::DataOnDemand::DataStream*>(__delBase.get());
            return __del->getName(__ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapper(__delBase, __ex);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

::TianShanIce::Streamer::DataOnDemand::StreamInfo
IceProxy::TianShanIce::Streamer::DataOnDemand::DataStream::getInfo(const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__TianShanIce__Streamer__DataOnDemand__DataStream__getInfo_name);
            __delBase = __getDelegate();
            ::IceDelegate::TianShanIce::Streamer::DataOnDemand::DataStream* __del = dynamic_cast< ::IceDelegate::TianShanIce::Streamer::DataOnDemand::DataStream*>(__delBase.get());
            return __del->getInfo(__ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapper(__delBase, __ex);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

::Ice::Int
IceProxy::TianShanIce::Streamer::DataOnDemand::DataStream::control(::Ice::Int code, const ::std::string& param, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__TianShanIce__Streamer__DataOnDemand__DataStream__control_name);
            __delBase = __getDelegate();
            ::IceDelegate::TianShanIce::Streamer::DataOnDemand::DataStream* __del = dynamic_cast< ::IceDelegate::TianShanIce::Streamer::DataOnDemand::DataStream*>(__delBase.get());
            return __del->control(code, param, __ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapper(__delBase, __ex);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

::TianShanIce::Streamer::DataOnDemand::MuxItemPrx
IceProxy::TianShanIce::Streamer::DataOnDemand::DataStream::createMuxItem(const ::TianShanIce::Streamer::DataOnDemand::MuxItemInfo& info, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__TianShanIce__Streamer__DataOnDemand__DataStream__createMuxItem_name);
            __delBase = __getDelegate();
            ::IceDelegate::TianShanIce::Streamer::DataOnDemand::DataStream* __del = dynamic_cast< ::IceDelegate::TianShanIce::Streamer::DataOnDemand::DataStream*>(__delBase.get());
            return __del->createMuxItem(info, __ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapper(__delBase, __ex);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

::TianShanIce::Streamer::DataOnDemand::MuxItemPrx
IceProxy::TianShanIce::Streamer::DataOnDemand::DataStream::getMuxItem(const ::std::string& name, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__TianShanIce__Streamer__DataOnDemand__DataStream__getMuxItem_name);
            __delBase = __getDelegate();
            ::IceDelegate::TianShanIce::Streamer::DataOnDemand::DataStream* __del = dynamic_cast< ::IceDelegate::TianShanIce::Streamer::DataOnDemand::DataStream*>(__delBase.get());
            return __del->getMuxItem(name, __ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapper(__delBase, __ex);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

::Ice::StringSeq
IceProxy::TianShanIce::Streamer::DataOnDemand::DataStream::listMuxItems(const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__TianShanIce__Streamer__DataOnDemand__DataStream__listMuxItems_name);
            __delBase = __getDelegate();
            ::IceDelegate::TianShanIce::Streamer::DataOnDemand::DataStream* __del = dynamic_cast< ::IceDelegate::TianShanIce::Streamer::DataOnDemand::DataStream*>(__delBase.get());
            return __del->listMuxItems(__ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapper(__delBase, __ex);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

void
IceProxy::TianShanIce::Streamer::DataOnDemand::DataStream::ping(const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __delBase = __getDelegate();
            ::IceDelegate::TianShanIce::Streamer::DataOnDemand::DataStream* __del = dynamic_cast< ::IceDelegate::TianShanIce::Streamer::DataOnDemand::DataStream*>(__delBase.get());
            __del->ping(__ctx);
            return;
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapper(__delBase, __ex);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

const ::std::string&
IceProxy::TianShanIce::Streamer::DataOnDemand::DataStream::ice_staticId()
{
    return ::TianShanIce::Streamer::DataOnDemand::DataStream::ice_staticId();
}

::IceInternal::Handle< ::IceDelegateM::Ice::Object>
IceProxy::TianShanIce::Streamer::DataOnDemand::DataStream::__createDelegateM()
{
    return ::IceInternal::Handle< ::IceDelegateM::Ice::Object>(new ::IceDelegateM::TianShanIce::Streamer::DataOnDemand::DataStream);
}

::IceInternal::Handle< ::IceDelegateD::Ice::Object>
IceProxy::TianShanIce::Streamer::DataOnDemand::DataStream::__createDelegateD()
{
    return ::IceInternal::Handle< ::IceDelegateD::Ice::Object>(new ::IceDelegateD::TianShanIce::Streamer::DataOnDemand::DataStream);
}

bool
IceProxy::TianShanIce::Streamer::DataOnDemand::operator==(const ::IceProxy::TianShanIce::Streamer::DataOnDemand::DataStream& l, const ::IceProxy::TianShanIce::Streamer::DataOnDemand::DataStream& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) == static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::TianShanIce::Streamer::DataOnDemand::operator!=(const ::IceProxy::TianShanIce::Streamer::DataOnDemand::DataStream& l, const ::IceProxy::TianShanIce::Streamer::DataOnDemand::DataStream& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) != static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::TianShanIce::Streamer::DataOnDemand::operator<(const ::IceProxy::TianShanIce::Streamer::DataOnDemand::DataStream& l, const ::IceProxy::TianShanIce::Streamer::DataOnDemand::DataStream& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) < static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::TianShanIce::Streamer::DataOnDemand::operator<=(const ::IceProxy::TianShanIce::Streamer::DataOnDemand::DataStream& l, const ::IceProxy::TianShanIce::Streamer::DataOnDemand::DataStream& r)
{
    return l < r || l == r;
}

bool
IceProxy::TianShanIce::Streamer::DataOnDemand::operator>(const ::IceProxy::TianShanIce::Streamer::DataOnDemand::DataStream& l, const ::IceProxy::TianShanIce::Streamer::DataOnDemand::DataStream& r)
{
    return !(l < r) && !(l == r);
}

bool
IceProxy::TianShanIce::Streamer::DataOnDemand::operator>=(const ::IceProxy::TianShanIce::Streamer::DataOnDemand::DataStream& l, const ::IceProxy::TianShanIce::Streamer::DataOnDemand::DataStream& r)
{
    return !(l < r);
}

::TianShanIce::Streamer::DataOnDemand::DataStreamPrx
IceProxy::TianShanIce::Streamer::DataOnDemand::DataStreamService::createStreamByApp(const ::TianShanIce::Transport::PathTicketPrx& pathTicket, const ::std::string& space, const ::TianShanIce::Streamer::DataOnDemand::StreamInfo& info, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__TianShanIce__Streamer__DataOnDemand__DataStreamService__createStreamByApp_name);
            __delBase = __getDelegate();
            ::IceDelegate::TianShanIce::Streamer::DataOnDemand::DataStreamService* __del = dynamic_cast< ::IceDelegate::TianShanIce::Streamer::DataOnDemand::DataStreamService*>(__delBase.get());
            return __del->createStreamByApp(pathTicket, space, info, __ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapper(__delBase, __ex);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

::TianShanIce::Streamer::DataOnDemand::DataStreamPrx
IceProxy::TianShanIce::Streamer::DataOnDemand::DataStreamService::getStream(const ::std::string& space, const ::std::string& name, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__TianShanIce__Streamer__DataOnDemand__DataStreamService__getStream_name);
            __delBase = __getDelegate();
            ::IceDelegate::TianShanIce::Streamer::DataOnDemand::DataStreamService* __del = dynamic_cast< ::IceDelegate::TianShanIce::Streamer::DataOnDemand::DataStreamService*>(__delBase.get());
            return __del->getStream(space, name, __ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapper(__delBase, __ex);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

::TianShanIce::StrValues
IceProxy::TianShanIce::Streamer::DataOnDemand::DataStreamService::listStreams(const ::std::string& space, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__TianShanIce__Streamer__DataOnDemand__DataStreamService__listStreams_name);
            __delBase = __getDelegate();
            ::IceDelegate::TianShanIce::Streamer::DataOnDemand::DataStreamService* __del = dynamic_cast< ::IceDelegate::TianShanIce::Streamer::DataOnDemand::DataStreamService*>(__delBase.get());
            return __del->listStreams(space, __ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapper(__delBase, __ex);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

void
IceProxy::TianShanIce::Streamer::DataOnDemand::DataStreamService::clear(const ::std::string& space, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __delBase = __getDelegate();
            ::IceDelegate::TianShanIce::Streamer::DataOnDemand::DataStreamService* __del = dynamic_cast< ::IceDelegate::TianShanIce::Streamer::DataOnDemand::DataStreamService*>(__delBase.get());
            __del->clear(space, __ctx);
            return;
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapper(__delBase, __ex);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

void
IceProxy::TianShanIce::Streamer::DataOnDemand::DataStreamService::destroy(const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __delBase = __getDelegate();
            ::IceDelegate::TianShanIce::Streamer::DataOnDemand::DataStreamService* __del = dynamic_cast< ::IceDelegate::TianShanIce::Streamer::DataOnDemand::DataStreamService*>(__delBase.get());
            __del->destroy(__ctx);
            return;
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapper(__delBase, __ex);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

void
IceProxy::TianShanIce::Streamer::DataOnDemand::DataStreamService::ping(const ::std::string& space, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __delBase = __getDelegate();
            ::IceDelegate::TianShanIce::Streamer::DataOnDemand::DataStreamService* __del = dynamic_cast< ::IceDelegate::TianShanIce::Streamer::DataOnDemand::DataStreamService*>(__delBase.get());
            __del->ping(space, __ctx);
            return;
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapper(__delBase, __ex);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

const ::std::string&
IceProxy::TianShanIce::Streamer::DataOnDemand::DataStreamService::ice_staticId()
{
    return ::TianShanIce::Streamer::DataOnDemand::DataStreamService::ice_staticId();
}

::IceInternal::Handle< ::IceDelegateM::Ice::Object>
IceProxy::TianShanIce::Streamer::DataOnDemand::DataStreamService::__createDelegateM()
{
    return ::IceInternal::Handle< ::IceDelegateM::Ice::Object>(new ::IceDelegateM::TianShanIce::Streamer::DataOnDemand::DataStreamService);
}

::IceInternal::Handle< ::IceDelegateD::Ice::Object>
IceProxy::TianShanIce::Streamer::DataOnDemand::DataStreamService::__createDelegateD()
{
    return ::IceInternal::Handle< ::IceDelegateD::Ice::Object>(new ::IceDelegateD::TianShanIce::Streamer::DataOnDemand::DataStreamService);
}

bool
IceProxy::TianShanIce::Streamer::DataOnDemand::operator==(const ::IceProxy::TianShanIce::Streamer::DataOnDemand::DataStreamService& l, const ::IceProxy::TianShanIce::Streamer::DataOnDemand::DataStreamService& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) == static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::TianShanIce::Streamer::DataOnDemand::operator!=(const ::IceProxy::TianShanIce::Streamer::DataOnDemand::DataStreamService& l, const ::IceProxy::TianShanIce::Streamer::DataOnDemand::DataStreamService& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) != static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::TianShanIce::Streamer::DataOnDemand::operator<(const ::IceProxy::TianShanIce::Streamer::DataOnDemand::DataStreamService& l, const ::IceProxy::TianShanIce::Streamer::DataOnDemand::DataStreamService& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) < static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::TianShanIce::Streamer::DataOnDemand::operator<=(const ::IceProxy::TianShanIce::Streamer::DataOnDemand::DataStreamService& l, const ::IceProxy::TianShanIce::Streamer::DataOnDemand::DataStreamService& r)
{
    return l < r || l == r;
}

bool
IceProxy::TianShanIce::Streamer::DataOnDemand::operator>(const ::IceProxy::TianShanIce::Streamer::DataOnDemand::DataStreamService& l, const ::IceProxy::TianShanIce::Streamer::DataOnDemand::DataStreamService& r)
{
    return !(l < r) && !(l == r);
}

bool
IceProxy::TianShanIce::Streamer::DataOnDemand::operator>=(const ::IceProxy::TianShanIce::Streamer::DataOnDemand::DataStreamService& l, const ::IceProxy::TianShanIce::Streamer::DataOnDemand::DataStreamService& r)
{
    return !(l < r);
}

::std::string
IceDelegateM::TianShanIce::Streamer::DataOnDemand::MuxItem::getName(const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __TianShanIce__Streamer__DataOnDemand__MuxItem__getName_name, ::Ice::Normal, __context, __compress);
    bool __ok = __og.invoke();
    try
    {
        ::IceInternal::BasicStream* __is = __og.is();
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        ::std::string __ret;
        __is->read(__ret);
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateM::TianShanIce::Streamer::DataOnDemand::MuxItem::notifyFullUpdate(const ::std::string& fileName, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __TianShanIce__Streamer__DataOnDemand__MuxItem__notifyFullUpdate_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(fileName);
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __og.abort(__ex);
    }
    bool __ok = __og.invoke();
    try
    {
        ::IceInternal::BasicStream* __is = __og.is();
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateM::TianShanIce::Streamer::DataOnDemand::MuxItem::notifyFileAdded(const ::std::string& fileName, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __TianShanIce__Streamer__DataOnDemand__MuxItem__notifyFileAdded_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(fileName);
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __og.abort(__ex);
    }
    bool __ok = __og.invoke();
    try
    {
        ::IceInternal::BasicStream* __is = __og.is();
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateM::TianShanIce::Streamer::DataOnDemand::MuxItem::notifyFileDeleted(const ::std::string& fileName, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __TianShanIce__Streamer__DataOnDemand__MuxItem__notifyFileDeleted_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(fileName);
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __og.abort(__ex);
    }
    bool __ok = __og.invoke();
    try
    {
        ::IceInternal::BasicStream* __is = __og.is();
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::TianShanIce::Streamer::DataOnDemand::MuxItemInfo
IceDelegateM::TianShanIce::Streamer::DataOnDemand::MuxItem::getInfo(const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __TianShanIce__Streamer__DataOnDemand__MuxItem__getInfo_name, ::Ice::Normal, __context, __compress);
    bool __ok = __og.invoke();
    try
    {
        ::IceInternal::BasicStream* __is = __og.is();
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        ::TianShanIce::Streamer::DataOnDemand::MuxItemInfo __ret;
        __ret.__read(__is);
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateM::TianShanIce::Streamer::DataOnDemand::MuxItem::destroy(const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __TianShanIce__Streamer__DataOnDemand__MuxItem__destroy_name, ::Ice::Normal, __context, __compress);
    bool __ok = __og.invoke();
    try
    {
        ::IceInternal::BasicStream* __is = __og.is();
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::TianShanIce::Streamer::DataOnDemand::DataStreamError&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateM::TianShanIce::Streamer::DataOnDemand::MuxItem::setProperties(const ::TianShanIce::Properties& props, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __TianShanIce__Streamer__DataOnDemand__MuxItem__setProperties_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        ::TianShanIce::__write(__os, props, ::TianShanIce::__U__Properties());
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __og.abort(__ex);
    }
    bool __ok = __og.invoke();
    try
    {
        ::IceInternal::BasicStream* __is = __og.is();
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::TianShanIce::Properties
IceDelegateM::TianShanIce::Streamer::DataOnDemand::MuxItem::getProperties(const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __TianShanIce__Streamer__DataOnDemand__MuxItem__getProperties_name, ::Ice::Normal, __context, __compress);
    bool __ok = __og.invoke();
    try
    {
        ::IceInternal::BasicStream* __is = __og.is();
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        ::TianShanIce::Properties __ret;
        ::TianShanIce::__read(__is, __ret, ::TianShanIce::__U__Properties());
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::std::string
IceDelegateM::TianShanIce::Streamer::DataOnDemand::DataStream::getName(const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __TianShanIce__Streamer__DataOnDemand__DataStream__getName_name, ::Ice::Normal, __context, __compress);
    bool __ok = __og.invoke();
    try
    {
        ::IceInternal::BasicStream* __is = __og.is();
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        ::std::string __ret;
        __is->read(__ret);
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::TianShanIce::Streamer::DataOnDemand::StreamInfo
IceDelegateM::TianShanIce::Streamer::DataOnDemand::DataStream::getInfo(const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __TianShanIce__Streamer__DataOnDemand__DataStream__getInfo_name, ::Ice::Normal, __context, __compress);
    bool __ok = __og.invoke();
    try
    {
        ::IceInternal::BasicStream* __is = __og.is();
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::TianShanIce::Streamer::DataOnDemand::DataStreamError&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        ::TianShanIce::Streamer::DataOnDemand::StreamInfo __ret;
        __ret.__read(__is);
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::Ice::Int
IceDelegateM::TianShanIce::Streamer::DataOnDemand::DataStream::control(::Ice::Int code, const ::std::string& param, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __TianShanIce__Streamer__DataOnDemand__DataStream__control_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(code);
        __os->write(param);
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __og.abort(__ex);
    }
    bool __ok = __og.invoke();
    try
    {
        ::IceInternal::BasicStream* __is = __og.is();
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::TianShanIce::InvalidParameter&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        ::Ice::Int __ret;
        __is->read(__ret);
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::TianShanIce::Streamer::DataOnDemand::MuxItemPrx
IceDelegateM::TianShanIce::Streamer::DataOnDemand::DataStream::createMuxItem(const ::TianShanIce::Streamer::DataOnDemand::MuxItemInfo& info, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __TianShanIce__Streamer__DataOnDemand__DataStream__createMuxItem_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        info.__write(__os);
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __og.abort(__ex);
    }
    bool __ok = __og.invoke();
    try
    {
        ::IceInternal::BasicStream* __is = __og.is();
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::TianShanIce::InvalidParameter&)
            {
                throw;
            }
            catch(const ::TianShanIce::Streamer::DataOnDemand::DataStreamError&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        ::TianShanIce::Streamer::DataOnDemand::MuxItemPrx __ret;
        ::TianShanIce::Streamer::DataOnDemand::__read(__is, __ret);
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::TianShanIce::Streamer::DataOnDemand::MuxItemPrx
IceDelegateM::TianShanIce::Streamer::DataOnDemand::DataStream::getMuxItem(const ::std::string& name, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __TianShanIce__Streamer__DataOnDemand__DataStream__getMuxItem_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(name);
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __og.abort(__ex);
    }
    bool __ok = __og.invoke();
    try
    {
        ::IceInternal::BasicStream* __is = __og.is();
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::TianShanIce::Streamer::DataOnDemand::DataStreamError&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        ::TianShanIce::Streamer::DataOnDemand::MuxItemPrx __ret;
        ::TianShanIce::Streamer::DataOnDemand::__read(__is, __ret);
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::Ice::StringSeq
IceDelegateM::TianShanIce::Streamer::DataOnDemand::DataStream::listMuxItems(const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __TianShanIce__Streamer__DataOnDemand__DataStream__listMuxItems_name, ::Ice::Normal, __context, __compress);
    bool __ok = __og.invoke();
    try
    {
        ::IceInternal::BasicStream* __is = __og.is();
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        ::Ice::StringSeq __ret;
        __is->read(__ret);
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateM::TianShanIce::Streamer::DataOnDemand::DataStream::ping(const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __TianShanIce__Streamer__DataOnDemand__DataStream__ping_name, ::Ice::Normal, __context, __compress);
    bool __ok = __og.invoke();
    try
    {
        ::IceInternal::BasicStream* __is = __og.is();
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::TianShanIce::Streamer::DataOnDemand::DataStreamPrx
IceDelegateM::TianShanIce::Streamer::DataOnDemand::DataStreamService::createStreamByApp(const ::TianShanIce::Transport::PathTicketPrx& pathTicket, const ::std::string& space, const ::TianShanIce::Streamer::DataOnDemand::StreamInfo& info, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __TianShanIce__Streamer__DataOnDemand__DataStreamService__createStreamByApp_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        ::TianShanIce::Transport::__write(__os, pathTicket);
        __os->write(space);
        info.__write(__os);
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __og.abort(__ex);
    }
    bool __ok = __og.invoke();
    try
    {
        ::IceInternal::BasicStream* __is = __og.is();
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::TianShanIce::InvalidParameter&)
            {
                throw;
            }
            catch(const ::TianShanIce::Streamer::DataOnDemand::NameDupException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        ::TianShanIce::Streamer::DataOnDemand::DataStreamPrx __ret;
        ::TianShanIce::Streamer::DataOnDemand::__read(__is, __ret);
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::TianShanIce::Streamer::DataOnDemand::DataStreamPrx
IceDelegateM::TianShanIce::Streamer::DataOnDemand::DataStreamService::getStream(const ::std::string& space, const ::std::string& name, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __TianShanIce__Streamer__DataOnDemand__DataStreamService__getStream_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(space);
        __os->write(name);
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __og.abort(__ex);
    }
    bool __ok = __og.invoke();
    try
    {
        ::IceInternal::BasicStream* __is = __og.is();
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        ::TianShanIce::Streamer::DataOnDemand::DataStreamPrx __ret;
        ::TianShanIce::Streamer::DataOnDemand::__read(__is, __ret);
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::TianShanIce::StrValues
IceDelegateM::TianShanIce::Streamer::DataOnDemand::DataStreamService::listStreams(const ::std::string& space, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __TianShanIce__Streamer__DataOnDemand__DataStreamService__listStreams_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(space);
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __og.abort(__ex);
    }
    bool __ok = __og.invoke();
    try
    {
        ::IceInternal::BasicStream* __is = __og.is();
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        ::TianShanIce::StrValues __ret;
        __is->read(__ret);
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateM::TianShanIce::Streamer::DataOnDemand::DataStreamService::clear(const ::std::string& space, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __TianShanIce__Streamer__DataOnDemand__DataStreamService__clear_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(space);
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __og.abort(__ex);
    }
    bool __ok = __og.invoke();
    try
    {
        ::IceInternal::BasicStream* __is = __og.is();
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateM::TianShanIce::Streamer::DataOnDemand::DataStreamService::destroy(const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __TianShanIce__Streamer__DataOnDemand__DataStreamService__destroy_name, ::Ice::Normal, __context, __compress);
    bool __ok = __og.invoke();
    try
    {
        ::IceInternal::BasicStream* __is = __og.is();
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateM::TianShanIce::Streamer::DataOnDemand::DataStreamService::ping(const ::std::string& space, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __TianShanIce__Streamer__DataOnDemand__DataStreamService__ping_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(space);
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __og.abort(__ex);
    }
    bool __ok = __og.invoke();
    try
    {
        ::IceInternal::BasicStream* __is = __og.is();
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::std::string
IceDelegateD::TianShanIce::Streamer::DataOnDemand::MuxItem::getName(const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __TianShanIce__Streamer__DataOnDemand__MuxItem__getName_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::std::string __ret;
        try
        {
            ::TianShanIce::Streamer::DataOnDemand::MuxItem* __servant = dynamic_cast< ::TianShanIce::Streamer::DataOnDemand::MuxItem*>(__direct.servant().get());
            if(!__servant)
            {
                ::Ice::OperationNotExistException __opEx(__FILE__, __LINE__);
                __opEx.id = __current.id;
                __opEx.facet = __current.facet;
                __opEx.operation = __current.operation;
                throw __opEx;
            }
            try
            {
                __ret = __servant->getName(__current);
            }
            catch(const ::Ice::LocalException& __ex)
            {
                throw ::IceInternal::LocalExceptionWrapper(__ex, false);
            }
        }
        catch(...)
        {
            __direct.destroy();
            throw;
        }
        __direct.destroy();
        return __ret;
    }
}

void
IceDelegateD::TianShanIce::Streamer::DataOnDemand::MuxItem::notifyFullUpdate(const ::std::string& fileName, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __TianShanIce__Streamer__DataOnDemand__MuxItem__notifyFullUpdate_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        try
        {
            ::TianShanIce::Streamer::DataOnDemand::MuxItem* __servant = dynamic_cast< ::TianShanIce::Streamer::DataOnDemand::MuxItem*>(__direct.servant().get());
            if(!__servant)
            {
                ::Ice::OperationNotExistException __opEx(__FILE__, __LINE__);
                __opEx.id = __current.id;
                __opEx.facet = __current.facet;
                __opEx.operation = __current.operation;
                throw __opEx;
            }
            try
            {
                __servant->notifyFullUpdate(fileName, __current);
            }
            catch(const ::Ice::LocalException& __ex)
            {
                throw ::IceInternal::LocalExceptionWrapper(__ex, false);
            }
        }
        catch(...)
        {
            __direct.destroy();
            throw;
        }
        __direct.destroy();
        return;
    }
}

void
IceDelegateD::TianShanIce::Streamer::DataOnDemand::MuxItem::notifyFileAdded(const ::std::string& fileName, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __TianShanIce__Streamer__DataOnDemand__MuxItem__notifyFileAdded_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        try
        {
            ::TianShanIce::Streamer::DataOnDemand::MuxItem* __servant = dynamic_cast< ::TianShanIce::Streamer::DataOnDemand::MuxItem*>(__direct.servant().get());
            if(!__servant)
            {
                ::Ice::OperationNotExistException __opEx(__FILE__, __LINE__);
                __opEx.id = __current.id;
                __opEx.facet = __current.facet;
                __opEx.operation = __current.operation;
                throw __opEx;
            }
            try
            {
                __servant->notifyFileAdded(fileName, __current);
            }
            catch(const ::Ice::LocalException& __ex)
            {
                throw ::IceInternal::LocalExceptionWrapper(__ex, false);
            }
        }
        catch(...)
        {
            __direct.destroy();
            throw;
        }
        __direct.destroy();
        return;
    }
}

void
IceDelegateD::TianShanIce::Streamer::DataOnDemand::MuxItem::notifyFileDeleted(const ::std::string& fileName, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __TianShanIce__Streamer__DataOnDemand__MuxItem__notifyFileDeleted_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        try
        {
            ::TianShanIce::Streamer::DataOnDemand::MuxItem* __servant = dynamic_cast< ::TianShanIce::Streamer::DataOnDemand::MuxItem*>(__direct.servant().get());
            if(!__servant)
            {
                ::Ice::OperationNotExistException __opEx(__FILE__, __LINE__);
                __opEx.id = __current.id;
                __opEx.facet = __current.facet;
                __opEx.operation = __current.operation;
                throw __opEx;
            }
            try
            {
                __servant->notifyFileDeleted(fileName, __current);
            }
            catch(const ::Ice::LocalException& __ex)
            {
                throw ::IceInternal::LocalExceptionWrapper(__ex, false);
            }
        }
        catch(...)
        {
            __direct.destroy();
            throw;
        }
        __direct.destroy();
        return;
    }
}

::TianShanIce::Streamer::DataOnDemand::MuxItemInfo
IceDelegateD::TianShanIce::Streamer::DataOnDemand::MuxItem::getInfo(const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __TianShanIce__Streamer__DataOnDemand__MuxItem__getInfo_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::TianShanIce::Streamer::DataOnDemand::MuxItemInfo __ret;
        try
        {
            ::TianShanIce::Streamer::DataOnDemand::MuxItem* __servant = dynamic_cast< ::TianShanIce::Streamer::DataOnDemand::MuxItem*>(__direct.servant().get());
            if(!__servant)
            {
                ::Ice::OperationNotExistException __opEx(__FILE__, __LINE__);
                __opEx.id = __current.id;
                __opEx.facet = __current.facet;
                __opEx.operation = __current.operation;
                throw __opEx;
            }
            try
            {
                __ret = __servant->getInfo(__current);
            }
            catch(const ::Ice::LocalException& __ex)
            {
                throw ::IceInternal::LocalExceptionWrapper(__ex, false);
            }
        }
        catch(...)
        {
            __direct.destroy();
            throw;
        }
        __direct.destroy();
        return __ret;
    }
}

void
IceDelegateD::TianShanIce::Streamer::DataOnDemand::MuxItem::destroy(const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __TianShanIce__Streamer__DataOnDemand__MuxItem__destroy_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        try
        {
            ::TianShanIce::Streamer::DataOnDemand::MuxItem* __servant = dynamic_cast< ::TianShanIce::Streamer::DataOnDemand::MuxItem*>(__direct.servant().get());
            if(!__servant)
            {
                ::Ice::OperationNotExistException __opEx(__FILE__, __LINE__);
                __opEx.id = __current.id;
                __opEx.facet = __current.facet;
                __opEx.operation = __current.operation;
                throw __opEx;
            }
            try
            {
                __servant->destroy(__current);
            }
            catch(const ::Ice::LocalException& __ex)
            {
                throw ::IceInternal::LocalExceptionWrapper(__ex, false);
            }
        }
        catch(...)
        {
            __direct.destroy();
            throw;
        }
        __direct.destroy();
        return;
    }
}

void
IceDelegateD::TianShanIce::Streamer::DataOnDemand::MuxItem::setProperties(const ::TianShanIce::Properties& props, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __TianShanIce__Streamer__DataOnDemand__MuxItem__setProperties_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        try
        {
            ::TianShanIce::Streamer::DataOnDemand::MuxItem* __servant = dynamic_cast< ::TianShanIce::Streamer::DataOnDemand::MuxItem*>(__direct.servant().get());
            if(!__servant)
            {
                ::Ice::OperationNotExistException __opEx(__FILE__, __LINE__);
                __opEx.id = __current.id;
                __opEx.facet = __current.facet;
                __opEx.operation = __current.operation;
                throw __opEx;
            }
            try
            {
                __servant->setProperties(props, __current);
            }
            catch(const ::Ice::LocalException& __ex)
            {
                throw ::IceInternal::LocalExceptionWrapper(__ex, false);
            }
        }
        catch(...)
        {
            __direct.destroy();
            throw;
        }
        __direct.destroy();
        return;
    }
}

::TianShanIce::Properties
IceDelegateD::TianShanIce::Streamer::DataOnDemand::MuxItem::getProperties(const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __TianShanIce__Streamer__DataOnDemand__MuxItem__getProperties_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::TianShanIce::Properties __ret;
        try
        {
            ::TianShanIce::Streamer::DataOnDemand::MuxItem* __servant = dynamic_cast< ::TianShanIce::Streamer::DataOnDemand::MuxItem*>(__direct.servant().get());
            if(!__servant)
            {
                ::Ice::OperationNotExistException __opEx(__FILE__, __LINE__);
                __opEx.id = __current.id;
                __opEx.facet = __current.facet;
                __opEx.operation = __current.operation;
                throw __opEx;
            }
            try
            {
                __ret = __servant->getProperties(__current);
            }
            catch(const ::Ice::LocalException& __ex)
            {
                throw ::IceInternal::LocalExceptionWrapper(__ex, false);
            }
        }
        catch(...)
        {
            __direct.destroy();
            throw;
        }
        __direct.destroy();
        return __ret;
    }
}

::std::string
IceDelegateD::TianShanIce::Streamer::DataOnDemand::DataStream::getName(const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __TianShanIce__Streamer__DataOnDemand__DataStream__getName_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::std::string __ret;
        try
        {
            ::TianShanIce::Streamer::DataOnDemand::DataStream* __servant = dynamic_cast< ::TianShanIce::Streamer::DataOnDemand::DataStream*>(__direct.servant().get());
            if(!__servant)
            {
                ::Ice::OperationNotExistException __opEx(__FILE__, __LINE__);
                __opEx.id = __current.id;
                __opEx.facet = __current.facet;
                __opEx.operation = __current.operation;
                throw __opEx;
            }
            try
            {
                __ret = __servant->getName(__current);
            }
            catch(const ::Ice::LocalException& __ex)
            {
                throw ::IceInternal::LocalExceptionWrapper(__ex, false);
            }
        }
        catch(...)
        {
            __direct.destroy();
            throw;
        }
        __direct.destroy();
        return __ret;
    }
}

::TianShanIce::Streamer::DataOnDemand::StreamInfo
IceDelegateD::TianShanIce::Streamer::DataOnDemand::DataStream::getInfo(const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __TianShanIce__Streamer__DataOnDemand__DataStream__getInfo_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::TianShanIce::Streamer::DataOnDemand::StreamInfo __ret;
        try
        {
            ::TianShanIce::Streamer::DataOnDemand::DataStream* __servant = dynamic_cast< ::TianShanIce::Streamer::DataOnDemand::DataStream*>(__direct.servant().get());
            if(!__servant)
            {
                ::Ice::OperationNotExistException __opEx(__FILE__, __LINE__);
                __opEx.id = __current.id;
                __opEx.facet = __current.facet;
                __opEx.operation = __current.operation;
                throw __opEx;
            }
            try
            {
                __ret = __servant->getInfo(__current);
            }
            catch(const ::Ice::LocalException& __ex)
            {
                throw ::IceInternal::LocalExceptionWrapper(__ex, false);
            }
        }
        catch(...)
        {
            __direct.destroy();
            throw;
        }
        __direct.destroy();
        return __ret;
    }
}

::Ice::Int
IceDelegateD::TianShanIce::Streamer::DataOnDemand::DataStream::control(::Ice::Int code, const ::std::string& param, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __TianShanIce__Streamer__DataOnDemand__DataStream__control_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::Ice::Int __ret;
        try
        {
            ::TianShanIce::Streamer::DataOnDemand::DataStream* __servant = dynamic_cast< ::TianShanIce::Streamer::DataOnDemand::DataStream*>(__direct.servant().get());
            if(!__servant)
            {
                ::Ice::OperationNotExistException __opEx(__FILE__, __LINE__);
                __opEx.id = __current.id;
                __opEx.facet = __current.facet;
                __opEx.operation = __current.operation;
                throw __opEx;
            }
            try
            {
                __ret = __servant->control(code, param, __current);
            }
            catch(const ::Ice::LocalException& __ex)
            {
                throw ::IceInternal::LocalExceptionWrapper(__ex, false);
            }
        }
        catch(...)
        {
            __direct.destroy();
            throw;
        }
        __direct.destroy();
        return __ret;
    }
}

::TianShanIce::Streamer::DataOnDemand::MuxItemPrx
IceDelegateD::TianShanIce::Streamer::DataOnDemand::DataStream::createMuxItem(const ::TianShanIce::Streamer::DataOnDemand::MuxItemInfo& info, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __TianShanIce__Streamer__DataOnDemand__DataStream__createMuxItem_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::TianShanIce::Streamer::DataOnDemand::MuxItemPrx __ret;
        try
        {
            ::TianShanIce::Streamer::DataOnDemand::DataStream* __servant = dynamic_cast< ::TianShanIce::Streamer::DataOnDemand::DataStream*>(__direct.servant().get());
            if(!__servant)
            {
                ::Ice::OperationNotExistException __opEx(__FILE__, __LINE__);
                __opEx.id = __current.id;
                __opEx.facet = __current.facet;
                __opEx.operation = __current.operation;
                throw __opEx;
            }
            try
            {
                __ret = __servant->createMuxItem(info, __current);
            }
            catch(const ::Ice::LocalException& __ex)
            {
                throw ::IceInternal::LocalExceptionWrapper(__ex, false);
            }
        }
        catch(...)
        {
            __direct.destroy();
            throw;
        }
        __direct.destroy();
        return __ret;
    }
}

::TianShanIce::Streamer::DataOnDemand::MuxItemPrx
IceDelegateD::TianShanIce::Streamer::DataOnDemand::DataStream::getMuxItem(const ::std::string& name, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __TianShanIce__Streamer__DataOnDemand__DataStream__getMuxItem_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::TianShanIce::Streamer::DataOnDemand::MuxItemPrx __ret;
        try
        {
            ::TianShanIce::Streamer::DataOnDemand::DataStream* __servant = dynamic_cast< ::TianShanIce::Streamer::DataOnDemand::DataStream*>(__direct.servant().get());
            if(!__servant)
            {
                ::Ice::OperationNotExistException __opEx(__FILE__, __LINE__);
                __opEx.id = __current.id;
                __opEx.facet = __current.facet;
                __opEx.operation = __current.operation;
                throw __opEx;
            }
            try
            {
                __ret = __servant->getMuxItem(name, __current);
            }
            catch(const ::Ice::LocalException& __ex)
            {
                throw ::IceInternal::LocalExceptionWrapper(__ex, false);
            }
        }
        catch(...)
        {
            __direct.destroy();
            throw;
        }
        __direct.destroy();
        return __ret;
    }
}

::Ice::StringSeq
IceDelegateD::TianShanIce::Streamer::DataOnDemand::DataStream::listMuxItems(const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __TianShanIce__Streamer__DataOnDemand__DataStream__listMuxItems_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::Ice::StringSeq __ret;
        try
        {
            ::TianShanIce::Streamer::DataOnDemand::DataStream* __servant = dynamic_cast< ::TianShanIce::Streamer::DataOnDemand::DataStream*>(__direct.servant().get());
            if(!__servant)
            {
                ::Ice::OperationNotExistException __opEx(__FILE__, __LINE__);
                __opEx.id = __current.id;
                __opEx.facet = __current.facet;
                __opEx.operation = __current.operation;
                throw __opEx;
            }
            try
            {
                __ret = __servant->listMuxItems(__current);
            }
            catch(const ::Ice::LocalException& __ex)
            {
                throw ::IceInternal::LocalExceptionWrapper(__ex, false);
            }
        }
        catch(...)
        {
            __direct.destroy();
            throw;
        }
        __direct.destroy();
        return __ret;
    }
}

void
IceDelegateD::TianShanIce::Streamer::DataOnDemand::DataStream::ping(const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __TianShanIce__Streamer__DataOnDemand__DataStream__ping_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        try
        {
            ::TianShanIce::Streamer::DataOnDemand::DataStream* __servant = dynamic_cast< ::TianShanIce::Streamer::DataOnDemand::DataStream*>(__direct.servant().get());
            if(!__servant)
            {
                ::Ice::OperationNotExistException __opEx(__FILE__, __LINE__);
                __opEx.id = __current.id;
                __opEx.facet = __current.facet;
                __opEx.operation = __current.operation;
                throw __opEx;
            }
            try
            {
                __servant->ping(__current);
            }
            catch(const ::Ice::LocalException& __ex)
            {
                throw ::IceInternal::LocalExceptionWrapper(__ex, false);
            }
        }
        catch(...)
        {
            __direct.destroy();
            throw;
        }
        __direct.destroy();
        return;
    }
}

::TianShanIce::Streamer::DataOnDemand::DataStreamPrx
IceDelegateD::TianShanIce::Streamer::DataOnDemand::DataStreamService::createStreamByApp(const ::TianShanIce::Transport::PathTicketPrx& pathTicket, const ::std::string& space, const ::TianShanIce::Streamer::DataOnDemand::StreamInfo& info, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __TianShanIce__Streamer__DataOnDemand__DataStreamService__createStreamByApp_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::TianShanIce::Streamer::DataOnDemand::DataStreamPrx __ret;
        try
        {
            ::TianShanIce::Streamer::DataOnDemand::DataStreamService* __servant = dynamic_cast< ::TianShanIce::Streamer::DataOnDemand::DataStreamService*>(__direct.servant().get());
            if(!__servant)
            {
                ::Ice::OperationNotExistException __opEx(__FILE__, __LINE__);
                __opEx.id = __current.id;
                __opEx.facet = __current.facet;
                __opEx.operation = __current.operation;
                throw __opEx;
            }
            try
            {
                __ret = __servant->createStreamByApp(pathTicket, space, info, __current);
            }
            catch(const ::Ice::LocalException& __ex)
            {
                throw ::IceInternal::LocalExceptionWrapper(__ex, false);
            }
        }
        catch(...)
        {
            __direct.destroy();
            throw;
        }
        __direct.destroy();
        return __ret;
    }
}

::TianShanIce::Streamer::DataOnDemand::DataStreamPrx
IceDelegateD::TianShanIce::Streamer::DataOnDemand::DataStreamService::getStream(const ::std::string& space, const ::std::string& name, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __TianShanIce__Streamer__DataOnDemand__DataStreamService__getStream_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::TianShanIce::Streamer::DataOnDemand::DataStreamPrx __ret;
        try
        {
            ::TianShanIce::Streamer::DataOnDemand::DataStreamService* __servant = dynamic_cast< ::TianShanIce::Streamer::DataOnDemand::DataStreamService*>(__direct.servant().get());
            if(!__servant)
            {
                ::Ice::OperationNotExistException __opEx(__FILE__, __LINE__);
                __opEx.id = __current.id;
                __opEx.facet = __current.facet;
                __opEx.operation = __current.operation;
                throw __opEx;
            }
            try
            {
                __ret = __servant->getStream(space, name, __current);
            }
            catch(const ::Ice::LocalException& __ex)
            {
                throw ::IceInternal::LocalExceptionWrapper(__ex, false);
            }
        }
        catch(...)
        {
            __direct.destroy();
            throw;
        }
        __direct.destroy();
        return __ret;
    }
}

::TianShanIce::StrValues
IceDelegateD::TianShanIce::Streamer::DataOnDemand::DataStreamService::listStreams(const ::std::string& space, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __TianShanIce__Streamer__DataOnDemand__DataStreamService__listStreams_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::TianShanIce::StrValues __ret;
        try
        {
            ::TianShanIce::Streamer::DataOnDemand::DataStreamService* __servant = dynamic_cast< ::TianShanIce::Streamer::DataOnDemand::DataStreamService*>(__direct.servant().get());
            if(!__servant)
            {
                ::Ice::OperationNotExistException __opEx(__FILE__, __LINE__);
                __opEx.id = __current.id;
                __opEx.facet = __current.facet;
                __opEx.operation = __current.operation;
                throw __opEx;
            }
            try
            {
                __ret = __servant->listStreams(space, __current);
            }
            catch(const ::Ice::LocalException& __ex)
            {
                throw ::IceInternal::LocalExceptionWrapper(__ex, false);
            }
        }
        catch(...)
        {
            __direct.destroy();
            throw;
        }
        __direct.destroy();
        return __ret;
    }
}

void
IceDelegateD::TianShanIce::Streamer::DataOnDemand::DataStreamService::clear(const ::std::string& space, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __TianShanIce__Streamer__DataOnDemand__DataStreamService__clear_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        try
        {
            ::TianShanIce::Streamer::DataOnDemand::DataStreamService* __servant = dynamic_cast< ::TianShanIce::Streamer::DataOnDemand::DataStreamService*>(__direct.servant().get());
            if(!__servant)
            {
                ::Ice::OperationNotExistException __opEx(__FILE__, __LINE__);
                __opEx.id = __current.id;
                __opEx.facet = __current.facet;
                __opEx.operation = __current.operation;
                throw __opEx;
            }
            try
            {
                __servant->clear(space, __current);
            }
            catch(const ::Ice::LocalException& __ex)
            {
                throw ::IceInternal::LocalExceptionWrapper(__ex, false);
            }
        }
        catch(...)
        {
            __direct.destroy();
            throw;
        }
        __direct.destroy();
        return;
    }
}

void
IceDelegateD::TianShanIce::Streamer::DataOnDemand::DataStreamService::destroy(const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __TianShanIce__Streamer__DataOnDemand__DataStreamService__destroy_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        try
        {
            ::TianShanIce::Streamer::DataOnDemand::DataStreamService* __servant = dynamic_cast< ::TianShanIce::Streamer::DataOnDemand::DataStreamService*>(__direct.servant().get());
            if(!__servant)
            {
                ::Ice::OperationNotExistException __opEx(__FILE__, __LINE__);
                __opEx.id = __current.id;
                __opEx.facet = __current.facet;
                __opEx.operation = __current.operation;
                throw __opEx;
            }
            try
            {
                __servant->destroy(__current);
            }
            catch(const ::Ice::LocalException& __ex)
            {
                throw ::IceInternal::LocalExceptionWrapper(__ex, false);
            }
        }
        catch(...)
        {
            __direct.destroy();
            throw;
        }
        __direct.destroy();
        return;
    }
}

void
IceDelegateD::TianShanIce::Streamer::DataOnDemand::DataStreamService::ping(const ::std::string& space, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __TianShanIce__Streamer__DataOnDemand__DataStreamService__ping_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        try
        {
            ::TianShanIce::Streamer::DataOnDemand::DataStreamService* __servant = dynamic_cast< ::TianShanIce::Streamer::DataOnDemand::DataStreamService*>(__direct.servant().get());
            if(!__servant)
            {
                ::Ice::OperationNotExistException __opEx(__FILE__, __LINE__);
                __opEx.id = __current.id;
                __opEx.facet = __current.facet;
                __opEx.operation = __current.operation;
                throw __opEx;
            }
            try
            {
                __servant->ping(space, __current);
            }
            catch(const ::Ice::LocalException& __ex)
            {
                throw ::IceInternal::LocalExceptionWrapper(__ex, false);
            }
        }
        catch(...)
        {
            __direct.destroy();
            throw;
        }
        __direct.destroy();
        return;
    }
}

::Ice::ObjectPtr
TianShanIce::Streamer::DataOnDemand::MuxItem::ice_clone() const
{
    throw ::Ice::CloneNotImplementedException(__FILE__, __LINE__);
    return 0; // to avoid a warning with some compilers
}

static const ::std::string __TianShanIce__Streamer__DataOnDemand__MuxItem_ids[2] =
{
    "::Ice::Object",
    "::TianShanIce::Streamer::DataOnDemand::MuxItem"
};

bool
TianShanIce::Streamer::DataOnDemand::MuxItem::ice_isA(const ::std::string& _s, const ::Ice::Current&) const
{
    return ::std::binary_search(__TianShanIce__Streamer__DataOnDemand__MuxItem_ids, __TianShanIce__Streamer__DataOnDemand__MuxItem_ids + 2, _s);
}

::std::vector< ::std::string>
TianShanIce::Streamer::DataOnDemand::MuxItem::ice_ids(const ::Ice::Current&) const
{
    return ::std::vector< ::std::string>(&__TianShanIce__Streamer__DataOnDemand__MuxItem_ids[0], &__TianShanIce__Streamer__DataOnDemand__MuxItem_ids[2]);
}

const ::std::string&
TianShanIce::Streamer::DataOnDemand::MuxItem::ice_id(const ::Ice::Current&) const
{
    return __TianShanIce__Streamer__DataOnDemand__MuxItem_ids[1];
}

const ::std::string&
TianShanIce::Streamer::DataOnDemand::MuxItem::ice_staticId()
{
    return __TianShanIce__Streamer__DataOnDemand__MuxItem_ids[1];
}

::IceInternal::DispatchStatus
TianShanIce::Streamer::DataOnDemand::MuxItem::___getName(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string __ret = getName(__current);
    __os->write(__ret);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
TianShanIce::Streamer::DataOnDemand::MuxItem::___notifyFullUpdate(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::std::string fileName;
    __is->read(fileName);
    notifyFullUpdate(fileName, __current);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
TianShanIce::Streamer::DataOnDemand::MuxItem::___notifyFileAdded(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::std::string fileName;
    __is->read(fileName);
    notifyFileAdded(fileName, __current);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
TianShanIce::Streamer::DataOnDemand::MuxItem::___notifyFileDeleted(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::std::string fileName;
    __is->read(fileName);
    notifyFileDeleted(fileName, __current);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
TianShanIce::Streamer::DataOnDemand::MuxItem::___getInfo(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __os = __inS.os();
    ::TianShanIce::Streamer::DataOnDemand::MuxItemInfo __ret = getInfo(__current);
    __ret.__write(__os);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
TianShanIce::Streamer::DataOnDemand::MuxItem::___destroy(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __os = __inS.os();
    try
    {
        destroy(__current);
    }
    catch(const ::TianShanIce::Streamer::DataOnDemand::DataStreamError& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
TianShanIce::Streamer::DataOnDemand::MuxItem::___setProperties(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::TianShanIce::Properties props;
    ::TianShanIce::__read(__is, props, ::TianShanIce::__U__Properties());
    setProperties(props, __current);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
TianShanIce::Streamer::DataOnDemand::MuxItem::___getProperties(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __os = __inS.os();
    ::TianShanIce::Properties __ret = getProperties(__current);
    ::TianShanIce::__write(__os, __ret, ::TianShanIce::__U__Properties());
    return ::IceInternal::DispatchOK;
}

static ::std::string __TianShanIce__Streamer__DataOnDemand__MuxItem_all[] =
{
    "destroy",
    "getInfo",
    "getName",
    "getProperties",
    "ice_id",
    "ice_ids",
    "ice_isA",
    "ice_ping",
    "notifyFileAdded",
    "notifyFileDeleted",
    "notifyFullUpdate",
    "setProperties"
};

::IceInternal::DispatchStatus
TianShanIce::Streamer::DataOnDemand::MuxItem::__dispatch(::IceInternal::Incoming& in, const ::Ice::Current& current)
{
    ::std::pair< ::std::string*, ::std::string*> r = ::std::equal_range(__TianShanIce__Streamer__DataOnDemand__MuxItem_all, __TianShanIce__Streamer__DataOnDemand__MuxItem_all + 12, current.operation);
    if(r.first == r.second)
    {
        return ::IceInternal::DispatchOperationNotExist;
    }

    switch(r.first - __TianShanIce__Streamer__DataOnDemand__MuxItem_all)
    {
        case 0:
        {
            return ___destroy(in, current);
        }
        case 1:
        {
            return ___getInfo(in, current);
        }
        case 2:
        {
            return ___getName(in, current);
        }
        case 3:
        {
            return ___getProperties(in, current);
        }
        case 4:
        {
            return ___ice_id(in, current);
        }
        case 5:
        {
            return ___ice_ids(in, current);
        }
        case 6:
        {
            return ___ice_isA(in, current);
        }
        case 7:
        {
            return ___ice_ping(in, current);
        }
        case 8:
        {
            return ___notifyFileAdded(in, current);
        }
        case 9:
        {
            return ___notifyFileDeleted(in, current);
        }
        case 10:
        {
            return ___notifyFullUpdate(in, current);
        }
        case 11:
        {
            return ___setProperties(in, current);
        }
    }

    assert(false);
    return ::IceInternal::DispatchOperationNotExist;
}

static ::std::string __TianShanIce__Streamer__DataOnDemand__MuxItem_freezeWriteOperations[] =
{
    "destroy",
    "notifyFileAdded",
    "notifyFileDeleted",
    "notifyFullUpdate",
    "setProperties"
};

::Ice::Int
TianShanIce::Streamer::DataOnDemand::MuxItem::ice_operationAttributes(const ::std::string& opName) const
{
    ::std::string* end = __TianShanIce__Streamer__DataOnDemand__MuxItem_freezeWriteOperations + 5;
    ::std::string* r = ::std::find(__TianShanIce__Streamer__DataOnDemand__MuxItem_freezeWriteOperations, end, opName);
    return r == end ? 0 : 1;
}

void
TianShanIce::Streamer::DataOnDemand::MuxItem::__write(::IceInternal::BasicStream* __os) const
{
    __os->writeTypeId(ice_staticId());
    __os->startWriteSlice();
    __os->endWriteSlice();
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    Object::__write(__os);
#else
    ::Ice::Object::__write(__os);
#endif
}

void
TianShanIce::Streamer::DataOnDemand::MuxItem::__read(::IceInternal::BasicStream* __is, bool __rid)
{
    if(__rid)
    {
        ::std::string myId;
        __is->readTypeId(myId);
    }
    __is->startReadSlice();
    __is->endReadSlice();
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    Object::__read(__is, true);
#else
    ::Ice::Object::__read(__is, true);
#endif
}

void
TianShanIce::Streamer::DataOnDemand::MuxItem::__write(const ::Ice::OutputStreamPtr&) const
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type TianShanIce::Streamer::DataOnDemand::MuxItem was not generated with stream support";
    throw ex;
}

void
TianShanIce::Streamer::DataOnDemand::MuxItem::__read(const ::Ice::InputStreamPtr&, bool)
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type TianShanIce::Streamer::DataOnDemand::MuxItem was not generated with stream support";
    throw ex;
}

void 
TianShanIce::Streamer::DataOnDemand::__patch__MuxItemPtr(void* __addr, ::Ice::ObjectPtr& v)
{
    ::TianShanIce::Streamer::DataOnDemand::MuxItemPtr* p = static_cast< ::TianShanIce::Streamer::DataOnDemand::MuxItemPtr*>(__addr);
    assert(p);
    *p = ::TianShanIce::Streamer::DataOnDemand::MuxItemPtr::dynamicCast(v);
    if(v && !*p)
    {
        ::Ice::UnexpectedObjectException e(__FILE__, __LINE__);
        e.type = v->ice_id();
        e.expectedType = ::TianShanIce::Streamer::DataOnDemand::MuxItem::ice_staticId();
        throw e;
    }
}

bool
TianShanIce::Streamer::DataOnDemand::operator==(const ::TianShanIce::Streamer::DataOnDemand::MuxItem& l, const ::TianShanIce::Streamer::DataOnDemand::MuxItem& r)
{
    return static_cast<const ::Ice::Object&>(l) == static_cast<const ::Ice::Object&>(r);
}

bool
TianShanIce::Streamer::DataOnDemand::operator!=(const ::TianShanIce::Streamer::DataOnDemand::MuxItem& l, const ::TianShanIce::Streamer::DataOnDemand::MuxItem& r)
{
    return static_cast<const ::Ice::Object&>(l) != static_cast<const ::Ice::Object&>(r);
}

bool
TianShanIce::Streamer::DataOnDemand::operator<(const ::TianShanIce::Streamer::DataOnDemand::MuxItem& l, const ::TianShanIce::Streamer::DataOnDemand::MuxItem& r)
{
    return static_cast<const ::Ice::Object&>(l) < static_cast<const ::Ice::Object&>(r);
}

bool
TianShanIce::Streamer::DataOnDemand::operator<=(const ::TianShanIce::Streamer::DataOnDemand::MuxItem& l, const ::TianShanIce::Streamer::DataOnDemand::MuxItem& r)
{
    return l < r || l == r;
}

bool
TianShanIce::Streamer::DataOnDemand::operator>(const ::TianShanIce::Streamer::DataOnDemand::MuxItem& l, const ::TianShanIce::Streamer::DataOnDemand::MuxItem& r)
{
    return !(l < r) && !(l == r);
}

bool
TianShanIce::Streamer::DataOnDemand::operator>=(const ::TianShanIce::Streamer::DataOnDemand::MuxItem& l, const ::TianShanIce::Streamer::DataOnDemand::MuxItem& r)
{
    return !(l < r);
}

TianShanIce::Streamer::DataOnDemand::DataStream::DataStream(const ::Ice::Identity& __ice_ident) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    Stream(__ice_ident)
#else
    ::TianShanIce::Streamer::Stream(__ice_ident)
#endif

{
}

::Ice::ObjectPtr
TianShanIce::Streamer::DataOnDemand::DataStream::ice_clone() const
{
    throw ::Ice::CloneNotImplementedException(__FILE__, __LINE__);
    return 0; // to avoid a warning with some compilers
}

static const ::std::string __TianShanIce__Streamer__DataOnDemand__DataStream_ids[3] =
{
    "::Ice::Object",
    "::TianShanIce::Streamer::DataOnDemand::DataStream",
    "::TianShanIce::Streamer::Stream"
};

bool
TianShanIce::Streamer::DataOnDemand::DataStream::ice_isA(const ::std::string& _s, const ::Ice::Current&) const
{
    return ::std::binary_search(__TianShanIce__Streamer__DataOnDemand__DataStream_ids, __TianShanIce__Streamer__DataOnDemand__DataStream_ids + 3, _s);
}

::std::vector< ::std::string>
TianShanIce::Streamer::DataOnDemand::DataStream::ice_ids(const ::Ice::Current&) const
{
    return ::std::vector< ::std::string>(&__TianShanIce__Streamer__DataOnDemand__DataStream_ids[0], &__TianShanIce__Streamer__DataOnDemand__DataStream_ids[3]);
}

const ::std::string&
TianShanIce::Streamer::DataOnDemand::DataStream::ice_id(const ::Ice::Current&) const
{
    return __TianShanIce__Streamer__DataOnDemand__DataStream_ids[1];
}

const ::std::string&
TianShanIce::Streamer::DataOnDemand::DataStream::ice_staticId()
{
    return __TianShanIce__Streamer__DataOnDemand__DataStream_ids[1];
}

::IceInternal::DispatchStatus
TianShanIce::Streamer::DataOnDemand::DataStream::___getName(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string __ret = getName(__current);
    __os->write(__ret);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
TianShanIce::Streamer::DataOnDemand::DataStream::___getInfo(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __os = __inS.os();
    try
    {
        ::TianShanIce::Streamer::DataOnDemand::StreamInfo __ret = getInfo(__current);
        __ret.__write(__os);
    }
    catch(const ::TianShanIce::Streamer::DataOnDemand::DataStreamError& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
TianShanIce::Streamer::DataOnDemand::DataStream::___control(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::Ice::Int code;
    ::std::string param;
    __is->read(code);
    __is->read(param);
    try
    {
        ::Ice::Int __ret = control(code, param, __current);
        __os->write(__ret);
    }
    catch(const ::TianShanIce::InvalidParameter& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
TianShanIce::Streamer::DataOnDemand::DataStream::___createMuxItem(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::TianShanIce::Streamer::DataOnDemand::MuxItemInfo info;
    info.__read(__is);
    try
    {
        ::TianShanIce::Streamer::DataOnDemand::MuxItemPrx __ret = createMuxItem(info, __current);
        ::TianShanIce::Streamer::DataOnDemand::__write(__os, __ret);
    }
    catch(const ::TianShanIce::InvalidParameter& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::TianShanIce::Streamer::DataOnDemand::DataStreamError& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
TianShanIce::Streamer::DataOnDemand::DataStream::___getMuxItem(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string name;
    __is->read(name);
    try
    {
        ::TianShanIce::Streamer::DataOnDemand::MuxItemPrx __ret = getMuxItem(name, __current);
        ::TianShanIce::Streamer::DataOnDemand::__write(__os, __ret);
    }
    catch(const ::TianShanIce::Streamer::DataOnDemand::DataStreamError& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
TianShanIce::Streamer::DataOnDemand::DataStream::___listMuxItems(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __os = __inS.os();
    ::Ice::StringSeq __ret = listMuxItems(__current);
    if(__ret.size() == 0)
    {
        __os->writeSize(0);
    }
    else
    {
        __os->write(&__ret[0], &__ret[0] + __ret.size());
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
TianShanIce::Streamer::DataOnDemand::DataStream::___ping(::IceInternal::Incoming&, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Normal, __current.mode);
    ping(__current);
    return ::IceInternal::DispatchOK;
}

static ::std::string __TianShanIce__Streamer__DataOnDemand__DataStream_all[] =
{
    "allocDVBCResource",
    "allotPathTicket",
    "commit",
    "control",
    "createMuxItem",
    "destroy",
    "getCurrentState",
    "getIdent",
    "getInfo",
    "getMuxItem",
    "getName",
    "getProperties",
    "getSession",
    "ice_id",
    "ice_ids",
    "ice_isA",
    "ice_ping",
    "lastError",
    "listMuxItems",
    "pause",
    "pauseEx",
    "ping",
    "play",
    "playEx",
    "resume",
    "seekStream",
    "setConditionalControl",
    "setMuxRate",
    "setProperties",
    "setSpeed"
};

::IceInternal::DispatchStatus
TianShanIce::Streamer::DataOnDemand::DataStream::__dispatch(::IceInternal::Incoming& in, const ::Ice::Current& current)
{
    ::std::pair< ::std::string*, ::std::string*> r = ::std::equal_range(__TianShanIce__Streamer__DataOnDemand__DataStream_all, __TianShanIce__Streamer__DataOnDemand__DataStream_all + 30, current.operation);
    if(r.first == r.second)
    {
        return ::IceInternal::DispatchOperationNotExist;
    }

    switch(r.first - __TianShanIce__Streamer__DataOnDemand__DataStream_all)
    {
        case 0:
        {
            return ___allocDVBCResource(in, current);
        }
        case 1:
        {
            return ___allotPathTicket(in, current);
        }
        case 2:
        {
            return ___commit(in, current);
        }
        case 3:
        {
            return ___control(in, current);
        }
        case 4:
        {
            return ___createMuxItem(in, current);
        }
        case 5:
        {
            return ___destroy(in, current);
        }
        case 6:
        {
            return ___getCurrentState(in, current);
        }
        case 7:
        {
            return ___getIdent(in, current);
        }
        case 8:
        {
            return ___getInfo(in, current);
        }
        case 9:
        {
            return ___getMuxItem(in, current);
        }
        case 10:
        {
            return ___getName(in, current);
        }
        case 11:
        {
            return ___getProperties(in, current);
        }
        case 12:
        {
            return ___getSession(in, current);
        }
        case 13:
        {
            return ___ice_id(in, current);
        }
        case 14:
        {
            return ___ice_ids(in, current);
        }
        case 15:
        {
            return ___ice_isA(in, current);
        }
        case 16:
        {
            return ___ice_ping(in, current);
        }
        case 17:
        {
            return ___lastError(in, current);
        }
        case 18:
        {
            return ___listMuxItems(in, current);
        }
        case 19:
        {
            return ___pause(in, current);
        }
        case 20:
        {
            return ___pauseEx(in, current);
        }
        case 21:
        {
            return ___ping(in, current);
        }
        case 22:
        {
            return ___play(in, current);
        }
        case 23:
        {
            return ___playEx(in, current);
        }
        case 24:
        {
            return ___resume(in, current);
        }
        case 25:
        {
            return ___seekStream(in, current);
        }
        case 26:
        {
            return ___setConditionalControl(in, current);
        }
        case 27:
        {
            return ___setMuxRate(in, current);
        }
        case 28:
        {
            return ___setProperties(in, current);
        }
        case 29:
        {
            return ___setSpeed(in, current);
        }
    }

    assert(false);
    return ::IceInternal::DispatchOperationNotExist;
}

static ::std::string __TianShanIce__Streamer__DataOnDemand__DataStream_freezeWriteOperations[] =
{
    "allocDVBCResource",
    "allotPathTicket",
    "commit",
    "control",
    "createMuxItem",
    "destroy",
    "getCurrentState",
    "getMuxItem",
    "getSession",
    "pause",
    "pauseEx",
    "play",
    "playEx",
    "resume",
    "seekStream",
    "setConditionalControl",
    "setMuxRate",
    "setProperties",
    "setSpeed"
};

::Ice::Int
TianShanIce::Streamer::DataOnDemand::DataStream::ice_operationAttributes(const ::std::string& opName) const
{
    ::std::string* end = __TianShanIce__Streamer__DataOnDemand__DataStream_freezeWriteOperations + 19;
    ::std::string* r = ::std::find(__TianShanIce__Streamer__DataOnDemand__DataStream_freezeWriteOperations, end, opName);
    return r == end ? 0 : 1;
}

void
TianShanIce::Streamer::DataOnDemand::DataStream::__write(::IceInternal::BasicStream* __os) const
{
    __os->writeTypeId(ice_staticId());
    __os->startWriteSlice();
    __os->endWriteSlice();
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    Stream::__write(__os);
#else
    ::TianShanIce::Streamer::Stream::__write(__os);
#endif
}

void
TianShanIce::Streamer::DataOnDemand::DataStream::__read(::IceInternal::BasicStream* __is, bool __rid)
{
    if(__rid)
    {
        ::std::string myId;
        __is->readTypeId(myId);
    }
    __is->startReadSlice();
    __is->endReadSlice();
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    Stream::__read(__is, true);
#else
    ::TianShanIce::Streamer::Stream::__read(__is, true);
#endif
}

void
TianShanIce::Streamer::DataOnDemand::DataStream::__write(const ::Ice::OutputStreamPtr&) const
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type TianShanIce::Streamer::DataOnDemand::DataStream was not generated with stream support";
    throw ex;
}

void
TianShanIce::Streamer::DataOnDemand::DataStream::__read(const ::Ice::InputStreamPtr&, bool)
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type TianShanIce::Streamer::DataOnDemand::DataStream was not generated with stream support";
    throw ex;
}

void 
TianShanIce::Streamer::DataOnDemand::__patch__DataStreamPtr(void* __addr, ::Ice::ObjectPtr& v)
{
    ::TianShanIce::Streamer::DataOnDemand::DataStreamPtr* p = static_cast< ::TianShanIce::Streamer::DataOnDemand::DataStreamPtr*>(__addr);
    assert(p);
    *p = ::TianShanIce::Streamer::DataOnDemand::DataStreamPtr::dynamicCast(v);
    if(v && !*p)
    {
        ::Ice::UnexpectedObjectException e(__FILE__, __LINE__);
        e.type = v->ice_id();
        e.expectedType = ::TianShanIce::Streamer::DataOnDemand::DataStream::ice_staticId();
        throw e;
    }
}

bool
TianShanIce::Streamer::DataOnDemand::operator==(const ::TianShanIce::Streamer::DataOnDemand::DataStream& l, const ::TianShanIce::Streamer::DataOnDemand::DataStream& r)
{
    return static_cast<const ::Ice::Object&>(l) == static_cast<const ::Ice::Object&>(r);
}

bool
TianShanIce::Streamer::DataOnDemand::operator!=(const ::TianShanIce::Streamer::DataOnDemand::DataStream& l, const ::TianShanIce::Streamer::DataOnDemand::DataStream& r)
{
    return static_cast<const ::Ice::Object&>(l) != static_cast<const ::Ice::Object&>(r);
}

bool
TianShanIce::Streamer::DataOnDemand::operator<(const ::TianShanIce::Streamer::DataOnDemand::DataStream& l, const ::TianShanIce::Streamer::DataOnDemand::DataStream& r)
{
    return static_cast<const ::Ice::Object&>(l) < static_cast<const ::Ice::Object&>(r);
}

bool
TianShanIce::Streamer::DataOnDemand::operator<=(const ::TianShanIce::Streamer::DataOnDemand::DataStream& l, const ::TianShanIce::Streamer::DataOnDemand::DataStream& r)
{
    return l < r || l == r;
}

bool
TianShanIce::Streamer::DataOnDemand::operator>(const ::TianShanIce::Streamer::DataOnDemand::DataStream& l, const ::TianShanIce::Streamer::DataOnDemand::DataStream& r)
{
    return !(l < r) && !(l == r);
}

bool
TianShanIce::Streamer::DataOnDemand::operator>=(const ::TianShanIce::Streamer::DataOnDemand::DataStream& l, const ::TianShanIce::Streamer::DataOnDemand::DataStream& r)
{
    return !(l < r);
}

::Ice::ObjectPtr
TianShanIce::Streamer::DataOnDemand::DataStreamService::ice_clone() const
{
    throw ::Ice::CloneNotImplementedException(__FILE__, __LINE__);
    return 0; // to avoid a warning with some compilers
}

static const ::std::string __TianShanIce__Streamer__DataOnDemand__DataStreamService_ids[5] =
{
    "::Ice::Object",
    "::TianShanIce::BaseService",
    "::TianShanIce::ReplicaQuery",
    "::TianShanIce::Streamer::DataOnDemand::DataStreamService",
    "::TianShanIce::Streamer::StreamService"
};

bool
TianShanIce::Streamer::DataOnDemand::DataStreamService::ice_isA(const ::std::string& _s, const ::Ice::Current&) const
{
    return ::std::binary_search(__TianShanIce__Streamer__DataOnDemand__DataStreamService_ids, __TianShanIce__Streamer__DataOnDemand__DataStreamService_ids + 5, _s);
}

::std::vector< ::std::string>
TianShanIce::Streamer::DataOnDemand::DataStreamService::ice_ids(const ::Ice::Current&) const
{
    return ::std::vector< ::std::string>(&__TianShanIce__Streamer__DataOnDemand__DataStreamService_ids[0], &__TianShanIce__Streamer__DataOnDemand__DataStreamService_ids[5]);
}

const ::std::string&
TianShanIce::Streamer::DataOnDemand::DataStreamService::ice_id(const ::Ice::Current&) const
{
    return __TianShanIce__Streamer__DataOnDemand__DataStreamService_ids[3];
}

const ::std::string&
TianShanIce::Streamer::DataOnDemand::DataStreamService::ice_staticId()
{
    return __TianShanIce__Streamer__DataOnDemand__DataStreamService_ids[3];
}

::IceInternal::DispatchStatus
TianShanIce::Streamer::DataOnDemand::DataStreamService::___createStreamByApp(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::TianShanIce::Transport::PathTicketPrx pathTicket;
    ::std::string space;
    ::TianShanIce::Streamer::DataOnDemand::StreamInfo info;
    ::TianShanIce::Transport::__read(__is, pathTicket);
    __is->read(space);
    info.__read(__is);
    try
    {
        ::TianShanIce::Streamer::DataOnDemand::DataStreamPrx __ret = createStreamByApp(pathTicket, space, info, __current);
        ::TianShanIce::Streamer::DataOnDemand::__write(__os, __ret);
    }
    catch(const ::TianShanIce::InvalidParameter& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::TianShanIce::Streamer::DataOnDemand::NameDupException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
TianShanIce::Streamer::DataOnDemand::DataStreamService::___getStream(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string space;
    ::std::string name;
    __is->read(space);
    __is->read(name);
    ::TianShanIce::Streamer::DataOnDemand::DataStreamPrx __ret = getStream(space, name, __current);
    ::TianShanIce::Streamer::DataOnDemand::__write(__os, __ret);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
TianShanIce::Streamer::DataOnDemand::DataStreamService::___listStreams(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string space;
    __is->read(space);
    ::TianShanIce::StrValues __ret = listStreams(space, __current);
    if(__ret.size() == 0)
    {
        __os->writeSize(0);
    }
    else
    {
        __os->write(&__ret[0], &__ret[0] + __ret.size());
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
TianShanIce::Streamer::DataOnDemand::DataStreamService::___clear(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::std::string space;
    __is->read(space);
    clear(space, __current);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
TianShanIce::Streamer::DataOnDemand::DataStreamService::___destroy(::IceInternal::Incoming&, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    destroy(__current);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
TianShanIce::Streamer::DataOnDemand::DataStreamService::___ping(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::std::string space;
    __is->read(space);
    ping(space, __current);
    return ::IceInternal::DispatchOK;
}

static ::std::string __TianShanIce__Streamer__DataOnDemand__DataStreamService_all[] =
{
    "clear",
    "createStream",
    "createStreamByApp",
    "destroy",
    "getAdminUri",
    "getNetId",
    "getState",
    "getStream",
    "ice_id",
    "ice_ids",
    "ice_isA",
    "ice_ping",
    "listStreamers",
    "listStreams",
    "ping",
    "queryReplicas"
};

::IceInternal::DispatchStatus
TianShanIce::Streamer::DataOnDemand::DataStreamService::__dispatch(::IceInternal::Incoming& in, const ::Ice::Current& current)
{
    ::std::pair< ::std::string*, ::std::string*> r = ::std::equal_range(__TianShanIce__Streamer__DataOnDemand__DataStreamService_all, __TianShanIce__Streamer__DataOnDemand__DataStreamService_all + 16, current.operation);
    if(r.first == r.second)
    {
        return ::IceInternal::DispatchOperationNotExist;
    }

    switch(r.first - __TianShanIce__Streamer__DataOnDemand__DataStreamService_all)
    {
        case 0:
        {
            return ___clear(in, current);
        }
        case 1:
        {
            return ___createStream(in, current);
        }
        case 2:
        {
            return ___createStreamByApp(in, current);
        }
        case 3:
        {
            return ___destroy(in, current);
        }
        case 4:
        {
            return ___getAdminUri(in, current);
        }
        case 5:
        {
            return ___getNetId(in, current);
        }
        case 6:
        {
            return ___getState(in, current);
        }
        case 7:
        {
            return ___getStream(in, current);
        }
        case 8:
        {
            return ___ice_id(in, current);
        }
        case 9:
        {
            return ___ice_ids(in, current);
        }
        case 10:
        {
            return ___ice_isA(in, current);
        }
        case 11:
        {
            return ___ice_ping(in, current);
        }
        case 12:
        {
            return ___listStreamers(in, current);
        }
        case 13:
        {
            return ___listStreams(in, current);
        }
        case 14:
        {
            return ___ping(in, current);
        }
        case 15:
        {
            return ___queryReplicas(in, current);
        }
    }

    assert(false);
    return ::IceInternal::DispatchOperationNotExist;
}

static ::std::string __TianShanIce__Streamer__DataOnDemand__DataStreamService_freezeWriteOperations[] =
{
    "clear",
    "createStreamByApp",
    "destroy"
};

::Ice::Int
TianShanIce::Streamer::DataOnDemand::DataStreamService::ice_operationAttributes(const ::std::string& opName) const
{
    ::std::string* end = __TianShanIce__Streamer__DataOnDemand__DataStreamService_freezeWriteOperations + 3;
    ::std::string* r = ::std::find(__TianShanIce__Streamer__DataOnDemand__DataStreamService_freezeWriteOperations, end, opName);
    return r == end ? 0 : 1;
}

void
TianShanIce::Streamer::DataOnDemand::DataStreamService::__write(::IceInternal::BasicStream* __os) const
{
    __os->writeTypeId(ice_staticId());
    __os->startWriteSlice();
    __os->endWriteSlice();
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    Object::__write(__os);
#else
    ::Ice::Object::__write(__os);
#endif
}

void
TianShanIce::Streamer::DataOnDemand::DataStreamService::__read(::IceInternal::BasicStream* __is, bool __rid)
{
    if(__rid)
    {
        ::std::string myId;
        __is->readTypeId(myId);
    }
    __is->startReadSlice();
    __is->endReadSlice();
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    Object::__read(__is, true);
#else
    ::Ice::Object::__read(__is, true);
#endif
}

void
TianShanIce::Streamer::DataOnDemand::DataStreamService::__write(const ::Ice::OutputStreamPtr&) const
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type TianShanIce::Streamer::DataOnDemand::DataStreamService was not generated with stream support";
    throw ex;
}

void
TianShanIce::Streamer::DataOnDemand::DataStreamService::__read(const ::Ice::InputStreamPtr&, bool)
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type TianShanIce::Streamer::DataOnDemand::DataStreamService was not generated with stream support";
    throw ex;
}

void 
TianShanIce::Streamer::DataOnDemand::__patch__DataStreamServicePtr(void* __addr, ::Ice::ObjectPtr& v)
{
    ::TianShanIce::Streamer::DataOnDemand::DataStreamServicePtr* p = static_cast< ::TianShanIce::Streamer::DataOnDemand::DataStreamServicePtr*>(__addr);
    assert(p);
    *p = ::TianShanIce::Streamer::DataOnDemand::DataStreamServicePtr::dynamicCast(v);
    if(v && !*p)
    {
        ::Ice::UnexpectedObjectException e(__FILE__, __LINE__);
        e.type = v->ice_id();
        e.expectedType = ::TianShanIce::Streamer::DataOnDemand::DataStreamService::ice_staticId();
        throw e;
    }
}

bool
TianShanIce::Streamer::DataOnDemand::operator==(const ::TianShanIce::Streamer::DataOnDemand::DataStreamService& l, const ::TianShanIce::Streamer::DataOnDemand::DataStreamService& r)
{
    return static_cast<const ::Ice::Object&>(l) == static_cast<const ::Ice::Object&>(r);
}

bool
TianShanIce::Streamer::DataOnDemand::operator!=(const ::TianShanIce::Streamer::DataOnDemand::DataStreamService& l, const ::TianShanIce::Streamer::DataOnDemand::DataStreamService& r)
{
    return static_cast<const ::Ice::Object&>(l) != static_cast<const ::Ice::Object&>(r);
}

bool
TianShanIce::Streamer::DataOnDemand::operator<(const ::TianShanIce::Streamer::DataOnDemand::DataStreamService& l, const ::TianShanIce::Streamer::DataOnDemand::DataStreamService& r)
{
    return static_cast<const ::Ice::Object&>(l) < static_cast<const ::Ice::Object&>(r);
}

bool
TianShanIce::Streamer::DataOnDemand::operator<=(const ::TianShanIce::Streamer::DataOnDemand::DataStreamService& l, const ::TianShanIce::Streamer::DataOnDemand::DataStreamService& r)
{
    return l < r || l == r;
}

bool
TianShanIce::Streamer::DataOnDemand::operator>(const ::TianShanIce::Streamer::DataOnDemand::DataStreamService& l, const ::TianShanIce::Streamer::DataOnDemand::DataStreamService& r)
{
    return !(l < r) && !(l == r);
}

bool
TianShanIce::Streamer::DataOnDemand::operator>=(const ::TianShanIce::Streamer::DataOnDemand::DataStreamService& l, const ::TianShanIce::Streamer::DataOnDemand::DataStreamService& r)
{
    return !(l < r);
}
