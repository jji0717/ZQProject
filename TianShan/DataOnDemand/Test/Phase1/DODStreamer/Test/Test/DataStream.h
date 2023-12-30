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

#ifndef __DataStream_h__
#define __DataStream_h__

#include <Ice/LocalObjectF.h>
#include <Ice/ProxyF.h>
#include <Ice/ObjectF.h>
#include <Ice/Exception.h>
#include <Ice/LocalObject.h>
#include <Ice/Proxy.h>
#include <Ice/Object.h>
#include <Ice/Outgoing.h>
#include <Ice/Incoming.h>
#include <Ice/IncomingAsync.h>
#include <Ice/Direct.h>
#include <Ice/UserExceptionFactory.h>
#include <Ice/FactoryTable.h>
#include <Ice/StreamF.h>
#include <Ice/BuiltinSequences.h>
#include <TsStreamer.h>
#include <Ice/UndefSysMacros.h>

#ifndef ICE_IGNORE_VERSION
#   if ICE_INT_VERSION / 100 != 301
#       error Ice version mismatch!
#   endif
#   if ICE_INT_VERSION % 100 < 1
#       error Ice patch level mismatch!
#   endif
#endif

namespace IceProxy
{

namespace DataOnDemand
{

class MuxItem;
bool operator==(const MuxItem&, const MuxItem&);
bool operator!=(const MuxItem&, const MuxItem&);
bool operator<(const MuxItem&, const MuxItem&);
bool operator<=(const MuxItem&, const MuxItem&);
bool operator>(const MuxItem&, const MuxItem&);
bool operator>=(const MuxItem&, const MuxItem&);

class DataStream;
bool operator==(const DataStream&, const DataStream&);
bool operator!=(const DataStream&, const DataStream&);
bool operator<(const DataStream&, const DataStream&);
bool operator<=(const DataStream&, const DataStream&);
bool operator>(const DataStream&, const DataStream&);
bool operator>=(const DataStream&, const DataStream&);

class DataStreamService;
bool operator==(const DataStreamService&, const DataStreamService&);
bool operator!=(const DataStreamService&, const DataStreamService&);
bool operator<(const DataStreamService&, const DataStreamService&);
bool operator<=(const DataStreamService&, const DataStreamService&);
bool operator>(const DataStreamService&, const DataStreamService&);
bool operator>=(const DataStreamService&, const DataStreamService&);

}

}

namespace DataOnDemand
{

class MuxItem;
bool operator==(const MuxItem&, const MuxItem&);
bool operator!=(const MuxItem&, const MuxItem&);
bool operator<(const MuxItem&, const MuxItem&);
bool operator<=(const MuxItem&, const MuxItem&);
bool operator>(const MuxItem&, const MuxItem&);
bool operator>=(const MuxItem&, const MuxItem&);

class DataStream;
bool operator==(const DataStream&, const DataStream&);
bool operator!=(const DataStream&, const DataStream&);
bool operator<(const DataStream&, const DataStream&);
bool operator<=(const DataStream&, const DataStream&);
bool operator>(const DataStream&, const DataStream&);
bool operator>=(const DataStream&, const DataStream&);

class DataStreamService;
bool operator==(const DataStreamService&, const DataStreamService&);
bool operator!=(const DataStreamService&, const DataStreamService&);
bool operator<(const DataStreamService&, const DataStreamService&);
bool operator<=(const DataStreamService&, const DataStreamService&);
bool operator>(const DataStreamService&, const DataStreamService&);
bool operator>=(const DataStreamService&, const DataStreamService&);

}

namespace IceInternal
{

void incRef(::DataOnDemand::MuxItem*);
void decRef(::DataOnDemand::MuxItem*);

void incRef(::IceProxy::DataOnDemand::MuxItem*);
void decRef(::IceProxy::DataOnDemand::MuxItem*);

void incRef(::DataOnDemand::DataStream*);
void decRef(::DataOnDemand::DataStream*);

void incRef(::IceProxy::DataOnDemand::DataStream*);
void decRef(::IceProxy::DataOnDemand::DataStream*);

void incRef(::DataOnDemand::DataStreamService*);
void decRef(::DataOnDemand::DataStreamService*);

void incRef(::IceProxy::DataOnDemand::DataStreamService*);
void decRef(::IceProxy::DataOnDemand::DataStreamService*);

}

namespace DataOnDemand
{

typedef ::IceInternal::Handle< ::DataOnDemand::MuxItem> MuxItemPtr;
typedef ::IceInternal::ProxyHandle< ::IceProxy::DataOnDemand::MuxItem> MuxItemPrx;

void __write(::IceInternal::BasicStream*, const MuxItemPrx&);
void __read(::IceInternal::BasicStream*, MuxItemPrx&);
void __write(::IceInternal::BasicStream*, const MuxItemPtr&);
void __patch__MuxItemPtr(void*, ::Ice::ObjectPtr&);

void __addObject(const MuxItemPtr&, ::IceInternal::GCCountMap&);
bool __usesClasses(const MuxItemPtr&);
void __decRefUnsafe(MuxItemPtr&);
void __clearHandleUnsafe(MuxItemPtr&);

typedef ::IceInternal::Handle< ::DataOnDemand::DataStream> DataStreamPtr;
typedef ::IceInternal::ProxyHandle< ::IceProxy::DataOnDemand::DataStream> DataStreamPrx;

void __write(::IceInternal::BasicStream*, const DataStreamPrx&);
void __read(::IceInternal::BasicStream*, DataStreamPrx&);
void __write(::IceInternal::BasicStream*, const DataStreamPtr&);
void __patch__DataStreamPtr(void*, ::Ice::ObjectPtr&);

void __addObject(const DataStreamPtr&, ::IceInternal::GCCountMap&);
bool __usesClasses(const DataStreamPtr&);
void __decRefUnsafe(DataStreamPtr&);
void __clearHandleUnsafe(DataStreamPtr&);

typedef ::IceInternal::Handle< ::DataOnDemand::DataStreamService> DataStreamServicePtr;
typedef ::IceInternal::ProxyHandle< ::IceProxy::DataOnDemand::DataStreamService> DataStreamServicePrx;

void __write(::IceInternal::BasicStream*, const DataStreamServicePrx&);
void __read(::IceInternal::BasicStream*, DataStreamServicePrx&);
void __write(::IceInternal::BasicStream*, const DataStreamServicePtr&);
void __patch__DataStreamServicePtr(void*, ::Ice::ObjectPtr&);

void __addObject(const DataStreamServicePtr&, ::IceInternal::GCCountMap&);
bool __usesClasses(const DataStreamServicePtr&);
void __decRefUnsafe(DataStreamServicePtr&);
void __clearHandleUnsafe(DataStreamServicePtr&);

}

namespace DataOnDemand
{

class DataStreamError : public ::TianShanIce::ServerError
{
public:

    DataStreamError() {}
    explicit DataStreamError(const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    static const ::IceInternal::UserExceptionFactoryPtr& ice_factory();

    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*, bool);

    virtual void __write(const ::Ice::OutputStreamPtr&) const;
    virtual void __read(const ::Ice::InputStreamPtr&, bool);
};

static DataStreamError __DataStreamError_init;

class NameDupException : public ::DataOnDemand::DataStreamError
{
public:

    NameDupException() {}
    explicit NameDupException(const ::std::string&);

    virtual const ::std::string ice_name() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    static const ::IceInternal::UserExceptionFactoryPtr& ice_factory();

    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*, bool);

    virtual void __write(const ::Ice::OutputStreamPtr&) const;
    virtual void __read(const ::Ice::InputStreamPtr&, bool);
};

enum CacheType
{
    dodCacheTypeSmb,
    dodCacheTypeTcp,
    dodCacheTypeUdp,
    dodCacheTypeIce
};

void __write(::IceInternal::BasicStream*, CacheType);
void __read(::IceInternal::BasicStream*, CacheType&);

struct MuxItemInfo
{
    ::std::string name;
    ::Ice::Int streamId;
    ::Ice::Int streamType;
    ::Ice::Int bandWidth;
    ::Ice::Int tag;
    ::Ice::Long expiration;
    ::Ice::Int repeatTime;
    ::DataOnDemand::CacheType ctype;
    ::std::string cacheAddr;
    ::Ice::Int encryptMode;
    ::Ice::Int subchannelCount;

    bool operator==(const MuxItemInfo&) const;
    bool operator!=(const MuxItemInfo&) const;
    bool operator<(const MuxItemInfo&) const;
    bool operator<=(const MuxItemInfo& __rhs) const
    {
	return operator<(__rhs) || operator==(__rhs);
    }
    bool operator>(const MuxItemInfo& __rhs) const
    {
	return !operator<(__rhs) && !operator==(__rhs);
    }
    bool operator>=(const MuxItemInfo& __rhs) const
    {
	return !operator<(__rhs);
    }

    void __write(::IceInternal::BasicStream*) const;
    void __read(::IceInternal::BasicStream*);
};

struct StreamInfo
{
    ::std::string name;
    ::Ice::Int totalBandwidth;
    ::std::string destAddress;
    ::Ice::Int pmtPid;

    bool operator==(const StreamInfo&) const;
    bool operator!=(const StreamInfo&) const;
    bool operator<(const StreamInfo&) const;
    bool operator<=(const StreamInfo& __rhs) const
    {
	return operator<(__rhs) || operator==(__rhs);
    }
    bool operator>(const StreamInfo& __rhs) const
    {
	return !operator<(__rhs) && !operator==(__rhs);
    }
    bool operator>=(const StreamInfo& __rhs) const
    {
	return !operator<(__rhs);
    }

    void __write(::IceInternal::BasicStream*) const;
    void __read(::IceInternal::BasicStream*);
};

}

namespace IceAsync
{

}

namespace IceProxy
{

namespace DataOnDemand
{

class MuxItem : virtual public ::IceProxy::Ice::Object
{
public:

    ::std::string getName()
    {
	return getName(__defaultContext());
    }
    ::std::string getName(const ::Ice::Context&);

    void notifyFullUpdate()
    {
	notifyFullUpdate(__defaultContext());
    }
    void notifyFullUpdate(const ::Ice::Context&);

    void notifyFileAdded(const ::std::string& fileName)
    {
	notifyFileAdded(fileName, __defaultContext());
    }
    void notifyFileAdded(const ::std::string&, const ::Ice::Context&);

    void notifyFileDeleted(const ::std::string& fileName)
    {
	notifyFileDeleted(fileName, __defaultContext());
    }
    void notifyFileDeleted(const ::std::string&, const ::Ice::Context&);

    ::DataOnDemand::MuxItemInfo getInfo()
    {
	return getInfo(__defaultContext());
    }
    ::DataOnDemand::MuxItemInfo getInfo(const ::Ice::Context&);

    void destory()
    {
	destory(__defaultContext());
    }
    void destory(const ::Ice::Context&);

    void setProperies(const ::TianShanIce::Properties& props)
    {
	setProperies(props, __defaultContext());
    }
    void setProperies(const ::TianShanIce::Properties&, const ::Ice::Context&);

    ::TianShanIce::Properties getProperties()
    {
	return getProperties(__defaultContext());
    }
    ::TianShanIce::Properties getProperties(const ::Ice::Context&);
    
    static const ::std::string& ice_staticId();

private: 

    virtual ::IceInternal::Handle< ::IceDelegateM::Ice::Object> __createDelegateM();
    virtual ::IceInternal::Handle< ::IceDelegateD::Ice::Object> __createDelegateD();
};

class DataStream : virtual public ::IceProxy::TianShanIce::Streamer::Stream
{
public:

    ::std::string getName()
    {
	return getName(__defaultContext());
    }
    ::std::string getName(const ::Ice::Context&);

    ::DataOnDemand::StreamInfo getInfo()
    {
	return getInfo(__defaultContext());
    }
    ::DataOnDemand::StreamInfo getInfo(const ::Ice::Context&);

    ::DataOnDemand::MuxItemPrx createMuxItem(const ::std::string& name, const ::DataOnDemand::MuxItemInfo& info)
    {
	return createMuxItem(name, info, __defaultContext());
    }
    ::DataOnDemand::MuxItemPrx createMuxItem(const ::std::string&, const ::DataOnDemand::MuxItemInfo&, const ::Ice::Context&);

    ::DataOnDemand::MuxItemPrx getMuxItem(const ::std::string& name)
    {
	return getMuxItem(name, __defaultContext());
    }
    ::DataOnDemand::MuxItemPrx getMuxItem(const ::std::string&, const ::Ice::Context&);

    ::Ice::StringSeq listMuxItems()
    {
	return listMuxItems(__defaultContext());
    }
    ::Ice::StringSeq listMuxItems(const ::Ice::Context&);

    void setProperies(const ::TianShanIce::Properties& props)
    {
	setProperies(props, __defaultContext());
    }
    void setProperies(const ::TianShanIce::Properties&, const ::Ice::Context&);

    ::TianShanIce::Properties getProperties()
    {
	return getProperties(__defaultContext());
    }
    ::TianShanIce::Properties getProperties(const ::Ice::Context&);
    
    static const ::std::string& ice_staticId();

private: 

    virtual ::IceInternal::Handle< ::IceDelegateM::Ice::Object> __createDelegateM();
    virtual ::IceInternal::Handle< ::IceDelegateD::Ice::Object> __createDelegateD();
};

class DataStreamService : virtual public ::IceProxy::TianShanIce::Streamer::StreamService
{
public:

    ::DataOnDemand::DataStreamPrx createStreamByApp(const ::TianShanIce::AccreditedPath::PathTicketPrx& pathTicket, const ::std::string& name, const ::DataOnDemand::StreamInfo& info)
    {
	return createStreamByApp(pathTicket, name, info, __defaultContext());
    }
    ::DataOnDemand::DataStreamPrx createStreamByApp(const ::TianShanIce::AccreditedPath::PathTicketPrx&, const ::std::string&, const ::DataOnDemand::StreamInfo&, const ::Ice::Context&);

    ::DataOnDemand::DataStreamPrx getStream(const ::std::string& name)
    {
	return getStream(name, __defaultContext());
    }
    ::DataOnDemand::DataStreamPrx getStream(const ::std::string&, const ::Ice::Context&);

    void setProperies(const ::TianShanIce::Properties& props)
    {
	setProperies(props, __defaultContext());
    }
    void setProperies(const ::TianShanIce::Properties&, const ::Ice::Context&);

    ::TianShanIce::Properties getProperties()
    {
	return getProperties(__defaultContext());
    }
    ::TianShanIce::Properties getProperties(const ::Ice::Context&);
    
    static const ::std::string& ice_staticId();

private: 

    virtual ::IceInternal::Handle< ::IceDelegateM::Ice::Object> __createDelegateM();
    virtual ::IceInternal::Handle< ::IceDelegateD::Ice::Object> __createDelegateD();
};

}

}

namespace IceDelegate
{

namespace DataOnDemand
{

class MuxItem : virtual public ::IceDelegate::Ice::Object
{
public:

    virtual ::std::string getName(const ::Ice::Context&) = 0;

    virtual void notifyFullUpdate(const ::Ice::Context&) = 0;

    virtual void notifyFileAdded(const ::std::string&, const ::Ice::Context&) = 0;

    virtual void notifyFileDeleted(const ::std::string&, const ::Ice::Context&) = 0;

    virtual ::DataOnDemand::MuxItemInfo getInfo(const ::Ice::Context&) = 0;

    virtual void destory(const ::Ice::Context&) = 0;

    virtual void setProperies(const ::TianShanIce::Properties&, const ::Ice::Context&) = 0;

    virtual ::TianShanIce::Properties getProperties(const ::Ice::Context&) = 0;
};

class DataStream : virtual public ::IceDelegate::TianShanIce::Streamer::Stream
{
public:

    virtual ::std::string getName(const ::Ice::Context&) = 0;

    virtual ::DataOnDemand::StreamInfo getInfo(const ::Ice::Context&) = 0;

    virtual ::DataOnDemand::MuxItemPrx createMuxItem(const ::std::string&, const ::DataOnDemand::MuxItemInfo&, const ::Ice::Context&) = 0;

    virtual ::DataOnDemand::MuxItemPrx getMuxItem(const ::std::string&, const ::Ice::Context&) = 0;

    virtual ::Ice::StringSeq listMuxItems(const ::Ice::Context&) = 0;

    virtual void setProperies(const ::TianShanIce::Properties&, const ::Ice::Context&) = 0;

    virtual ::TianShanIce::Properties getProperties(const ::Ice::Context&) = 0;
};

class DataStreamService : virtual public ::IceDelegate::TianShanIce::Streamer::StreamService
{
public:

    virtual ::DataOnDemand::DataStreamPrx createStreamByApp(const ::TianShanIce::AccreditedPath::PathTicketPrx&, const ::std::string&, const ::DataOnDemand::StreamInfo&, const ::Ice::Context&) = 0;

    virtual ::DataOnDemand::DataStreamPrx getStream(const ::std::string&, const ::Ice::Context&) = 0;

    virtual void setProperies(const ::TianShanIce::Properties&, const ::Ice::Context&) = 0;

    virtual ::TianShanIce::Properties getProperties(const ::Ice::Context&) = 0;
};

}

}

namespace IceDelegateM
{

namespace DataOnDemand
{

class MuxItem : virtual public ::IceDelegate::DataOnDemand::MuxItem,
		virtual public ::IceDelegateM::Ice::Object
{
public:

    virtual ::std::string getName(const ::Ice::Context&);

    virtual void notifyFullUpdate(const ::Ice::Context&);

    virtual void notifyFileAdded(const ::std::string&, const ::Ice::Context&);

    virtual void notifyFileDeleted(const ::std::string&, const ::Ice::Context&);

    virtual ::DataOnDemand::MuxItemInfo getInfo(const ::Ice::Context&);

    virtual void destory(const ::Ice::Context&);

    virtual void setProperies(const ::TianShanIce::Properties&, const ::Ice::Context&);

    virtual ::TianShanIce::Properties getProperties(const ::Ice::Context&);
};

class DataStream : virtual public ::IceDelegate::DataOnDemand::DataStream,
		   virtual public ::IceDelegateM::TianShanIce::Streamer::Stream
{
public:

    virtual ::std::string getName(const ::Ice::Context&);

    virtual ::DataOnDemand::StreamInfo getInfo(const ::Ice::Context&);

    virtual ::DataOnDemand::MuxItemPrx createMuxItem(const ::std::string&, const ::DataOnDemand::MuxItemInfo&, const ::Ice::Context&);

    virtual ::DataOnDemand::MuxItemPrx getMuxItem(const ::std::string&, const ::Ice::Context&);

    virtual ::Ice::StringSeq listMuxItems(const ::Ice::Context&);

    virtual void setProperies(const ::TianShanIce::Properties&, const ::Ice::Context&);

    virtual ::TianShanIce::Properties getProperties(const ::Ice::Context&);
};

class DataStreamService : virtual public ::IceDelegate::DataOnDemand::DataStreamService,
			  virtual public ::IceDelegateM::TianShanIce::Streamer::StreamService
{
public:

    virtual ::DataOnDemand::DataStreamPrx createStreamByApp(const ::TianShanIce::AccreditedPath::PathTicketPrx&, const ::std::string&, const ::DataOnDemand::StreamInfo&, const ::Ice::Context&);

    virtual ::DataOnDemand::DataStreamPrx getStream(const ::std::string&, const ::Ice::Context&);

    virtual void setProperies(const ::TianShanIce::Properties&, const ::Ice::Context&);

    virtual ::TianShanIce::Properties getProperties(const ::Ice::Context&);
};

}

}

namespace IceDelegateD
{

namespace DataOnDemand
{

class MuxItem : virtual public ::IceDelegate::DataOnDemand::MuxItem,
		virtual public ::IceDelegateD::Ice::Object
{
public:

    virtual ::std::string getName(const ::Ice::Context&);

    virtual void notifyFullUpdate(const ::Ice::Context&);

    virtual void notifyFileAdded(const ::std::string&, const ::Ice::Context&);

    virtual void notifyFileDeleted(const ::std::string&, const ::Ice::Context&);

    virtual ::DataOnDemand::MuxItemInfo getInfo(const ::Ice::Context&);

    virtual void destory(const ::Ice::Context&);

    virtual void setProperies(const ::TianShanIce::Properties&, const ::Ice::Context&);

    virtual ::TianShanIce::Properties getProperties(const ::Ice::Context&);
};

class DataStream : virtual public ::IceDelegate::DataOnDemand::DataStream,
		   virtual public ::IceDelegateD::TianShanIce::Streamer::Stream
{
public:

    virtual ::std::string getName(const ::Ice::Context&);

    virtual ::DataOnDemand::StreamInfo getInfo(const ::Ice::Context&);

    virtual ::DataOnDemand::MuxItemPrx createMuxItem(const ::std::string&, const ::DataOnDemand::MuxItemInfo&, const ::Ice::Context&);

    virtual ::DataOnDemand::MuxItemPrx getMuxItem(const ::std::string&, const ::Ice::Context&);

    virtual ::Ice::StringSeq listMuxItems(const ::Ice::Context&);

    virtual void setProperies(const ::TianShanIce::Properties&, const ::Ice::Context&);

    virtual ::TianShanIce::Properties getProperties(const ::Ice::Context&);
};

class DataStreamService : virtual public ::IceDelegate::DataOnDemand::DataStreamService,
			  virtual public ::IceDelegateD::TianShanIce::Streamer::StreamService
{
public:

    virtual ::DataOnDemand::DataStreamPrx createStreamByApp(const ::TianShanIce::AccreditedPath::PathTicketPrx&, const ::std::string&, const ::DataOnDemand::StreamInfo&, const ::Ice::Context&);

    virtual ::DataOnDemand::DataStreamPrx getStream(const ::std::string&, const ::Ice::Context&);

    virtual void setProperies(const ::TianShanIce::Properties&, const ::Ice::Context&);

    virtual ::TianShanIce::Properties getProperties(const ::Ice::Context&);
};

}

}

namespace DataOnDemand
{

class MuxItem : virtual public ::Ice::Object
{
public:

    typedef MuxItemPrx ProxyType;
    typedef MuxItemPtr PointerType;
    
    MuxItem() {}
    virtual ::Ice::ObjectPtr ice_clone() const;

    virtual bool ice_isA(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) const;
    virtual ::std::vector< ::std::string> ice_ids(const ::Ice::Current& = ::Ice::Current()) const;
    virtual const ::std::string& ice_id(const ::Ice::Current& = ::Ice::Current()) const;
    static const ::std::string& ice_staticId();

    virtual ::std::string getName(const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___getName(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual void notifyFullUpdate(const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___notifyFullUpdate(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual void notifyFileAdded(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___notifyFileAdded(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual void notifyFileDeleted(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___notifyFileDeleted(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual ::DataOnDemand::MuxItemInfo getInfo(const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___getInfo(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual void destory(const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___destory(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual void setProperies(const ::TianShanIce::Properties&, const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___setProperies(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual ::TianShanIce::Properties getProperties(const ::Ice::Current& = ::Ice::Current()) const = 0;
    ::IceInternal::DispatchStatus ___getProperties(::IceInternal::Incoming&, const ::Ice::Current&) const;

    virtual ::IceInternal::DispatchStatus __dispatch(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*, bool);
    virtual void __write(const ::Ice::OutputStreamPtr&) const;
    virtual void __read(const ::Ice::InputStreamPtr&, bool);
};

void __patch__MuxItemPtr(void*, ::Ice::ObjectPtr&);

class DataStream : virtual public ::TianShanIce::Streamer::Stream
{
public:

    typedef DataStreamPrx ProxyType;
    typedef DataStreamPtr PointerType;
    
    DataStream() {}
    explicit DataStream(const ::Ice::Identity&);
    virtual ::Ice::ObjectPtr ice_clone() const;

    virtual bool ice_isA(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) const;
    virtual ::std::vector< ::std::string> ice_ids(const ::Ice::Current& = ::Ice::Current()) const;
    virtual const ::std::string& ice_id(const ::Ice::Current& = ::Ice::Current()) const;
    static const ::std::string& ice_staticId();

    virtual ::std::string getName(const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___getName(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual ::DataOnDemand::StreamInfo getInfo(const ::Ice::Current& = ::Ice::Current()) const = 0;
    ::IceInternal::DispatchStatus ___getInfo(::IceInternal::Incoming&, const ::Ice::Current&) const;

    virtual ::DataOnDemand::MuxItemPrx createMuxItem(const ::std::string&, const ::DataOnDemand::MuxItemInfo&, const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___createMuxItem(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual ::DataOnDemand::MuxItemPrx getMuxItem(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___getMuxItem(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual ::Ice::StringSeq listMuxItems(const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___listMuxItems(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual void setProperies(const ::TianShanIce::Properties&, const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___setProperies(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual ::TianShanIce::Properties getProperties(const ::Ice::Current& = ::Ice::Current()) const = 0;
    ::IceInternal::DispatchStatus ___getProperties(::IceInternal::Incoming&, const ::Ice::Current&) const;

    virtual ::IceInternal::DispatchStatus __dispatch(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*, bool);
    virtual void __write(const ::Ice::OutputStreamPtr&) const;
    virtual void __read(const ::Ice::InputStreamPtr&, bool);
};

void __patch__DataStreamPtr(void*, ::Ice::ObjectPtr&);

class DataStreamService : virtual public ::TianShanIce::Streamer::StreamService
{
public:

    typedef DataStreamServicePrx ProxyType;
    typedef DataStreamServicePtr PointerType;
    
    DataStreamService() {}
    virtual ::Ice::ObjectPtr ice_clone() const;

    virtual bool ice_isA(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) const;
    virtual ::std::vector< ::std::string> ice_ids(const ::Ice::Current& = ::Ice::Current()) const;
    virtual const ::std::string& ice_id(const ::Ice::Current& = ::Ice::Current()) const;
    static const ::std::string& ice_staticId();

    virtual ::DataOnDemand::DataStreamPrx createStreamByApp(const ::TianShanIce::AccreditedPath::PathTicketPrx&, const ::std::string&, const ::DataOnDemand::StreamInfo&, const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___createStreamByApp(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual ::DataOnDemand::DataStreamPrx getStream(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___getStream(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual void setProperies(const ::TianShanIce::Properties&, const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___setProperies(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual ::TianShanIce::Properties getProperties(const ::Ice::Current& = ::Ice::Current()) const = 0;
    ::IceInternal::DispatchStatus ___getProperties(::IceInternal::Incoming&, const ::Ice::Current&) const;

    virtual ::IceInternal::DispatchStatus __dispatch(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*, bool);
    virtual void __write(const ::Ice::OutputStreamPtr&) const;
    virtual void __read(const ::Ice::InputStreamPtr&, bool);
};

void __patch__DataStreamServicePtr(void*, ::Ice::ObjectPtr&);

}

#endif
