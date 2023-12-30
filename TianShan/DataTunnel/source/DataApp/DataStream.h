// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// Ice version 3.2.1
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
#include <Ice/OutgoingAsync.h>
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

namespace Streamer
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

}

}

namespace TianShanIce
{

namespace Streamer
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

}

namespace IceInternal
{

void incRef(::TianShanIce::Streamer::DataOnDemand::MuxItem*);
void decRef(::TianShanIce::Streamer::DataOnDemand::MuxItem*);

void incRef(::IceProxy::TianShanIce::Streamer::DataOnDemand::MuxItem*);
void decRef(::IceProxy::TianShanIce::Streamer::DataOnDemand::MuxItem*);

void incRef(::TianShanIce::Streamer::DataOnDemand::DataStream*);
void decRef(::TianShanIce::Streamer::DataOnDemand::DataStream*);

void incRef(::IceProxy::TianShanIce::Streamer::DataOnDemand::DataStream*);
void decRef(::IceProxy::TianShanIce::Streamer::DataOnDemand::DataStream*);

void incRef(::TianShanIce::Streamer::DataOnDemand::DataStreamService*);
void decRef(::TianShanIce::Streamer::DataOnDemand::DataStreamService*);

void incRef(::IceProxy::TianShanIce::Streamer::DataOnDemand::DataStreamService*);
void decRef(::IceProxy::TianShanIce::Streamer::DataOnDemand::DataStreamService*);

}

namespace TianShanIce
{

namespace Streamer
{

namespace DataOnDemand
{

typedef ::IceInternal::Handle< ::TianShanIce::Streamer::DataOnDemand::MuxItem> MuxItemPtr;
typedef ::IceInternal::ProxyHandle< ::IceProxy::TianShanIce::Streamer::DataOnDemand::MuxItem> MuxItemPrx;

void __write(::IceInternal::BasicStream*, const MuxItemPrx&);
void __read(::IceInternal::BasicStream*, MuxItemPrx&);
void __write(::IceInternal::BasicStream*, const MuxItemPtr&);
void __patch__MuxItemPtr(void*, ::Ice::ObjectPtr&);

void __addObject(const MuxItemPtr&, ::IceInternal::GCCountMap&);
bool __usesClasses(const MuxItemPtr&);
void __decRefUnsafe(const MuxItemPtr&);
void __clearHandleUnsafe(MuxItemPtr&);

typedef ::IceInternal::Handle< ::TianShanIce::Streamer::DataOnDemand::DataStream> DataStreamPtr;
typedef ::IceInternal::ProxyHandle< ::IceProxy::TianShanIce::Streamer::DataOnDemand::DataStream> DataStreamPrx;

void __write(::IceInternal::BasicStream*, const DataStreamPrx&);
void __read(::IceInternal::BasicStream*, DataStreamPrx&);
void __write(::IceInternal::BasicStream*, const DataStreamPtr&);
void __patch__DataStreamPtr(void*, ::Ice::ObjectPtr&);

void __addObject(const DataStreamPtr&, ::IceInternal::GCCountMap&);
bool __usesClasses(const DataStreamPtr&);
void __decRefUnsafe(const DataStreamPtr&);
void __clearHandleUnsafe(DataStreamPtr&);

typedef ::IceInternal::Handle< ::TianShanIce::Streamer::DataOnDemand::DataStreamService> DataStreamServicePtr;
typedef ::IceInternal::ProxyHandle< ::IceProxy::TianShanIce::Streamer::DataOnDemand::DataStreamService> DataStreamServicePrx;

void __write(::IceInternal::BasicStream*, const DataStreamServicePrx&);
void __read(::IceInternal::BasicStream*, DataStreamServicePrx&);
void __write(::IceInternal::BasicStream*, const DataStreamServicePtr&);
void __patch__DataStreamServicePtr(void*, ::Ice::ObjectPtr&);

void __addObject(const DataStreamServicePtr&, ::IceInternal::GCCountMap&);
bool __usesClasses(const DataStreamServicePtr&);
void __decRefUnsafe(const DataStreamServicePtr&);
void __clearHandleUnsafe(DataStreamServicePtr&);

}

}

}

namespace TianShanIce
{

namespace Streamer
{

namespace DataOnDemand
{

class DataStreamError : public ::TianShanIce::ServerError
{
public:

    DataStreamError() {}
    DataStreamError(const ::std::string&, ::Ice::Int, const ::std::string&);
    virtual ~DataStreamError() throw();

    virtual ::std::string ice_name() const;
    virtual ::Ice::Exception* ice_clone() const;
    virtual void ice_throw() const;

    static const ::IceInternal::UserExceptionFactoryPtr& ice_factory();

    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*, bool);

    virtual void __write(const ::Ice::OutputStreamPtr&) const;
    virtual void __read(const ::Ice::InputStreamPtr&, bool);
};

static DataStreamError __DataStreamError_init;

class NameDupException : public ::TianShanIce::Streamer::DataOnDemand::DataStreamError
{
public:

    NameDupException() {}
    NameDupException(const ::std::string&, ::Ice::Int, const ::std::string&);
    virtual ~NameDupException() throw();

    virtual ::std::string ice_name() const;
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
    ::TianShanIce::Streamer::DataOnDemand::CacheType ctype;
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

}

}

namespace IceAsync
{

}

namespace IceProxy
{

namespace TianShanIce
{

namespace Streamer
{

namespace DataOnDemand
{

class MuxItem : virtual public ::IceProxy::Ice::Object
{
public:

    ::std::string getName()
    {
        return getName(0);
    }
    ::std::string getName(const ::Ice::Context& __ctx)
    {
        return getName(&__ctx);
    }
    
private:

    ::std::string getName(const ::Ice::Context*);
    
public:

    void notifyFullUpdate(const ::std::string& fileName)
    {
        notifyFullUpdate(fileName, 0);
    }
    void notifyFullUpdate(const ::std::string& fileName, const ::Ice::Context& __ctx)
    {
        notifyFullUpdate(fileName, &__ctx);
    }
    
private:

    void notifyFullUpdate(const ::std::string&, const ::Ice::Context*);
    
public:

    void notifyFileAdded(const ::std::string& fileName)
    {
        notifyFileAdded(fileName, 0);
    }
    void notifyFileAdded(const ::std::string& fileName, const ::Ice::Context& __ctx)
    {
        notifyFileAdded(fileName, &__ctx);
    }
    
private:

    void notifyFileAdded(const ::std::string&, const ::Ice::Context*);
    
public:

    void notifyFileDeleted(const ::std::string& fileName)
    {
        notifyFileDeleted(fileName, 0);
    }
    void notifyFileDeleted(const ::std::string& fileName, const ::Ice::Context& __ctx)
    {
        notifyFileDeleted(fileName, &__ctx);
    }
    
private:

    void notifyFileDeleted(const ::std::string&, const ::Ice::Context*);
    
public:

    ::TianShanIce::Streamer::DataOnDemand::MuxItemInfo getInfo()
    {
        return getInfo(0);
    }
    ::TianShanIce::Streamer::DataOnDemand::MuxItemInfo getInfo(const ::Ice::Context& __ctx)
    {
        return getInfo(&__ctx);
    }
    
private:

    ::TianShanIce::Streamer::DataOnDemand::MuxItemInfo getInfo(const ::Ice::Context*);
    
public:

    void destroy()
    {
        destroy(0);
    }
    void destroy(const ::Ice::Context& __ctx)
    {
        destroy(&__ctx);
    }
    
private:

    void destroy(const ::Ice::Context*);
    
public:

    void setProperties(const ::TianShanIce::Properties& props)
    {
        setProperties(props, 0);
    }
    void setProperties(const ::TianShanIce::Properties& props, const ::Ice::Context& __ctx)
    {
        setProperties(props, &__ctx);
    }
    
private:

    void setProperties(const ::TianShanIce::Properties&, const ::Ice::Context*);
    
public:

    ::TianShanIce::Properties getProperties()
    {
        return getProperties(0);
    }
    ::TianShanIce::Properties getProperties(const ::Ice::Context& __ctx)
    {
        return getProperties(&__ctx);
    }
    
private:

    ::TianShanIce::Properties getProperties(const ::Ice::Context*);
    
public:
    
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
        return getName(0);
    }
    ::std::string getName(const ::Ice::Context& __ctx)
    {
        return getName(&__ctx);
    }
    
private:

    ::std::string getName(const ::Ice::Context*);
    
public:

    ::TianShanIce::Streamer::DataOnDemand::StreamInfo getInfo()
    {
        return getInfo(0);
    }
    ::TianShanIce::Streamer::DataOnDemand::StreamInfo getInfo(const ::Ice::Context& __ctx)
    {
        return getInfo(&__ctx);
    }
    
private:

    ::TianShanIce::Streamer::DataOnDemand::StreamInfo getInfo(const ::Ice::Context*);
    
public:

    ::Ice::Int control(::Ice::Int code, const ::std::string& param)
    {
        return control(code, param, 0);
    }
    ::Ice::Int control(::Ice::Int code, const ::std::string& param, const ::Ice::Context& __ctx)
    {
        return control(code, param, &__ctx);
    }
    
private:

    ::Ice::Int control(::Ice::Int, const ::std::string&, const ::Ice::Context*);
    
public:

    ::TianShanIce::Streamer::DataOnDemand::MuxItemPrx createMuxItem(const ::TianShanIce::Streamer::DataOnDemand::MuxItemInfo& info)
    {
        return createMuxItem(info, 0);
    }
    ::TianShanIce::Streamer::DataOnDemand::MuxItemPrx createMuxItem(const ::TianShanIce::Streamer::DataOnDemand::MuxItemInfo& info, const ::Ice::Context& __ctx)
    {
        return createMuxItem(info, &__ctx);
    }
    
private:

    ::TianShanIce::Streamer::DataOnDemand::MuxItemPrx createMuxItem(const ::TianShanIce::Streamer::DataOnDemand::MuxItemInfo&, const ::Ice::Context*);
    
public:

    ::TianShanIce::Streamer::DataOnDemand::MuxItemPrx getMuxItem(const ::std::string& name)
    {
        return getMuxItem(name, 0);
    }
    ::TianShanIce::Streamer::DataOnDemand::MuxItemPrx getMuxItem(const ::std::string& name, const ::Ice::Context& __ctx)
    {
        return getMuxItem(name, &__ctx);
    }
    
private:

    ::TianShanIce::Streamer::DataOnDemand::MuxItemPrx getMuxItem(const ::std::string&, const ::Ice::Context*);
    
public:

    ::Ice::StringSeq listMuxItems()
    {
        return listMuxItems(0);
    }
    ::Ice::StringSeq listMuxItems(const ::Ice::Context& __ctx)
    {
        return listMuxItems(&__ctx);
    }
    
private:

    ::Ice::StringSeq listMuxItems(const ::Ice::Context*);
    
public:

    void ping()
    {
        ping(0);
    }
    void ping(const ::Ice::Context& __ctx)
    {
        ping(&__ctx);
    }
    
private:

    void ping(const ::Ice::Context*);
    
public:
    
    static const ::std::string& ice_staticId();

private: 

    virtual ::IceInternal::Handle< ::IceDelegateM::Ice::Object> __createDelegateM();
    virtual ::IceInternal::Handle< ::IceDelegateD::Ice::Object> __createDelegateD();
};

class DataStreamService : virtual public ::IceProxy::TianShanIce::Streamer::StreamService
{
public:

    ::TianShanIce::Streamer::DataOnDemand::DataStreamPrx createStreamByApp(const ::TianShanIce::Transport::PathTicketPrx& pathTicket, const ::std::string& space, const ::TianShanIce::Streamer::DataOnDemand::StreamInfo& info)
    {
        return createStreamByApp(pathTicket, space, info, 0);
    }
    ::TianShanIce::Streamer::DataOnDemand::DataStreamPrx createStreamByApp(const ::TianShanIce::Transport::PathTicketPrx& pathTicket, const ::std::string& space, const ::TianShanIce::Streamer::DataOnDemand::StreamInfo& info, const ::Ice::Context& __ctx)
    {
        return createStreamByApp(pathTicket, space, info, &__ctx);
    }
    
private:

    ::TianShanIce::Streamer::DataOnDemand::DataStreamPrx createStreamByApp(const ::TianShanIce::Transport::PathTicketPrx&, const ::std::string&, const ::TianShanIce::Streamer::DataOnDemand::StreamInfo&, const ::Ice::Context*);
    
public:

    ::TianShanIce::Streamer::DataOnDemand::DataStreamPrx getStream(const ::std::string& space, const ::std::string& name)
    {
        return getStream(space, name, 0);
    }
    ::TianShanIce::Streamer::DataOnDemand::DataStreamPrx getStream(const ::std::string& space, const ::std::string& name, const ::Ice::Context& __ctx)
    {
        return getStream(space, name, &__ctx);
    }
    
private:

    ::TianShanIce::Streamer::DataOnDemand::DataStreamPrx getStream(const ::std::string&, const ::std::string&, const ::Ice::Context*);
    
public:

    ::TianShanIce::StrValues listStreams(const ::std::string& space)
    {
        return listStreams(space, 0);
    }
    ::TianShanIce::StrValues listStreams(const ::std::string& space, const ::Ice::Context& __ctx)
    {
        return listStreams(space, &__ctx);
    }
    
private:

    ::TianShanIce::StrValues listStreams(const ::std::string&, const ::Ice::Context*);
    
public:

    void clear(const ::std::string& space)
    {
        clear(space, 0);
    }
    void clear(const ::std::string& space, const ::Ice::Context& __ctx)
    {
        clear(space, &__ctx);
    }
    
private:

    void clear(const ::std::string&, const ::Ice::Context*);
    
public:

    void destroy()
    {
        destroy(0);
    }
    void destroy(const ::Ice::Context& __ctx)
    {
        destroy(&__ctx);
    }
    
private:

    void destroy(const ::Ice::Context*);
    
public:

    void ping(const ::std::string& space)
    {
        ping(space, 0);
    }
    void ping(const ::std::string& space, const ::Ice::Context& __ctx)
    {
        ping(space, &__ctx);
    }
    
private:

    void ping(const ::std::string&, const ::Ice::Context*);
    
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

namespace Streamer
{

namespace DataOnDemand
{

class MuxItem : virtual public ::IceDelegate::Ice::Object
{
public:

    virtual ::std::string getName(const ::Ice::Context*) = 0;

    virtual void notifyFullUpdate(const ::std::string&, const ::Ice::Context*) = 0;

    virtual void notifyFileAdded(const ::std::string&, const ::Ice::Context*) = 0;

    virtual void notifyFileDeleted(const ::std::string&, const ::Ice::Context*) = 0;

    virtual ::TianShanIce::Streamer::DataOnDemand::MuxItemInfo getInfo(const ::Ice::Context*) = 0;

    virtual void destroy(const ::Ice::Context*) = 0;

    virtual void setProperties(const ::TianShanIce::Properties&, const ::Ice::Context*) = 0;

    virtual ::TianShanIce::Properties getProperties(const ::Ice::Context*) = 0;
};

class DataStream : virtual public ::IceDelegate::TianShanIce::Streamer::Stream
{
public:

    virtual ::std::string getName(const ::Ice::Context*) = 0;

    virtual ::TianShanIce::Streamer::DataOnDemand::StreamInfo getInfo(const ::Ice::Context*) = 0;

    virtual ::Ice::Int control(::Ice::Int, const ::std::string&, const ::Ice::Context*) = 0;

    virtual ::TianShanIce::Streamer::DataOnDemand::MuxItemPrx createMuxItem(const ::TianShanIce::Streamer::DataOnDemand::MuxItemInfo&, const ::Ice::Context*) = 0;

    virtual ::TianShanIce::Streamer::DataOnDemand::MuxItemPrx getMuxItem(const ::std::string&, const ::Ice::Context*) = 0;

    virtual ::Ice::StringSeq listMuxItems(const ::Ice::Context*) = 0;

    virtual void ping(const ::Ice::Context*) = 0;
};

class DataStreamService : virtual public ::IceDelegate::TianShanIce::Streamer::StreamService
{
public:

    virtual ::TianShanIce::Streamer::DataOnDemand::DataStreamPrx createStreamByApp(const ::TianShanIce::Transport::PathTicketPrx&, const ::std::string&, const ::TianShanIce::Streamer::DataOnDemand::StreamInfo&, const ::Ice::Context*) = 0;

    virtual ::TianShanIce::Streamer::DataOnDemand::DataStreamPrx getStream(const ::std::string&, const ::std::string&, const ::Ice::Context*) = 0;

    virtual ::TianShanIce::StrValues listStreams(const ::std::string&, const ::Ice::Context*) = 0;

    virtual void clear(const ::std::string&, const ::Ice::Context*) = 0;

    virtual void destroy(const ::Ice::Context*) = 0;

    virtual void ping(const ::std::string&, const ::Ice::Context*) = 0;
};

}

}

}

}

namespace IceDelegateM
{

namespace TianShanIce
{

namespace Streamer
{

namespace DataOnDemand
{

class MuxItem : virtual public ::IceDelegate::TianShanIce::Streamer::DataOnDemand::MuxItem,
                virtual public ::IceDelegateM::Ice::Object
{
public:

    virtual ::std::string getName(const ::Ice::Context*);

    virtual void notifyFullUpdate(const ::std::string&, const ::Ice::Context*);

    virtual void notifyFileAdded(const ::std::string&, const ::Ice::Context*);

    virtual void notifyFileDeleted(const ::std::string&, const ::Ice::Context*);

    virtual ::TianShanIce::Streamer::DataOnDemand::MuxItemInfo getInfo(const ::Ice::Context*);

    virtual void destroy(const ::Ice::Context*);

    virtual void setProperties(const ::TianShanIce::Properties&, const ::Ice::Context*);

    virtual ::TianShanIce::Properties getProperties(const ::Ice::Context*);
};

class DataStream : virtual public ::IceDelegate::TianShanIce::Streamer::DataOnDemand::DataStream,
                   virtual public ::IceDelegateM::TianShanIce::Streamer::Stream
{
public:

    virtual ::std::string getName(const ::Ice::Context*);

    virtual ::TianShanIce::Streamer::DataOnDemand::StreamInfo getInfo(const ::Ice::Context*);

    virtual ::Ice::Int control(::Ice::Int, const ::std::string&, const ::Ice::Context*);

    virtual ::TianShanIce::Streamer::DataOnDemand::MuxItemPrx createMuxItem(const ::TianShanIce::Streamer::DataOnDemand::MuxItemInfo&, const ::Ice::Context*);

    virtual ::TianShanIce::Streamer::DataOnDemand::MuxItemPrx getMuxItem(const ::std::string&, const ::Ice::Context*);

    virtual ::Ice::StringSeq listMuxItems(const ::Ice::Context*);

    virtual void ping(const ::Ice::Context*);
};

class DataStreamService : virtual public ::IceDelegate::TianShanIce::Streamer::DataOnDemand::DataStreamService,
                          virtual public ::IceDelegateM::TianShanIce::Streamer::StreamService
{
public:

    virtual ::TianShanIce::Streamer::DataOnDemand::DataStreamPrx createStreamByApp(const ::TianShanIce::Transport::PathTicketPrx&, const ::std::string&, const ::TianShanIce::Streamer::DataOnDemand::StreamInfo&, const ::Ice::Context*);

    virtual ::TianShanIce::Streamer::DataOnDemand::DataStreamPrx getStream(const ::std::string&, const ::std::string&, const ::Ice::Context*);

    virtual ::TianShanIce::StrValues listStreams(const ::std::string&, const ::Ice::Context*);

    virtual void clear(const ::std::string&, const ::Ice::Context*);

    virtual void destroy(const ::Ice::Context*);

    virtual void ping(const ::std::string&, const ::Ice::Context*);
};

}

}

}

}

namespace IceDelegateD
{

namespace TianShanIce
{

namespace Streamer
{

namespace DataOnDemand
{

class MuxItem : virtual public ::IceDelegate::TianShanIce::Streamer::DataOnDemand::MuxItem,
                virtual public ::IceDelegateD::Ice::Object
{
public:

    virtual ::std::string getName(const ::Ice::Context*);

    virtual void notifyFullUpdate(const ::std::string&, const ::Ice::Context*);

    virtual void notifyFileAdded(const ::std::string&, const ::Ice::Context*);

    virtual void notifyFileDeleted(const ::std::string&, const ::Ice::Context*);

    virtual ::TianShanIce::Streamer::DataOnDemand::MuxItemInfo getInfo(const ::Ice::Context*);

    virtual void destroy(const ::Ice::Context*);

    virtual void setProperties(const ::TianShanIce::Properties&, const ::Ice::Context*);

    virtual ::TianShanIce::Properties getProperties(const ::Ice::Context*);
};

class DataStream : virtual public ::IceDelegate::TianShanIce::Streamer::DataOnDemand::DataStream,
                   virtual public ::IceDelegateD::TianShanIce::Streamer::Stream
{
public:

    virtual ::std::string getName(const ::Ice::Context*);

    virtual ::TianShanIce::Streamer::DataOnDemand::StreamInfo getInfo(const ::Ice::Context*);

    virtual ::Ice::Int control(::Ice::Int, const ::std::string&, const ::Ice::Context*);

    virtual ::TianShanIce::Streamer::DataOnDemand::MuxItemPrx createMuxItem(const ::TianShanIce::Streamer::DataOnDemand::MuxItemInfo&, const ::Ice::Context*);

    virtual ::TianShanIce::Streamer::DataOnDemand::MuxItemPrx getMuxItem(const ::std::string&, const ::Ice::Context*);

    virtual ::Ice::StringSeq listMuxItems(const ::Ice::Context*);

    virtual void ping(const ::Ice::Context*);
};

class DataStreamService : virtual public ::IceDelegate::TianShanIce::Streamer::DataOnDemand::DataStreamService,
                          virtual public ::IceDelegateD::TianShanIce::Streamer::StreamService
{
public:

    virtual ::TianShanIce::Streamer::DataOnDemand::DataStreamPrx createStreamByApp(const ::TianShanIce::Transport::PathTicketPrx&, const ::std::string&, const ::TianShanIce::Streamer::DataOnDemand::StreamInfo&, const ::Ice::Context*);

    virtual ::TianShanIce::Streamer::DataOnDemand::DataStreamPrx getStream(const ::std::string&, const ::std::string&, const ::Ice::Context*);

    virtual ::TianShanIce::StrValues listStreams(const ::std::string&, const ::Ice::Context*);

    virtual void clear(const ::std::string&, const ::Ice::Context*);

    virtual void destroy(const ::Ice::Context*);

    virtual void ping(const ::std::string&, const ::Ice::Context*);
};

}

}

}

}

namespace TianShanIce
{

namespace Streamer
{

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

    virtual ::std::string getName(const ::Ice::Current& = ::Ice::Current()) const = 0;
    ::IceInternal::DispatchStatus ___getName(::IceInternal::Incoming&, const ::Ice::Current&) const;

    virtual void notifyFullUpdate(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___notifyFullUpdate(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual void notifyFileAdded(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___notifyFileAdded(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual void notifyFileDeleted(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___notifyFileDeleted(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual ::TianShanIce::Streamer::DataOnDemand::MuxItemInfo getInfo(const ::Ice::Current& = ::Ice::Current()) const = 0;
    ::IceInternal::DispatchStatus ___getInfo(::IceInternal::Incoming&, const ::Ice::Current&) const;

    virtual void destroy(const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___destroy(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual void setProperties(const ::TianShanIce::Properties&, const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___setProperties(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual ::TianShanIce::Properties getProperties(const ::Ice::Current& = ::Ice::Current()) const = 0;
    ::IceInternal::DispatchStatus ___getProperties(::IceInternal::Incoming&, const ::Ice::Current&) const;

    virtual ::IceInternal::DispatchStatus __dispatch(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual ::Ice::Int ice_operationAttributes(const ::std::string&) const;

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

    virtual ::std::string getName(const ::Ice::Current& = ::Ice::Current()) const = 0;
    ::IceInternal::DispatchStatus ___getName(::IceInternal::Incoming&, const ::Ice::Current&) const;

    virtual ::TianShanIce::Streamer::DataOnDemand::StreamInfo getInfo(const ::Ice::Current& = ::Ice::Current()) const = 0;
    ::IceInternal::DispatchStatus ___getInfo(::IceInternal::Incoming&, const ::Ice::Current&) const;

    virtual ::Ice::Int control(::Ice::Int, const ::std::string&, const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___control(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual ::TianShanIce::Streamer::DataOnDemand::MuxItemPrx createMuxItem(const ::TianShanIce::Streamer::DataOnDemand::MuxItemInfo&, const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___createMuxItem(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual ::TianShanIce::Streamer::DataOnDemand::MuxItemPrx getMuxItem(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___getMuxItem(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual ::Ice::StringSeq listMuxItems(const ::Ice::Current& = ::Ice::Current()) const = 0;
    ::IceInternal::DispatchStatus ___listMuxItems(::IceInternal::Incoming&, const ::Ice::Current&) const;

    virtual void ping(const ::Ice::Current& = ::Ice::Current()) const = 0;
    ::IceInternal::DispatchStatus ___ping(::IceInternal::Incoming&, const ::Ice::Current&) const;

    virtual ::IceInternal::DispatchStatus __dispatch(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual ::Ice::Int ice_operationAttributes(const ::std::string&) const;

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
    
    virtual ::Ice::ObjectPtr ice_clone() const;

    virtual bool ice_isA(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) const;
    virtual ::std::vector< ::std::string> ice_ids(const ::Ice::Current& = ::Ice::Current()) const;
    virtual const ::std::string& ice_id(const ::Ice::Current& = ::Ice::Current()) const;
    static const ::std::string& ice_staticId();

    virtual ::TianShanIce::Streamer::DataOnDemand::DataStreamPrx createStreamByApp(const ::TianShanIce::Transport::PathTicketPrx&, const ::std::string&, const ::TianShanIce::Streamer::DataOnDemand::StreamInfo&, const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___createStreamByApp(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual ::TianShanIce::Streamer::DataOnDemand::DataStreamPrx getStream(const ::std::string&, const ::std::string&, const ::Ice::Current& = ::Ice::Current()) const = 0;
    ::IceInternal::DispatchStatus ___getStream(::IceInternal::Incoming&, const ::Ice::Current&) const;

    virtual ::TianShanIce::StrValues listStreams(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) const = 0;
    ::IceInternal::DispatchStatus ___listStreams(::IceInternal::Incoming&, const ::Ice::Current&) const;

    virtual void clear(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___clear(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual void destroy(const ::Ice::Current& = ::Ice::Current()) = 0;
    ::IceInternal::DispatchStatus ___destroy(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual void ping(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) const = 0;
    ::IceInternal::DispatchStatus ___ping(::IceInternal::Incoming&, const ::Ice::Current&) const;

    virtual ::IceInternal::DispatchStatus __dispatch(::IceInternal::Incoming&, const ::Ice::Current&);

    virtual ::Ice::Int ice_operationAttributes(const ::std::string&) const;

    virtual void __write(::IceInternal::BasicStream*) const;
    virtual void __read(::IceInternal::BasicStream*, bool);
    virtual void __write(const ::Ice::OutputStreamPtr&) const;
    virtual void __read(const ::Ice::InputStreamPtr&, bool);
};

void __patch__DataStreamServicePtr(void*, ::Ice::ObjectPtr&);

}

}

}

#endif
