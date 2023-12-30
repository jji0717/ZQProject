// FileName : A3Client.cpp
// Author   : Zheng Junming
// Date     : 2009-05
// Desc     : 

#include "A3Client.h"

using namespace CRG::Plugin::A3Server;

A3Client::A3Client()
:_sequence(0)
{
}

A3Client::~A3Client()
{
}


int A3Client::SendRequest(const std::string &url, std::string &buffer)
{
	ZQ::common::HttpClient _httpClient;
	_httpClient.init();
	_httpClient.setRecvTimeout(20);
	_httpClient.setLog(&glog);

	// set http head
	glog(ZQ::common::Log::L_INFO, CLOGFMT(A3Client, "SendRequest() : request [%s] to [%s]"), buffer.c_str(), url.c_str());
	char length[64];
	snprintf(length, sizeof(length), "%ld", buffer.length());
	_httpClient.setHeader("Content-Type", "text/xml");
	_httpClient.setHeader("Content-Length", length);
	long sequence = 0;
	{
		ZQ::common::MutexGuard gd(_lock);
		sequence = ++ _sequence;
	}
	snprintf(length, sizeof(length), "%ld", sequence);
	_httpClient.setHeader("CSeq", length);

	if(_httpClient.httpConnect(url.c_str())) 
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT( A3Client, "SendRequest() : connect failed,endpoint: %s"),url.c_str());
		_httpClient.uninit();
		return -1;
	}

	if(_httpClient.httpSendContent(buffer.data(), buffer.length()) || _httpClient.httpEndSend()) 
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT( A3Client,"_strHost() send errorcode: %d"), _httpClient.getErrorcode());
		_httpClient.uninit();
		return -1;
	}

	if(_httpClient.httpBeginRecv())
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT( A3Client, "_strHost() receive errorstring:%s"),_httpClient.getErrorstr());
		_httpClient.uninit();
		return -1;
	}

	while(!_httpClient.isEOF()) 
	{

		if(_httpClient.httpContinueRecv()) 
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT( A3Client, "_strHost() continue receive errorstring:%s"),_httpClient.getErrorstr());
			_httpClient.uninit();	
			return -1;
		}
	}

	_httpClient.getContent(buffer);

	_httpClient.uninit(); // may be have a problem here

	int statusCode = _httpClient.getStatusCode();
	glog(ZQ::common::Log::L_INFO, CLOGFMT(A3Client, "SendRequest() : status code[%d]"), statusCode);
	return statusCode;
}
