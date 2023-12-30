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

#ifndef _____TimeoutCall_h__
#define _____TimeoutCall_h__

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
#include <Ice/Direct.h>
#include <Ice/StreamF.h>
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

namespace test
{

class timeout;
bool operator==(const timeout&, const timeout&);
bool operator!=(const timeout&, const timeout&);
bool operator<(const timeout&, const timeout&);
bool operator<=(const timeout&, const timeout&);
bool operator>(const timeout&, const timeout&);
bool operator>=(const timeout&, const timeout&);

}

}

namespace test
{

class timeout;
bool operator==(const timeout&, const timeout&);
bool operator!=(const timeout&, const timeout&);
bool operator<(const timeout&, const timeout&);
bool operator<=(const timeout&, const timeout&);
bool operator>(const timeout&, const timeout&);
bool operator>=(const timeout&, const timeout&);

}

namespace IceInternal
{

void incRef(::test::timeout*);
void decRef(::test::timeout*);

void incRef(::IceProxy::test::timeout*);
void decRef(::IceProxy::test::timeout*);

}

namespace test
{

typedef ::IceInternal::Handle< ::test::timeout> timeoutPtr;
typedef ::IceInternal::ProxyHandle< ::IceProxy::test::timeout> timeoutPrx;

void __write(::IceInternal::BasicStream*, const timeoutPrx&);
void __read(::IceInternal::BasicStream*, timeoutPrx&);
void __write(::IceInternal::BasicStream*, const timeoutPtr&);
void __patch__timeoutPtr(void*, ::Ice::ObjectPtr&);

void __addObject(const timeoutPtr&, ::IceInternal::GCCountMap&);
bool __usesClasses(const timeoutPtr&);
void __decRefUnsafe(const timeoutPtr&);
void __clearHandleUnsafe(timeoutPtr&);

}

namespace test
{

}

namespace test
{

class AMI_timeout_call : public ::IceInternal::OutgoingAsync
{
public:

    virtual void ice_response(::Ice::Int) = 0;
    virtual void ice_exception(const ::Ice::Exception&) = 0;

    void __invoke(const ::test::timeoutPrx&, ::Ice::Int, ::Ice::Int, const ::Ice::Context*);

protected:

    virtual void __response(bool);
};

typedef ::IceUtil::Handle< ::test::AMI_timeout_call> AMI_timeout_callPtr;

}

namespace IceProxy
{

namespace test
{

class timeout : virtual public ::IceProxy::Ice::Object
{
public:

    ::Ice::Int call(::Ice::Int clientId, ::Ice::Int t)
    {
        return call(clientId, t, 0);
    }
    ::Ice::Int call(::Ice::Int clientId, ::Ice::Int t, const ::Ice::Context& __ctx)
    {
        return call(clientId, t, &__ctx);
    }
    
private:

    ::Ice::Int call(::Ice::Int, ::Ice::Int, const ::Ice::Context*);
    
public:
    void call_async(const ::test::AMI_timeout_callPtr&, ::Ice::Int, ::Ice::Int);
    void call_async(const ::test::AMI_timeout_callPtr&, ::Ice::Int, ::Ice::Int, const ::Ice::Context&);
    
    static const ::std::string& ice_staticId();

private: 

    virtual ::IceInternal::Handle< ::IceDelegateM::Ice::Object> __createDelegateM();
    virtual ::IceInternal::Handle< ::IceDelegateD::Ice::Object> __createDelegateD();
};

}

}

namespace IceDelegate
{

namespace test
{

class timeout : virtual public ::IceDelegate::Ice::Object
{
public:

    virtual ::Ice::Int call(::Ice::Int, ::Ice::Int, const ::Ice::Context*) = 0;
};

}

}

namespace IceDelegateM
{

namespace test
{

class timeout : virtual public ::IceDelegate::test::timeout,
                virtual public ::IceDelegateM::Ice::Object
{
public:

    virtual ::Ice::Int call(::Ice::Int, ::Ice::Int, const ::Ice::Context*);
};

}

}

namespace IceDelegateD
{

namespace test
{

class timeout : virtual public ::IceDelegate::test::timeout,
                virtual public ::IceDelegateD::Ice::Object
{
public:

    virtual ::Ice::Int call(::Ice::Int, ::Ice::Int, const ::Ice::Context*);
};

}

}

namespace test
{

class timeout : virtual public ::Ice::Object
{
public:

    typedef timeoutPrx ProxyType;
    typedef timeoutPtr PointerType;
    
    timeout() {}
    virtual ::Ice::ObjectPtr ice_clone() const;

    virtual bool ice_isA(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) const;
    virtual ::std::vector< ::std::string> ice_ids(const ::Ice::Current& = ::Ice::Current()) const;
    virtual const ::std::string& ice_id(const ::Ice::Current& = ::Ice::Current()) const;
    static const ::std::string& ice_staticId();

    virtual ::Ice::Int call(::Ice::Int, ::Ice::Int, const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___call(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual ::IceInternal::DispatchStatus __dispatch(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*, bool);
    virtual void __write(const ::Ice::OutputStreamPtr&) const;
    virtual void __read(const ::Ice::InputStreamPtr&, bool);
};

void __patch__timeoutPtr(void*, ::Ice::ObjectPtr&);

}

#endif
