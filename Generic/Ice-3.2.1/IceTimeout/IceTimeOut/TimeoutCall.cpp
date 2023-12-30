// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// Ice version 3.2.1
// Generated from file `TimeoutCall.ice'

#include <TimeoutCall.h>
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

static const ::std::string __test__timeout__call_name = "call";

void
IceInternal::incRef(::test::timeout* p)
{
    p->__incRef();
}

void
IceInternal::decRef(::test::timeout* p)
{
    p->__decRef();
}

void
IceInternal::incRef(::IceProxy::test::timeout* p)
{
    p->__incRef();
}

void
IceInternal::decRef(::IceProxy::test::timeout* p)
{
    p->__decRef();
}

void
test::__write(::IceInternal::BasicStream* __os, const ::test::timeoutPrx& v)
{
    __os->write(::Ice::ObjectPrx(v));
}

void
test::__read(::IceInternal::BasicStream* __is, ::test::timeoutPrx& v)
{
    ::Ice::ObjectPrx proxy;
    __is->read(proxy);
    if(!proxy)
    {
        v = 0;
    }
    else
    {
        v = new ::IceProxy::test::timeout;
        v->__copyFrom(proxy);
    }
}

void
test::__write(::IceInternal::BasicStream* __os, const ::test::timeoutPtr& v)
{
    __os->write(::Ice::ObjectPtr(v));
}

void
test::__addObject(const timeoutPtr& p, ::IceInternal::GCCountMap& c)
{
    p->__addObject(c);
}

bool
test::__usesClasses(const timeoutPtr& p)
{
    return p->__usesClasses();
}

void
test::__decRefUnsafe(const timeoutPtr& p)
{
    p->__decRefUnsafe();
}

void
test::__clearHandleUnsafe(timeoutPtr& p)
{
    p.__clearHandleUnsafe();
}

void
test::AMI_timeout_call::__invoke(const ::test::timeoutPrx& __prx, ::Ice::Int clientId, ::Ice::Int t, const ::Ice::Context* __ctx)
{
    try
    {
        __prepare(__prx, __test__timeout__call_name, ::Ice::Normal, __ctx);
        __os->write(clientId);
        __os->write(t);
        __os->endWriteEncaps();
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __finished(__ex);
        return;
    }
    __send();
}

void
test::AMI_timeout_call::__response(bool __ok)
{
    ::Ice::Int __ret;
    try
    {
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
        __is->read(__ret);
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __finished(__ex);
        return;
    }
    ice_response(__ret);
}

::Ice::Int
IceProxy::test::timeout::call(::Ice::Int clientId, ::Ice::Int t, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__test__timeout__call_name);
            __delBase = __getDelegate();
            ::IceDelegate::test::timeout* __del = dynamic_cast< ::IceDelegate::test::timeout*>(__delBase.get());
            return __del->call(clientId, t, __ctx);
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
IceProxy::test::timeout::call_async(const ::test::AMI_timeout_callPtr& __cb, ::Ice::Int clientId, ::Ice::Int t)
{
    __cb->__invoke(this, clientId, t, 0);
}

void
IceProxy::test::timeout::call_async(const ::test::AMI_timeout_callPtr& __cb, ::Ice::Int clientId, ::Ice::Int t, const ::Ice::Context& __ctx)
{
    __cb->__invoke(this, clientId, t, &__ctx);
}

const ::std::string&
IceProxy::test::timeout::ice_staticId()
{
    return ::test::timeout::ice_staticId();
}

::IceInternal::Handle< ::IceDelegateM::Ice::Object>
IceProxy::test::timeout::__createDelegateM()
{
    return ::IceInternal::Handle< ::IceDelegateM::Ice::Object>(new ::IceDelegateM::test::timeout);
}

::IceInternal::Handle< ::IceDelegateD::Ice::Object>
IceProxy::test::timeout::__createDelegateD()
{
    return ::IceInternal::Handle< ::IceDelegateD::Ice::Object>(new ::IceDelegateD::test::timeout);
}

bool
IceProxy::test::operator==(const ::IceProxy::test::timeout& l, const ::IceProxy::test::timeout& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) == static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::test::operator!=(const ::IceProxy::test::timeout& l, const ::IceProxy::test::timeout& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) != static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::test::operator<(const ::IceProxy::test::timeout& l, const ::IceProxy::test::timeout& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) < static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::test::operator<=(const ::IceProxy::test::timeout& l, const ::IceProxy::test::timeout& r)
{
    return l < r || l == r;
}

bool
IceProxy::test::operator>(const ::IceProxy::test::timeout& l, const ::IceProxy::test::timeout& r)
{
    return !(l < r) && !(l == r);
}

bool
IceProxy::test::operator>=(const ::IceProxy::test::timeout& l, const ::IceProxy::test::timeout& r)
{
    return !(l < r);
}

::Ice::Int
IceDelegateM::test::timeout::call(::Ice::Int clientId, ::Ice::Int t, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __test__timeout__call_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(clientId);
        __os->write(t);
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
        ::Ice::Int __ret;
        __is->read(__ret);
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::Ice::Int
IceDelegateD::test::timeout::call(::Ice::Int clientId, ::Ice::Int t, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __test__timeout__call_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::Ice::Int __ret;
        try
        {
            ::test::timeout* __servant = dynamic_cast< ::test::timeout*>(__direct.servant().get());
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
                __ret = __servant->call(clientId, t, __current);
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

::Ice::ObjectPtr
test::timeout::ice_clone() const
{
    throw ::Ice::CloneNotImplementedException(__FILE__, __LINE__);
    return 0; // to avoid a warning with some compilers
}

static const ::std::string __test__timeout_ids[2] =
{
    "::Ice::Object",
    "::test::timeout"
};

bool
test::timeout::ice_isA(const ::std::string& _s, const ::Ice::Current&) const
{
    return ::std::binary_search(__test__timeout_ids, __test__timeout_ids + 2, _s);
}

::std::vector< ::std::string>
test::timeout::ice_ids(const ::Ice::Current&) const
{
    return ::std::vector< ::std::string>(&__test__timeout_ids[0], &__test__timeout_ids[2]);
}

const ::std::string&
test::timeout::ice_id(const ::Ice::Current&) const
{
    return __test__timeout_ids[1];
}

const ::std::string&
test::timeout::ice_staticId()
{
    return __test__timeout_ids[1];
}

::IceInternal::DispatchStatus
test::timeout::___call(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::Ice::Int clientId;
    ::Ice::Int t;
    __is->read(clientId);
    __is->read(t);
    ::Ice::Int __ret = call(clientId, t, __current);
    __os->write(__ret);
    return ::IceInternal::DispatchOK;
}

static ::std::string __test__timeout_all[] =
{
    "call",
    "ice_id",
    "ice_ids",
    "ice_isA",
    "ice_ping"
};

::IceInternal::DispatchStatus
test::timeout::__dispatch(::IceInternal::Incoming& in, const ::Ice::Current& current)
{
    ::std::pair< ::std::string*, ::std::string*> r = ::std::equal_range(__test__timeout_all, __test__timeout_all + 5, current.operation);
    if(r.first == r.second)
    {
        return ::IceInternal::DispatchOperationNotExist;
    }

    switch(r.first - __test__timeout_all)
    {
        case 0:
        {
            return ___call(in, current);
        }
        case 1:
        {
            return ___ice_id(in, current);
        }
        case 2:
        {
            return ___ice_ids(in, current);
        }
        case 3:
        {
            return ___ice_isA(in, current);
        }
        case 4:
        {
            return ___ice_ping(in, current);
        }
    }

    assert(false);
    return ::IceInternal::DispatchOperationNotExist;
}

void
test::timeout::__write(::IceInternal::BasicStream* __os) const
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
test::timeout::__read(::IceInternal::BasicStream* __is, bool __rid)
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
test::timeout::__write(const ::Ice::OutputStreamPtr&) const
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type test::timeout was not generated with stream support";
    throw ex;
}

void
test::timeout::__read(const ::Ice::InputStreamPtr&, bool)
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type test::timeout was not generated with stream support";
    throw ex;
}

void 
test::__patch__timeoutPtr(void* __addr, ::Ice::ObjectPtr& v)
{
    ::test::timeoutPtr* p = static_cast< ::test::timeoutPtr*>(__addr);
    assert(p);
    *p = ::test::timeoutPtr::dynamicCast(v);
    if(v && !*p)
    {
        ::Ice::UnexpectedObjectException e(__FILE__, __LINE__);
        e.type = v->ice_id();
        e.expectedType = ::test::timeout::ice_staticId();
        throw e;
    }
}

bool
test::operator==(const ::test::timeout& l, const ::test::timeout& r)
{
    return static_cast<const ::Ice::Object&>(l) == static_cast<const ::Ice::Object&>(r);
}

bool
test::operator!=(const ::test::timeout& l, const ::test::timeout& r)
{
    return static_cast<const ::Ice::Object&>(l) != static_cast<const ::Ice::Object&>(r);
}

bool
test::operator<(const ::test::timeout& l, const ::test::timeout& r)
{
    return static_cast<const ::Ice::Object&>(l) < static_cast<const ::Ice::Object&>(r);
}

bool
test::operator<=(const ::test::timeout& l, const ::test::timeout& r)
{
    return l < r || l == r;
}

bool
test::operator>(const ::test::timeout& l, const ::test::timeout& r)
{
    return !(l < r) && !(l == r);
}

bool
test::operator>=(const ::test::timeout& l, const ::test::timeout& r)
{
    return !(l < r);
}
