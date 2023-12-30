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

#include <DODContentStore.h>
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

static const ::std::string __TianShanIce__Storage__DataOnDemand__DODContent__setDataWrappingParam_name = "setDataWrappingParam";

void
IceInternal::incRef(::TianShanIce::Storage::DataOnDemand::DODContent* p)
{
    p->__incRef();
}

void
IceInternal::decRef(::TianShanIce::Storage::DataOnDemand::DODContent* p)
{
    p->__decRef();
}

void
IceInternal::incRef(::IceProxy::TianShanIce::Storage::DataOnDemand::DODContent* p)
{
    p->__incRef();
}

void
IceInternal::decRef(::IceProxy::TianShanIce::Storage::DataOnDemand::DODContent* p)
{
    p->__decRef();
}

void
TianShanIce::Storage::DataOnDemand::__write(::IceInternal::BasicStream* __os, const ::TianShanIce::Storage::DataOnDemand::DODContentPrx& v)
{
    __os->write(::Ice::ObjectPrx(v));
}

void
TianShanIce::Storage::DataOnDemand::__read(::IceInternal::BasicStream* __is, ::TianShanIce::Storage::DataOnDemand::DODContentPrx& v)
{
    ::Ice::ObjectPrx proxy;
    __is->read(proxy);
    if(!proxy)
    {
        v = 0;
    }
    else
    {
        v = new ::IceProxy::TianShanIce::Storage::DataOnDemand::DODContent;
        v->__copyFrom(proxy);
    }
}

void
TianShanIce::Storage::DataOnDemand::__write(::IceInternal::BasicStream* __os, const ::TianShanIce::Storage::DataOnDemand::DODContentPtr& v)
{
    __os->write(::Ice::ObjectPtr(v));
}

bool
TianShanIce::Storage::DataOnDemand::DataWrappingParam::operator==(const DataWrappingParam& __rhs) const
{
    return !operator!=(__rhs);
}

bool
TianShanIce::Storage::DataOnDemand::DataWrappingParam::operator!=(const DataWrappingParam& __rhs) const
{
    if(this == &__rhs)
    {
        return false;
    }
    if(esPID != __rhs.esPID)
    {
        return true;
    }
    if(streamType != __rhs.streamType)
    {
        return true;
    }
    if(subStreamCount != __rhs.subStreamCount)
    {
        return true;
    }
    if(dataType != __rhs.dataType)
    {
        return true;
    }
    if(withObjIndex != __rhs.withObjIndex)
    {
        return true;
    }
    if(objTag != __rhs.objTag)
    {
        return true;
    }
    if(encryptType != __rhs.encryptType)
    {
        return true;
    }
    if(versionNumber != __rhs.versionNumber)
    {
        return true;
    }
    return false;
}

bool
TianShanIce::Storage::DataOnDemand::DataWrappingParam::operator<(const DataWrappingParam& __rhs) const
{
    if(this == &__rhs)
    {
        return false;
    }
    if(esPID < __rhs.esPID)
    {
        return true;
    }
    else if(__rhs.esPID < esPID)
    {
        return false;
    }
    if(streamType < __rhs.streamType)
    {
        return true;
    }
    else if(__rhs.streamType < streamType)
    {
        return false;
    }
    if(subStreamCount < __rhs.subStreamCount)
    {
        return true;
    }
    else if(__rhs.subStreamCount < subStreamCount)
    {
        return false;
    }
    if(dataType < __rhs.dataType)
    {
        return true;
    }
    else if(__rhs.dataType < dataType)
    {
        return false;
    }
    if(withObjIndex < __rhs.withObjIndex)
    {
        return true;
    }
    else if(__rhs.withObjIndex < withObjIndex)
    {
        return false;
    }
    if(objTag < __rhs.objTag)
    {
        return true;
    }
    else if(__rhs.objTag < objTag)
    {
        return false;
    }
    if(encryptType < __rhs.encryptType)
    {
        return true;
    }
    else if(__rhs.encryptType < encryptType)
    {
        return false;
    }
    if(versionNumber < __rhs.versionNumber)
    {
        return true;
    }
    else if(__rhs.versionNumber < versionNumber)
    {
        return false;
    }
    return false;
}

void
TianShanIce::Storage::DataOnDemand::DataWrappingParam::__write(::IceInternal::BasicStream* __os) const
{
    __os->write(esPID);
    __os->write(streamType);
    __os->write(subStreamCount);
    __os->write(dataType);
    __os->write(withObjIndex);
    __os->write(objTag);
    __os->write(encryptType);
    __os->write(versionNumber);
}

void
TianShanIce::Storage::DataOnDemand::DataWrappingParam::__read(::IceInternal::BasicStream* __is)
{
    __is->read(esPID);
    __is->read(streamType);
    __is->read(subStreamCount);
    __is->read(dataType);
    __is->read(withObjIndex);
    __is->read(objTag);
    __is->read(encryptType);
    __is->read(versionNumber);
}

void
TianShanIce::Storage::DataOnDemand::__addObject(const DODContentPtr& p, ::IceInternal::GCCountMap& c)
{
    p->__addObject(c);
}

bool
TianShanIce::Storage::DataOnDemand::__usesClasses(const DODContentPtr& p)
{
    return p->__usesClasses();
}

void
TianShanIce::Storage::DataOnDemand::__decRefUnsafe(const DODContentPtr& p)
{
    p->__decRefUnsafe();
}

void
TianShanIce::Storage::DataOnDemand::__clearHandleUnsafe(DODContentPtr& p)
{
    p.__clearHandleUnsafe();
}

void
IceProxy::TianShanIce::Storage::DataOnDemand::DODContent::setDataWrappingParam(const ::TianShanIce::Storage::DataOnDemand::DataWrappingParam& param, const ::Ice::Context* __ctx)
{
    int __cnt = 0;
    while(true)
    {
        ::IceInternal::Handle< ::IceDelegate::Ice::Object> __delBase;
        try
        {
            __delBase = __getDelegate();
            ::IceDelegate::TianShanIce::Storage::DataOnDemand::DODContent* __del = dynamic_cast< ::IceDelegate::TianShanIce::Storage::DataOnDemand::DODContent*>(__delBase.get());
            __del->setDataWrappingParam(param, __ctx);
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
IceProxy::TianShanIce::Storage::DataOnDemand::DODContent::ice_staticId()
{
    return ::TianShanIce::Storage::DataOnDemand::DODContent::ice_staticId();
}

::IceInternal::Handle< ::IceDelegateM::Ice::Object>
IceProxy::TianShanIce::Storage::DataOnDemand::DODContent::__createDelegateM()
{
    return ::IceInternal::Handle< ::IceDelegateM::Ice::Object>(new ::IceDelegateM::TianShanIce::Storage::DataOnDemand::DODContent);
}

::IceInternal::Handle< ::IceDelegateD::Ice::Object>
IceProxy::TianShanIce::Storage::DataOnDemand::DODContent::__createDelegateD()
{
    return ::IceInternal::Handle< ::IceDelegateD::Ice::Object>(new ::IceDelegateD::TianShanIce::Storage::DataOnDemand::DODContent);
}

bool
IceProxy::TianShanIce::Storage::DataOnDemand::operator==(const ::IceProxy::TianShanIce::Storage::DataOnDemand::DODContent& l, const ::IceProxy::TianShanIce::Storage::DataOnDemand::DODContent& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) == static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::TianShanIce::Storage::DataOnDemand::operator!=(const ::IceProxy::TianShanIce::Storage::DataOnDemand::DODContent& l, const ::IceProxy::TianShanIce::Storage::DataOnDemand::DODContent& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) != static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::TianShanIce::Storage::DataOnDemand::operator<(const ::IceProxy::TianShanIce::Storage::DataOnDemand::DODContent& l, const ::IceProxy::TianShanIce::Storage::DataOnDemand::DODContent& r)
{
    return static_cast<const ::IceProxy::Ice::Object&>(l) < static_cast<const ::IceProxy::Ice::Object&>(r);
}

bool
IceProxy::TianShanIce::Storage::DataOnDemand::operator<=(const ::IceProxy::TianShanIce::Storage::DataOnDemand::DODContent& l, const ::IceProxy::TianShanIce::Storage::DataOnDemand::DODContent& r)
{
    return l < r || l == r;
}

bool
IceProxy::TianShanIce::Storage::DataOnDemand::operator>(const ::IceProxy::TianShanIce::Storage::DataOnDemand::DODContent& l, const ::IceProxy::TianShanIce::Storage::DataOnDemand::DODContent& r)
{
    return !(l < r) && !(l == r);
}

bool
IceProxy::TianShanIce::Storage::DataOnDemand::operator>=(const ::IceProxy::TianShanIce::Storage::DataOnDemand::DODContent& l, const ::IceProxy::TianShanIce::Storage::DataOnDemand::DODContent& r)
{
    return !(l < r);
}

void
IceDelegateM::TianShanIce::Storage::DataOnDemand::DODContent::setDataWrappingParam(const ::TianShanIce::Storage::DataOnDemand::DataWrappingParam& param, const ::Ice::Context* __context)
{
    ::IceInternal::Outgoing __og(__connection.get(), __reference.get(), __TianShanIce__Storage__DataOnDemand__DODContent__setDataWrappingParam_name, ::Ice::Normal, __context, __compress);
    try
    {
        ::IceInternal::BasicStream* __os = __og.os();
        param.__write(__os);
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
        throw ::IceInternal::LocalExceptionWrapper(__ex, false);
    }
}

void
IceDelegateD::TianShanIce::Storage::DataOnDemand::DODContent::setDataWrappingParam(const ::TianShanIce::Storage::DataOnDemand::DataWrappingParam& param, const ::Ice::Context* __context)
{
    ::Ice::Current __current;
    __initCurrent(__current, __TianShanIce__Storage__DataOnDemand__DODContent__setDataWrappingParam_name, ::Ice::Normal, __context);
    while(true)
    {
        ::IceInternal::Direct __direct(__current);
        try
        {
            ::TianShanIce::Storage::DataOnDemand::DODContent* __servant = dynamic_cast< ::TianShanIce::Storage::DataOnDemand::DODContent*>(__direct.servant().get());
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
                __servant->setDataWrappingParam(param, __current);
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

TianShanIce::Storage::DataOnDemand::DODContent::DODContent(::Ice::Long __ice_stampCreated, ::Ice::Long __ice_stampProvisioned, ::Ice::Long __ice_stampLastUpdated, ::TianShanIce::Storage::ContentState __ice_state, const ::std::string& __ice_provisionPrxStr, const ::TianShanIce::Storage::TrickSpeedCollection& __ice_trickSpeeds, const ::TianShanIce::Properties& __ice_metaData, const ::std::string& __ice_name) :
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    Content(__ice_stampCreated, __ice_stampProvisioned, __ice_stampLastUpdated, __ice_state, __ice_provisionPrxStr, __ice_trickSpeeds, __ice_metaData)
#else
    ::TianShanIce::Storage::Content(__ice_stampCreated, __ice_stampProvisioned, __ice_stampLastUpdated, __ice_state, __ice_provisionPrxStr, __ice_trickSpeeds, __ice_metaData)
#endif
,
    name(__ice_name)
{
}

::Ice::ObjectPtr
TianShanIce::Storage::DataOnDemand::DODContent::ice_clone() const
{
    throw ::Ice::CloneNotImplementedException(__FILE__, __LINE__);
    return 0; // to avoid a warning with some compilers
}

static const ::std::string __TianShanIce__Storage__DataOnDemand__DODContent_ids[3] =
{
    "::Ice::Object",
    "::TianShanIce::Storage::Content",
    "::TianShanIce::Storage::DataOnDemand::DODContent"
};

bool
TianShanIce::Storage::DataOnDemand::DODContent::ice_isA(const ::std::string& _s, const ::Ice::Current&) const
{
    return ::std::binary_search(__TianShanIce__Storage__DataOnDemand__DODContent_ids, __TianShanIce__Storage__DataOnDemand__DODContent_ids + 3, _s);
}

::std::vector< ::std::string>
TianShanIce::Storage::DataOnDemand::DODContent::ice_ids(const ::Ice::Current&) const
{
    return ::std::vector< ::std::string>(&__TianShanIce__Storage__DataOnDemand__DODContent_ids[0], &__TianShanIce__Storage__DataOnDemand__DODContent_ids[3]);
}

const ::std::string&
TianShanIce::Storage::DataOnDemand::DODContent::ice_id(const ::Ice::Current&) const
{
    return __TianShanIce__Storage__DataOnDemand__DODContent_ids[2];
}

const ::std::string&
TianShanIce::Storage::DataOnDemand::DODContent::ice_staticId()
{
    return __TianShanIce__Storage__DataOnDemand__DODContent_ids[2];
}

::IceInternal::DispatchStatus
TianShanIce::Storage::DataOnDemand::DODContent::___setDataWrappingParam(::IceInternal::Incoming&__inS, const ::Ice::Current& __current)
{
    __checkMode(::Ice::Normal, __current.mode);
    ::IceInternal::BasicStream* __is = __inS.is();
    ::TianShanIce::Storage::DataOnDemand::DataWrappingParam param;
    param.__read(__is);
    setDataWrappingParam(param, __current);
    return ::IceInternal::DispatchOK;
}

static ::std::string __TianShanIce__Storage__DataOnDemand__DODContent_all[] =
{
    "cancelProvision",
    "destroy",
    "destroy2",
    "getBitRate",
    "getExportURL",
    "getFilesize",
    "getFramerate",
    "getLocaltype",
    "getMD5Checksum",
    "getMetaData",
    "getName",
    "getPlayTime",
    "getPlayTimeEx",
    "getProvisionTime",
    "getResolution",
    "getSourceUrl",
    "getState",
    "getStore",
    "getSubtype",
    "getSupportFileSize",
    "getTrickSpeedCollection",
    "ice_id",
    "ice_ids",
    "ice_isA",
    "ice_ping",
    "isProvisioned",
    "provision",
    "provisionPassive",
    "setDataWrappingParam",
    "setUserMetaData",
    "setUserMetaData2",
    "theVolume"
};

::IceInternal::DispatchStatus
TianShanIce::Storage::DataOnDemand::DODContent::__dispatch(::IceInternal::Incoming& in, const ::Ice::Current& current)
{
    ::std::pair< ::std::string*, ::std::string*> r = ::std::equal_range(__TianShanIce__Storage__DataOnDemand__DODContent_all, __TianShanIce__Storage__DataOnDemand__DODContent_all + 32, current.operation);
    if(r.first == r.second)
    {
        return ::IceInternal::DispatchOperationNotExist;
    }

    switch(r.first - __TianShanIce__Storage__DataOnDemand__DODContent_all)
    {
        case 0:
        {
            return ___cancelProvision(in, current);
        }
        case 1:
        {
            return ___destroy(in, current);
        }
        case 2:
        {
            return ___destroy2(in, current);
        }
        case 3:
        {
            return ___getBitRate(in, current);
        }
        case 4:
        {
            return ___getExportURL(in, current);
        }
        case 5:
        {
            return ___getFilesize(in, current);
        }
        case 6:
        {
            return ___getFramerate(in, current);
        }
        case 7:
        {
            return ___getLocaltype(in, current);
        }
        case 8:
        {
            return ___getMD5Checksum(in, current);
        }
        case 9:
        {
            return ___getMetaData(in, current);
        }
        case 10:
        {
            return ___getName(in, current);
        }
        case 11:
        {
            return ___getPlayTime(in, current);
        }
        case 12:
        {
            return ___getPlayTimeEx(in, current);
        }
        case 13:
        {
            return ___getProvisionTime(in, current);
        }
        case 14:
        {
            return ___getResolution(in, current);
        }
        case 15:
        {
            return ___getSourceUrl(in, current);
        }
        case 16:
        {
            return ___getState(in, current);
        }
        case 17:
        {
            return ___getStore(in, current);
        }
        case 18:
        {
            return ___getSubtype(in, current);
        }
        case 19:
        {
            return ___getSupportFileSize(in, current);
        }
        case 20:
        {
            return ___getTrickSpeedCollection(in, current);
        }
        case 21:
        {
            return ___ice_id(in, current);
        }
        case 22:
        {
            return ___ice_ids(in, current);
        }
        case 23:
        {
            return ___ice_isA(in, current);
        }
        case 24:
        {
            return ___ice_ping(in, current);
        }
        case 25:
        {
            return ___isProvisioned(in, current);
        }
        case 26:
        {
            return ___provision(in, current);
        }
        case 27:
        {
            return ___provisionPassive(in, current);
        }
        case 28:
        {
            return ___setDataWrappingParam(in, current);
        }
        case 29:
        {
            return ___setUserMetaData(in, current);
        }
        case 30:
        {
            return ___setUserMetaData2(in, current);
        }
        case 31:
        {
            return ___theVolume(in, current);
        }
    }

    assert(false);
    return ::IceInternal::DispatchOperationNotExist;
}

static ::std::string __TianShanIce__Storage__DataOnDemand__DODContent_freezeWriteOperations[] =
{
    "cancelProvision",
    "destroy",
    "destroy2",
    "getExportURL",
    "provision",
    "provisionPassive",
    "setUserMetaData",
    "setUserMetaData2"
};

::Ice::Int
TianShanIce::Storage::DataOnDemand::DODContent::ice_operationAttributes(const ::std::string& opName) const
{
    ::std::string* end = __TianShanIce__Storage__DataOnDemand__DODContent_freezeWriteOperations + 8;
    ::std::string* r = ::std::find(__TianShanIce__Storage__DataOnDemand__DODContent_freezeWriteOperations, end, opName);
    return r == end ? 0 : 1;
}

void
TianShanIce::Storage::DataOnDemand::DODContent::__write(::IceInternal::BasicStream* __os) const
{
    __os->writeTypeId(ice_staticId());
    __os->startWriteSlice();
    __os->write(name);
    __os->endWriteSlice();
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    Content::__write(__os);
#else
    ::TianShanIce::Storage::Content::__write(__os);
#endif
}

void
TianShanIce::Storage::DataOnDemand::DODContent::__read(::IceInternal::BasicStream* __is, bool __rid)
{
    if(__rid)
    {
        ::std::string myId;
        __is->readTypeId(myId);
    }
    __is->startReadSlice();
    __is->read(name);
    __is->endReadSlice();
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    Content::__read(__is, true);
#else
    ::TianShanIce::Storage::Content::__read(__is, true);
#endif
}

void
TianShanIce::Storage::DataOnDemand::DODContent::__write(const ::Ice::OutputStreamPtr&) const
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type TianShanIce::Storage::DataOnDemand::DODContent was not generated with stream support";
    throw ex;
}

void
TianShanIce::Storage::DataOnDemand::DODContent::__read(const ::Ice::InputStreamPtr&, bool)
{
    Ice::MarshalException ex(__FILE__, __LINE__);
    ex.reason = "type TianShanIce::Storage::DataOnDemand::DODContent was not generated with stream support";
    throw ex;
}

void 
TianShanIce::Storage::DataOnDemand::__patch__DODContentPtr(void* __addr, ::Ice::ObjectPtr& v)
{
    ::TianShanIce::Storage::DataOnDemand::DODContentPtr* p = static_cast< ::TianShanIce::Storage::DataOnDemand::DODContentPtr*>(__addr);
    assert(p);
    *p = ::TianShanIce::Storage::DataOnDemand::DODContentPtr::dynamicCast(v);
    if(v && !*p)
    {
        ::Ice::UnexpectedObjectException e(__FILE__, __LINE__);
        e.type = v->ice_id();
        e.expectedType = ::TianShanIce::Storage::DataOnDemand::DODContent::ice_staticId();
        throw e;
    }
}

bool
TianShanIce::Storage::DataOnDemand::operator==(const ::TianShanIce::Storage::DataOnDemand::DODContent& l, const ::TianShanIce::Storage::DataOnDemand::DODContent& r)
{
    return static_cast<const ::Ice::Object&>(l) == static_cast<const ::Ice::Object&>(r);
}

bool
TianShanIce::Storage::DataOnDemand::operator!=(const ::TianShanIce::Storage::DataOnDemand::DODContent& l, const ::TianShanIce::Storage::DataOnDemand::DODContent& r)
{
    return static_cast<const ::Ice::Object&>(l) != static_cast<const ::Ice::Object&>(r);
}

bool
TianShanIce::Storage::DataOnDemand::operator<(const ::TianShanIce::Storage::DataOnDemand::DODContent& l, const ::TianShanIce::Storage::DataOnDemand::DODContent& r)
{
    return static_cast<const ::Ice::Object&>(l) < static_cast<const ::Ice::Object&>(r);
}

bool
TianShanIce::Storage::DataOnDemand::operator<=(const ::TianShanIce::Storage::DataOnDemand::DODContent& l, const ::TianShanIce::Storage::DataOnDemand::DODContent& r)
{
    return l < r || l == r;
}

bool
TianShanIce::Storage::DataOnDemand::operator>(const ::TianShanIce::Storage::DataOnDemand::DODContent& l, const ::TianShanIce::Storage::DataOnDemand::DODContent& r)
{
    return !(l < r) && !(l == r);
}

bool
TianShanIce::Storage::DataOnDemand::operator>=(const ::TianShanIce::Storage::DataOnDemand::DODContent& l, const ::TianShanIce::Storage::DataOnDemand::DODContent& r)
{
    return !(l < r);
}
