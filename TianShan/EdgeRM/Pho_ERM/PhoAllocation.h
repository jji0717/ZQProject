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

#ifndef ____PhoAllocation_h__
#define ____PhoAllocation_h__

#include <Ice/LocalObjectF.h>
#include <Ice/ProxyF.h>
#include <Ice/ObjectF.h>
#include <Ice/Exception.h>
#include <Ice/LocalObject.h>
#include <Ice/Proxy.h>
#include <Ice/Object.h>
#include <Ice/Outgoing.h>
#include <Ice/OutgoingAsync.h>
#include <Ice/Incoming.h>
#include <Ice/IncomingAsync.h>
#include <Ice/Direct.h>
#include <Ice/UserExceptionFactory.h>
#include <Ice/FactoryTable.h>
#include <Ice/StreamF.h>
#include <TsEdgeResource.h>
#include <Ice/UndefSysMacros.h>

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

namespace IceProxy
{

namespace TianShanIce
{

namespace EdgeResource
{

class PhoAllocation;
bool operator==(const PhoAllocation&, const PhoAllocation&);
bool operator!=(const PhoAllocation&, const PhoAllocation&);
bool operator<(const PhoAllocation&, const PhoAllocation&);
bool operator<=(const PhoAllocation&, const PhoAllocation&);
bool operator>(const PhoAllocation&, const PhoAllocation&);
bool operator>=(const PhoAllocation&, const PhoAllocation&);

}

}

}

namespace TianShanIce
{

namespace EdgeResource
{

class PhoAllocation;
bool operator==(const PhoAllocation&, const PhoAllocation&);
bool operator!=(const PhoAllocation&, const PhoAllocation&);
bool operator<(const PhoAllocation&, const PhoAllocation&);
bool operator<=(const PhoAllocation&, const PhoAllocation&);
bool operator>(const PhoAllocation&, const PhoAllocation&);
bool operator>=(const PhoAllocation&, const PhoAllocation&);

}

}

namespace IceInternal
{

void incRef(::TianShanIce::EdgeResource::PhoAllocation*);
void decRef(::TianShanIce::EdgeResource::PhoAllocation*);

void incRef(::IceProxy::TianShanIce::EdgeResource::PhoAllocation*);
void decRef(::IceProxy::TianShanIce::EdgeResource::PhoAllocation*);

}

namespace TianShanIce
{

namespace EdgeResource
{

typedef ::IceInternal::Handle< ::TianShanIce::EdgeResource::PhoAllocation> PhoAllocationPtr;
typedef ::IceInternal::ProxyHandle< ::IceProxy::TianShanIce::EdgeResource::PhoAllocation> PhoAllocationPrx;

void __write(::IceInternal::BasicStream*, const PhoAllocationPrx&);
void __read(::IceInternal::BasicStream*, PhoAllocationPrx&);
void __write(::IceInternal::BasicStream*, const PhoAllocationPtr&);
void __patch__PhoAllocationPtr(void*, ::Ice::ObjectPtr&);

void __addObject(const PhoAllocationPtr&, ::IceInternal::GCCountMap&);
bool __usesClasses(const PhoAllocationPtr&);
void __decRefUnsafe(const PhoAllocationPtr&);
void __clearHandleUnsafe(PhoAllocationPtr&);

}

}

namespace TianShanIce
{

namespace EdgeResource
{

}

}

namespace IceAsync
{

}

namespace IceProxy
{

namespace TianShanIce
{

namespace EdgeResource
{

class PhoAllocation : virtual public ::IceProxy::Ice::Object
{
public:

    ::Ice::Identity getIdent()
    {
        return getIdent(0);
    }
    ::Ice::Identity getIdent(const ::Ice::Context& __ctx)
    {
        return getIdent(&__ctx);
    }
    
private:

    ::Ice::Identity getIdent(const ::Ice::Context*);
    
public:

    ::TianShanIce::EdgeResource::AllocationPrx getAllocation()
    {
        return getAllocation(0);
    }
    ::TianShanIce::EdgeResource::AllocationPrx getAllocation(const ::Ice::Context& __ctx)
    {
        return getAllocation(&__ctx);
    }
    
private:

    ::TianShanIce::EdgeResource::AllocationPrx getAllocation(const ::Ice::Context*);
    
public:

    ::std::string getSessionGroup()
    {
        return getSessionGroup(0);
    }
    ::std::string getSessionGroup(const ::Ice::Context& __ctx)
    {
        return getSessionGroup(&__ctx);
    }
    
private:

    ::std::string getSessionGroup(const ::Ice::Context*);
    
public:

    ::std::string getSessKey()
    {
        return getSessKey(0);
    }
    ::std::string getSessKey(const ::Ice::Context& __ctx)
    {
        return getSessKey(&__ctx);
    }
    
private:

    ::std::string getSessKey(const ::Ice::Context*);
    
public:
    
    static const ::std::string& ice_staticId();

private: 

    virtual ::IceInternal::Handle< ::IceDelegateM::Ice::Object> __createDelegateM();
    virtual ::IceInternal::Handle< ::IceDelegateD::Ice::Object> __createDelegateD();
};

}

}

}

namespace IceDelegate
{

namespace TianShanIce
{

namespace EdgeResource
{

class PhoAllocation : virtual public ::IceDelegate::Ice::Object
{
public:

    virtual ::Ice::Identity getIdent(const ::Ice::Context*) = 0;

    virtual ::TianShanIce::EdgeResource::AllocationPrx getAllocation(const ::Ice::Context*) = 0;

    virtual ::std::string getSessionGroup(const ::Ice::Context*) = 0;

    virtual ::std::string getSessKey(const ::Ice::Context*) = 0;
};

}

}

}

namespace IceDelegateM
{

namespace TianShanIce
{

namespace EdgeResource
{

class PhoAllocation : virtual public ::IceDelegate::TianShanIce::EdgeResource::PhoAllocation,
                      virtual public ::IceDelegateM::Ice::Object
{
public:

    virtual ::Ice::Identity getIdent(const ::Ice::Context*);

    virtual ::TianShanIce::EdgeResource::AllocationPrx getAllocation(const ::Ice::Context*);

    virtual ::std::string getSessionGroup(const ::Ice::Context*);

    virtual ::std::string getSessKey(const ::Ice::Context*);
};

}

}

}

namespace IceDelegateD
{

namespace TianShanIce
{

namespace EdgeResource
{

class PhoAllocation : virtual public ::IceDelegate::TianShanIce::EdgeResource::PhoAllocation,
                      virtual public ::IceDelegateD::Ice::Object
{
public:

    virtual ::Ice::Identity getIdent(const ::Ice::Context*);

    virtual ::TianShanIce::EdgeResource::AllocationPrx getAllocation(const ::Ice::Context*);

    virtual ::std::string getSessionGroup(const ::Ice::Context*);

    virtual ::std::string getSessKey(const ::Ice::Context*);
};

}

}

}

namespace TianShanIce
{

namespace EdgeResource
{

class PhoAllocation : virtual public ::Ice::Object
{
public:

    typedef PhoAllocationPrx ProxyType;
    typedef PhoAllocationPtr PointerType;
    
    PhoAllocation() {}
    PhoAllocation(const ::Ice::Identity&, const ::std::string&, const ::std::string&, const ::TianShanIce::EdgeResource::AllocationPrx&);
    virtual ::Ice::ObjectPtr ice_clone() const;

    virtual bool ice_isA(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) const;
    virtual ::std::vector< ::std::string> ice_ids(const ::Ice::Current& = ::Ice::Current()) const;
    virtual const ::std::string& ice_id(const ::Ice::Current& = ::Ice::Current()) const;
    static const ::std::string& ice_staticId();


    virtual ::Ice::Identity getIdent(const ::Ice::Current& = ::Ice::Current()) const = 0;
    ::IceInternal::DispatchStatus ___getIdent(::IceInternal::Incoming&, const ::Ice::Current&) const;

    virtual ::TianShanIce::EdgeResource::AllocationPrx getAllocation(const ::Ice::Current& = ::Ice::Current()) const = 0;
    ::IceInternal::DispatchStatus ___getAllocation(::IceInternal::Incoming&, const ::Ice::Current&) const;

    virtual ::std::string getSessionGroup(const ::Ice::Current& = ::Ice::Current()) const = 0;
    ::IceInternal::DispatchStatus ___getSessionGroup(::IceInternal::Incoming&, const ::Ice::Current&) const;

    virtual ::std::string getSessKey(const ::Ice::Current& = ::Ice::Current()) const = 0;
    ::IceInternal::DispatchStatus ___getSessKey(::IceInternal::Incoming&, const ::Ice::Current&) const;

    virtual ::IceInternal::DispatchStatus __dispatch(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*, bool);
    virtual void __write(const ::Ice::OutputStreamPtr&) const;
    virtual void __read(const ::Ice::InputStreamPtr&, bool);

    ::Ice::Identity ident;

    ::std::string sessKey;

    ::std::string sessionGroup;

    ::TianShanIce::EdgeResource::AllocationPrx alloc;
};

void __patch__PhoAllocationPtr(void*, ::Ice::ObjectPtr&);

}

}

#endif
