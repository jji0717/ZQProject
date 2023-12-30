#ifndef __PROVISIONSTORE_H__ 
#define __PROVISIONSTORE_H__

#include "ProvisionResourceBook.h"
#include "Locks.h"
#include "Ice/Identity.h"
#include "Log.h"
#include <string>
#include <map>

class ProvisionStore
{
public:
	struct ProvisionStoreTtem
	{
		int64 timeStart;
		int64 timeStop;
		unsigned int bandwidthKBps;
		std::string methodType;
	};
    
	ProvisionStore(void);
	~ProvisionStore(void);

	bool addProvisionStore(const Ice::Identity& ident, const std::string& methodType, unsigned int timeStart, unsigned int timeEnd, unsigned int bandwidthKBps);
	bool addResourceBookForProvision(ZQTianShan::ContentProvision::ProvisionResourceBook& presourcebook);
	bool delProvisionStore(Ice::Identity& itent);
	ProvisionStoreTtem* findProvisionStore(const Ice::Identity& ident);
    bool updateProvisionStore(const Ice::Identity& ident, const std::string& methodType, unsigned int timeStart, unsigned int timeEnd, unsigned int bandwidthKBps);
	struct xxxcmp 
	{
		bool operator()( const Ice::Identity& a , const Ice::Identity& b) const
		{
			if( a.category < b.category )
			{
				return true;
			}
			else if( a.category == b.category )
			{
				return a.name < b.name;
			}
			else
			{
				return false;
			}
		}
	};
	typedef std::map<Ice::Identity,ProvisionStoreTtem,xxxcmp> Provisionmap;
	Provisionmap _provisionMap;
private:
	
	ZQ::common::Mutex		_psLock;

};
#endif//__PROVISIONSTORE_H__

