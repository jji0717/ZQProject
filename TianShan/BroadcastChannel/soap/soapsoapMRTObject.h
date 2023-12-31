/* soapsoapMRTObject.h
   Generated by gSOAP 2.7.10 from soapMRT.h
   Copyright(C) 2000-2008, Robert van Engelen, Genivia Inc. All Rights Reserved.
   This part of the software is released under one of the following licenses:
   GPL, the gSOAP public license, or Genivia's license for commercial use.
*/

#ifndef soapsoapMRTObject_H
#define soapsoapMRTObject_H
#include "soapH.h"

/******************************************************************************\
 *                                                                            *
 * Service Object                                                             *
 *                                                                            *
\******************************************************************************/

class soapMRTService : public soap
{    public:
	soapMRTService()
	{ static const struct Namespace namespaces[] =
{
	{"SOAP-ENV", "http://schemas.xmlsoap.org/soap/envelope/", "http://www.w3.org/*/soap-envelope", NULL},
	{"SOAP-ENC", "http://schemas.xmlsoap.org/soap/encoding/", "http://www.w3.org/*/soap-encoding", NULL},
	{"xsi", "http://www.w3.org/2001/XMLSchema-instance", "http://www.w3.org/*/XMLSchema-instance", NULL},
	{"xsd", "http://www.w3.org/2001/XMLSchema", "http://www.w3.org/*/XMLSchema", NULL},
	{"wsdl", "http://tempuri.org/wsdl.xsd", NULL, NULL},
	{"ZQ2", "ZQ:soapMRT", NULL, NULL},
	{NULL, NULL, NULL, NULL}
};
	if (!this->namespaces) this->namespaces = namespaces; };
	virtual ~soapMRTService() { };
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


SOAP_FMAC5 int SOAP_FMAC6 ZQ2__setup(struct soap*, ZQ2__setupInfo *setupInfo, struct ZQ2__setupResponse &_param_1);

SOAP_FMAC5 int SOAP_FMAC6 ZQ2__teardown(struct soap*, std::string sessionId, bool &ret);

SOAP_FMAC5 int SOAP_FMAC6 ZQ2__getStatus(struct soap*, std::string sessionId, struct ZQ2__getStatusResponse &_param_2);

SOAP_FMAC5 int SOAP_FMAC6 ZQ2__notifyStatus(struct soap*, struct ZQ2__notifyStatusResponse &_param_3);

#endif
