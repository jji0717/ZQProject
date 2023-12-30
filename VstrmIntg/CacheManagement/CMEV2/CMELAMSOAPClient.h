#ifndef _CMELAMSOAPCLIENT_H
#define _CMELAMSOAPCLIENT_H

#include "LAMFacadeForCMELAMServiceForCMESoapBindingProxy.h"


namespace CacheManagement {

class Cluster 
{
public:
	Cluster() {};
	virtual ~Cluster() {};

public:
	std::string  clusterID;
	int          cacheLevel;
	int			 cacheable;
	int			 status;
	uint64		 totalSize;
	uint64       freeSize;
	std::string  nodeIPs;
	std::string  serviceGroup;
};
typedef std::vector<Cluster*>  LAMCLUSTERS;

class CMELAMSOAPClient
{
public:
	CMELAMSOAPClient(std::string lamEndpoint, std::string cmeEndpoint);
	virtual ~CMELAMSOAPClient();

public:
	bool initialize();
	bool listCluster(LAMCLUSTERS& lamClusters);
	
protected:
	void unintialize();

private:
	std::string	_lamEndpoint;
	std::string _cmeEndpoint;
	
	LAMServiceForCMESoapBinding  _lamSoapClient;

	// need retry for following calls
	bool doHandshake();
	bool getClusterConfig(Cluster& cluster);
	bool doListProvider();
};



} // end of namespace
#endif

