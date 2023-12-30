
#include "DSServantLocatorImpl.h"
#include "log.h"

using namespace std;
using namespace Freeze;
using namespace Ice;
using namespace ZQ::common;

Freeze::DSServantLocatorImpl::DSServantLocatorImpl(ZQADAPTER_DECLTYPE& adapter, Ice::CommunicatorPtr& communicator):
_adapter(adapter),_communicator(communicator),_pingobject(new PingObject)
{

}

Freeze::DSServantLocatorImpl::~DSServantLocatorImpl()
{

}

::Ice::ObjectPrx
Freeze::DSServantLocatorImpl::add(const ::Ice::ObjectPtr& servant,
                               const ::Ice::Identity& id)
{
   return addFacet(servant, id, "");
}

::Ice::ObjectPrx
Freeze::DSServantLocatorImpl::addFacet(const ::Ice::ObjectPtr& servant,
									   const ::Ice::Identity& id,
									   const ::std::string& facet)
{
	Lock sync(*this);

	// if has this facet
	if(hasFacet(id, facet))
	{
		::Ice::AlreadyRegisteredException ex(__FILE__, __LINE__);
		ex.kindOfObject = "servant";
		ex.id = _communicator->identityToString(id);
		if(!facet.empty())
		{
			ex.id += " -f " + IceUtil::escapeString(facet, "");
		}
		throw ex;
	}
	else
	{		
		DSObject dsobj;
		dsobj.id = id;
		dsobj.servant = servant;

		StoreObjMap::iterator itor;
		itor = _storeobjmap.find(facet);
		if(itor != _storeobjmap.end())
		{
			itor->second.push_back(dsobj);
		}
		else
		{
			DSObjectList dsobjlist;
			dsobjlist.push_back(dsobj);
			_storeobjmap.insert(StoreObjMap::value_type(facet, dsobjlist));
		}
	}
	return _adapter->createProxy(id);
}

::Ice::ObjectPtr
Freeze::DSServantLocatorImpl::remove(const ::Ice::Identity& id)
{
   return removeFacet(id, "");
}

::Ice::ObjectPtr
Freeze::DSServantLocatorImpl::removeFacet(const ::Ice::Identity& id,
                                       const ::std::string& facet)
{
	Lock sync(*this);
	StoreObjMap::iterator itor;
	Ice::ObjectPtr servant = 0;
	itor = _storeobjmap.find(facet);
	if(itor != _storeobjmap.end())
	{
		DSObjectList::iterator dsobjItor;
		DSObjectList& dsobjlist = itor->second;
		for(dsobjItor = dsobjlist.begin(); dsobjItor != dsobjlist.end(); dsobjItor++)
		{
			if(dsobjItor->id == id)
			{
				servant = dsobjItor->servant;
				dsobjlist.erase(dsobjItor);
				if(dsobjlist.empty())
				{
					_storeobjmap.erase(itor);
				}
				break;
			}
		} 
	}

	if(servant == 0)
	{
		Ice::NotRegisteredException ex(__FILE__, __LINE__);
		ex.kindOfObject = "servant";
		ex.id = _communicator->identityToString(id);
		if(!facet.empty())
		{
			ex.id += " -f " + IceUtil::escapeString(facet, "");
		}
		throw ex;
	}
	return servant;
}

bool
Freeze::DSServantLocatorImpl::hasObject(const ::Ice::Identity& id)
{
    return hasFacet(id,"");
}

bool
Freeze::DSServantLocatorImpl::hasFacet(const ::Ice::Identity& id,
                                    const ::std::string& facet)
{
	Lock sync(*this);
	StoreObjMap::iterator itor;
	itor = _storeobjmap.find(facet);
	if(itor != _storeobjmap.end())
	{
		DSObjectList::iterator dsobjItor;
		DSObjectList& dsobjlist = itor->second;
		for(dsobjItor = dsobjlist.begin(); dsobjItor != dsobjlist.end(); dsobjItor++)
		{
			if(dsobjItor->id == id)
			{
				return true;
			}
		}
	}
    return false;
}
Ice::ObjectPtr
Freeze::DSServantLocatorImpl::locate(const Ice::Current& current, Ice::LocalObjectPtr& cookie)
{

	 Lock sync(*this);

	// Special ice_ping() handling
	if(current.operation == "ice_ping")
	{
		if(hasFacet(current.id, current.facet))
		{
			glog(ZQ::common::Log::L_DEBUG,
				"DSServantLocator ice_ping found '%s' with facet'%s'",
				(_communicator->identityToString(current.id)).c_str(), current.facet.c_str());

			cookie = 0;
			return _pingobject;
		}
/*		else if(hasAnotherFacet(current.id, current.facet))
		{
				Trace out(_communicator->getLogger(), "Freeze.Evictor");
				out << "ice_ping raises FacetNotExistException for \"" << _communicator->identityToString(current.id)  
					<< "\" with facet \"" << current.facet + "\"";
			throw FacetNotExistException(__FILE__, __LINE__);
		}*/
		else
		{
			glog(ZQ::common::Log::L_ERROR,
				 "DSServantLocator ice_ping will raise ObjectNotExistException for '%s' with facet '%s'",
				(_communicator->identityToString(current.id)).c_str(), current.facet.c_str());
			return 0;
		}
	}

	StoreObjMap::iterator itor;
	itor = _storeobjmap.find(current.facet);
	if(itor != _storeobjmap.end())
	{
		DSObjectList::iterator dsobjItor;
		DSObjectList& dsobjlist = itor->second;
		for(dsobjItor = dsobjlist.begin(); dsobjItor != dsobjlist.end(); dsobjItor++)
		{
			if(dsobjItor->id == current.id)
			{
				return dsobjItor->servant;
			}
		}
	}
	glog(ZQ::common::Log::L_ERROR,
		"DSServantLocator ice_ping will raise FacetNotExistException for '%s' with facet '%s'",
		(_communicator->identityToString(current.id)).c_str(), current.facet.c_str());

	throw FacetNotExistException(__FILE__, __LINE__);
	return 0;
}

void
Freeze::DSServantLocatorImpl::finished(const Ice::Current& current, const Ice::ObjectPtr& servant, const Ice::LocalObjectPtr& cookie)
{
	assert(servant);
}

void
Freeze::DSServantLocatorImpl::deactivate(const ::std::string&)
{
}
