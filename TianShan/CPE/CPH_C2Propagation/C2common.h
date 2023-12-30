#pragma once
const std::string XML_HEADER = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n";

#define HTTP_HEAD_HOST              "Host"
#define HTTP_HEAD_CONTENTTYPE       "Content-Type"
#define HTTP_HEAD_CONTENTLENGTH     "Content-Length"
#define HTTP_HEAD_USERAGENT         "User-Agent"
#define HTTP_HEAD_INGRESSCAPACITY   "Ingress-Capacity"


#define HTTP_RESPONSE_200	"CDN received and accepted the request"
#define HTTP_RESPONSE_201	"CDN has created a reservation for the requested transfer."
#define HTTP_RESPONSE_206	"Partial Content"
#define HTTP_RESPONSE_307	"Temporary Redirect"
#define HTTP_RESPONSE_400	"Bad Request"
#define HTTP_RESPONSE_404	"Content does not exists, or cannot be retrieved"
#define HTTP_RESPONSE_410	"The reservation has expired"
#define HTTP_RESPONSE_416	"At least one byte in the requested range could not be returned."
#define HTTP_RESPONSE_500	"Internal server error"
#define HTTP_RESPONSE_501	"Not implemented"
#define HTTP_RESPONSE_503	"No transfer port is available for a new locate |Resources prevent servicing a new transfer request."
#define HTTP_RESPONSE_505	"Version not supported"

