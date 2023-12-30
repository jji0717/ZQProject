#include "PlayRequest.h"
#include <TianShanIceHelper.h>

extern ZQ::common::Log* s1log;

namespace TianShanS1
{

#define HandlerFmtEx(_C, _X) CLOGFMT(_C, "Req(%s)Sess(%s)Seq(%s)Mtd(%s:%s) " _X), _pHandler->_reqIdent.c_str(), _pHandler->_session.c_str(), _pHandler->_sequence.c_str(), _pHandler->_phase.c_str(), _pHandler->_method.c_str()

	class playAsyncResponse
	{
	public:
		playAsyncResponse( HandlePlay::Ptr pHandler )
		{	
			_pHandler = pHandler;
			assert( _pHandler );
			_pHandler->_pRequestWriter->addRef();
			_pHandler->setReturnType( RequestHandler::RETURN_ASYNC );
			_pHandler->_pRequestWriter->setContext(CLIENT_REQUEST_DISABLE_AUTO_DELETE,"1");

		}
		virtual ~playAsyncResponse( )
		{
			if ( true == _pHandler->needModifyResponse())
			{
				IStreamSmithSite* pSite = _pHandler->_pRequest->getSite();
				assert(pSite);
				pSite->postRequest(_pHandler->_pRequestWriter,IClientRequest::FixupResponse);
			}
			else
			{
				_pHandler->postResponse(); // need not modify response, post directly
				// 				delete _pHandler->_pRequest;
			}
			_pHandler->_pRequestWriter->release( );

			/*delete _pHandler;*/
		}

	public:
		virtual void AsyncResponse(const ::TianShanIce::Streamer::StreamInfo& infoRet)
		{
			SSMLOG(ZQ::common::Log::L_INFO,
				HandlerFmtEx(HandlePlay, "perform playEx() on stream: [%s] successfully:%s"), 
				_pHandler->_cltSessCtx.streamPrxID.c_str() , _pHandler->dumpTSStreamInfo(infoRet).c_str() );

			if( _pHandler->prepareResponseParameter(infoRet))
			{
				_pHandler->_statusCode = 200;
				_pHandler->composeResponse(_pHandler->_statusCode);
			}		

			Ice::Long cur = ZQTianShan::now();
			Ice::Long timeUsed = cur - _pHandler->getStartTime();
			SSMLOG(ZQ::common::Log::L_INFO,
				CLOGFMT(NGODEnv, "Sess(%s)Seq(%s)[success]process[%s]request, used [%lld]ms"), 
				_pHandler->getSession().c_str(),
				_pHandler->getSequence().c_str(),
				_pHandler->getMethod().c_str(), 
				timeUsed);

		}
		virtual void ASyncException(const ::Ice::Exception& ex)
		{	
			try
			{
				ex.ice_throw();
			}
			catch( const Ice::TimeoutException& ex)
			{
				SSMLOG(ZQ::common::Log::L_ERROR,
					HandlerFmtEx(HandlePlay, "failed perform playEx() on stream[%s] with Ice Exception:%s "), 
					ex.ice_name().c_str() , _pHandler->_cltSessCtx.streamPrxID.c_str() );
				_pHandler->_statusCode = 503;
				_pHandler->composeResponse(_pHandler->_statusCode, ex.ice_name().c_str());
			}
			catch( const Ice::SocketException& ex)
			{
				SSMLOG(ZQ::common::Log::L_ERROR,
					HandlerFmtEx(HandlePlay, "failed perform playEx() on stream[%s] with Ice Exception:%s "), 
					ex.ice_name().c_str() , _pHandler->_cltSessCtx.streamPrxID.c_str() );
				_pHandler->_statusCode = 503;
				_pHandler->composeResponse(_pHandler->_statusCode, ex.ice_name().c_str());
			}
			catch( const TianShanIce::InvalidStateOfArt& ex)
			{
				SSMLOG(ZQ::common::Log::L_ERROR, HandlerFmtEx(HandlePlay, "failed perform playEx() with TianShan Exception:%s"), 
					ex.message.c_str() );					
				_pHandler->_statusCode = 455;
				_pHandler->composeResponse(_pHandler->_statusCode, ex.message.c_str());
			}
			catch( const TianShanIce::ServerError& ex)
			{
				_pHandler->processServerErrorWithRtspErrCode(ex);
				_pHandler->composeResponse(_pHandler->_statusCode, ex.message.c_str());
			}
			catch( const TianShanIce::BaseException& ex)
			{
				const TianShanIce::BaseException& bEx = dynamic_cast<const TianShanIce::BaseException&>(ex);			
				SSMLOG(ZQ::common::Log::L_ERROR, HandlerFmtEx(HandlePlay, "failed perform playEx() with TianShan Exception:%s"), 
					ex.message.c_str() );	
				_pHandler->_statusCode = 500;		
				_pHandler->composeResponse(_pHandler->_statusCode, ex.message.c_str());
			}
			catch( const Ice::ObjectNotExistException& ex)
			{				
				SSMLOG(ZQ::common::Log::L_ERROR, HandlerFmtEx(HandlePlay, "failed perform playEx() with Ice Exception:%s"), 
					ex.ice_name().c_str() );					
				_pHandler->_statusCode = 404;
				_pHandler->composeResponse(_pHandler->_statusCode, ex.ice_name().c_str());
				_pHandler->clearSessionResource();
			}
			catch( const Ice::Exception& ex)
			{
				SSMLOG(ZQ::common::Log::L_ERROR,
					HandlerFmtEx(HandlePlay, "failed perform playEx() on stream[%s] with Ice Exception:%s "), 
					ex.ice_name().c_str() , _pHandler->_cltSessCtx.streamPrxID.c_str() );
				_pHandler->_statusCode = 500;
				_pHandler->composeResponse(_pHandler->_statusCode, ex.ice_name().c_str());
			}	
			
			_pHandler->rollbackSpeed();//exception occurred, roll back last updated speed

			Ice::Long cur = ZQTianShan::now();
			Ice::Long timeUsed = cur - _pHandler->getStartTime();
			SSMLOG(ZQ::common::Log::L_INFO,
				CLOGFMT(NGODEnv, "Sess(%s)Seq(%s)[failed]process[%s]request, used [%lld]ms"), 
				_pHandler->getSession().c_str(),
				_pHandler->getSequence().c_str(),
				_pHandler->getMethod().c_str(), 
				timeUsed);
		}

	private:
		HandlePlay::Ptr		_pHandler;

	};
#if  ICE_INT_VERSION / 100 >= 306
	class playAsyncCB : public playAsyncResponse, public IceUtil::Shared
	{
	public:
		playAsyncCB(HandlePlay::Ptr pHandler)
			:playAsyncResponse(pHandler)
		{
		}
	private:
		void handleException(const Ice::Exception& ex){}
	public:
		void playEx(const Ice::AsyncResultPtr& r)
		{
			TianShanIce::Streamer::PlaylistPrx playlsPrx = TianShanIce::Streamer::PlaylistPrx::uncheckedCast(r->getProxy());
			try
			{
				playlsPrx->end_playEx(r);
			}
			catch(const Ice::Exception& ex)
			{
				handleException(ex);
			}
		}

		void playItem(const Ice::AsyncResultPtr& r)
		{
			TianShanIce::Streamer::PlaylistPrx playlsPrx = TianShanIce::Streamer::PlaylistPrx::uncheckedCast(r->getProxy());
			try
			{
				playlsPrx->end_playItem(r);
			}
			catch(const Ice::Exception& ex)
			{
				handleException(ex);
			}
		}
	private:
		HandlePlay::Ptr pHandler;
	};
	typedef IceUtil::Handle<playAsyncCB> playAsyncCBPtr;
#else

	class playExAsync : public playAsyncResponse, public TianShanIce::Streamer::AMI_Stream_playEx
	{
	public:
		playExAsync( HandlePlay::Ptr pHandler )
			:playAsyncResponse(pHandler)
		{
		}
		virtual ~playExAsync( )
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

	class playItemAsync : public playAsyncResponse , public TianShanIce::Streamer::AMI_Playlist_playItem
	{
	public:
		playItemAsync( HandlePlay::Ptr pHandler )
			:playAsyncResponse(pHandler)
		{

		}
		virtual ~playItemAsync()
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
	FixupPlay::FixupPlay(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse)
		: FixupRequest(env, pSite, pReq, pResponse)
	{
	}

	FixupPlay::~FixupPlay()
	{
	}

	bool FixupPlay::process()
	{
		std::string require = getRequestHeader(HeaderRequire);
		if(require.find("com.comcast.ngod.c1") != std::string::npos && getRequestHeader(HeaderSession).empty())// s1 spec
		{
			std::string urlStr;
			memset(_szBuf, 0, sizeof(_szBuf));
			urlStr = _pRequest->getUri(_szBuf, sizeof(_szBuf) - 1);
			if (!urlStr.empty() && urlStr != "*" && urlStr.find_last_of('/') != std::string::npos)	// sample : rtsp://10.50.19.61:554/716195834
			{
				std::string sessionId = urlStr.substr(urlStr.find_last_of('/')+1, urlStr.size());
				_pRequestWriter->setHeader(HeaderSession, (char *)sessionId.c_str());
			}
		}
		return true;
	}

	HandlePlay::HandlePlay(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse)
		: ContentHandler(env, pSite, pReq, pResponse)
	{
	}

	HandlePlay::~HandlePlay()
	{
	}

	bool HandlePlay::setRangePrefix(const std::string& rngPf)
	{
		try
		{
			_cltSessPrx->setRangePrefix(rngPf);
			_cltSessCtx.rangePrefix = rngPf;
		}
		catch (const Ice::TimeoutException& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "perform setRangePrefix() caught(%s) on client session context proxy", ex.ice_name().c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandlePlay, "%s"), _szBuf);
			_statusCode = 503;
			return false;
		}
		catch (Ice::Exception& ex)
		{
			snprintf(_szBuf, sizeof(_szBuf) - 1, "perform setRangePrefix() caught(%s) on client session context proxy", ex.ice_name().c_str());
			SSMLOG(ErrorLevel, HandlerFmt(HandlePlay, "%s"), _szBuf);
			return false;
		}
		return true;
	}

	bool HandlePlay::prepareResponseParameter( const ::TianShanIce::Streamer::StreamInfo& info )
	{		
		ZQTianShan::Util::getPropertyDataWithDefault( info.props , "SPEED" , "0.000" , _returnScale );

		if( 0 == stricmp( _cltSessCtx.rangePrefix.c_str() , "npt" ) )
		{
			Ice::Int iCurPos = 0;
			Ice::Int iTotalTime = 0 ;
			ZQTianShan::Util::getPropertyDataWithDefault( info.props ,"CURRENTPOS" , 0 , iCurPos );
			ZQTianShan::Util::getPropertyDataWithDefault( info.props , "TOTALPOS" , 0 , iTotalTime );
			prepareResponseParameter( _returnScale , iCurPos , iTotalTime );
			return true;
		}
		else if (0 == stricmp( _cltSessCtx.rangePrefix.c_str(), "clock"))
		{
			Ice::Int iCtrlNum = 0 ;
			Ice::Int iOffset = 0;
			ZQTianShan::Util::getPropertyDataWithDefault( info.props , "USERCTRLNUM" , -3 , iCtrlNum );
			ZQTianShan::Util::getPropertyDataWithDefault( info.props , "ITEM_CURRENTPOS" , 0, iOffset );
			if( iCtrlNum <= -3 )
			{
				composeResponse( 500, "Can't get ctrlNum from PlayEx AMI callback");
				return false;
			}
			std::string utcTime;
			if (false == PlayInfo2UtcTime(iCtrlNum, iOffset, utcTime))
			{				
				composeResponse( _statusCode);
				return false;
			}
			_returnRange = "clock=";
			_returnRange += utcTime;
			_returnRange += '-';
			_pResponse->setHeader(HeaderRange, _returnRange.c_str());
			_pResponse->setHeader(HeaderScale, _returnScale.c_str());
			return true;
		}
		else
		{
			SSMLOG(ZQ::common::Log::L_ERROR,HandlerFmt(HandlePlay,"rangePrefix is wrong, neither clock nor npt, maybe DB corrput ?"));
			composeResponse( 500, "rangePrefix is wrong, neither clock nor npt, maybe DB corrput ?");
			return false;
		}
		
	}
	void HandlePlay::prepareResponseParameter( std::string& scale, Ice::Int& iCurrentPos, Ice::Int& iTotalPos )
	{		
		if (iTotalPos != 0)
		{
			snprintf( _szBuf, sizeof( _szBuf ) - 1, "%d.%03d-%d.%03d", 
				iCurrentPos / 1000, iCurrentPos % 1000, iTotalPos / 1000, iTotalPos % 1000);
		}
		else 
		{
			snprintf(_szBuf, sizeof( _szBuf ) - 1, "%d.%03d-", iCurrentPos / 1000, iCurrentPos % 1000);
		}

		_returnRange = "npt=";
		_returnRange += _szBuf;
		if ( iTotalPos == 0 &&  _tsConfig._response._dummyEndNpt > 0 )
		{
			char endNpt[20] = {0};
			snprintf(endNpt, sizeof(endNpt), "%d", _tsConfig._response._dummyEndNpt);
			_returnRange += endNpt;
		}
		_pResponse->setHeader(HeaderRange, _returnRange.c_str());
		_pResponse->setHeader(HeaderScale, _returnScale.c_str());
	}

	void HandlePlay::rollbackSpeed( )
	{
		if( _tsConfig._fixedSpeedSet.enable <= 0 )
			return ;
		int direction = 1, index = -1;
		int lastDirection =1 , lastIndex =-1;
		getLastScaleIndex( direction , index ,&lastDirection , &lastIndex);
		if( index <= -1)
			return;
		updateLastScaleIndex( lastDirection , lastIndex , lastDirection ,lastIndex);
	}

	bool HandlePlay::fixSpeed( float& requestScale )
	{
		if( _tsConfig._fixedSpeedSet.enable <= 0 )
			return true;
		
		int direction = 1, index = -1;
		int lastDirection = 1, lastIndex =-1;
		getLastScaleIndex( direction , index );
		lastDirection	= direction ;
		lastIndex		= index;
		
		int requestDirection = 0 ;
		if(  ( requestScale - 1.0f ) > 0.0001f )
		{//fast forward
			requestDirection = 1;
		}
		else if( requestScale < 0.0f )
		{//fast rewind
			requestDirection = -1;
		}
		else
		{
			//reset last scale status
			index		= -1;
			direction	= 1;
			updateLastScaleIndex( direction , index , lastDirection, lastIndex );			
			return true;
		}

		int iFastforwardSetSize = static_cast<int>( _tsConfig._fixedSpeedSet.forwardSpeeds.size());
		int iFastRewindSetSize	=  static_cast<int>( _tsConfig._fixedSpeedSet.backwardSpeeds.size());

		bool bUseForwardSet		= requestDirection > 0;
		
		std::vector<float> scaleSet;
		if( bUseForwardSet )
		{
			scaleSet = _tsConfig._fixedSpeedSet.forwardSpeeds;
		}
		else
		{
			scaleSet = _tsConfig._fixedSpeedSet.backwardSpeeds;
		}
		if( scaleSet.size() <= 0 )
		{
			updateLastScaleIndex( requestDirection , -1 , lastDirection, lastIndex);			
			return true;
		}

		if( _tsConfig._fixedSpeedSet.enableSpeedLoop >= 1 )//EnableFixedSpeedLoop
		{
			index = (int)( direction * requestDirection ) < 0 ? 0 : ( (++index) % scaleSet.size() );
		}
		else
		{
			index = ( direction * requestDirection ) < 0 ? 0 : (++index) ;
			if( index >= (int)scaleSet.size() )
			{//reach the fix speed set end
				SSMLOG(WarningLevel, HandlerFmt(HandlePlay, "can't not change speed, we reach the end of fixed speed set"));
				return false;
			}		
		}
		float scaleOld = requestScale;
		requestScale = scaleSet[index];
		updateLastScaleIndex( requestDirection , index ,lastDirection , lastIndex );
		SSMLOG(InfoLevel ,HandlerFmt(HandlePlay,"scale changed from [%f] to [%f]"),scaleOld , requestScale );
		return true;
	}

	bool HandlePlay::process()
	{
		
		Ice::Identity ident;
		ident.name = _session;
		ident.category = ServantType;
		if (false == openSessionCtx(ident))
		{
			//composeErrorResponse();
			composeResponse(_statusCode);
			return false;
		}

		if (1 == _cltSessCtx.requestType) // SeaChange
			_pRequestWriter->setHeader(HeaderFormatType, SeaChangeFormat);
		else 
		{
			_pRequestWriter->setHeader(HeaderFormatType, TianShanFormat);
			_bNeedModifyResponse = false;
		}

		if (false == getWeiwooPrx() || false == getStreamPrx() || false == getPurchasePrx() || false == renewSession())
		{
			//composeErrorResponse();
			composeResponse(_statusCode);
			return false;
		}		
		
		int from = 0; //default to 0		

		float	fSeekTime = 0.0f;

		TianShanIce::StrValues expectValues;

		float fScale = 0.0f;

		String::removeChar(_requestScale = getRequestHeader(HeaderScale), ' ');
		if (false == _requestScale.empty())
		{			
			fScale = static_cast<float>( atof(_requestScale.c_str()) );
		}

		if( !fixSpeed( fScale ))
		{
			int errCode = 455;
			composeResponse( errCode );//method not allowed
			return false;

			/* what's the hell the following logic about?
			switch( _tsConfig._fixedSpeedSet.errorcode)
			{
			case 200:
				{
					SSMLOG(ZQ::common::Log::L_DEBUG,HandlerFmt(HandlePlay, "failed to loop speed, try to get stream information"));
					std::string strScale ;
					Ice::Int iCurPos = 0;
					Ice::Int iTotalTime = 0;
					if( !getStreamPlayInfo(strScale , iCurPos , iTotalTime))
					{
						composeResponse( _statusCode );
						return false;
					}
					prepareResponseParameter( strScale , iCurPos , iTotalTime);					
					composeResponse( 200 );
					return true;
				}
				break;

			default:
				{
					errCode = _tsConfig._fixedSpeedSet.errorcode;
					composeResponse( errCode );//method not allowed
					return false;
				}
			}
			*/
		}		

		expectValues.push_back("SPEED");
		expectValues.push_back("STATE");
		expectValues.push_back("USERCTRLNUM");

		// gain range field, and remove any blank char.
		String::removeChar(_requestRange = getRequestHeader(HeaderRange), ' ');
		// take "Range: ", "Range: clock=", "Range: npt=", "Range: npt=now-" as not seek semantics.
		if (false == _requestRange.empty() // not empty
			&& 0 != stricmp(_requestRange.c_str(), "clock=") // and not equal to "clock="
			&& 0 != stricmp(_requestRange.c_str(), "npt=") // and not equal to "npt="
			//&& 0 != stricmp(_requestRange.c_str(), "npt=now-")) // and not equal to "npt=now-"
			&& 0 != stricmp(String::nLeftStr(_requestRange, 7).c_str(), "npt=now") ) // and not equal to "npt=now"
		{
			if (0 == stricmp(String::nLeftStr(_requestRange, 4).c_str(), "npt="))
			{
				// "Range: npt=<seconds>[-[<seconds>]]
				std::string seekTime;
				int chrPos;
				seekTime = String::getRightStr(_requestRange, "=", true);
				if (true == String::hasChar(seekTime, '-', chrPos))
					seekTime = String::leftStr(seekTime, chrPos); // get the first <seconds> string


				from = 1;
				sscanf(seekTime.c_str() ,"%f",&fSeekTime);

				if (0 != stricmp(_cltSessCtx.rangePrefix.c_str(), "npt"))
				{
					if (false == setRangePrefix("npt"))
					{
						//composeErrorResponse();
						composeResponse(_statusCode);
						return false;
					}
				}				
			}
			else if (0 == stricmp(String::nLeftStr(_requestRange, 6).c_str(), "clock=")
				|| 0 == stricmp(String::nLeftStr(_requestRange, 3).c_str(), "now"))
			{
				// "Range: clock=<UTC-time>[-[<UTC-time>]]
				std::string utcStr = _requestRange;
				if (0 != stricmp(String::nLeftStr(_requestRange, 3).c_str(), "now"))
				{
					int chrPos;
					utcStr = String::getRightStr(_requestRange, "=", true);
					if (true == String::hasChar(utcStr, '-', chrPos))
						utcStr = String::leftStr(utcStr, chrPos); // get the first <UTC-time>
				}

				// DO: get userCtrlNum and Offset
				Ice::Int iCtrlNum = 0;
				Ice::Int iOffset = 0;
				Ice::Int iStartPos = 0;
				if (false == UtcTime2PlayInfo(utcStr, iCtrlNum, iOffset, iStartPos))
				{
					//composeErrorResponse();
					composeResponse(_statusCode);
					return false;
				}

				try
				{
					
					TianShanIce::Streamer::PlaylistPrx playlistPrx = TianShanIce::Streamer::PlaylistPrx::uncheckedCast(_streamPrx);
					// throw exception InvalidParameter, ServerError, InvalidStateOfArt;
					// according to TsStream.ICE, version: 42
					//playlistPrx->seekToPosition(iCtrlNum, iOffset, iStartPos);

					expectValues.push_back("ITEM_CURRENTPOS");
					expectValues.push_back("ITEM_TOTALPOS");
					//use new api playItem
					SSMLOG(DebugLevel, 
						HandlerFmt(HandlePlay, "do stream(%s).seekToPosition with ctrlNum[%d] offset[%d] startPos[%d] scale[%f]"),
						_cltSessCtx.streamPrxID.c_str() , iCtrlNum , iOffset , iStartPos ,fScale );
#if ICE_INT_VERSION / 100 >= 306
					playAsyncCBPtr onPlayCbPtr = new playAsyncCB(this);
					Ice::CallbackPtr genericCB = Ice::newCallback(onPlayCbPtr, &playAsyncCB::playItem);
					playlistPrx->begin_playItem(iCtrlNum,iOffset,iStartPos,fScale,expectValues,genericCB);
#else
					playlistPrx->playItem_async( new playItemAsync(this), iCtrlNum,iOffset,iStartPos,fScale,expectValues);
#endif
					SSMLOG(InfoLevel, HandlerFmt(HandlePlay, "stream(%s).seekToPosition successfully"), _cltSessCtx.streamPrxID.c_str());
					
				}
				catch (TianShanIce::InvalidStateOfArt& ex)
				{
					snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught by stream(%s).seekToPosition"
						, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), _cltSessCtx.streamPrxID.c_str());
					SSMLOG(WarningLevel, HandlerFmt(HandlePlay, "%s"), _szBuf);
					//_ok_statusline = ResponseMethodNotValidInThisState;
					_statusCode = 455;
				}
				catch (TianShanIce::InvalidParameter& ex)
				{
					snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught by stream(%s).seekToPosition"
						, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), _cltSessCtx.streamPrxID.c_str());
					SSMLOG(ErrorLevel, HandlerFmt(HandlePlay, "%s"), _szBuf);
					//_error_statusline = ResponseParameterNotUnderstood;
					//composeErrorResponse();
					_statusCode = 451;
					composeResponse(_statusCode);
					return false;
				}
				catch (TianShanIce::ServerError& ex)
				{
					snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[%s:%04d] %s caught by stream(%s).seekToPosition"
						, ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str(), _cltSessCtx.streamPrxID.c_str());
					SSMLOG(ErrorLevel, HandlerFmt(HandlePlay, "%s"), _szBuf);
					//composeErrorResponse();
					composeResponse(_statusCode);
					return false;
				}
				catch (const Ice::TimeoutException& ex)
				{
					snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[PlayRequest:0301] caught by stream(%s).seekToPosition", 
						ex.ice_name().c_str(), _cltSessCtx.streamPrxID.c_str());
					SSMLOG(ErrorLevel, HandlerFmt(HandlePlay, "%s"), _szBuf);
					//composeErrorResponse();
					_statusCode = 503;
					composeResponse(_statusCode);
					return false;
				}
				catch (Ice::Exception& ex)
				{
					snprintf(_szBuf, sizeof(_szBuf) - 1, "%s[PlayRequest:0301] caught by stream(%s).seekToPosition", 
						ex.ice_name().c_str(), _cltSessCtx.streamPrxID.c_str());
					SSMLOG(ErrorLevel, HandlerFmt(HandlePlay, "%s"), _szBuf);
					//composeErrorResponse();
					composeResponse(_statusCode);
					return false;
				}

				if (0 != stricmp(_cltSessCtx.rangePrefix.c_str(), "clock"))
				{
					if (false == setRangePrefix("clock"))
					{
						//composeErrorResponse();
						composeResponse(_statusCode);
						return false;
					}
				}
				return true;
			}
			else // #define ResponseInvalidRange		"RTSP/1.0 457 Invalid Range"
			{
				snprintf(_szBuf, sizeof(_szBuf) - 1, "invalid range field(%s)", _requestRange.c_str());
				SSMLOG(ErrorLevel, HandlerFmt(HandlePlay, "%s"), _szBuf);
				//_error_statusline = ResponseInvalidRange;
				//composeErrorResponse();
				_statusCode = 457;
				composeResponse(_statusCode);
				return false;
			}
		}
		if (0 == stricmp(_cltSessCtx.rangePrefix.c_str(), "npt"))
		{
			expectValues.push_back("CURRENTPOS");
			expectValues.push_back("TOTALPOS");
		}
		else
		{
			expectValues.push_back("ITEM_CURRENTPOS");
			expectValues.push_back("ITEM_TOTALPOS");
		}

		{ // ticket#19706 STB of S1 is restrict at the connection thru which to receive the ANNOUNCE
		  // so save connId as a property
			char tmpbuf[32];
			uint16 bufSize	= sizeof(tmpbuf) - 1;
			const char* connId = _pRequestWriter->getHeader("SYS#ConnID", tmpbuf, &bufSize );

			if (NULL != connId && strlen(connId) >0)
				updateProperty("ctrlConnId", connId);
		}
		
		//call playEx		
		try
		{
#if ICE_INT_VERSION / 100 >= 306
			playAsyncCBPtr onPlayCbPtr = new playAsyncCB(this);
			Ice::CallbackPtr genericCB = Ice::newCallback(onPlayCbPtr, &playAsyncCB::playEx);
			_streamPrx->begin_playEx(fScale , 
				static_cast<Ice::Long>( fSeekTime * 1000 ) ,
				from ,
				expectValues, genericCB);
#else
			_streamPrx->playEx_async( ( new playExAsync(this)) ,
										fScale , 
										static_cast<Ice::Long>( fSeekTime * 1000 ) ,
										from ,
										expectValues );
#endif
		}
		catch (const Ice::TimeoutException& ex)
		{
			SSMLOG(ZQ::common::Log::L_ERROR, HandlerFmt(HandlePlay," call PlayEx_async failed , caught an Ice Exception:%s"),ex.ice_name().c_str());
			//composeErrorResponse();
			_statusCode = 503;
			composeResponse( _statusCode , ex.ice_name().c_str() );
			return false;
		}
		catch( const Ice::Exception& ex )
		{
			SSMLOG(ZQ::common::Log::L_ERROR, HandlerFmt(HandlePlay," call PlayEx_async failed , caught an Ice Exception:%s"),ex.ice_name().c_str());
			//composeErrorResponse();
			composeResponse(_statusCode);
			return false;
		}		

		_env.removeCachedSess(_session);
		return true;
	}

	PlayResponse::PlayResponse(Environment& env, IStreamSmithSite* pSite, IClientRequest* pReq, IServerResponse* pResponse)
		: FixupResponse(env, pSite, pReq, pResponse)
	{
	}

	PlayResponse::~PlayResponse()
	{
	}

	bool PlayResponse::process()
	{
		if (0 == stricmp(getRequestHeader(HeaderFormatType).c_str(), SeaChangeFormat))
		{
			_pResponse->setHeader(HeaderSeaChangeNotice, getResponseHeader(HeaderTianShanNotice).c_str());
			_pResponse->setHeader(HeaderTianShanNotice, ""); // remove "TianShan-Notice"
		}
		//postResponse(); // need not modify response, post directly
		return true;
	}

} // end namespace TianShanS1

