// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// Ice version 3.1.1
// Generated from file `DataStream.ice'

#include <DataStream.h>
#include <Ice/LocalException.h>
#include <Ice/ObjectFactory.h>
#include <Ice/BasicStream.h>
#include <Ice/Object.h>
#include <IceUtil/Iterator.h>

#ifndef ICE_IGNORE_VERSION
#   if ICE_INT_VERSION / 100 != 301
#       error Ice version mismatch!
#   endif
#   if ICE_INT_VERSION % 100 < 1
#       error Ice patch level mismatch!
#   endif
#endif

static const ::std::string __DataOnDemand__MuxItem__getName_name = "getName";

static const ::std::string __DataOnDemand__MuxItem__notifyFullUpdate_name = "notifyFullUpdate";

static const ::std::string __DataOnDemand__MuxItem__notifyFileAdded_name = "notifyFileAdded";

static const ::std::string __DataOnDemand__MuxItem__notifyFileDeleted_name = "notifyFileDeleted";

static const ::std::string __DataOnDemand__MuxItem__getInfo_name = "getInfo";

static const ::std::string __DataOnDemand__MuxItem__destory_name = "destory";

static const ::std::string __DataOnDemand__MuxItem__setProperies_name = "setProperies";

static const ::std::string __DataOnDemand__MuxItem__getProperties_name = "getProperties";

static const ::std::string __DataOnDemand__DataStream__getName_name = "getName";

static const ::std::string __DataOnDemand__DataStream__getInfo_name = "getInfo";

static const ::std::string __DataOnDemand__DataStream__createMuxItem_name = "createMuxItem";

static const ::std::string __DataOnDemand__DataStream__getMuxItem_name = "getMuxItem";

static const ::std::string __DataOnDemand__DataStream__listMuxItems_name = "listMuxItems";

static const ::std::string __DataOnDemand__DataStream__setProperies_name = "setProperies";

static const ::std::string __DataOnDemand__DataStream__getProperties_name = "getProperties";

static const ::std::string __DataOnDemand__DataStreamService__createStreamByApp_name = "createStreamByApp";

static const ::std::string __DataOnDemand__DataStreamService__getStream_name = "getStream";

static const ::std::string __DataOnDemand__DataStreamService__setProperies_name = "setProperies";

static const ::std::string __DataOnDemand__DataStreamService__getProperties_name = "getProperties";

void
IceInternal::incRef(::DataOnDemand::MuxItem* p)
{
    p->__incRef();
}

void
IceInternal::decRef(::DataOnDemand::MuxItem* p)
{
    p->__decRef();
}

void
IceInternal::incRef(::IceProxy::DataOnDemand::MuxItem* p)
{
    p->__incRef();
}

void
IceInternal::decRef(::IceProxy::DataOnDemand::MuxItem* p)
{
    p->__decRef();
}

void
IceInternal::incRef(::DataOnDemand::DataStream* p)
{
    p->__incRef();
}

void
IceInternal::decRef(::DataOnDemand::DataStream* p)
{
    p->__decRef();
}

void
IceInternal::incRef(::IceProxy::DataOnDemand::DataStream* p)
{
    p->__incRef();
}

void
IceInternal::decRef(::IceProxy::DataOnDemand::DataStream* p)
{
    p->__decRef();
}

void
IceInternal::incRef(::DataOnDemand::DataStreamService* p)
{
    p->__incRef();
}

void
IceInternal::decRef(::DataOnDemand::DataStreamService* p)
{
    p->__decRef();
}

void
IceInternal::incRef(::IceProxy::DataOnDemand::DataStreamService* p)
{
    p->__incRef();
}

void
IceInternal::decRef(::IceProxy::DataOnDemand::DataStreamService* p)
{
    p->__decRef();
}

void
DataOnDemand::__write(::IceInternal::BasicStream* __os, const ::DataOnDemand::MuxItemPrx& v)
{
    __os->write(::Ice::ObjectPrx(v));
}

void
DataOnDemand::__read(::IceInternal::BasicStream* __is, ::DataOnDemand::MuxItemPrx& v)
{
    ::Ice::ObjectPrx proxy;
    __is->read(proxy);
    if(!proxy)
    {
	v = 0;
    }
    else
    {
	v = new ::IceProxy::DataOnDemand::MuxItem;
	v->__copyFrom(proxy);
    }
}

void
DataOnDemand::__write(::IceInternal::BasicStream* __os, const ::DataOnDemand::MuxItemPtr& v)
{
    __os->write(::Ice::ObjectPtr(v));
}

void
DataOnDemand::__write(::IceInternal::BasicStream* __os, const ::DataOnDemand::DataStreamPrx& v)
{
    __os->write(::Ice::ObjectPrx(v));
}

void
DataOnDemand::__read(::IceInternal::BasicStream* __is, ::DataOnDemand::DataStreamPrx& v)
{
    ::Ice::ObjectPrx proxy;
    __is->read(proxy);
    if(!proxy)
    {
	v = 0;
    }
    else
    {
	v = new ::IceProxy::DataOnDemand::DataStream;
	v->__copyFrom(proxy);
    }
}

void
DataOnDemand::__write(::IceInternal::BasicStream* __os, const ::DataOnDemand::DataStreamPtr& v)
{
    __os->write(::Ice::ObjectPtr(v));
}

void
DataOnDemand::__write(::IceInternal::BasicStream* __os, const ::DataOnDemand::DataStreamServicePrx& v)
{
    __os->write(::Ice::ObjectPrx(v));
}

void
DataOnDemand::__read(::IceInternal::BasicStream* __is, ::DataOnDemand::DataStreamServicePrx& v)
{
    ::Ice::ObjectPrx proxy;
    __is->read(proxy);
    if(!proxy)
    {
	v = 0;
    }
    else
    {
	v = new ::IceProxy::DataOnDemand::DataStreamService;
	v->__copyFrom(proxy);
    }
}

void
DataOnDemand::__write(::IceInternal::BasicStream* __os, const ::DataOnDemand::DataStreamServicePtr& v)
{
    __os->write(::Ice::ObjectPtr(v));
}

DataOnDemand::DataStreamError::DataStreamError(const ::std::string& __ice_message) :
#if (defined(_MSC_VER) && (_MSC_VER < 1300)) // VC++ 6 compiler bug
    ServerError(__ice_message)
#else
    ::TianShanIce::ServerError(__ice_message)
#endif
{
}

static const char* __DataOnDemand__DataStreamError_name = "DataOnDemand::DataStreamError";

const ::std::string
DataOnDemand::DataStreamError::ice_name() const
{
    return __DataOnDemand__DataStreamError_name;
}

::Ice::Exception*
DataOnDemand::DataStreamError::ice_clone() const
{
    return new DataStreamError(*this);
}

void
DataOnDemand::DataStreamError::ice_throw() const
{
    throw *this;
}

void
DataOnDemand::DataStreamError::__write(::IceInternal::BasicStream* __os) const
{
    __os->write(::std::string("::DataOnDemand::DataStreamError"), false);
    __os->startWriteSlice();
    __os->endWriteSlice();
#if (defined(_MSC_VER) && (_MSC_VER < 1300)) // VC++ 6 compiler bug
    ServerError::__write(__os);
#else
    ::TianShanIce::ServerError::__write(__os);
#endif
}

void
DataOnDemand::DataStreamError::__read(::IceInternal::BasicStream* __is, bool __rid)
{
    if(__rid)
    {
	::std::string myId;
	__is->read(myId, false);
    }
    __is->startReadSlice();
    __is->endReadSlice();
#if (defined(_MSC_VER) && (_MSC_VER < 1300)) // VC++ 6 compiler bug
    ServerError::__read(__is, true);
#else
    ::TianShanIce::ServerError::__read(__is, true);
#endif
}

void
DataOnDemand::DataStreamError::__write(const ::Ice::OutputStreamPtr&) const
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "exception DataOnDemand::DataStreamError was not generated with stream support";
    throw ex;
}

void
DataOnDemand::DataStreamError::__read(const ::Ice::InputStreamPtr&, bool)
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "exception DataOnDemand::DataStreamError was not generated with stream support";
    throw ex;
}

struct __F__DataOnDemand__DataStreamError : public ::IceInternal::UserExceptionFactory
{
    virtual void
    createAndThrow()
    {
	throw ::DataOnDemand::DataStreamError();
    }
};

static ::IceInternal::UserExceptionFactoryPtr __F__DataOnDemand__DataStreamError__Ptr = new __F__DataOnDemand__DataStreamError;

const ::IceInternal::UserExceptionFactoryPtr&
DataOnDemand::DataStreamError::ice_factory()
{
    return __F__DataOnDemand__DataStreamError__Ptr;
}

class __F__DataOnDemand__DataStreamError__Init
{
public:

    __F__DataOnDemand__DataStreamError__Init()
    {
	::IceInternal::factoryTable->addExceptionFactory("::DataOnDemand::DataStreamError", ::DataOnDemand::DataStreamError::ice_factory());
    }

    ~__F__DataOnDemand__DataStreamError__Init()
    {
	::IceInternal::factoryTable->removeExceptionFactory("::DataOnDemand::DataStreamError");
    }
};

static __F__DataOnDemand__DataStreamError__Init __F__DataOnDemand__DataStreamError__i;

#ifdef __APPLE__
extern "C" { void __F__DataOnDemand__DataStreamError__initializer() {} }
#endif

DataOnDemand::NameDupException::NameDupException(const ::std::string& __ice_message) :
#if (defined(_MSC_VER) && (_MSC_VER < 1300)) // VC++ 6 compiler bug
    DataStreamError(__ice_message)
#else
    ::DataOnDemand::DataStreamError(__ice_message)
#endif
{
}

static const char* __DataOnDemand__NameDupException_name = "DataOnDemand::NameDupException";

const ::std::string
DataOnDemand::NameDupException::ice_name() const
{
    return __DataOnDemand__NameDupException_name;
}

::Ice::Exception*
DataOnDemand::NameDupException::ice_clone() const
{
    return new NameDupException(*this);
}

void
DataOnDemand::NameDupException::ice_throw() const
{
    throw *this;
}

void
DataOnDemand::NameDupException::__write(::IceInternal::BasicStream* __os) const
{
    __os->write(::std::string("::DataOnDemand::NameDupException"), false);
    __os->startWriteSlice();
    __os->endWriteSlice();
#if (defined(_MSC_VER) && (_MSC_VER < 1300)) // VC++ 6 compiler bug
    DataStreamError::__write(__os);
#else
    ::DataOnDemand::DataStreamError::__write(__os);
#endif
}

void
DataOnDemand::NameDupException::__read(::IceInternal::BasicStream* __is, bool __rid)
{
    if(__rid)
    {
	::std::string myId;
	__is->read(myId, false);
    }
    __is->startReadSlice();
    __is->endReadSlice();
#if (defined(_MSC_VER) && (_MSC_VER < 1300)) // VC++ 6 compiler bug
    DataStreamError::__read(__is, true);
#else
    ::DataOnDemand::DataStreamError::__read(__is, true);
#endif
}

void
DataOnDemand::NameDupException::__write(const ::Ice::OutputStreamPtr&) const
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "exception DataOnDemand::NameDupException was not generated with stream support";
    throw ex;
}

void
DataOnDemand::NameDupException::__read(const ::Ice::InputStreamPtr&, bool)
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "exception DataOnDemand::NameDupException was not generated with stream support";
    throw ex;
}

struct __F__DataOnDemand__NameDupException : public ::IceInternal::UserExceptionFactory
{
    virtual void
    createAndThrow()
    {
	throw ::DataOnDemand::NameDupException();
    }
};

static ::IceInternal::UserExceptionFactoryPtr __F__DataOnDemand__NameDupException__Ptr = new __F__DataOnDemand__NameDupException;

const ::IceInternal::UserExceptionFactoryPtr&
DataOnDemand::NameDupException::ice_factory()
{
    return __F__DataOnDemand__NameDupException__Ptr;
}

class __F__DataOnDemand__NameDupException__Init
{
public:

    __F__DataOnDemand__NameDupException__Init()
    {
	::IceInternal::factoryTable->addExceptionFactory("::DataOnDemand::NameDupException", ::DataOnDemand::NameDupException::ice_factory());
    }

    ~__F__DataOnDemand__NameDupException__Init()
    {
	::IceInternal::factoryTable->removeExceptionFactory("::DataOnDemand::NameDupException");
    }
};

static __F__DataOnDemand__NameDupException__Init __F__DataOnDemand__NameDupException__i;

#ifdef __APPLE__
extern "C" { void __F__DataOnDemand__NameDupException__initializer() {} }
#endif

void
DataOnDemand::__write(::IceInternal::BasicStream* __os, ::DataOnDemand::CacheType v)
{
    __os->write(static_cast< ::Ice::Byte>(v));
}

void
DataOnDemand::__read(::IceInternal::BasicStream* __is, ::DataOnDemand::CacheType& v)
{
    ::Ice::Byte val;
    __is->read(val);
    v = static_cast< ::DataOnDemand::CacheType>(val);
}

bool
DataOnDemand::MuxItemInfo::operator==(const MuxItemInfo& __rhs) const
{
    return !operator!=(__rhs);
}

bool
DataOnDemand::MuxItemInfo::operator!=(const MuxItemInfo& __rhs) const
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
DataOnDemand::MuxItemInfo::operator<(const MuxItemInfo& __rhs) const
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
DataOnDemand::MuxItemInfo::__write(::IceInternal::BasicStream* __os) const
{
    __os->write(name);
    __os->write(streamId);
    __os->write(streamType);
    __os->write(bandWidth);
    __os->write(tag);
    __os->write(expiration);
    __os->write(repeatTime);
    ::DataOnDemand::__write(__os, ctype);
    __os->write(cacheAddr);
    __os->write(encryptMode);
    __os->write(subchannelCount);
}

void
DataOnDemand::MuxItemInfo::__read(::IceInternal::BasicStream* __is)
{
    __is->read(name);
    __is->read(streamId);
    __is->read(streamType);
    __is->read(bandWidth);
    __is->read(tag);
    __is->read(expiration);
    __is->read(repeatTime);
    ::DataOnDemand::__read(__is, ctype);
    __is->read(cacheAddr);
    __is->read(encryptMode);
    __is->read(subchannelCount);
}

void
DataOnDemand::__addObject(const MuxItemPtr& p, ::IceInternal::GCCountMap& c)
{
    p->__addObject(c);
}

bool
DataOnDemand::__usesClasses(const MuxItemPtr& p)
{
    return p->__usesClasses();
}

void
DataOnDemand::__decRefUnsafe(MuxItemPtr& p)
{
    p->__decRefUnsafe();
}

void
DataOnDemand::__clearHandleUnsafe(MuxItemPtr& p)
{
    p.__clearHandleUnsafe();
}

bool
DataOnDemand::StreamInfo::operator==(const StreamInfo& __rhs) const
{
    return !operator!=(__rhs);
}

bool
DataOnDemand::StreamInfo::operator!=(const StreamInfo& __rhs) const
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
DataOnDemand::StreamInfo::operator<(const StreamInfo& __rhs) const
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
DataOnDemand::StreamInfo::__write(::IceInternal::BasicStream* __os) const
{
    __os->write(name);
    __os->write(totalBandwidth);
    __os->write(destAddress);
    __os->write(pmtPid);
}

void
DataOnDemand::StreamInfo::__read(::IceInternal::BasicStream* __is)
{
    __is->read(name);
    __is->read(totalBandwidth);
    __is->read(destAddress);
    __is->read(pmtPid);
}

void
DataOnDemand::__addObject(const DataStreamPtr& p, ::IceInternal::GCCountMap& c)
{
    p->__addObject(c);
}

bool
DataOnDemand::__usesClasses(const DataStreamPtr& p)
{
    return p->__usesClasses();
}

void
DataOnDemand::__decRefUnsafe(DataStreamPtr& p)
{
    p->__decRefUnsafe();
}

void
DataOnDemand::__clearHandleUnsafe(DataStreamPtr& p)
{
    p.__clearHandleUnsafe();
}

void
DataOnDemand::__addObject(const DataStreamServicePtr& p, ::IceInternal::GCCountMap& c)
{
    p->__addObject(c);
}

bool
DataOnDemand::__usesClasses(const DataStreamServicePtr& p)
{
    return p->__usesClasses();
}

void
DataOnDemand::__decRefUnsafe(DataStreamServicePtr& p)
{
    p->__decRefUnsafe();
}

void
DataOnDemand::__clearHandleUnsafe(DataStreamServicePtr& p)
{
    p.__clearHandleUnsafe();
}

::std::string
IceProxy::DataOnDemand::MuxItem::getName(const ::Ice::Context& __ctx)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    __checkTwowayOnly(__DataOnDemand__MuxItem__getName_name);
	    ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();
	    ::IceDelegate::DataOnDemand::MuxItem* __del = dynamic_cast< ::IceDelegate::DataOnDemand::MuxItem*>(__delBase.get());
	    return __del->getName(__ctx);
	}
	catch(const ::IceInternal::LocalExceptionWrapper& __ex)
	{
	    __handleExceptionWrapper(__ex);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

void
IceProxy::DataOnDemand::MuxItem::notifyFullUpdate(const ::Ice::Context& __ctx)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();
	    ::IceDelegate::DataOnDemand::MuxItem* __del = dynamic_cast< ::IceDelegate::DataOnDemand::MuxItem*>(__delBase.get());
	    __del->notifyFullUpdate(__ctx);
	    return;
	}
	catch(const ::IceInternal::LocalExceptionWrapper& __ex)
	{
	    __handleExceptionWrapper(__ex);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

void
IceProxy::DataOnDemand::MuxItem::notifyFileAdded(const ::std::string& fileName, const ::Ice::Context& __ctx)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();
	    ::IceDelegate::DataOnDemand::MuxItem* __del = dynamic_cast< ::IceDelegate::DataOnDemand::MuxItem*>(__delBase.get());
	    __del->notifyFileAdded(fileName, __ctx);
	    return;
	}
	catch(const ::IceInternal::LocalExceptionWrapper& __ex)
	{
	    __handleExceptionWrapper(__ex);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

void
IceProxy::DataOnDemand::MuxItem::notifyFileDeleted(const ::std::string& fileName, const ::Ice::Context& __ctx)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();
	    ::IceDelegate::DataOnDemand::MuxItem* __del = dynamic_cast< ::IceDelegate::DataOnDemand::MuxItem*>(__delBase.get());
	    __del->notifyFileDeleted(fileName, __ctx);
	    return;
	}
	catch(const ::IceInternal::LocalExceptionWrapper& __ex)
	{
	    __handleExceptionWrapper(__ex);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

::DataOnDemand::MuxItemInfo
IceProxy::DataOnDemand::MuxItem::getInfo(const ::Ice::Context& __ctx)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    __checkTwowayOnly(__DataOnDemand__MuxItem__getInfo_name);
	    ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();
	    ::IceDelegate::DataOnDemand::MuxItem* __del = dynamic_cast< ::IceDelegate::DataOnDemand::MuxItem*>(__delBase.get());
	    return __del->getInfo(__ctx);
	}
	catch(const ::IceInternal::LocalExceptionWrapper& __ex)
	{
	    __handleExceptionWrapper(__ex);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

void
IceProxy::DataOnDemand::MuxItem::destory(const ::Ice::Context& __ctx)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    __checkTwowayOnly(__DataOnDemand__MuxItem__destory_name);
	    ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();
	    ::IceDelegate::DataOnDemand::MuxItem* __del = dynamic_cast< ::IceDelegate::DataOnDemand::MuxItem*>(__delBase.get());
	    __del->destory(__ctx);
	    return;
	}
	catch(const ::IceInternal::LocalExceptionWrapper& __ex)
	{
	    __handleExceptionWrapper(__ex);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

void
IceProxy::DataOnDemand::MuxItem::setProperies(const ::TianShanIce::Properties& props, const ::Ice::Context& __ctx)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();
	    ::IceDelegate::DataOnDemand::MuxItem* __del = dynamic_cast< ::IceDelegate::DataOnDemand::MuxItem*>(__delBase.get());
	    __del->setProperies(props, __ctx);
	    return;
	}
	catch(const ::IceInternal::LocalExceptionWrapper& __ex)
	{
	    __handleExceptionWrapper(__ex);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

::TianShanIce::Properties
IceProxy::DataOnDemand::MuxItem::getProperties(const ::Ice::Context& __ctx)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    __checkTwowayOnly(__DataOnDemand__MuxItem__getProperties_name);
	    ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();
	    ::IceDelegate::DataOnDemand::MuxItem* __del = dynamic_cast< ::IceDelegate::DataOnDemand::MuxItem*>(__delBase.get());
	    return __del->getProperties(__ctx);
	}
	catch(const ::IceInternal::LocalExceptionWrapper& __ex)
	{
	    __handleExceptionWrapperRelaxed(__ex, __cnt);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

const ::std::string&
IceProxy::DataOnDemand::MuxItem::ice_staticId()
{
    return ::DataOnDemand::MuxItem::ice_staticId();
}

::IceInternal::Handle< ::IceDelegateM::Ice::Object>
IceProxy::DataOnDemand::MuxItem::__createDelegateM()
{
    return ::IceInternal::Handle< ::IceDelegateM::Ice::Object>(new ::IceDelegateM::DataOnDemand::MuxItem);
}

::IceInternal::Handle< ::IceDelegateD::Ice::Object>
IceProxy::DataOnDemand::MuxItem::__createDelegateD()
{
    return ::IceInternal::Handle< ::IceDelegateD::Ice::Object>(new ::IceDelegateD::DataOnDemand::MuxItem);
}

bool
IceProxy::DataOnDemand::operator==(const ::IceProxy::DataOnDemand::MuxItem& l, const ::IceProxy::DataOnDemand::MuxItem& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) == static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::DataOnDemand::operator!=(const ::IceProxy::DataOnDemand::MuxItem& l, const ::IceProxy::DataOnDemand::MuxItem& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) != static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::DataOnDemand::operator<(const ::IceProxy::DataOnDemand::MuxItem& l, const ::IceProxy::DataOnDemand::MuxItem& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) < static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::DataOnDemand::operator<=(const ::IceProxy::DataOnDemand::MuxItem& l, const ::IceProxy::DataOnDemand::MuxItem& r)
{
    return l < r || l == r;
}

bool
IceProxy::DataOnDemand::operator>(const ::IceProxy::DataOnDemand::MuxItem& l, const ::IceProxy::DataOnDemand::MuxItem& r)
{
    return !(l < r) && !(l == r);
}

bool
IceProxy::DataOnDemand::operator>=(const ::IceProxy::DataOnDemand::MuxItem& l, const ::IceProxy::DataOnDemand::MuxItem& r)
{
    return !(l < r);
}

::std::string
IceProxy::DataOnDemand::DataStream::getName(const ::Ice::Context& __ctx)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    __checkTwowayOnly(__DataOnDemand__DataStream__getName_name);
	    ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();
	    ::IceDelegate::DataOnDemand::DataStream* __del = dynamic_cast< ::IceDelegate::DataOnDemand::DataStream*>(__delBase.get());
	    return __del->getName(__ctx);
	}
	catch(const ::IceInternal::LocalExceptionWrapper& __ex)
	{
	    __handleExceptionWrapper(__ex);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

::DataOnDemand::StreamInfo
IceProxy::DataOnDemand::DataStream::getInfo(const ::Ice::Context& __ctx)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    __checkTwowayOnly(__DataOnDemand__DataStream__getInfo_name);
	    ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();
	    ::IceDelegate::DataOnDemand::DataStream* __del = dynamic_cast< ::IceDelegate::DataOnDemand::DataStream*>(__delBase.get());
	    return __del->getInfo(__ctx);
	}
	catch(const ::IceInternal::LocalExceptionWrapper& __ex)
	{
	    __handleExceptionWrapperRelaxed(__ex, __cnt);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

::DataOnDemand::MuxItemPrx
IceProxy::DataOnDemand::DataStream::createMuxItem(const ::std::string& name, const ::DataOnDemand::MuxItemInfo& info, const ::Ice::Context& __ctx)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    __checkTwowayOnly(__DataOnDemand__DataStream__createMuxItem_name);
	    ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();
	    ::IceDelegate::DataOnDemand::DataStream* __del = dynamic_cast< ::IceDelegate::DataOnDemand::DataStream*>(__delBase.get());
	    return __del->createMuxItem(name, info, __ctx);
	}
	catch(const ::IceInternal::LocalExceptionWrapper& __ex)
	{
	    __handleExceptionWrapper(__ex);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

::DataOnDemand::MuxItemPrx
IceProxy::DataOnDemand::DataStream::getMuxItem(const ::std::string& name, const ::Ice::Context& __ctx)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    __checkTwowayOnly(__DataOnDemand__DataStream__getMuxItem_name);
	    ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();
	    ::IceDelegate::DataOnDemand::DataStream* __del = dynamic_cast< ::IceDelegate::DataOnDemand::DataStream*>(__delBase.get());
	    return __del->getMuxItem(name, __ctx);
	}
	catch(const ::IceInternal::LocalExceptionWrapper& __ex)
	{
	    __handleExceptionWrapper(__ex);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

::Ice::StringSeq
IceProxy::DataOnDemand::DataStream::listMuxItems(const ::Ice::Context& __ctx)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    __checkTwowayOnly(__DataOnDemand__DataStream__listMuxItems_name);
	    ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();
	    ::IceDelegate::DataOnDemand::DataStream* __del = dynamic_cast< ::IceDelegate::DataOnDemand::DataStream*>(__delBase.get());
	    return __del->listMuxItems(__ctx);
	}
	catch(const ::IceInternal::LocalExceptionWrapper& __ex)
	{
	    __handleExceptionWrapper(__ex);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

void
IceProxy::DataOnDemand::DataStream::setProperies(const ::TianShanIce::Properties& props, const ::Ice::Context& __ctx)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();
	    ::IceDelegate::DataOnDemand::DataStream* __del = dynamic_cast< ::IceDelegate::DataOnDemand::DataStream*>(__delBase.get());
	    __del->setProperies(props, __ctx);
	    return;
	}
	catch(const ::IceInternal::LocalExceptionWrapper& __ex)
	{
	    __handleExceptionWrapper(__ex);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

::TianShanIce::Properties
IceProxy::DataOnDemand::DataStream::getProperties(const ::Ice::Context& __ctx)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    __checkTwowayOnly(__DataOnDemand__DataStream__getProperties_name);
	    ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();
	    ::IceDelegate::DataOnDemand::DataStream* __del = dynamic_cast< ::IceDelegate::DataOnDemand::DataStream*>(__delBase.get());
	    return __del->getProperties(__ctx);
	}
	catch(const ::IceInternal::LocalExceptionWrapper& __ex)
	{
	    __handleExceptionWrapperRelaxed(__ex, __cnt);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

const ::std::string&
IceProxy::DataOnDemand::DataStream::ice_staticId()
{
    return ::DataOnDemand::DataStream::ice_staticId();
}

::IceInternal::Handle< ::IceDelegateM::Ice::Object>
IceProxy::DataOnDemand::DataStream::__createDelegateM()
{
    return ::IceInternal::Handle< ::IceDelegateM::Ice::Object>(new ::IceDelegateM::DataOnDemand::DataStream);
}

::IceInternal::Handle< ::IceDelegateD::Ice::Object>
IceProxy::DataOnDemand::DataStream::__createDelegateD()
{
    return ::IceInternal::Handle< ::IceDelegateD::Ice::Object>(new ::IceDelegateD::DataOnDemand::DataStream);
}

bool
IceProxy::DataOnDemand::operator==(const ::IceProxy::DataOnDemand::DataStream& l, const ::IceProxy::DataOnDemand::DataStream& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) == static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::DataOnDemand::operator!=(const ::IceProxy::DataOnDemand::DataStream& l, const ::IceProxy::DataOnDemand::DataStream& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) != static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::DataOnDemand::operator<(const ::IceProxy::DataOnDemand::DataStream& l, const ::IceProxy::DataOnDemand::DataStream& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) < static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::DataOnDemand::operator<=(const ::IceProxy::DataOnDemand::DataStream& l, const ::IceProxy::DataOnDemand::DataStream& r)
{
    return l < r || l == r;
}

bool
IceProxy::DataOnDemand::operator>(const ::IceProxy::DataOnDemand::DataStream& l, const ::IceProxy::DataOnDemand::DataStream& r)
{
    return !(l < r) && !(l == r);
}

bool
IceProxy::DataOnDemand::operator>=(const ::IceProxy::DataOnDemand::DataStream& l, const ::IceProxy::DataOnDemand::DataStream& r)
{
    return !(l < r);
}

::DataOnDemand::DataStreamPrx
IceProxy::DataOnDemand::DataStreamService::createStreamByApp(const ::TianShanIce::AccreditedPath::PathTicketPrx& pathTicket, const ::std::string& name, const ::DataOnDemand::StreamInfo& info, const ::Ice::Context& __ctx)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    __checkTwowayOnly(__DataOnDemand__DataStreamService__createStreamByApp_name);
	    ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();
	    ::IceDelegate::DataOnDemand::DataStreamService* __del = dynamic_cast< ::IceDelegate::DataOnDemand::DataStreamService*>(__delBase.get());
	    return __del->createStreamByApp(pathTicket, name, info, __ctx);
	}
	catch(const ::IceInternal::LocalExceptionWrapper& __ex)
	{
	    __handleExceptionWrapper(__ex);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

::DataOnDemand::DataStreamPrx
IceProxy::DataOnDemand::DataStreamService::getStream(const ::std::string& name, const ::Ice::Context& __ctx)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    __checkTwowayOnly(__DataOnDemand__DataStreamService__getStream_name);
	    ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();
	    ::IceDelegate::DataOnDemand::DataStreamService* __del = dynamic_cast< ::IceDelegate::DataOnDemand::DataStreamService*>(__delBase.get());
	    return __del->getStream(name, __ctx);
	}
	catch(const ::IceInternal::LocalExceptionWrapper& __ex)
	{
	    __handleExceptionWrapper(__ex);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

void
IceProxy::DataOnDemand::DataStreamService::setProperies(const ::TianShanIce::Properties& props, const ::Ice::Context& __ctx)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();
	    ::IceDelegate::DataOnDemand::DataStreamService* __del = dynamic_cast< ::IceDelegate::DataOnDemand::DataStreamService*>(__delBase.get());
	    __del->setProperies(props, __ctx);
	    return;
	}
	catch(const ::IceInternal::LocalExceptionWrapper& __ex)
	{
	    __handleExceptionWrapper(__ex);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

::TianShanIce::Properties
IceProxy::DataOnDemand::DataStreamService::getProperties(const ::Ice::Context& __ctx)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    __checkTwowayOnly(__DataOnDemand__DataStreamService__getProperties_name);
	    ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();
	    ::IceDelegate::DataOnDemand::DataStreamService* __del = dynamic_cast< ::IceDelegate::DataOnDemand::DataStreamService*>(__delBase.get());
	    return __del->getProperties(__ctx);
	}
	catch(const ::IceInternal::LocalExceptionWrapper& __ex)
	{
	    __handleExceptionWrapperRelaxed(__ex, __cnt);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

const ::std::string&
IceProxy::DataOnDemand::DataStreamService::ice_staticId()
{
    return ::DataOnDemand::DataStreamService::ice_staticId();
}

::IceInternal::Handle< ::IceDelegateM::Ice::Object>
IceProxy::DataOnDemand::DataStreamService::__createDelegateM()
{
    return ::IceInternal::Handle< ::IceDelegateM::Ice::Object>(new ::IceDelegateM::DataOnDemand::DataStreamService);
}

::IceInternal::Handle< ::IceDelegateD::Ice::Object>
IceProxy::DataOnDemand::DataStreamService::__createDelegateD()
{
    return ::IceInternal::Handle< ::IceDelegateD::Ice::Object>(new ::IceDelegateD::DataOnDemand::DataStreamService);
}

bool
IceProxy::DataOnDemand::operator==(const ::IceProxy::DataOnDemand::DataStreamService& l, const ::IceProxy::DataOnDemand::DataStreamService& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) == static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::DataOnDemand::operator!=(const ::IceProxy::DataOnDemand::DataStreamService& l, const ::IceProxy::DataOnDemand::DataStreamService& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) != static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::DataOnDemand::operator<(const ::IceProxy::DataOnDemand::DataStreamService& l, const ::IceProxy::DataOnDemand::DataStreamService& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) < static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::DataOnDemand::operator<=(const ::IceProxy::DataOnDemand::DataStreamService& l, const ::IceProxy::DataOnDemand::DataStreamService& r)
{
    return l < r || l == r;
}

bool
IceProxy::DataOnDemand::operator>(const ::IceProxy::DataOnDemand::DataStreamService& l, const ::IceProxy::DataOnDemand::DataStreamService& r)
{
    return !(l < r) && !(l == r);
}

bool
IceProxy::DataOnDemand::operator>=(const ::IceProxy::DataOnDemand::DataStreamService& l, const ::IceProxy::DataOnDemand::DataStreamService& r)
{
    return !(l < r);
}

::std::string
IceDelegateM::DataOnDemand::MuxItem::getName(const ::Ice::Context& __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __DataOnDemand__MuxItem__getName_name, ::Ice::Normal, __context, __compress);
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
IceDelegateM::DataOnDemand::MuxItem::notifyFullUpdate(const ::Ice::Context& __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __DataOnDemand__MuxItem__notifyFullUpdate_name, ::Ice::Normal, __context, __compress);
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
IceDelegateM::DataOnDemand::MuxItem::notifyFileAdded(const ::std::string& fileName, const ::Ice::Context& __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __DataOnDemand__MuxItem__notifyFileAdded_name, ::Ice::Normal, __context, __compress);
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
IceDelegateM::DataOnDemand::MuxItem::notifyFileDeleted(const ::std::string& fileName, const ::Ice::Context& __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __DataOnDemand__MuxItem__notifyFileDeleted_name, ::Ice::Normal, __context, __compress);
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

::DataOnDemand::MuxItemInfo
IceDelegateM::DataOnDemand::MuxItem::getInfo(const ::Ice::Context& __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __DataOnDemand__MuxItem__getInfo_name, ::Ice::Normal, __context, __compress);
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
	::DataOnDemand::MuxItemInfo __ret;
	__ret.__read(__is);
	return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
	throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateM::DataOnDemand::MuxItem::destory(const ::Ice::Context& __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __DataOnDemand__MuxItem__destory_name, ::Ice::Normal, __context, __compress);
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
	    catch(const ::DataOnDemand::DataStreamError&)
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
IceDelegateM::DataOnDemand::MuxItem::setProperies(const ::TianShanIce::Properties& props, const ::Ice::Context& __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __DataOnDemand__MuxItem__setProperies_name, ::Ice::Normal, __context, __compress);
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
IceDelegateM::DataOnDemand::MuxItem::getProperties(const ::Ice::Context& __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __DataOnDemand__MuxItem__getProperties_name, ::Ice::Nonmutating, __context, __compress);
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
IceDelegateM::DataOnDemand::DataStream::getName(const ::Ice::Context& __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __DataOnDemand__DataStream__getName_name, ::Ice::Normal, __context, __compress);
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

::DataOnDemand::StreamInfo
IceDelegateM::DataOnDemand::DataStream::getInfo(const ::Ice::Context& __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __DataOnDemand__DataStream__getInfo_name, ::Ice::Nonmutating, __context, __compress);
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
	    catch(const ::DataOnDemand::DataStreamError&)
	    {
		throw;
	    }
	    catch(const ::Ice::UserException& __ex)
	    {
		throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
	    }
	}
	::DataOnDemand::StreamInfo __ret;
	__ret.__read(__is);
	return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
	throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::DataOnDemand::MuxItemPrx
IceDelegateM::DataOnDemand::DataStream::createMuxItem(const ::std::string& name, const ::DataOnDemand::MuxItemInfo& info, const ::Ice::Context& __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __DataOnDemand__DataStream__createMuxItem_name, ::Ice::Normal, __context, __compress);
    try
    {
	::IceInternal::BasicStream* __os = __og.os();
	__os->write(name);
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
	    catch(const ::DataOnDemand::DataStreamError&)
	    {
		throw;
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
	::DataOnDemand::MuxItemPrx __ret;
	::DataOnDemand::__read(__is, __ret);
	return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
	throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::DataOnDemand::MuxItemPrx
IceDelegateM::DataOnDemand::DataStream::getMuxItem(const ::std::string& name, const ::Ice::Context& __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __DataOnDemand__DataStream__getMuxItem_name, ::Ice::Normal, __context, __compress);
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
	    catch(const ::DataOnDemand::DataStreamError&)
	    {
		throw;
	    }
	    catch(const ::Ice::UserException& __ex)
	    {
		throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
	    }
	}
	::DataOnDemand::MuxItemPrx __ret;
	::DataOnDemand::__read(__is, __ret);
	return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
	throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::Ice::StringSeq
IceDelegateM::DataOnDemand::DataStream::listMuxItems(const ::Ice::Context& __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __DataOnDemand__DataStream__listMuxItems_name, ::Ice::Normal, __context, __compress);
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
IceDelegateM::DataOnDemand::DataStream::setProperies(const ::TianShanIce::Properties& props, const ::Ice::Context& __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __DataOnDemand__DataStream__setProperies_name, ::Ice::Normal, __context, __compress);
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
IceDelegateM::DataOnDemand::DataStream::getProperties(const ::Ice::Context& __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __DataOnDemand__DataStream__getProperties_name, ::Ice::Nonmutating, __context, __compress);
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

::DataOnDemand::DataStreamPrx
IceDelegateM::DataOnDemand::DataStreamService::createStreamByApp(const ::TianShanIce::AccreditedPath::PathTicketPrx& pathTicket, const ::std::string& name, const ::DataOnDemand::StreamInfo& info, const ::Ice::Context& __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __DataOnDemand__DataStreamService__createStreamByApp_name, ::Ice::Normal, __context, __compress);
    try
    {
	::IceInternal::BasicStream* __os = __og.os();
	::TianShanIce::AccreditedPath::__write(__os, pathTicket);
	__os->write(name);
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
	    catch(const ::DataOnDemand::NameDupException&)
	    {
		throw;
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
	::DataOnDemand::DataStreamPrx __ret;
	::DataOnDemand::__read(__is, __ret);
	return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
	throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::DataOnDemand::DataStreamPrx
IceDelegateM::DataOnDemand::DataStreamService::getStream(const ::std::string& name, const ::Ice::Context& __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __DataOnDemand__DataStreamService__getStream_name, ::Ice::Normal, __context, __compress);
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
	    catch(const ::Ice::UserException& __ex)
	    {
		throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
	    }
	}
	::DataOnDemand::DataStreamPrx __ret;
	::DataOnDemand::__read(__is, __ret);
	return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
	throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateM::DataOnDemand::DataStreamService::setProperies(const ::TianShanIce::Properties& props, const ::Ice::Context& __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __DataOnDemand__DataStreamService__setProperies_name, ::Ice::Normal, __context, __compress);
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
IceDelegateM::DataOnDemand::DataStreamService::getProperties(const ::Ice::Context& __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __DataOnDemand__DataStreamService__getProperties_name, ::Ice::Nonmutating, __context, __compress);
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
IceDelegateD::DataOnDemand::MuxItem::getName(const ::Ice::Context& __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __DataOnDemand__MuxItem__getName_name, ::Ice::Normal, __context);
    while(true)
    {
	::IceInternal::Direct __direct(__current);
	::DataOnDemand::MuxItem* __servant = dynamic_cast< ::DataOnDemand::MuxItem*>(__direct.servant().get());
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
	    return __servant->getName(__current);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    throw ::IceInternal::LocalExceptionWrapper(__ex, false);
	}
    }
}

void
IceDelegateD::DataOnDemand::MuxItem::notifyFullUpdate(const ::Ice::Context& __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __DataOnDemand__MuxItem__notifyFullUpdate_name, ::Ice::Normal, __context);
    while(true)
    {
	::IceInternal::Direct __direct(__current);
	::DataOnDemand::MuxItem* __servant = dynamic_cast< ::DataOnDemand::MuxItem*>(__direct.servant().get());
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
	    __servant->notifyFullUpdate(__current);
	    return;
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    throw ::IceInternal::LocalExceptionWrapper(__ex, false);
	}
    }
}

void
IceDelegateD::DataOnDemand::MuxItem::notifyFileAdded(const ::std::string& fileName, const ::Ice::Context& __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __DataOnDemand__MuxItem__notifyFileAdded_name, ::Ice::Normal, __context);
    while(true)
    {
	::IceInternal::Direct __direct(__current);
	::DataOnDemand::MuxItem* __servant = dynamic_cast< ::DataOnDemand::MuxItem*>(__direct.servant().get());
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
	    return;
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    throw ::IceInternal::LocalExceptionWrapper(__ex, false);
	}
    }
}

void
IceDelegateD::DataOnDemand::MuxItem::notifyFileDeleted(const ::std::string& fileName, const ::Ice::Context& __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __DataOnDemand__MuxItem__notifyFileDeleted_name, ::Ice::Normal, __context);
    while(true)
    {
	::IceInternal::Direct __direct(__current);
	::DataOnDemand::MuxItem* __servant = dynamic_cast< ::DataOnDemand::MuxItem*>(__direct.servant().get());
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
	    return;
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    throw ::IceInternal::LocalExceptionWrapper(__ex, false);
	}
    }
}

::DataOnDemand::MuxItemInfo
IceDelegateD::DataOnDemand::MuxItem::getInfo(const ::Ice::Context& __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __DataOnDemand__MuxItem__getInfo_name, ::Ice::Normal, __context);
    while(true)
    {
	::IceInternal::Direct __direct(__current);
	::DataOnDemand::MuxItem* __servant = dynamic_cast< ::DataOnDemand::MuxItem*>(__direct.servant().get());
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
	    return __servant->getInfo(__current);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    throw ::IceInternal::LocalExceptionWrapper(__ex, false);
	}
    }
}

void
IceDelegateD::DataOnDemand::MuxItem::destory(const ::Ice::Context& __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __DataOnDemand__MuxItem__destory_name, ::Ice::Normal, __context);
    while(true)
    {
	::IceInternal::Direct __direct(__current);
	::DataOnDemand::MuxItem* __servant = dynamic_cast< ::DataOnDemand::MuxItem*>(__direct.servant().get());
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
	    __servant->destory(__current);
	    return;
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    throw ::IceInternal::LocalExceptionWrapper(__ex, false);
	}
    }
}

void
IceDelegateD::DataOnDemand::MuxItem::setProperies(const ::TianShanIce::Properties& props, const ::Ice::Context& __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __DataOnDemand__MuxItem__setProperies_name, ::Ice::Normal, __context);
    while(true)
    {
	::IceInternal::Direct __direct(__current);
	::DataOnDemand::MuxItem* __servant = dynamic_cast< ::DataOnDemand::MuxItem*>(__direct.servant().get());
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
	    __servant->setProperies(props, __current);
	    return;
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    throw ::IceInternal::LocalExceptionWrapper(__ex, false);
	}
    }
}

::TianShanIce::Properties
IceDelegateD::DataOnDemand::MuxItem::getProperties(const ::Ice::Context& __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __DataOnDemand__MuxItem__getProperties_name, ::Ice::Nonmutating, __context);
    while(true)
    {
	::IceInternal::Direct __direct(__current);
	::DataOnDemand::MuxItem* __servant = dynamic_cast< ::DataOnDemand::MuxItem*>(__direct.servant().get());
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
	    return __servant->getProperties(__current);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    throw ::IceInternal::LocalExceptionWrapper(__ex, false);
	}
    }
}

::std::string
IceDelegateD::DataOnDemand::DataStream::getName(const ::Ice::Context& __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __DataOnDemand__DataStream__getName_name, ::Ice::Normal, __context);
    while(true)
    {
	::IceInternal::Direct __direct(__current);
	::DataOnDemand::DataStream* __servant = dynamic_cast< ::DataOnDemand::DataStream*>(__direct.servant().get());
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
	    return __servant->getName(__current);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    throw ::IceInternal::LocalExceptionWrapper(__ex, false);
	}
    }
}

::DataOnDemand::StreamInfo
IceDelegateD::DataOnDemand::DataStream::getInfo(const ::Ice::Context& __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __DataOnDemand__DataStream__getInfo_name, ::Ice::Nonmutating, __context);
    while(true)
    {
	::IceInternal::Direct __direct(__current);
	::DataOnDemand::DataStream* __servant = dynamic_cast< ::DataOnDemand::DataStream*>(__direct.servant().get());
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
	    return __servant->getInfo(__current);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    throw ::IceInternal::LocalExceptionWrapper(__ex, false);
	}
    }
}

::DataOnDemand::MuxItemPrx
IceDelegateD::DataOnDemand::DataStream::createMuxItem(const ::std::string& name, const ::DataOnDemand::MuxItemInfo& info, const ::Ice::Context& __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __DataOnDemand__DataStream__createMuxItem_name, ::Ice::Normal, __context);
    while(true)
    {
	::IceInternal::Direct __direct(__current);
	::DataOnDemand::DataStream* __servant = dynamic_cast< ::DataOnDemand::DataStream*>(__direct.servant().get());
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
	    return __servant->createMuxItem(name, info, __current);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    throw ::IceInternal::LocalExceptionWrapper(__ex, false);
	}
    }
}

::DataOnDemand::MuxItemPrx
IceDelegateD::DataOnDemand::DataStream::getMuxItem(const ::std::string& name, const ::Ice::Context& __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __DataOnDemand__DataStream__getMuxItem_name, ::Ice::Normal, __context);
    while(true)
    {
	::IceInternal::Direct __direct(__current);
	::DataOnDemand::DataStream* __servant = dynamic_cast< ::DataOnDemand::DataStream*>(__direct.servant().get());
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
	    return __servant->getMuxItem(name, __current);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    throw ::IceInternal::LocalExceptionWrapper(__ex, false);
	}
    }
}

::Ice::StringSeq
IceDelegateD::DataOnDemand::DataStream::listMuxItems(const ::Ice::Context& __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __DataOnDemand__DataStream__listMuxItems_name, ::Ice::Normal, __context);
    while(true)
    {
	::IceInternal::Direct __direct(__current);
	::DataOnDemand::DataStream* __servant = dynamic_cast< ::DataOnDemand::DataStream*>(__direct.servant().get());
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
	    return __servant->listMuxItems(__current);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    throw ::IceInternal::LocalExceptionWrapper(__ex, false);
	}
    }
}

void
IceDelegateD::DataOnDemand::DataStream::setProperies(const ::TianShanIce::Properties& props, const ::Ice::Context& __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __DataOnDemand__DataStream__setProperies_name, ::Ice::Normal, __context);
    while(true)
    {
	::IceInternal::Direct __direct(__current);
	::DataOnDemand::DataStream* __servant = dynamic_cast< ::DataOnDemand::DataStream*>(__direct.servant().get());
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
	    __servant->setProperies(props, __current);
	    return;
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    throw ::IceInternal::LocalExceptionWrapper(__ex, false);
	}
    }
}

::TianShanIce::Properties
IceDelegateD::DataOnDemand::DataStream::getProperties(const ::Ice::Context& __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __DataOnDemand__DataStream__getProperties_name, ::Ice::Nonmutating, __context);
    while(true)
    {
	::IceInternal::Direct __direct(__current);
	::DataOnDemand::DataStream* __servant = dynamic_cast< ::DataOnDemand::DataStream*>(__direct.servant().get());
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
	    return __servant->getProperties(__current);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    throw ::IceInternal::LocalExceptionWrapper(__ex, false);
	}
    }
}

::DataOnDemand::DataStreamPrx
IceDelegateD::DataOnDemand::DataStreamService::createStreamByApp(const ::TianShanIce::AccreditedPath::PathTicketPrx& pathTicket, const ::std::string& name, const ::DataOnDemand::StreamInfo& info, const ::Ice::Context& __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __DataOnDemand__DataStreamService__createStreamByApp_name, ::Ice::Normal, __context);
    while(true)
    {
	::IceInternal::Direct __direct(__current);
	::DataOnDemand::DataStreamService* __servant = dynamic_cast< ::DataOnDemand::DataStreamService*>(__direct.servant().get());
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
	    return __servant->createStreamByApp(pathTicket, name, info, __current);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    throw ::IceInternal::LocalExceptionWrapper(__ex, false);
	}
    }
}

::DataOnDemand::DataStreamPrx
IceDelegateD::DataOnDemand::DataStreamService::getStream(const ::std::string& name, const ::Ice::Context& __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __DataOnDemand__DataStreamService__getStream_name, ::Ice::Normal, __context);
    while(true)
    {
	::IceInternal::Direct __direct(__current);
	::DataOnDemand::DataStreamService* __servant = dynamic_cast< ::DataOnDemand::DataStreamService*>(__direct.servant().get());
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
	    return __servant->getStream(name, __current);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    throw ::IceInternal::LocalExceptionWrapper(__ex, false);
	}
    }
}

void
IceDelegateD::DataOnDemand::DataStreamService::setProperies(const ::TianShanIce::Properties& props, const ::Ice::Context& __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __DataOnDemand__DataStreamService__setProperies_name, ::Ice::Normal, __context);
    while(true)
    {
	::IceInternal::Direct __direct(__current);
	::DataOnDemand::DataStreamService* __servant = dynamic_cast< ::DataOnDemand::DataStreamService*>(__direct.servant().get());
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
	    __servant->setProperies(props, __current);
	    return;
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    throw ::IceInternal::LocalExceptionWrapper(__ex, false);
	}
    }
}

::TianShanIce::Properties
IceDelegateD::DataOnDemand::DataStreamService::getProperties(const ::Ice::Context& __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __DataOnDemand__DataStreamService__getProperties_name, ::Ice::Nonmutating, __context);
    while(true)
    {
	::IceInternal::Direct __direct(__current);
	::DataOnDemand::DataStreamService* __servant = dynamic_cast< ::DataOnDemand::DataStreamService*>(__direct.servant().get());
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
	    return __servant->getProperties(__current);
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    throw ::IceInternal::LocalExceptionWrapper(__ex, false);
	}
    }
}

::Ice::ObjectPtr
DataOnDemand::MuxItem::ice_clone() const
{
    throw ::Ice::CloneNotImplementedException(__FILE__, __LINE__);
    return 0; // to avoid a warning with some compilers
}

static const ::std::string __DataOnDemand__MuxItem_ids[2] =
{
    "::DataOnDemand::MuxItem",
    "::Ice::Object"
};

bool
DataOnDemand::MuxItem::ice_isA(const ::std::string& _s, const ::Ice::Current&) const
{
    return ::std::binary_search(__DataOnDemand__MuxItem_ids, __DataOnDemand__MuxItem_ids + 2, _s);
}

::std::vector< ::std::string>
DataOnDemand::MuxItem::ice_ids(const ::Ice::Current&) const
{
    return ::std::vector< ::std::string>(&__DataOnDemand__MuxItem_ids[0], &__DataOnDemand__MuxItem_ids[2]);
}

const ::std::string&
DataOnDemand::MuxItem::ice_id(const ::Ice::Current&) const
{
    return __DataOnDemand__MuxItem_ids[0];
}

const ::std::string&
DataOnDemand::MuxItem::ice_staticId()
{
    return __DataOnDemand__MuxItem_ids[0];
}

::IceInternal::DispatchStatus
DataOnDemand::MuxItem::___getName(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string __ret = getName(__current);
    __os->write(__ret);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
DataOnDemand::MuxItem::___notifyFullUpdate(::IceInternal::Incoming&, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    notifyFullUpdate(__current);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
DataOnDemand::MuxItem::___notifyFileAdded(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::std::string fileName;
    __is->read(fileName);
    notifyFileAdded(fileName, __current);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
DataOnDemand::MuxItem::___notifyFileDeleted(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::std::string fileName;
    __is->read(fileName);
    notifyFileDeleted(fileName, __current);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
DataOnDemand::MuxItem::___getInfo(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __os = __inS.os();
    ::DataOnDemand::MuxItemInfo __ret = getInfo(__current);
    __ret.__write(__os);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
DataOnDemand::MuxItem::___destory(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __os = __inS.os();
    try
    {
	destory(__current);
    }
    catch(const ::DataOnDemand::DataStreamError& __ex)
    {
	__os->write(__ex);
	return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
DataOnDemand::MuxItem::___setProperies(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::TianShanIce::Properties props;
    ::TianShanIce::__read(__is, props, ::TianShanIce::__U__Properties());
    setProperies(props, __current);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
DataOnDemand::MuxItem::___getProperties(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Nonmutating, __current.mode);
    ::IceInternal::BasicStream* __os = __inS.os();
    ::TianShanIce::Properties __ret = getProperties(__current);
    ::TianShanIce::__write(__os, __ret, ::TianShanIce::__U__Properties());
    return ::IceInternal::DispatchOK;
}

static ::std::string __DataOnDemand__MuxItem_all[] =
{
    "destory",
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
    "setProperies"
};

::IceInternal::DispatchStatus
DataOnDemand::MuxItem::__dispatch(::IceInternal::Incoming& in, const ::Ice::Current& current)
{
    ::std::pair< ::std::string*, ::std::string*> r = ::std::equal_range(__DataOnDemand__MuxItem_all, __DataOnDemand__MuxItem_all + 12, current.operation);
    if(r.first == r.second)
    {
	return ::IceInternal::DispatchOperationNotExist;
    }

    switch(r.first - __DataOnDemand__MuxItem_all)
    {
	case 0:
	{
	    return ___destory(in, current);
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
	    return ___setProperies(in, current);
	}
    }

    assert(false);
    return ::IceInternal::DispatchOperationNotExist;
}

void
DataOnDemand::MuxItem::__write(::IceInternal::BasicStream* __os) const
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
DataOnDemand::MuxItem::__read(::IceInternal::BasicStream* __is, bool __rid)
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
DataOnDemand::MuxItem::__write(const ::Ice::OutputStreamPtr&) const
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type DataOnDemand::MuxItem was not generated with stream support";
    throw ex;
}

void
DataOnDemand::MuxItem::__read(const ::Ice::InputStreamPtr&, bool)
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type DataOnDemand::MuxItem was not generated with stream support";
    throw ex;
}

void 
DataOnDemand::__patch__MuxItemPtr(void* __addr, ::Ice::ObjectPtr& v)
{
    ::DataOnDemand::MuxItemPtr* p = static_cast< ::DataOnDemand::MuxItemPtr*>(__addr);
    assert(p);
    *p = ::DataOnDemand::MuxItemPtr::dynamicCast(v);
    if(v && !*p)
    {
	::Ice::NoObjectFactoryException e(__FILE__, __LINE__);
	e.type = ::DataOnDemand::MuxItem::ice_staticId();
	throw e;
    }
}

bool
DataOnDemand::operator==(const ::DataOnDemand::MuxItem& l, const ::DataOnDemand::MuxItem& r)
{
    return static_cast<const ::Ice::Object&>(l) == static_cast<const ::Ice::Object&>(r);
}

bool
DataOnDemand::operator!=(const ::DataOnDemand::MuxItem& l, const ::DataOnDemand::MuxItem& r)
{
    return static_cast<const ::Ice::Object&>(l) != static_cast<const ::Ice::Object&>(r);
}

bool
DataOnDemand::operator<(const ::DataOnDemand::MuxItem& l, const ::DataOnDemand::MuxItem& r)
{
    return static_cast<const ::Ice::Object&>(l) < static_cast<const ::Ice::Object&>(r);
}

bool
DataOnDemand::operator<=(const ::DataOnDemand::MuxItem& l, const ::DataOnDemand::MuxItem& r)
{
    return l < r || l == r;
}

bool
DataOnDemand::operator>(const ::DataOnDemand::MuxItem& l, const ::DataOnDemand::MuxItem& r)
{
    return !(l < r) && !(l == r);
}

bool
DataOnDemand::operator>=(const ::DataOnDemand::MuxItem& l, const ::DataOnDemand::MuxItem& r)
{
    return !(l < r);
}

DataOnDemand::DataStream::DataStream(const ::Ice::Identity& __ice_ident) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    Stream(__ice_ident)
#else
    ::TianShanIce::Streamer::Stream(__ice_ident)
#endif

{
}

::Ice::ObjectPtr
DataOnDemand::DataStream::ice_clone() const
{
    throw ::Ice::CloneNotImplementedException(__FILE__, __LINE__);
    return 0; // to avoid a warning with some compilers
}

static const ::std::string __DataOnDemand__DataStream_ids[3] =
{
    "::DataOnDemand::DataStream",
    "::Ice::Object",
    "::TianShanIce::Streamer::Stream"
};

bool
DataOnDemand::DataStream::ice_isA(const ::std::string& _s, const ::Ice::Current&) const
{
    return ::std::binary_search(__DataOnDemand__DataStream_ids, __DataOnDemand__DataStream_ids + 3, _s);
}

::std::vector< ::std::string>
DataOnDemand::DataStream::ice_ids(const ::Ice::Current&) const
{
    return ::std::vector< ::std::string>(&__DataOnDemand__DataStream_ids[0], &__DataOnDemand__DataStream_ids[3]);
}

const ::std::string&
DataOnDemand::DataStream::ice_id(const ::Ice::Current&) const
{
    return __DataOnDemand__DataStream_ids[0];
}

const ::std::string&
DataOnDemand::DataStream::ice_staticId()
{
    return __DataOnDemand__DataStream_ids[0];
}

::IceInternal::DispatchStatus
DataOnDemand::DataStream::___getName(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string __ret = getName(__current);
    __os->write(__ret);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
DataOnDemand::DataStream::___getInfo(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Nonmutating, __current.mode);
    ::IceInternal::BasicStream* __os = __inS.os();
    try
    {
	::DataOnDemand::StreamInfo __ret = getInfo(__current);
	__ret.__write(__os);
    }
    catch(const ::DataOnDemand::DataStreamError& __ex)
    {
	__os->write(__ex);
	return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
DataOnDemand::DataStream::___createMuxItem(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string name;
    ::DataOnDemand::MuxItemInfo info;
    __is->read(name);
    info.__read(__is);
    try
    {
	::DataOnDemand::MuxItemPrx __ret = createMuxItem(name, info, __current);
	::DataOnDemand::__write(__os, __ret);
    }
    catch(const ::DataOnDemand::DataStreamError& __ex)
    {
	__os->write(__ex);
	return ::IceInternal::DispatchUserException;
    }
    catch(const ::TianShanIce::InvalidParameter& __ex)
    {
	__os->write(__ex);
	return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
DataOnDemand::DataStream::___getMuxItem(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string name;
    __is->read(name);
    try
    {
	::DataOnDemand::MuxItemPrx __ret = getMuxItem(name, __current);
	::DataOnDemand::__write(__os, __ret);
    }
    catch(const ::DataOnDemand::DataStreamError& __ex)
    {
	__os->write(__ex);
	return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
DataOnDemand::DataStream::___listMuxItems(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
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
DataOnDemand::DataStream::___setProperies(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::TianShanIce::Properties props;
    ::TianShanIce::__read(__is, props, ::TianShanIce::__U__Properties());
    setProperies(props, __current);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
DataOnDemand::DataStream::___getProperties(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Nonmutating, __current.mode);
    ::IceInternal::BasicStream* __os = __inS.os();
    ::TianShanIce::Properties __ret = getProperties(__current);
    ::TianShanIce::__write(__os, __ret, ::TianShanIce::__U__Properties());
    return ::IceInternal::DispatchOK;
}

static ::std::string __DataOnDemand__DataStream_all[] =
{
    "allocDVBCResource",
    "allotAccreditPathTicket",
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
    "play",
    "resume",
    "setConditionalControl",
    "setMuxRate",
    "setProperies",
    "setSpeed"
};

::IceInternal::DispatchStatus
DataOnDemand::DataStream::__dispatch(::IceInternal::Incoming& in, const ::Ice::Current& current)
{
    ::std::pair< ::std::string*, ::std::string*> r = ::std::equal_range(__DataOnDemand__DataStream_all, __DataOnDemand__DataStream_all + 24, current.operation);
    if(r.first == r.second)
    {
	return ::IceInternal::DispatchOperationNotExist;
    }

    switch(r.first - __DataOnDemand__DataStream_all)
    {
	case 0:
	{
	    return ___allocDVBCResource(in, current);
	}
	case 1:
	{
	    return ___allotAccreditPathTicket(in, current);
	}
	case 2:
	{
	    return ___createMuxItem(in, current);
	}
	case 3:
	{
	    return ___destroy(in, current);
	}
	case 4:
	{
	    return ___getCurrentState(in, current);
	}
	case 5:
	{
	    return ___getIdent(in, current);
	}
	case 6:
	{
	    return ___getInfo(in, current);
	}
	case 7:
	{
	    return ___getMuxItem(in, current);
	}
	case 8:
	{
	    return ___getName(in, current);
	}
	case 9:
	{
	    return ___getProperties(in, current);
	}
	case 10:
	{
	    return ___getSession(in, current);
	}
	case 11:
	{
	    return ___ice_id(in, current);
	}
	case 12:
	{
	    return ___ice_ids(in, current);
	}
	case 13:
	{
	    return ___ice_isA(in, current);
	}
	case 14:
	{
	    return ___ice_ping(in, current);
	}
	case 15:
	{
	    return ___lastError(in, current);
	}
	case 16:
	{
	    return ___listMuxItems(in, current);
	}
	case 17:
	{
	    return ___pause(in, current);
	}
	case 18:
	{
	    return ___play(in, current);
	}
	case 19:
	{
	    return ___resume(in, current);
	}
	case 20:
	{
	    return ___setConditionalControl(in, current);
	}
	case 21:
	{
	    return ___setMuxRate(in, current);
	}
	case 22:
	{
	    return ___setProperies(in, current);
	}
	case 23:
	{
	    return ___setSpeed(in, current);
	}
    }

    assert(false);
    return ::IceInternal::DispatchOperationNotExist;
}

void
DataOnDemand::DataStream::__write(::IceInternal::BasicStream* __os) const
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
DataOnDemand::DataStream::__read(::IceInternal::BasicStream* __is, bool __rid)
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
DataOnDemand::DataStream::__write(const ::Ice::OutputStreamPtr&) const
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type DataOnDemand::DataStream was not generated with stream support";
    throw ex;
}

void
DataOnDemand::DataStream::__read(const ::Ice::InputStreamPtr&, bool)
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type DataOnDemand::DataStream was not generated with stream support";
    throw ex;
}

void 
DataOnDemand::__patch__DataStreamPtr(void* __addr, ::Ice::ObjectPtr& v)
{
    ::DataOnDemand::DataStreamPtr* p = static_cast< ::DataOnDemand::DataStreamPtr*>(__addr);
    assert(p);
    *p = ::DataOnDemand::DataStreamPtr::dynamicCast(v);
    if(v && !*p)
    {
	::Ice::NoObjectFactoryException e(__FILE__, __LINE__);
	e.type = ::DataOnDemand::DataStream::ice_staticId();
	throw e;
    }
}

bool
DataOnDemand::operator==(const ::DataOnDemand::DataStream& l, const ::DataOnDemand::DataStream& r)
{
    return static_cast<const ::Ice::Object&>(l) == static_cast<const ::Ice::Object&>(r);
}

bool
DataOnDemand::operator!=(const ::DataOnDemand::DataStream& l, const ::DataOnDemand::DataStream& r)
{
    return static_cast<const ::Ice::Object&>(l) != static_cast<const ::Ice::Object&>(r);
}

bool
DataOnDemand::operator<(const ::DataOnDemand::DataStream& l, const ::DataOnDemand::DataStream& r)
{
    return static_cast<const ::Ice::Object&>(l) < static_cast<const ::Ice::Object&>(r);
}

bool
DataOnDemand::operator<=(const ::DataOnDemand::DataStream& l, const ::DataOnDemand::DataStream& r)
{
    return l < r || l == r;
}

bool
DataOnDemand::operator>(const ::DataOnDemand::DataStream& l, const ::DataOnDemand::DataStream& r)
{
    return !(l < r) && !(l == r);
}

bool
DataOnDemand::operator>=(const ::DataOnDemand::DataStream& l, const ::DataOnDemand::DataStream& r)
{
    return !(l < r);
}

::Ice::ObjectPtr
DataOnDemand::DataStreamService::ice_clone() const
{
    throw ::Ice::CloneNotImplementedException(__FILE__, __LINE__);
    return 0; // to avoid a warning with some compilers
}

static const ::std::string __DataOnDemand__DataStreamService_ids[4] =
{
    "::DataOnDemand::DataStreamService",
    "::Ice::Object",
    "::TianShanIce::BaseService",
    "::TianShanIce::Streamer::StreamService"
};

bool
DataOnDemand::DataStreamService::ice_isA(const ::std::string& _s, const ::Ice::Current&) const
{
    return ::std::binary_search(__DataOnDemand__DataStreamService_ids, __DataOnDemand__DataStreamService_ids + 4, _s);
}

::std::vector< ::std::string>
DataOnDemand::DataStreamService::ice_ids(const ::Ice::Current&) const
{
    return ::std::vector< ::std::string>(&__DataOnDemand__DataStreamService_ids[0], &__DataOnDemand__DataStreamService_ids[4]);
}

const ::std::string&
DataOnDemand::DataStreamService::ice_id(const ::Ice::Current&) const
{
    return __DataOnDemand__DataStreamService_ids[0];
}

const ::std::string&
DataOnDemand::DataStreamService::ice_staticId()
{
    return __DataOnDemand__DataStreamService_ids[0];
}

::IceInternal::DispatchStatus
DataOnDemand::DataStreamService::___createStreamByApp(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::TianShanIce::AccreditedPath::PathTicketPrx pathTicket;
    ::std::string name;
    ::DataOnDemand::StreamInfo info;
    ::TianShanIce::AccreditedPath::__read(__is, pathTicket);
    __is->read(name);
    info.__read(__is);
    try
    {
	::DataOnDemand::DataStreamPrx __ret = createStreamByApp(pathTicket, name, info, __current);
	::DataOnDemand::__write(__os, __ret);
    }
    catch(const ::DataOnDemand::NameDupException& __ex)
    {
	__os->write(__ex);
	return ::IceInternal::DispatchUserException;
    }
    catch(const ::TianShanIce::InvalidParameter& __ex)
    {
	__os->write(__ex);
	return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
DataOnDemand::DataStreamService::___getStream(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string name;
    __is->read(name);
    ::DataOnDemand::DataStreamPrx __ret = getStream(name, __current);
    ::DataOnDemand::__write(__os, __ret);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
DataOnDemand::DataStreamService::___setProperies(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::TianShanIce::Properties props;
    ::TianShanIce::__read(__is, props, ::TianShanIce::__U__Properties());
    setProperies(props, __current);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
DataOnDemand::DataStreamService::___getProperties(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Nonmutating, __current.mode);
    ::IceInternal::BasicStream* __os = __inS.os();
    ::TianShanIce::Properties __ret = getProperties(__current);
    ::TianShanIce::__write(__os, __ret, ::TianShanIce::__U__Properties());
    return ::IceInternal::DispatchOK;
}

static ::std::string __DataOnDemand__DataStreamService_all[] =
{
    "createStream",
    "createStreamByApp",
    "getAdminUri",
    "getNetId",
    "getProperties",
    "getState",
    "getStream",
    "ice_id",
    "ice_ids",
    "ice_isA",
    "ice_ping",
    "listStreamers",
    "setProperies"
};

::IceInternal::DispatchStatus
DataOnDemand::DataStreamService::__dispatch(::IceInternal::Incoming& in, const ::Ice::Current& current)
{
    ::std::pair< ::std::string*, ::std::string*> r = ::std::equal_range(__DataOnDemand__DataStreamService_all, __DataOnDemand__DataStreamService_all + 13, current.operation);
    if(r.first == r.second)
    {
	return ::IceInternal::DispatchOperationNotExist;
    }

    switch(r.first - __DataOnDemand__DataStreamService_all)
    {
	case 0:
	{
	    return ___createStream(in, current);
	}
	case 1:
	{
	    return ___createStreamByApp(in, current);
	}
	case 2:
	{
	    return ___getAdminUri(in, current);
	}
	case 3:
	{
	    return ___getNetId(in, current);
	}
	case 4:
	{
	    return ___getProperties(in, current);
	}
	case 5:
	{
	    return ___getState(in, current);
	}
	case 6:
	{
	    return ___getStream(in, current);
	}
	case 7:
	{
	    return ___ice_id(in, current);
	}
	case 8:
	{
	    return ___ice_ids(in, current);
	}
	case 9:
	{
	    return ___ice_isA(in, current);
	}
	case 10:
	{
	    return ___ice_ping(in, current);
	}
	case 11:
	{
	    return ___listStreamers(in, current);
	}
	case 12:
	{
	    return ___setProperies(in, current);
	}
    }

    assert(false);
    return ::IceInternal::DispatchOperationNotExist;
}

void
DataOnDemand::DataStreamService::__write(::IceInternal::BasicStream* __os) const
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
DataOnDemand::DataStreamService::__read(::IceInternal::BasicStream* __is, bool __rid)
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
DataOnDemand::DataStreamService::__write(const ::Ice::OutputStreamPtr&) const
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type DataOnDemand::DataStreamService was not generated with stream support";
    throw ex;
}

void
DataOnDemand::DataStreamService::__read(const ::Ice::InputStreamPtr&, bool)
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type DataOnDemand::DataStreamService was not generated with stream support";
    throw ex;
}

void 
DataOnDemand::__patch__DataStreamServicePtr(void* __addr, ::Ice::ObjectPtr& v)
{
    ::DataOnDemand::DataStreamServicePtr* p = static_cast< ::DataOnDemand::DataStreamServicePtr*>(__addr);
    assert(p);
    *p = ::DataOnDemand::DataStreamServicePtr::dynamicCast(v);
    if(v && !*p)
    {
	::Ice::NoObjectFactoryException e(__FILE__, __LINE__);
	e.type = ::DataOnDemand::DataStreamService::ice_staticId();
	throw e;
    }
}

bool
DataOnDemand::operator==(const ::DataOnDemand::DataStreamService& l, const ::DataOnDemand::DataStreamService& r)
{
    return static_cast<const ::Ice::Object&>(l) == static_cast<const ::Ice::Object&>(r);
}

bool
DataOnDemand::operator!=(const ::DataOnDemand::DataStreamService& l, const ::DataOnDemand::DataStreamService& r)
{
    return static_cast<const ::Ice::Object&>(l) != static_cast<const ::Ice::Object&>(r);
}

bool
DataOnDemand::operator<(const ::DataOnDemand::DataStreamService& l, const ::DataOnDemand::DataStreamService& r)
{
    return static_cast<const ::Ice::Object&>(l) < static_cast<const ::Ice::Object&>(r);
}

bool
DataOnDemand::operator<=(const ::DataOnDemand::DataStreamService& l, const ::DataOnDemand::DataStreamService& r)
{
    return l < r || l == r;
}

bool
DataOnDemand::operator>(const ::DataOnDemand::DataStreamService& l, const ::DataOnDemand::DataStreamService& r)
{
    return !(l < r) && !(l == r);
}

bool
DataOnDemand::operator>=(const ::DataOnDemand::DataStreamService& l, const ::DataOnDemand::DataStreamService& r)
{
    return !(l < r);
}
