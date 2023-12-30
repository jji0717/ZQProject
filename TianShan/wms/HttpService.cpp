// HttpService.cpp: implementation of the HttpService class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HttpService.h"
#include "UrlParser.h"
#include "SSLLib/ssllib.h"

using namespace TianShanIce;
using namespace ZQ;

Ice::ObjectPrx HttpService::IceEnv::getObject(const std::string& name)
{
	char endPoint[256];
	sprintf(endPoint, "%s:default -p %d", name.c_str(), ICE_PORT);
	return _ic->stringToProxy(endPoint);
}

//////////////////////////////////////////////////////////////////////////
// HttpRequest

class HttpRequest: public common::ThreadRequest {
public:
	HttpRequest(common::NativeThreadPool& threadPool, 
		HttpService::IceEnv& iceEnv, SSLSocket* sock) :
	  common::ThreadRequest(threadPool), _iceEnv(iceEnv)
	{
		_sock = sock;		
	}

	virtual bool init()
	{
		return true;
	}

	bool getUrl(char buf[], std::string& url)
	{
		const char* c = buf;
		while(*c && *c != ' ' && *c != '\t') {
			c ++;
		}
		if (*c == 0 || *(++ c) == 0)
			return false;

		const char* s = c;
		while(*c && *c != ' ' && *c != '\r' && *c != '\n') {
			c ++;
		}
		if (*c == 0)
			return false;

		url = std::string(s, (u_long )c - (u_long )s);
		return true;
	}

	virtual int run()
	{
		char req[1024];
		_sock->recv(req, sizeof(req));
		std::string url;
		if (!getUrl(req, url)) 
			return -1;

		_urlParser.parse(url.c_str());
		Ice::ObjectPrx prx = _iceEnv.getObject("WMSStreamerService");
		WMSStreamerServicePrx service = WMSStreamerServicePrx::checkedCast(prx);
		std::string playlistId = _urlParser.getQueryItem("guid");
		std::string queryItem = _urlParser.getQueryItem("item_count");
		int itemCount = atoi(queryItem.c_str());

		PlaylistItems items;
		char itemName[64];
		for (int i = 0; i < itemCount; i ++) {
			sprintf(itemName, "item%d", i);
			queryItem = _urlParser.getQueryItem(itemName);
			items.push_back(queryItem);
		}

		std::string reloc;
		sockaddr_in addr;
		int addrLen = sizeof(addr);
		SOCKET s = _sock->getpeername((sockaddr* )&addr, &addrLen);
		std::string ipaddr = inet_ntoa(addr.sin_addr);
		Streamer::PlaylistPrx playlist = service->createPublishingPoint(
			playlistId, items, OnDemandPublishingPoint, ipaddr, reloc);

		char* httpResp = "HTTP/1.1 301 Moved Permanently\r\n"
		"Location: rtsp://%s/%s\r\n"
		"Connection: close\r\n"
		"Content-Type: text/html;\r\n\r\n";
		char buf[512];
		sprintf(buf, httpResp, "mediasvc", reloc.c_str());
		_sock->send(buf, strlen(buf));

		// Streamer::PlaylistPrx playlist = service->createStream();
		
		
		return 0;
	}

	virtual void final()
	{
		if (_sock)
			delete _sock;
		delete this;		
	}

protected:
	UrlParser				_urlParser;
	HttpService::IceEnv&	_iceEnv;
	SSLSocket*				_sock;
};

//////////////////////////////////////////////////////////////////////
// HttpService
HttpService::HttpService(ZQ::common::NativeThreadPool& threadPool, 
						 const sockaddr& bindAddr) :
	_threadPool(threadPool)
{
	_bindAddr = bindAddr;
}

HttpService::~HttpService()
{

}

bool HttpService::init()
{
	_sock.socket();
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(8080);
	_sock.bind((sockaddr* )&addr, sizeof(addr));
	_sock.listen();
	return _iceEnv.init();
}

int HttpService::run()
{
	while (true) {
		SSLSocket* client = _sock.accept(NULL, NULL, false);
		if (client) {
			HttpRequest* req = new HttpRequest(_threadPool, 
				_iceEnv, client);
			req->start();
		}
	}
	return 0;
}

void HttpService::final()
{
	
}
