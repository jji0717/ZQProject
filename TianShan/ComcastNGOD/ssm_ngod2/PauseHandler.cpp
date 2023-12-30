#include "./PauseHandler.h"
#include <TianShanIceHelper.h>

#ifdef _DEBUG
#include <iostream>
using namespace std;
#endif


#define HANDLERLOGEX _pHandler->_ssmNGODr2c1._fileLog
#define HANDLERLOGFMTEX(_X, _T) CLOGFMT(_X, "Sess(%s)Seq(%s)Req(%p)Mtd(%s) " _T),_pHandler->_session.c_str(), _pHandler->_sequence.c_str(), _pHandler->_pRequest, _pHandler->_method.c_str()

class pauseHandlerAsync : public TianShanIce::Streamer::AMI_Stream_pauseEx
{
public:
	pauseHandlerAsync( PauseHandler::Ptr pHandler )
	{
		_pHandler = pHandler;
		assert( _pHandler != NULL );
		_pHandler->_pRequest->addRef();
		_pHandler->setReturnType( RequestHandler::RETURN_ASYNC );
		_pHandler->_pRequest->setContext(CLIENT_REQUEST_DISABLE_AUTO_DELETE,"1");
	}
	~pauseHandlerAsync( )
	{
		_pHandler->_pRequest->release( );

		// 			_pHandler->_pRequest->setContext(CLIENT_REQUEST_DISABLE_AUTO_DELETE,"0");
		// 			//post to next phase
		// 			IStreamSmithSite* pSite = _pHandler->_pRequest->getSite();
		// 			assert( pSite != NULL );
		// 			pSite->postRequest(_pHandler->_pRequest,IClientRequest::FixupResponse);
// 		{
// 			delete _pHandler;
// 			_pHandler = NULL;
// 		}
		
	}

	virtual void ice_response(const ::TianShanIce::Streamer::StreamInfo& infoRet) 
	{
		char localBuffer[1024];
		HANDLERLOGEX(ZQ::common::Log::L_INFO, HANDLERLOGFMTEX(pauseHandlerAsync, "performed pauseEx() on stream[%s] successfully: [%s]"), 
			_pHandler->_context.streamFullID.c_str(),
			ZQTianShan::Util::dumpStreamInfo( infoRet, localBuffer, sizeof(localBuffer)-1 ) );

		//get result information
		::TianShanIce::Properties::const_iterator itRet ;

		std::string		strTimeOffset;
		std::string		strCurrentSpeed;

		itRet = infoRet.props.find("CURRENTPOS");
		if( itRet != infoRet.props.end() )
		{
			int iOffset = atoi( itRet->second.c_str() );
			char szTemp[256];
			snprintf(szTemp,sizeof(szTemp)-1,"%d.%d",iOffset/1000,iOffset%1000);
			strTimeOffset = "npt=" ;
			strTimeOffset = strTimeOffset + szTemp ;
			strTimeOffset = strTimeOffset + "- ";
		}
		else
		{
			strTimeOffset = "npt = 0 - ";
		}

		itRet = infoRet.props.find("TOTALPOS");
		if( itRet != infoRet.props.end( ) )
		{
			if( itRet->second.length() > 0 )
			{
				int iOffset = atoi( itRet->second.c_str() );
				if( iOffset > 0 )
				{
					char szTemp[256];
					snprintf(szTemp,sizeof(szTemp)-1,"%d.%d",iOffset/1000,iOffset%1000);
					strTimeOffset = strTimeOffset + szTemp;
				}
			}
		}


		itRet = infoRet.props.find("SPEED");

		if ( itRet != infoRet.props.end() )
			strCurrentSpeed = itRet->second;
		else
			strCurrentSpeed = "0.0";

		// add by zjm to support session history
		std::string strCurTime = "0";
		itRet = infoRet.props.find("ITEM_CURRENTPOS");
		if (itRet != infoRet.props.end())
		{
			int iOffset = atoi( itRet->second.c_str() );
			char szTemp[256];
			snprintf(szTemp,sizeof(szTemp)-1,"%d.%d",iOffset/1000,iOffset%1000);
			strCurTime = szTemp;
		}

		std::string strCtrlNum = "1";
		itRet = infoRet.props.find("USERCTRLNUM");
		if (itRet != infoRet.props.end())
		{
			strCtrlNum = itRet->second;
		}
		_pHandler->addPauseEvent(strCurrentSpeed, strCurTime, strCtrlNum);


		IServerResponse* pResponse = _pHandler->getResponse();

		//pResponse->setHeader(NGOD_HEADER_SCALE, strCurrentSpeed.c_str() );
		pResponse->setHeader(NGOD_HEADER_RANGE, strTimeOffset.c_str() );

		_pHandler->responseOK();

		Ice::Long cur = ZQTianShan::now();
		Ice::Long timeUsed = cur - _pHandler->getStartTime();
		_pHandler->_ssmNGODr2c1._fileLog(ZQ::common::Log::L_INFO,
			CLOGFMT(pauseHandlerAsync, "Sess(%s)Seq(%s)[success]process[%s]request, used [%lld]ms"), 
			_pHandler->getSession().c_str(),
			_pHandler->getSequence().c_str(),
			_pHandler->getRequestType().c_str(), 
			timeUsed);

	}
	virtual void ice_exception(const ::Ice::Exception& ex) 
	{
		try
		{
			ex.ice_throw();
		}
		catch( const Ice::TimeoutException& )
		{
			HANDLERLOGEX(ZQ::common::Log::L_INFO, HANDLERLOGFMTEX(pauseHandlerAsync, "caught exception[Ice::TimeoutException] during perform pauseEx "));

			if( _sopConfig._sopRestrict._penaltyEnableMask & PENALTY_ENABLE_MASK_PAUSE)
				_pHandler->_ssmNGODr2c1.addPenaltyToStreamer( _pHandler->_context.sopname , _pHandler->_context.streamNetId );

			_pHandler->responseError(RESPONSE_INTERNAL_ERROR);
		}
		catch( const Ice::ConnectionRefusedException& )
		{
			HANDLERLOGEX(ZQ::common::Log::L_INFO, HANDLERLOGFMTEX(pauseHandlerAsync, "caught exception[Ice::ConnectionRefusedException] during perform pauseEx"));	

			if( _sopConfig._sopRestrict._penaltyEnableMask & PENALTY_ENABLE_MASK_PAUSE)
				_pHandler->_ssmNGODr2c1.addPenaltyToStreamer( _pHandler->_context.sopname , _pHandler->_context.streamNetId );

			_pHandler->responseError(RESPONSE_INTERNAL_ERROR);
		}
		catch( const TianShanIce::InvalidStateOfArt& bEx )
		{
			HANDLERLOGEX(ZQ::common::Log::L_INFO, HANDLERLOGFMTEX(pauseHandlerAsync, "caugth exception during perform pauseEx(): [%s]:%d %s"), 
				bEx.ice_name().c_str(), bEx.errorCode, bEx.message.c_str() );
			_pHandler->responseError(RESPONSE_INVALID_STATE);
		}
		catch( const TianShanIce::BaseException& bEx)
		{
			HANDLERLOGEX(ZQ::common::Log::L_INFO, HANDLERLOGFMTEX(pauseHandlerAsync, "caugth exception during perform pauseEx(): [%s]:%d %s"), 
				bEx.ice_name().c_str(), bEx.errorCode, bEx.message.c_str() );	
			_pHandler->responseError(RESPONSE_INTERNAL_ERROR);

		}
		catch( const Ice::Exception& ex )
		{
			HANDLERLOGEX(ZQ::common::Log::L_INFO, HANDLERLOGFMTEX(pauseHandlerAsync, "caugth exception[%s] during perform pausEx() on stream[%s]"), 
				ex.ice_name().c_str(), _pHandler->_context.streamFullID.c_str() );		
			_pHandler->responseError(RESPONSE_INTERNAL_ERROR);
		}		
		
		
		Ice::Long cur = ZQTianShan::now();
		Ice::Long timeUsed = cur - _pHandler->getStartTime();
		_pHandler->_ssmNGODr2c1._fileLog(ZQ::common::Log::L_INFO, CLOGFMT(pauseHandlerAsync, "Sess(%s)Seq(%s)[failed]process[%s]request, used [%lld]ms"), 
			_pHandler->getSession().c_str(),
			_pHandler->getSequence().c_str(),
			_pHandler->getRequestType().c_str(), 
			timeUsed);
	}

private:
	PauseHandler::Ptr	_pHandler;

};



PauseHandler::PauseHandler(NGODEnv& ssm, IStreamSmithSite* pSite, IClientRequestWriter* pReq) : RequestHandler(ssm, pSite, pReq)
{
	_method = "PAUSE";
	_inoutMap[MAP_KEY_METHOD] = _method;
#ifdef _DEBUG
	cout<<"construct PAUSE handler"<<endl;
#endif

}

PauseHandler::~PauseHandler()
{
#ifdef _DEBUG
	cout<<"deconstruct PAUSE handler"<<endl;
#endif

}

void PauseHandler::addPauseEvent(const std::string strSpeed, const std::string strNPT, const std::string streamResourceID)
{
	// add by zjm to support session history
	if (_ngodConfig._sessionHistory.enableHistory > 0 && _ngodConfig._sessionHistory.enablePauseEvent > 0)
	{
		NGODr2c1::SessionEventRecord sessionEvent;
		sessionEvent.eventTime = NgodUtilsClass::generatorISOTime();
		sessionEvent.streamResourceID = streamResourceID;
		sessionEvent.NPT = strNPT;
		sessionEvent.prop["scale"] = strSpeed;
		sessionEvent.prop["newState"] = "PAUSE";
		sessionEvent.eventType = NGODr2c1::UserEvent;
		_pContextPrx->addEventRecord(sessionEvent);
	}
}

RequestProcessResult PauseHandler::process()
{
	HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(PauseHandler, "start processing"));

	if (false == _canProcess)
	{
		HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PauseHandler, "failed to process the request because[%s]"), szBuf);
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PauseHandler, "failed to process the request because[%s]"), szBuf);
		return RequestError;
	}
	if (_ngodConfig._MessageFmt.rtspNptUsage <= 0)
	{
		if (!handshake(_requireProtocol, 2, 4))
		{
			return RequestError;
		}
	}

	if (false == getContext())
	{
		responseError(RESPONSE_SESSION_NOTFOUND);
		return RequestError;
	}

	if (false == renewSession())
	{
		responseError(RESPONSE_INTERNAL_ERROR);
		return RequestError;
	}
	
	if( !checkStreamer() )
	{
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PauseHandler, "streamer is unavailable [%s:%s]"), 
			_context.sopname.c_str() , _context.streamNetId.c_str() );
		responseError(RESPONSE_SERVICE_UNAVAILABLE);
		return RequestError;
	}

	if (_ngodConfig._MessageFmt.rtspNptUsage <= 0)
	{
		char versionCode[8];
		sprintf(versionCode, "%d", _requireProtocol);
		if (false == updateContextProp("RequireC1", versionCode))
		{
			responseError(RESPONSE_INTERNAL_ERROR);
			return RequestError;
		}
		HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(PauseHandler, "update C1 Reqire header in context[RequireC1]=%s]"), versionCode);
	}

	std::string strTimeOffset;

	// DO: get stream proxy in order to pause()
	if (false == getStream())
	{
		responseError(RESPONSE_INTERNAL_ERROR);
		return RequestError;
	}

	//update C1ConnectionId
	::std::string connId = getRequestHeader("SYS#ConnID");
	if (false == updateContextProp(C1CONNID, connId))
	{
		responseError(RESPONSE_INTERNAL_ERROR);
		return RequestError;
	}

	try
	{
		TianShanIce::StrValues expectValues;

		// add by zjm to support item npt 
		expectValues.push_back("ITEM_CURRENTPOS");

		expectValues.push_back("CURRENTPOS");
		expectValues.push_back("TOTALPOS");
		expectValues.push_back("SPEED");
		expectValues.push_back("STATE");
		expectValues.push_back("USERCTRLNUM");

		setReturnType(RETURN_ASYNC);
		HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(PauseHandler, "calling pauseEx() on stream: [%s]"), _context.streamFullID.c_str());
		mStreamPrx->pauseEx_async( (new pauseHandlerAsync(this)), expectValues);		
		
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		snprintf(szBuf, MY_BUFFER_SIZE - 1,"pauseEx() on stream[%s] caught [%s]:[%s]", _context.streamFullID.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PauseHandler, "%s"), szBuf);
		HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PauseHandler, "%s"), szBuf);
		responseError(RESPONSE_INTERNAL_ERROR);
		return RequestError;
	}
	catch(const ::Ice::Exception& ex)
	{
		snprintf(szBuf, MY_BUFFER_SIZE - 1,"pauseEx() on stream[%s] caught [%s]", _context.streamFullID.c_str(), ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PauseHandler, "%s"), szBuf);
		HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PauseHandler, "%s"), szBuf);
		responseError(RESPONSE_INTERNAL_ERROR);
		return RequestError;
	}
	return RequestDone;
}

