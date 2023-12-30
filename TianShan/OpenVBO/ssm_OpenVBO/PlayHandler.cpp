// File Name : PlayHandler.cpp

#include "PlayHandler.h"
#include "SelectionResourceManager.h"
#include "Environment.h"
#include "RtspRelevant.h"
#include "TianShanDefines.h"
#include "TianShanIceHelper.h"

#include "stroprt.h"

#define HANDLERLOGEX _pHandler->_fileLog
#define HANDLERLOGFMTEX(_X, _T) CLOGFMT(_X, "Sess(%s)Seq(%s)Req(%p)Mtd(%s) " _T),_pHandler->_session.c_str(), _pHandler->_sequence.c_str(), _pHandler->_request, _pHandler->_method.c_str()

namespace EventISVODI5
{

class PlayResponseAsync 
{
public:
	PlayResponseAsync(PlayHandler::Ptr pHandler)
	{
		pHandler->_request->setContext(CLIENT_REQUEST_DISABLE_AUTO_DELETE, "1");
		_pHandler = pHandler;
		assert(_pHandler);
		_pHandler->_request->addRef();
	}

	~PlayResponseAsync()
	{
		_pHandler->_request->release();
	}

public:
	virtual void AsyncResponse(const ::TianShanIce::Streamer::StreamInfo& infoRet)
	{
		char localBuffer[1024];
		HANDLERLOGEX(ZQ::common::Log::L_INFO, HANDLERLOGFMTEX(PlayResponseAsync, "performed playEx/playItem on stream[%s] successfully: [%s]"), (_pHandler->_streamId).c_str(), ZQTianShan::Util::dumpStreamInfo(infoRet, localBuffer, sizeof(localBuffer)-1));

		//get result information
		std::string	strTimeOffset;
		std::string strCurrentSpeed;
		::TianShanIce::Properties::const_iterator itRet = infoRet.props.find("CURRENTPOS");
		if(itRet != infoRet.props.end())
		{
			int iOffset = atoi(itRet->second.c_str());
			char szTemp[256];
			snprintf(szTemp, sizeof(szTemp)-1, "%d.%d", iOffset/1000, iOffset%1000);
			strTimeOffset = "npt=" ;
			strTimeOffset = strTimeOffset + szTemp ;
			strTimeOffset = strTimeOffset + "-";
		}
		else
		{
			strTimeOffset = "npt=0-";
		}

		itRet = infoRet.props.find("TOTALPOS");
		if(itRet != infoRet.props.end())
		{
			if(itRet->second.length() > 0)
			{
				int iOffset = atoi(itRet->second.c_str());
				if(iOffset > 0)
				{
					char szTemp[256];
					snprintf(szTemp, sizeof(szTemp)-1, "%d.%d", iOffset/1000, iOffset%1000);
					strTimeOffset = strTimeOffset + szTemp;
				}
			}
		}

		itRet = infoRet.props.find("SPEED");
		if (itRet != infoRet.props.end())
		{
			strCurrentSpeed = itRet->second;
		}
		else
		{
			strCurrentSpeed = "0.0";
		}

		IServerResponse* pResponse = _pHandler->_response;
		pResponse->setHeader(HeaderScale, strCurrentSpeed.c_str());
		pResponse->setHeader(HeaderRange, strTimeOffset.c_str());
		_pHandler->_statusCode = 200;
		_pHandler->composeResponse();

		Ice::Long cur = ZQTianShan::now();
		Ice::Long timeUsed = cur - _pHandler->_startTime;
		HANDLERLOGEX(ZQ::common::Log::L_INFO, CLOGFMT(PlayResponseAsync, "Sess(%s)Seq(%s)[success]process request, used [%lld]ms"), _pHandler->_session.c_str(), _pHandler->_sequence.c_str(), timeUsed);
	}

	virtual void ASyncException(const ::Ice::Exception& ex)
	{
		try
		{
			ex.ice_throw();
		}
		catch (const Ice::TimeoutException& ex)
		{
			// add penalty
			_pHandler->addPenalty();
			_pHandler->_statusCode = 503;
			snprintf(_pHandler->_szBuf, sizeof(_pHandler->_szBuf) - 1, "caught [%s] during playEx/playItem on stream[%s]", ex.ice_name().c_str(), (_pHandler->_streamId).c_str());
			HANDLERLOGEX(ZQ::common::Log::L_WARNING, HANDLERLOGFMTEX(PlayResponseAsync, "%s"), _pHandler->_szBuf);
		}
		catch (const Ice::SocketException& ex)
		{
			// add penalty
			_pHandler->addPenalty();

			_pHandler->_statusCode = 503;
			snprintf(_pHandler->_szBuf, sizeof(_pHandler->_szBuf) - 1, "caught [%s] during playEx/playItem on stream[%s]",ex.ice_name().c_str() ,(_pHandler->_streamId).c_str());
			HANDLERLOGEX(ZQ::common::Log::L_WARNING, HANDLERLOGFMTEX(PlayResponseAsync, "%s"), _pHandler->_szBuf);
		}
		catch (const Ice::ObjectNotExistException& ex)
		{
			_pHandler->_statusCode = 454;
			snprintf(_pHandler->_szBuf, sizeof(_pHandler->_szBuf) - 1, "caught [%s] during playEx/playItem on stream[%s]", ex.ice_name().c_str() ,(_pHandler->_streamId).c_str());
			HANDLERLOGEX(ZQ::common::Log::L_WARNING, HANDLERLOGFMTEX(PlayResponseAsync, "%s"), _pHandler->_szBuf);
		}
		catch (const TianShanIce::InvalidStateOfArt& bEx)
		{
			_pHandler->_statusCode = 405;
			snprintf(_pHandler->_szBuf, sizeof(_pHandler->_szBuf) - 1, "caught exception[%s]:%d %s during playEx/playItem on stream[%s]", bEx.ice_name().c_str(), bEx.errorCode, bEx.message.c_str(), (_pHandler->_streamId).c_str());
			HANDLERLOGEX(ZQ::common::Log::L_WARNING, HANDLERLOGFMTEX(PlayResponseAsync, "%s"), _pHandler->_szBuf);
		}
		catch (const TianShanIce::InvalidParameter& bEx)
		{
			switch (bEx.errorCode)
			{
			case EXT_ERRCODE_INVALID_RANGE:
				{
					_pHandler->_statusCode = 457;
				}
				break;
			case EXT_ERRCODE_BANDWIDTH_EXCEEDED:
				{
					_pHandler->_statusCode = 453;
				}
				break;
			default:
				{
					_pHandler->_statusCode = 451;
				}
				break;
			}
			snprintf(_pHandler->_szBuf, sizeof(_pHandler->_szBuf) - 1, "caught exception[%s]:%d %s during playEx/playItem on stream[%s]", bEx.ice_name().c_str(), bEx.errorCode, bEx.message.c_str(), (_pHandler->_streamId).c_str());
			HANDLERLOGEX(ZQ::common::Log::L_WARNING, HANDLERLOGFMTEX(PlayResponseAsync, "%s"), _pHandler->_szBuf);
		}
		catch (const TianShanIce::ServerError& bEx)
		{
			switch (bEx.errorCode)
			{
			case EXT_ERRCODE_INVALID_RANGE:
				{
					_pHandler->_statusCode = 457;
				}
				break;
			case EXT_ERRCODE_BANDWIDTH_EXCEEDED:
				{
					_pHandler->_statusCode = 453;
				}
				break;
			default:
				{
					_pHandler->_statusCode = 500;
				}
				break;
			}
			snprintf(_pHandler->_szBuf, sizeof(_pHandler->_szBuf) - 1, "caught exception [%s]:%d %s during playEx/playItem on stream[%s]", bEx.ice_name().c_str(), bEx.errorCode, bEx.message.c_str(), (_pHandler->_streamId).c_str());
			HANDLERLOGEX(ZQ::common::Log::L_WARNING, HANDLERLOGFMTEX(PlayResponseAsync, "%s"), _pHandler->_szBuf);
		}
		catch (const Ice::Exception& ex)
		{
			_pHandler->_statusCode = 500;
			snprintf(_pHandler->_szBuf, sizeof(_pHandler->_szBuf) - 1, "caught [%s] during playEx/playItem on stream[%s]", ex.ice_name().c_str() ,(_pHandler->_streamId).c_str());
			HANDLERLOGEX(ZQ::common::Log::L_WARNING, HANDLERLOGFMTEX(PlayResponseAsync, "%s"), _pHandler->_szBuf);
		}

		Ice::Long cur = ZQTianShan::now();
		Ice::Long timeUsed = cur - _pHandler->_startTime;
		_pHandler->composeResponse();
		HANDLERLOGEX(ZQ::common::Log::L_INFO, CLOGFMT(PlayResponseAsync, "Sess(%s)Seq(%s)[failed]process request, used [%lld]ms"), _pHandler->_session.c_str(), _pHandler->_sequence.c_str(), timeUsed);
	}
private:
	PlayHandler::Ptr _pHandler;
};

#if  ICE_INT_VERSION / 100 >= 306
    class playAsyncCB : public IceUtil::Shared
    {
    public:
        playAsyncCB(PlayHandler::Ptr pHandler)
        {   
        }   
    private:
        void handleException(const Ice::Exception& ex){}
    public:
        void playEx(const Ice::AsyncResultPtr& r)
        {   
            TianShanIce::Streamer::PlaylistPrx playlistPrx = TianShanIce::Streamer::PlaylistPrx::uncheckedCast(r->getProxy());
            try 
            {   
                playlistPrx->end_playEx(r);
            }   
            catch(const Ice::Exception& ex) 
            {   
                handleException(ex);
            }   
        }   

        void playItem(const Ice::AsyncResultPtr& r)
        {   
            TianShanIce::Streamer::PlaylistPrx playlistPrx = TianShanIce::Streamer::PlaylistPrx::uncheckedCast(r->getProxy());
            try 
            {   
                playlistPrx->end_playItem(r);
            }   
            catch(const Ice::Exception& ex) 
            {   
                handleException(ex);
            }   
        }   
    private:
        PlayHandler::Ptr pHandler;
    };  
    typedef IceUtil::Handle<playAsyncCB> playAsyncCBPtr;
#else
class PlayExAsync : public PlayResponseAsync, public TianShanIce::Streamer::AMI_Stream_playEx
{
public:
	PlayExAsync(PlayHandler::Ptr pHandler) : PlayResponseAsync(pHandler)
	{
	}
public:
	virtual void ice_response(const ::TianShanIce::Streamer::StreamInfo& infoRet)
	{
		AsyncResponse(infoRet);
	}
	virtual void ice_exception(const ::Ice::Exception& ex)
	{
		ASyncException(ex);
	}	

};
#endif
//class playItemAsync : public PlayResponseAsync, public TianShanIce::Streamer::AMI_Playlist_playItem
//{
//public:
//	playItemAsync(PlayHandler::Ptr pHandler) : PlayResponseAsync(pHandler)
//	{
//	}
//public:
//	virtual void ice_response(const ::TianShanIce::Streamer::StreamInfo& infoRet)
//	{
//		AsyncResponse(infoRet);
//	}
//	virtual void ice_exception(const ::Ice::Exception& ex)
//	{
//		ASyncException(ex);
//	}
//};

PlayHandler::PlayHandler(ZQ::common::Log &fileLog, Environment &env, IStreamSmithSite *pSite, IClientRequestWriter *pReq)
:RequestHandler(fileLog, env, pSite, pReq, "PLAY")
{
}

PlayHandler::~PlayHandler()
{

}

RequestProcessResult PlayHandler::doContentHandler()
{
	HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(PlayHandler, "start processing"));

	if (!getSessionContext())
	{
		return RequestError;
	}

	if (!renewSession())
	{
		return RequestError;
	}

	if (!findPlaylist())
	{
		return RequestError;
	}

	// update STB Connection ID
	::std::string strSTBConnectionID = getRequestHeader("SYS#ConnID");
	if (!updateSessionMetaData(SESSION_META_DATA_STB_CONNECTION_ID, strSTBConnectionID))
	{
		return RequestError;
	}

	Ice::Short from = 0;
	Ice::Long milliSecs = 0;
	Ice::Float f_speed = 0.0f;
	//Ice::Int assetIndex = -1;

	// get range
	_reqRange = getRequestHeader(HeaderRange);
	if (_reqRange != "")
	{		
		if (stricmp(ZQ::StringOperation::nLeftStr(_reqRange, 4).c_str(), "npt=") != 0)
		{
			_statusCode = 400; //
			snprintf(_szBuf, sizeof(_szBuf) - 1, "Illegal Range: [%s] specified, missed [npt=]", _reqRange.c_str());
			HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PlayHandler, "%s"), _szBuf);
			//HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PlayHandler, "%s"), szBuf);
			return RequestError;
		}
		std::string right_str = ZQ::StringOperation::getRightStr(_reqRange, "=", true);
		int pos_tmp;
		std::string second_str;
		if (true == ZQ::StringOperation::hasChar(right_str, '-', pos_tmp))
		{
			second_str = ZQ::StringOperation::midStr(right_str, -1, pos_tmp);
		}
		else 
		{
			second_str = right_str;
		}

		if (!second_str.empty() && 0 != stricmp(second_str.c_str(), "now"))
		{
			from = 1; // from begining if specify npt=...
			milliSecs = (Ice::Long) (atof(second_str.c_str()) * 1000.0);			
		}
	}
	//else
	//{
	//	// first play
	//	TianShanIce::Streamer::StreamState state;
	//	if (!getPlaylistState(state))
	//	{
	//		ZQTianShan::Util::getPropertyDataWithDefault(_sessionContext, REQUEST_STARTPOINT_OFFSET, 0, milliSecs);
	//	}
	//	else
	//	{
	//		if (state != TianShanIce::Streamer::stsStreaming && state != TianShanIce::Streamer::stsPause)
	//		{
	//			ZQTianShan::Util::getPropertyDataWithDefault(_sessionContext, REQUEST_STARTPOINT_OFFSET, 0, milliSecs);
	//		}
	//	}
	//}

	// get scale
	_reqScale = getRequestHeader(HeaderScale);
	if (_reqScale != "")
	{	
		f_speed = (float) atof( _reqScale.c_str() );
	}

	TianShanIce::StrValues expectValues;
	expectValues.push_back("ITEM_CURRENTPOS");
	expectValues.push_back("CURRENTPOS");
#pragma message(__MSGLOC__"TODO:Should I remove TOTALPOS from expect values ?")
	expectValues.push_back("TOTALPOS");
	expectValues.push_back("SPEED");
	expectValues.push_back("STATE");
	expectValues.push_back("USERCTRLNUM");
	try
	{
		//if request include range , just play with range
		HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(PlayHandler, "calling playEx() with scale[%f] npt[%lld] from[%d] on stream[%s]"), f_speed, milliSecs, from, _streamId.c_str());
        #if ICE_INT_VERSION / 100 >= 306
            playAsyncCBPtr onPlayCbPtr = new playAsyncCB(this);
            Ice::CallbackPtr genericCB = Ice::newCallback(onPlayCbPtr, &playAsyncCB::playEx);
            _playlistPrx->begin_playEx(f_speed, milliSecs, from, expectValues,genericCB);
        #else
		_playlistPrx->playEx_async( (new PlayExAsync(this)), f_speed, milliSecs, from, expectValues);
        #endif
		if(_sessionProxy->getSessStatusFlags() != SESS1STPLAY)
			_sessionProxy->setSessStatusFlags(SESS1STPLAY);
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		_statusCode = 500;
		snprintf(_szBuf, sizeof(_szBuf) - 1, "playEx() on stream[%s] caught exception[%s] %s", _streamId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PlayHandler, "%s"), _szBuf);
		//HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PlayHandler, "%s"), szBuf);
		return RequestError;
	}
	catch(const ::Ice::Exception& ex)
	{
		_statusCode = 500;
		snprintf(_szBuf, sizeof(_szBuf) - 1, "playEx() on stream[%s] caught exception[%s]", _streamId.c_str(), ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PlayHandler, "%s"), _szBuf);
		//HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PlayHandler, "%s"), szBuf);
		return RequestError;
	}
	_returnType = RETURN_ASYNC;
	return RequestProcessed;
}

} // end EventISVODI5
