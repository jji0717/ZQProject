#include "ZQ_common_conf.h"
#include "Log.h"
#include "CMELAMSOAPClient.h"

#define CME_SOAP_CLIENT_CONN_TIMEOUT    10    // seconds
#define CME_SOAP_CLIENT_SEND_TIMEOUT    10    // seconds
#define CME_SOAP_CLIENT_RECV_TIMEOUT    10    // seconds
#define CMEV2_VERSION	"2.0.0"

extern void logSoapFault(struct soap* soap, ZQ::common::Log& log);

namespace CacheManagement {

struct Namespace lamnamespaces[] = 
{
	{"SOAP-ENV", "http://schemas.xmlsoap.org/soap/envelope/", "http://www.w3.org/*/soap-envelope", NULL},
	{"SOAP-ENC", "http://schemas.xmlsoap.org/soap/encoding/", "http://www.w3.org/*/soap-encoding", NULL},
	{"xsi", "http://www.w3.org/2001/XMLSchema-instance", "http://www.w3.org/*/XMLSchema-instance", NULL},
	{"xsd", "http://www.w3.org/2001/XMLSchema", "http://www.w3.org/*/XMLSchema", NULL},
	{"ns1", "http://10.50.18.1:8080/services/LAMServiceForCME", NULL, NULL},
	{"ns2", "http://cme.integration.am.izq.com", NULL, NULL},
	{NULL, NULL, NULL, NULL}
};

CMELAMSOAPClient::CMELAMSOAPClient(std::string lamEndpoint, std::string cmeEndpoint)
{
	_lamEndpoint = lamEndpoint;
	_cmeEndpoint = cmeEndpoint;
}

CMELAMSOAPClient::~CMELAMSOAPClient()
{
	// make sure resource are released
	unintialize();
}

bool CMELAMSOAPClient::doHandshake()
{
    LONG64 hsRet = 0;
	int soapRet = SOAP_OK;

	glog(ZQ::common::Log::L_INFO, CLOGFMT(CMELAMSOAPClient, "Begin handshake with LAM endpoint %s"), 
		_lamEndpoint.c_str());
	
	// call LAM soap interface "handshake"
	_lamSoapClient.soap->connect_timeout = CME_SOAP_CLIENT_CONN_TIMEOUT;
	soapRet = _lamSoapClient.ns2__handshake(CMEV2_VERSION, 0, _cmeEndpoint, hsRet);

    if(SOAP_OK != soapRet)
    {
		glog(ZQ::common::Log::L_INFO, CLOGFMT(CMELAMSOAPClient, "handshake failed with SOAP error %d"), 
			soapRet);
		// soap level error
		logSoapFault(_lamSoapClient.soap, glog);
		
		return false;
    }

	// invoke successfully
	if(0 == hsRet)
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(CMELAMSOAPClient, "handshake with LAM succeeded"));
		
		return true;
	}
	else
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(CMELAMSOAPClient, "handshake with LAM failed with code %llu"), 
			hsRet);
	}

	return true;
}  // handshake()

bool CMELAMSOAPClient::doListProvider()
{
    struct ns2__listProviderResponse listProvRes;
	int soapRet = SOAP_OK;

	glog(ZQ::common::Log::L_INFO, CLOGFMT(CMELAMSOAPClient, "Begin listProvider from LAM endpoint %s"), 
		_lamEndpoint.c_str());

	// call LAM soap interface "listProvider"
	_lamSoapClient.soap->connect_timeout = CME_SOAP_CLIENT_CONN_TIMEOUT;
	soapRet = _lamSoapClient.ns2__listProvider(listProvRes);

	if (SOAP_OK != soapRet)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(CMELAMSOAPClient, "listProvider rom LAM failed with SOAP error %d"), 
			soapRet);
		// soap level error
		logSoapFault(_lamSoapClient.soap, glog);

		return false;
	}
	
	glog(ZQ::common::Log::L_INFO, CLOGFMT(CMELAMSOAPClient, "listProvider from LAM succeeded with return code %lld"), 
		listProvRes.listProviderReturn->returnCode);

	if (0 == listProvRes.listProviderReturn->returnCode)
	{
		unsigned int i=0, count=0;
		ns1__ProviderList *pids=listProvRes.listProviderReturn;
		count = (unsigned int) pids->ProviderID.size();

		// Loop over the list of providers, supplying each
		// to our master list.
		//
		for (i=0; i < count; i++)
		{
//			lookupPID((char *)pids->ProviderID[i].c_str());
		}  // for i
	}

	return true;
}

bool CMELAMSOAPClient::listCluster(LAMCLUSTERS& lamClusters)
{
	ns2__listClusterResponse listCluRes;
	int soapRet = SOAP_OK;
	bool ret;

	glog(ZQ::common::Log::L_INFO, CLOGFMT(CMELAMSOAPClient, "Begin listCluster from LAM endpoint %s"), 
		_lamEndpoint.c_str());

	// call LAM soap interface "listProvider"
	_lamSoapClient.soap->connect_timeout = CME_SOAP_CLIENT_CONN_TIMEOUT;
	soapRet = _lamSoapClient.ns2__listCluster(listCluRes);  // Ask for clusters

	if (SOAP_OK != soapRet)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(CMELAMSOAPClient, "listCluster from LAM failed with SOAP error %d"), 
			soapRet);
		// soap level error
		logSoapFault(_lamSoapClient.soap, glog);

		return false;
	}

	ns1__ClusterList *cluList = listCluRes.listClusterReturn;
    unsigned int i, count;

	count = (unsigned int) cluList->clusters.size();

	glog(ZQ::common::Log::L_INFO, CLOGFMT(CMELAMSOAPClient, "listCluster from LAM returend %d cluster(s)"), count);
	
	// save the cluster into _cacheStorages
    for (i=0; i < count; i++)
    {
		glog(ZQ::common::Log::L_INFO, CLOGFMT(CMELAMSOAPClient, "cluster No.%d: clusterID=%s, cacheLevel=%d, cacheable=%d"),
			i+1, cluList->clusters[i]->clusterID.c_str(), cluList->clusters[i]->cacheLevel, cluList->clusters[i]->cacheable);

		Cluster* pCluster = new Cluster();

		pCluster->clusterID = cluList->clusters[i]->clusterID;
		pCluster->cacheLevel = cluList->clusters[i]->cacheLevel;
		pCluster->cacheable = cluList->clusters[i]->cacheable;

		// get cluster configuraiton
		ret = getClusterConfig(*pCluster);
		if(!ret)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(CMELAMSOAPClient, "cluster No.%d: getClusterConfig(%s) failed, abort doListCluster()"),
				i+1, cluList->clusters[i]->clusterID.c_str());

			delete pCluster;

			continue;
		}
		// save this cluster
		lamClusters.push_back(pCluster);

	}  // for i

	return true;
}

bool CMELAMSOAPClient::getClusterConfig(Cluster& cluster)
{
	//retrieve the cluster id
	struct ns2__getClusterConfigResponse cluConfigRes;
	int soapRet = SOAP_OK;

	glog(ZQ::common::Log::L_INFO, CLOGFMT(CMELAMSOAPClient, "Begin getClusterConfig for %s from LAM endpoint %s"), 
		cluster.clusterID.c_str(), _lamEndpoint.c_str());

	// call LAM soap interface "getClusterConfig"
	_lamSoapClient.soap->connect_timeout = CME_SOAP_CLIENT_CONN_TIMEOUT;
	soapRet = _lamSoapClient.ns2__getClusterConfig(cluster.clusterID, cluConfigRes);


	if (SOAP_OK != soapRet)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(CMELAMSOAPClient, "getClusterConfig failed with SOAP error %d"), 
			soapRet);
		// soap level error
		logSoapFault(_lamSoapClient.soap, glog);

		return false;
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(CMELAMSOAPClient,"Cluster %s: state=%s free=%llu MB used=%llu MB total=%llu MB IPs=%s, ServiceGroup=%s"), 
			cluster.clusterID.c_str(), cluConfigRes.state==0 ? "Operational" : "OutOfService", cluConfigRes.freeSize/1000000, 
		   (cluConfigRes.totalSize - cluConfigRes.freeSize)/1000000, cluConfigRes.totalSize/1000000, 
		    cluConfigRes.nodeIPs.c_str(), cluConfigRes.serviceGroup.c_str());

	if("" == cluConfigRes.nodeIPs)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(CMELAMSOAPClient, "getClusterConfig returned empty NodeIPs from cluster %s, please check LAM configuration"), 
			cluster.clusterID.c_str());

		return false;
	}

	cluster.status = (int)cluConfigRes.state;
	cluster.freeSize = cluConfigRes.freeSize;
	cluster.totalSize = cluConfigRes.totalSize;
	cluster.nodeIPs = cluConfigRes.nodeIPs;
	cluster.serviceGroup = cluConfigRes.serviceGroup;

    return true;
}  // _GetClusterConfig()

bool CMELAMSOAPClient::initialize()
{
	bool bResult = false;

	_lamSoapClient.soap->namespaces = lamnamespaces;
	_lamSoapClient.endpoint = _lamEndpoint.c_str();
	
	_lamSoapClient.soap->connect_timeout = CME_SOAP_CLIENT_CONN_TIMEOUT;  // parameter in seconds
	_lamSoapClient.soap->send_timeout    = CME_SOAP_CLIENT_SEND_TIMEOUT;
	_lamSoapClient.soap->recv_timeout    = CME_SOAP_CLIENT_RECV_TIMEOUT;

#ifdef ZQ_OS_LINUX
	_lamSoapClient.soap->socket_flags = MSG_NOSIGNAL;
#endif
	// do unitize() first, caller may retry this initiazlie call
	unintialize();

	bResult = doHandshake();
	if(!bResult)
	{
		return false;
	}

	return true;
}

void CMELAMSOAPClient::unintialize()
{
	// nothing need to be done now
}

} // end of namespace