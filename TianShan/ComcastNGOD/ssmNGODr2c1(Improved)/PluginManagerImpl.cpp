#define _WINSOCK2API_
#include "./PluginManagerImpl.h"
#include "./ssmNGODr2c1.h"
#include "./ContextImpl.h"

namespace NGODr2c1
{

	PluginManagerImpl::PluginManagerImpl(ssmNGODr2c1& ngodr2c1) : _ssmNGODr2c1(ngodr2c1)
	{/*MAX_SESSION_CONTEXT*/
	}

	PluginManagerImpl::~PluginManagerImpl()
	{
	}

	::Ice::Int PluginManagerImpl::getAllContext(const ::Ice::Current& c)
	{
		ctxs.clear();
		contextCount = 0;
		::Freeze::EvictorIteratorPtr itor = _ssmNGODr2c1._pContextEvtr->getIterator("", MAX_SESSION_CONTEXT);

		INOUTMAP inoutMap;
		inoutMap[MAP_KEY_METHOD] = "PluginManagerTool";
		while (itor->hasNext())
		{
			Ice::Identity ident = itor->next();
			ContextImplPtr pNewContext = new ContextImpl();
			ContextPrx pNewContextPrx = NULL;

			if (false == _ssmNGODr2c1.getContextByIdentity(pNewContext, pNewContextPrx, ident, inoutMap))
				continue;

			ctxs.push_back(pNewContext);
			contextCount ++;
		}

		return contextCount;
	}

	void PluginManagerImpl::setPageSize(::Ice::Int size, const ::Ice::Current& c)
	{
		pageSize = size;
	}

	::NGODr2c1::Contexts PluginManagerImpl::getFirstPage(::Ice::Int& page, const ::Ice::Current& c) const
	{
		if (contextCount <= pageSize)
			return ctxs;

		::NGODr2c1::Contexts retCtxs;
		for (int cur = 0; cur < pageSize; cur ++)
		{
			retCtxs.push_back(ctxs[cur]);
		}
		page = 1;

		return ctxs;
	}

	::NGODr2c1::Contexts PluginManagerImpl::getLastPage(::Ice::Int& page, const ::Ice::Current& c) const
	{
		if (contextCount <= pageSize)
			return ctxs;

		::NGODr2c1::Contexts retCtxs;
		for (int cur = (contextCount/pageSize) * pageSize; cur < contextCount; cur ++)
		{
			retCtxs.push_back(ctxs[cur]);
		}
		page = (contextCount % pageSize != 0) ? contextCount/pageSize + 1 : contextCount/pageSize;

		return ctxs;
	}

	::NGODr2c1::Contexts PluginManagerImpl::getSpecialPage(::Ice::Int& page, const ::Ice::Current& c) const
	{
		int pageCount = (contextCount % pageSize != 0) ? contextCount/pageSize + 1 : contextCount/pageSize;

		if (page < 1)
			return getFirstPage(page);

		if (page > pageCount)
			return getLastPage(page);

		::NGODr2c1::Contexts retCtxs;
		int endPos = (page + 1) * pageSize;
		if ((page + 1) * pageSize > contextCount)
			endPos = contextCount;
		for (int cur = page * pageSize; cur < endPos; cur ++)
		{
			retCtxs.push_back(ctxs[cur]);
		}
		return retCtxs;
	}

}

