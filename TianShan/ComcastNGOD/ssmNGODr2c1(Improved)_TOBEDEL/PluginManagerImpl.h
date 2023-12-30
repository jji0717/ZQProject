#ifndef __PluginManager_H__
#define __PluginManager_H__

#include "./NGODr2c1.h"
#include <IceUtil/IceUtil.h>

class ssmNGODr2c1;
namespace NGODr2c1
{

class PluginManagerImpl : public NGODr2c1::PluginManager
{
public: 
	PluginManagerImpl(ssmNGODr2c1& ngodr2c1);
	virtual ~PluginManagerImpl();

	typedef IceInternal::Handle<PluginManagerImpl> Ptr;
	virtual ::Ice::Int getAllContext(const ::Ice::Current& = ::Ice::Current());
	virtual void setPageSize(::Ice::Int, const ::Ice::Current& = ::Ice::Current());
	virtual ::NGODr2c1::Contexts getFirstPage(::Ice::Int&, const ::Ice::Current& = ::Ice::Current()) const;
	virtual ::NGODr2c1::Contexts getLastPage(::Ice::Int&, const ::Ice::Current& = ::Ice::Current()) const;
	virtual ::NGODr2c1::Contexts getSpecialPage(::Ice::Int&, const ::Ice::Current& = ::Ice::Current()) const;

	ssmNGODr2c1& _ssmNGODr2c1;
};

typedef PluginManagerImpl::Ptr PluginManagerImplPtr;

}

#endif // __PluginManager_H__