// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// Ice version 3.2.1
// Generated from file `Admin.ice'

#include <IceGrid/Admin.h>
#include <Ice/LocalException.h>
#include <Ice/ObjectFactory.h>
#include <Ice/BasicStream.h>
#include <Ice/Object.h>
#include <Ice/SliceChecksums.h>
#include <IceUtil/Iterator.h>
#include <IceUtil/ScopedArray.h>
#include <IceUtil/DisableWarnings.h>

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

static const ::std::string __IceGrid__Admin__addApplication_name = "addApplication";

static const ::std::string __IceGrid__Admin__syncApplication_name = "syncApplication";

static const ::std::string __IceGrid__Admin__updateApplication_name = "updateApplication";

static const ::std::string __IceGrid__Admin__removeApplication_name = "removeApplication";

static const ::std::string __IceGrid__Admin__instantiateServer_name = "instantiateServer";

static const ::std::string __IceGrid__Admin__patchApplication_name = "patchApplication";

static const ::std::string __IceGrid__Admin__getApplicationInfo_name = "getApplicationInfo";

static const ::std::string __IceGrid__Admin__getDefaultApplicationDescriptor_name = "getDefaultApplicationDescriptor";

static const ::std::string __IceGrid__Admin__getAllApplicationNames_name = "getAllApplicationNames";

static const ::std::string __IceGrid__Admin__getServerInfo_name = "getServerInfo";

static const ::std::string __IceGrid__Admin__getServerState_name = "getServerState";

static const ::std::string __IceGrid__Admin__getServerPid_name = "getServerPid";

static const ::std::string __IceGrid__Admin__enableServer_name = "enableServer";

static const ::std::string __IceGrid__Admin__isServerEnabled_name = "isServerEnabled";

static const ::std::string __IceGrid__Admin__startServer_name = "startServer";

static const ::std::string __IceGrid__Admin__stopServer_name = "stopServer";

static const ::std::string __IceGrid__Admin__patchServer_name = "patchServer";

static const ::std::string __IceGrid__Admin__sendSignal_name = "sendSignal";

static const ::std::string __IceGrid__Admin__writeMessage_name = "writeMessage";

static const ::std::string __IceGrid__Admin__getAllServerIds_name = "getAllServerIds";

static const ::std::string __IceGrid__Admin__getAdapterInfo_name = "getAdapterInfo";

static const ::std::string __IceGrid__Admin__removeAdapter_name = "removeAdapter";

static const ::std::string __IceGrid__Admin__getAllAdapterIds_name = "getAllAdapterIds";

static const ::std::string __IceGrid__Admin__addObject_name = "addObject";

static const ::std::string __IceGrid__Admin__updateObject_name = "updateObject";

static const ::std::string __IceGrid__Admin__addObjectWithType_name = "addObjectWithType";

static const ::std::string __IceGrid__Admin__removeObject_name = "removeObject";

static const ::std::string __IceGrid__Admin__getObjectInfo_name = "getObjectInfo";

static const ::std::string __IceGrid__Admin__getObjectInfosByType_name = "getObjectInfosByType";

static const ::std::string __IceGrid__Admin__getAllObjectInfos_name = "getAllObjectInfos";

static const ::std::string __IceGrid__Admin__pingNode_name = "pingNode";

static const ::std::string __IceGrid__Admin__getNodeLoad_name = "getNodeLoad";

static const ::std::string __IceGrid__Admin__getNodeInfo_name = "getNodeInfo";

static const ::std::string __IceGrid__Admin__shutdownNode_name = "shutdownNode";

static const ::std::string __IceGrid__Admin__getNodeHostname_name = "getNodeHostname";

static const ::std::string __IceGrid__Admin__getAllNodeNames_name = "getAllNodeNames";

static const ::std::string __IceGrid__Admin__pingRegistry_name = "pingRegistry";

static const ::std::string __IceGrid__Admin__getRegistryInfo_name = "getRegistryInfo";

static const ::std::string __IceGrid__Admin__shutdownRegistry_name = "shutdownRegistry";

static const ::std::string __IceGrid__Admin__getAllRegistryNames_name = "getAllRegistryNames";

static const ::std::string __IceGrid__Admin__shutdown_name = "shutdown";

static const ::std::string __IceGrid__Admin__getSliceChecksums_name = "getSliceChecksums";

static const ::std::string __IceGrid__FileIterator__read_name = "read";

static const ::std::string __IceGrid__FileIterator__destroy_name = "destroy";

static const ::std::string __IceGrid__AdminSession__keepAlive_name = "keepAlive";

static const ::std::string __IceGrid__AdminSession__getAdmin_name = "getAdmin";

static const ::std::string __IceGrid__AdminSession__setObservers_name = "setObservers";

static const ::std::string __IceGrid__AdminSession__setObserversByIdentity_name = "setObserversByIdentity";

static const ::std::string __IceGrid__AdminSession__startUpdate_name = "startUpdate";

static const ::std::string __IceGrid__AdminSession__finishUpdate_name = "finishUpdate";

static const ::std::string __IceGrid__AdminSession__getReplicaName_name = "getReplicaName";

static const ::std::string __IceGrid__AdminSession__openServerLog_name = "openServerLog";

static const ::std::string __IceGrid__AdminSession__openServerStdErr_name = "openServerStdErr";

static const ::std::string __IceGrid__AdminSession__openServerStdOut_name = "openServerStdOut";

static const ::std::string __IceGrid__AdminSession__openNodeStdErr_name = "openNodeStdErr";

static const ::std::string __IceGrid__AdminSession__openNodeStdOut_name = "openNodeStdOut";

static const ::std::string __IceGrid__AdminSession__openRegistryStdErr_name = "openRegistryStdErr";

static const ::std::string __IceGrid__AdminSession__openRegistryStdOut_name = "openRegistryStdOut";

void
IceInternal::incRef(::IceGrid::Admin* p)
{
    p->__incRef();
}

void
IceInternal::decRef(::IceGrid::Admin* p)
{
    p->__decRef();
}

void
IceInternal::incRef(::IceProxy::IceGrid::Admin* p)
{
    p->__incRef();
}

void
IceInternal::decRef(::IceProxy::IceGrid::Admin* p)
{
    p->__decRef();
}

void
IceInternal::incRef(::IceGrid::FileIterator* p)
{
    p->__incRef();
}

void
IceInternal::decRef(::IceGrid::FileIterator* p)
{
    p->__decRef();
}

void
IceInternal::incRef(::IceProxy::IceGrid::FileIterator* p)
{
    p->__incRef();
}

void
IceInternal::decRef(::IceProxy::IceGrid::FileIterator* p)
{
    p->__decRef();
}

void
IceInternal::incRef(::IceGrid::AdminSession* p)
{
    p->__incRef();
}

void
IceInternal::decRef(::IceGrid::AdminSession* p)
{
    p->__decRef();
}

void
IceInternal::incRef(::IceProxy::IceGrid::AdminSession* p)
{
    p->__incRef();
}

void
IceInternal::decRef(::IceProxy::IceGrid::AdminSession* p)
{
    p->__decRef();
}

void
IceGrid::__write(::IceInternal::BasicStream* __os, const ::IceGrid::AdminPrx& v)
{
    __os->write(::Ice::ObjectPrx(v));
}

void
IceGrid::__read(::IceInternal::BasicStream* __is, ::IceGrid::AdminPrx& v)
{
    ::Ice::ObjectPrx proxy;
    __is->read(proxy);
    if(!proxy)
    {
        v = 0;
    }
    else
    {
        v = new ::IceProxy::IceGrid::Admin;
        v->__copyFrom(proxy);
    }
}

void
IceGrid::__write(::IceInternal::BasicStream* __os, const ::IceGrid::AdminPtr& v)
{
    __os->write(::Ice::ObjectPtr(v));
}

void
IceGrid::__write(::IceInternal::BasicStream* __os, const ::IceGrid::FileIteratorPrx& v)
{
    __os->write(::Ice::ObjectPrx(v));
}

void
IceGrid::__read(::IceInternal::BasicStream* __is, ::IceGrid::FileIteratorPrx& v)
{
    ::Ice::ObjectPrx proxy;
    __is->read(proxy);
    if(!proxy)
    {
        v = 0;
    }
    else
    {
        v = new ::IceProxy::IceGrid::FileIterator;
        v->__copyFrom(proxy);
    }
}

void
IceGrid::__write(::IceInternal::BasicStream* __os, const ::IceGrid::FileIteratorPtr& v)
{
    __os->write(::Ice::ObjectPtr(v));
}

void
IceGrid::__write(::IceInternal::BasicStream* __os, const ::IceGrid::AdminSessionPrx& v)
{
    __os->write(::Ice::ObjectPrx(v));
}

void
IceGrid::__read(::IceInternal::BasicStream* __is, ::IceGrid::AdminSessionPrx& v)
{
    ::Ice::ObjectPrx proxy;
    __is->read(proxy);
    if(!proxy)
    {
        v = 0;
    }
    else
    {
        v = new ::IceProxy::IceGrid::AdminSession;
        v->__copyFrom(proxy);
    }
}

void
IceGrid::__write(::IceInternal::BasicStream* __os, const ::IceGrid::AdminSessionPtr& v)
{
    __os->write(::Ice::ObjectPtr(v));
}

void
IceGrid::__write(::IceInternal::BasicStream* __os, ::IceGrid::ServerState v)
{
    __os->write(static_cast< ::Ice::Byte>(v));
}

void
IceGrid::__read(::IceInternal::BasicStream* __is, ::IceGrid::ServerState& v)
{
    ::Ice::Byte val;
    __is->read(val);
    v = static_cast< ::IceGrid::ServerState>(val);
}

void
IceGrid::__write(::IceInternal::BasicStream* __os, const ::IceGrid::StringObjectProxyDict& v, ::IceGrid::__U__StringObjectProxyDict)
{
    __os->writeSize(::Ice::Int(v.size()));
    ::IceGrid::StringObjectProxyDict::const_iterator p;
    for(p = v.begin(); p != v.end(); ++p)
    {
        __os->write(p->first);
        __os->write(p->second);
    }
}

void
IceGrid::__read(::IceInternal::BasicStream* __is, ::IceGrid::StringObjectProxyDict& v, ::IceGrid::__U__StringObjectProxyDict)
{
    ::Ice::Int sz;
    __is->readSize(sz);
    while(sz--)
    {
        ::std::pair<const  ::std::string, ::Ice::ObjectPrx> pair;
        __is->read(const_cast< ::std::string&>(pair.first));
        ::IceGrid::StringObjectProxyDict::iterator __i = v.insert(v.end(), pair);
        __is->read(__i->second);
    }
}

bool
IceGrid::ObjectInfo::operator==(const ObjectInfo& __rhs) const
{
    return !operator!=(__rhs);
}

bool
IceGrid::ObjectInfo::operator!=(const ObjectInfo& __rhs) const
{
    if(this == &__rhs)
    {
        return false;
    }
    if(proxy != __rhs.proxy)
    {
        return true;
    }
    if(type != __rhs.type)
    {
        return true;
    }
    return false;
}

bool
IceGrid::ObjectInfo::operator<(const ObjectInfo& __rhs) const
{
    if(this == &__rhs)
    {
        return false;
    }
    if(proxy < __rhs.proxy)
    {
        return true;
    }
    else if(__rhs.proxy < proxy)
    {
        return false;
    }
    if(type < __rhs.type)
    {
        return true;
    }
    else if(__rhs.type < type)
    {
        return false;
    }
    return false;
}

void
IceGrid::ObjectInfo::__write(::IceInternal::BasicStream* __os) const
{
    __os->write(proxy);
    __os->write(type);
}

void
IceGrid::ObjectInfo::__read(::IceInternal::BasicStream* __is)
{
    __is->read(proxy);
    __is->read(type);
}

void
IceGrid::__write(::IceInternal::BasicStream* __os, const ::IceGrid::ObjectInfo* begin, const ::IceGrid::ObjectInfo* end, ::IceGrid::__U__ObjectInfoSeq)
{
    ::Ice::Int size = static_cast< ::Ice::Int>(end - begin);
    __os->writeSize(size);
    for(int i = 0; i < size; ++i)
    {
        begin[i].__write(__os);
    }
}

void
IceGrid::__read(::IceInternal::BasicStream* __is, ::IceGrid::ObjectInfoSeq& v, ::IceGrid::__U__ObjectInfoSeq)
{
    ::Ice::Int sz;
    __is->readSize(sz);
    __is->startSeq(sz, 3);
    v.resize(sz);
    for(int i = 0; i < sz; ++i)
    {
        v[i].__read(__is);
        __is->checkSeq();
        __is->endElement();
    }
    __is->endSeq(sz);
}

bool
IceGrid::AdapterInfo::operator==(const AdapterInfo& __rhs) const
{
    return !operator!=(__rhs);
}

bool
IceGrid::AdapterInfo::operator!=(const AdapterInfo& __rhs) const
{
    if(this == &__rhs)
    {
        return false;
    }
    if(id != __rhs.id)
    {
        return true;
    }
    if(proxy != __rhs.proxy)
    {
        return true;
    }
    if(replicaGroupId != __rhs.replicaGroupId)
    {
        return true;
    }
    return false;
}

bool
IceGrid::AdapterInfo::operator<(const AdapterInfo& __rhs) const
{
    if(this == &__rhs)
    {
        return false;
    }
    if(id < __rhs.id)
    {
        return true;
    }
    else if(__rhs.id < id)
    {
        return false;
    }
    if(proxy < __rhs.proxy)
    {
        return true;
    }
    else if(__rhs.proxy < proxy)
    {
        return false;
    }
    if(replicaGroupId < __rhs.replicaGroupId)
    {
        return true;
    }
    else if(__rhs.replicaGroupId < replicaGroupId)
    {
        return false;
    }
    return false;
}

void
IceGrid::AdapterInfo::__write(::IceInternal::BasicStream* __os) const
{
    __os->write(id);
    __os->write(proxy);
    __os->write(replicaGroupId);
}

void
IceGrid::AdapterInfo::__read(::IceInternal::BasicStream* __is)
{
    __is->read(id);
    __is->read(proxy);
    __is->read(replicaGroupId);
}

void
IceGrid::__write(::IceInternal::BasicStream* __os, const ::IceGrid::AdapterInfo* begin, const ::IceGrid::AdapterInfo* end, ::IceGrid::__U__AdapterInfoSeq)
{
    ::Ice::Int size = static_cast< ::Ice::Int>(end - begin);
    __os->writeSize(size);
    for(int i = 0; i < size; ++i)
    {
        begin[i].__write(__os);
    }
}

void
IceGrid::__read(::IceInternal::BasicStream* __is, ::IceGrid::AdapterInfoSeq& v, ::IceGrid::__U__AdapterInfoSeq)
{
    ::Ice::Int sz;
    __is->readSize(sz);
    __is->startSeq(sz, 4);
    v.resize(sz);
    for(int i = 0; i < sz; ++i)
    {
        v[i].__read(__is);
        __is->checkSeq();
        __is->endElement();
    }
    __is->endSeq(sz);
}

bool
IceGrid::ServerInfo::operator==(const ServerInfo& __rhs) const
{
    return !operator!=(__rhs);
}

bool
IceGrid::ServerInfo::operator!=(const ServerInfo& __rhs) const
{
    if(this == &__rhs)
    {
        return false;
    }
    if(application != __rhs.application)
    {
        return true;
    }
    if(uuid != __rhs.uuid)
    {
        return true;
    }
    if(revision != __rhs.revision)
    {
        return true;
    }
    if(node != __rhs.node)
    {
        return true;
    }
    if(descriptor != __rhs.descriptor)
    {
        return true;
    }
    if(sessionId != __rhs.sessionId)
    {
        return true;
    }
    return false;
}

bool
IceGrid::ServerInfo::operator<(const ServerInfo& __rhs) const
{
    if(this == &__rhs)
    {
        return false;
    }
    if(application < __rhs.application)
    {
        return true;
    }
    else if(__rhs.application < application)
    {
        return false;
    }
    if(uuid < __rhs.uuid)
    {
        return true;
    }
    else if(__rhs.uuid < uuid)
    {
        return false;
    }
    if(revision < __rhs.revision)
    {
        return true;
    }
    else if(__rhs.revision < revision)
    {
        return false;
    }
    if(node < __rhs.node)
    {
        return true;
    }
    else if(__rhs.node < node)
    {
        return false;
    }
    if(descriptor < __rhs.descriptor)
    {
        return true;
    }
    else if(__rhs.descriptor < descriptor)
    {
        return false;
    }
    if(sessionId < __rhs.sessionId)
    {
        return true;
    }
    else if(__rhs.sessionId < sessionId)
    {
        return false;
    }
    return false;
}

void
IceGrid::ServerInfo::__write(::IceInternal::BasicStream* __os) const
{
    __os->write(application);
    __os->write(uuid);
    __os->write(revision);
    __os->write(node);
    ::IceGrid::__write(__os, descriptor);
    __os->write(sessionId);
}

void
IceGrid::ServerInfo::__read(::IceInternal::BasicStream* __is)
{
    __is->read(application);
    __is->read(uuid);
    __is->read(revision);
    __is->read(node);
    __is->read(::IceGrid::__patch__ServerDescriptorPtr, &descriptor);
    __is->read(sessionId);
}

bool
IceGrid::NodeInfo::operator==(const NodeInfo& __rhs) const
{
    return !operator!=(__rhs);
}

bool
IceGrid::NodeInfo::operator!=(const NodeInfo& __rhs) const
{
    if(this == &__rhs)
    {
        return false;
    }
    if(name != __rhs.name)
    {
        return true;
    }
    if(os != __rhs.os)
    {
        return true;
    }
    if(hostname != __rhs.hostname)
    {
        return true;
    }
    if(release != __rhs.release)
    {
        return true;
    }
    if(version != __rhs.version)
    {
        return true;
    }
    if(machine != __rhs.machine)
    {
        return true;
    }
    if(nProcessors != __rhs.nProcessors)
    {
        return true;
    }
    if(dataDir != __rhs.dataDir)
    {
        return true;
    }
    return false;
}

bool
IceGrid::NodeInfo::operator<(const NodeInfo& __rhs) const
{
    if(this == &__rhs)
    {
        return false;
    }
    if(name < __rhs.name)
    {
        return true;
    }
    else if(__rhs.name < name)
    {
        return false;
    }
    if(os < __rhs.os)
    {
        return true;
    }
    else if(__rhs.os < os)
    {
        return false;
    }
    if(hostname < __rhs.hostname)
    {
        return true;
    }
    else if(__rhs.hostname < hostname)
    {
        return false;
    }
    if(release < __rhs.release)
    {
        return true;
    }
    else if(__rhs.release < release)
    {
        return false;
    }
    if(version < __rhs.version)
    {
        return true;
    }
    else if(__rhs.version < version)
    {
        return false;
    }
    if(machine < __rhs.machine)
    {
        return true;
    }
    else if(__rhs.machine < machine)
    {
        return false;
    }
    if(nProcessors < __rhs.nProcessors)
    {
        return true;
    }
    else if(__rhs.nProcessors < nProcessors)
    {
        return false;
    }
    if(dataDir < __rhs.dataDir)
    {
        return true;
    }
    else if(__rhs.dataDir < dataDir)
    {
        return false;
    }
    return false;
}

void
IceGrid::NodeInfo::__write(::IceInternal::BasicStream* __os) const
{
    __os->write(name);
    __os->write(os);
    __os->write(hostname);
    __os->write(release);
    __os->write(version);
    __os->write(machine);
    __os->write(nProcessors);
    __os->write(dataDir);
}

void
IceGrid::NodeInfo::__read(::IceInternal::BasicStream* __is)
{
    __is->read(name);
    __is->read(os);
    __is->read(hostname);
    __is->read(release);
    __is->read(version);
    __is->read(machine);
    __is->read(nProcessors);
    __is->read(dataDir);
}

bool
IceGrid::RegistryInfo::operator==(const RegistryInfo& __rhs) const
{
    return !operator!=(__rhs);
}

bool
IceGrid::RegistryInfo::operator!=(const RegistryInfo& __rhs) const
{
    if(this == &__rhs)
    {
        return false;
    }
    if(name != __rhs.name)
    {
        return true;
    }
    if(hostname != __rhs.hostname)
    {
        return true;
    }
    return false;
}

bool
IceGrid::RegistryInfo::operator<(const RegistryInfo& __rhs) const
{
    if(this == &__rhs)
    {
        return false;
    }
    if(name < __rhs.name)
    {
        return true;
    }
    else if(__rhs.name < name)
    {
        return false;
    }
    if(hostname < __rhs.hostname)
    {
        return true;
    }
    else if(__rhs.hostname < hostname)
    {
        return false;
    }
    return false;
}

void
IceGrid::RegistryInfo::__write(::IceInternal::BasicStream* __os) const
{
    __os->write(name);
    __os->write(hostname);
}

void
IceGrid::RegistryInfo::__read(::IceInternal::BasicStream* __is)
{
    __is->read(name);
    __is->read(hostname);
}

void
IceGrid::__write(::IceInternal::BasicStream* __os, const ::IceGrid::RegistryInfo* begin, const ::IceGrid::RegistryInfo* end, ::IceGrid::__U__RegistryInfoSeq)
{
    ::Ice::Int size = static_cast< ::Ice::Int>(end - begin);
    __os->writeSize(size);
    for(int i = 0; i < size; ++i)
    {
        begin[i].__write(__os);
    }
}

void
IceGrid::__read(::IceInternal::BasicStream* __is, ::IceGrid::RegistryInfoSeq& v, ::IceGrid::__U__RegistryInfoSeq)
{
    ::Ice::Int sz;
    __is->readSize(sz);
    __is->startSeq(sz, 2);
    v.resize(sz);
    for(int i = 0; i < sz; ++i)
    {
        v[i].__read(__is);
        __is->checkSeq();
        __is->endElement();
    }
    __is->endSeq(sz);
}

bool
IceGrid::LoadInfo::operator==(const LoadInfo& __rhs) const
{
    return !operator!=(__rhs);
}

bool
IceGrid::LoadInfo::operator!=(const LoadInfo& __rhs) const
{
    if(this == &__rhs)
    {
        return false;
    }
    if(avg1 != __rhs.avg1)
    {
        return true;
    }
    if(avg5 != __rhs.avg5)
    {
        return true;
    }
    if(avg15 != __rhs.avg15)
    {
        return true;
    }
    return false;
}

bool
IceGrid::LoadInfo::operator<(const LoadInfo& __rhs) const
{
    if(this == &__rhs)
    {
        return false;
    }
    if(avg1 < __rhs.avg1)
    {
        return true;
    }
    else if(__rhs.avg1 < avg1)
    {
        return false;
    }
    if(avg5 < __rhs.avg5)
    {
        return true;
    }
    else if(__rhs.avg5 < avg5)
    {
        return false;
    }
    if(avg15 < __rhs.avg15)
    {
        return true;
    }
    else if(__rhs.avg15 < avg15)
    {
        return false;
    }
    return false;
}

void
IceGrid::LoadInfo::__write(::IceInternal::BasicStream* __os) const
{
    __os->write(avg1);
    __os->write(avg5);
    __os->write(avg15);
}

void
IceGrid::LoadInfo::__read(::IceInternal::BasicStream* __is)
{
    __is->read(avg1);
    __is->read(avg5);
    __is->read(avg15);
}

bool
IceGrid::ApplicationInfo::operator==(const ApplicationInfo& __rhs) const
{
    return !operator!=(__rhs);
}

bool
IceGrid::ApplicationInfo::operator!=(const ApplicationInfo& __rhs) const
{
    if(this == &__rhs)
    {
        return false;
    }
    if(uuid != __rhs.uuid)
    {
        return true;
    }
    if(createTime != __rhs.createTime)
    {
        return true;
    }
    if(createUser != __rhs.createUser)
    {
        return true;
    }
    if(updateTime != __rhs.updateTime)
    {
        return true;
    }
    if(updateUser != __rhs.updateUser)
    {
        return true;
    }
    if(revision != __rhs.revision)
    {
        return true;
    }
    if(descriptor != __rhs.descriptor)
    {
        return true;
    }
    return false;
}

bool
IceGrid::ApplicationInfo::operator<(const ApplicationInfo& __rhs) const
{
    if(this == &__rhs)
    {
        return false;
    }
    if(uuid < __rhs.uuid)
    {
        return true;
    }
    else if(__rhs.uuid < uuid)
    {
        return false;
    }
    if(createTime < __rhs.createTime)
    {
        return true;
    }
    else if(__rhs.createTime < createTime)
    {
        return false;
    }
    if(createUser < __rhs.createUser)
    {
        return true;
    }
    else if(__rhs.createUser < createUser)
    {
        return false;
    }
    if(updateTime < __rhs.updateTime)
    {
        return true;
    }
    else if(__rhs.updateTime < updateTime)
    {
        return false;
    }
    if(updateUser < __rhs.updateUser)
    {
        return true;
    }
    else if(__rhs.updateUser < updateUser)
    {
        return false;
    }
    if(revision < __rhs.revision)
    {
        return true;
    }
    else if(__rhs.revision < revision)
    {
        return false;
    }
    if(descriptor < __rhs.descriptor)
    {
        return true;
    }
    else if(__rhs.descriptor < descriptor)
    {
        return false;
    }
    return false;
}

void
IceGrid::ApplicationInfo::__write(::IceInternal::BasicStream* __os) const
{
    __os->write(uuid);
    __os->write(createTime);
    __os->write(createUser);
    __os->write(updateTime);
    __os->write(updateUser);
    __os->write(revision);
    descriptor.__write(__os);
}

void
IceGrid::ApplicationInfo::__read(::IceInternal::BasicStream* __is)
{
    __is->read(uuid);
    __is->read(createTime);
    __is->read(createUser);
    __is->read(updateTime);
    __is->read(updateUser);
    __is->read(revision);
    descriptor.__read(__is);
}

void
IceGrid::__write(::IceInternal::BasicStream* __os, const ::IceGrid::ApplicationInfo* begin, const ::IceGrid::ApplicationInfo* end, ::IceGrid::__U__ApplicationInfoSeq)
{
    ::Ice::Int size = static_cast< ::Ice::Int>(end - begin);
    __os->writeSize(size);
    for(int i = 0; i < size; ++i)
    {
        begin[i].__write(__os);
    }
}

void
IceGrid::__read(::IceInternal::BasicStream* __is, ::IceGrid::ApplicationInfoSeq& v, ::IceGrid::__U__ApplicationInfoSeq)
{
    ::Ice::Int sz;
    __is->readSize(sz);
    __is->startSeq(sz, 33);
    v.resize(sz);
    for(int i = 0; i < sz; ++i)
    {
        v[i].__read(__is);
        __is->checkSeq();
        __is->endElement();
    }
    __is->endSeq(sz);
}

bool
IceGrid::ApplicationUpdateInfo::operator==(const ApplicationUpdateInfo& __rhs) const
{
    return !operator!=(__rhs);
}

bool
IceGrid::ApplicationUpdateInfo::operator!=(const ApplicationUpdateInfo& __rhs) const
{
    if(this == &__rhs)
    {
        return false;
    }
    if(updateTime != __rhs.updateTime)
    {
        return true;
    }
    if(updateUser != __rhs.updateUser)
    {
        return true;
    }
    if(revision != __rhs.revision)
    {
        return true;
    }
    if(descriptor != __rhs.descriptor)
    {
        return true;
    }
    return false;
}

bool
IceGrid::ApplicationUpdateInfo::operator<(const ApplicationUpdateInfo& __rhs) const
{
    if(this == &__rhs)
    {
        return false;
    }
    if(updateTime < __rhs.updateTime)
    {
        return true;
    }
    else if(__rhs.updateTime < updateTime)
    {
        return false;
    }
    if(updateUser < __rhs.updateUser)
    {
        return true;
    }
    else if(__rhs.updateUser < updateUser)
    {
        return false;
    }
    if(revision < __rhs.revision)
    {
        return true;
    }
    else if(__rhs.revision < revision)
    {
        return false;
    }
    if(descriptor < __rhs.descriptor)
    {
        return true;
    }
    else if(__rhs.descriptor < descriptor)
    {
        return false;
    }
    return false;
}

void
IceGrid::ApplicationUpdateInfo::__write(::IceInternal::BasicStream* __os) const
{
    __os->write(updateTime);
    __os->write(updateUser);
    __os->write(revision);
    descriptor.__write(__os);
}

void
IceGrid::ApplicationUpdateInfo::__read(::IceInternal::BasicStream* __is)
{
    __is->read(updateTime);
    __is->read(updateUser);
    __is->read(revision);
    descriptor.__read(__is);
}

void
IceGrid::__addObject(const AdminPtr& p, ::IceInternal::GCCountMap& c)
{
    p->__addObject(c);
}

bool
IceGrid::__usesClasses(const AdminPtr& p)
{
    return p->__usesClasses();
}

void
IceGrid::__decRefUnsafe(const AdminPtr& p)
{
    p->__decRefUnsafe();
}

void
IceGrid::__clearHandleUnsafe(AdminPtr& p)
{
    p.__clearHandleUnsafe();
}

void
IceGrid::__addObject(const FileIteratorPtr& p, ::IceInternal::GCCountMap& c)
{
    p->__addObject(c);
}

bool
IceGrid::__usesClasses(const FileIteratorPtr& p)
{
    return p->__usesClasses();
}

void
IceGrid::__decRefUnsafe(const FileIteratorPtr& p)
{
    p->__decRefUnsafe();
}

void
IceGrid::__clearHandleUnsafe(FileIteratorPtr& p)
{
    p.__clearHandleUnsafe();
}

void
IceGrid::__addObject(const AdminSessionPtr& p, ::IceInternal::GCCountMap& c)
{
    p->__addObject(c);
}

bool
IceGrid::__usesClasses(const AdminSessionPtr& p)
{
    return p->__usesClasses();
}

void
IceGrid::__decRefUnsafe(const AdminSessionPtr& p)
{
    p->__decRefUnsafe();
}

void
IceGrid::__clearHandleUnsafe(AdminSessionPtr& p)
{
    p.__clearHandleUnsafe();
}

void
IceGrid::AMI_Admin_addApplication::__invoke(const ::IceGrid::AdminPrx& __prx, const ::IceGrid::ApplicationDescriptor& descriptor, const ::Ice::Context* __ctx)
{
    try
    {
        __prepare(__prx, __IceGrid__Admin__addApplication_name, ::Ice::Normal, __ctx);
        descriptor.__write(__os);
        __os->writePendingObjects();
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
IceGrid::AMI_Admin_addApplication::__response(bool __ok)
{
    try
    {
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::IceGrid::AccessDeniedException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::IceGrid::DeploymentException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __finished(__ex);
        return;
    }
    ice_response();
}

void
IceGrid::AMI_Admin_syncApplication::__invoke(const ::IceGrid::AdminPrx& __prx, const ::IceGrid::ApplicationDescriptor& descriptor, const ::Ice::Context* __ctx)
{
    try
    {
        __prepare(__prx, __IceGrid__Admin__syncApplication_name, ::Ice::Normal, __ctx);
        descriptor.__write(__os);
        __os->writePendingObjects();
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
IceGrid::AMI_Admin_syncApplication::__response(bool __ok)
{
    try
    {
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::IceGrid::AccessDeniedException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::IceGrid::ApplicationNotExistException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::IceGrid::DeploymentException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __finished(__ex);
        return;
    }
    ice_response();
}

void
IceGrid::AMI_Admin_updateApplication::__invoke(const ::IceGrid::AdminPrx& __prx, const ::IceGrid::ApplicationUpdateDescriptor& descriptor, const ::Ice::Context* __ctx)
{
    try
    {
        __prepare(__prx, __IceGrid__Admin__updateApplication_name, ::Ice::Normal, __ctx);
        descriptor.__write(__os);
        __os->writePendingObjects();
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
IceGrid::AMI_Admin_updateApplication::__response(bool __ok)
{
    try
    {
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::IceGrid::AccessDeniedException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::IceGrid::ApplicationNotExistException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::IceGrid::DeploymentException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __finished(__ex);
        return;
    }
    ice_response();
}

void
IceGrid::AMI_Admin_removeApplication::__invoke(const ::IceGrid::AdminPrx& __prx, const ::std::string& name, const ::Ice::Context* __ctx)
{
    try
    {
        __prepare(__prx, __IceGrid__Admin__removeApplication_name, ::Ice::Normal, __ctx);
        __os->write(name);
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
IceGrid::AMI_Admin_removeApplication::__response(bool __ok)
{
    try
    {
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::IceGrid::AccessDeniedException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::IceGrid::ApplicationNotExistException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::IceGrid::DeploymentException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __finished(__ex);
        return;
    }
    ice_response();
}

void
IceGrid::AMI_Admin_patchApplication::__invoke(const ::IceGrid::AdminPrx& __prx, const ::std::string& name, bool shutdown, const ::Ice::Context* __ctx)
{
    try
    {
        __prepare(__prx, __IceGrid__Admin__patchApplication_name, ::Ice::Normal, __ctx);
        __os->write(name);
        __os->write(shutdown);
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
IceGrid::AMI_Admin_patchApplication::__response(bool __ok)
{
    try
    {
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::IceGrid::ApplicationNotExistException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::IceGrid::PatchException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __finished(__ex);
        return;
    }
    ice_response();
}

void
IceGrid::AMI_Admin_enableServer::__invoke(const ::IceGrid::AdminPrx& __prx, const ::std::string& id, bool enabled, const ::Ice::Context* __ctx)
{
    try
    {
        __prepare(__prx, __IceGrid__Admin__enableServer_name, ::Ice::Idempotent, __ctx);
        __os->write(id);
        __os->write(enabled);
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
IceGrid::AMI_Admin_enableServer::__response(bool __ok)
{
    try
    {
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::IceGrid::DeploymentException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::IceGrid::NodeUnreachableException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::IceGrid::ServerNotExistException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __finished(__ex);
        return;
    }
    ice_response();
}

void
IceGrid::AMI_Admin_startServer::__invoke(const ::IceGrid::AdminPrx& __prx, const ::std::string& id, const ::Ice::Context* __ctx)
{
    try
    {
        __prepare(__prx, __IceGrid__Admin__startServer_name, ::Ice::Normal, __ctx);
        __os->write(id);
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
IceGrid::AMI_Admin_startServer::__response(bool __ok)
{
    try
    {
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::IceGrid::DeploymentException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::IceGrid::NodeUnreachableException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::IceGrid::ServerNotExistException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::IceGrid::ServerStartException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __finished(__ex);
        return;
    }
    ice_response();
}

void
IceGrid::AMI_Admin_stopServer::__invoke(const ::IceGrid::AdminPrx& __prx, const ::std::string& id, const ::Ice::Context* __ctx)
{
    try
    {
        __prepare(__prx, __IceGrid__Admin__stopServer_name, ::Ice::Normal, __ctx);
        __os->write(id);
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
IceGrid::AMI_Admin_stopServer::__response(bool __ok)
{
    try
    {
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::IceGrid::DeploymentException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::IceGrid::NodeUnreachableException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::IceGrid::ServerNotExistException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::IceGrid::ServerStopException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __finished(__ex);
        return;
    }
    ice_response();
}

void
IceGrid::AMI_Admin_patchServer::__invoke(const ::IceGrid::AdminPrx& __prx, const ::std::string& id, bool shutdown, const ::Ice::Context* __ctx)
{
    try
    {
        __prepare(__prx, __IceGrid__Admin__patchServer_name, ::Ice::Normal, __ctx);
        __os->write(id);
        __os->write(shutdown);
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
IceGrid::AMI_Admin_patchServer::__response(bool __ok)
{
    try
    {
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::IceGrid::DeploymentException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::IceGrid::NodeUnreachableException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::IceGrid::PatchException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::IceGrid::ServerNotExistException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __finished(__ex);
        return;
    }
    ice_response();
}

void
IceGrid::AMI_Admin_sendSignal::__invoke(const ::IceGrid::AdminPrx& __prx, const ::std::string& id, const ::std::string& signal, const ::Ice::Context* __ctx)
{
    try
    {
        __prepare(__prx, __IceGrid__Admin__sendSignal_name, ::Ice::Normal, __ctx);
        __os->write(id);
        __os->write(signal);
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
IceGrid::AMI_Admin_sendSignal::__response(bool __ok)
{
    try
    {
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::IceGrid::BadSignalException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::IceGrid::DeploymentException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::IceGrid::NodeUnreachableException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::IceGrid::ServerNotExistException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __finished(__ex);
        return;
    }
    ice_response();
}

void
IceGrid::AMI_Admin_writeMessage::__invoke(const ::IceGrid::AdminPrx& __prx, const ::std::string& id, const ::std::string& message, ::Ice::Int fd, const ::Ice::Context* __ctx)
{
    try
    {
        __prepare(__prx, __IceGrid__Admin__writeMessage_name, ::Ice::Normal, __ctx);
        __os->write(id);
        __os->write(message);
        __os->write(fd);
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
IceGrid::AMI_Admin_writeMessage::__response(bool __ok)
{
    try
    {
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::IceGrid::DeploymentException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::IceGrid::NodeUnreachableException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::IceGrid::ServerNotExistException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __finished(__ex);
        return;
    }
    ice_response();
}

void
IceGrid::AMI_Admin_removeAdapter::__invoke(const ::IceGrid::AdminPrx& __prx, const ::std::string& adapterId, const ::Ice::Context* __ctx)
{
    try
    {
        __prepare(__prx, __IceGrid__Admin__removeAdapter_name, ::Ice::Normal, __ctx);
        __os->write(adapterId);
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
IceGrid::AMI_Admin_removeAdapter::__response(bool __ok)
{
    try
    {
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::IceGrid::AdapterNotExistException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::IceGrid::DeploymentException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __finished(__ex);
        return;
    }
    ice_response();
}

void
IceGrid::AMI_Admin_addObject::__invoke(const ::IceGrid::AdminPrx& __prx, const ::Ice::ObjectPrx& obj, const ::Ice::Context* __ctx)
{
    try
    {
        __prepare(__prx, __IceGrid__Admin__addObject_name, ::Ice::Normal, __ctx);
        __os->write(obj);
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
IceGrid::AMI_Admin_addObject::__response(bool __ok)
{
    try
    {
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::IceGrid::DeploymentException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::IceGrid::ObjectExistsException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __finished(__ex);
        return;
    }
    ice_response();
}

void
IceGrid::AMI_Admin_addObjectWithType::__invoke(const ::IceGrid::AdminPrx& __prx, const ::Ice::ObjectPrx& obj, const ::std::string& type, const ::Ice::Context* __ctx)
{
    try
    {
        __prepare(__prx, __IceGrid__Admin__addObjectWithType_name, ::Ice::Normal, __ctx);
        __os->write(obj);
        __os->write(type);
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
IceGrid::AMI_Admin_addObjectWithType::__response(bool __ok)
{
    try
    {
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::IceGrid::DeploymentException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::IceGrid::ObjectExistsException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __finished(__ex);
        return;
    }
    ice_response();
}

void
IceGrid::AMI_Admin_removeObject::__invoke(const ::IceGrid::AdminPrx& __prx, const ::Ice::Identity& id, const ::Ice::Context* __ctx)
{
    try
    {
        __prepare(__prx, __IceGrid__Admin__removeObject_name, ::Ice::Normal, __ctx);
        id.__write(__os);
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
IceGrid::AMI_Admin_removeObject::__response(bool __ok)
{
    try
    {
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::IceGrid::DeploymentException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::IceGrid::ObjectNotRegisteredException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __finished(__ex);
        return;
    }
    ice_response();
}

void
IceGrid::AMI_Admin_getNodeLoad::__invoke(const ::IceGrid::AdminPrx& __prx, const ::std::string& name, const ::Ice::Context* __ctx)
{
    try
    {
        __prepare(__prx, __IceGrid__Admin__getNodeLoad_name, ::Ice::Nonmutating, __ctx);
        __os->write(name);
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
IceGrid::AMI_Admin_getNodeLoad::__response(bool __ok)
{
    ::IceGrid::LoadInfo __ret;
    try
    {
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::IceGrid::NodeNotExistException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::IceGrid::NodeUnreachableException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        __ret.__read(__is);
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __finished(__ex);
        return;
    }
    ice_response(__ret);
}

void
IceGrid::AMI_Admin_shutdownNode::__invoke(const ::IceGrid::AdminPrx& __prx, const ::std::string& name, const ::Ice::Context* __ctx)
{
    try
    {
        __prepare(__prx, __IceGrid__Admin__shutdownNode_name, ::Ice::Normal, __ctx);
        __os->write(name);
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
IceGrid::AMI_Admin_shutdownNode::__response(bool __ok)
{
    try
    {
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::IceGrid::NodeNotExistException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::IceGrid::NodeUnreachableException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __finished(__ex);
        return;
    }
    ice_response();
}

void
IceGrid::AMI_Admin_shutdownRegistry::__invoke(const ::IceGrid::AdminPrx& __prx, const ::std::string& name, const ::Ice::Context* __ctx)
{
    try
    {
        __prepare(__prx, __IceGrid__Admin__shutdownRegistry_name, ::Ice::Idempotent, __ctx);
        __os->write(name);
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
IceGrid::AMI_Admin_shutdownRegistry::__response(bool __ok)
{
    try
    {
        if(!__ok)
        {
            try
            {
                __is->throwException();
            }
            catch(const ::IceGrid::RegistryNotExistException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::IceGrid::RegistryUnreachableException& __ex)
            {
                ice_exception(__ex);
                return;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        __finished(__ex);
        return;
    }
    ice_response();
}

IceAsync::IceGrid::AMD_Admin_patchApplication::AMD_Admin_patchApplication(::IceInternal::Incoming& in) :
    ::IceInternal::IncomingAsync(in)
{
}

void
IceAsync::IceGrid::AMD_Admin_patchApplication::ice_response()
{
    __response(true);
}

void
IceAsync::IceGrid::AMD_Admin_patchApplication::ice_exception(const ::Ice::Exception& ex)
{
    try
    {
        ex.ice_throw();
    }
    catch(const ::IceGrid::ApplicationNotExistException& __ex)
    {
        __os()->write(__ex);
        __response(false);
    }
    catch(const ::IceGrid::PatchException& __ex)
    {
        __os()->write(__ex);
        __response(false);
    }
    catch(const ::Ice::Exception& __ex)
    {
        __exception(__ex);
    }
}

void
IceAsync::IceGrid::AMD_Admin_patchApplication::ice_exception(const ::std::exception& ex)
{
    __exception(ex);
}

void
IceAsync::IceGrid::AMD_Admin_patchApplication::ice_exception()
{
    __exception();
}

IceAsync::IceGrid::AMD_Admin_patchServer::AMD_Admin_patchServer(::IceInternal::Incoming& in) :
    ::IceInternal::IncomingAsync(in)
{
}

void
IceAsync::IceGrid::AMD_Admin_patchServer::ice_response()
{
    __response(true);
}

void
IceAsync::IceGrid::AMD_Admin_patchServer::ice_exception(const ::Ice::Exception& ex)
{
    try
    {
        ex.ice_throw();
    }
    catch(const ::IceGrid::DeploymentException& __ex)
    {
        __os()->write(__ex);
        __response(false);
    }
    catch(const ::IceGrid::NodeUnreachableException& __ex)
    {
        __os()->write(__ex);
        __response(false);
    }
    catch(const ::IceGrid::PatchException& __ex)
    {
        __os()->write(__ex);
        __response(false);
    }
    catch(const ::IceGrid::ServerNotExistException& __ex)
    {
        __os()->write(__ex);
        __response(false);
    }
    catch(const ::Ice::Exception& __ex)
    {
        __exception(__ex);
    }
}

void
IceAsync::IceGrid::AMD_Admin_patchServer::ice_exception(const ::std::exception& ex)
{
    __exception(ex);
}

void
IceAsync::IceGrid::AMD_Admin_patchServer::ice_exception()
{
    __exception();
}

void
IceProxy::IceGrid::Admin::addApplication(const ::IceGrid::ApplicationDescriptor& descriptor, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__addApplication_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            __del->addApplication(descriptor, __ctx);
            return;
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
IceProxy::IceGrid::Admin::addApplication_async(const ::IceGrid::AMI_Admin_addApplicationPtr& __cb, const ::IceGrid::ApplicationDescriptor& descriptor)
{
    __cb->__invoke(this, descriptor, 0);
}

void
IceProxy::IceGrid::Admin::addApplication_async(const ::IceGrid::AMI_Admin_addApplicationPtr& __cb, const ::IceGrid::ApplicationDescriptor& descriptor, const ::Ice::Context& __ctx)
{
    __cb->__invoke(this, descriptor, &__ctx);
}

void
IceProxy::IceGrid::Admin::syncApplication(const ::IceGrid::ApplicationDescriptor& descriptor, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__syncApplication_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            __del->syncApplication(descriptor, __ctx);
            return;
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
IceProxy::IceGrid::Admin::syncApplication_async(const ::IceGrid::AMI_Admin_syncApplicationPtr& __cb, const ::IceGrid::ApplicationDescriptor& descriptor)
{
    __cb->__invoke(this, descriptor, 0);
}

void
IceProxy::IceGrid::Admin::syncApplication_async(const ::IceGrid::AMI_Admin_syncApplicationPtr& __cb, const ::IceGrid::ApplicationDescriptor& descriptor, const ::Ice::Context& __ctx)
{
    __cb->__invoke(this, descriptor, &__ctx);
}

void
IceProxy::IceGrid::Admin::updateApplication(const ::IceGrid::ApplicationUpdateDescriptor& descriptor, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__updateApplication_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            __del->updateApplication(descriptor, __ctx);
            return;
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
IceProxy::IceGrid::Admin::updateApplication_async(const ::IceGrid::AMI_Admin_updateApplicationPtr& __cb, const ::IceGrid::ApplicationUpdateDescriptor& descriptor)
{
    __cb->__invoke(this, descriptor, 0);
}

void
IceProxy::IceGrid::Admin::updateApplication_async(const ::IceGrid::AMI_Admin_updateApplicationPtr& __cb, const ::IceGrid::ApplicationUpdateDescriptor& descriptor, const ::Ice::Context& __ctx)
{
    __cb->__invoke(this, descriptor, &__ctx);
}

void
IceProxy::IceGrid::Admin::removeApplication(const ::std::string& name, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__removeApplication_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            __del->removeApplication(name, __ctx);
            return;
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
IceProxy::IceGrid::Admin::removeApplication_async(const ::IceGrid::AMI_Admin_removeApplicationPtr& __cb, const ::std::string& name)
{
    __cb->__invoke(this, name, 0);
}

void
IceProxy::IceGrid::Admin::removeApplication_async(const ::IceGrid::AMI_Admin_removeApplicationPtr& __cb, const ::std::string& name, const ::Ice::Context& __ctx)
{
    __cb->__invoke(this, name, &__ctx);
}

void
IceProxy::IceGrid::Admin::instantiateServer(const ::std::string& application, const ::std::string& node, const ::IceGrid::ServerInstanceDescriptor& desc, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__instantiateServer_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            __del->instantiateServer(application, node, desc, __ctx);
            return;
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
IceProxy::IceGrid::Admin::patchApplication(const ::std::string& name, bool shutdown, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__patchApplication_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            __del->patchApplication(name, shutdown, __ctx);
            return;
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
IceProxy::IceGrid::Admin::patchApplication_async(const ::IceGrid::AMI_Admin_patchApplicationPtr& __cb, const ::std::string& name, bool shutdown)
{
    __cb->__invoke(this, name, shutdown, 0);
}

void
IceProxy::IceGrid::Admin::patchApplication_async(const ::IceGrid::AMI_Admin_patchApplicationPtr& __cb, const ::std::string& name, bool shutdown, const ::Ice::Context& __ctx)
{
    __cb->__invoke(this, name, shutdown, &__ctx);
}

::IceGrid::ApplicationInfo
IceProxy::IceGrid::Admin::getApplicationInfo(const ::std::string& name, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__getApplicationInfo_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            return __del->getApplicationInfo(name, __ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__delBase, __ex, __cnt);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

::IceGrid::ApplicationDescriptor
IceProxy::IceGrid::Admin::getDefaultApplicationDescriptor(const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__getDefaultApplicationDescriptor_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            return __del->getDefaultApplicationDescriptor(__ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__delBase, __ex, __cnt);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

::Ice::StringSeq
IceProxy::IceGrid::Admin::getAllApplicationNames(const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__getAllApplicationNames_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            return __del->getAllApplicationNames(__ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__delBase, __ex, __cnt);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

::IceGrid::ServerInfo
IceProxy::IceGrid::Admin::getServerInfo(const ::std::string& id, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__getServerInfo_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            return __del->getServerInfo(id, __ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__delBase, __ex, __cnt);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

::IceGrid::ServerState
IceProxy::IceGrid::Admin::getServerState(const ::std::string& id, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__getServerState_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            return __del->getServerState(id, __ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__delBase, __ex, __cnt);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

::Ice::Int
IceProxy::IceGrid::Admin::getServerPid(const ::std::string& id, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__getServerPid_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            return __del->getServerPid(id, __ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__delBase, __ex, __cnt);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

void
IceProxy::IceGrid::Admin::enableServer(const ::std::string& id, bool enabled, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__enableServer_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            __del->enableServer(id, enabled, __ctx);
            return;
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__delBase, __ex, __cnt);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

void
IceProxy::IceGrid::Admin::enableServer_async(const ::IceGrid::AMI_Admin_enableServerPtr& __cb, const ::std::string& id, bool enabled)
{
    __cb->__invoke(this, id, enabled, 0);
}

void
IceProxy::IceGrid::Admin::enableServer_async(const ::IceGrid::AMI_Admin_enableServerPtr& __cb, const ::std::string& id, bool enabled, const ::Ice::Context& __ctx)
{
    __cb->__invoke(this, id, enabled, &__ctx);
}

bool
IceProxy::IceGrid::Admin::isServerEnabled(const ::std::string& id, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__isServerEnabled_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            return __del->isServerEnabled(id, __ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__delBase, __ex, __cnt);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

void
IceProxy::IceGrid::Admin::startServer(const ::std::string& id, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__startServer_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            __del->startServer(id, __ctx);
            return;
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
IceProxy::IceGrid::Admin::startServer_async(const ::IceGrid::AMI_Admin_startServerPtr& __cb, const ::std::string& id)
{
    __cb->__invoke(this, id, 0);
}

void
IceProxy::IceGrid::Admin::startServer_async(const ::IceGrid::AMI_Admin_startServerPtr& __cb, const ::std::string& id, const ::Ice::Context& __ctx)
{
    __cb->__invoke(this, id, &__ctx);
}

void
IceProxy::IceGrid::Admin::stopServer(const ::std::string& id, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__stopServer_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            __del->stopServer(id, __ctx);
            return;
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
IceProxy::IceGrid::Admin::stopServer_async(const ::IceGrid::AMI_Admin_stopServerPtr& __cb, const ::std::string& id)
{
    __cb->__invoke(this, id, 0);
}

void
IceProxy::IceGrid::Admin::stopServer_async(const ::IceGrid::AMI_Admin_stopServerPtr& __cb, const ::std::string& id, const ::Ice::Context& __ctx)
{
    __cb->__invoke(this, id, &__ctx);
}

void
IceProxy::IceGrid::Admin::patchServer(const ::std::string& id, bool shutdown, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__patchServer_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            __del->patchServer(id, shutdown, __ctx);
            return;
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
IceProxy::IceGrid::Admin::patchServer_async(const ::IceGrid::AMI_Admin_patchServerPtr& __cb, const ::std::string& id, bool shutdown)
{
    __cb->__invoke(this, id, shutdown, 0);
}

void
IceProxy::IceGrid::Admin::patchServer_async(const ::IceGrid::AMI_Admin_patchServerPtr& __cb, const ::std::string& id, bool shutdown, const ::Ice::Context& __ctx)
{
    __cb->__invoke(this, id, shutdown, &__ctx);
}

void
IceProxy::IceGrid::Admin::sendSignal(const ::std::string& id, const ::std::string& signal, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__sendSignal_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            __del->sendSignal(id, signal, __ctx);
            return;
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
IceProxy::IceGrid::Admin::sendSignal_async(const ::IceGrid::AMI_Admin_sendSignalPtr& __cb, const ::std::string& id, const ::std::string& signal)
{
    __cb->__invoke(this, id, signal, 0);
}

void
IceProxy::IceGrid::Admin::sendSignal_async(const ::IceGrid::AMI_Admin_sendSignalPtr& __cb, const ::std::string& id, const ::std::string& signal, const ::Ice::Context& __ctx)
{
    __cb->__invoke(this, id, signal, &__ctx);
}

void
IceProxy::IceGrid::Admin::writeMessage(const ::std::string& id, const ::std::string& message, ::Ice::Int fd, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__writeMessage_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            __del->writeMessage(id, message, fd, __ctx);
            return;
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
IceProxy::IceGrid::Admin::writeMessage_async(const ::IceGrid::AMI_Admin_writeMessagePtr& __cb, const ::std::string& id, const ::std::string& message, ::Ice::Int fd)
{
    __cb->__invoke(this, id, message, fd, 0);
}

void
IceProxy::IceGrid::Admin::writeMessage_async(const ::IceGrid::AMI_Admin_writeMessagePtr& __cb, const ::std::string& id, const ::std::string& message, ::Ice::Int fd, const ::Ice::Context& __ctx)
{
    __cb->__invoke(this, id, message, fd, &__ctx);
}

::Ice::StringSeq
IceProxy::IceGrid::Admin::getAllServerIds(const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__getAllServerIds_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            return __del->getAllServerIds(__ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__delBase, __ex, __cnt);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

::IceGrid::AdapterInfoSeq
IceProxy::IceGrid::Admin::getAdapterInfo(const ::std::string& id, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__getAdapterInfo_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            return __del->getAdapterInfo(id, __ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__delBase, __ex, __cnt);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

void
IceProxy::IceGrid::Admin::removeAdapter(const ::std::string& adapterId, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__removeAdapter_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            __del->removeAdapter(adapterId, __ctx);
            return;
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
IceProxy::IceGrid::Admin::removeAdapter_async(const ::IceGrid::AMI_Admin_removeAdapterPtr& __cb, const ::std::string& adapterId)
{
    __cb->__invoke(this, adapterId, 0);
}

void
IceProxy::IceGrid::Admin::removeAdapter_async(const ::IceGrid::AMI_Admin_removeAdapterPtr& __cb, const ::std::string& adapterId, const ::Ice::Context& __ctx)
{
    __cb->__invoke(this, adapterId, &__ctx);
}

::Ice::StringSeq
IceProxy::IceGrid::Admin::getAllAdapterIds(const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__getAllAdapterIds_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            return __del->getAllAdapterIds(__ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__delBase, __ex, __cnt);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

void
IceProxy::IceGrid::Admin::addObject(const ::Ice::ObjectPrx& obj, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__addObject_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            __del->addObject(obj, __ctx);
            return;
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
IceProxy::IceGrid::Admin::addObject_async(const ::IceGrid::AMI_Admin_addObjectPtr& __cb, const ::Ice::ObjectPrx& obj)
{
    __cb->__invoke(this, obj, 0);
}

void
IceProxy::IceGrid::Admin::addObject_async(const ::IceGrid::AMI_Admin_addObjectPtr& __cb, const ::Ice::ObjectPrx& obj, const ::Ice::Context& __ctx)
{
    __cb->__invoke(this, obj, &__ctx);
}

void
IceProxy::IceGrid::Admin::updateObject(const ::Ice::ObjectPrx& obj, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__updateObject_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            __del->updateObject(obj, __ctx);
            return;
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
IceProxy::IceGrid::Admin::addObjectWithType(const ::Ice::ObjectPrx& obj, const ::std::string& type, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__addObjectWithType_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            __del->addObjectWithType(obj, type, __ctx);
            return;
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
IceProxy::IceGrid::Admin::addObjectWithType_async(const ::IceGrid::AMI_Admin_addObjectWithTypePtr& __cb, const ::Ice::ObjectPrx& obj, const ::std::string& type)
{
    __cb->__invoke(this, obj, type, 0);
}

void
IceProxy::IceGrid::Admin::addObjectWithType_async(const ::IceGrid::AMI_Admin_addObjectWithTypePtr& __cb, const ::Ice::ObjectPrx& obj, const ::std::string& type, const ::Ice::Context& __ctx)
{
    __cb->__invoke(this, obj, type, &__ctx);
}

void
IceProxy::IceGrid::Admin::removeObject(const ::Ice::Identity& id, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__removeObject_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            __del->removeObject(id, __ctx);
            return;
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
IceProxy::IceGrid::Admin::removeObject_async(const ::IceGrid::AMI_Admin_removeObjectPtr& __cb, const ::Ice::Identity& id)
{
    __cb->__invoke(this, id, 0);
}

void
IceProxy::IceGrid::Admin::removeObject_async(const ::IceGrid::AMI_Admin_removeObjectPtr& __cb, const ::Ice::Identity& id, const ::Ice::Context& __ctx)
{
    __cb->__invoke(this, id, &__ctx);
}

::IceGrid::ObjectInfo
IceProxy::IceGrid::Admin::getObjectInfo(const ::Ice::Identity& id, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__getObjectInfo_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            return __del->getObjectInfo(id, __ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__delBase, __ex, __cnt);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

::IceGrid::ObjectInfoSeq
IceProxy::IceGrid::Admin::getObjectInfosByType(const ::std::string& type, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__getObjectInfosByType_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            return __del->getObjectInfosByType(type, __ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__delBase, __ex, __cnt);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

::IceGrid::ObjectInfoSeq
IceProxy::IceGrid::Admin::getAllObjectInfos(const ::std::string& expr, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__getAllObjectInfos_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            return __del->getAllObjectInfos(expr, __ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__delBase, __ex, __cnt);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

bool
IceProxy::IceGrid::Admin::pingNode(const ::std::string& name, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__pingNode_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            return __del->pingNode(name, __ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__delBase, __ex, __cnt);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

::IceGrid::LoadInfo
IceProxy::IceGrid::Admin::getNodeLoad(const ::std::string& name, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__getNodeLoad_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            return __del->getNodeLoad(name, __ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__delBase, __ex, __cnt);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

void
IceProxy::IceGrid::Admin::getNodeLoad_async(const ::IceGrid::AMI_Admin_getNodeLoadPtr& __cb, const ::std::string& name)
{
    __cb->__invoke(this, name, 0);
}

void
IceProxy::IceGrid::Admin::getNodeLoad_async(const ::IceGrid::AMI_Admin_getNodeLoadPtr& __cb, const ::std::string& name, const ::Ice::Context& __ctx)
{
    __cb->__invoke(this, name, &__ctx);
}

::IceGrid::NodeInfo
IceProxy::IceGrid::Admin::getNodeInfo(const ::std::string& name, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__getNodeInfo_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            return __del->getNodeInfo(name, __ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__delBase, __ex, __cnt);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

void
IceProxy::IceGrid::Admin::shutdownNode(const ::std::string& name, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__shutdownNode_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            __del->shutdownNode(name, __ctx);
            return;
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
IceProxy::IceGrid::Admin::shutdownNode_async(const ::IceGrid::AMI_Admin_shutdownNodePtr& __cb, const ::std::string& name)
{
    __cb->__invoke(this, name, 0);
}

void
IceProxy::IceGrid::Admin::shutdownNode_async(const ::IceGrid::AMI_Admin_shutdownNodePtr& __cb, const ::std::string& name, const ::Ice::Context& __ctx)
{
    __cb->__invoke(this, name, &__ctx);
}

::std::string
IceProxy::IceGrid::Admin::getNodeHostname(const ::std::string& name, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__getNodeHostname_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            return __del->getNodeHostname(name, __ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__delBase, __ex, __cnt);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

::Ice::StringSeq
IceProxy::IceGrid::Admin::getAllNodeNames(const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__getAllNodeNames_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            return __del->getAllNodeNames(__ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__delBase, __ex, __cnt);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

bool
IceProxy::IceGrid::Admin::pingRegistry(const ::std::string& name, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__pingRegistry_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            return __del->pingRegistry(name, __ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__delBase, __ex, __cnt);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

::IceGrid::RegistryInfo
IceProxy::IceGrid::Admin::getRegistryInfo(const ::std::string& name, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__getRegistryInfo_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            return __del->getRegistryInfo(name, __ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__delBase, __ex, __cnt);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

void
IceProxy::IceGrid::Admin::shutdownRegistry(const ::std::string& name, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__shutdownRegistry_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            __del->shutdownRegistry(name, __ctx);
            return;
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__delBase, __ex, __cnt);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

void
IceProxy::IceGrid::Admin::shutdownRegistry_async(const ::IceGrid::AMI_Admin_shutdownRegistryPtr& __cb, const ::std::string& name)
{
    __cb->__invoke(this, name, 0);
}

void
IceProxy::IceGrid::Admin::shutdownRegistry_async(const ::IceGrid::AMI_Admin_shutdownRegistryPtr& __cb, const ::std::string& name, const ::Ice::Context& __ctx)
{
    __cb->__invoke(this, name, &__ctx);
}

::Ice::StringSeq
IceProxy::IceGrid::Admin::getAllRegistryNames(const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__getAllRegistryNames_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            return __del->getAllRegistryNames(__ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__delBase, __ex, __cnt);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

void
IceProxy::IceGrid::Admin::shutdown(const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            __del->shutdown(__ctx);
            return;
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

::Ice::SliceChecksumDict
IceProxy::IceGrid::Admin::getSliceChecksums(const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__Admin__getSliceChecksums_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::Admin* __del = dynamic_cast< ::IceDelegate::IceGrid::Admin*>(__delBase.get());
            return __del->getSliceChecksums(__ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__delBase, __ex, __cnt);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

const ::std::string&
IceProxy::IceGrid::Admin::ice_staticId()
{
    return ::IceGrid::Admin::ice_staticId();
}

::IceInternal::Handle< ::IceDelegateM::Ice::Object>
IceProxy::IceGrid::Admin::__createDelegateM()
{
    return ::IceInternal::Handle< ::IceDelegateM::Ice::Object>(new ::IceDelegateM::IceGrid::Admin);
}

::IceInternal::Handle< ::IceDelegateD::Ice::Object>
IceProxy::IceGrid::Admin::__createDelegateD()
{
    return ::IceInternal::Handle< ::IceDelegateD::Ice::Object>(new ::IceDelegateD::IceGrid::Admin);
}

bool
IceProxy::IceGrid::operator==(const ::IceProxy::IceGrid::Admin& l, const ::IceProxy::IceGrid::Admin& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) == static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::IceGrid::operator!=(const ::IceProxy::IceGrid::Admin& l, const ::IceProxy::IceGrid::Admin& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) != static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::IceGrid::operator<(const ::IceProxy::IceGrid::Admin& l, const ::IceProxy::IceGrid::Admin& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) < static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::IceGrid::operator<=(const ::IceProxy::IceGrid::Admin& l, const ::IceProxy::IceGrid::Admin& r)
{
    return l < r || l == r;
}

bool
IceProxy::IceGrid::operator>(const ::IceProxy::IceGrid::Admin& l, const ::IceProxy::IceGrid::Admin& r)
{
    return !(l < r) && !(l == r);
}

bool
IceProxy::IceGrid::operator>=(const ::IceProxy::IceGrid::Admin& l, const ::IceProxy::IceGrid::Admin& r)
{
    return !(l < r);
}

bool
IceProxy::IceGrid::FileIterator::read(::Ice::Int size, ::Ice::StringSeq& lines, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__FileIterator__read_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::FileIterator* __del = dynamic_cast< ::IceDelegate::IceGrid::FileIterator*>(__delBase.get());
            return __del->read(size, lines, __ctx);
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
IceProxy::IceGrid::FileIterator::destroy(const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::FileIterator* __del = dynamic_cast< ::IceDelegate::IceGrid::FileIterator*>(__delBase.get());
            __del->destroy(__ctx);
            return;
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
IceProxy::IceGrid::FileIterator::ice_staticId()
{
    return ::IceGrid::FileIterator::ice_staticId();
}

::IceInternal::Handle< ::IceDelegateM::Ice::Object>
IceProxy::IceGrid::FileIterator::__createDelegateM()
{
    return ::IceInternal::Handle< ::IceDelegateM::Ice::Object>(new ::IceDelegateM::IceGrid::FileIterator);
}

::IceInternal::Handle< ::IceDelegateD::Ice::Object>
IceProxy::IceGrid::FileIterator::__createDelegateD()
{
    return ::IceInternal::Handle< ::IceDelegateD::Ice::Object>(new ::IceDelegateD::IceGrid::FileIterator);
}

bool
IceProxy::IceGrid::operator==(const ::IceProxy::IceGrid::FileIterator& l, const ::IceProxy::IceGrid::FileIterator& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) == static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::IceGrid::operator!=(const ::IceProxy::IceGrid::FileIterator& l, const ::IceProxy::IceGrid::FileIterator& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) != static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::IceGrid::operator<(const ::IceProxy::IceGrid::FileIterator& l, const ::IceProxy::IceGrid::FileIterator& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) < static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::IceGrid::operator<=(const ::IceProxy::IceGrid::FileIterator& l, const ::IceProxy::IceGrid::FileIterator& r)
{
    return l < r || l == r;
}

bool
IceProxy::IceGrid::operator>(const ::IceProxy::IceGrid::FileIterator& l, const ::IceProxy::IceGrid::FileIterator& r)
{
    return !(l < r) && !(l == r);
}

bool
IceProxy::IceGrid::operator>=(const ::IceProxy::IceGrid::FileIterator& l, const ::IceProxy::IceGrid::FileIterator& r)
{
    return !(l < r);
}

void
IceProxy::IceGrid::AdminSession::keepAlive(const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::AdminSession* __del = dynamic_cast< ::IceDelegate::IceGrid::AdminSession*>(__delBase.get());
            __del->keepAlive(__ctx);
            return;
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__delBase, __ex, __cnt);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

::IceGrid::AdminPrx
IceProxy::IceGrid::AdminSession::getAdmin(const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__AdminSession__getAdmin_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::AdminSession* __del = dynamic_cast< ::IceDelegate::IceGrid::AdminSession*>(__delBase.get());
            return __del->getAdmin(__ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__delBase, __ex, __cnt);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

void
IceProxy::IceGrid::AdminSession::setObservers(const ::IceGrid::RegistryObserverPrx& registryObs, const ::IceGrid::NodeObserverPrx& nodeObs, const ::IceGrid::ApplicationObserverPrx& appObs, const ::IceGrid::AdapterObserverPrx& adptObs, const ::IceGrid::ObjectObserverPrx& objObs, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__AdminSession__setObservers_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::AdminSession* __del = dynamic_cast< ::IceDelegate::IceGrid::AdminSession*>(__delBase.get());
            __del->setObservers(registryObs, nodeObs, appObs, adptObs, objObs, __ctx);
            return;
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__delBase, __ex, __cnt);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

void
IceProxy::IceGrid::AdminSession::setObserversByIdentity(const ::Ice::Identity& registryObs, const ::Ice::Identity& nodeObs, const ::Ice::Identity& appObs, const ::Ice::Identity& adptObs, const ::Ice::Identity& objObs, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__AdminSession__setObserversByIdentity_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::AdminSession* __del = dynamic_cast< ::IceDelegate::IceGrid::AdminSession*>(__delBase.get());
            __del->setObserversByIdentity(registryObs, nodeObs, appObs, adptObs, objObs, __ctx);
            return;
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__delBase, __ex, __cnt);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

::Ice::Int
IceProxy::IceGrid::AdminSession::startUpdate(const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__AdminSession__startUpdate_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::AdminSession* __del = dynamic_cast< ::IceDelegate::IceGrid::AdminSession*>(__delBase.get());
            return __del->startUpdate(__ctx);
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
IceProxy::IceGrid::AdminSession::finishUpdate(const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__AdminSession__finishUpdate_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::AdminSession* __del = dynamic_cast< ::IceDelegate::IceGrid::AdminSession*>(__delBase.get());
            __del->finishUpdate(__ctx);
            return;
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
IceProxy::IceGrid::AdminSession::getReplicaName(const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__AdminSession__getReplicaName_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::AdminSession* __del = dynamic_cast< ::IceDelegate::IceGrid::AdminSession*>(__delBase.get());
            return __del->getReplicaName(__ctx);
        }
        catch(const ::IceInternal::LocalExceptionWrapper& __ex)
        {
            __handleExceptionWrapperRelaxed(__delBase, __ex, __cnt);
        }
        catch(const ::Ice::LocalException& __ex)
        {
            __handleException(__delBase, __ex, __cnt);
        }
    }
}

::IceGrid::FileIteratorPrx
IceProxy::IceGrid::AdminSession::openServerLog(const ::std::string& id, const ::std::string& path, ::Ice::Int count, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__AdminSession__openServerLog_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::AdminSession* __del = dynamic_cast< ::IceDelegate::IceGrid::AdminSession*>(__delBase.get());
            return __del->openServerLog(id, path, count, __ctx);
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

::IceGrid::FileIteratorPrx
IceProxy::IceGrid::AdminSession::openServerStdErr(const ::std::string& id, ::Ice::Int count, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__AdminSession__openServerStdErr_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::AdminSession* __del = dynamic_cast< ::IceDelegate::IceGrid::AdminSession*>(__delBase.get());
            return __del->openServerStdErr(id, count, __ctx);
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

::IceGrid::FileIteratorPrx
IceProxy::IceGrid::AdminSession::openServerStdOut(const ::std::string& id, ::Ice::Int count, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__AdminSession__openServerStdOut_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::AdminSession* __del = dynamic_cast< ::IceDelegate::IceGrid::AdminSession*>(__delBase.get());
            return __del->openServerStdOut(id, count, __ctx);
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

::IceGrid::FileIteratorPrx
IceProxy::IceGrid::AdminSession::openNodeStdErr(const ::std::string& name, ::Ice::Int count, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__AdminSession__openNodeStdErr_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::AdminSession* __del = dynamic_cast< ::IceDelegate::IceGrid::AdminSession*>(__delBase.get());
            return __del->openNodeStdErr(name, count, __ctx);
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

::IceGrid::FileIteratorPrx
IceProxy::IceGrid::AdminSession::openNodeStdOut(const ::std::string& name, ::Ice::Int count, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__AdminSession__openNodeStdOut_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::AdminSession* __del = dynamic_cast< ::IceDelegate::IceGrid::AdminSession*>(__delBase.get());
            return __del->openNodeStdOut(name, count, __ctx);
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

::IceGrid::FileIteratorPrx
IceProxy::IceGrid::AdminSession::openRegistryStdErr(const ::std::string& name, ::Ice::Int count, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__AdminSession__openRegistryStdErr_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::AdminSession* __del = dynamic_cast< ::IceDelegate::IceGrid::AdminSession*>(__delBase.get());
            return __del->openRegistryStdErr(name, count, __ctx);
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

::IceGrid::FileIteratorPrx
IceProxy::IceGrid::AdminSession::openRegistryStdOut(const ::std::string& name, ::Ice::Int count, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __checkTwowayOnly(__IceGrid__AdminSession__openRegistryStdOut_name);
            __delBase = __getDelegate();
            ::IceDelegate::IceGrid::AdminSession* __del = dynamic_cast< ::IceDelegate::IceGrid::AdminSession*>(__delBase.get());
            return __del->openRegistryStdOut(name, count, __ctx);
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
IceProxy::IceGrid::AdminSession::ice_staticId()
{
    return ::IceGrid::AdminSession::ice_staticId();
}

::IceInternal::Handle< ::IceDelegateM::Ice::Object>
IceProxy::IceGrid::AdminSession::__createDelegateM()
{
    return ::IceInternal::Handle< ::IceDelegateM::Ice::Object>(new ::IceDelegateM::IceGrid::AdminSession);
}

::IceInternal::Handle< ::IceDelegateD::Ice::Object>
IceProxy::IceGrid::AdminSession::__createDelegateD()
{
    return ::IceInternal::Handle< ::IceDelegateD::Ice::Object>(new ::IceDelegateD::IceGrid::AdminSession);
}

bool
IceProxy::IceGrid::operator==(const ::IceProxy::IceGrid::AdminSession& l, const ::IceProxy::IceGrid::AdminSession& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) == static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::IceGrid::operator!=(const ::IceProxy::IceGrid::AdminSession& l, const ::IceProxy::IceGrid::AdminSession& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) != static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::IceGrid::operator<(const ::IceProxy::IceGrid::AdminSession& l, const ::IceProxy::IceGrid::AdminSession& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) < static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::IceGrid::operator<=(const ::IceProxy::IceGrid::AdminSession& l, const ::IceProxy::IceGrid::AdminSession& r)
{
    return l < r || l == r;
}

bool
IceProxy::IceGrid::operator>(const ::IceProxy::IceGrid::AdminSession& l, const ::IceProxy::IceGrid::AdminSession& r)
{
    return !(l < r) && !(l == r);
}

bool
IceProxy::IceGrid::operator>=(const ::IceProxy::IceGrid::AdminSession& l, const ::IceProxy::IceGrid::AdminSession& r)
{
    return !(l < r);
}

void
IceDelegateM::IceGrid::Admin::addApplication(const ::IceGrid::ApplicationDescriptor& descriptor, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__addApplication_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        descriptor.__write(__os);
        __os->writePendingObjects();
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
            catch(const ::IceGrid::AccessDeniedException&)
            {
                throw;
            }
            catch(const ::IceGrid::DeploymentException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateM::IceGrid::Admin::syncApplication(const ::IceGrid::ApplicationDescriptor& descriptor, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__syncApplication_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        descriptor.__write(__os);
        __os->writePendingObjects();
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
            catch(const ::IceGrid::AccessDeniedException&)
            {
                throw;
            }
            catch(const ::IceGrid::ApplicationNotExistException&)
            {
                throw;
            }
            catch(const ::IceGrid::DeploymentException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateM::IceGrid::Admin::updateApplication(const ::IceGrid::ApplicationUpdateDescriptor& descriptor, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__updateApplication_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        descriptor.__write(__os);
        __os->writePendingObjects();
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
            catch(const ::IceGrid::AccessDeniedException&)
            {
                throw;
            }
            catch(const ::IceGrid::ApplicationNotExistException&)
            {
                throw;
            }
            catch(const ::IceGrid::DeploymentException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateM::IceGrid::Admin::removeApplication(const ::std::string& name, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__removeApplication_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(name);
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
            catch(const ::IceGrid::AccessDeniedException&)
            {
                throw;
            }
            catch(const ::IceGrid::ApplicationNotExistException&)
            {
                throw;
            }
            catch(const ::IceGrid::DeploymentException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateM::IceGrid::Admin::instantiateServer(const ::std::string& application, const ::std::string& node, const ::IceGrid::ServerInstanceDescriptor& desc, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__instantiateServer_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(application);
        __os->write(node);
        desc.__write(__os);
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
            catch(const ::IceGrid::AccessDeniedException&)
            {
                throw;
            }
            catch(const ::IceGrid::ApplicationNotExistException&)
            {
                throw;
            }
            catch(const ::IceGrid::DeploymentException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateM::IceGrid::Admin::patchApplication(const ::std::string& name, bool shutdown, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__patchApplication_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(name);
        __os->write(shutdown);
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
            catch(const ::IceGrid::ApplicationNotExistException&)
            {
                throw;
            }
            catch(const ::IceGrid::PatchException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::IceGrid::ApplicationInfo
IceDelegateM::IceGrid::Admin::getApplicationInfo(const ::std::string& name, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__getApplicationInfo_name, ::Ice::Nonmutating, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(name);
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
            catch(const ::IceGrid::ApplicationNotExistException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        ::IceGrid::ApplicationInfo __ret;
        __ret.__read(__is);
        __is->readPendingObjects();
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::IceGrid::ApplicationDescriptor
IceDelegateM::IceGrid::Admin::getDefaultApplicationDescriptor(const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__getDefaultApplicationDescriptor_name, ::Ice::Nonmutating, __context, __compress);
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
            catch(const ::IceGrid::DeploymentException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        ::IceGrid::ApplicationDescriptor __ret;
        __ret.__read(__is);
        __is->readPendingObjects();
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::Ice::StringSeq
IceDelegateM::IceGrid::Admin::getAllApplicationNames(const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__getAllApplicationNames_name, ::Ice::Nonmutating, __context, __compress);
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
        ::Ice::StringSeq __ret;
        __is->read(__ret);
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::IceGrid::ServerInfo
IceDelegateM::IceGrid::Admin::getServerInfo(const ::std::string& id, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__getServerInfo_name, ::Ice::Nonmutating, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(id);
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
            catch(const ::IceGrid::ServerNotExistException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        ::IceGrid::ServerInfo __ret;
        __ret.__read(__is);
        __is->readPendingObjects();
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::IceGrid::ServerState
IceDelegateM::IceGrid::Admin::getServerState(const ::std::string& id, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__getServerState_name, ::Ice::Nonmutating, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(id);
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
            catch(const ::IceGrid::DeploymentException&)
            {
                throw;
            }
            catch(const ::IceGrid::NodeUnreachableException&)
            {
                throw;
            }
            catch(const ::IceGrid::ServerNotExistException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        ::IceGrid::ServerState __ret;
        ::IceGrid::__read(__is, __ret);
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::Ice::Int
IceDelegateM::IceGrid::Admin::getServerPid(const ::std::string& id, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__getServerPid_name, ::Ice::Nonmutating, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(id);
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
            catch(const ::IceGrid::DeploymentException&)
            {
                throw;
            }
            catch(const ::IceGrid::NodeUnreachableException&)
            {
                throw;
            }
            catch(const ::IceGrid::ServerNotExistException&)
            {
                throw;
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

void
IceDelegateM::IceGrid::Admin::enableServer(const ::std::string& id, bool enabled, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__enableServer_name, ::Ice::Idempotent, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(id);
        __os->write(enabled);
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
            catch(const ::IceGrid::DeploymentException&)
            {
                throw;
            }
            catch(const ::IceGrid::NodeUnreachableException&)
            {
                throw;
            }
            catch(const ::IceGrid::ServerNotExistException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

bool
IceDelegateM::IceGrid::Admin::isServerEnabled(const ::std::string& id, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__isServerEnabled_name, ::Ice::Nonmutating, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(id);
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
            catch(const ::IceGrid::DeploymentException&)
            {
                throw;
            }
            catch(const ::IceGrid::NodeUnreachableException&)
            {
                throw;
            }
            catch(const ::IceGrid::ServerNotExistException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        bool __ret;
        __is->read(__ret);
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateM::IceGrid::Admin::startServer(const ::std::string& id, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__startServer_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(id);
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
            catch(const ::IceGrid::DeploymentException&)
            {
                throw;
            }
            catch(const ::IceGrid::NodeUnreachableException&)
            {
                throw;
            }
            catch(const ::IceGrid::ServerNotExistException&)
            {
                throw;
            }
            catch(const ::IceGrid::ServerStartException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateM::IceGrid::Admin::stopServer(const ::std::string& id, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__stopServer_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(id);
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
            catch(const ::IceGrid::DeploymentException&)
            {
                throw;
            }
            catch(const ::IceGrid::NodeUnreachableException&)
            {
                throw;
            }
            catch(const ::IceGrid::ServerNotExistException&)
            {
                throw;
            }
            catch(const ::IceGrid::ServerStopException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateM::IceGrid::Admin::patchServer(const ::std::string& id, bool shutdown, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__patchServer_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(id);
        __os->write(shutdown);
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
            catch(const ::IceGrid::DeploymentException&)
            {
                throw;
            }
            catch(const ::IceGrid::NodeUnreachableException&)
            {
                throw;
            }
            catch(const ::IceGrid::PatchException&)
            {
                throw;
            }
            catch(const ::IceGrid::ServerNotExistException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateM::IceGrid::Admin::sendSignal(const ::std::string& id, const ::std::string& signal, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__sendSignal_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(id);
        __os->write(signal);
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
            catch(const ::IceGrid::BadSignalException&)
            {
                throw;
            }
            catch(const ::IceGrid::DeploymentException&)
            {
                throw;
            }
            catch(const ::IceGrid::NodeUnreachableException&)
            {
                throw;
            }
            catch(const ::IceGrid::ServerNotExistException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateM::IceGrid::Admin::writeMessage(const ::std::string& id, const ::std::string& message, ::Ice::Int fd, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__writeMessage_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(id);
        __os->write(message);
        __os->write(fd);
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
            catch(const ::IceGrid::DeploymentException&)
            {
                throw;
            }
            catch(const ::IceGrid::NodeUnreachableException&)
            {
                throw;
            }
            catch(const ::IceGrid::ServerNotExistException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::Ice::StringSeq
IceDelegateM::IceGrid::Admin::getAllServerIds(const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__getAllServerIds_name, ::Ice::Nonmutating, __context, __compress);
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
        ::Ice::StringSeq __ret;
        __is->read(__ret);
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::IceGrid::AdapterInfoSeq
IceDelegateM::IceGrid::Admin::getAdapterInfo(const ::std::string& id, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__getAdapterInfo_name, ::Ice::Nonmutating, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(id);
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
            catch(const ::IceGrid::AdapterNotExistException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        ::IceGrid::AdapterInfoSeq __ret;
        ::IceGrid::__read(__is, __ret, ::IceGrid::__U__AdapterInfoSeq());
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateM::IceGrid::Admin::removeAdapter(const ::std::string& adapterId, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__removeAdapter_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(adapterId);
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
            catch(const ::IceGrid::AdapterNotExistException&)
            {
                throw;
            }
            catch(const ::IceGrid::DeploymentException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::Ice::StringSeq
IceDelegateM::IceGrid::Admin::getAllAdapterIds(const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__getAllAdapterIds_name, ::Ice::Nonmutating, __context, __compress);
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
        ::Ice::StringSeq __ret;
        __is->read(__ret);
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateM::IceGrid::Admin::addObject(const ::Ice::ObjectPrx& obj, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__addObject_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(obj);
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
            catch(const ::IceGrid::DeploymentException&)
            {
                throw;
            }
            catch(const ::IceGrid::ObjectExistsException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateM::IceGrid::Admin::updateObject(const ::Ice::ObjectPrx& obj, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__updateObject_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(obj);
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
            catch(const ::IceGrid::DeploymentException&)
            {
                throw;
            }
            catch(const ::IceGrid::ObjectNotRegisteredException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateM::IceGrid::Admin::addObjectWithType(const ::Ice::ObjectPrx& obj, const ::std::string& type, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__addObjectWithType_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(obj);
        __os->write(type);
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
            catch(const ::IceGrid::DeploymentException&)
            {
                throw;
            }
            catch(const ::IceGrid::ObjectExistsException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateM::IceGrid::Admin::removeObject(const ::Ice::Identity& id, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__removeObject_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        id.__write(__os);
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
            catch(const ::IceGrid::DeploymentException&)
            {
                throw;
            }
            catch(const ::IceGrid::ObjectNotRegisteredException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::IceGrid::ObjectInfo
IceDelegateM::IceGrid::Admin::getObjectInfo(const ::Ice::Identity& id, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__getObjectInfo_name, ::Ice::Nonmutating, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        id.__write(__os);
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
            catch(const ::IceGrid::ObjectNotRegisteredException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        ::IceGrid::ObjectInfo __ret;
        __ret.__read(__is);
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::IceGrid::ObjectInfoSeq
IceDelegateM::IceGrid::Admin::getObjectInfosByType(const ::std::string& type, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__getObjectInfosByType_name, ::Ice::Nonmutating, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(type);
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
        ::IceGrid::ObjectInfoSeq __ret;
        ::IceGrid::__read(__is, __ret, ::IceGrid::__U__ObjectInfoSeq());
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::IceGrid::ObjectInfoSeq
IceDelegateM::IceGrid::Admin::getAllObjectInfos(const ::std::string& expr, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__getAllObjectInfos_name, ::Ice::Nonmutating, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(expr);
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
        ::IceGrid::ObjectInfoSeq __ret;
        ::IceGrid::__read(__is, __ret, ::IceGrid::__U__ObjectInfoSeq());
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

bool
IceDelegateM::IceGrid::Admin::pingNode(const ::std::string& name, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__pingNode_name, ::Ice::Nonmutating, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(name);
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
            catch(const ::IceGrid::NodeNotExistException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        bool __ret;
        __is->read(__ret);
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::IceGrid::LoadInfo
IceDelegateM::IceGrid::Admin::getNodeLoad(const ::std::string& name, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__getNodeLoad_name, ::Ice::Nonmutating, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(name);
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
            catch(const ::IceGrid::NodeNotExistException&)
            {
                throw;
            }
            catch(const ::IceGrid::NodeUnreachableException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        ::IceGrid::LoadInfo __ret;
        __ret.__read(__is);
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::IceGrid::NodeInfo
IceDelegateM::IceGrid::Admin::getNodeInfo(const ::std::string& name, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__getNodeInfo_name, ::Ice::Nonmutating, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(name);
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
            catch(const ::IceGrid::NodeNotExistException&)
            {
                throw;
            }
            catch(const ::IceGrid::NodeUnreachableException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        ::IceGrid::NodeInfo __ret;
        __ret.__read(__is);
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateM::IceGrid::Admin::shutdownNode(const ::std::string& name, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__shutdownNode_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(name);
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
            catch(const ::IceGrid::NodeNotExistException&)
            {
                throw;
            }
            catch(const ::IceGrid::NodeUnreachableException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::std::string
IceDelegateM::IceGrid::Admin::getNodeHostname(const ::std::string& name, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__getNodeHostname_name, ::Ice::Nonmutating, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(name);
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
            catch(const ::IceGrid::NodeNotExistException&)
            {
                throw;
            }
            catch(const ::IceGrid::NodeUnreachableException&)
            {
                throw;
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

::Ice::StringSeq
IceDelegateM::IceGrid::Admin::getAllNodeNames(const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__getAllNodeNames_name, ::Ice::Nonmutating, __context, __compress);
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
        ::Ice::StringSeq __ret;
        __is->read(__ret);
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

bool
IceDelegateM::IceGrid::Admin::pingRegistry(const ::std::string& name, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__pingRegistry_name, ::Ice::Idempotent, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(name);
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
            catch(const ::IceGrid::RegistryNotExistException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        bool __ret;
        __is->read(__ret);
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::IceGrid::RegistryInfo
IceDelegateM::IceGrid::Admin::getRegistryInfo(const ::std::string& name, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__getRegistryInfo_name, ::Ice::Idempotent, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(name);
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
            catch(const ::IceGrid::RegistryNotExistException&)
            {
                throw;
            }
            catch(const ::IceGrid::RegistryUnreachableException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        ::IceGrid::RegistryInfo __ret;
        __ret.__read(__is);
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateM::IceGrid::Admin::shutdownRegistry(const ::std::string& name, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__shutdownRegistry_name, ::Ice::Idempotent, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(name);
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
            catch(const ::IceGrid::RegistryNotExistException&)
            {
                throw;
            }
            catch(const ::IceGrid::RegistryUnreachableException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::Ice::StringSeq
IceDelegateM::IceGrid::Admin::getAllRegistryNames(const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__getAllRegistryNames_name, ::Ice::Idempotent, __context, __compress);
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
        ::Ice::StringSeq __ret;
        __is->read(__ret);
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateM::IceGrid::Admin::shutdown(const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__shutdown_name, ::Ice::Normal, __context, __compress);
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
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::Ice::SliceChecksumDict
IceDelegateM::IceGrid::Admin::getSliceChecksums(const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__Admin__getSliceChecksums_name, ::Ice::Nonmutating, __context, __compress);
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
        ::Ice::SliceChecksumDict __ret;
        ::Ice::__read(__is, __ret, ::Ice::__U__SliceChecksumDict());
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

bool
IceDelegateM::IceGrid::FileIterator::read(::Ice::Int size, ::Ice::StringSeq& lines, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__FileIterator__read_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(size);
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
            catch(const ::IceGrid::FileNotAvailableException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        bool __ret;
        __is->read(lines);
        __is->read(__ret);
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateM::IceGrid::FileIterator::destroy(const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__FileIterator__destroy_name, ::Ice::Normal, __context, __compress);
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
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateM::IceGrid::AdminSession::keepAlive(const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__AdminSession__keepAlive_name, ::Ice::Idempotent, __context, __compress);
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
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::IceGrid::AdminPrx
IceDelegateM::IceGrid::AdminSession::getAdmin(const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__AdminSession__getAdmin_name, ::Ice::Nonmutating, __context, __compress);
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
        ::IceGrid::AdminPrx __ret;
        ::IceGrid::__read(__is, __ret);
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateM::IceGrid::AdminSession::setObservers(const ::IceGrid::RegistryObserverPrx& registryObs, const ::IceGrid::NodeObserverPrx& nodeObs, const ::IceGrid::ApplicationObserverPrx& appObs, const ::IceGrid::AdapterObserverPrx& adptObs, const ::IceGrid::ObjectObserverPrx& objObs, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__AdminSession__setObservers_name, ::Ice::Idempotent, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        ::IceGrid::__write(__os, registryObs);
        ::IceGrid::__write(__os, nodeObs);
        ::IceGrid::__write(__os, appObs);
        ::IceGrid::__write(__os, adptObs);
        ::IceGrid::__write(__os, objObs);
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
            catch(const ::IceGrid::ObserverAlreadyRegisteredException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateM::IceGrid::AdminSession::setObserversByIdentity(const ::Ice::Identity& registryObs, const ::Ice::Identity& nodeObs, const ::Ice::Identity& appObs, const ::Ice::Identity& adptObs, const ::Ice::Identity& objObs, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__AdminSession__setObserversByIdentity_name, ::Ice::Idempotent, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        registryObs.__write(__os);
        nodeObs.__write(__os);
        appObs.__write(__os);
        adptObs.__write(__os);
        objObs.__write(__os);
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
            catch(const ::IceGrid::ObserverAlreadyRegisteredException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::Ice::Int
IceDelegateM::IceGrid::AdminSession::startUpdate(const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__AdminSession__startUpdate_name, ::Ice::Normal, __context, __compress);
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
            catch(const ::IceGrid::AccessDeniedException&)
            {
                throw;
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

void
IceDelegateM::IceGrid::AdminSession::finishUpdate(const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__AdminSession__finishUpdate_name, ::Ice::Normal, __context, __compress);
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
            catch(const ::IceGrid::AccessDeniedException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::std::string
IceDelegateM::IceGrid::AdminSession::getReplicaName(const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__AdminSession__getReplicaName_name, ::Ice::Idempotent, __context, __compress);
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

::IceGrid::FileIteratorPrx
IceDelegateM::IceGrid::AdminSession::openServerLog(const ::std::string& id, const ::std::string& path, ::Ice::Int count, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__AdminSession__openServerLog_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(id);
        __os->write(path);
        __os->write(count);
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
            catch(const ::IceGrid::DeploymentException&)
            {
                throw;
            }
            catch(const ::IceGrid::FileNotAvailableException&)
            {
                throw;
            }
            catch(const ::IceGrid::NodeUnreachableException&)
            {
                throw;
            }
            catch(const ::IceGrid::ServerNotExistException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        ::IceGrid::FileIteratorPrx __ret;
        ::IceGrid::__read(__is, __ret);
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::IceGrid::FileIteratorPrx
IceDelegateM::IceGrid::AdminSession::openServerStdErr(const ::std::string& id, ::Ice::Int count, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__AdminSession__openServerStdErr_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(id);
        __os->write(count);
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
            catch(const ::IceGrid::DeploymentException&)
            {
                throw;
            }
            catch(const ::IceGrid::FileNotAvailableException&)
            {
                throw;
            }
            catch(const ::IceGrid::NodeUnreachableException&)
            {
                throw;
            }
            catch(const ::IceGrid::ServerNotExistException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        ::IceGrid::FileIteratorPrx __ret;
        ::IceGrid::__read(__is, __ret);
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::IceGrid::FileIteratorPrx
IceDelegateM::IceGrid::AdminSession::openServerStdOut(const ::std::string& id, ::Ice::Int count, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__AdminSession__openServerStdOut_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(id);
        __os->write(count);
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
            catch(const ::IceGrid::DeploymentException&)
            {
                throw;
            }
            catch(const ::IceGrid::FileNotAvailableException&)
            {
                throw;
            }
            catch(const ::IceGrid::NodeUnreachableException&)
            {
                throw;
            }
            catch(const ::IceGrid::ServerNotExistException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        ::IceGrid::FileIteratorPrx __ret;
        ::IceGrid::__read(__is, __ret);
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::IceGrid::FileIteratorPrx
IceDelegateM::IceGrid::AdminSession::openNodeStdErr(const ::std::string& name, ::Ice::Int count, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__AdminSession__openNodeStdErr_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(name);
        __os->write(count);
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
            catch(const ::IceGrid::FileNotAvailableException&)
            {
                throw;
            }
            catch(const ::IceGrid::NodeNotExistException&)
            {
                throw;
            }
            catch(const ::IceGrid::NodeUnreachableException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        ::IceGrid::FileIteratorPrx __ret;
        ::IceGrid::__read(__is, __ret);
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::IceGrid::FileIteratorPrx
IceDelegateM::IceGrid::AdminSession::openNodeStdOut(const ::std::string& name, ::Ice::Int count, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__AdminSession__openNodeStdOut_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(name);
        __os->write(count);
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
            catch(const ::IceGrid::FileNotAvailableException&)
            {
                throw;
            }
            catch(const ::IceGrid::NodeNotExistException&)
            {
                throw;
            }
            catch(const ::IceGrid::NodeUnreachableException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        ::IceGrid::FileIteratorPrx __ret;
        ::IceGrid::__read(__is, __ret);
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::IceGrid::FileIteratorPrx
IceDelegateM::IceGrid::AdminSession::openRegistryStdErr(const ::std::string& name, ::Ice::Int count, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__AdminSession__openRegistryStdErr_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(name);
        __os->write(count);
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
            catch(const ::IceGrid::FileNotAvailableException&)
            {
                throw;
            }
            catch(const ::IceGrid::RegistryNotExistException&)
            {
                throw;
            }
            catch(const ::IceGrid::RegistryUnreachableException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        ::IceGrid::FileIteratorPrx __ret;
        ::IceGrid::__read(__is, __ret);
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

::IceGrid::FileIteratorPrx
IceDelegateM::IceGrid::AdminSession::openRegistryStdOut(const ::std::string& name, ::Ice::Int count, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __IceGrid__AdminSession__openRegistryStdOut_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        __os->write(name);
        __os->write(count);
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
            catch(const ::IceGrid::FileNotAvailableException&)
            {
                throw;
            }
            catch(const ::IceGrid::RegistryNotExistException&)
            {
                throw;
            }
            catch(const ::IceGrid::RegistryUnreachableException&)
            {
                throw;
            }
            catch(const ::Ice::UserException& __ex)
            {
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        ::IceGrid::FileIteratorPrx __ret;
        ::IceGrid::__read(__is, __ret);
        return __ret;
    }
    catch(const ::Ice::LocalException& __ex)
    {
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateD::IceGrid::Admin::addApplication(const ::IceGrid::ApplicationDescriptor& descriptor, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__addApplication_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __servant->addApplication(descriptor, __current);
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
        return;
    }
}

void
IceDelegateD::IceGrid::Admin::syncApplication(const ::IceGrid::ApplicationDescriptor& descriptor, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__syncApplication_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __servant->syncApplication(descriptor, __current);
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
        return;
    }
}

void
IceDelegateD::IceGrid::Admin::updateApplication(const ::IceGrid::ApplicationUpdateDescriptor& descriptor, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__updateApplication_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __servant->updateApplication(descriptor, __current);
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
        return;
    }
}

void
IceDelegateD::IceGrid::Admin::removeApplication(const ::std::string& name, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__removeApplication_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __servant->removeApplication(name, __current);
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
        return;
    }
}

void
IceDelegateD::IceGrid::Admin::instantiateServer(const ::std::string& application, const ::std::string& node, const ::IceGrid::ServerInstanceDescriptor& desc, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__instantiateServer_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __servant->instantiateServer(application, node, desc, __current);
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
        return;
    }
}

void
IceDelegateD::IceGrid::Admin::patchApplication(const ::std::string&, bool, const ::Ice::Context*)
{
    throw ::Ice::CollocationOptimizationException(__FILE__, __LINE__);
}

::IceGrid::ApplicationInfo
IceDelegateD::IceGrid::Admin::getApplicationInfo(const ::std::string& name, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__getApplicationInfo_name, ::Ice::Nonmutating, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::IceGrid::ApplicationInfo __ret;
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __ret = __servant->getApplicationInfo(name, __current);
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

::IceGrid::ApplicationDescriptor
IceDelegateD::IceGrid::Admin::getDefaultApplicationDescriptor(const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__getDefaultApplicationDescriptor_name, ::Ice::Nonmutating, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::IceGrid::ApplicationDescriptor __ret;
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __ret = __servant->getDefaultApplicationDescriptor(__current);
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

::Ice::StringSeq
IceDelegateD::IceGrid::Admin::getAllApplicationNames(const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__getAllApplicationNames_name, ::Ice::Nonmutating, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::Ice::StringSeq __ret;
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __ret = __servant->getAllApplicationNames(__current);
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

::IceGrid::ServerInfo
IceDelegateD::IceGrid::Admin::getServerInfo(const ::std::string& id, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__getServerInfo_name, ::Ice::Nonmutating, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::IceGrid::ServerInfo __ret;
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __ret = __servant->getServerInfo(id, __current);
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

::IceGrid::ServerState
IceDelegateD::IceGrid::Admin::getServerState(const ::std::string& id, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__getServerState_name, ::Ice::Nonmutating, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::IceGrid::ServerState __ret;
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __ret = __servant->getServerState(id, __current);
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

::Ice::Int
IceDelegateD::IceGrid::Admin::getServerPid(const ::std::string& id, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__getServerPid_name, ::Ice::Nonmutating, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::Ice::Int __ret;
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __ret = __servant->getServerPid(id, __current);
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

void
IceDelegateD::IceGrid::Admin::enableServer(const ::std::string& id, bool enabled, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__enableServer_name, ::Ice::Idempotent, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __servant->enableServer(id, enabled, __current);
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
        return;
    }
}

bool
IceDelegateD::IceGrid::Admin::isServerEnabled(const ::std::string& id, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__isServerEnabled_name, ::Ice::Nonmutating, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        bool __ret;
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __ret = __servant->isServerEnabled(id, __current);
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

void
IceDelegateD::IceGrid::Admin::startServer(const ::std::string& id, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__startServer_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __servant->startServer(id, __current);
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
        return;
    }
}

void
IceDelegateD::IceGrid::Admin::stopServer(const ::std::string& id, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__stopServer_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __servant->stopServer(id, __current);
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
        return;
    }
}

void
IceDelegateD::IceGrid::Admin::patchServer(const ::std::string&, bool, const ::Ice::Context*)
{
    throw ::Ice::CollocationOptimizationException(__FILE__, __LINE__);
}

void
IceDelegateD::IceGrid::Admin::sendSignal(const ::std::string& id, const ::std::string& signal, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__sendSignal_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __servant->sendSignal(id, signal, __current);
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
        return;
    }
}

void
IceDelegateD::IceGrid::Admin::writeMessage(const ::std::string& id, const ::std::string& message, ::Ice::Int fd, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__writeMessage_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __servant->writeMessage(id, message, fd, __current);
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
        return;
    }
}

::Ice::StringSeq
IceDelegateD::IceGrid::Admin::getAllServerIds(const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__getAllServerIds_name, ::Ice::Nonmutating, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::Ice::StringSeq __ret;
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __ret = __servant->getAllServerIds(__current);
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

::IceGrid::AdapterInfoSeq
IceDelegateD::IceGrid::Admin::getAdapterInfo(const ::std::string& id, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__getAdapterInfo_name, ::Ice::Nonmutating, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::IceGrid::AdapterInfoSeq __ret;
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __ret = __servant->getAdapterInfo(id, __current);
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

void
IceDelegateD::IceGrid::Admin::removeAdapter(const ::std::string& adapterId, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__removeAdapter_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __servant->removeAdapter(adapterId, __current);
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
        return;
    }
}

::Ice::StringSeq
IceDelegateD::IceGrid::Admin::getAllAdapterIds(const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__getAllAdapterIds_name, ::Ice::Nonmutating, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::Ice::StringSeq __ret;
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __ret = __servant->getAllAdapterIds(__current);
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

void
IceDelegateD::IceGrid::Admin::addObject(const ::Ice::ObjectPrx& obj, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__addObject_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __servant->addObject(obj, __current);
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
        return;
    }
}

void
IceDelegateD::IceGrid::Admin::updateObject(const ::Ice::ObjectPrx& obj, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__updateObject_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __servant->updateObject(obj, __current);
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
        return;
    }
}

void
IceDelegateD::IceGrid::Admin::addObjectWithType(const ::Ice::ObjectPrx& obj, const ::std::string& type, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__addObjectWithType_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __servant->addObjectWithType(obj, type, __current);
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
        return;
    }
}

void
IceDelegateD::IceGrid::Admin::removeObject(const ::Ice::Identity& id, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__removeObject_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __servant->removeObject(id, __current);
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
        return;
    }
}

::IceGrid::ObjectInfo
IceDelegateD::IceGrid::Admin::getObjectInfo(const ::Ice::Identity& id, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__getObjectInfo_name, ::Ice::Nonmutating, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::IceGrid::ObjectInfo __ret;
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __ret = __servant->getObjectInfo(id, __current);
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

::IceGrid::ObjectInfoSeq
IceDelegateD::IceGrid::Admin::getObjectInfosByType(const ::std::string& type, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__getObjectInfosByType_name, ::Ice::Nonmutating, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::IceGrid::ObjectInfoSeq __ret;
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __ret = __servant->getObjectInfosByType(type, __current);
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

::IceGrid::ObjectInfoSeq
IceDelegateD::IceGrid::Admin::getAllObjectInfos(const ::std::string& expr, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__getAllObjectInfos_name, ::Ice::Nonmutating, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::IceGrid::ObjectInfoSeq __ret;
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __ret = __servant->getAllObjectInfos(expr, __current);
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

bool
IceDelegateD::IceGrid::Admin::pingNode(const ::std::string& name, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__pingNode_name, ::Ice::Nonmutating, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        bool __ret;
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __ret = __servant->pingNode(name, __current);
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

::IceGrid::LoadInfo
IceDelegateD::IceGrid::Admin::getNodeLoad(const ::std::string& name, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__getNodeLoad_name, ::Ice::Nonmutating, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::IceGrid::LoadInfo __ret;
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __ret = __servant->getNodeLoad(name, __current);
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

::IceGrid::NodeInfo
IceDelegateD::IceGrid::Admin::getNodeInfo(const ::std::string& name, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__getNodeInfo_name, ::Ice::Nonmutating, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::IceGrid::NodeInfo __ret;
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __ret = __servant->getNodeInfo(name, __current);
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

void
IceDelegateD::IceGrid::Admin::shutdownNode(const ::std::string& name, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__shutdownNode_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __servant->shutdownNode(name, __current);
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
        return;
    }
}

::std::string
IceDelegateD::IceGrid::Admin::getNodeHostname(const ::std::string& name, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__getNodeHostname_name, ::Ice::Nonmutating, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::std::string __ret;
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __ret = __servant->getNodeHostname(name, __current);
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

::Ice::StringSeq
IceDelegateD::IceGrid::Admin::getAllNodeNames(const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__getAllNodeNames_name, ::Ice::Nonmutating, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::Ice::StringSeq __ret;
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __ret = __servant->getAllNodeNames(__current);
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

bool
IceDelegateD::IceGrid::Admin::pingRegistry(const ::std::string& name, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__pingRegistry_name, ::Ice::Idempotent, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        bool __ret;
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __ret = __servant->pingRegistry(name, __current);
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

::IceGrid::RegistryInfo
IceDelegateD::IceGrid::Admin::getRegistryInfo(const ::std::string& name, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__getRegistryInfo_name, ::Ice::Idempotent, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::IceGrid::RegistryInfo __ret;
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __ret = __servant->getRegistryInfo(name, __current);
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

void
IceDelegateD::IceGrid::Admin::shutdownRegistry(const ::std::string& name, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__shutdownRegistry_name, ::Ice::Idempotent, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __servant->shutdownRegistry(name, __current);
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
        return;
    }
}

::Ice::StringSeq
IceDelegateD::IceGrid::Admin::getAllRegistryNames(const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__getAllRegistryNames_name, ::Ice::Idempotent, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::Ice::StringSeq __ret;
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __ret = __servant->getAllRegistryNames(__current);
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

void
IceDelegateD::IceGrid::Admin::shutdown(const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__shutdown_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __servant->shutdown(__current);
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
        return;
    }
}

::Ice::SliceChecksumDict
IceDelegateD::IceGrid::Admin::getSliceChecksums(const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__Admin__getSliceChecksums_name, ::Ice::Nonmutating, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::Ice::SliceChecksumDict __ret;
        try
        {
            ::IceGrid::Admin* __servant = dynamic_cast< ::IceGrid::Admin*>(__direct.servant().get());
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
                __ret = __servant->getSliceChecksums(__current);
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

bool
IceDelegateD::IceGrid::FileIterator::read(::Ice::Int size, ::Ice::StringSeq& lines, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__FileIterator__read_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        bool __ret;
        try
        {
            ::IceGrid::FileIterator* __servant = dynamic_cast< ::IceGrid::FileIterator*>(__direct.servant().get());
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
                __ret = __servant->read(size, lines, __current);
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

void
IceDelegateD::IceGrid::FileIterator::destroy(const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__FileIterator__destroy_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        try
        {
            ::IceGrid::FileIterator* __servant = dynamic_cast< ::IceGrid::FileIterator*>(__direct.servant().get());
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
                __servant->destroy(__current);
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
        return;
    }
}

void
IceDelegateD::IceGrid::AdminSession::keepAlive(const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__AdminSession__keepAlive_name, ::Ice::Idempotent, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        try
        {
            ::IceGrid::AdminSession* __servant = dynamic_cast< ::IceGrid::AdminSession*>(__direct.servant().get());
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
                __servant->keepAlive(__current);
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
        return;
    }
}

::IceGrid::AdminPrx
IceDelegateD::IceGrid::AdminSession::getAdmin(const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__AdminSession__getAdmin_name, ::Ice::Nonmutating, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::IceGrid::AdminPrx __ret;
        try
        {
            ::IceGrid::AdminSession* __servant = dynamic_cast< ::IceGrid::AdminSession*>(__direct.servant().get());
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
                __ret = __servant->getAdmin(__current);
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

void
IceDelegateD::IceGrid::AdminSession::setObservers(const ::IceGrid::RegistryObserverPrx& registryObs, const ::IceGrid::NodeObserverPrx& nodeObs, const ::IceGrid::ApplicationObserverPrx& appObs, const ::IceGrid::AdapterObserverPrx& adptObs, const ::IceGrid::ObjectObserverPrx& objObs, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__AdminSession__setObservers_name, ::Ice::Idempotent, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        try
        {
            ::IceGrid::AdminSession* __servant = dynamic_cast< ::IceGrid::AdminSession*>(__direct.servant().get());
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
                __servant->setObservers(registryObs, nodeObs, appObs, adptObs, objObs, __current);
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
        return;
    }
}

void
IceDelegateD::IceGrid::AdminSession::setObserversByIdentity(const ::Ice::Identity& registryObs, const ::Ice::Identity& nodeObs, const ::Ice::Identity& appObs, const ::Ice::Identity& adptObs, const ::Ice::Identity& objObs, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__AdminSession__setObserversByIdentity_name, ::Ice::Idempotent, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        try
        {
            ::IceGrid::AdminSession* __servant = dynamic_cast< ::IceGrid::AdminSession*>(__direct.servant().get());
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
                __servant->setObserversByIdentity(registryObs, nodeObs, appObs, adptObs, objObs, __current);
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
        return;
    }
}

::Ice::Int
IceDelegateD::IceGrid::AdminSession::startUpdate(const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__AdminSession__startUpdate_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::Ice::Int __ret;
        try
        {
            ::IceGrid::AdminSession* __servant = dynamic_cast< ::IceGrid::AdminSession*>(__direct.servant().get());
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
                __ret = __servant->startUpdate(__current);
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

void
IceDelegateD::IceGrid::AdminSession::finishUpdate(const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__AdminSession__finishUpdate_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        try
        {
            ::IceGrid::AdminSession* __servant = dynamic_cast< ::IceGrid::AdminSession*>(__direct.servant().get());
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
                __servant->finishUpdate(__current);
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
        return;
    }
}

::std::string
IceDelegateD::IceGrid::AdminSession::getReplicaName(const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__AdminSession__getReplicaName_name, ::Ice::Idempotent, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::std::string __ret;
        try
        {
            ::IceGrid::AdminSession* __servant = dynamic_cast< ::IceGrid::AdminSession*>(__direct.servant().get());
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
                __ret = __servant->getReplicaName(__current);
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

::IceGrid::FileIteratorPrx
IceDelegateD::IceGrid::AdminSession::openServerLog(const ::std::string& id, const ::std::string& path, ::Ice::Int count, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__AdminSession__openServerLog_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::IceGrid::FileIteratorPrx __ret;
        try
        {
            ::IceGrid::AdminSession* __servant = dynamic_cast< ::IceGrid::AdminSession*>(__direct.servant().get());
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
                __ret = __servant->openServerLog(id, path, count, __current);
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

::IceGrid::FileIteratorPrx
IceDelegateD::IceGrid::AdminSession::openServerStdErr(const ::std::string& id, ::Ice::Int count, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__AdminSession__openServerStdErr_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::IceGrid::FileIteratorPrx __ret;
        try
        {
            ::IceGrid::AdminSession* __servant = dynamic_cast< ::IceGrid::AdminSession*>(__direct.servant().get());
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
                __ret = __servant->openServerStdErr(id, count, __current);
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

::IceGrid::FileIteratorPrx
IceDelegateD::IceGrid::AdminSession::openServerStdOut(const ::std::string& id, ::Ice::Int count, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__AdminSession__openServerStdOut_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::IceGrid::FileIteratorPrx __ret;
        try
        {
            ::IceGrid::AdminSession* __servant = dynamic_cast< ::IceGrid::AdminSession*>(__direct.servant().get());
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
                __ret = __servant->openServerStdOut(id, count, __current);
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

::IceGrid::FileIteratorPrx
IceDelegateD::IceGrid::AdminSession::openNodeStdErr(const ::std::string& name, ::Ice::Int count, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__AdminSession__openNodeStdErr_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::IceGrid::FileIteratorPrx __ret;
        try
        {
            ::IceGrid::AdminSession* __servant = dynamic_cast< ::IceGrid::AdminSession*>(__direct.servant().get());
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
                __ret = __servant->openNodeStdErr(name, count, __current);
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

::IceGrid::FileIteratorPrx
IceDelegateD::IceGrid::AdminSession::openNodeStdOut(const ::std::string& name, ::Ice::Int count, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__AdminSession__openNodeStdOut_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::IceGrid::FileIteratorPrx __ret;
        try
        {
            ::IceGrid::AdminSession* __servant = dynamic_cast< ::IceGrid::AdminSession*>(__direct.servant().get());
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
                __ret = __servant->openNodeStdOut(name, count, __current);
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

::IceGrid::FileIteratorPrx
IceDelegateD::IceGrid::AdminSession::openRegistryStdErr(const ::std::string& name, ::Ice::Int count, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__AdminSession__openRegistryStdErr_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::IceGrid::FileIteratorPrx __ret;
        try
        {
            ::IceGrid::AdminSession* __servant = dynamic_cast< ::IceGrid::AdminSession*>(__direct.servant().get());
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
                __ret = __servant->openRegistryStdErr(name, count, __current);
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

::IceGrid::FileIteratorPrx
IceDelegateD::IceGrid::AdminSession::openRegistryStdOut(const ::std::string& name, ::Ice::Int count, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __IceGrid__AdminSession__openRegistryStdOut_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        ::IceGrid::FileIteratorPrx __ret;
        try
        {
            ::IceGrid::AdminSession* __servant = dynamic_cast< ::IceGrid::AdminSession*>(__direct.servant().get());
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
                __ret = __servant->openRegistryStdOut(name, count, __current);
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
IceGrid::Admin::ice_clone() const
{
    throw ::Ice::CloneNotImplementedException(__FILE__, __LINE__);
    return 0; // to avoid a warning with some compilers
}

static const ::std::string __IceGrid__Admin_ids[2] =
{
    "::Ice::Object",
    "::IceGrid::Admin"
};

bool
IceGrid::Admin::ice_isA(const ::std::string& _s, const ::Ice::Current&) const
{
    return ::std::binary_search(__IceGrid__Admin_ids, __IceGrid__Admin_ids + 2, _s);
}

::std::vector< ::std::string>
IceGrid::Admin::ice_ids(const ::Ice::Current&) const
{
    return ::std::vector< ::std::string>(&__IceGrid__Admin_ids[0], &__IceGrid__Admin_ids[2]);
}

const ::std::string&
IceGrid::Admin::ice_id(const ::Ice::Current&) const
{
    return __IceGrid__Admin_ids[1];
}

const ::std::string&
IceGrid::Admin::ice_staticId()
{
    return __IceGrid__Admin_ids[1];
}

::IceInternal::DispatchStatus
IceGrid::Admin::___addApplication(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::IceGrid::ApplicationDescriptor descriptor;
    descriptor.__read(__is);
    __is->readPendingObjects();
    try
    {
        addApplication(descriptor, __current);
    }
    catch(const ::IceGrid::AccessDeniedException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::DeploymentException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___syncApplication(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::IceGrid::ApplicationDescriptor descriptor;
    descriptor.__read(__is);
    __is->readPendingObjects();
    try
    {
        syncApplication(descriptor, __current);
    }
    catch(const ::IceGrid::AccessDeniedException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::ApplicationNotExistException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::DeploymentException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___updateApplication(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::IceGrid::ApplicationUpdateDescriptor descriptor;
    descriptor.__read(__is);
    __is->readPendingObjects();
    try
    {
        updateApplication(descriptor, __current);
    }
    catch(const ::IceGrid::AccessDeniedException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::ApplicationNotExistException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::DeploymentException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___removeApplication(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string name;
    __is->read(name);
    try
    {
        removeApplication(name, __current);
    }
    catch(const ::IceGrid::AccessDeniedException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::ApplicationNotExistException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::DeploymentException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___instantiateServer(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string application;
    ::std::string node;
    ::IceGrid::ServerInstanceDescriptor desc;
    __is->read(application);
    __is->read(node);
    desc.__read(__is);
    try
    {
        instantiateServer(application, node, desc, __current);
    }
    catch(const ::IceGrid::AccessDeniedException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::ApplicationNotExistException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::DeploymentException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___patchApplication(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::std::string name;
    bool shutdown;
    __is->read(name);
    __is->read(shutdown);
    ::IceGrid::AMD_Admin_patchApplicationPtr __cb = new IceAsync::IceGrid::AMD_Admin_patchApplication(__inS);
    try
    {
        patchApplication_async(__cb, name, shutdown, __current);
    }
    catch(const ::Ice::Exception& __ex)
    {
        __cb->ice_exception(__ex);
    }
    catch(const ::std::exception& __ex)
    {
        __cb->ice_exception(__ex);
    }
    catch(...)
    {
        __cb->ice_exception();
    }
    return ::IceInternal::DispatchAsync;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___getApplicationInfo(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Idempotent, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string name;
    __is->read(name);
    try
    {
        ::IceGrid::ApplicationInfo __ret = getApplicationInfo(name, __current);
        __ret.__write(__os);
        __os->writePendingObjects();
    }
    catch(const ::IceGrid::ApplicationNotExistException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___getDefaultApplicationDescriptor(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Idempotent, __current.mode);
    ::IceInternal::BasicStream* __os = __inS.os();
    try
    {
        ::IceGrid::ApplicationDescriptor __ret = getDefaultApplicationDescriptor(__current);
        __ret.__write(__os);
        __os->writePendingObjects();
    }
    catch(const ::IceGrid::DeploymentException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___getAllApplicationNames(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Idempotent, __current.mode);
    ::IceInternal::BasicStream* __os = __inS.os();
    ::Ice::StringSeq __ret = getAllApplicationNames(__current);
    if(__ret.size() == 0)
    {
        __os->writeSize(0);
    }
    else
    {
        __os->write(&__ret[0], &__ret[0] + __ret.size());
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___getServerInfo(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Idempotent, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string id;
    __is->read(id);
    try
    {
        ::IceGrid::ServerInfo __ret = getServerInfo(id, __current);
        __ret.__write(__os);
        __os->writePendingObjects();
    }
    catch(const ::IceGrid::ServerNotExistException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___getServerState(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Idempotent, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string id;
    __is->read(id);
    try
    {
        ::IceGrid::ServerState __ret = getServerState(id, __current);
        ::IceGrid::__write(__os, __ret);
    }
    catch(const ::IceGrid::DeploymentException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::NodeUnreachableException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::ServerNotExistException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___getServerPid(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Idempotent, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string id;
    __is->read(id);
    try
    {
        ::Ice::Int __ret = getServerPid(id, __current);
        __os->write(__ret);
    }
    catch(const ::IceGrid::DeploymentException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::NodeUnreachableException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::ServerNotExistException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___enableServer(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Idempotent, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string id;
    bool enabled;
    __is->read(id);
    __is->read(enabled);
    try
    {
        enableServer(id, enabled, __current);
    }
    catch(const ::IceGrid::DeploymentException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::NodeUnreachableException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::ServerNotExistException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___isServerEnabled(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Idempotent, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string id;
    __is->read(id);
    try
    {
        bool __ret = isServerEnabled(id, __current);
        __os->write(__ret);
    }
    catch(const ::IceGrid::DeploymentException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::NodeUnreachableException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::ServerNotExistException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___startServer(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string id;
    __is->read(id);
    try
    {
        startServer(id, __current);
    }
    catch(const ::IceGrid::DeploymentException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::NodeUnreachableException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::ServerNotExistException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::ServerStartException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___stopServer(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string id;
    __is->read(id);
    try
    {
        stopServer(id, __current);
    }
    catch(const ::IceGrid::DeploymentException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::NodeUnreachableException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::ServerNotExistException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::ServerStopException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___patchServer(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::std::string id;
    bool shutdown;
    __is->read(id);
    __is->read(shutdown);
    ::IceGrid::AMD_Admin_patchServerPtr __cb = new IceAsync::IceGrid::AMD_Admin_patchServer(__inS);
    try
    {
        patchServer_async(__cb, id, shutdown, __current);
    }
    catch(const ::Ice::Exception& __ex)
    {
        __cb->ice_exception(__ex);
    }
    catch(const ::std::exception& __ex)
    {
        __cb->ice_exception(__ex);
    }
    catch(...)
    {
        __cb->ice_exception();
    }
    return ::IceInternal::DispatchAsync;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___sendSignal(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string id;
    ::std::string signal;
    __is->read(id);
    __is->read(signal);
    try
    {
        sendSignal(id, signal, __current);
    }
    catch(const ::IceGrid::BadSignalException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::DeploymentException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::NodeUnreachableException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::ServerNotExistException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___writeMessage(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string id;
    ::std::string message;
    ::Ice::Int fd;
    __is->read(id);
    __is->read(message);
    __is->read(fd);
    try
    {
        writeMessage(id, message, fd, __current);
    }
    catch(const ::IceGrid::DeploymentException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::NodeUnreachableException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::ServerNotExistException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___getAllServerIds(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Idempotent, __current.mode);
    ::IceInternal::BasicStream* __os = __inS.os();
    ::Ice::StringSeq __ret = getAllServerIds(__current);
    if(__ret.size() == 0)
    {
        __os->writeSize(0);
    }
    else
    {
        __os->write(&__ret[0], &__ret[0] + __ret.size());
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___getAdapterInfo(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Idempotent, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string id;
    __is->read(id);
    try
    {
        ::IceGrid::AdapterInfoSeq __ret = getAdapterInfo(id, __current);
        if(__ret.size() == 0)
        {
            __os->writeSize(0);
        }
        else
        {
            ::IceGrid::__write(__os, &__ret[0], &__ret[0] + __ret.size(), ::IceGrid::__U__AdapterInfoSeq());
        }
    }
    catch(const ::IceGrid::AdapterNotExistException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___removeAdapter(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string adapterId;
    __is->read(adapterId);
    try
    {
        removeAdapter(adapterId, __current);
    }
    catch(const ::IceGrid::AdapterNotExistException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::DeploymentException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___getAllAdapterIds(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Idempotent, __current.mode);
    ::IceInternal::BasicStream* __os = __inS.os();
    ::Ice::StringSeq __ret = getAllAdapterIds(__current);
    if(__ret.size() == 0)
    {
        __os->writeSize(0);
    }
    else
    {
        __os->write(&__ret[0], &__ret[0] + __ret.size());
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___addObject(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::Ice::ObjectPrx obj;
    __is->read(obj);
    try
    {
        addObject(obj, __current);
    }
    catch(const ::IceGrid::DeploymentException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::ObjectExistsException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___updateObject(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::Ice::ObjectPrx obj;
    __is->read(obj);
    try
    {
        updateObject(obj, __current);
    }
    catch(const ::IceGrid::DeploymentException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::ObjectNotRegisteredException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___addObjectWithType(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::Ice::ObjectPrx obj;
    ::std::string type;
    __is->read(obj);
    __is->read(type);
    try
    {
        addObjectWithType(obj, type, __current);
    }
    catch(const ::IceGrid::DeploymentException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::ObjectExistsException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___removeObject(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::Ice::Identity id;
    id.__read(__is);
    try
    {
        removeObject(id, __current);
    }
    catch(const ::IceGrid::DeploymentException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::ObjectNotRegisteredException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___getObjectInfo(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Idempotent, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::Ice::Identity id;
    id.__read(__is);
    try
    {
        ::IceGrid::ObjectInfo __ret = getObjectInfo(id, __current);
        __ret.__write(__os);
    }
    catch(const ::IceGrid::ObjectNotRegisteredException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___getObjectInfosByType(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Idempotent, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string type;
    __is->read(type);
    ::IceGrid::ObjectInfoSeq __ret = getObjectInfosByType(type, __current);
    if(__ret.size() == 0)
    {
        __os->writeSize(0);
    }
    else
    {
        ::IceGrid::__write(__os, &__ret[0], &__ret[0] + __ret.size(), ::IceGrid::__U__ObjectInfoSeq());
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___getAllObjectInfos(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Idempotent, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string expr;
    __is->read(expr);
    ::IceGrid::ObjectInfoSeq __ret = getAllObjectInfos(expr, __current);
    if(__ret.size() == 0)
    {
        __os->writeSize(0);
    }
    else
    {
        ::IceGrid::__write(__os, &__ret[0], &__ret[0] + __ret.size(), ::IceGrid::__U__ObjectInfoSeq());
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___pingNode(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Idempotent, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string name;
    __is->read(name);
    try
    {
        bool __ret = pingNode(name, __current);
        __os->write(__ret);
    }
    catch(const ::IceGrid::NodeNotExistException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___getNodeLoad(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Idempotent, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string name;
    __is->read(name);
    try
    {
        ::IceGrid::LoadInfo __ret = getNodeLoad(name, __current);
        __ret.__write(__os);
    }
    catch(const ::IceGrid::NodeNotExistException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::NodeUnreachableException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___getNodeInfo(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Idempotent, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string name;
    __is->read(name);
    try
    {
        ::IceGrid::NodeInfo __ret = getNodeInfo(name, __current);
        __ret.__write(__os);
    }
    catch(const ::IceGrid::NodeNotExistException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::NodeUnreachableException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___shutdownNode(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string name;
    __is->read(name);
    try
    {
        shutdownNode(name, __current);
    }
    catch(const ::IceGrid::NodeNotExistException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::NodeUnreachableException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___getNodeHostname(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Idempotent, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string name;
    __is->read(name);
    try
    {
        ::std::string __ret = getNodeHostname(name, __current);
        __os->write(__ret);
    }
    catch(const ::IceGrid::NodeNotExistException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::NodeUnreachableException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___getAllNodeNames(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Idempotent, __current.mode);
    ::IceInternal::BasicStream* __os = __inS.os();
    ::Ice::StringSeq __ret = getAllNodeNames(__current);
    if(__ret.size() == 0)
    {
        __os->writeSize(0);
    }
    else
    {
        __os->write(&__ret[0], &__ret[0] + __ret.size());
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___pingRegistry(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Idempotent, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string name;
    __is->read(name);
    try
    {
        bool __ret = pingRegistry(name, __current);
        __os->write(__ret);
    }
    catch(const ::IceGrid::RegistryNotExistException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___getRegistryInfo(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Idempotent, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string name;
    __is->read(name);
    try
    {
        ::IceGrid::RegistryInfo __ret = getRegistryInfo(name, __current);
        __ret.__write(__os);
    }
    catch(const ::IceGrid::RegistryNotExistException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::RegistryUnreachableException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___shutdownRegistry(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Idempotent, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string name;
    __is->read(name);
    try
    {
        shutdownRegistry(name, __current);
    }
    catch(const ::IceGrid::RegistryNotExistException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::RegistryUnreachableException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___getAllRegistryNames(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Idempotent, __current.mode);
    ::IceInternal::BasicStream* __os = __inS.os();
    ::Ice::StringSeq __ret = getAllRegistryNames(__current);
    if(__ret.size() == 0)
    {
        __os->writeSize(0);
    }
    else
    {
        __os->write(&__ret[0], &__ret[0] + __ret.size());
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___shutdown(::IceInternal::Incoming&, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    shutdown(__current);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::Admin::___getSliceChecksums(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Idempotent, __current.mode);
    ::IceInternal::BasicStream* __os = __inS.os();
    ::Ice::SliceChecksumDict __ret = getSliceChecksums(__current);
    ::Ice::__write(__os, __ret, ::Ice::__U__SliceChecksumDict());
    return ::IceInternal::DispatchOK;
}

static ::std::string __IceGrid__Admin_all[] =
{
    "addApplication",
    "addObject",
    "addObjectWithType",
    "enableServer",
    "getAdapterInfo",
    "getAllAdapterIds",
    "getAllApplicationNames",
    "getAllNodeNames",
    "getAllObjectInfos",
    "getAllRegistryNames",
    "getAllServerIds",
    "getApplicationInfo",
    "getDefaultApplicationDescriptor",
    "getNodeHostname",
    "getNodeInfo",
    "getNodeLoad",
    "getObjectInfo",
    "getObjectInfosByType",
    "getRegistryInfo",
    "getServerInfo",
    "getServerPid",
    "getServerState",
    "getSliceChecksums",
    "ice_id",
    "ice_ids",
    "ice_isA",
    "ice_ping",
    "instantiateServer",
    "isServerEnabled",
    "patchApplication",
    "patchServer",
    "pingNode",
    "pingRegistry",
    "removeAdapter",
    "removeApplication",
    "removeObject",
    "sendSignal",
    "shutdown",
    "shutdownNode",
    "shutdownRegistry",
    "startServer",
    "stopServer",
    "syncApplication",
    "updateApplication",
    "updateObject",
    "writeMessage"
};

::IceInternal::DispatchStatus
IceGrid::Admin::__dispatch(::IceInternal::Incoming& in, const ::Ice::Current& current)
{
    ::std::pair< ::std::string*, ::std::string*> r = ::std::equal_range(__IceGrid__Admin_all, __IceGrid__Admin_all + 46, current.operation);
    if(r.first == r.second)
    {
        return ::IceInternal::DispatchOperationNotExist;
    }

    switch(r.first - __IceGrid__Admin_all)
    {
        case 0:
        {
            return ___addApplication(in, current);
        }
        case 1:
        {
            return ___addObject(in, current);
        }
        case 2:
        {
            return ___addObjectWithType(in, current);
        }
        case 3:
        {
            return ___enableServer(in, current);
        }
        case 4:
        {
            return ___getAdapterInfo(in, current);
        }
        case 5:
        {
            return ___getAllAdapterIds(in, current);
        }
        case 6:
        {
            return ___getAllApplicationNames(in, current);
        }
        case 7:
        {
            return ___getAllNodeNames(in, current);
        }
        case 8:
        {
            return ___getAllObjectInfos(in, current);
        }
        case 9:
        {
            return ___getAllRegistryNames(in, current);
        }
        case 10:
        {
            return ___getAllServerIds(in, current);
        }
        case 11:
        {
            return ___getApplicationInfo(in, current);
        }
        case 12:
        {
            return ___getDefaultApplicationDescriptor(in, current);
        }
        case 13:
        {
            return ___getNodeHostname(in, current);
        }
        case 14:
        {
            return ___getNodeInfo(in, current);
        }
        case 15:
        {
            return ___getNodeLoad(in, current);
        }
        case 16:
        {
            return ___getObjectInfo(in, current);
        }
        case 17:
        {
            return ___getObjectInfosByType(in, current);
        }
        case 18:
        {
            return ___getRegistryInfo(in, current);
        }
        case 19:
        {
            return ___getServerInfo(in, current);
        }
        case 20:
        {
            return ___getServerPid(in, current);
        }
        case 21:
        {
            return ___getServerState(in, current);
        }
        case 22:
        {
            return ___getSliceChecksums(in, current);
        }
        case 23:
        {
            return ___ice_id(in, current);
        }
        case 24:
        {
            return ___ice_ids(in, current);
        }
        case 25:
        {
            return ___ice_isA(in, current);
        }
        case 26:
        {
            return ___ice_ping(in, current);
        }
        case 27:
        {
            return ___instantiateServer(in, current);
        }
        case 28:
        {
            return ___isServerEnabled(in, current);
        }
        case 29:
        {
            return ___patchApplication(in, current);
        }
        case 30:
        {
            return ___patchServer(in, current);
        }
        case 31:
        {
            return ___pingNode(in, current);
        }
        case 32:
        {
            return ___pingRegistry(in, current);
        }
        case 33:
        {
            return ___removeAdapter(in, current);
        }
        case 34:
        {
            return ___removeApplication(in, current);
        }
        case 35:
        {
            return ___removeObject(in, current);
        }
        case 36:
        {
            return ___sendSignal(in, current);
        }
        case 37:
        {
            return ___shutdown(in, current);
        }
        case 38:
        {
            return ___shutdownNode(in, current);
        }
        case 39:
        {
            return ___shutdownRegistry(in, current);
        }
        case 40:
        {
            return ___startServer(in, current);
        }
        case 41:
        {
            return ___stopServer(in, current);
        }
        case 42:
        {
            return ___syncApplication(in, current);
        }
        case 43:
        {
            return ___updateApplication(in, current);
        }
        case 44:
        {
            return ___updateObject(in, current);
        }
        case 45:
        {
            return ___writeMessage(in, current);
        }
    }

    assert(false);
    return ::IceInternal::DispatchOperationNotExist;
}

void
IceGrid::Admin::__write(::IceInternal::BasicStream* __os) const
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
IceGrid::Admin::__read(::IceInternal::BasicStream* __is, bool __rid)
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
IceGrid::Admin::__write(const ::Ice::OutputStreamPtr&) const
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type IceGrid::Admin was not generated with stream support";
    throw ex;
}

void
IceGrid::Admin::__read(const ::Ice::InputStreamPtr&, bool)
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type IceGrid::Admin was not generated with stream support";
    throw ex;
}

void ICE_GRID_API 
IceGrid::__patch__AdminPtr(void* __addr, ::Ice::ObjectPtr& v)
{
    ::IceGrid::AdminPtr* p = static_cast< ::IceGrid::AdminPtr*>(__addr);
    assert(p);
    *p = ::IceGrid::AdminPtr::dynamicCast(v);
    if(v && !*p)
    {
        ::Ice::UnexpectedObjectException e(__FILE__, __LINE__);
        e.type = v->ice_id();
        e.expectedType = ::IceGrid::Admin::ice_staticId();
        throw e;
    }
}

bool
IceGrid::operator==(const ::IceGrid::Admin& l, const ::IceGrid::Admin& r)
{
    return static_cast<const ::Ice::Object&>(l) == static_cast<const ::Ice::Object&>(r);
}

bool
IceGrid::operator!=(const ::IceGrid::Admin& l, const ::IceGrid::Admin& r)
{
    return static_cast<const ::Ice::Object&>(l) != static_cast<const ::Ice::Object&>(r);
}

bool
IceGrid::operator<(const ::IceGrid::Admin& l, const ::IceGrid::Admin& r)
{
    return static_cast<const ::Ice::Object&>(l) < static_cast<const ::Ice::Object&>(r);
}

bool
IceGrid::operator<=(const ::IceGrid::Admin& l, const ::IceGrid::Admin& r)
{
    return l < r || l == r;
}

bool
IceGrid::operator>(const ::IceGrid::Admin& l, const ::IceGrid::Admin& r)
{
    return !(l < r) && !(l == r);
}

bool
IceGrid::operator>=(const ::IceGrid::Admin& l, const ::IceGrid::Admin& r)
{
    return !(l < r);
}

::Ice::ObjectPtr
IceGrid::FileIterator::ice_clone() const
{
    throw ::Ice::CloneNotImplementedException(__FILE__, __LINE__);
    return 0; // to avoid a warning with some compilers
}

static const ::std::string __IceGrid__FileIterator_ids[2] =
{
    "::Ice::Object",
    "::IceGrid::FileIterator"
};

bool
IceGrid::FileIterator::ice_isA(const ::std::string& _s, const ::Ice::Current&) const
{
    return ::std::binary_search(__IceGrid__FileIterator_ids, __IceGrid__FileIterator_ids + 2, _s);
}

::std::vector< ::std::string>
IceGrid::FileIterator::ice_ids(const ::Ice::Current&) const
{
    return ::std::vector< ::std::string>(&__IceGrid__FileIterator_ids[0], &__IceGrid__FileIterator_ids[2]);
}

const ::std::string&
IceGrid::FileIterator::ice_id(const ::Ice::Current&) const
{
    return __IceGrid__FileIterator_ids[1];
}

const ::std::string&
IceGrid::FileIterator::ice_staticId()
{
    return __IceGrid__FileIterator_ids[1];
}

::IceInternal::DispatchStatus
IceGrid::FileIterator::___read(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::Ice::Int size;
    __is->read(size);
    ::Ice::StringSeq lines;
    try
    {
        bool __ret = read(size, lines, __current);
        if(lines.size() == 0)
        {
            __os->writeSize(0);
        }
        else
        {
            __os->write(&lines[0], &lines[0] + lines.size());
        }
        __os->write(__ret);
    }
    catch(const ::IceGrid::FileNotAvailableException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::FileIterator::___destroy(::IceInternal::Incoming&, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    destroy(__current);
    return ::IceInternal::DispatchOK;
}

static ::std::string __IceGrid__FileIterator_all[] =
{
    "destroy",
    "ice_id",
    "ice_ids",
    "ice_isA",
    "ice_ping",
    "read"
};

::IceInternal::DispatchStatus
IceGrid::FileIterator::__dispatch(::IceInternal::Incoming& in, const ::Ice::Current& current)
{
    ::std::pair< ::std::string*, ::std::string*> r = ::std::equal_range(__IceGrid__FileIterator_all, __IceGrid__FileIterator_all + 6, current.operation);
    if(r.first == r.second)
    {
        return ::IceInternal::DispatchOperationNotExist;
    }

    switch(r.first - __IceGrid__FileIterator_all)
    {
        case 0:
        {
            return ___destroy(in, current);
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
        case 5:
        {
            return ___read(in, current);
        }
    }

    assert(false);
    return ::IceInternal::DispatchOperationNotExist;
}

void
IceGrid::FileIterator::__write(::IceInternal::BasicStream* __os) const
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
IceGrid::FileIterator::__read(::IceInternal::BasicStream* __is, bool __rid)
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
IceGrid::FileIterator::__write(const ::Ice::OutputStreamPtr&) const
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type IceGrid::FileIterator was not generated with stream support";
    throw ex;
}

void
IceGrid::FileIterator::__read(const ::Ice::InputStreamPtr&, bool)
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type IceGrid::FileIterator was not generated with stream support";
    throw ex;
}

void ICE_GRID_API 
IceGrid::__patch__FileIteratorPtr(void* __addr, ::Ice::ObjectPtr& v)
{
    ::IceGrid::FileIteratorPtr* p = static_cast< ::IceGrid::FileIteratorPtr*>(__addr);
    assert(p);
    *p = ::IceGrid::FileIteratorPtr::dynamicCast(v);
    if(v && !*p)
    {
        ::Ice::UnexpectedObjectException e(__FILE__, __LINE__);
        e.type = v->ice_id();
        e.expectedType = ::IceGrid::FileIterator::ice_staticId();
        throw e;
    }
}

bool
IceGrid::operator==(const ::IceGrid::FileIterator& l, const ::IceGrid::FileIterator& r)
{
    return static_cast<const ::Ice::Object&>(l) == static_cast<const ::Ice::Object&>(r);
}

bool
IceGrid::operator!=(const ::IceGrid::FileIterator& l, const ::IceGrid::FileIterator& r)
{
    return static_cast<const ::Ice::Object&>(l) != static_cast<const ::Ice::Object&>(r);
}

bool
IceGrid::operator<(const ::IceGrid::FileIterator& l, const ::IceGrid::FileIterator& r)
{
    return static_cast<const ::Ice::Object&>(l) < static_cast<const ::Ice::Object&>(r);
}

bool
IceGrid::operator<=(const ::IceGrid::FileIterator& l, const ::IceGrid::FileIterator& r)
{
    return l < r || l == r;
}

bool
IceGrid::operator>(const ::IceGrid::FileIterator& l, const ::IceGrid::FileIterator& r)
{
    return !(l < r) && !(l == r);
}

bool
IceGrid::operator>=(const ::IceGrid::FileIterator& l, const ::IceGrid::FileIterator& r)
{
    return !(l < r);
}

::Ice::ObjectPtr
IceGrid::AdminSession::ice_clone() const
{
    throw ::Ice::CloneNotImplementedException(__FILE__, __LINE__);
    return 0; // to avoid a warning with some compilers
}

static const ::std::string __IceGrid__AdminSession_ids[3] =
{
    "::Glacier2::Session",
    "::Ice::Object",
    "::IceGrid::AdminSession"
};

bool
IceGrid::AdminSession::ice_isA(const ::std::string& _s, const ::Ice::Current&) const
{
    return ::std::binary_search(__IceGrid__AdminSession_ids, __IceGrid__AdminSession_ids + 3, _s);
}

::std::vector< ::std::string>
IceGrid::AdminSession::ice_ids(const ::Ice::Current&) const
{
    return ::std::vector< ::std::string>(&__IceGrid__AdminSession_ids[0], &__IceGrid__AdminSession_ids[3]);
}

const ::std::string&
IceGrid::AdminSession::ice_id(const ::Ice::Current&) const
{
    return __IceGrid__AdminSession_ids[2];
}

const ::std::string&
IceGrid::AdminSession::ice_staticId()
{
    return __IceGrid__AdminSession_ids[2];
}

::IceInternal::DispatchStatus
IceGrid::AdminSession::___keepAlive(::IceInternal::Incoming&, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Idempotent, __current.mode);
    keepAlive(__current);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::AdminSession::___getAdmin(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Idempotent, __current.mode);
    ::IceInternal::BasicStream* __os = __inS.os();
    ::IceGrid::AdminPrx __ret = getAdmin(__current);
    ::IceGrid::__write(__os, __ret);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::AdminSession::___setObservers(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Idempotent, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::IceGrid::RegistryObserverPrx registryObs;
    ::IceGrid::NodeObserverPrx nodeObs;
    ::IceGrid::ApplicationObserverPrx appObs;
    ::IceGrid::AdapterObserverPrx adptObs;
    ::IceGrid::ObjectObserverPrx objObs;
    ::IceGrid::__read(__is, registryObs);
    ::IceGrid::__read(__is, nodeObs);
    ::IceGrid::__read(__is, appObs);
    ::IceGrid::__read(__is, adptObs);
    ::IceGrid::__read(__is, objObs);
    try
    {
        setObservers(registryObs, nodeObs, appObs, adptObs, objObs, __current);
    }
    catch(const ::IceGrid::ObserverAlreadyRegisteredException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::AdminSession::___setObserversByIdentity(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Idempotent, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::Ice::Identity registryObs;
    ::Ice::Identity nodeObs;
    ::Ice::Identity appObs;
    ::Ice::Identity adptObs;
    ::Ice::Identity objObs;
    registryObs.__read(__is);
    nodeObs.__read(__is);
    appObs.__read(__is);
    adptObs.__read(__is);
    objObs.__read(__is);
    try
    {
        setObserversByIdentity(registryObs, nodeObs, appObs, adptObs, objObs, __current);
    }
    catch(const ::IceGrid::ObserverAlreadyRegisteredException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::AdminSession::___startUpdate(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __os = __inS.os();
    try
    {
        ::Ice::Int __ret = startUpdate(__current);
        __os->write(__ret);
    }
    catch(const ::IceGrid::AccessDeniedException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::AdminSession::___finishUpdate(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __os = __inS.os();
    try
    {
        finishUpdate(__current);
    }
    catch(const ::IceGrid::AccessDeniedException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::AdminSession::___getReplicaName(::IceInternal::Incoming&__inS, const ::Ice::Current& __current) const
{
    __checkMode(::Ice::Idempotent, __current.mode);
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string __ret = getReplicaName(__current);
    __os->write(__ret);
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::AdminSession::___openServerLog(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string id;
    ::std::string path;
    ::Ice::Int count;
    __is->read(id);
    __is->read(path);
    __is->read(count);
    try
    {
        ::IceGrid::FileIteratorPrx __ret = openServerLog(id, path, count, __current);
        ::IceGrid::__write(__os, __ret);
    }
    catch(const ::IceGrid::DeploymentException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::FileNotAvailableException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::NodeUnreachableException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::ServerNotExistException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::AdminSession::___openServerStdErr(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string id;
    ::Ice::Int count;
    __is->read(id);
    __is->read(count);
    try
    {
        ::IceGrid::FileIteratorPrx __ret = openServerStdErr(id, count, __current);
        ::IceGrid::__write(__os, __ret);
    }
    catch(const ::IceGrid::DeploymentException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::FileNotAvailableException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::NodeUnreachableException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::ServerNotExistException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::AdminSession::___openServerStdOut(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string id;
    ::Ice::Int count;
    __is->read(id);
    __is->read(count);
    try
    {
        ::IceGrid::FileIteratorPrx __ret = openServerStdOut(id, count, __current);
        ::IceGrid::__write(__os, __ret);
    }
    catch(const ::IceGrid::DeploymentException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::FileNotAvailableException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::NodeUnreachableException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::ServerNotExistException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::AdminSession::___openNodeStdErr(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string name;
    ::Ice::Int count;
    __is->read(name);
    __is->read(count);
    try
    {
        ::IceGrid::FileIteratorPrx __ret = openNodeStdErr(name, count, __current);
        ::IceGrid::__write(__os, __ret);
    }
    catch(const ::IceGrid::FileNotAvailableException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::NodeNotExistException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::NodeUnreachableException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::AdminSession::___openNodeStdOut(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string name;
    ::Ice::Int count;
    __is->read(name);
    __is->read(count);
    try
    {
        ::IceGrid::FileIteratorPrx __ret = openNodeStdOut(name, count, __current);
        ::IceGrid::__write(__os, __ret);
    }
    catch(const ::IceGrid::FileNotAvailableException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::NodeNotExistException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::NodeUnreachableException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::AdminSession::___openRegistryStdErr(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string name;
    ::Ice::Int count;
    __is->read(name);
    __is->read(count);
    try
    {
        ::IceGrid::FileIteratorPrx __ret = openRegistryStdErr(name, count, __current);
        ::IceGrid::__write(__os, __ret);
    }
    catch(const ::IceGrid::FileNotAvailableException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::RegistryNotExistException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::RegistryUnreachableException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

::IceInternal::DispatchStatus
IceGrid::AdminSession::___openRegistryStdOut(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::IceInternal::BasicStream* __os = __inS.os();
    ::std::string name;
    ::Ice::Int count;
    __is->read(name);
    __is->read(count);
    try
    {
        ::IceGrid::FileIteratorPrx __ret = openRegistryStdOut(name, count, __current);
        ::IceGrid::__write(__os, __ret);
    }
    catch(const ::IceGrid::FileNotAvailableException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::RegistryNotExistException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    catch(const ::IceGrid::RegistryUnreachableException& __ex)
    {
        __os->write(__ex);
        return ::IceInternal::DispatchUserException;
    }
    return ::IceInternal::DispatchOK;
}

static ::std::string __IceGrid__AdminSession_all[] =
{
    "destroy",
    "finishUpdate",
    "getAdmin",
    "getReplicaName",
    "ice_id",
    "ice_ids",
    "ice_isA",
    "ice_ping",
    "keepAlive",
    "openNodeStdErr",
    "openNodeStdOut",
    "openRegistryStdErr",
    "openRegistryStdOut",
    "openServerLog",
    "openServerStdErr",
    "openServerStdOut",
    "setObservers",
    "setObserversByIdentity",
    "startUpdate"
};

::IceInternal::DispatchStatus
IceGrid::AdminSession::__dispatch(::IceInternal::Incoming& in, const ::Ice::Current& current)
{
    ::std::pair< ::std::string*, ::std::string*> r = ::std::equal_range(__IceGrid__AdminSession_all, __IceGrid__AdminSession_all + 19, current.operation);
    if(r.first == r.second)
    {
        return ::IceInternal::DispatchOperationNotExist;
    }

    switch(r.first - __IceGrid__AdminSession_all)
    {
        case 0:
        {
            return ___destroy(in, current);
        }
        case 1:
        {
            return ___finishUpdate(in, current);
        }
        case 2:
        {
            return ___getAdmin(in, current);
        }
        case 3:
        {
            return ___getReplicaName(in, current);
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
        case 8:
        {
            return ___keepAlive(in, current);
        }
        case 9:
        {
            return ___openNodeStdErr(in, current);
        }
        case 10:
        {
            return ___openNodeStdOut(in, current);
        }
        case 11:
        {
            return ___openRegistryStdErr(in, current);
        }
        case 12:
        {
            return ___openRegistryStdOut(in, current);
        }
        case 13:
        {
            return ___openServerLog(in, current);
        }
        case 14:
        {
            return ___openServerStdErr(in, current);
        }
        case 15:
        {
            return ___openServerStdOut(in, current);
        }
        case 16:
        {
            return ___setObservers(in, current);
        }
        case 17:
        {
            return ___setObserversByIdentity(in, current);
        }
        case 18:
        {
            return ___startUpdate(in, current);
        }
    }

    assert(false);
    return ::IceInternal::DispatchOperationNotExist;
}

void
IceGrid::AdminSession::__write(::IceInternal::BasicStream* __os) const
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
IceGrid::AdminSession::__read(::IceInternal::BasicStream* __is, bool __rid)
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
IceGrid::AdminSession::__write(const ::Ice::OutputStreamPtr&) const
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type IceGrid::AdminSession was not generated with stream support";
    throw ex;
}

void
IceGrid::AdminSession::__read(const ::Ice::InputStreamPtr&, bool)
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type IceGrid::AdminSession was not generated with stream support";
    throw ex;
}

void ICE_GRID_API 
IceGrid::__patch__AdminSessionPtr(void* __addr, ::Ice::ObjectPtr& v)
{
    ::IceGrid::AdminSessionPtr* p = static_cast< ::IceGrid::AdminSessionPtr*>(__addr);
    assert(p);
    *p = ::IceGrid::AdminSessionPtr::dynamicCast(v);
    if(v && !*p)
    {
        ::Ice::UnexpectedObjectException e(__FILE__, __LINE__);
        e.type = v->ice_id();
        e.expectedType = ::IceGrid::AdminSession::ice_staticId();
        throw e;
    }
}

bool
IceGrid::operator==(const ::IceGrid::AdminSession& l, const ::IceGrid::AdminSession& r)
{
    return static_cast<const ::Ice::Object&>(l) == static_cast<const ::Ice::Object&>(r);
}

bool
IceGrid::operator!=(const ::IceGrid::AdminSession& l, const ::IceGrid::AdminSession& r)
{
    return static_cast<const ::Ice::Object&>(l) != static_cast<const ::Ice::Object&>(r);
}

bool
IceGrid::operator<(const ::IceGrid::AdminSession& l, const ::IceGrid::AdminSession& r)
{
    return static_cast<const ::Ice::Object&>(l) < static_cast<const ::Ice::Object&>(r);
}

bool
IceGrid::operator<=(const ::IceGrid::AdminSession& l, const ::IceGrid::AdminSession& r)
{
    return l < r || l == r;
}

bool
IceGrid::operator>(const ::IceGrid::AdminSession& l, const ::IceGrid::AdminSession& r)
{
    return !(l < r) && !(l == r);
}

bool
IceGrid::operator>=(const ::IceGrid::AdminSession& l, const ::IceGrid::AdminSession& r)
{
    return !(l < r);
}

static const char* __sliceChecksums[] =
{
    "::IceGrid::AdapterInfo", "a22de437e0d82d91cca7d476992b2a43",
    "::IceGrid::AdapterInfoSeq", "9fdbbb3c2d938b4e5f3bf5a21f234147",
    "::IceGrid::Admin", "badd0584b5f98eedd9f26add683c59c",
    "::IceGrid::AdminSession", "628095c4d58e88719563db88b1d41a",
    "::IceGrid::ApplicationInfo", "44ab5928481a1441216f93965f9e6c5",
    "::IceGrid::ApplicationInfoSeq", "dc7429d6b923c3e66eea573eccc1598",
    "::IceGrid::ApplicationUpdateInfo", "c21c8cfe85e332fd9ad194e611bc6b7f",
    "::IceGrid::FileIterator", "54341a38932f89d199f28ffc4712c7",
    "::IceGrid::LoadInfo", "c28c339f5af52a46ac64c33864ae6",
    "::IceGrid::NodeInfo", "f348b389deb653ac28b2b991e23d63b9",
    "::IceGrid::ObjectInfo", "6c8a382c348df5cbda50e58d87189e33",
    "::IceGrid::ObjectInfoSeq", "1491c01cb93b575c602baed26ed0f989",
    "::IceGrid::RegistryInfo", "60e64fc1e37ce59ecbeed4a0e276ba",
    "::IceGrid::RegistryInfoSeq", "fabb868b9f2164f68bc9eb68240c8a6",
    "::IceGrid::ServerInfo", "7f99dc872345b2c3c741c8b4c23440da",
    "::IceGrid::ServerState", "21e8ecba86a4678f3b783de286583093",
    "::IceGrid::StringObjectProxyDict", "978c325e58cebefb212e5ebde28acdc",
    0
};
static IceInternal::SliceChecksumInit __sliceChecksumInit(__sliceChecksums);
