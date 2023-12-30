// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// Ice version 3.2.1
// Generated from file `DODContentStore.ice'

#ifndef __DODContentStore_h__
#define __DODContentStore_h__

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
#include <TsStorage.h>
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

namespace Storage
{

namespace DataOnDemand
{

class DODContent;
bool operator==(const DODContent&, const DODContent&);
bool operator!=(const DODContent&, const DODContent&);
bool operator<(const DODContent&, const DODContent&);
bool operator<=(const DODContent&, const DODContent&);
bool operator>(const DODContent&, const DODContent&);
bool operator>=(const DODContent&, const DODContent&);

}

}

}

}

namespace TianShanIce
{

namespace Storage
{

namespace DataOnDemand
{

class DODContent;
bool operator==(const DODContent&, const DODContent&);
bool operator!=(const DODContent&, const DODContent&);
bool operator<(const DODContent&, const DODContent&);
bool operator<=(const DODContent&, const DODContent&);
bool operator>(const DODContent&, const DODContent&);
bool operator>=(const DODContent&, const DODContent&);

}

}

}

namespace IceInternal
{

void incRef(::TianShanIce::Storage::DataOnDemand::DODContent*);
void decRef(::TianShanIce::Storage::DataOnDemand::DODContent*);

void incRef(::IceProxy::TianShanIce::Storage::DataOnDemand::DODContent*);
void decRef(::IceProxy::TianShanIce::Storage::DataOnDemand::DODContent*);

}

namespace TianShanIce
{

namespace Storage
{

namespace DataOnDemand
{

typedef ::IceInternal::Handle< ::TianShanIce::Storage::DataOnDemand::DODContent> DODContentPtr;
typedef ::IceInternal::ProxyHandle< ::IceProxy::TianShanIce::Storage::DataOnDemand::DODContent> DODContentPrx;

void __write(::IceInternal::BasicStream*, const DODContentPrx&);
void __read(::IceInternal::BasicStream*, DODContentPrx&);
void __write(::IceInternal::BasicStream*, const DODContentPtr&);
void __patch__DODContentPtr(void*, ::Ice::ObjectPtr&);

void __addObject(const DODContentPtr&, ::IceInternal::GCCountMap&);
bool __usesClasses(const DODContentPtr&);
void __decRefUnsafe(const DODContentPtr&);
void __clearHandleUnsafe(DODContentPtr&);

}

}

}

namespace TianShanIce
{

namespace Storage
{

namespace DataOnDemand
{

struct DataWrappingParam
{
    ::Ice::Int esPID;
    ::Ice::Int streamType;
    ::Ice::Int subStreamCount;
    ::Ice::Int dataType;
    ::Ice::Int withObjIndex;
    ::Ice::Int objTag;
    ::Ice::Int encryptType;
    ::Ice::Int versionNumber;

    bool operator==(const DataWrappingParam&) const;
    bool operator!=(const DataWrappingParam&) const;
    bool operator<(const DataWrappingParam&) const;
    bool operator<=(const DataWrappingParam& __rhs) const
    {
        return operator<(__rhs) || operator==(__rhs);
    }
    bool operator>(const DataWrappingParam& __rhs) const
    {
        return !operator<(__rhs) && !operator==(__rhs);
    }
    bool operator>=(const DataWrappingParam& __rhs) const
    {
        return !operator<(__rhs);
    }

    void __write(::IceInternal::BasicStream*) const;
    void __read(::IceInternal::BasicStream*);
};

}

}

}

namespace IceAsync
{

}

namespace IceProxy
{

namespace TianShanIce
{

namespace Storage
{

namespace DataOnDemand
{

class DODContent : virtual public ::IceProxy::TianShanIce::Storage::Content
{
public:

    void setDataWrappingParam(const ::TianShanIce::Storage::DataOnDemand::DataWrappingParam& param)
    {
        setDataWrappingParam(param, 0);
    }
    void setDataWrappingParam(const ::TianShanIce::Storage::DataOnDemand::DataWrappingParam& param, const ::Ice::Context& __ctx)
    {
        setDataWrappingParam(param, &__ctx);
    }
    
private:

    void setDataWrappingParam(const ::TianShanIce::Storage::DataOnDemand::DataWrappingParam&, const ::Ice::Context*);
    
public:
    
    static const ::std::string& ice_staticId();

private: 

    virtual ::IceInternal::Handle< ::IceDelegateM::Ice::Object> __createDelegateM();
    virtual ::IceInternal::Handle< ::IceDelegateD::Ice::Object> __createDelegateD();
};

}

}

}

}

namespace IceDelegate
{

namespace TianShanIce
{

namespace Storage
{

namespace DataOnDemand
{

class DODContent : virtual public ::IceDelegate::TianShanIce::Storage::Content
{
public:

    virtual void setDataWrappingParam(const ::TianShanIce::Storage::DataOnDemand::DataWrappingParam&, const ::Ice::Context*) = 0;
};

}

}

}

}

namespace IceDelegateM
{

namespace TianShanIce
{

namespace Storage
{

namespace DataOnDemand
{

class DODContent : virtual public ::IceDelegate::TianShanIce::Storage::DataOnDemand::DODContent,
                   virtual public ::IceDelegateM::TianShanIce::Storage::Content
{
public:

    virtual void setDataWrappingParam(const ::TianShanIce::Storage::DataOnDemand::DataWrappingParam&, const ::Ice::Context*);
};

}

}

}

}

namespace IceDelegateD
{

namespace TianShanIce
{

namespace Storage
{

namespace DataOnDemand
{

class DODContent : virtual public ::IceDelegate::TianShanIce::Storage::DataOnDemand::DODContent,
                   virtual public ::IceDelegateD::TianShanIce::Storage::Content
{
public:

    virtual void setDataWrappingParam(const ::TianShanIce::Storage::DataOnDemand::DataWrappingParam&, const ::Ice::Context*);
};

}

}

}

}

namespace TianShanIce
{

namespace Storage
{

namespace DataOnDemand
{

class DODContent : virtual public ::TianShanIce::Storage::Content
{
public:

    typedef DODContentPrx ProxyType;
    typedef DODContentPtr PointerType;
    
    DODContent() {}
    DODContent(::Ice::Long, ::Ice::Long, ::Ice::Long, ::TianShanIce::Storage::ContentState, const ::std::string&, const ::TianShanIce::Storage::TrickSpeedCollection&, const ::TianShanIce::Properties&, const ::std::string&);
    virtual ::Ice::ObjectPtr ice_clone() const;

    virtual bool ice_isA(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) const;
    virtual ::std::vector< ::std::string> ice_ids(const ::Ice::Current& = ::Ice::Current()) const;
    virtual const ::std::string& ice_id(const ::Ice::Current& = ::Ice::Current()) const;
    static const ::std::string& ice_staticId();


    virtual void setDataWrappingParam(const ::TianShanIce::Storage::DataOnDemand::DataWrappingParam&, const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___setDataWrappingParam(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual ::IceInternal::DispatchStatus __dispatch(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual ::Ice::Int ice_operationAttributes(const ::std::string&) const;

    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*, bool);
    virtual void __write(const ::Ice::OutputStreamPtr&) const;
    virtual void __read(const ::Ice::InputStreamPtr&, bool);

    ::std::string name;
};

void __patch__DODContentPtr(void*, ::Ice::ObjectPtr&);

}

}

}

#endif
