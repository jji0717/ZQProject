// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// Ice version 3.2.1
// Generated from file `PhoAllocation.ice'

#include <PhoAllocation.h>
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

static const ::std::string __TianShanIce__EdgeResource__PhoAllocation__getIdent_name = "getIdent";

static const ::std::string __TianShanIce__EdgeResource__PhoAllocation__getAllocation_name = "getAllocation";

static const ::std::string __TianShanIce__EdgeResource__PhoAllocation__getSessionGroup_name = "getSessionGroup";

static const ::std::string __TianShanIce__EdgeResource__PhoAllocation__getSessKey_name = "getSessKey";

void
IceInternal::incRef(::TianShanIce::EdgeResource::PhoAllocation* p)
{
    p->__incRef();
}

void
IceInternal::decRef(::TianShanIce::EdgeResource::PhoAllocation* p)
{
    p->__decRef();
}

void
IceInternal::incRef(::IceProxy::TianShanIce::EdgeResource::PhoAllocation* p)
{
    p->__incRef();
}

void
IceInternal::decRef(::IceProxy::TianShanIce::EdgeResource::PhoAllocation* p)
{
    p->__decRef();
}

void
TianShanIce::EdgeResource::__write(::IceInternal::BasicStream* __os, const ::TianShanIce::EdgeResource::PhoAllocationPrx& v)
{
    __os->write(::Ice::ObjectPrx(v));
}

void
TianShanIce::EdgeResource::__read(::IceInternal::BasicStream* __is, ::TianShanIce::EdgeResource::PhoAllocationPrx& v)
{
    ::Ice::ObjectPrx proxy;
    __is->read(proxy);
    if(!proxy)
    {
        v = 0;
    }
    else
    {
        v = new ::IceProxy::TianShanIce::EdgeResource::PhoAllocation;
        v->__copyFrom(proxy);
    }
}

void
TianShanIce::EdgeResource::__write(::IceInternal::BasicStream* __os, const ::TianShanIce::EdgeResource::PhoAllocationPtr& v)
{
    __os->write(::Ice::ObjectPtr(v));
}

void
TianShanIce::EdgeResource::__addObject(const PhoAllocationPtr& p, ::IceInternal::GCCountMap& c)
{
    p->__addObject(c);
}

bool
TianShanIce::EdgeResource::__usesClasses(const PhoAllocationPtr& p)
{
    return p->__usesClasses();
}

void
TianShanIce::EdgeResource::__decRefUnsafe(const PhoAllocationPtr& p)
{
    p->__decRefUnsafe();
}

void
TianShanIce::EdgeResource::__clearHandleUnsafe(PhoAllocationPtr& p)
{
    p.__clearHandleUnsafe();
}

::Ice::Identity
IceProxy::TianShanIce::EdgeResource::PhoAllocation::getIdent(const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__TianShanIce__EdgeResource__PhoAllocation__getIdent_name);
            __delBase = __getDelegate();
            ::IceDelegate::TianShanIce::EdgeResource::PhoAllocation* __del = dynamic_cast< ::IceDelegate::TianShanIce::EdgeResource::PhoAllocation*>(__delBase.get());
            return __del->getIdent(__ctx);
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

::TianShanIce::EdgeResource::AllocationPrx
IceProxy::TianShanIce::EdgeResource::PhoAllocation::getAllocation(const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__TianShanIce__EdgeResource__PhoAllocation__getAllocation_name);
            __delBase = __getDelegate();
            ::IceDelegate::TianShanIce::EdgeResource::PhoAllocation* __del = dynamic_cast< ::IceDelegate::TianShanIce::EdgeResource::PhoAllocation*>(__delBase.get());
            return __del->getAllocation(__ctx);
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
IceProxy::TianShanIce::EdgeResource::PhoAllocation::getSessionGroup(const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__TianShanIce__EdgeResource__PhoAllocation__getSessionGroup_name);
            __delBase = __getDelegate();
            ::IceDelegate::TianShanIce::EdgeResource::PhoAllocation* __del = dynamic_cast< ::IceDelegate::TianShanIce::EdgeResource::PhoAllocation*>(__delBase.get());
            return __del->getSessionGroup(__ctx);
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
IceProxy::TianShanIce::EdgeResource::PhoAllocation::getSessKey(const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__TianShanIce__EdgeResource__PhoAllocation__getSessKey_name);
            __delBase = __getDelegate();
            ::IceDelegate::TianShanIce::EdgeResource::PhoAllocation* __del = dynamic_cast< ::IceDelegate::TianShanIce::EdgeResource::PhoAllocation*>(__delBase.get());
            return __del->getSessKey(__ctx);
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
IceProxy::TianShanIce::EdgeResource::PhoAllocation::ice_staticId()
{
    return ::TianShanIce::EdgeResource::PhoAllocation::ice_staticId();
}

::IceInternal::Handle< ::IceDelegateM::Ice::Object>
IceProxy::TianShanIce::EdgeResource::PhoAllocation::__createDelegateM()
{
    return ::IceInternal::Handle< ::IceDelegateM::Ice::Object>(new ::IceDelegateM::TianShanIce::EdgeResource::PhoAllocation);
}

::IceInternal::Handle< ::IceDelegateD::Ice::Object>
IceProxy::TianShanIce::EdgeResource::PhoAllocation::__createDelegateD()
{
    return ::IceInternal::Handle< ::IceDelegateD::Ice::Object>(new ::IceDelegateD::TianShanIce::EdgeResource::PhoAllocation);
}

bool
IceProxy::TianShanIce::EdgeResource::operator==(const ::IceProxy::TianShanIce::EdgeResource::PhoAllocation& l, const ::IceProxy::TianShanIce::EdgeResource::PhoAllocation& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) == static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::TianShanIce::EdgeResource::operator!=(const ::IceProxy::TianShanIce::EdgeResource::PhoAllocation& l, const ::IceProxy::TianShanIce::EdgeResource::PhoAllocation& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) != static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::TianShanIce::EdgeResource::operator<(const ::IceProxy::TianShanIce::EdgeResource::PhoAllocation& l, const ::IceProxy::TianShanIce::EdgeResource::PhoAllocation& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) < static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::TianShanIce::EdgeResource::operator<=(const ::IceProxy::TianShanIce::EdgeResource::PhoAllocation& l, const ::IceProxy::TianShanIce::EdgeResource::PhoAllocation& r)
{
    return l < r || l == r;
}

bool
IceProxy::TianShanIce::EdgeResource::operator>(const ::IceProxy::TianShanIce::EdgeResource::PhoAllocation& l, const ::IceProxy::TianShanIce::EdgeResource::PhoAllocation& r)
{
    return !(l < r) && !(l == r);
}

bool
IceProxy::TianShanIce::EdgeResource::operator>=(const ::IceProxy::TianShanIce::EdgeResource::PhoAllocation& l, const ::IceProxy::TianShanIce::EdgeResource::PhoAllocation& r)
{
    return !(l < r);
}

::Ice::Identity
IceDelegateM::TianShanIce::EdgeResource::PhoAllocation::getIdent(const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __TianShanIce__EdgeResource__PhoAllocation__getIdent_name, ::Ice::Normal, __context, __compress);
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
        ::Ice::Identity __ret;
        __ret.__read(__is);
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::TianShanIce::EdgeResource::AllocationPrx
IceDelegateM::TianShanIce::EdgeResource::PhoAllocation::getAllocation(const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __TianShanIce__EdgeResource__PhoAllocation__getAllocation_name, ::Ice::Normal, __context, __compress);
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
        ::TianShanIce::EdgeResource::AllocationPrx __ret;
        ::TianShanIce::EdgeResource::__read(__is, __ret);
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::std::string
IceDelegateM::TianShanIce::EdgeResource::PhoAllocation::getSessionGroup(const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __TianShanIce__EdgeResource__PhoAllocation__getSessionGroup_name, ::Ice::Normal, __context, __compress);
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
IceDelegateM::TianShanIce::EdgeResource::PhoAllocation::getSessKey(const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __TianShanIce__EdgeResource__PhoAllocation__getSessKey_name, ::Ice::Normal, __context, __compress);
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

::Ice::Identity
IceDelegateD::TianShanIce::EdgeResource::PhoAllocation::getIdent(const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __TianShanIce__EdgeResource__PhoAllocation__getIdent_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::Ice::Identity __ret;
        try
        {
            ::TianShanIce::EdgeResource::PhoAllocation* __servant = dynamic_cast< ::TianShanIce::EdgeResource::PhoAllocation*>(__direct.servant().get());
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
                __ret = __servant->getIdent(__current);
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

::TianShanIce::EdgeResource::AllocationPrx
IceDelegateD::TianShanIce::EdgeResource::PhoAllocation::getAllocation(const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __TianShanIce__EdgeResource__PhoAllocation__getAllocation_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::TianShanIce::EdgeResource::AllocationPrx __ret;
        try
        {
            ::TianShanIce::EdgeResource::PhoAllocation* __servant = dynamic_cast< ::TianShanIce::EdgeResource::PhoAllocation*>(__direct.servant().get());
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
                __ret = __servant->getAllocation(__current);
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
IceDelegateD::TianShanIce::EdgeResource::PhoAllocation::getSessionGroup(const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __TianShanIce__EdgeResource__PhoAllocation__getSessionGroup_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::std::string __ret;
        try
        {
            ::TianShanIce::EdgeResource::PhoAllocation* __servant = dynamic_cast< ::TianShanIce::EdgeResource::PhoAllocation*>(__direct.servant().get());
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
                __ret = __servant->getSessionGroup(__current);
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
IceDelegateD::TianShanIce::EdgeResource::PhoAllocation::getSessKey(const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __TianShanIce__EdgeResource__PhoAllocation__getSessKey_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::std::string __ret;
        try
        {
            ::TianShanIce::EdgeResource::PhoAllocation* __servant = dynamic_cast< ::TianShanIce::EdgeResource::PhoAllocation*>(__direct.servant().get());
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
                __ret = __servant->getSessKey(__current);
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

TianShanIce::EdgeResource::PhoAllocation::PhoAllocation(const ::Ice::Identity& __ice_ident, const ::std::string& __ice_sessKey, const ::std::string& __ice_sessionGroup, const ::TianShanIce::EdgeResource::AllocationPrx& __ice_alloc) :
    ident(__ice_ident),
    sessKey(__ice_sessKey),
    sessionGroup(__ice_sessionGroup),
    alloc(__ice_alloc)
{
}

::Ice::ObjectPtr
TianShanIce::EdgeResource::PhoAllocation::ice_clone() const
{
    throw ::Ice::CloneNotImplementedException(__FILE__, __LINE__);
    return 0; // to avoid a warning with some compilers
}

static const ::std::string __TianShanIce__EdgeResource__PhoAllocation_ids[2] =
{
    "::Ice::Object",
    "::TianShanIce::EdgeResource::PhoAllocation"
};

bool
TianShanIce::EdgeResource::PhoAllocation::ice_isA(const ::std::string& _s, const ::Ice::Current&) const
{
    return ::std::binary_search(__TianShanIce__EdgeResource__PhoAllocation_ids, __TianShanIce__EdgeResource__PhoAllocation_ids + 2, _s);
}

::std::vector< ::std::string>
TianShanIce::EdgeResource::PhoAllocation::ice_ids(const ::Ice::Current&) const
{
    return ::std::vector< ::std::string>(&__TianShanIce__EdgeResource__PhoAllocation_ids[0], &__TianShanIce__EdgeResource__PhoAllocation_ids[2]);
}

const ::std::string&
TianShanIce::EdgeResource::PhoAllocation::ice_id(const ::Ice::Current&) const
{
    return __TianShanIce__EdgeResource__PhoAllocation_ids[1];
}

const ::std::string&
TianShanIce::EdgeResource::PhoAllocation::ice_staticId()
{
    return __TianShanIce__EdgeResource__PhoAllocation_ids[1];
}

::IceInternal::DispatchStatus
TianShanIce::EdgeResource::PhoAllocation::___getIdent(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __os = __inS.os();
    ::Ice::Identity __ret = getIdent(__current);
    __ret.__write(__os);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
TianShanIce::EdgeResource::PhoAllocation::___getAllocation(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __os = __inS.os();
    ::TianShanIce::EdgeResource::AllocationPrx __ret = getAllocation(__current);
    ::TianShanIce::EdgeResource::__write(__os, __ret);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
TianShanIce::EdgeResource::PhoAllocation::___getSessionGroup(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string __ret = getSessionGroup(__current);
    __os->write(__ret);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
TianShanIce::EdgeResource::PhoAllocation::___getSessKey(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string __ret = getSessKey(__current);
    __os->write(__ret);
    return ::IceInternal::DispatchOK;
}

static ::std::string __TianShanIce__EdgeResource__PhoAllocation_all[] =
{
    "getAllocation",
    "getIdent",
    "getSessKey",
    "getSessionGroup",
    "ice_id",
    "ice_ids",
    "ice_isA",
    "ice_ping"
};

::IceInternal::DispatchStatus
TianShanIce::EdgeResource::PhoAllocation::__dispatch(::IceInternal::Incoming& in, const ::Ice::Current& current)
{
    ::std::pair< ::std::string*, ::std::string*> r = ::std::equal_range(__TianShanIce__EdgeResource__PhoAllocation_all, __TianShanIce__EdgeResource__PhoAllocation_all + 8, current.operation);
    if(r.first == r.second)
    {
        return ::IceInternal::DispatchOperationNotExist;
    }

    switch(r.first - __TianShanIce__EdgeResource__PhoAllocation_all)
    {
        case 0:
        {
            return ___getAllocation(in, current);
        }
        case 1:
        {
            return ___getIdent(in, current);
        }
        case 2:
        {
            return ___getSessKey(in, current);
        }
        case 3:
        {
            return ___getSessionGroup(in, current);
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
    }

    assert(false);
    return ::IceInternal::DispatchOperationNotExist;
}

void
TianShanIce::EdgeResource::PhoAllocation::__write(::IceInternal::BasicStream* __os) const
{
    __os->writeTypeId(ice_staticId());
    __os->startWriteSlice();
    ident.__write(__os);
    __os->write(sessKey);
    __os->write(sessionGroup);
    ::TianShanIce::EdgeResource::__write(__os, alloc);
    __os->endWriteSlice();
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    Object::__write(__os);
#else
    ::Ice::Object::__write(__os);
#endif
}

void
TianShanIce::EdgeResource::PhoAllocation::__read(::IceInternal::BasicStream* __is, bool __rid)
{
    if(__rid)
    {
        ::std::string myId;
        __is->readTypeId(myId);
    }
    __is->startReadSlice();
    ident.__read(__is);
    __is->read(sessKey);
    __is->read(sessionGroup);
    ::TianShanIce::EdgeResource::__read(__is, alloc);
    __is->endReadSlice();
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    Object::__read(__is, true);
#else
    ::Ice::Object::__read(__is, true);
#endif
}

void
TianShanIce::EdgeResource::PhoAllocation::__write(const ::Ice::OutputStreamPtr&) const
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type TianShanIce::EdgeResource::PhoAllocation was not generated with stream support";
    throw ex;
}

void
TianShanIce::EdgeResource::PhoAllocation::__read(const ::Ice::InputStreamPtr&, bool)
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type TianShanIce::EdgeResource::PhoAllocation was not generated with stream support";
    throw ex;
}

void 
TianShanIce::EdgeResource::__patch__PhoAllocationPtr(void* __addr, ::Ice::ObjectPtr& v)
{
    ::TianShanIce::EdgeResource::PhoAllocationPtr* p = static_cast< ::TianShanIce::EdgeResource::PhoAllocationPtr*>(__addr);
    assert(p);
    *p = ::TianShanIce::EdgeResource::PhoAllocationPtr::dynamicCast(v);
    if(v && !*p)
    {
        ::Ice::UnexpectedObjectException e(__FILE__, __LINE__);
        e.type = v->ice_id();
        e.expectedType = ::TianShanIce::EdgeResource::PhoAllocation::ice_staticId();
        throw e;
    }
}

bool
TianShanIce::EdgeResource::operator==(const ::TianShanIce::EdgeResource::PhoAllocation& l, const ::TianShanIce::EdgeResource::PhoAllocation& r)
{
    return static_cast<const ::Ice::Object&>(l) == static_cast<const ::Ice::Object&>(r);
}

bool
TianShanIce::EdgeResource::operator!=(const ::TianShanIce::EdgeResource::PhoAllocation& l, const ::TianShanIce::EdgeResource::PhoAllocation& r)
{
    return static_cast<const ::Ice::Object&>(l) != static_cast<const ::Ice::Object&>(r);
}

bool
TianShanIce::EdgeResource::operator<(const ::TianShanIce::EdgeResource::PhoAllocation& l, const ::TianShanIce::EdgeResource::PhoAllocation& r)
{
    return static_cast<const ::Ice::Object&>(l) < static_cast<const ::Ice::Object&>(r);
}

bool
TianShanIce::EdgeResource::operator<=(const ::TianShanIce::EdgeResource::PhoAllocation& l, const ::TianShanIce::EdgeResource::PhoAllocation& r)
{
    return l < r || l == r;
}

bool
TianShanIce::EdgeResource::operator>(const ::TianShanIce::EdgeResource::PhoAllocation& l, const ::TianShanIce::EdgeResource::PhoAllocation& r)
{
    return !(l < r) && !(l == r);
}

bool
TianShanIce::EdgeResource::operator>=(const ::TianShanIce::EdgeResource::PhoAllocation& l, const ::TianShanIce::EdgeResource::PhoAllocation& r)
{
    return !(l < r);
}
