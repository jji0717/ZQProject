#include "PauseRequest.h"

extern ZQ::common::Log* s1log;

namespace TianShanS1
{

#define HandlerFmtEx(_C, _X) CLOGFMT(_C, "Req(%s)Sess(%s)Seq(%s)Mtd(%s:%s) " _X), _pHandler->_reqIdent.c_str(), _pHandler->_session.c_str(), _pHandler->_sequence.c_str(), _pHandler->_phase.c_str(), _pHandler->_method.c_str()

#if  ICE_INT_VERSION / 100 >= 306
	class pauseExAsyncCB : public IceUtil::Shared
	{
	public:
		pauseExAsyncCB(HandlePause::Ptr pHandler){
			_pHandler = pHandler;
		}
	private:
		void handleException(const Ice::Exception& ex){}
	public:
		void pauseEx(const Ice::AsyncResultPtr& r)
		{
				TianShanIce::Streamer::StreamPrx pausePrx = TianShanIce::Streamer::StreamPrx::uncheckedCast(r->getProxy());
            try 
            {   
                pausePrx->end_pauseEx(r);
            }   
            catch(const Ice::Exception& ex) 
            {   
 			    handleException(ex);
             }
               
		}
	private:
		HandlePause::Ptr _pHandler;
	};
	typedef IceUtil::Handle<pauseExAsyncCB> pauseExAsyncCBPtr;
#else
	class pauseExAsync : public TianShanIce::Streamer::AMI_Stream_pauseEx
	{
	public:
		pauseExAsync( HandlePause::Ptr pHandler )
		{
			_pHandler = pHandler;
			assert( _pHandler );
			_pHandler->_pRequestWriter->addRef();
			_pHandler->setReturnType( RequestHandler::RETURN_ASYNC );
			_pHandler->_pRequestWriter->setContext(CLIENT_REQUEST_DISABLE_AUTO_DELETE,"1");
		}
		
		~pauseExAsync( )
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
				//delete _pHandler->_pRequest;
			}
			_pHandler->_pRequestWriter->release();
			/*delete _pHandler;*/
		}	
	public:
		void ice_response(const ::TianShanIce::Streamer::StreamInfo& infoRet )
		{
			SSMLOG(ZQ::common::Log::L_INFO,
				HandlerFmtEx(PlayHandler, "perform pauseEx() on stream: [%s] successfully:%s"), 
				_pHandler->_cltSessCtx.streamPrxID.c_str() , _pHandler->dumpTSStreamInfo(infoRet).c_str() );

			TianShanIce::Properties::const_iterator itSpeed = infoRet.props.find("SPEED");
			if( itSpeed != infoRet.props.end() )
			{
				_pHandler->_returnScale = itSpeed->second;				
			}

			bool bHasRange = false;


			if ( 0 == stricmp( _pHandler->_cltSessCtx.rangePrefix.c_str(), "npt") )
			{
				Ice::Int iCurrentPos = 0, iTotalPos = 0;

				TianShanIce::Properties::const_iterator itCurPos = infoRet.props.find("CURRENTPOS");
				if( itCurPos != infoRet.props.end() )
				{
					bHasRange = true;
					iCurrentPos = atoi( itCurPos->second.c_str() );
				}
				else
				{
					iCurrentPos = 0;
				}

				TianShanIce::Properties::const_iterator itTotalPos = infoRet.props.find("TOTALPOS");
				if( itTotalPos != infoRet.props.end() )
				{
					iTotalPos = atoi( itTotalPos->second.c_str() );
				}
				else
				{
					iTotalPos = 0;
				}

				if (iTotalPos != 0)
				{
					snprintf( _pHandler->_szBuf, sizeof( _pHandler->_szBuf ) - 1, "%d.%03d-%d.%03d", 
						iCurrentPos / 1000, iCurrentPos % 1000, iTotalPos / 1000, iTotalPos % 1000);
				}
				else 
				{
					snprintf( _pHandler->_szBuf, sizeof( _pHandler->_szBuf ) - 1, "%d.%03d-", iCurrentPos / 1000, iCurrentPos % 1000);
				}

				if(bHasRange)
				{
					_pHandler->_returnRange = "npt=";
					_pHandler->_returnRange += _pHandler->_szBuf;
					if (_tsConfig._response._dummyEndNpt > 0)
					{
						char endNpt[20] = {0};
						snprintf(endNpt, sizeof(endNpt), "%d", _tsConfig._response._dummyEndNpt);
						_pHandler->_returnRange += endNpt;
					}
				}

			}
			else if (0 == stricmp(_pHandler->_cltSessCtx.rangePrefix.c_str(), "clock"))
			{
				_pHandler->_returnScale.clear();
				Ice::Int iCtrlNum = 0, iOffset = 0;
				if ( true == _pHandler->getPlaylistPlayInfo(_pHandler->_returnScale, iCtrlNum, iOffset) )
				{
					std::string utcTime;
					if (false == _pHandler->PlayInfo2UtcTime(iCtrlNum, iOffset, utcTime))
					{
						//_pHandler->composeErrorResponse();
						_pHandler->composeResponse(_pHandler->_statusCode);
						return;
					}
					_pHandler->_returnRange = "clock=";
					_pHandler->_returnRange += utcTime;
					_pHandler->_returnRange += '-';
				}
				else 
				{
					//_pHandler->composeErrorResponse();
					_pHandler->composeResponse(_pHandler->_statusCode);
					return ;
				}
			}

			_pHandler->_pResponse->setHeader(HeaderRange, _pHandler->_returnRange.c_str());
			//_pHandler->_pResponse->setHeader(HeaderScale, _pHandler->_returnScale.c_str());
			_pHandler->_statusCode = 200;
			_pHandler->composeResponse(_pHandler->_statusCode);
			//_pHandler->composeRightResponse();
			
			Ice::Long cur		= ZQTianShan::now();
			Ice::Long timeUsed	= cur - _pHandler->getStartTime();

			SSMLOG(ZQ::common::Log::L_INFO,
				CLOGFMT(NGODEnv, "Sess(%s)Seq(%s)[success]process[%s]request, used [%lld]ms"), 
				_pHandler->getSession().c_str(),
				_pHandler->getSequence().c_str(),
				_pHandler->getMethod().c_str(), 
				timeUsed );
		}

		virtual void ice_exception(const ::Ice::Exception& ex)
		{	
			try
			{
				ex.ice_throw();
			}
			catch( const Ice::TimeoutException& ex)
			{
				SSMLOG(ZQ::common::Log::L_INFO,HandlerFmtEx(HandlePause, "failed perform pauseEx() with TianShan Exception:%s"), 
					ex.ice_name().c_str() );					
				_pHandler->_statusCode = 503;
				_pHandler->composeResponse(_pHandler->_statusCode, ex.ice_name().c_str());
			}
			catch( const Ice::SocketException& ex)
			{
				SSMLOG(ZQ::common::Log::L_INFO,HandlerFmtEx(HandlePause, "failed perform pauseEx() with TianShan Exception:%s"), 
					ex.ice_name().c_str() );					
				_pHandler->_statusCode = 503;
				_pHandler->composeResponse(_pHandler->_statusCode, ex.ice_name().c_str());
			}
			catch( const TianShanIce::InvalidStateOfArt& ex)
			{
				SSMLOG(ZQ::common::Log::L_INFO,HandlerFmtEx(HandlePause, "failed perform pauseEx() with TianShan Exception:%s"), 
					ex.message.c_str() );					
				_pHandler->_statusCode = 455;
				_pHandler->composeResponse(_pHandler->_statusCode, ex.message.c_str());
			}
			catch( const TianShanIce::ServerError& ex)
			{
				_pHandler->processServerErrorWithRtspErrCode(ex);
				_pHandler->composeResponse(_pHandler->_statusCode, ex.message.c_str());
			}
			catch( TianShanIce::BaseException& ex)
			{
				SSMLOG(ZQ::common::Log::L_INFO,HandlerFmtEx(HandlePause, "failed perform pauseEx() with TianShan Exception:%s"), 
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
				SSMLOG(ZQ::common::Log::L_INFO,
					HandlerFmtEx(HandlePause, "failed perform pauseEx() on stream[%s] with Ice Exception:%s "), 
					ex.ice_name().c_str() , _pHandler->_cltSessCtx.streamPrxID.c_str() );				
				_pHandler->_statusCode = 500;
				_pHandler->composeResponse(_pHandler->_statusCode, ex.ice_name().c_str());
			}					

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

		HandlePause::Ptr _pHandler;

	};
#endif
	FixupPause::FixupPause(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse)
		: FixupRequest(env, pSite, pReq, pResponse)
	{
	}
	
	FixupPause::~FixupPause()
	{
	}

	bool FixupPause::process()
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

	HandlePause::HandlePause(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse)
		: ContentHandler(env, pSite, pReq, pResponse)
	{
	}

	HandlePause::~HandlePause()
	{
	}

	bool HandlePause::process()
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
		
		TianShanIce::StrValues expectValues;

		expectValues.push_back("SPEED");
		expectValues.push_back("STATE");
		expectValues.push_back("USERCTRLNUM");

		//clear last scale status
		updateLastScaleIndex( 1 , -1 , 1 , -1 );

		if (0 == stricmp(_cltSessCtx.rangePrefix.c_str(), "npt"))
		{
			expectValues.push_back("CURRENTPOS");
			expectValues.push_back("TOTALPOS");
		}
		else
		{
			expectValues.push_back("ITEM_CURRENTPOS");
			expectValues.push_back("GET_ITEM_TOTALPOS");
		}

		{ // ticket#19706 STB of S1 is restrict at the connection thru which to receive the ANNOUNCE
			// so save connId as a property
			char tmpbuf[32];
			uint16 bufSize	= sizeof(tmpbuf) - 1;
			const char* connId = _pRequestWriter->getHeader("SYS#ConnID", tmpbuf, &bufSize );

			if (NULL != connId && strlen(connId) >0)
				updateProperty("ctrlConnId", connId);
		}

		try
		{
#if ICE_INT_VERSION / 100 >= 306
			pauseExAsyncCBPtr onPauseCbPtr = new pauseExAsyncCB(this);
			Ice::CallbackPtr genericCB = Ice::newCallback(onPauseCbPtr, &pauseExAsyncCB::pauseEx);
			_streamPrx->begin_pauseEx(expectValues, genericCB);
#else
			_streamPrx->pauseEx_async( ( new pauseExAsync(this)) , expectValues );
#endif
		}
		catch (const Ice::TimeoutException& ex)
		{
			SSMLOG(ZQ::common::Log::L_ERROR, HandlerFmt(HandlePause," call pauseEx_async failed , caught an Ice Exception:%s"),ex.ice_name().c_str());
			//composeErrorResponse();
			_statusCode = 503;
			composeResponse(_statusCode,ex.ice_name().c_str());
			return false;
		}
		catch( const Ice::Exception& ex )
		{
			SSMLOG(ZQ::common::Log::L_ERROR, HandlerFmt(HandlePause," call pauseEx_async failed , caught an Ice Exception:%s"),ex.ice_name().c_str());
			//composeErrorResponse();
			composeResponse( _statusCode, ex.ice_name().c_str() );
			return false;
		}

		_env.removeCachedSess(_session);
		return true;
	}

	PauseResponse::PauseResponse(Environment& env, IStreamSmithSite* pSite, IClientRequest* pReq, IServerResponse* pResponse)
		: FixupResponse(env, pSite, pReq, pResponse)
	{
	}

	PauseResponse::~PauseResponse()
	{
	}

	bool PauseResponse::process()
	{
		if (0 == stricmp(getRequestHeader(HeaderFormatType).c_str(), SeaChangeFormat))
		{
			_pResponse->setHeader(HeaderSeaChangeNotice, getResponseHeader(HeaderTianShanNotice).c_str());
			_pResponse->setHeader(HeaderTianShanNotice, ""); // remove "TianShan-Notice"
		}
		//postResponse(); // need not modify response, post directly
		return true;
	}

} // namespace TianShanS1

