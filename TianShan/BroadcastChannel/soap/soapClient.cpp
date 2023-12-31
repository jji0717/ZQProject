/* soapClient.cpp
   Generated by gSOAP 2.7.10 from soapMRT.h
   Copyright(C) 2000-2008, Robert van Engelen, Genivia Inc. All Rights Reserved.
   This part of the software is released under one of the following licenses:
   GPL, the gSOAP public license, or Genivia's license for commercial use.
*/
#include "soapH.h"

SOAP_SOURCE_STAMP("@(#) soapClient.cpp ver 2.7.10 2014-11-14 06:29:55 GMT")


SOAP_FMAC5 int SOAP_FMAC6 soap_call_ZQ2__setup(struct soap *soap, const char *soap_endpoint, const char *soap_action, ZQ2__setupInfo *setupInfo, struct ZQ2__setupResponse &_param_1)
{	struct ZQ2__setup soap_tmp_ZQ2__setup;
	if (!soap_endpoint)
		soap_endpoint = "http://localhost:8895";
	if (!soap_action)
		soap_action = "";
	soap->encodingStyle = "http://schemas.xmlsoap.org/soap/encoding/";
	soap_tmp_ZQ2__setup.setupInfo = setupInfo;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize_ZQ2__setup(soap, &soap_tmp_ZQ2__setup);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ZQ2__setup(soap, &soap_tmp_ZQ2__setup, "ZQ2:setup", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ZQ2__setup(soap, &soap_tmp_ZQ2__setup, "ZQ2:setup", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	soap_default_ZQ2__setupResponse(soap, &_param_1);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	soap_get_ZQ2__setupResponse(soap, &_param_1, "ZQ2:setupResponse", "");
	if (soap->error)
	{	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
			return soap_recv_fault(soap);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ZQ2__teardown(struct soap *soap, const char *soap_endpoint, const char *soap_action, std::string sessionId, bool &ret)
{	struct ZQ2__teardown soap_tmp_ZQ2__teardown;
	struct ZQ2__teardownResponse *soap_tmp_ZQ2__teardownResponse;
	if (!soap_endpoint)
		soap_endpoint = "http://localhost:8895";
	if (!soap_action)
		soap_action = "";
	soap->encodingStyle = "http://schemas.xmlsoap.org/soap/encoding/";
	soap_tmp_ZQ2__teardown.sessionId = sessionId;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize_ZQ2__teardown(soap, &soap_tmp_ZQ2__teardown);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ZQ2__teardown(soap, &soap_tmp_ZQ2__teardown, "ZQ2:teardown", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ZQ2__teardown(soap, &soap_tmp_ZQ2__teardown, "ZQ2:teardown", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	soap_default_bool(soap, &ret);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	soap_tmp_ZQ2__teardownResponse = soap_get_ZQ2__teardownResponse(soap, NULL, "ZQ2:teardownResponse", "");
	if (soap->error)
	{	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
			return soap_recv_fault(soap);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	ret = soap_tmp_ZQ2__teardownResponse->ret;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ZQ2__getStatus(struct soap *soap, const char *soap_endpoint, const char *soap_action, std::string sessionId, struct ZQ2__getStatusResponse &_param_2)
{	struct ZQ2__getStatus soap_tmp_ZQ2__getStatus;
	if (!soap_endpoint)
		soap_endpoint = "http://localhost:8895";
	if (!soap_action)
		soap_action = "";
	soap->encodingStyle = "http://schemas.xmlsoap.org/soap/encoding/";
	soap_tmp_ZQ2__getStatus.sessionId = sessionId;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize_ZQ2__getStatus(soap, &soap_tmp_ZQ2__getStatus);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ZQ2__getStatus(soap, &soap_tmp_ZQ2__getStatus, "ZQ2:getStatus", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ZQ2__getStatus(soap, &soap_tmp_ZQ2__getStatus, "ZQ2:getStatus", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	soap_default_ZQ2__getStatusResponse(soap, &_param_2);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	soap_get_ZQ2__getStatusResponse(soap, &_param_2, "ZQ2:getStatusResponse", "");
	if (soap->error)
	{	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
			return soap_recv_fault(soap);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ZQ2__notifyStatus(struct soap *soap, const char *soap_endpoint, const char *soap_action, struct ZQ2__notifyStatusResponse &_param_3)
{	struct ZQ2__notifyStatus soap_tmp_ZQ2__notifyStatus;
	if (!soap_endpoint)
		soap_endpoint = "http://localhost:8895";
	if (!soap_action)
		soap_action = "";
	soap->encodingStyle = "http://schemas.xmlsoap.org/soap/encoding/";
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize_ZQ2__notifyStatus(soap, &soap_tmp_ZQ2__notifyStatus);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ZQ2__notifyStatus(soap, &soap_tmp_ZQ2__notifyStatus, "ZQ2:notifyStatus", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ZQ2__notifyStatus(soap, &soap_tmp_ZQ2__notifyStatus, "ZQ2:notifyStatus", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	soap_default_ZQ2__notifyStatusResponse(soap, &_param_3);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	soap_get_ZQ2__notifyStatusResponse(soap, &_param_3, "ZQ2:notifyStatusResponse", "");
	if (soap->error)
	{	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
			return soap_recv_fault(soap);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

/* End of soapClient.cpp */
