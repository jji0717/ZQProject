// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// Ice version 3.2.1
// Generated from file `LogPosition.ice'

#include <LogPosition.h>
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

static const ::std::string __EventSink__LogPosition__getSourceName_name = "getSourceName";

static const ::std::string __EventSink__LogPosition__getHandlerName_name = "getHandlerName";

static const ::std::string __EventSink__LogPosition__getLastUpdatedTime_name = "getLastUpdatedTime";

static const ::std::string __EventSink__LogPosition__getData_name = "getData";

static const ::std::string __EventSink__LogPosition__updateData_name = "updateData";

void
IceInternal::incRef(::EventSink::LogPosition* p)
{
    p->__incRef();
}

void
IceInternal::decRef(::EventSink::LogPosition* p)
{
    p->__decRef();
}

void
IceInternal::incRef(::IceProxy::EventSink::LogPosition* p)
{
    p->__incRef();
}

void
IceInternal::decRef(::IceProxy::EventSink::LogPosition* p)
{
    p->__decRef();
}

void
EventSink::__write(::IceInternal::BasicStream* __os, const ::EventSink::LogPositionPrx& v)
{
    __os->write(::Ice::ObjectPrx(v));
}

void
EventSink::__read(::IceInternal::BasicStream* __is, ::EventSink::LogPositionPrx& v)
{
    ::Ice::ObjectPrx proxy;
    __is->read(proxy);
    if(!proxy)
    {
        v = 0;
    }
    else
    {
        v = new ::IceProxy::EventSink::LogPosition;
        v->__copyFrom(proxy);
    }
}

void
EventSink::__write(::IceInternal::BasicStream* __os, const ::EventSink::LogPositionPtr& v)
{
    __os->write(::Ice::ObjectPtr(v));
}

bool
EventSink::PositionData::operator==(const PositionData& __rhs) const
{
    return !operator!=(__rhs);
}

bool
EventSink::PositionData::operator!=(const PositionData& __rhs) const
{
    if(this == &__rhs)
    {
        return false;
    }
    if(source != __rhs.source)
    {
        return true;
    }
    if(handler != __rhs.handler)
    {
        return true;
    }
    if(position != __rhs.position)
    {
        return true;
    }
    if(stamp != __rhs.stamp)
    {
        return true;
    }
    if(lastUpdated != __rhs.lastUpdated)
    {
        return true;
    }
    return false;
}

bool
EventSink::PositionData::operator<(const PositionData& __rhs) const
{
    if(this == &__rhs)
    {
        return false;
    }
    if(source < __rhs.source)
    {
        return true;
    }
    else if(__rhs.source < source)
    {
        return false;
    }
    if(handler < __rhs.handler)
    {
        return true;
    }
    else if(__rhs.handler < handler)
    {
        return false;
    }
    if(position < __rhs.position)
    {
        return true;
    }
    else if(__rhs.position < position)
    {
        return false;
    }
    if(stamp < __rhs.stamp)
    {
        return true;
    }
    else if(__rhs.stamp < stamp)
    {
        return false;
    }
    if(lastUpdated < __rhs.lastUpdated)
    {
        return true;
    }
    else if(__rhs.lastUpdated < lastUpdated)
    {
        return false;
    }
    return false;
}

void
EventSink::PositionData::__write(::IceInternal::BasicStream* __os) const
{
    __os->write(source);
    __os->write(handler);
    __os->write(position);
    __os->write(stamp);
    __os->write(lastUpdated);
}

void
EventSink::PositionData::__read(::IceInternal::BasicStream* __is)
{
    __is->read(source);
    __is->read(handler);
    __is->read(position);
    __is->read(stamp);
    __is->read(lastUpdated);
}

void
EventSink::__addObject(const LogPositionPtr& p, ::IceInternal::GCCountMap& c)
{
    p->__addObject(c);
}

bool
EventSink::__usesClasses(const LogPositionPtr& p)
{
    return p->__usesClasses();
}

void
EventSink::__decRefUnsafe(const LogPositionPtr& p)
{
    p->__decRefUnsafe();
}

void
EventSink::__clearHandleUnsafe(LogPositionPtr& p)
{
    p.__clearHandleUnsafe();
}

::std::string
IceProxy::EventSink::LogPosition::getSourceName(const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__EventSink__LogPosition__getSourceName_name);
            __delBase = __getDelegate();
            ::IceDelegate::EventSink::LogPosition* __del = dynamic_cast< ::IceDelegate::EventSink::LogPosition*>(__delBase.get());
            return __del->getSourceName(__ctx);
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

::std::string
IceProxy::EventSink::LogPosition::getHandlerName(const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__EventSink__LogPosition__getHandlerName_name);
            __delBase = __getDelegate();
            ::IceDelegate::EventSink::LogPosition* __del = dynamic_cast< ::IceDelegate::EventSink::LogPosition*>(__delBase.get());
            return __del->getHandlerName(__ctx);
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

::Ice::Long
IceProxy::EventSink::LogPosition::getLastUpdatedTime(const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__EventSink__LogPosition__getLastUpdatedTime_name);
            __delBase = __getDelegate();
            ::IceDelegate::EventSink::LogPosition* __del = dynamic_cast< ::IceDelegate::EventSink::LogPosition*>(__delBase.get());
            return __del->getLastUpdatedTime(__ctx);
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
IceProxy::EventSink::LogPosition::getData(::Ice::Long& position, ::Ice::Long& stamp, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__EventSink__LogPosition__getData_name);
            __delBase = __getDelegate();
            ::IceDelegate::EventSink::LogPosition* __del = dynamic_cast< ::IceDelegate::EventSink::LogPosition*>(__delBase.get());
            __del->getData(position, stamp, __ctx);
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
IceProxy::EventSink::LogPosition::updateData(::Ice::Long position, ::Ice::Long stamp, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __delBase = __getDelegate();
            ::IceDelegate::EventSink::LogPosition* __del = dynamic_cast< ::IceDelegate::EventSink::LogPosition*>(__delBase.get());
            __del->updateData(position, stamp, __ctx);
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
IceProxy::EventSink::LogPosition::ice_staticId()
{
    return ::EventSink::LogPosition::ice_staticId();
}

::IceInternal::Handle< ::IceDelegateM::Ice::Object>
IceProxy::EventSink::LogPosition::__createDelegateM()
{
    return ::IceInternal::Handle< ::IceDelegateM::Ice::Object>(new ::IceDelegateM::EventSink::LogPosition);
}

::IceInternal::Handle< ::IceDelegateD::Ice::Object>
IceProxy::EventSink::LogPosition::__createDelegateD()
{
    return ::IceInternal::Handle< ::IceDelegateD::Ice::Object>(new ::IceDelegateD::EventSink::LogPosition);
}

bool
IceProxy::EventSink::operator==(const ::IceProxy::EventSink::LogPosition& l, const ::IceProxy::EventSink::LogPosition& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) == static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::EventSink::operator!=(const ::IceProxy::EventSink::LogPosition& l, const ::IceProxy::EventSink::LogPosition& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) != static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::EventSink::operator<(const ::IceProxy::EventSink::LogPosition& l, const ::IceProxy::EventSink::LogPosition& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) < static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::EventSink::operator<=(const ::IceProxy::EventSink::LogPosition& l, const ::IceProxy::EventSink::LogPosition& r)
{
    return l < r || l == r;
}

bool
IceProxy::EventSink::operator>(const ::IceProxy::EventSink::LogPosition& l, const ::IceProxy::EventSink::LogPosition& r)
{
    return !(l < r) && !(l == r);
}

bool
IceProxy::EventSink::operator>=(const ::IceProxy::EventSink::LogPosition& l, const ::IceProxy::EventSink::LogPosition& r)
{
    return !(l < r);
}

::std::string
IceDelegateM::EventSink::LogPosition::getSourceName(const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __EventSink__LogPosition__getSourceName_name, ::Ice::Normal, __context, __compress);
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

::std::string
IceDelegateM::EventSink::LogPosition::getHandlerName(const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __EventSink__LogPosition__getHandlerName_name, ::Ice::Normal, __context, __compress);
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

::Ice::Long
IceDelegateM::EventSink::LogPosition::getLastUpdatedTime(const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __EventSink__LogPosition__getLastUpdatedTime_name, ::Ice::Normal, __context, __compress);
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
        ::Ice::Long __ret;
        __is->read(__ret);
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateM::EventSink::LogPosition::getData(::Ice::Long& position, ::Ice::Long& stamp, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __EventSink__LogPosition__getData_name, ::Ice::Normal, __context, __compress);
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
        __is->read(position);
        __is->read(stamp);
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateM::EventSink::LogPosition::updateData(::Ice::Long position, ::Ice::Long stamp, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __EventSink__LogPosition__updateData_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(position);
        __os->write(stamp);
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
IceDelegateD::EventSink::LogPosition::getSourceName(const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __EventSink__LogPosition__getSourceName_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::std::string __ret;
        try
        {
            ::EventSink::LogPosition* __servant = dynamic_cast< ::EventSink::LogPosition*>(__direct.servant().get());
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
                __ret = __servant->getSourceName(__current);
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
IceDelegateD::EventSink::LogPosition::getHandlerName(const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __EventSink__LogPosition__getHandlerName_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::std::string __ret;
        try
        {
            ::EventSink::LogPosition* __servant = dynamic_cast< ::EventSink::LogPosition*>(__direct.servant().get());
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
                __ret = __servant->getHandlerName(__current);
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

::Ice::Long
IceDelegateD::EventSink::LogPosition::getLastUpdatedTime(const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __EventSink__LogPosition__getLastUpdatedTime_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::Ice::Long __ret;
        try
        {
            ::EventSink::LogPosition* __servant = dynamic_cast< ::EventSink::LogPosition*>(__direct.servant().get());
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
                __ret = __servant->getLastUpdatedTime(__current);
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
IceDelegateD::EventSink::LogPosition::getData(::Ice::Long& position, ::Ice::Long& stamp, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __EventSink__LogPosition__getData_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        try
        {
            ::EventSink::LogPosition* __servant = dynamic_cast< ::EventSink::LogPosition*>(__direct.servant().get());
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
                __servant->getData(position, stamp, __current);
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
IceDelegateD::EventSink::LogPosition::updateData(::Ice::Long position, ::Ice::Long stamp, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __EventSink__LogPosition__updateData_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        try
        {
            ::EventSink::LogPosition* __servant = dynamic_cast< ::EventSink::LogPosition*>(__direct.servant().get());
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
                __servant->updateData(position, stamp, __current);
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

EventSink::LogPosition::LogPosition(const ::EventSink::PositionData& __ice_data) :
    data(__ice_data)
{
}

::Ice::ObjectPtr
EventSink::LogPosition::ice_clone() const
{
    throw ::Ice::CloneNotImplementedException(__FILE__, __LINE__);
    return 0; // to avoid a warning with some compilers
}

static const ::std::string __EventSink__LogPosition_ids[2] =
{
    "::EventSink::LogPosition",
    "::Ice::Object"
};

bool
EventSink::LogPosition::ice_isA(const ::std::string& _s, const ::Ice::Current&) const
{
    return ::std::binary_search(__EventSink__LogPosition_ids, __EventSink__LogPosition_ids + 2, _s);
}

::std::vector< ::std::string>
EventSink::LogPosition::ice_ids(const ::Ice::Current&) const
{
    return ::std::vector< ::std::string>(&__EventSink__LogPosition_ids[0], &__EventSink__LogPosition_ids[2]);
}

const ::std::string&
EventSink::LogPosition::ice_id(const ::Ice::Current&) const
{
    return __EventSink__LogPosition_ids[0];
}

const ::std::string&
EventSink::LogPosition::ice_staticId()
{
    return __EventSink__LogPosition_ids[0];
}

::IceInternal::DispatchStatus
EventSink::LogPosition::___getSourceName(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string __ret = getSourceName(__current);
    __os->write(__ret);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
EventSink::LogPosition::___getHandlerName(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string __ret = getHandlerName(__current);
    __os->write(__ret);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
EventSink::LogPosition::___getLastUpdatedTime(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __os = __inS.os();
    ::Ice::Long __ret = getLastUpdatedTime(__current);
    __os->write(__ret);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
EventSink::LogPosition::___getData(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __os = __inS.os();
    ::Ice::Long position;
    ::Ice::Long stamp;
    getData(position, stamp, __current);
    __os->write(position);
    __os->write(stamp);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
EventSink::LogPosition::___updateData(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::Ice::Long position;
    ::Ice::Long stamp;
    __is->read(position);
    __is->read(stamp);
    updateData(position, stamp, __current);
    return ::IceInternal::DispatchOK;
}

static ::std::string __EventSink__LogPosition_all[] =
{
    "getData",
    "getHandlerName",
    "getLastUpdatedTime",
    "getSourceName",
    "ice_id",
    "ice_ids",
    "ice_isA",
    "ice_ping",
    "updateData"
};

::IceInternal::DispatchStatus
EventSink::LogPosition::__dispatch(::IceInternal::Incoming& in, const ::Ice::Current& current)
{
    ::std::pair< ::std::string*, ::std::string*> r = ::std::equal_range(__EventSink__LogPosition_all, __EventSink__LogPosition_all + 9, current.operation);
    if(r.first == r.second)
    {
        return ::IceInternal::DispatchOperationNotExist;
    }

    switch(r.first - __EventSink__LogPosition_all)
    {
        case 0:
        {
            return ___getData(in, current);
        }
        case 1:
        {
            return ___getHandlerName(in, current);
        }
        case 2:
        {
            return ___getLastUpdatedTime(in, current);
        }
        case 3:
        {
            return ___getSourceName(in, current);
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
            return ___updateData(in, current);
        }
    }

    assert(false);
    return ::IceInternal::DispatchOperationNotExist;
}

static ::std::string __EventSink__LogPosition_freezeWriteOperations[] =
{
    "updateData"
};

::Ice::Int
EventSink::LogPosition::ice_operationAttributes(const ::std::string& opName) const
{
    ::std::string* end = __EventSink__LogPosition_freezeWriteOperations + 1;
    ::std::string* r = ::std::find(__EventSink__LogPosition_freezeWriteOperations, end, opName);
    return r == end ? 0 : 1;
}

void
EventSink::LogPosition::__write(::IceInternal::BasicStream* __os) const
{
    __os->writeTypeId(ice_staticId());
    __os->startWriteSlice();
    data.__write(__os);
    __os->endWriteSlice();
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    Object::__write(__os);
#else
    ::Ice::Object::__write(__os);
#endif
}

void
EventSink::LogPosition::__read(::IceInternal::BasicStream* __is, bool __rid)
{
    if(__rid)
    {
        ::std::string myId;
        __is->readTypeId(myId);
    }
    __is->startReadSlice();
    data.__read(__is);
    __is->endReadSlice();
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    Object::__read(__is, true);
#else
    ::Ice::Object::__read(__is, true);
#endif
}

void
EventSink::LogPosition::__write(const ::Ice::OutputStreamPtr&) const
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type EventSink::LogPosition was not generated with stream support";
    throw ex;
}

void
EventSink::LogPosition::__read(const ::Ice::InputStreamPtr&, bool)
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type EventSink::LogPosition was not generated with stream support";
    throw ex;
}

void 
EventSink::__patch__LogPositionPtr(void* __addr, ::Ice::ObjectPtr& v)
{
    ::EventSink::LogPositionPtr* p = static_cast< ::EventSink::LogPositionPtr*>(__addr);
    assert(p);
    *p = ::EventSink::LogPositionPtr::dynamicCast(v);
    if(v && !*p)
    {
        ::Ice::UnexpectedObjectException e(__FILE__, __LINE__);
        e.type = v->ice_id();
        e.expectedType = ::EventSink::LogPosition::ice_staticId();
        throw e;
    }
}

bool
EventSink::operator==(const ::EventSink::LogPosition& l, const ::EventSink::LogPosition& r)
{
    return static_cast<const ::Ice::Object&>(l) == static_cast<const ::Ice::Object&>(r);
}

bool
EventSink::operator!=(const ::EventSink::LogPosition& l, const ::EventSink::LogPosition& r)
{
    return static_cast<const ::Ice::Object&>(l) != static_cast<const ::Ice::Object&>(r);
}

bool
EventSink::operator<(const ::EventSink::LogPosition& l, const ::EventSink::LogPosition& r)
{
    return static_cast<const ::Ice::Object&>(l) < static_cast<const ::Ice::Object&>(r);
}

bool
EventSink::operator<=(const ::EventSink::LogPosition& l, const ::EventSink::LogPosition& r)
{
    return l < r || l == r;
}

bool
EventSink::operator>(const ::EventSink::LogPosition& l, const ::EventSink::LogPosition& r)
{
    return !(l < r) && !(l == r);
}

bool
EventSink::operator>=(const ::EventSink::LogPosition& l, const ::EventSink::LogPosition& r)
{
    return !(l < r);
}
