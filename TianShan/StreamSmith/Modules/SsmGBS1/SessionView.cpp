#define _WINSOCKAPI_
#include "./SessionView.h"
#include "./Environment.h"

namespace TianShanS1
{
	SessionViewImpl::SessionViewImpl(Environment& env) : _env(env)
	{
		_vSessDatas.clear();
	}

	SessionViewImpl::~SessionViewImpl()
	{
	}

	SessionDatas SessionViewImpl::getRange(::Ice::Int from, ::Ice::Int to, ::Ice::Int clientId, const ::Ice::Current&) const
	{
		IceUtil::Mutex::Lock lk(*this);
		if (clientId < 0 || clientId > (int)_vSessDatas.size() - 1)
			throw ErrorBase("Client not registered");

		SessionDatas retCtxs;
		if (from < 1)
			from = 1;
		while (from <= to && from <= (int)(*_vSessDatas[clientId]).size())
		{
			retCtxs.push_back((*_vSessDatas[clientId])[from - 1]);
			from ++;
		}
		return retCtxs;
	}

	::Ice::Int SessionViewImpl::getAllContext(::Ice::Int iIdent, ::Ice::Int& oIdent, const ::Ice::Current& c)
	{
		IceUtil::Mutex::Lock lk(*this);
		SessionDatas* pSessDatas = new SessionDatas();
		::Freeze::EvictorIteratorPtr itor = _env._pContextEvictor->getIterator("", 20000);
		while (itor->hasNext())
		{
			Ice::Identity ident = itor->next();
			SessionContextPrx sessPrx = NULL;
			SessionData data;
			sessPrx = SessionContextPrx::uncheckedCast(_env._pAdapter->createProxy(ident));
			if (!sessPrx)
				continue;
			data = sessPrx->getSessionData();
			(*pSessDatas).push_back(data);
		}
		
		if (0 <= iIdent && iIdent <= (int)_vSessDatas.size() - 1)
		{
			delete _vSessDatas[iIdent];
			_vSessDatas[iIdent] = pSessDatas;
			oIdent = iIdent;
		}
		else 
		{
			bool bEmpty = false;
			std ::vector<SessionDatas*>::size_type i;
			for (i = 0; i < _vSessDatas.size(); i ++)
			{
			if (NULL == _vSessDatas[i])
			{
				bEmpty = true;
				break;
			}
			}
			if (false == bEmpty)
			{
				_vSessDatas.push_back(pSessDatas);
				oIdent = (Ice::Int) _vSessDatas.size() - 1;
			}
			else 
			{
				_vSessDatas[i] = pSessDatas;
				oIdent = i;
			}
		}

		return (int)(*pSessDatas).size();
	}

	void SessionViewImpl::unregister(::Ice::Int iIdent, const ::Ice::Current& c)
	{
		IceUtil::Mutex::Lock lk(*this);
		if (0 <= iIdent && iIdent <= (int)_vSessDatas.size() - 1)
		{
			delete _vSessDatas[iIdent];
			_vSessDatas[iIdent] = NULL;
		}
		else 
			throw ErrorBase("Client not registered");
	}
}

