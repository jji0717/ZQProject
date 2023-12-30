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

#include <Test.h>
#include <Ice/LocalException.h>
#include <Ice/ObjectFactory.h>
#include <Ice/BasicStream.h>
#include <Ice/Object.h>

#ifndef ICE_IGNORE_VERSION
#   if ICE_INT_VERSION / 100 != 300
#       error Ice version mismatch!
#   endif
#   if ICE_INT_VERSION % 100 < 1
#       error Ice patch level mismatch!
#   endif
#endif

void
IceInternal::incRef(::Test::TestIf* p)
{
    p->__incRef();
}

void
IceInternal::decRef(::Test::TestIf* p)
{
    p->__decRef();
}

void
IceInternal::incRef(::IceProxy::Test::TestIf* p)
{
    p->__incRef();
}

void
IceInternal::decRef(::IceProxy::Test::TestIf* p)
{
    p->__decRef();
}

void
Test::__write(::IceInternal::BasicStream* __os, const ::Test::TestIfPrx& v)
{
    __os->write(::Ice::ObjectPrx(v));
}

void
Test::__read(::IceInternal::BasicStream* __is, ::Test::TestIfPrx& v)
{
    ::Ice::ObjectPrx proxy;
    __is->read(proxy);
    if(!proxy)
    {
	v = 0;
    }
    else
    {
	v = new ::IceProxy::Test::TestIf;
	v->__copyFrom(proxy);
    }
}

void
Test::__write(::IceInternal::BasicStream* __os, const ::Test::TestIfPtr& v)
{
    __os->write(::Ice::ObjectPtr(v));
}

void
IceProxy::Test::TestIf::TestFn(const ::std::string& s)
{
    TestFn(s, __defaultContext());
}

void
IceProxy::Test::TestIf::TestFn(const ::std::string& s, const ::Ice::Context& __ctx)
{
    int __cnt = 0;
    while(true)
    {
	try
	{
	    ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase = __getDelegate();
	    ::IceDelegate::Test::TestIf* __del = dynamic_cast< ::IceDelegate::Test::TestIf*>(__delBase.get());
	    __del->TestFn(s, __ctx);
	    return;
	}
	catch(const ::IceInternal::NonRepeatable& __ex)
	{
	    __rethrowException(*__ex.get());
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    __handleException(__ex, __cnt);
	}
    }
}

const ::std::string&
IceProxy::Test::TestIf::ice_staticId()
{
    return ::Test::TestIf::ice_staticId();
}

::IceInternal::Handle< ::IceDelegateM::Ice::Object>
IceProxy::Test::TestIf::__createDelegateM()
{
    return ::IceInternal::Handle< ::IceDelegateM::Ice::Object>(new ::IceDelegateM::Test::TestIf);
}

::IceInternal::Handle< ::IceDelegateD::Ice::Object>
IceProxy::Test::TestIf::__createDelegateD()
{
    return ::IceInternal::Handle< ::IceDelegateD::Ice::Object>(new ::IceDelegateD::Test::TestIf);
}

bool
IceProxy::Test::operator==(const ::IceProxy::Test::TestIf& l, const ::IceProxy::Test::TestIf& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) == static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::Test::operator!=(const ::IceProxy::Test::TestIf& l, const ::IceProxy::Test::TestIf& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) != static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::Test::operator<(const ::IceProxy::Test::TestIf& l, const ::IceProxy::Test::TestIf& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) < static_cast<const ::IceProxy::Ice::Object&>(r);
}

static const ::std::string __Test__TestIf__TestFn_name = "TestFn";

void
IceDelegateM::Test::TestIf::TestFn(const ::std::string& s, const ::Ice::Context& __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __Test__TestIf__TestFn_name, ::Ice::Normal, __context, __compress);
    try
    {
	::IceInternal::BasicStream* __os = __og.os();
	__os->write(s);
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
	throw ::IceInternal::NonRepeatable(__ex);
    }
}

void
IceDelegateD::Test::TestIf::TestFn(const ::std::string& s, const ::Ice::Context& __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, "TestFn", ::Ice::Normal, __context);
    while(true)
    {
	::IceInternal::Direct __direct(__current);
	::Test::TestIf* __servant = dynamic_cast< ::Test::TestIf*>(__direct.servant().get());
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
	    __servant->TestFn(s, __current);
	    return;
	}
	catch(const ::Ice::LocalException& __ex)
	{
	    throw ::IceInternal::NonRepeatable(__ex);
	}
    }
}

::Ice::ObjectPtr
Test::TestIf::ice_clone() const
{
    throw ::Ice::CloneNotImplementedException(__FILE__, __LINE__);
}

static const ::std::string __Test__TestIf_ids[2] =
{
    "::Ice::Object",
    "::Test::TestIf"
};

bool
Test::TestIf::ice_isA(const ::std::string& _s, const ::Ice::Current&) const
{
    return ::std::binary_search(__Test__TestIf_ids, __Test__TestIf_ids + 2, _s);
}

::std::vector< ::std::string>
Test::TestIf::ice_ids(const ::Ice::Current&) const
{
    return ::std::vector< ::std::string>(&__Test__TestIf_ids[0], &__Test__TestIf_ids[2]);
}

const ::std::string&
Test::TestIf::ice_id(const ::Ice::Current&) const
{
    return __Test__TestIf_ids[1];
}

const ::std::string&
Test::TestIf::ice_staticId()
{
    return __Test__TestIf_ids[1];
}

::IceInternal::DispatchStatus
Test::TestIf::___TestFn(::IceInternal::Incoming& __inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::std::string s;
    __is->read(s);
    TestFn(s, __current);
    return ::IceInternal::DispatchOK;
}

static ::std::string __Test__TestIf_all[] =
{
    "TestFn",
    "ice_id",
    "ice_ids",
    "ice_isA",
    "ice_ping"
};

::IceInternal::DispatchStatus
Test::TestIf::__dispatch(::IceInternal::Incoming& in, const ::Ice::Current& current)
{
    ::std::pair< ::std::string*, ::std::string*> r = ::std::equal_range(__Test__TestIf_all, __Test__TestIf_all + 5, current.operation);
    if(r.first == r.second)
    {
	return ::IceInternal::DispatchOperationNotExist;
    }

    switch(r.first - __Test__TestIf_all)
    {
	case 0:
	{
	    return ___TestFn(in, current);
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
Test::TestIf::__write(::IceInternal::BasicStream* __os) const
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
Test::TestIf::__read(::IceInternal::BasicStream* __is, bool __rid)
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
Test::TestIf::__write(const ::Ice::OutputStreamPtr&) const
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type Test::TestIf was not generated with stream support";
    throw ex;
}

void
Test::TestIf::__read(const ::Ice::InputStreamPtr&, bool)
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type Test::TestIf was not generated with stream support";
    throw ex;
}

void 
Test::__patch__TestIfPtr(void* __addr, ::Ice::ObjectPtr& v)
{
    ::Test::TestIfPtr* p = static_cast< ::Test::TestIfPtr*>(__addr);
    assert(p);
    *p = ::Test::TestIfPtr::dynamicCast(v);
    if(v && !*p)
    {
	::Ice::NoObjectFactoryException e(__FILE__, __LINE__);
	e.type = ::Test::TestIf::ice_staticId();
	throw e;
    }
}

bool
Test::operator==(const ::Test::TestIf& l, const ::Test::TestIf& r)
{
    return static_cast<const ::Ice::Object&>(l) == static_cast<const ::Ice::Object&>(r);
}

bool
Test::operator!=(const ::Test::TestIf& l, const ::Test::TestIf& r)
{
    return static_cast<const ::Ice::Object&>(l) != static_cast<const ::Ice::Object&>(r);
}

bool
Test::operator<(const ::Test::TestIf& l, const ::Test::TestIf& r)
{
    return static_cast<const ::Ice::Object&>(l) < static_cast<const ::Ice::Object&>(r);
}
