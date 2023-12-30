// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// Ice version 3.0.1
// Generated from file `Test.ice'

#ifndef __Test_h__
#define __Test_h__

#include <Ice/LocalObjectF.h>
#include <Ice/ProxyF.h>
#include <Ice/ObjectF.h>
#include <Ice/Exception.h>
#include <Ice/LocalObject.h>
#include <Ice/Proxy.h>
#include <Ice/Object.h>
#include <Ice/Outgoing.h>
#include <Ice/Incoming.h>
#include <Ice/Direct.h>
#include <Ice/StreamF.h>
#include <Ice/UndefSysMacros.h>

#ifndef ICE_IGNORE_VERSION
#   if ICE_INT_VERSION / 100 != 300
#       error Ice version mismatch!
#   endif
#   if ICE_INT_VERSION % 100 < 1
#       error Ice patch level mismatch!
#   endif
#endif

namespace IceProxy
{

namespace Test
{

class TestIf;
bool operator==(const TestIf&, const TestIf&);
bool operator!=(const TestIf&, const TestIf&);
bool operator<(const TestIf&, const TestIf&);

}

}

namespace Test
{

class TestIf;
bool operator==(const TestIf&, const TestIf&);
bool operator!=(const TestIf&, const TestIf&);
bool operator<(const TestIf&, const TestIf&);

}

namespace IceInternal
{

void incRef(::Test::TestIf*);
void decRef(::Test::TestIf*);

void incRef(::IceProxy::Test::TestIf*);
void decRef(::IceProxy::Test::TestIf*);

}

namespace Test
{

typedef ::IceInternal::Handle< ::Test::TestIf> TestIfPtr;
typedef ::IceInternal::ProxyHandle< ::IceProxy::Test::TestIf> TestIfPrx;

void __write(::IceInternal::BasicStream*, const TestIfPrx&);
void __read(::IceInternal::BasicStream*, TestIfPrx&);
void __write(::IceInternal::BasicStream*, const TestIfPtr&);
void __patch__TestIfPtr(void*, ::Ice::ObjectPtr&);

}

namespace Test
{

}

namespace IceProxy
{

namespace Test
{

class TestIf : virtual public ::IceProxy::Ice::Object
{
public:

    void TestFn(const ::std::string&);
    void TestFn(const ::std::string&, const ::Ice::Context&);
    
    static const ::std::string& ice_staticId();

private: 

    virtual ::IceInternal::Handle< ::IceDelegateM::Ice::Object> __createDelegateM();
    virtual ::IceInternal::Handle< ::IceDelegateD::Ice::Object> __createDelegateD();
};

}

}

namespace IceDelegate
{

namespace Test
{

class TestIf : virtual public ::IceDelegate::Ice::Object
{
public:

    virtual void TestFn(const ::std::string&, const ::Ice::Context&) = 0;
};

}

}

namespace IceDelegateM
{

namespace Test
{

class TestIf : virtual public ::IceDelegate::Test::TestIf,
	       virtual public ::IceDelegateM::Ice::Object
{
public:

    virtual void TestFn(const ::std::string&, const ::Ice::Context&);
};

}

}

namespace IceDelegateD
{

namespace Test
{

class TestIf : virtual public ::IceDelegate::Test::TestIf,
	       virtual public ::IceDelegateD::Ice::Object
{
public:

    virtual void TestFn(const ::std::string&, const ::Ice::Context&);
};

}

}

namespace Test
{

class TestIf : virtual public ::Ice::Object
{
public:

    virtual ::Ice::ObjectPtr ice_clone() const;

    virtual bool ice_isA(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) const;
    virtual ::std::vector< ::std::string> ice_ids(const ::Ice::Current& = ::Ice::Current()) const;
    virtual const ::std::string& ice_id(const ::Ice::Current& = ::Ice::Current()) const;
    static const ::std::string& ice_staticId();

    virtual void TestFn(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___TestFn(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual ::IceInternal::DispatchStatus __dispatch(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*, bool);
    virtual void __write(const ::Ice::OutputStreamPtr&) const;
    virtual void __read(const ::Ice::InputStreamPtr&, bool);
};

void __patch__TestIfPtr(void*, ::Ice::ObjectPtr&);

}

#endif
