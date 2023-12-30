#include "./PlayHandler.h"
#include <TianShanIceHelper.h>

#ifdef _DEBUG
#include <iostream>
using namespace std;
#endif

#define HANDLERLOGEX _pHandler->_ssmNGODr2c1._fileLog
#define HANDLERLOGFMTEX(_X, _T) CLOGFMT(_X, "Sess(%s)Seq(%s)Req(%p)Mtd(%s) " _T),_pHandler->_session.c_str(), _pHandler->_sequence.c_str(), _pHandler->_pRequest, _pHandler->_method.c_str()

class PlayResponseAsync 
{
public:
	PlayResponseAsync(PlayHandler::Ptr pHandler )
	{
		pHandler->_pRequest->setContext(CLIENT_REQUEST_DISABLE_AUTO_DELETE,"1");
		_pHandler = pHandler;
		assert(_pHandler);
		_pHandler->_pRequest->addRef();
	}

	~PlayResponseAsync( )
	{
		//should delete pHandler here
		_pHandler->_pRequest->release( );
// 		_pHandler->_pRequest->setContext(CLIENT_REQUEST_DISABLE_AUTO_DELETE,"0");
// 		//post to next phase
// 		IStreamSmithSite* pSite = _pHandler->_pRequest->getSite();
// 		assert( pSite != NULL );
// 		pSite->postRequest(_pHandler->_pRequest,IClientRequest::FixupResponse);
// 		{
// 			delete _pHandler;
// 			_pHandler = NULL;
// 		}		
	}

public:
	virtual void AsyncResponse(const ::TianShanIce::Streamer::StreamInfo& infoRet)
	{
		char localBuffer[1024];
		HANDLERLOGEX(ZQ::common::Log::L_INFO, HANDLERLOGFMTEX(PlayResponseAsync, "performed playEx/playItem on stream: [%s] successfully: [%s]"),
			_pHandler->_context.streamFullID.c_str(),
			ZQTianShan::Util::dumpStreamInfo( infoRet, localBuffer, sizeof(localBuffer)-1  ) );

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

		IServerResponse* pResponse = _pHandler->getResponse();

		pResponse->setHeader(NGOD_HEADER_SCALE, strCurrentSpeed.c_str() );
		pResponse->setHeader(NGOD_HEADER_RANGE, strTimeOffset.c_str() );

		_pHandler->responseOK();
		


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
		_pHandler->addPlayEvent(strCurrentSpeed, strCurTime, strCtrlNum);

		
		Ice::Long cur = ZQTianShan::now();
		Ice::Long timeUsed = cur - _pHandler->getStartTime();
		_pHandler->_ssmNGODr2c1._fileLog(ZQ::common::Log::L_INFO, CLOGFMT(PlayResponseAsync, "Sess(%s)Seq(%s)[success]process[%s]request, used [%lld]ms"),
			_pHandler->getSession().c_str(),
			_pHandler->getSequence().c_str(),
			_pHandler->getRequestType().c_str(), 
			timeUsed);

	}
	virtual void ASyncException(const ::Ice::Exception& ex)
	{	//RESPONSE_INVALID_STATE
		try
		{
			ex.ice_throw();
		}
		catch( const Ice::TimeoutException& )
		{
			HANDLERLOGEX(ZQ::common::Log::L_WARNING, HANDLERLOGFMTEX(PlayResponseAsync, "caught exception[Ice::TimeoutException] during playEx/playItem") );

			if( _sopConfig._sopRestrict._penaltyEnableMask & PENALTY_ENABLE_MASK_PLAY )
				_pHandler->_ssmNGODr2c1.addPenaltyToStreamer( _pHandler->_context.sopname , _pHandler->_context.streamNetId );

			_pHandler->responseError(RESPONSE_INTERNAL_ERROR);
		}
		catch( const Ice::ConnectionRefusedException& )
		{
			HANDLERLOGEX(ZQ::common::Log::L_WARNING, HANDLERLOGFMTEX(PlayResponseAsync, "caught exception[Ice::ConnectionRefusedException] during playEx/playItem"));
			
			if( _sopConfig._sopRestrict._penaltyEnableMask & PENALTY_ENABLE_MASK_PLAY )
				_pHandler->_ssmNGODr2c1.addPenaltyToStreamer( _pHandler->_context.sopname , _pHandler->_context.streamNetId );

			_pHandler->responseError(RESPONSE_INTERNAL_ERROR);
		}
		catch( const TianShanIce::InvalidStateOfArt& bEx )
		{
			HANDLERLOGEX(ZQ::common::Log::L_WARNING, HANDLERLOGFMTEX(PlayResponseAsync, "caught exception during playEx/playItem: [%s]:%d %s"), 
				bEx.ice_name().c_str(), bEx.errorCode, bEx.message.c_str());

			_pHandler->responseError(REPSONSE_REQUEST_NOT_ACCEPTABLE);
		}
		catch( const TianShanIce::InvalidParameter& bEx )
		{		
			HANDLERLOGEX(ZQ::common::Log::L_WARNING, HANDLERLOGFMTEX(PlayResponseAsync, "caught exception during playEx/playItem: [%s]:%d %s"), 
				bEx.ice_name().c_str(), bEx.errorCode, bEx.message.c_str());

			switch (bEx.errorCode)
			{
			case EXT_ERRCODE_INVALID_RANGE:
				{
					_pHandler->responseError(RESPONSE_INVALID_RANGE);
				}
				break;
			case EXT_ERRCODE_BANDWIDTH_EXCEEDED:
				{
					_pHandler->responseError( RESPONSE_NOT_ENOUGH_BW );
				}
				break;
			default:
				{
					_pHandler->responseError(RESPONSE_INVALID_PARAMETER);
				}
				break;
			}		
		}
		catch( const TianShanIce::ServerError& bEx)
		{
			HANDLERLOGEX(ZQ::common::Log::L_WARNING, HANDLERLOGFMTEX(PlayResponseAsync, "caught exception during playEx/playItem: [%s]:%d %s"), 
				bEx.ice_name().c_str(), bEx.errorCode, bEx.message.c_str());

			switch (bEx.errorCode)
			{
			case EXT_ERRCODE_INVALID_RANGE:
				{
					_pHandler->responseError(RESPONSE_INVALID_RANGE);
				}
				break;
			case EXT_ERRCODE_BANDWIDTH_EXCEEDED:
				{
					_pHandler->responseError( RESPONSE_NOT_ENOUGH_BW );
				}
				break;
			default:
				{
					_pHandler->responseError(RESPONSE_INTERNAL_ERROR);
				}
				break;

			}		
		}
		catch( const Ice::Exception& ex)
		{
			HANDLERLOGEX(ZQ::common::Log::L_WARNING, HANDLERLOGFMTEX(PlayResponseAsync, "caught exception during playEx/playItem(%s): [%s]"),
				ex.ice_name().c_str() , _pHandler->_context.streamFullID.c_str() );	

			_pHandler->responseError(RESPONSE_INTERNAL_ERROR);
		}

		Ice::Long cur = ZQTianShan::now();
		Ice::Long timeUsed = cur - _pHandler->getStartTime();
		//CLOGFMT(NGODEnv, "Sess(%s)Seq(%s)[failed]process[%s]request, used [%lld]ms"), 
		_pHandler->_ssmNGODr2c1._fileLog(ZQ::common::Log::L_INFO, CLOGFMT(PlayResponseAsync, "Sess(%s)Seq(%s)[failed]process[%s]request, used [%lld]ms"), 
			_pHandler->getSession().c_str(),
			_pHandler->getSequence().c_str(),
			_pHandler->getRequestType().c_str(), 
			timeUsed);
	}

private:
	PlayHandler::Ptr	_pHandler;
};

class PlayExAsync : public PlayResponseAsync , public TianShanIce::Streamer::AMI_Stream_playEx
{
public:
	PlayExAsync( PlayHandler::Ptr pHandler  )
		:PlayResponseAsync(pHandler)
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
class playItemAsync : public PlayResponseAsync , public TianShanIce::Streamer::AMI_Playlist_playItem
{
public:
	playItemAsync( PlayHandler::Ptr pHandler  )
		:PlayResponseAsync(pHandler)
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

PlayHandler::PlayHandler(NGODEnv& ssm, IStreamSmithSite* pSite, IClientRequestWriter* pReq) : RequestHandler(ssm, pSite, pReq)
{
	_method = "PLAY";
	_inoutMap[MAP_KEY_METHOD] = _method;
#ifdef _DEBUG
	cout<<"construct PLAY handler"<<endl;
#endif

}

PlayHandler::~PlayHandler()
{
#ifdef _DEBUG
	cout<<"deconstruct PLAY handler"<<endl;
#endif

}

void PlayHandler::addPlayEvent(const std::string strSpeed, const std::string strNPT, const std::string streamResourceID)
{
	// add by zjm to support session history
	if (_ngodConfig._sessionHistory.enableHistory > 0 && _ngodConfig._sessionHistory.enablePlayEvent > 0)
	{
		NGODr2c1::SessionEventRecord sessionEvent;
		sessionEvent.eventTime = NgodUtilsClass::generatorISOTime();
		sessionEvent.streamResourceID = streamResourceID;
		sessionEvent.NPT = strNPT;
		sessionEvent.prop["scale"] = strSpeed;
		sessionEvent.prop["newState"] = "PLAY";
		if (_context.prop["firstPlay"] == "true")
		{
			updateContextProp("firstPlay", "false");
			sessionEvent.eventType = NGODr2c1::StartStreamEvent;
			_pContextPrx->addEventRecord(sessionEvent);
		}
		else
		{
			sessionEvent.eventType = NGODr2c1::UserEvent;
			_pContextPrx->addEventRecord(sessionEvent);
		}
	}

}

RequestProcessResult PlayHandler::process()
{
	HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(PlayHandler, "start processing"));

	if (false == _canProcess)
	{
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PlayHandler, "failed to process the request because [%s]"), szBuf);
		HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PlayHandler, "failed to process the request because [%s]"), szBuf);
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
		responseError(RESPONSE_INTERNAL_ERROR);
		return RequestError;
	}

	if (false == renewSession())
	{
		responseError(RESPONSE_INTERNAL_ERROR);
		return RequestError;
	}

	if( !checkStreamer() )
	{
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PlayHandler, "streamer is unavailable [%s:%s]"), 
			_context.sopname.c_str() , _context.streamNetId.c_str() );
		responseError(RESPONSE_SERVICE_UNAVAILABLE);
		return RequestError;
	}

	// DO: get stream proxy
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

	if (_ngodConfig._MessageFmt.rtspNptUsage <= 0)
	{
		char versionCode[8];
		sprintf(versionCode, "%d", _requireProtocol);
		if (false == updateContextProp("RequireC1", versionCode))
		{
			responseError(RESPONSE_INTERNAL_ERROR);
			return RequestError;
		}
		HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(PlayHandler, "updated C1 Reqire header in context[RequireC1]=%s"), versionCode);
	}
	
	std::string startPoint = getRequestHeader(NGOD_START_POINT);
	_reqRange = getRequestHeader(NGOD_HEADER_RANGE);
	Ice::Long	milliSecs = 0;
	Ice::Short	from = 0;
	Ice::Float	f_speed = 0.0f;
	Ice::Int	assetIndex = -1;
#pragma message(__MSGLOC__"TODO: use the start point recorded from SETUP stage!!!!!!!!!!!!!!!!!!!!!!!!!!!")	
	ZQTianShan::Util::getPropertyDataWithDefault( _context.prop , REQUEST_STARTPOINT_IDX , -1 , assetIndex );
	ZQTianShan::Util::getPropertyDataWithDefault( _context.prop , REQUEST_STARTPOINT_OFFSET , 0 , milliSecs );
// 	if( !startPoint.empty() )
// 	{//use start point if not empty
// 		//I should record the playlist items information so I can calculate the real relative NPT which is used by streaming server		
// 		double nptTime = 0.0f;
// 		sscanf(startPoint.c_str(),"%d %f",&assetIndex, &nptTime);
// 		if( assetIndex >= 0 && assetIndex <= static_cast<Ice::Int>( _context.setupInfos.size() - 1 ) )
// 		{
// 			milliSecs = static_cast<Ice::Long>(nptTime * 1000) - _context.setupInfos[assetIndex].inTimeOffset;
// 			if( milliSecs < 0 )
// 				milliSecs = 0;
// 			HANDLERLOG(ZQ::common::Log::L_INFO,
// 				HANDLERLOGFMT(PlayHandler,"get asset index [%d] and npt[%lld] and inTimeOffset[%lld]"),
// 				assetIndex , milliSecs , _context.setupInfos[assetIndex].inTimeOffset );
// 		}
// 		else
// 		{
// 			HANDLERLOG(ZQ::common::Log::L_WARNING,
// 				HANDLERLOGFMT(PlayHandler,"get start-point header, but with invalid asset index [%d] and total asset[%d]"),
// 				assetIndex, _context.setupInfos.size() );
// 			assetIndex = -1; //reset asset index to -1
// 		}
// 	}
// 	else 
	
	if (false == _reqRange.empty())
	{		
		if (stricmp(ZQ::StringOperation::nLeftStr(_reqRange, 4).c_str(), "npt=") != 0)
		{
			snprintf(szBuf, MY_BUFFER_SIZE - 1, "Illegal Range: [%s] specified, missed [npt=]", _reqRange.c_str());
			HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PlayHandler, "%s"), szBuf);
            HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PlayHandler, "%s"), szBuf);
			responseError(RESPONSE_BAD_REQUEST);
			return RequestError;
		}
		
		std::string right_str;
		right_str = ZQ::StringOperation::getRightStr(_reqRange, "=", true);
		
		std::string second_str;
		int pos_tmp;
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
	
	std::string		strTimeOffset;
	std::string		strCurrentSpeed;

	_reqScale = getRequestHeader(NGOD_HEADER_SCALE);
	if ( false == _reqScale.empty() )
	{	
		f_speed = (float) atof( _reqScale.c_str() );
	}
	
	try
	{
		TianShanIce::StrValues expectValues;

		// add by zjm to support item npt 
		expectValues.push_back("ITEM_CURRENTPOS");
		expectValues.push_back("CURRENTPOS");
#pragma message(__MSGLOC__"TODO:Should I remove TOTALPOS from expect values ?")
		expectValues.push_back("TOTALPOS");
		expectValues.push_back("SPEED");
		expectValues.push_back("STATE");
		expectValues.push_back("USERCTRLNUM");

		setReturnType(RETURN_ASYNC);

		if( !_reqRange.empty() )
		{//if request include range , just play with range
			HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(PlayHandler, "calling playEx() with scale[%f] npt[%lld] from[%d] on stream[%s]"), 
				f_speed,
				milliSecs,
				from,
				_context.streamFullID.c_str());
			mStreamPrx->playEx_async( (new PlayExAsync(this)) , f_speed ,milliSecs , from ,expectValues );
		}
		else if( assetIndex >= 0 )
		{//has start point , play item			
			bool bNeedSeek =false;
			TianShanIce::Streamer::StreamState state = TianShanIce::Streamer::stsSetup;
			std::string stateStr;
			if( !getStreamState(state, stateStr) )
			{
				bNeedSeek = true;
			}
			else
			{
				bNeedSeek = ! (( state == TianShanIce::Streamer::stsStreaming ) || ( state == TianShanIce::Streamer::stsPause ));
			}

			if( !bNeedSeek )//if stream is running, just only change scale
			{
				from		= 0 ;
				milliSecs	= 0;

				HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(PlayHandler, "calling playEx() with scale[%f] npt[%lld] from[%d] on stream[%s]"), 
					f_speed,
					milliSecs,
					from,
					_context.streamFullID.c_str());
				mStreamPrx->playEx_async( (new PlayExAsync(this)) , f_speed ,milliSecs , from ,expectValues );
			}
			else
			{
				from = 1 ;//play from beginning

				HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(PlayHandler, "calling playItem() with ctrlnum[%d] scale[%f] npt[%lld] from[%d] on stream[%s]"), 
					assetIndex + 1,
					f_speed,
					milliSecs,
					from,
					_context.streamFullID.c_str());
				mStreamPrx->playItem_async( (new playItemAsync(this)) , assetIndex + 1,  milliSecs , from , f_speed, expectValues );
			}
		}
		else
		{//nothing is specified , play stream
			HANDLERLOG(ZQ::common::Log::L_DEBUG, HANDLERLOGFMT(PlayHandler, "calling playEx() with scale[%f] npt[%lld] from[%d] on stream[%s]"), 
				f_speed,
				milliSecs,
				from,
				_context.streamFullID.c_str());
			mStreamPrx->playEx_async( (new PlayExAsync(this)) , f_speed ,milliSecs , from ,expectValues );
		}
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		snprintf(szBuf, MY_BUFFER_SIZE - 1,"playEx() on stream[%s] caught exception[%s] %s", _context.streamFullID.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PlayHandler, "%s"), szBuf);
		HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PlayHandler, "%s"), szBuf);
		responseError(RESPONSE_INTERNAL_ERROR);
		return RequestError;
	}
	catch(const ::Ice::Exception& ex)
	{
		snprintf(szBuf, MY_BUFFER_SIZE - 1,"playEx() on stream[%s] caught exception[%s]", _context.streamFullID.c_str(), ex.ice_name().c_str());
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PlayHandler, "%s"), szBuf);
		HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PlayHandler, "%s"), szBuf);
		responseError(RESPONSE_INTERNAL_ERROR);
		return RequestError;
	}

	return RequestDone;
}


