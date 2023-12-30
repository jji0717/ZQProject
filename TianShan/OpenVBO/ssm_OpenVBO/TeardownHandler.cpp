// File Name : TeardownHandler.cpp

#include "TeardownHandler.h"
#include "Environment.h"
#include "CRGSessionManager.h"
#include "RtspHeaderDefines.h"

namespace EventISVODI5
{

TeardownHandler::TeardownHandler(ZQ::common::Log& fileLog, Environment& env, 
								 IStreamSmithSite* pSite, IClientRequestWriter* pReq)
: RequestHandler(fileLog, env, pSite, pReq, "TEARDOWN")
{
}

TeardownHandler::~TeardownHandler()
{

}

RequestProcessResult TeardownHandler::doContentHandler()
{
	HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(TeardownHandler, "start processing"));

	if (!getSessionContext())
	{
		return RequestError;
	}
	_streamerNetId	= _sessionContext["StreamerNetID"];
	_streamId		= _sessionContext["StreamID"];
	// session exist ?
	if (!findPlaylist())
	{
		return RequestError;
	}
	try
	{
		TianShanIce::ValueMap valMap;
		_playlistPrx->getInfo( ::TianShanIce::Streamer::infoSTREAMNPTPOS, valMap);	

		int	npt;	
		int	playTime;

		ZQTianShan::Util::getValueMapDataWithDefault( valMap , "playposition" , 0 , npt );
		ZQTianShan::Util::getValueMapDataWithDefault( valMap , "totalplaytime" , 0, playTime );
		
		char buf[64]="", *p=buf;
		snprintf(p, buf + sizeof(buf)-2 -p, "npt="); p+=strlen(p);
		char* finalNpt = p;
		snprintf(p, buf + sizeof(buf)-2 -p, "%d.%03d", (int32) (npt/1000), (int32) (npt%1000)); p+=strlen(p);
		//_response->setHeader(HeaderFinalNPT, finalNpt); 

		std::string current_npt = finalNpt;
		std::string content = "current_npt=" + current_npt;
		_response->printf_postheader(content.c_str());

		*p++= '-'; *p= 0x00;
		if (playTime > 0)
		{
			snprintf(p, buf + sizeof(buf)-2 -p, "%d.%03d", (int32) (playTime/1000), (int32) (playTime%1000)); p+=strlen(p);
		}

		_response->setHeader(HeaderRange, buf);	
		// destroy stream instance
		_statusCode = _sessionProxy->destroy() ? 200 : 503;
		
	}
	catch( const Ice::ObjectNotExistException& )
	{
		_statusCode = 454;
		snprintf(_szBuf, sizeof(_szBuf) - 1, "caught ObjectNotExistException when destoried stream instance");
		HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(TeardownHandler, "%s"), _szBuf);
	}
	catch( const Ice::TimeoutException& ex)
	{
		_statusCode = 503;
		snprintf(_szBuf, sizeof(_szBuf) - 1, "caught [%s] when destoried stream instance", ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(TeardownHandler, "%s"), _szBuf);
		return RequestError; 
	}
	catch( const Ice::SocketException& ex)
	{
		_statusCode = 503;
		snprintf(_szBuf, sizeof(_szBuf) - 1, "caught [%s] when destoried stream instance", ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(TeardownHandler, "%s"), _szBuf);
		return RequestError; 
	}
	catch (const Ice::Exception& ex)
	{
		_statusCode = 500;
		snprintf(_szBuf, sizeof(_szBuf) - 1, "caught [%s] when destoried stream instance", ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(TeardownHandler, "%s"), _szBuf);
		return RequestError; 
	}

	return RequestProcessed;
}

} // end EventISVODI5
