// File Name : RequestHandler.h

#ifndef __EVENT_IS_VODI5_REQUEST_HANDLER_H__
#define __EVENT_IS_VODI5_REQUEST_HANDLER_H__

#include <Ice/Object.h>
#include <IceUtil/IceUtil.h>

// ZQ Common
#include "Log.h"

// Stream Smith
#include "StreamSmithModule.h"

// project file
#include "SsmGBss.h"

// share by children class
#define CRLF "\r\n"
#define MY_BUFFER_SIZE 2048

#define HANDLERLOG _fileLog
#define HANDLERLOGFMT(_X, _T) CLOGFMT(_X, "Sess(%s)Seq(%s)Req(%p)Mtd(%s) " _T), _session.c_str(), _sequence.c_str(), _request, _method.c_str()

namespace GBss
{

class Environment;

enum ReturnType
{
	RETURN_SYNC,
	RETURN_ASYNC
};

class RequestHandler : public Ice::Object
{
public:
	typedef IceUtil::Handle<RequestHandler> Ptr;

	RequestHandler(ZQ::common::Log& fileLog, Environment& env, 
		IStreamSmithSite* pSite, IClientRequestWriter* pReq);

	virtual ~RequestHandler();

	/// define process algorithm schema 
	RequestProcessResult process();

protected:
	/// if _site, _request, _response valid
	bool preprocess();

	/// this method should be derived by children class to handle request
	virtual RequestProcessResult doContentHandler();

	/// compose and pose response
	void composeResponse();

protected:
	
	/// get the value of request header, if no specified header return an empty string
	std::string getRequestHeader(const char* pHeader, int logLevel = 7);

	/// get request content body
	void getContentBody(int logLevel = 7);

protected:
	/// get session proxy
	bool findSession();

	/// add or update session meteData
	bool updateSessionMetaData(const std::string& key, const std::string& val);

	/// get session context including session meteData
	bool getSessionContext();

protected:

	/// set session expiration to ttl and watch session with ttl timeout
	bool renewSession(Ice::Long ttl = 0);

	/// get stream proxy
	bool findPlaylist();

	/// get stream current position and scale
	bool getPositionAndScale(std::string& range, std::string& scale);

	/// get stream state
	bool getPlaylistState(TianShanIce::Streamer::StreamState& streamState);

	/// get stream state string
	std::string getStateString(TianShanIce::Streamer::StreamState& streamState);

	/// add penalty in streamer
	void addPenalty();

	bool getStopNPT(int& playPosition, int& npt);

	bool getStreamSourceAddressInfo( std::string& sourceIP, int& sourcePort );

public:
	friend class PauseHandlerAsync;
	friend class PlayResponseAsync;

protected:
	ZQ::common::Log& _fileLog;
	Environment& _env;
	IStreamSmithSite* _site;
	IClientRequestWriter* _request;
	IServerResponse* _response;

protected:
	std::string _session;
	std::string	_method;
	std::string	_sequence;
	std::string _requestBody;

protected:
	char _szBuf[MY_BUFFER_SIZE];
	std::string _strLastError;

protected:
	TianShanIce::Properties _sessionContext;
	SsmGBss::CRGSessionPrx _sessionProxy;

protected:
	std::string _streamId;
	std::string _streamerNetId;
	TianShanIce::Streamer::PlaylistPrx _playlistPrx;

protected:
	int _statusCode;
	Ice::Long _startTime;
	ReturnType _returnType;

};

} // end GBss

#endif // end __EVENT_IS_VODI5_REQUEST_HANDLER_H__
