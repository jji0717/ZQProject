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

#ifndef ____LogPosition_h__
#define ____LogPosition_h__

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

namespace EventSink
{

class LogPosition;
bool operator==(const LogPosition&, const LogPosition&);
bool operator!=(const LogPosition&, const LogPosition&);
bool operator<(const LogPosition&, const LogPosition&);
bool operator<=(const LogPosition&, const LogPosition&);
bool operator>(const LogPosition&, const LogPosition&);
bool operator>=(const LogPosition&, const LogPosition&);

}

}

namespace EventSink
{

class LogPosition;
bool operator==(const LogPosition&, const LogPosition&);
bool operator!=(const LogPosition&, const LogPosition&);
bool operator<(const LogPosition&, const LogPosition&);
bool operator<=(const LogPosition&, const LogPosition&);
bool operator>(const LogPosition&, const LogPosition&);
bool operator>=(const LogPosition&, const LogPosition&);

}

namespace IceInternal
{

void incRef(::EventSink::LogPosition*);
void decRef(::EventSink::LogPosition*);

void incRef(::IceProxy::EventSink::LogPosition*);
void decRef(::IceProxy::EventSink::LogPosition*);

}

namespace EventSink
{

typedef ::IceInternal::Handle< ::EventSink::LogPosition> LogPositionPtr;
typedef ::IceInternal::ProxyHandle< ::IceProxy::EventSink::LogPosition> LogPositionPrx;

void __write(::IceInternal::BasicStream*, const LogPositionPrx&);
void __read(::IceInternal::BasicStream*, LogPositionPrx&);
void __write(::IceInternal::BasicStream*, const LogPositionPtr&);
void __patch__LogPositionPtr(void*, ::Ice::ObjectPtr&);

void __addObject(const LogPositionPtr&, ::IceInternal::GCCountMap&);
bool __usesClasses(const LogPositionPtr&);
void __decRefUnsafe(const LogPositionPtr&);
void __clearHandleUnsafe(LogPositionPtr&);

}

namespace EventSink
{

struct PositionData
{
    ::std::string source;
    ::std::string handler;
    ::Ice::Long position;
    ::Ice::Long stamp;
    ::Ice::Long lastUpdated;

    bool operator==(const PositionData&) const;
    bool operator!=(const PositionData&) const;
    bool operator<(const PositionData&) const;
    bool operator<=(const PositionData& __rhs) const
    {
        return operator<(__rhs) || operator==(__rhs);
    }
    bool operator>(const PositionData& __rhs) const
    {
        return !operator<(__rhs) && !operator==(__rhs);
    }
    bool operator>=(const PositionData& __rhs) const
    {
        return !operator<(__rhs);
    }

    void __write(::IceInternal::BasicStream*) const;
    void __read(::IceInternal::BasicStream*);
};

}

namespace IceProxy
{

namespace EventSink
{

class LogPosition : virtual public ::IceProxy::Ice::Object
{
public:

    ::std::string getSourceName()
    {
        return getSourceName(0);
    }
    ::std::string getSourceName(const ::Ice::Context& __ctx)
    {
        return getSourceName(&__ctx);
    }
    
private:

    ::std::string getSourceName(const ::Ice::Context*);
    
public:

    ::std::string getHandlerName()
    {
        return getHandlerName(0);
    }
    ::std::string getHandlerName(const ::Ice::Context& __ctx)
    {
        return getHandlerName(&__ctx);
    }
    
private:

    ::std::string getHandlerName(const ::Ice::Context*);
    
public:

    ::Ice::Long getLastUpdatedTime()
    {
        return getLastUpdatedTime(0);
    }
    ::Ice::Long getLastUpdatedTime(const ::Ice::Context& __ctx)
    {
        return getLastUpdatedTime(&__ctx);
    }
    
private:

    ::Ice::Long getLastUpdatedTime(const ::Ice::Context*);
    
public:

    void getData(::Ice::Long& position, ::Ice::Long& stamp)
    {
        getData(position, stamp, 0);
    }
    void getData(::Ice::Long& position, ::Ice::Long& stamp, const ::Ice::Context& __ctx)
    {
        getData(position, stamp, &__ctx);
    }
    
private:

    void getData(::Ice::Long&, ::Ice::Long&, const ::Ice::Context*);
    
public:

    void updateData(::Ice::Long position, ::Ice::Long stamp)
    {
        updateData(position, stamp, 0);
    }
    void updateData(::Ice::Long position, ::Ice::Long stamp, const ::Ice::Context& __ctx)
    {
        updateData(position, stamp, &__ctx);
    }
    
private:

    void updateData(::Ice::Long, ::Ice::Long, const ::Ice::Context*);
    
public:
    
    static const ::std::string& ice_staticId();

private: 

    virtual ::IceInternal::Handle< ::IceDelegateM::Ice::Object> __createDelegateM();
    virtual ::IceInternal::Handle< ::IceDelegateD::Ice::Object> __createDelegateD();
};

}

}

namespace IceDelegate
{

namespace EventSink
{

class LogPosition : virtual public ::IceDelegate::Ice::Object
{
public:

    virtual ::std::string getSourceName(const ::Ice::Context*) = 0;

    virtual ::std::string getHandlerName(const ::Ice::Context*) = 0;

    virtual ::Ice::Long getLastUpdatedTime(const ::Ice::Context*) = 0;

    virtual void getData(::Ice::Long&, ::Ice::Long&, const ::Ice::Context*) = 0;

    virtual void updateData(::Ice::Long, ::Ice::Long, const ::Ice::Context*) = 0;
};

}

}

namespace IceDelegateM
{

namespace EventSink
{

class LogPosition : virtual public ::IceDelegate::EventSink::LogPosition,
                    virtual public ::IceDelegateM::Ice::Object
{
public:

    virtual ::std::string getSourceName(const ::Ice::Context*);

    virtual ::std::string getHandlerName(const ::Ice::Context*);

    virtual ::Ice::Long getLastUpdatedTime(const ::Ice::Context*);

    virtual void getData(::Ice::Long&, ::Ice::Long&, const ::Ice::Context*);

    virtual void updateData(::Ice::Long, ::Ice::Long, const ::Ice::Context*);
};

}

}

namespace IceDelegateD
{

namespace EventSink
{

class LogPosition : virtual public ::IceDelegate::EventSink::LogPosition,
                    virtual public ::IceDelegateD::Ice::Object
{
public:

    virtual ::std::string getSourceName(const ::Ice::Context*);

    virtual ::std::string getHandlerName(const ::Ice::Context*);

    virtual ::Ice::Long getLastUpdatedTime(const ::Ice::Context*);

    virtual void getData(::Ice::Long&, ::Ice::Long&, const ::Ice::Context*);

    virtual void updateData(::Ice::Long, ::Ice::Long, const ::Ice::Context*);
};

}

}

namespace EventSink
{

class LogPosition : virtual public ::Ice::Object
{
public:

    typedef LogPositionPrx ProxyType;
    typedef LogPositionPtr PointerType;
    
    LogPosition() {}
    explicit LogPosition(const ::EventSink::PositionData&);
    virtual ::Ice::ObjectPtr ice_clone() const;

    virtual bool ice_isA(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) const;
    virtual ::std::vector< ::std::string> ice_ids(const ::Ice::Current& = ::Ice::Current()) const;
    virtual const ::std::string& ice_id(const ::Ice::Current& = ::Ice::Current()) const;
    static const ::std::string& ice_staticId();


    virtual ::std::string getSourceName(const ::Ice::Current& = ::Ice::Current()) const = 0;
    ::IceInternal::DispatchStatus ___getSourceName(::IceInternal::Incoming&, const ::Ice::Current&) const;

    virtual ::std::string getHandlerName(const ::Ice::Current& = ::Ice::Current()) const = 0;
    ::IceInternal::DispatchStatus ___getHandlerName(::IceInternal::Incoming&, const ::Ice::Current&) const;

    virtual ::Ice::Long getLastUpdatedTime(const ::Ice::Current& = ::Ice::Current()) const = 0;
    ::IceInternal::DispatchStatus ___getLastUpdatedTime(::IceInternal::Incoming&, const ::Ice::Current&) const;

    virtual void getData(::Ice::Long&, ::Ice::Long&, const ::Ice::Current& = ::Ice::Current()) const = 0;
    ::IceInternal::DispatchStatus ___getData(::IceInternal::Incoming&, const ::Ice::Current&) const;

    virtual void updateData(::Ice::Long, ::Ice::Long, const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___updateData(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual ::IceInternal::DispatchStatus __dispatch(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual ::Ice::Int ice_operationAttributes(const ::std::string&) const;

    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*, bool);
    virtual void __write(const ::Ice::OutputStreamPtr&) const;
    virtual void __read(const ::Ice::InputStreamPtr&, bool);

    ::EventSink::PositionData data;
};

void __patch__LogPositionPtr(void*, ::Ice::ObjectPtr&);

}

#endif
