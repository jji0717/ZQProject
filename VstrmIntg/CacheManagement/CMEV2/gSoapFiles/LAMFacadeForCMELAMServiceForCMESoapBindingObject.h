/* LAMFacadeForCMELAMServiceForCMESoapBindingObject.h
   Generated by gSOAP 2.7.10 from LAMFacadeForCME.h
   Copyright(C) 2000-2008, Robert van Engelen, Genivia Inc. All Rights Reserved.
   This part of the software is released under one of the following licenses:
   GPL, the gSOAP public license, or Genivia's license for commercial use.
*/

#ifndef LAMFacadeForCMELAMServiceForCMESoapBindingObject_H
#define LAMFacadeForCMELAMServiceForCMESoapBindingObject_H
#include "LAMFacadeForCMEH.h"

/******************************************************************************\
 *                                                                            *
 * Service Object                                                             *
 *                                                                            *
\******************************************************************************/

class LAMServiceForCMESoapBindingService : public soap
{    public:
	LAMServiceForCMESoapBindingService()
	{ static const struct Namespace namespaces[] =
{
	{"SOAP-ENV", "http://schemas.xmlsoap.org/soap/envelope/", "http://www.w3.org/*/soap-envelope", NULL},
	{"SOAP-ENC", "http://schemas.xmlsoap.org/soap/encoding/", "http://www.w3.org/*/soap-encoding", NULL},
	{"xsi", "http://www.w3.org/2001/XMLSchema-instance", "http://www.w3.org/*/XMLSchema-instance", NULL},
	{"xsd", "http://www.w3.org/2001/XMLSchema", "http://www.w3.org/*/XMLSchema", NULL},
	{"ns1", "http://www.i-zq.com/services/LAMServiceForCME", NULL, NULL},
	{"ns2", "http://cme.integration.am.izq.com", NULL, NULL},
	{NULL, NULL, NULL, NULL}
};
	if (!this->namespaces) this->namespaces = namespaces; };
	virtual ~LAMServiceForCMESoapBindingService() { };
	/// Bind service to port (returns master socket or SOAP_INVALID_SOCKET)
	virtual	SOAP_SOCKET bind(const char *host, int port, int backlog) { return soap_bind(this, host, port, backlog); };
	/// Accept next request (returns socket or SOAP_INVALID_SOCKET)
	virtual	SOAP_SOCKET accept() { return soap_accept(this); };
	/// Serve this request (returns error code or SOAP_OK)
	virtual	int serve() { return soap_serve(this); };
};

/******************************************************************************\
 *                                                                            *
 * Service Operations (you should define these globally)                      *
 *                                                                            *
\******************************************************************************/


SOAP_FMAC5 int SOAP_FMAC6 ns2__addCache(struct soap*, std::string _providerID, std::string _providerAssetID, std::string _clusterID, LONG64 &_addCacheReturn);

SOAP_FMAC5 int SOAP_FMAC6 ns2__deleteCache(struct soap*, std::string _providerID, std::string _providerAssetID, std::string _clusterID, LONG64 &_deleteCacheReturn);

SOAP_FMAC5 int SOAP_FMAC6 ns2__getAsset(struct soap*, std::string _providerID, std::string _providerAssetID, std::string _clusterID, struct ns2__getAssetResponse &_param_1);

SOAP_FMAC5 int SOAP_FMAC6 ns2__listAsset(struct soap*, std::string _providerID, std::string _clusterID, struct ns2__listAssetResponse &_param_2);

SOAP_FMAC5 int SOAP_FMAC6 ns2__listProvider(struct soap*, struct ns2__listProviderResponse &_param_3);

SOAP_FMAC5 int SOAP_FMAC6 ns2__getClusterConfig(struct soap*, std::string clusterID, struct ns2__getClusterConfigResponse &_param_4);

SOAP_FMAC5 int SOAP_FMAC6 ns2__handshake(struct soap*, std::string _cmeVersion, LONG64 _cmeStatus, std::string _cmeEndPoint, LONG64 &_handshakeReturn);

SOAP_FMAC5 int SOAP_FMAC6 ns2__listCluster(struct soap*, struct ns2__listClusterResponse &_param_5);

SOAP_FMAC5 int SOAP_FMAC6 ns2__getExportURL(struct soap*, std::string _providerID, std::string _providerAssetID, std::string _clusterID, struct ns2__getExportURLResponse &_param_6);

SOAP_FMAC5 int SOAP_FMAC6 ns2__copyFrom(struct soap*, std::string _providerID, std::string _providerAssetID, std::string _clusterID, std::string _contentType, std::string _contentSubtype, std::string _sourceURL, std::string _userName, std::string _password, LONG64 _bitrate, std::string _windowStart, std::string _windowEnd, LONG64 &_copyFromReturn);

#endif
