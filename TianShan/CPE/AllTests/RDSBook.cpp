// RDSBook.cpp: implementation of the RDSBook class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable:4244)

#include "stdio.h"
#include "RDSBook.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define TIMECMP(_X, _Y) ((time_t)(_X) - (time_t)(_Y))

//#define  CHAIN_TRACE

#if !defined(_DEBUG)
#  undef  CHAIN_TRACE
#endif // _DEBUG



#ifdef CHAIN_TRACE
#undef CHAIN_TRACE
#define CHAIN_TRACE	{ printf("\n%s(%3d)\n", __FILE__, __LINE__);\
					for (bookchain_t::iterator cit = chain.begin(); cit < chain.end(); cit++) \
					printf("NODE{st=%d; et=%d; ld=%d} ", cit->st, cit->et, cit->value); }
#else
#define CHAIN_TRACE
#endif


#define DURATON_MIN_SKIP	60		//if the time duration of a item is too small and its' value is very big, we can skip this


namespace TianShanIce{ 
namespace Storage{ 
namespace PushContentModule{

// add current exist RDS id
void RDSBook::addRDS(const std::string& strRDSId, unsigned int maxBandwidthKbps,  unsigned int maxInstance)
{
	if (!maxBandwidthKbps || !maxInstance)
		return;

	{
		ProvisionChain*	pProvisionChain = NULL;

		//find the provision chain, create if not exist
		ProvisionChainMap::iterator it = _chainMap.find(strRDSId);
		if(it == _chainMap.end())
		{
			pProvisionChain = new ProvisionChain(strRDSId, maxBandwidthKbps, maxInstance);	
			_chainMap[strRDSId] = pProvisionChain;
		}
	}
}

// must call addRDS before this, because if the RDS id not found, item will not be added
void RDSBook::addBookedItem(const std::string& strRDSId, unsigned int timeStart, unsigned int timeStop, unsigned int bandwidthKbps)
{
	ProvisionChain*	pProvisionChain = NULL;

	//find the provision chain, create if not exist
	ProvisionChainMap::iterator it = _chainMap.find(strRDSId);
	if(it == _chainMap.end())
		return;

	pProvisionChain = it->second;

	ProvisionChain::ProvisionItem itemToBook;
	itemToBook.st = timeStart;
	itemToBook.et = timeStop;
	itemToBook.bandwidth = bandwidthKbps;
	itemToBook.instance = 1;

	pProvisionChain->add(itemToBook);
}

// id, the returned booked RDS id
bool RDSBook::bookRDS(unsigned int timeStart, unsigned int timeStop, unsigned int bandwidthKbps, std::string& strRDSId, unsigned int& nPercentLoad, RDSCostList& costList)
{
	ProvisionChain*	pProvisionChain = NULL;
	unsigned int nMaxCost = MAX_LOAD_VALUE + 1;

	ProvisionChainMap::iterator it;
	for (it = _chainMap.begin(); it != _chainMap.end(); it++ )
	{
		ProvisionChain* pChain = it->second;

		ProvisionChain::ProvisionItem itemToBook;
		itemToBook.st = timeStart;
		itemToBook.et = timeStop;
		itemToBook.bandwidth = bandwidthKbps;
		itemToBook.instance = 1;

		pChain->add(itemToBook);
		RDSCost rdsCost;
		rdsCost.strIdent = pChain->getIdent();
		rdsCost.nCost = pChain->getMaxCost();
		costList.push_back(rdsCost);

		if (pChain->isOverLoad())
		{
			continue;
		}

		if (nMaxCost > pChain->getMaxCost())
		{
			nMaxCost = pChain->getMaxCost();
			pProvisionChain = pChain;
		}
	}

	if (!pProvisionChain)
		return false;

	strRDSId = pProvisionChain->getIdent();
	nPercentLoad = (unsigned int)(nMaxCost*100.0/MAX_LOAD_VALUE);
	return true;
}

RDSBook::~RDSBook()
{
	ProvisionChainMap::iterator it = _chainMap.begin();
	for(;it != _chainMap.end();it++)
	{
		if (it->second)
		{
			delete it->second;
		}
	}

	_chainMap.clear();
}


ProvisionChain::ProvisionChain( const std::string& strRDSIdent, unsigned int maxBandwidthKbps, unsigned int maxSessions )
	:_provisionCost(maxBandwidthKbps, maxSessions), _strRDSIdent(strRDSIdent)
{
	_nMaxCost = 0;
}

std::string ProvisionChain::getIdent()
{
	return _strRDSIdent;
}

bool ProvisionChain::add( const ProvisionItem& in )
{
	// locate where to start insert;
	BookChain::iterator it = _bookChain.begin();
	for (; it < _bookChain.end(); it++)
	{
		if (TIMECMP(in.st, it->et) <0)
			break;
	}

	BookItem node;
	node.st = in.st;
	node.et = in.et;
	node.instance = in.instance;
	node.bandwidth = in.bandwidth;
	node.cost = 0;

	while(TIMECMP(node.st, in.et) <0 && !_provisionCost.isOverLoad(_nMaxCost))
	{
		if (it == _bookChain.end() || TIMECMP(node.et, it->st) <=0)
		{
			// simply insert a node here
			setProvisionCost(node);
			_bookChain.insert(it, node);

			CHAIN_TRACE;
			break;
		}

		if (TIMECMP(node.st, it->st) <0)
		{
			node.instance = in.instance;
			node.bandwidth = in.bandwidth;
			node.et = it->st;

			setProvisionCost(node);
			it = _bookChain.insert(it, node); // insert front
			it++;
			node.st = node.et;
			node.et = in.et;
			CHAIN_TRACE;
			continue;
		}

		if (TIMECMP(node.st, it->st) >0)
		{
			node.et = it->et;
			it->et = node.st;

			node.instance = it->instance + in.instance;
			node.bandwidth = it->bandwidth + in.bandwidth;

			setProvisionCost(node);

			if (TIMECMP(in.et, node.et) >=0)
			{
				it = _bookChain.insert(it+1, node); // insert back
				it++;

				node.st = node.et;
				node.et = in.et;
				node.instance = in.instance;
				node.bandwidth = in.bandwidth;
			}
			else
			{
				unsigned int et = node.et;
				node.et = in.et;
				it = _bookChain.insert(it+1, node);
				it++;
				node.st = in.et;
				node.et = et;

				node.instance -= in.instance;
				node.bandwidth -= in.bandwidth;
				setProvisionCost(node);

				it = _bookChain.insert(it, node);
				it++;
			}

			node.et = in.et;
			continue;
		}

		if (TIMECMP(node.st, it->st) ==0 && TIMECMP(in.et, it->st) >=0)
		{
			if (TIMECMP(in.et, it->et) >=0)
			{
				//					it->value += in.value;
				it->instance += in.instance;
				it->bandwidth += in.bandwidth;
				setProvisionCost(*it);

				node.st = it->et;
			}
			else
			{
				node.instance = it->instance;
				node.bandwidth = it->bandwidth;
				node.et = it->et;
				node.st = in.et;

				it->et = in.et;			
				it->instance += in.instance;
				it->bandwidth += in.bandwidth;
				setProvisionCost(*it);

				setProvisionCost(node);
				_bookChain.insert(it+1, node); // insert behind
				CHAIN_TRACE;
				break;
			}
		}

		node.et = in.et;
		it++;
	}
	// end of build up chainput chain;

	CHAIN_TRACE;

	bool bRet = !_provisionCost.isOverLoad(_nMaxCost);
	return bRet;
}

void ProvisionChain::setProvisionCost( BookItem& node)
{
	node.cost = _provisionCost.evaluateCost(node.bandwidth, node.instance);

	if (node.cost > _nMaxCost)
		_nMaxCost = node.cost;
}

unsigned int ProvisionChain::getMaxCost()
{
	return _nMaxCost;
}

bool ProvisionChain::isOverLoad()
{
	return _provisionCost.isOverLoad(_nMaxCost);
}


unsigned int ProvisionCost::evaluateCost( unsigned int bandwidthKbps, unsigned int sessions )
{
	if (!_maxBandwidthKbps || !_maxInstanceCount)
		return MAX_LOAD_VALUE + 1;

	if (bandwidthKbps > _maxBandwidthKbps)
		return MAX_LOAD_VALUE + 1;

	if (sessions > _maxInstanceCount)
		return MAX_LOAD_VALUE + 1;

	int nCost1 = (int)((bandwidthKbps/(float)_maxBandwidthKbps)*MAX_LOAD_VALUE);
	int nCost2 = (int)((sessions/(float)_maxInstanceCount)*MAX_LOAD_VALUE);

	//两种独占的resource, cost以大的为准
	return __max(nCost1, nCost2);
}

void ProvisionCost::setResource( unsigned int maxBandwidthKbps, unsigned int maxSessions )
{
	_maxBandwidthKbps = maxBandwidthKbps;
	_maxInstanceCount = maxSessions;
}


}}}