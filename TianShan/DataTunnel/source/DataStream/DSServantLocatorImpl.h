#ifndef __DSServantLocatorI_h__
#define __DSServantLocatorI_h__
#pragma once
#include <TianShanDefines.h>
#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <Freeze/Freeze.h>
#include <Freeze/Index.h>
#include <list>
#include <vector>
#include <deque>
#include <IceUtil/DisableWarnings.h>
#include <DSServantLocator.h>

struct DSObject
{
	Ice::Identity id;
	Ice::ObjectPtr servant;
};
typedef std::vector<DSObject> DSObjectList;
typedef std::map<std::string, DSObjectList>StoreObjMap;
namespace Freeze
{
class DSServantLocatorImpl : public DSServantLocator,
						  public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    DSServantLocatorImpl(ZQADAPTER_DECLTYPE& adapter,Ice::CommunicatorPtr& communicator);
	~DSServantLocatorImpl();
    virtual ::Ice::ObjectPrx add(const ::Ice::ObjectPtr&,
                                 const ::Ice::Identity&);

    virtual ::Ice::ObjectPrx addFacet(const ::Ice::ObjectPtr&,
                                      const ::Ice::Identity&,
                                      const ::std::string&);

    virtual ::Ice::ObjectPtr remove(const ::Ice::Identity&);

    virtual ::Ice::ObjectPtr removeFacet(const ::Ice::Identity&,
                                         const ::std::string&);

    virtual bool hasObject(const ::Ice::Identity&);

    virtual bool hasFacet(const ::Ice::Identity&,
                          const ::std::string&);

	virtual ::Ice::ObjectPtr locate(const ::Ice::Current&, ::Ice::LocalObjectPtr&);

	virtual void finished(const ::Ice::Current&, const ::Ice::ObjectPtr&, const ::Ice::LocalObjectPtr&);

	virtual void deactivate(const ::std::string&);
protected:
    StoreObjMap _storeobjmap;
	Ice::CommunicatorPtr& _communicator;
	ZQADAPTER_DECLTYPE& _adapter;
	PingObjectPtr _pingobject;
};
typedef ::IceInternal::Handle< ::Freeze::DSServantLocatorImpl> DSServantLocatorIPtr;
}

#endif
