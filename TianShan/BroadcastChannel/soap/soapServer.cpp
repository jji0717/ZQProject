/* soapServer.cpp
   Generated by gSOAP 2.7.10 from soapMRT.h
   Copyright(C) 2000-2008, Robert van Engelen, Genivia Inc. All Rights Reserved.
   This part of the software is released under one of the following licenses:
   GPL, the gSOAP public license, or Genivia's license for commercial use.
*/
#include "soapH.h"

SOAP_SOURCE_STAMP("@(#) soapServer.cpp ver 2.7.10 2014-11-14 06:29:55 GMT")


SOAP_FMAC5 int SOAP_FMAC6 soap_serve(struct soap *soap)
{
#ifndef WITH_FASTCGI
	unsigned int k = soap->max_keep_alive;
#endif

	do
	{
#ifdef WITH_FASTCGI
		if (FCGI_Accept() < 0)
		{
			soap->error = SOAP_EOF;
			return soap_send_fault(soap);
		}
#endif

		soap_begin(soap);

#ifndef WITH_FASTCGI
		if (soap->max_keep_alive > 0 && !--k)
			soap->keep_alive = 0;
#endif

		if (soap_begin_recv(soap))
		{	if (soap->error < SOAP_STOP)
			{
#ifdef WITH_FASTCGI
				soap_send_fault(soap);
#else 
				return soap_send_fault(soap);
#endif
			}
			soap_closesock(soap);

			continue;
		}

		if (soap_envelope_begin_in(soap)
		 || soap_recv_header(soap)
		 || soap_body_begin_in(soap)
		 || soap_serve_request(soap)
		 || (soap->fserveloop && soap->fserveloop(soap)))
		{
#ifdef WITH_FASTCGI
			soap_send_fault(soap);
#else
			return soap_send_fault(soap);
#endif
		}

#ifdef WITH_FASTCGI
		soap_destroy(soap);
		soap_end(soap);
	} while (1);
#else
	} while (soap->keep_alive);
#endif
	return SOAP_OK;
}

#ifndef WITH_NOSERVEREQUEST
SOAP_FMAC5 int SOAP_FMAC6 soap_serve_request(struct soap *soap)
{
	soap_peek_element(soap);
	if (!soap_match_tag(soap, soap->tag, "ZQ2:setup"))
		return soap_serve_ZQ2__setup(soap);
	if (!soap_match_tag(soap, soap->tag, "ZQ2:teardown"))
		return soap_serve_ZQ2__teardown(soap);
	if (!soap_match_tag(soap, soap->tag, "ZQ2:getStatus"))
		return soap_serve_ZQ2__getStatus(soap);
	if (!soap_match_tag(soap, soap->tag, "ZQ2:notifyStatus"))
		return soap_serve_ZQ2__notifyStatus(soap);
	return soap->error = SOAP_NO_METHOD;
}
#endif

SOAP_FMAC5 int SOAP_FMAC6 soap_serve_ZQ2__setup(struct soap *soap)
{	struct ZQ2__setup soap_tmp_ZQ2__setup;
	struct ZQ2__setupResponse _param_1;
	soap_default_ZQ2__setupResponse(soap, &_param_1);
	soap_default_ZQ2__setup(soap, &soap_tmp_ZQ2__setup);
	soap->encodingStyle = "http://schemas.xmlsoap.org/soap/encoding/";
	if (!soap_get_ZQ2__setup(soap, &soap_tmp_ZQ2__setup, "ZQ2:setup", NULL))
		return soap->error;
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap->error;
	soap->error = ZQ2__setup(soap, soap_tmp_ZQ2__setup.setupInfo, _param_1);
	if (soap->error)
		return soap->error;
	soap_serializeheader(soap);
	soap_serialize_ZQ2__setupResponse(soap, &_param_1);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ZQ2__setupResponse(soap, &_param_1, "ZQ2:setupResponse", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	};
	if (soap_end_count(soap)
	 || soap_response(soap, SOAP_OK)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ZQ2__setupResponse(soap, &_param_1, "ZQ2:setupResponse", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap->error;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_serve_ZQ2__teardown(struct soap *soap)
{	struct ZQ2__teardown soap_tmp_ZQ2__teardown;
	struct ZQ2__teardownResponse soap_tmp_ZQ2__teardownResponse;
	soap_default_ZQ2__teardownResponse(soap, &soap_tmp_ZQ2__teardownResponse);
	soap_default_ZQ2__teardown(soap, &soap_tmp_ZQ2__teardown);
	soap->encodingStyle = "http://schemas.xmlsoap.org/soap/encoding/";
	if (!soap_get_ZQ2__teardown(soap, &soap_tmp_ZQ2__teardown, "ZQ2:teardown", NULL))
		return soap->error;
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap->error;
	soap->error = ZQ2__teardown(soap, soap_tmp_ZQ2__teardown.sessionId, soap_tmp_ZQ2__teardownResponse.ret);
	if (soap->error)
		return soap->error;
	soap_serializeheader(soap);
	soap_serialize_ZQ2__teardownResponse(soap, &soap_tmp_ZQ2__teardownResponse);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ZQ2__teardownResponse(soap, &soap_tmp_ZQ2__teardownResponse, "ZQ2:teardownResponse", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	};
	if (soap_end_count(soap)
	 || soap_response(soap, SOAP_OK)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ZQ2__teardownResponse(soap, &soap_tmp_ZQ2__teardownResponse, "ZQ2:teardownResponse", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap->error;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_serve_ZQ2__getStatus(struct soap *soap)
{	struct ZQ2__getStatus soap_tmp_ZQ2__getStatus;
	struct ZQ2__getStatusResponse _param_2;
	soap_default_ZQ2__getStatusResponse(soap, &_param_2);
	soap_default_ZQ2__getStatus(soap, &soap_tmp_ZQ2__getStatus);
	soap->encodingStyle = "http://schemas.xmlsoap.org/soap/encoding/";
	if (!soap_get_ZQ2__getStatus(soap, &soap_tmp_ZQ2__getStatus, "ZQ2:getStatus", NULL))
		return soap->error;
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap->error;
	soap->error = ZQ2__getStatus(soap, soap_tmp_ZQ2__getStatus.sessionId, _param_2);
	if (soap->error)
		return soap->error;
	soap_serializeheader(soap);
	soap_serialize_ZQ2__getStatusResponse(soap, &_param_2);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ZQ2__getStatusResponse(soap, &_param_2, "ZQ2:getStatusResponse", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	};
	if (soap_end_count(soap)
	 || soap_response(soap, SOAP_OK)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ZQ2__getStatusResponse(soap, &_param_2, "ZQ2:getStatusResponse", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap->error;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_serve_ZQ2__notifyStatus(struct soap *soap)
{	struct ZQ2__notifyStatus soap_tmp_ZQ2__notifyStatus;
	struct ZQ2__notifyStatusResponse _param_3;
	soap_default_ZQ2__notifyStatusResponse(soap, &_param_3);
	soap_default_ZQ2__notifyStatus(soap, &soap_tmp_ZQ2__notifyStatus);
	soap->encodingStyle = "http://schemas.xmlsoap.org/soap/encoding/";
	if (!soap_get_ZQ2__notifyStatus(soap, &soap_tmp_ZQ2__notifyStatus, "ZQ2:notifyStatus", NULL))
		return soap->error;
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap->error;
	soap->error = ZQ2__notifyStatus(soap, _param_3);
	if (soap->error)
		return soap->error;
	soap_serializeheader(soap);
	soap_serialize_ZQ2__notifyStatusResponse(soap, &_param_3);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ZQ2__notifyStatusResponse(soap, &_param_3, "ZQ2:notifyStatusResponse", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	};
	if (soap_end_count(soap)
	 || soap_response(soap, SOAP_OK)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ZQ2__notifyStatusResponse(soap, &_param_3, "ZQ2:notifyStatusResponse", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap->error;
	return soap_closesock(soap);
}

/* End of soapServer.cpp */
