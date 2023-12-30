#include "IMgr.hpp"

int  IMgr::add(ICommuncator::Ptr val, IAsyncClientHandler::Ptr client)    
{
	ZQ::common::MutexGuard guard(_connsMutex);
	_conns[val] = client;
	return true;
}

int  IMgr::lookup(ICommuncator::Ptr val)    
{
	ZQ::common::MutexGuard guard(_connsMutex);

	Items::iterator it =  _conns.find(val);//std::find(_conns.begin(), _conns.end(), val);
	if (it == _conns.end())
	{
		//_connsSem.post();
		return false;//not exist
	}

	return true;
}
int  IMgr::remove(ICommuncator::Ptr val)    
{
	ZQ::common::MutexGuard guard(_connsMutex);
	Items::iterator it = _conns.find(val); //std::find(_conns.begin(), _conns.end(), val);
	if (it == _conns.end())
	{
		//_connsSem.post();
		return false;//not exist
	}
	_conns.erase(it);

	return true;
}