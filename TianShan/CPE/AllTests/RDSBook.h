// RDSBook.h: interface for the RDSBook class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RDSBOOK_H__EBC952C6_F07C_43C3_8737_C66201172756__INCLUDED_)
#define AFX_RDSBOOK_H__EBC952C6_F07C_43C3_8737_C66201172756__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning(disable: 4786)


#include <deque>
#include <map>
#include <string>
#include <vector>



namespace TianShanIce{ 
namespace Storage{ 
namespace PushContentModule{


#define MAX_LOAD_VALUE (10000)



class ProvisionCost
{
public:
	ProvisionCost(unsigned int maxBandwidthKbps, unsigned int maxSessions)
	{
		setResource(maxBandwidthKbps, maxSessions);
	}

	ProvisionCost()
	{
		_maxBandwidthKbps = 0;
		_maxInstanceCount = 0;
	}

	bool isOverLoad(unsigned int nCost)
	{
		return (nCost > MAX_LOAD_VALUE);
	}

	/// evaluate the cost per given session count and total allocated bandwidth
	///@param[in] bandwidthKbps to specify the allocated bandwidth in Kbps
	///@param[in] sessions to specify the allocated session instances
	///@return a cost in the range of [0, MAX_LOAD_VALUE+] at the given load level:
	///		0 - fully available
	///		MAX_LOAD_VALUE	 - just available and no more could accept
	///		MAX_LOAD_VALUE+	 - not available	
	unsigned int evaluateCost(unsigned int bandwidthKbps, unsigned int sessions);

	void setResource(unsigned int maxBandwidthKbps, unsigned int maxSessions);

protected:

	unsigned int _maxBandwidthKbps;
	unsigned int _maxInstanceCount;
};

class ProvisionChain
{
public:
	struct ProvisionItem
	{
		unsigned int st, et;
		unsigned int bandwidth;			//in Kbps
		unsigned int instance;
	};

	ProvisionChain(const std::string& strRDSIdent, unsigned int maxBandwidthKbps, unsigned int maxSessions);

	//return false if over load, else return true
	bool add(const ProvisionItem& in);

	unsigned int getMaxCost();

	std::string getIdent();

	bool isOverLoad();

protected:

	struct BookItem : public ProvisionItem
	{
		unsigned int cost;
	};

	typedef std::deque< BookItem > BookChain;

	//update the cost in BookItem,
	//and update the max cost
	void setProvisionCost(BookItem& node);

protected:

	BookChain		_bookChain;
	unsigned int	_nMaxCost;
	std::string		_strRDSIdent;
	ProvisionCost	_provisionCost;
};

class RDSBook  
{
public:
	~RDSBook();

	struct RDSCost
	{
		std::string  strIdent;
		unsigned int nCost;
	};
	typedef std::vector<RDSCost>	RDSCostList;

	// add current exist RDS id
	void addRDS(const std::string& strRDSId, unsigned int maxBandwidthKbps,  unsigned int maxInstance);
	
	// must call addRDS before this, because if the RDS id not found, item will not be added
	void addBookedItem(const std::string& strRDSId, unsigned int timeStart, unsigned int timeStop, unsigned int bandwidthKbps);

	// id, the returned booked RDS id
	bool bookRDS(unsigned int timeStart, unsigned int timeStop, unsigned int bandwidthKbps, std::string& strRDSId, unsigned int& nPercentLoad, RDSCostList& costList);

protected:
	
	typedef std::map <std::string, ProvisionChain*>	 ProvisionChainMap;
	ProvisionChainMap		_chainMap;

};


}}}

#endif // !defined(AFX_RDSBOOK_H__EBC952C6_F07C_43C3_8737_C66201172756__INCLUDED_)
