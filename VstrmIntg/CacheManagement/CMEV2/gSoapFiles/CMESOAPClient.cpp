/* CMESOAPClient.cpp
   Generated by gSOAP 2.7.10 from CMESOAPService.h
   Copyright(C) 2000-2008, Robert van Engelen, Genivia Inc. All Rights Reserved.
   This part of the software is released under one of the following licenses:
   GPL, the gSOAP public license, or Genivia's license for commercial use.
*/
#include "CMESOAPH.h"

namespace CMESOAP {

SOAP_SOURCE_STAMP("@(#) CMESOAPClient.cpp ver 2.7.10 2014-03-12 02:23:34 GMT")


SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns1__cacheNotification(struct soap *soap, const char *soap_endpoint, const char *soap_action, std::string _providerID, std::string _providerAssetID, std::string _clusterID, LONG64 _status, struct ns1__cacheNotificationResponse &_param_1)
{	struct ns1__cacheNotification soap_tmp_ns1__cacheNotification;
	if (!soap_endpoint)
		soap_endpoint = "http://www.schange.com/services/CMEService";
	soap->encodingStyle = "http://schemas.xmlsoap.org/soap/encoding/";
	soap_tmp_ns1__cacheNotification._providerID = _providerID;
	soap_tmp_ns1__cacheNotification._providerAssetID = _providerAssetID;
	soap_tmp_ns1__cacheNotification._clusterID = _clusterID;
	soap_tmp_ns1__cacheNotification._status = _status;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize_ns1__cacheNotification(soap, &soap_tmp_ns1__cacheNotification);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns1__cacheNotification(soap, &soap_tmp_ns1__cacheNotification, "ns1:cacheNotification", "")
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
	 || soap_put_ns1__cacheNotification(soap, &soap_tmp_ns1__cacheNotification, "ns1:cacheNotification", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	soap_default_ns1__cacheNotificationResponse(soap, &_param_1);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	soap_get_ns1__cacheNotificationResponse(soap, &_param_1, "ns1:cacheNotificationResponse", "");
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

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns1__sessionNotification(struct soap *soap, const char *soap_endpoint, const char *soap_action, std::string _providerID, std::string _providerAssetID, int _func, std::string _clusterID, std::string _sessionID, std::string _timeStamp, struct ns1__sessionNotificationResponse &_param_2)
{	struct ns1__sessionNotification soap_tmp_ns1__sessionNotification;
	if (!soap_endpoint)
		soap_endpoint = "http://www.schange.com/services/CMEService";
	soap->encodingStyle = "http://schemas.xmlsoap.org/soap/encoding/";
	soap_tmp_ns1__sessionNotification._providerID = _providerID;
	soap_tmp_ns1__sessionNotification._providerAssetID = _providerAssetID;
	soap_tmp_ns1__sessionNotification._func = _func;
	soap_tmp_ns1__sessionNotification._clusterID = _clusterID;
	soap_tmp_ns1__sessionNotification._sessionID = _sessionID;
	soap_tmp_ns1__sessionNotification._timeStamp = _timeStamp;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize_ns1__sessionNotification(soap, &soap_tmp_ns1__sessionNotification);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns1__sessionNotification(soap, &soap_tmp_ns1__sessionNotification, "ns1:sessionNotification", "")
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
	 || soap_put_ns1__sessionNotification(soap, &soap_tmp_ns1__sessionNotification, "ns1:sessionNotification", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	soap_default_ns1__sessionNotificationResponse(soap, &_param_2);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	soap_get_ns1__sessionNotificationResponse(soap, &_param_2, "ns1:sessionNotificationResponse", "");
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

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns1__importNotification(struct soap *soap, const char *soap_endpoint, const char *soap_action, std::string _providerID, std::string _providerAssetID, std::string _clusterID, LONG64 _increment, LONG64 _lifetime, LONG64 &_importNotificationReturn)
{	struct ns1__importNotification soap_tmp_ns1__importNotification;
	struct ns1__importNotificationResponse *soap_tmp_ns1__importNotificationResponse;
	if (!soap_endpoint)
		soap_endpoint = "http://www.schange.com/services/CMEService";
	soap->encodingStyle = "http://schemas.xmlsoap.org/soap/encoding/";
	soap_tmp_ns1__importNotification._providerID = _providerID;
	soap_tmp_ns1__importNotification._providerAssetID = _providerAssetID;
	soap_tmp_ns1__importNotification._clusterID = _clusterID;
	soap_tmp_ns1__importNotification._increment = _increment;
	soap_tmp_ns1__importNotification._lifetime = _lifetime;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize_ns1__importNotification(soap, &soap_tmp_ns1__importNotification);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns1__importNotification(soap, &soap_tmp_ns1__importNotification, "ns1:importNotification", "")
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
	 || soap_put_ns1__importNotification(soap, &soap_tmp_ns1__importNotification, "ns1:importNotification", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	soap_default_LONG64(soap, &_importNotificationReturn);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	soap_tmp_ns1__importNotificationResponse = soap_get_ns1__importNotificationResponse(soap, NULL, "ns1:importNotificationResponse", "");
	if (soap->error)
	{	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
			return soap_recv_fault(soap);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	_importNotificationReturn = soap_tmp_ns1__importNotificationResponse->_importNotificationReturn;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns1__deletableNotification(struct soap *soap, const char *soap_endpoint, const char *soap_action, std::string _providerID, std::string _providerAssetID, std::string _clusterID, LONG64 &_deletableNotificationReturn)
{	struct ns1__deletableNotification soap_tmp_ns1__deletableNotification;
	struct ns1__deletableNotificationResponse *soap_tmp_ns1__deletableNotificationResponse;
	if (!soap_endpoint)
		soap_endpoint = "http://www.schange.com/services/CMEService";
	soap->encodingStyle = "http://schemas.xmlsoap.org/soap/encoding/";
	soap_tmp_ns1__deletableNotification._providerID = _providerID;
	soap_tmp_ns1__deletableNotification._providerAssetID = _providerAssetID;
	soap_tmp_ns1__deletableNotification._clusterID = _clusterID;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize_ns1__deletableNotification(soap, &soap_tmp_ns1__deletableNotification);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns1__deletableNotification(soap, &soap_tmp_ns1__deletableNotification, "ns1:deletableNotification", "")
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
	 || soap_put_ns1__deletableNotification(soap, &soap_tmp_ns1__deletableNotification, "ns1:deletableNotification", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	soap_default_LONG64(soap, &_deletableNotificationReturn);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	soap_tmp_ns1__deletableNotificationResponse = soap_get_ns1__deletableNotificationResponse(soap, NULL, "ns1:deletableNotificationResponse", "");
	if (soap->error)
	{	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
			return soap_recv_fault(soap);
		return soap_closesock(soap);
	}
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	_deletableNotificationReturn = soap_tmp_ns1__deletableNotificationResponse->_deletableNotificationReturn;
	return soap_closesock(soap);
}

} // namespace CMESOAP


/* End of CMESOAPClient.cpp */
