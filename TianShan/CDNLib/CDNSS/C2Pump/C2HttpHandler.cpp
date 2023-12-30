
#include <ZQ_common_conf.h>
#include <assert.h>
#include <sstream>
#include <strHelper.h>
#include <urlstr.h>
#include "C2HttpHandler.h"
#include "C2StreamerEnv.h"
#include "C2StreamerService.h"
#include "C2SessionManager.h"
#include "C2Session.h"


#if defined ZQ_OS_MSWIN
	#define	SESSFMT(x,y) 	CLOGFMT(x, "REQUEST[%s]\t"##y), request->requestHint.c_str() 
#elif defined ZQ_OS_LINUX
	#define	SESSFMT(x,y) 	CLOGFMT(x, "REQUEST[%s]\t"y) , request->requestHint.c_str() 
#endif
#include <boost/concept_check.hpp>


namespace C2Streamer
{
bool getValueEx( const SimpleXMLParser::Node* pParent , const std::string& nodeName , std::string& value)
{
	if( !pParent || nodeName.empty() )	return false;
	const SimpleXMLParser::Node* pNode = findNode( pParent , nodeName );
	if( !pNode )
	{
		return false;
	}
	value = pNode->content;
	return !value.empty();
}

bool getValue( const SimpleXMLParser::Node* pParent , const std::string& nodeName , std::vector<std::string>& values )
{
	SiblingNode nodes = childNodes( pParent , nodeName );
	for( const SimpleXMLParser::Node* i = nodes.first() ; i != 0 ; i = nodes.next() )
	{
		values.push_back( i->content );
	}
	return nodes.count() > 0 ;
}

void parseRangeValue( const std::string& r , TransferRange& range )
{
	std::string::size_type pos = r.find('-');
	if( pos != std::string::npos )
	{
		std::string::size_type pos2 = r.find("-",pos+1);
		if( pos2 != std::string::npos)
			pos = pos2;

		std::string start = r.substr( 0 , pos );
		std::string stop = r.substr( pos + 1 );
		ZQ::common::stringHelper::TrimExtra( start, " \t\v\r\n");
		ZQ::common::stringHelper::TrimExtra( stop , " \t\v\r\n-");
		int64 t = 0;
		if( !start.empty() )
		{
			sscanf(start.c_str(),FMT64,&t);
			range.bStartValid = true;
			range.startPos = t;
		}
		if( !stop.empty() )
		{
			sscanf(stop.c_str() , FMT64 , &t );
			range.bEndValid = true;
			range.endPos = t;
		}
	}
}

bool getValue( const SimpleXMLParser::Node* pParent , const std::string& nodeName , TransferRange& value )
{
	std::string result;
	if(!getValueEx(pParent , nodeName, result))	return false;
	//result = ZQ::common::stringHelper::TrimExtra(result);
	//parse range, finding dash
	parseRangeValue(result,value);
	return true;
}

bool getValue( const SimpleXMLParser::Node* pParent , const std::string& nodeName , bool& value )
{
	std::string result;
	if(!getValueEx(pParent , nodeName, result))	return false;

	int i = 0;
	sscanf( result.c_str() , "%d", &i );
	value = ( i != 0 );
	return true;
}

bool getValue( const SimpleXMLParser::Node* pParent , const std::string& nodeName , std::string& value)
{
	return getValueEx(pParent , nodeName , value );
}
bool getValue( const SimpleXMLParser::Node* pParent , const std::string& nodeName , int32& value)
{
	std::string result;
	if(!getValue(pParent , nodeName, result))	return false;
	sscanf(result.c_str(),"%d",&value);
	return true;

}
bool getValue( const SimpleXMLParser::Node* pParent , const std::string& nodeName , int64& value)
{
	std::string result;
	if(!getValueEx(pParent , nodeName, result))	return false;
	sscanf(result.c_str(), FMT64, &value);
	return true;
}
bool getValue( const SimpleXMLParser::Node* pParent , const std::string& nodeName , float& value)
{
	std::string result;
	if(!getValueEx(pParent , nodeName, result))	return false;
	sscanf(result.c_str(),"%f",&value);
	return true;
}

#define MandatoryGetValue( pnode , key , value ) if(!getValue(pnode,key,value)  ) \
{\
	mResponseParam->setLastErr(mRequestParam,errorCodeBadRequest,"[%s] is not found in xml content body",key);\
	return false;\
}
#define NonMandatoryGetValue( pnode , key , value ) getValue(pnode,key,value);



C2HttpHandler::C2HttpHandler( C2StreamerEnv& env , C2Service& svc)
:mEnv(env),
mSvc(svc),
mRequest(0),
mResponse(0),
mbBreaked(false),
mResponseParam(0),
mRequestParam(0),
mConn(0)
{
}

C2HttpHandler::~C2HttpHandler(void)
{
	if( mRequestParam )
	{
		mRequestParam = 0;
	}
	if( mResponseParam )
	{
		mResponseParam = 0;
	}
}

bool C2HttpHandler::onConnected( ZQHttp::IConnection& conn)
{
	mConn = &conn;
	return true;
}

bool C2HttpHandler::onRequest(	const ZQHttp::IRequest& req, ZQHttp::IResponse& resp)
{
	mRequest	= &req;
	mResponse	= new C2HttpResponseHanlder(&resp,mConn, mRequest->getLoop());
	return true;
}

bool C2HttpHandler::onPostData( const ZQHttp::PostDataFrag& frag)
{
	mXmlContent.append( frag.data , frag.len );
	return true;
}

bool C2HttpHandler::onPostDataEnd()
{
	return true;
}

void C2HttpHandler::onRequestEnd()
{
	//we got a complete request, let's start to process it
	assert( mResponse != 0 );
	assert( mRequest != 0 );
	if( mResponse && mConn )
		mResponse->setConnectionId( mConn->getId() );

	if ( errorWorkingInProcess != process() ) {
		postResponse();
	}
}

bool C2HttpHandler::postResponse()
{
	prepareResponseMsg();
	return true;
}

bool C2HttpHandler::parseRequest( )
{
	assert( mResponseParam != 0 );
	assert( mRequestParam != 0 );

	//check HTTP version, we only take HTTP/1.1
	//TODO: check HTTP version, reject if 1.0
	mRequestParam->evtLoop = mRequest->getLoop();
	mResponseParam->httpHandler = this;

	//create new request hint
	{
		std::ostringstream oss;
		oss<<"Sess["<<mRequestParam->sessionId<<"] Seq["<< mEnv.getRequestSequence()<<"]";
		mRequestParam->requestHint = oss.str();
	}

	if( mXmlContent.empty() )
		return true;

	//parse XML content
	try
	{
		mXmlParser.parse(mXmlContent.c_str() , static_cast<int>(mXmlContent.length()) , 1);
	}
	catch( const ZQ::common::ExpatException& ex)
	{
		mResponseParam->setLastErr( mRequestParam , errorCodeBadRequest , "failed to decode XML content body due to [%s]", ex.what() );
		return false;
	}
	return true;
}

int C2HttpHandler::process()
{
	assert( mResponseParam != 0 );
	assert( mResponse != 0 );
	mResponseParam->responseHandler = mResponse;

	bool bSessionOK = false;
	C2SessionPtr c2sess = 0;
	if( !c2sess && !mRequestParam->sessionId.empty() )
	{
		c2sess = mSvc.getSessManager().findSession( mRequestParam->sessionId);
		if( !c2sess )
		{
			mResponseParam->setLastErr( mRequestParam , errorCodeSessionGone , "failed to find session [%s]", mRequestParam->sessionId.c_str() );
		}
		else
		{
			bSessionOK = true;
		}
	}
	else
	{
		bSessionOK = true;
	}

	if( bSessionOK )
	{
		c2sess->processRequest( mRequestParam,mResponseParam );
		if( !isSuccessCode(  mResponseParam->errorCode ) )
		{
			processFailed();
		}
		//TODO: do not post response here, let it to be done in caller code
	}
	return mResponseParam->errorCode;
}

void C2HttpHandler::prepareResponseMsg( )
{
}
void C2HttpHandler::processFailed( )
{
}

void C2HttpHandler::postResponseFaied()
{
}

void C2HttpHandler::onBreak()
{
	mbBreaked = true;
	RequestResponseParamPtr resp = mResponseParam;
	if( resp && resp->responseHandler)
	{
		resp->responseHandler->setConnectionBroken( true );
	}
}

bool C2HttpHandler::isConnectionBroken() const
{
	return mbBreaked;
}

void C2HttpHandler::setStandardHeader( const RequestResponseParamPtr& responsePara , bool bContentBodyXml)
{
	assert( mResponse != 0 );
	mResponse->updateStartline( responsePara->errorCode , convertErrCodeToString(responsePara->errorCode) );
	mResponse->updateHeader( "Connection" , "close");
	mResponse->updateHeader( "Server" , mEnv.getConfig().serverNameTag );
	if(bContentBodyXml)
		mResponse->updateHeader( "Content-type","text/xml;charset=utf-8");
}


//////////////////////////////////////////////////////////////////////////
///HandlerTransferInit
HandlerTransferInit::HandlerTransferInit( C2StreamerEnv& env , C2Service& svc)
:C2HttpHandler(env,svc)
{
}
HandlerTransferInit::~HandlerTransferInit()
{
}

int HandlerTransferInit::process( )
{
	assert( mRequestParam == 0 );
	assert( mResponseParam == 0 );
	mRequestParam	= new TransferInitRequestParam(mEnv, URLRULE_C2INIT);
	mResponseParam	= new TransferInitResponseParam();
	assert( mRequestParam != 0 );
	assert( mResponseParam != 0 );

	C2SessionPtr c2session;
	assert(c2session == 0 );
	c2session = mSvc.getSessManager().createSession(mRequestParam->sessionId);
	if( !c2session )
	{
		mResponseParam->setLastErr( mRequestParam , errorCodeInternalError , "failed to create a new session" );
		return errorCodeInternalError;
	}
	mRequestParam->method = METHOD_TRANSFER_INIT;
	mRequestParam->sessionId = c2session->getSessId();

	if( !parseRequest() )
		return errorCodeBadRequest;

	return C2HttpHandler::process();
}

void HandlerTransferInit::processFailed( )
{
	mSvc.getSessManager().destroySession( mRequestParam->sessionId );
}

void HandlerTransferInit::postResponseFaied()
{
	mSvc.getSessManager().destroySession( mRequestParam->sessionId );
}
void HandlerTransferInit::composeBody( std::ostringstream& oss) const
{
	TransferInitResponseParamPtr responsePara = TransferInitResponseParamPtr::dynamicCast(mResponseParam);
	assert( responsePara != 0 );

	if( isSuccessCode(responsePara->errorCode ) )
	{
		oss	<<"<TransferInitiateResponse>\n"
			<<"	<TransferID>"<< responsePara->transferId <<"</TransferID> \n"
			<<"	<AvailableRange>"<< (responsePara->availRange.toString())<<"</AvailableRange>\n"
			<<"	<OpenForWrite>"<< std::string(responsePara->openForWrite ? "yes":"no") <<"</OpenForWrite>\n"
			<<"</TransferInitiateResponse>";
	}
	else
	{
		oss	<<"<TransferInitiateResponse>\n"
			<<"	<ErrorText>" << responsePara->errorText << "</ErrorText>\n"
			<<"</TransferInitiateResponse>";
	}
}
void HandlerTransferInit::prepareResponseMsg( )
{
	//prepare the message
	TransferInitResponseParamPtr responsePara = TransferInitResponseParamPtr::dynamicCast(mResponseParam);
	assert( responsePara != 0 );

	setStandardHeader( responsePara );

	//prepare content body
	std::ostringstream oss;
	composeBody( oss );
	std::string strBody = oss.str();
	mResponse->flushHeader( strBody.length() );
	mResponse->addBodyContent( strBody.c_str() , strBody.length() );

	if( !mResponse->complete() )
	{
		postResponseFaied();
	}
}

bool HandlerTransferInit::parseRequest()
{
	if( !C2HttpHandler::parseRequest() )
		return false;

	const SimpleXMLParser::Node& root = mXmlParser.document();
	const SimpleXMLParser::Node* pTransferInit = findNode( &root, "TransferInitiate");
	if( !pTransferInit )
	{
		mResponseParam->setLastErr( mRequestParam , errorCodeBadRequest ,"Node[%s] is not found" , "TransferInitiate" );
		return false;
	}

	TransferInitRequestParamPtr request = TransferInitRequestParamPtr::dynamicCast( mRequestParam );
	assert( request != 0 );

	request->method		= METHOD_TRANSFER_INIT;

	MandatoryGetValue( pTransferInit , "ClientTransfer" , request->clientTransfer );
	MandatoryGetValue( pTransferInit , "TransferAddress" , request->transferAddress );
	MandatoryGetValue( pTransferInit , "IngressCapacity" , request->ingressCapacity );
	MandatoryGetValue( pTransferInit , "Filename", request->fileName );
	MandatoryGetValue( pTransferInit , "TransferRate", request->transferRate );
	MandatoryGetValue( pTransferInit , "TransferTimeout", request->transferTimeout );

	NonMandatoryGetValue( pTransferInit , "AllocatedTransferRate" , request->allocatedTransferRate);

	NonMandatoryGetValue( pTransferInit , "ExtraIngressCapacity", request->extraIngressCapcity );
	NonMandatoryGetValue( pTransferInit , "Range" , request->requestRange );
	NonMandatoryGetValue( pTransferInit , "TransferDelay" , request->transferDelay );

	////////////////////////////////////////////////
	/// TODO: validate the request paramter here
	/// TODO: I do not understand parameter of AllocatedTransferRate , AllocatedIngressCapacity
	////////////////////////////////////////////////

	MLOG(ZQ::common::Log::L_INFO,SESSFMT(HandlerTransferInit, "new request: client[%s] server[%s] ic["FMT64"] eic["FMT64"] file[%s] range[%s] delay[%d] timeout[%d]"),
		request->clientTransfer.c_str(), request->transferAddress.c_str() , request->ingressCapacity , request->extraIngressCapcity , request->fileName.c_str() ,
		request->requestRange.toString().c_str(),request->transferDelay , request->transferTimeout	);

	return true;
}

HanlderTransferTerminate::HanlderTransferTerminate( C2StreamerEnv& env , C2Service& svc)
:C2HttpHandler(env,svc)
{
}

HanlderTransferTerminate::~HanlderTransferTerminate( )
{
}

int HanlderTransferTerminate::process()
{
	assert( mRequestParam == 0 );
	assert( mResponseParam == 0 );
	mRequestParam	= new TransferTermRequestParam( mEnv, URLRULE_C2TERM);
	mResponseParam	= new TransferTermResponseParam();
	assert( mRequestParam != 0 );
	assert( mResponseParam != 0 );

	if(!parseRequest())
		return errorCodeBadRequest;

	mSvc.getSessManager().destroySession( getSessionIdFromCompoundString(mRequestParam->sessionId) );

	return errorCodeOK;
}

bool HanlderTransferTerminate::parseRequest()
{


	const SimpleXMLParser::Node& root = mXmlParser.document();
	const SimpleXMLParser::Node* pTransfeTerm = findNode( &root, "TransferTerminate");
	if( !pTransfeTerm )
	{
		mResponseParam->setLastErr( mRequestParam , errorCodeBadRequest ,"Node[%s] is not found" , "TransferTerminate" );
		return false;
	}

	TransferTermRequestParamPtr request = TransferTermRequestParamPtr::dynamicCast( mRequestParam );
	assert( request != 0 );

	request->method	= METHOD_TRANSFER_TERM;

	MandatoryGetValue( pTransfeTerm , "ClientTransfer" ,  request->clientTransfer );
	MandatoryGetValue( pTransfeTerm , "TransferID", request->sessionId );

	if(!C2HttpHandler::parseRequest())
		return false;

	MLOG(ZQ::common::Log::L_INFO,SESSFMT(HanlderTransferTerminate,"new request: client[%s] sessionId[%s]"),
		request->clientTransfer.c_str() , request->sessionId.c_str() );

	return true;
}

void HanlderTransferTerminate::composeBody( std::ostringstream& oss) const
{
	TransferTermResponseParamPtr responsePara = TransferTermResponseParamPtr::dynamicCast( mResponseParam );
	assert( responsePara != 0 );
	if( isSuccessCode( responsePara->errorCode ) )
	{
		oss <<"<TransferTerminateResponse>\n"
			<<"</TransferTerminateResponse>";
	}
	else
	{
		oss <<"<TransferTerminateResponse>\n"
			<<"	<ErrorText>"<< responsePara->errorText <<"</ErrorText>\n"
			<<"</TransferTerminateResponse>";
	}
}

void HanlderTransferTerminate::prepareResponseMsg( )
{
	TransferTermResponseParamPtr responsePara = TransferTermResponseParamPtr::dynamicCast( mResponseParam );
	assert( responsePara != 0 );

	//update header
	setStandardHeader( responsePara );

	std::ostringstream oss;
	composeBody( oss );
	std::string strBody = oss.str();
	mResponse->flushHeader( strBody.length() );
	mResponse->addBodyContent( strBody.c_str() , strBody.length() );

	mResponse->complete();//what ever success or fail
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//HanlderSessionStatus
HanlderSessionStatus::HanlderSessionStatus( C2StreamerEnv& env , C2Service& svc)
:C2HttpHandler(env,svc)
{
}

HanlderSessionStatus::~HanlderSessionStatus()
{
}

int HanlderSessionStatus::process()
{
	assert( mRequestParam == 0 );
	assert( mResponseParam == 0 );
	mRequestParam	= new SessionStatusRequestParam(mEnv, URLRULE_C2STATUS);
	mResponseParam	= new SessionStatusResponseParam();
	assert( mRequestParam != 0 );
	assert( mResponseParam != 0 );

	if( !parseRequest() )
		return errorCodeBadRequest;

	SessionStatusRequestParamPtr request = SessionStatusRequestParamPtr::dynamicCast(mRequestParam);
	SessionStatusResponseParamPtr response = SessionStatusResponseParamPtr::dynamicCast(mResponseParam);
	return mSvc.getSessionStatus( request , response ) ? errorCodeOK : errorCodeInternalError;
}

bool HanlderSessionStatus::parseRequest()
{
	if( !C2HttpHandler::parseRequest() )
		return false;

	SessionStatusRequestParamPtr request = SessionStatusRequestParamPtr::dynamicCast( mRequestParam );
	assert( request != 0 );

	request->method	= METHOD_SESSION_STATUS;

	const SimpleXMLParser::Node& root = mXmlParser.document();
	const SimpleXMLParser::Node* pSessionStatus = findNode( &root, "Status");
	if( !pSessionStatus )
	{
		mResponseParam->setLastErr( mRequestParam , errorCodeBadRequest ,"Node[%s] is not found" , "Status" );
		return false;
	}

	request->includeAggregate = true;
	NonMandatoryGetValue( pSessionStatus , "IncludeAggregate" ,  request->includeAggregate );
	MandatoryGetValue( pSessionStatus , "Client", request->clientTransfers );

	MLOG(ZQ::common::Log::L_INFO,SESSFMT(HanlderSessionStatus,"new request: includeAggregat[%s] client[%s] "),
		request->includeAggregate ? "true" : "false",
		dumpStringVector( request->clientTransfers).c_str() );
	return true;
}

void HanlderSessionStatus::prepareResponseMsg( )
{
	SessionStatusResponseParamPtr responsePara = SessionStatusResponseParamPtr::dynamicCast( mResponseParam );

	//update start line
	mResponse->updateStartline( responsePara->errorCode , convertErrCodeToString( responsePara->errorCode ) );

	//update header
	setStandardHeader( responsePara );

	std::ostringstream oss;
	composeBody(oss);
	std::string strBody = oss.str();

	mResponse->flushHeader( strBody.length() );
	mResponse->addBodyContent( strBody.c_str() , strBody.length() );

	mResponse->complete();

}
void HanlderSessionStatus::composeBody( std::ostringstream& oss )
{
	SessionStatusResponseParamPtr responsePara = SessionStatusResponseParamPtr::dynamicCast( mResponseParam );
	assert( responsePara != 0 );

	oss << "<StatusResponse>\n";
	if( !isSuccessCode(responsePara->errorCode) )
	{
		oss	<< "	<ErrorText>"<<responsePara->errorText<<"</ErrorText>\n";
	}
	else
	{
		SessionStatusRequestParamPtr requestPara = SessionStatusRequestParamPtr::dynamicCast( mRequestParam );
		assert( requestPara);
		if( requestPara->includeAggregate )
		{
			AggregateStatisticsParam& stat = responsePara->statistics;
			oss << "	<AggregateStatistics>\n";
			oss << "		<ActiveSessions>" << stat.activeSessions << "</ActiveSessions>\n";
			oss << "		<IdleSessions>" << stat.idleSessions << "</IdleSessions>\n";
			oss << "		<TotalSessions>" << stat.totalSessions << "</TotalSessions>\n";
			oss << "		<AllocatedBandwidth>" << stat.allocatedBandwidth << "</AllocatedBandwidth>\n";
			oss << "		<TotalBandwidth>" << stat.totalBandwidth << "</TotalBandwidth>\n";
			oss << "		<BytesTransmitted>"<< stat.bytesTransfered << "</BytesTransmitted>\n";
			oss << "		<Uptime>"<< stat.uptime <<"</Uptime>\n";;
			oss << "	</AggregateStatistics>\n";
		}
		const std::vector<SessionStatusInfo>& infos = responsePara->sessionInfos;
		std::vector<SessionStatusInfo>::const_iterator itSess = infos.begin();

		for( ; itSess != infos.end() ; itSess ++ )
		{//output session information
			oss << "	<Session>\n";
			oss << "		<TransferID>" << itSess->transferId << "</TransferID>\n";
			oss << "		<Filename>" << itSess->fileName << "</Filename>\n";
			oss << "		<ClientTransfer>" << itSess->clientTransfer << "</ClientTransfer>\n";
			oss << "		<TransferAddress>" << itSess->transferAddress << "</TransferAddress>\n";
			oss << "		<TransferPort>" << itSess->transferPortName << "</TransferPort>\n";
			oss << "		<State>" <<  convertSessionStateToString( itSess->sessionState ) << "</State>\n";
			oss << "		<TimeInState>" << itSess->timeInState << "</TimeInState>\n";
			oss << "		<TransferRate>" << itSess->transferRate << "</TransferRate>\n";
			oss << "		<BytesTransferred>" << itSess->bytesTransfered << "</BytesTransferred>\n";
			oss << "	</Session>\n";
		}
	}
	oss	<< "</StatusResponse>";
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
///HandlerResourceStatus
HandlerResourceStatus::HandlerResourceStatus( C2StreamerEnv& env , C2Service& svc)
:C2HttpHandler(env,svc)
{
}

HandlerResourceStatus::~HandlerResourceStatus()
{
}

int HandlerResourceStatus::process()
{
	assert( mRequestParam == 0 );
	assert( mResponseParam == 0 );
	mRequestParam	= new ResourceStatusRequestParam(mEnv, URLRULE_C2RESSTAT);
	mResponseParam	= new ResourceStatusResponseParam();
	assert( mRequestParam != 0 );
	assert( mResponseParam != 0 );
	if( !parseRequest() )
		return errorCodeBadRequest;

	ResourceStatusRequestParamPtr request = ResourceStatusRequestParamPtr::dynamicCast(mRequestParam);
	ResourceStatusResponseParamPtr response = ResourceStatusResponseParamPtr::dynamicCast(mResponseParam);
	return mSvc.getResourceStatus( request , response ) ? errorCodeOK: errorCodeInternalError;
}

bool HandlerResourceStatus::parseRequest()
{
	if(!C2HttpHandler::parseRequest())
		return false;

	ResourceStatusRequestParamPtr request = ResourceStatusRequestParamPtr::dynamicCast( mRequestParam );
	assert( request != 0 );

	request->method	= METHOD_RESOURCE_STATUS;

	const SimpleXMLParser::Node& root = mXmlParser.document();
	const SimpleXMLParser::Node* pStatus = findNode( &root, "ResourceStatus");
	if( !pStatus )
	{
		mResponseParam->setLastErr( mRequestParam , errorCodeBadRequest ,"Node[%s] is not found" , "ResourceStatus" );
		return false;
	}

	NonMandatoryGetValue( pStatus, "PortName" , request->portNames );

	MLOG(ZQ::common::Log::L_INFO,SESSFMT(HandlerResourceStatus,"new request: portNames[%s]"),
		dumpStringVector(request->portNames).c_str() );

	return true;
}

void HandlerResourceStatus::prepareResponseMsg( )
{
	ResourceStatusResponseParamPtr responsePara = ResourceStatusResponseParamPtr::dynamicCast( mResponseParam );
	assert( responsePara != 0 );

	mResponse->updateStartline( responsePara->errorCode , convertErrCodeToString(responsePara->errorCode) );

	setStandardHeader( responsePara );


	std::ostringstream oss;
	composeBody( oss );
	std::string strBody = oss.str();
	mResponse->flushHeader( strBody.length() );
	mResponse->addBodyContent( strBody.c_str() , strBody.length() );

	mResponse->complete();

}

void HandlerResourceStatus::composeBody( std::ostringstream& oss )
{
	ResourceStatusResponseParamPtr responsePara = ResourceStatusResponseParamPtr::dynamicCast( mResponseParam );
	assert( responsePara != 0 );

	oss << "<ResourceStatusResponse>\n";
	if( !isSuccessCode( responsePara->errorCode ) )
	{
		oss << "	<ErrorText>"<< responsePara->errorText <<"<ErrorText/>\n";
	}
	else
	{
		std::vector<ResourceStatusInfo>::const_iterator itPort = responsePara->portInfos.begin();
		for( ; itPort != responsePara->portInfos.end() ; itPort ++ )
		{
			oss << "	<Port>";
			oss << "		<Name>" << itPort->portName << "</Name>\n";
			std::vector<std::string>::const_iterator itAddr = itPort->portAddressIpv4.begin();
			for ( ; itAddr != itPort->portAddressIpv4.end() ; itAddr++ )
			{
				oss << "		<Address>" << *itAddr << "</Address>\n";
			}
			itAddr = itPort->portAddressIpv6.begin();
			for ( ; itAddr != itPort->portAddressIpv6.end() ; itAddr++ )
			{
				oss << "		<Address>" << *itAddr << "</Address>\n";
			}
			oss << "		<TCPPortNumber>" << itPort->tcpPortNumber << "</TCPPortNumber>\n";
			oss << "		<Capacity>" << itPort->capacity << "</Capacity>\n";
			oss << "		<State>" << std::string((itPort->portState == PORT_STATE_UP) ? "UP" : "DOWN") << "</State>\n";
			oss << "		<ActiveTransferCount>" << itPort->activeTransferCount << "</ActiveTransferCount>\n";
			oss << "		<ActiveBandwidth>" << itPort->activeBandwidth <<"</ActiveBandwidth>\n";
			oss << "	</Port>";
		}
	}
	oss<<"</ResourceStatusResponse>";
}

//////////////////////////////////////////////////////////////////////////
////HandlerGetFile


HandlerGetFile::HandlerGetFile(C2StreamerEnv& env , C2Service& svc , const std::string& url)
:HandlerSessionTransfer(env,svc,url),
mLastErrorCode(200),
mGetFileUrl(url)
{
}

HandlerGetFile::~HandlerGetFile()
{
}

void HandlerGetFile::fixupFilenameAndRange( std::string& filename, TransferRange& range ) {
	// filename : xxx-offsetPsize
	std::string::size_type posDash = filename.find_last_of('-'); // the delimitor of StartOffset
	std::string::size_type posPlus = filename.find_last_of('L'); // the delimitor of Length
	if( posDash == std::string::npos || posPlus == std::string::npos || posDash > posPlus )
		return;//not the filename pattern we want
	sscanf(filename.c_str()+posDash+1,"%ld",&range.startPos); // read the start offset
	sscanf(filename.c_str()+posPlus+1,"%ld",&range.endPos); // read the length, temporarily save it in endPos
	range.endPos = range.startPos + range.endPos - 1; // calculate the real endPos
	range.bEndValid = range.bStartValid = true;
	filename = filename.substr(0,posDash);
}

bool HandlerGetFile::createSession(  )
{
	TransferInitRequestParamPtr request = new TransferInitRequestParam(mEnv, URLRULE_C2TRANSFER);
	std::string clientTransfer ;
	std::string transferAddress;
	std::string fileName;
	int64 ingressCapacity = ( 2000LL * 2000 * 1000 * 1000);
	int64 transferRate = request->getConfWriter()->mDefaultTransferBitrate;
	int64 transferTimeout = 5000;
	TransferRange trange;
	int64 delay = -50;
	bool isPathASessId = false;

	if( mConn )
	{
		int port = 0;
		mConn->getRemoteEndpoint( clientTransfer , port );
		mConn->getLocalEndpoint(transferAddress, port );
	}

	std::string::size_type posFileName = mGetFileUrl.find( URLRULE_GETFILE );
	if( posFileName != std::string::npos ) {
		static size_t prefixLen = strlen( URLRULE_GETFILE );
		if( posFileName + prefixLen + 1 < mGetFileUrl.length() ) {
			fileName = mGetFileUrl.substr( posFileName + prefixLen + 1);
		}
	}

	std::map<std::string, std::string> vars = mRequest->queryArguments();

	if(vars["file"].length() > 0 )
		fileName = vars["file"];
	if(vars["ic"].length() > 0 )
		ingressCapacity = atoll(vars["ic"].c_str());
	if(vars["rate"].length() > 0 )
		transferRate = atoll(vars["rate"].c_str());
	if(vars["timeout"].length() > 0 )
		transferTimeout = atoi(vars["timeout"].c_str());
	if(vars["delay"].length() > 0 )
		delay = atoi(vars["delay"].c_str());
	if(vars["range"].length() > 0 )
		parseRangeValue(vars["range"],trange);
	if(vars["sid"].length() > 0)
		isPathASessId = (vars["sid"] == "true");


	fixupFilenameAndRange(fileName, trange);

	if( fileName.empty() || transferAddress.empty() )
	{
		mSessionInitErrorCode = errorCodeBadRequest;
		return false;
	}
	C2SessionManager& sessManager = getC2StreamerService()->getSessManager();

	std::string oldFileName = fileName;
	if(isPathASessId) {
		fileName = sessManager.getPathFromSess(fileName);
	}

	if(fileName.empty()) {
		MLOG.error(CLOGFMT(HandlerGetFile, "failed to find file path name according to sess [%s]"), oldFileName.c_str());
		return false;
	}

	C2SessionPtr sess =  sessManager.createSession("");
	if( !sess )	{
		mSessionInitErrorCode = errorCodeInternalError;
		return false;
	}

	mSessionId					= sess->getSessId();
	//int64 sesStart = ZQ::common::TimeUtil::now();
	//mEnv.setLatencyMap(fileName, trange.startPos, sesStart);
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(HandlerGetFile,"init session[%s]: client[%s] transferServer[%s] file[%s] ic[%ld] rate[%ld] timeout[%ld] range[%s]"),
		 mSessionId.c_str() ,
		 clientTransfer.c_str(), transferAddress.c_str() , fileName.c_str(),
		 ingressCapacity , transferRate , transferTimeout ,
		 trange.toString().c_str());

	// this is a special one, new with init but use transfer setting
	request->method				= C2Streamer::METHOD_TRANSFER_INIT;
	request->sessionId			= mSessionId;
	request->clientTransfer		= clientTransfer;
	request->transferAddress	= transferAddress;
	request->ingressCapacity	= ingressCapacity;
	request->fileName			= fileName;
	request->transferRate		= transferRate;
	request->transferTimeout	= transferTimeout;
	request->requestRange		= trange;
	request->transferDelay		= delay;

	C2Streamer::TransferInitResponseParamPtr response = new C2Streamer::TransferInitResponseParam();
	mSessionInitErrorCode = sess->processRequest( request, response);
	mResponse->updateHeader("X-SessionId", mSessionId );
	if(!isSuccessCode(mSessionInitErrorCode))
	{
		return false;
	}
	else
	{
		return true;
	}
}

/*
void HandlerGetFile::prepareResponseMsg( )
{
	if( isSuccessCode(mSessionInitErrorCode) )
	{
		HandlerSessionTransfer::prepareResponseMsg();
	}
	else
	{
		mResponse->updateStartline( mSessionInitErrorCode , convertErrCodeToString(mSessionInitErrorCode) );
		mResponse->updateHeader( "Connection" , "close");
		mResponse->updateHeader( "Server" , mEnv.getConfig().serverNameTag );
		mResponse->updateHeader("X-SessionId", mSessionId );
		mResponse->flushHeader(0);
		mResponse->complete();
	}
}
*/

int HandlerGetFile::process( )
{
	if(!createSession())
	{
		mResponse->updateStartline(mSessionInitErrorCode, convertErrCodeToString(mSessionInitErrorCode));
		mResponse->flushHeader(0);
		mResponse->complete();
		return errorWorkingInProcess;//return errorWorkingInProcess to disable C2HttpHandler to make a response
	}
	mUrl = mSessionId;

	C2Streamer::HandlerSessionTransfer::process();
	mLastErrorCode = mResponseParam->errorCode;
	return mLastErrorCode;
}

bool HandlerGetFile::parseRequest()
{
	if( !HandlerSessionTransfer::parseRequest() )
		return false;

	//get the session id
	mRequestParam->sessionId = mSessionId;
	{//re-make requestHint because sessionId is changed during parseRequest
		std::ostringstream oss;
		oss<<"Sess["<<mRequestParam->sessionId<<"] Seq["<< mEnv.getRequestSequence()<<"]";
		mRequestParam->requestHint = oss.str();
	}

	SessionTransferParamPtr request = SessionTransferParamPtr::dynamicCast( mRequestParam );
	return true;
}

//////////////////////////////////////////////////////////////////////////
////HandlerSessionTransfer
HandlerSessionTransfer::HandlerSessionTransfer(C2StreamerEnv& env, C2Service& svc, const std::string& url )
:C2HttpHandler(env, svc),
mUrl(url),
mLastErrorCode(200)
{
}

HandlerSessionTransfer::~HandlerSessionTransfer()
{
}

void HandlerSessionTransfer::postResponseFaied() {
	mSvc.getSessManager().destroySession( mRequestParam->sessionId );
}

std::string HandlerSessionTransfer::getConnectionPeerInfo()
{
	if(!mConn)
		return std::string("");
	std::string peerAddress;
	int peerPort = 0;
	mConn->getRemoteEndpoint( peerAddress , peerPort );
	char szBuf[256];
	szBuf[sizeof(szBuf)-1] = 0;
	snprintf(szBuf,sizeof(szBuf)-1,"COMM["FMT64 "] , client address[%s:%d]",mConn->getId() , peerAddress.c_str() , peerPort);
	return std::string(szBuf);
}

int HandlerSessionTransfer::process()
{
	if(mRequestParam == NULL || mResponseParam == NULL ) {
		mRequestParam	= new SessionTransferParam( mEnv, URLRULE_C2TRANSFER );
		mResponseParam	= new SessionTransferResponseParam();
	}
	assert( mRequestParam != 0 );
	assert( mResponseParam != 0 );
	if( !parseRequest() )
		return errorCodeBadRequest;

	mRequestParam->method = METHOD_TRANSFER_RUN;

	RequestParamPtr request =  mRequestParam;

	MLOG(ZQ::common::Log::L_DEBUG,SESSFMT(HandlerSessionTransfer,"got transfer command, peer info : %s") , getConnectionPeerInfo().c_str() );

	C2Streamer::C2HttpHandler::process();
	mLastErrorCode = mResponseParam->errorCode;
	return mLastErrorCode;
}

void HandlerSessionTransfer::onBreak() {
	C2HttpHandler::onBreak();
	std::string sessId = mUrl;
	std::string::size_type pos = sessId.find("/");
	if( pos != std::string::npos) {
		sessId= sessId.substr(pos+1);
	}
	if( sessId.empty())
		return;
	C2SessionPtr sess = mSvc.getSessManager().findSession(sessId);
	if(!sess)
		return;
	sess->markAsConnBroken();
}

bool HandlerSessionTransfer::parseRequest()
{
	//get the session id
	std::string::size_type posLastSlash = mUrl.find_last_of('/');
	if( posLastSlash == std::string::npos )
	{
		mRequestParam->sessionId = mUrl;
	}
	else
	{
		mRequestParam->sessionId = mUrl.substr( posLastSlash + 1);
	}
	mSessionId = mRequestParam->sessionId;

	SessionTransferParamPtr request = SessionTransferParamPtr::dynamicCast( mRequestParam );

	const char* subfilename = mRequest->queryArgument("subfile");
	if( subfilename && subfilename[0] )
	{
		request->requestFileExt = subfilename;
	}

	if( !C2HttpHandler::parseRequest() )
		return false;

	//get client ip and port
	if( mConn )
	{
		mConn->getRemoteEndpoint( request->clientIp , request->clientPort );
	}

	//get Start-From-IFrame flag
	const char* pStartFromIFrame = mRequest->header("Start-From-IFrame");
	if( pStartFromIFrame && pStartFromIFrame[0] != 0 ) {
		if( atoi(pStartFromIFrame) == 1) {
			request->seekIFrame = true;
		}
	}

	//get transfer delay
	const char* pTransferDelay = mRequest->header("Transfer-Delay");
	if( pTransferDelay && pTransferDelay[0] != 0 )
	{
		request->transferDelay = atoi( pTransferDelay );
		request->bHasTransferDelay = true;
	}

	const char* pForwardfor = mRequest->header("X-Forwarded-For");
	const char* pViaProxy = mRequest->header("Via");
	if( ( pViaProxy && strlen(pViaProxy) > 0 ) ||
			(pForwardfor && strlen(pForwardfor) > 0 ) ) {
		request->viaProxy = true;
	}

	//get range
	assert(request != 0 );
	const char* pRange = mRequest->header("Range");

	if( ! ( pRange && pRange[0] != 0 ) )
	{
		MLOG(ZQ::common::Log::L_DEBUG,SESSFMT(HandlerSessionTransfer,"no range is specified"));
		return true; //no range is specified
	}

	MLOG(ZQ::common::Log::L_DEBUG,SESSFMT(HandlerSessionTransfer,"got range:%s"),pRange);

	const char* pRangeBytes = strstr(pRange,"bytes=");
	if(  pRangeBytes != 0 )
		pRange = pRangeBytes + 6;//skip 'bytes='
	std::string strRange(pRange);
	ZQ::common::stringHelper::TrimExtra(strRange);
	parseRangeValue( strRange, request->range );
	return true;
}

void HandlerSessionTransfer::prepareResponseMsg( )
{
	SessionTransferResponseParamPtr responsePara = SessionTransferResponseParamPtr::dynamicCast( mResponseParam );
	SessionTransferParamPtr request	= SessionTransferParamPtr::dynamicCast(mRequestParam);
	assert( request != 0 );
	assert( responsePara != 0 );
	setStandardHeader( responsePara , false );
	if(isSuccessCode(responsePara->errorCode))
	{
		std::ostringstream oss;
		oss<<"attachment;filename="<<responsePara->filename;
		mResponse->updateHeader("Accept-Ranges","bytes");

		if( request->getConfUrlRule()->readerType == CLIENT_TYPE_DISKAIO ) {//read data from local filesystem
			mResponse->updateHeader("Content-Disposition",oss.str());
		}

		mResponse->updateHeader("Content-Type","application/octet-stream");
		if( request->viaProxy && responsePara->cacheable) {
			mResponse->updateHeader("Cache-Control","max-age=2592000");
		} else {
			mResponse->updateHeader("Cache-Control","no-cache");
		}

		if( responsePara->sessionType & SESSION_TYPE_SCHANGEREQ )//SeaChange request session, need to add x-File-Size
		{
			oss.str("");
			oss<<responsePara->fileSize;
			mResponse->updateHeader( "x-File-Size",oss.str() );
			mResponse->updateHeader( "Require","com.schange.cdn.v1");
		}
		oss.str("");
		oss<<responsePara->range.toString()<<"/"<<responsePara->fileSize;
		mResponse->updateHeader("X-SessionId", mRequestParam->sessionId );
		mResponse->flushHeader( );
	} else {
		mResponse->updateHeader("X-SessionId", mRequestParam->sessionId );
		mResponse->flushHeader( 0 );
	}

	MLOG(ZQ::common::Log::L_DEBUG,SESSFMT(HandlerSessionTransfer,"prepareResponseMsg() header flushed, code[%d] range[%s] sessId[%s]"), 
			responsePara->errorCode, responsePara->range.toString().c_str(), mRequestParam->sessionId.c_str() );

	C2SessionPtr sess = mSvc.getSessManager().findSession(request->sessionId);

	if( !sess )
	{
		MLOG(ZQ::common::Log::L_ERROR, SESSFMT(HandlerSessionTransfer,"prepareResponseMsg() fatal error, can't find session[%s]"),
			 request->sessionId.c_str() );
		mResponse->complete();
		return;
	}

	if( !isSuccessCode(responsePara->errorCode) )
	{
		mResponse->complete();
		sess->destroy();
	}
	else
	{
		sess->startTransfer();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//HandlerHLS
HandlerHLS::HandlerHLS( C2StreamerEnv& env, C2Service& svc, const std::string& uri )
	:HandlerGetFile(env, svc, uri) {
}

HandlerHLS::~HandlerHLS() {
}

int HandlerHLS::process( ) {
	assert( mRequestParam == 0 );
	assert( mResponseParam == 0 );

	mRequestParam	= new SessionTransferParam(mEnv, URLRULE_HLS);
	mResponseParam	= new SessionTransferResponseParam();

	assert( mRequestParam != 0 );
	assert( mResponseParam != 0 );
	mRequestParam->sessProp.queryIndex = false;

	return HandlerGetFile::process();
}


//////////////////////////////////////////////////////////////////////////////////////////
//AssetQueryHandlerCallback
AssetQueryHandlerCallback::AssetQueryHandlerCallback( HandlerAssetAttributeQuery* handler, LibAsync::EventLoop* loop )
:LibAsync::AsyncWork(*loop),
mHandler(handler){
}

AssetQueryHandlerCallback::~AssetQueryHandlerCallback() {
}

void AssetQueryHandlerCallback::onNotified() {
	queueWork();
}

void AssetQueryHandlerCallback::onAsyncWork() {
	mHandler->postProcess();
}


//////////////////////////////////////////////////////////////////////////////////////////
//HandlerAssetAttributeQuery
HandlerAssetAttributeQuery::HandlerAssetAttributeQuery( C2StreamerEnv& env, C2Service& svc ):
C2HttpHandler(env, svc) {
}

HandlerAssetAttributeQuery::~HandlerAssetAttributeQuery() {
}

char return2whitespace(const char& t ) {
	if( t == '\r' || t == '\n')
		return ' ';
	return t;
}

void HandlerAssetAttributeQuery::onBreak() {
	C2HttpHandler::onBreak();
	if(mResponse)
		mResponse->complete();
}

void HandlerAssetAttributeQuery::postProcess() {
	AssetAttribute::Ptr attr = mAssetAttr;
	if( attr->lastError() != 0 ) {
		MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(HandlerAssetAttributeQuery,"get an error while query asset info for:%s reqID[%ld]"),
				mFileName.c_str(), attr->reqId() );
		mResponse->updateStartline( 500, convertErrCodeToString( 500 ) );
		mResponse->flushHeader();
		mResponse->complete();
	} else {
		std::string baseinfo = attr->assetBaseInfo( );
		std::string memberinfo = attr->assetMemberInfo( );
		std::transform( baseinfo.begin(), baseinfo.end(), baseinfo.begin(), return2whitespace );
		std::transform( memberinfo.begin(), memberinfo.end(), memberinfo.begin(), return2whitespace );
		mResponse->updateStartline( 200, convertErrCodeToString(200));
		std::ostringstream oss;
		oss << "pwe: " << (attr->pwe() ? "true" : "false") <<"\n";
		oss << "baseinfo: " << baseinfo << "\n";
		oss << "memberinfo: " << memberinfo << "\n";
		std::string body = oss.str();
		mResponse->flushHeader( body.length() );
        mLeftMsg.clear();
		int ret = mResponse->addBodyContent( body.c_str(), body.length() );

		if (ret == (int)body.length())
        {
            mResponse->complete();
        }
        else if (ret > 0)
        {
            mLeftMsg.append(body.substr(ret));
            mResponse->registerWrite(this);
        }
		else if (ret == LibAsync::ERR_EAGAIN)
        {
            mLeftMsg = body;
            mResponse->registerWrite(this);
        }else{
            MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(HandlerAssetAttributeQuery,"postProcess() get an error while addBodyContent, filename[%s] error[%d]"), mFileName.c_str(), ret);
            postResponseFaied();
			mResponse->complete();
        }
	}
}

int HandlerAssetAttributeQuery::process( ) {
	std::map<std::string,std::string> args = mRequest->queryArguments();
	mFileName = args["filename"];
	if(mFileName.empty()) {
		MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(HandlerAssetAttributeQuery,"filename is not found in query string"));
		mResponse->updateStartline( 400, convertErrCodeToString( 400 ) );
		mResponse->flushHeader();
		mResponse->complete();
		return errorCodeBadRequest;
	}
	const ConfPerSessionConfig* perSessConf = mEnv.getConfig().getPerSessConf( URLRULE_QUERYASSETINFO );
	if(!perSessConf) {
		perSessConf = mEnv.getConfig().getPerSessConf("*");
	}
	assert(perSessConf != NULL);
	mAssetAttr = mSvc.getSessManager().getAssetAttribute("", mFileName, perSessConf->urlRule.readerType );
	assert( mAssetAttr!= NULL);
	if( mAssetAttr->asyncWait( new AssetQueryHandlerCallback( this, mRequest->getLoop() ))) {
		return errorWorkingInProcess;
	}
	postProcess();
	return errorCodeOK;
}

void HandlerAssetAttributeQuery::onWritable()
{
    std::string body = mLeftMsg;
    mLeftMsg.clear();
    int ret = mResponse->addBodyContent( body.c_str(), body.length() );

    if (ret == (int)body.length())
    {
        mResponse->complete();
    }
    else if (ret > 0)
    {
        mLeftMsg.append(body.substr(ret));
        mResponse->registerWrite(this);
    }
	else if (ret == LibAsync::ERR_EAGAIN)
    {
        mLeftMsg = body;
        mResponse->registerWrite(this);
    }else{
        MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(HandlerAssetAttributeQuery,"onWritable() get an error while addBodyContent, filename[%s] error[%d]"), mFileName.c_str(), ret);
		mResponse->complete();
        postResponseFaied();
    }
}

///////////////////////////////
//HandlerC2Locate
HandlerC2Locate::HandlerC2Locate( C2StreamerEnv& env, C2Service& svc )
:C2HttpHandler(env,svc){
}

HandlerC2Locate::~HandlerC2Locate() {
	if(mLocateCb)
	{
		mLocateCb->setLocateRequestPtrToNull();
	}
}

int HandlerC2Locate::process() {
    mLocateCb = new C2LocateCB(mRequest->getLoop(), this,mConnInfo.connID);

    char buf[32];
    ZQ::common::Guid id;
    id.create();
    id.toCompactIdstr(buf, sizeof(buf));
    mConnInfo.connID = buf;
    mConnInfo.uri = mRequest->uri();
    mConn->getRemoteEndpoint(mConnInfo.peerIP, mConnInfo.peerPort);
    mConn->getLocalEndpoint(mConnInfo.localIP, mConnInfo.localPort); 
	{
		ZQ::common::NativeThreadPool* pool = mEnv.mLocateThreadPool;
		int activeCount = pool->activeCount();
		int size = pool->size();
		int pending = pool->pendingRequestSize();
		if( pending >= size/2 ) {
			MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(HandlerC2Locate,"too many pending request: pending[%d] active[%d] threadSize[%d]"),
					pending, activeCount, size );
			if(pending > mEnv.getConfig().cacheLocateMaxPending ) {
				MLOG.warning(CLOGFMT(HandlerC2Locate, "reject new request due to too many request: pending[%d] active[%d] threadSize[%d]"),
						pending, activeCount, size );
				// simplify the response procedure  due to lacking of prepareResponse function of HandlerC2Locate
				//  
				mResponse->updateStartline( errorCodeServerUnavail, convertErrCodeToString(errorCodeServerUnavail));
				mResponse->updateHeader("Connection", "close");
				mResponse->updateHeader("Server", mEnv.getConfig().serverNameTag);
				mResponse->flushHeader();
				mResponse->complete();
				return errorCodeServerUnavail;
			}
		}
		(new CacheServerRequest(mEnv, mSvc, mLocateCb, mConnInfo, mXmlContent, mRequest))->start();
	}
	return errorWorkingInProcess;
}

void HandlerC2Locate::onBreak() {
    // C2HttpHandler::onBreak();
    // if(mResponse)
    //    mResponse->complete();
}

void HandlerC2Locate::onWritable()
{
    std::string body = mLeftMsg;
    mLeftMsg.clear();
    int ret = mResponse->addBodyContent( body.c_str(), body.length()  );

    if (ret == (int)body.length())
    {
        mResponse->complete();
    }
    else if (ret > 0)
    {
        mLeftMsg.append(body.substr(ret));
        mResponse->registerWrite(this);
    }
    else if (ret == LibAsync::ERR_EAGAIN)
    {
        mLeftMsg = body;
        mResponse->registerWrite(this);
    }else{
        MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(HandlerC2Locate, "[%s] onWritable() get an error while addBodyContent, error[%d]"), mConnInfo.connID.c_str(), ret);
        mResponse->complete();
        postResponseFaied();
    }
}

void HandlerC2Locate::postProcess(int code, const std::string& message, const std::string& xSessID)
{
    mResponse->updateStartline(code, convertErrCodeToString(code));
	mResponse->updateHeader("X-SessionID", xSessID);
    if(code >= 300)
    {
         // locate request failed
        MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(HandlerC2Locate, "[%s] get an error[%d:%s] when locate"), mConnInfo.connID.c_str(), code, message.c_str());
        mResponse->flushHeader();
        mResponse->complete();
        return;
    }
    std::string body = message;
    mResponse->flushHeader( body.length());
    mLeftMsg.clear();
    int ret = mResponse->addBodyContent( body.c_str(), body.length()  );

    if (ret == (int)body.length())
    {
        mResponse->complete();
    }
    else if (ret > 0)
    {
        mLeftMsg.append(body.substr(ret));
        mResponse->registerWrite(this);
    }
    else if (ret == LibAsync::ERR_EAGAIN)
    {
        mLeftMsg = body;
        mResponse->registerWrite(this);
    }else{
        MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(HandlerC2Locate,"[%s] postProcess() get an error while addBodyContent, error[%d]"), mConnInfo.connID.c_str(), ret);
        postResponseFaied();
        mResponse->complete();
    }
}

/////////////////////////////////////////////////////
/// HandlerDiagnosis

HandlerDiagnosis::HandlerDiagnosis( C2StreamerEnv& env, C2Service& svc )
:C2HttpHandler(env, svc) {
}

HandlerDiagnosis::~HandlerDiagnosis() {
}

void HandlerDiagnosis::setDefaultResponse( int code) {
	mResponse->updateStartline( code, convertErrCodeToString(code) );
	mResponse->updateHeader("Server","Process Information Diagnosis");
}

int HandlerDiagnosis::process( ) {
	std::map<std::string, std::string> args = mRequest->queryArguments();
	std::string method = args["method"];
	int code = 200;
	if( method == "list_buffer" ) {
		code = pickOutstandingBuffer();
	} else {
		setDefaultResponse(errorCodeContentNotFound);
		mResponse->flushHeader();
		code = errorCodeContentNotFound;
	}
	mResponse->complete();
	return code;
}

int HandlerDiagnosis::pickOutstandingBuffer( ) {
	CacheCenter& cc = mSvc.getCacheCenter();
	std::vector<CacheBufferStatusInfo> bufs;
	cc.getFlyBuffersStatus(bufs);
	setDefaultResponse(errorCodeOK);
	if( bufs.size() == 0 ) {
		return errorCodeOK;
	}
	mResponse->updateHeader("content-type","text/*");
	std::ostringstream oss;

	std::vector<CacheBufferStatusInfo>::const_iterator it = bufs.begin();
	for ( ; it != bufs.end() ; it ++ ) {
		oss<<"buf["<<std::setw(8)<<it->bufId<<"] reqId["
			<<std::setw(8)<<it->reqId<<"]"<<std::endl;
	}
	std::string output = oss.str();
	mResponse->addBodyContent( output.c_str(), output.length());
	return errorCodeOK;
}

//////////////////////////////////////////////////////////////////////////////////////////
///HttpHanlderFactory
HttpHanlderFactory::HttpHanlderFactory( C2StreamerEnv& env , C2Service& svc)
:mEnv(env),
mSvc(svc)
{
}
HttpHanlderFactory::~HttpHanlderFactory()
{
}

#define STRSWITCH() if(0){
#define STRCASE(x)	} else if(::strstr( uri , x ) != 0 ){
#define STRENDCASE() }


ZQHttp::IRequestHandler* HttpHanlderFactory::create(const char* uri)
{
	STRSWITCH()
		STRCASE(URLRULE_C2INIT)				return new HandlerTransferInit( mEnv, mSvc);
		STRCASE(URLRULE_C2TERM)				return new HanlderTransferTerminate( mEnv, mSvc);
		STRCASE(URLRULE_C2STATUS)			return new HanlderSessionStatus( mEnv, mSvc);
		STRCASE(URLRULE_C2RESSTAT)			return new HandlerResourceStatus( mEnv, mSvc);
		STRCASE(URLRULE_GETFILE)			return new HandlerGetFile(mEnv,mSvc,std::string(uri));
		STRCASE(URLRULE_C2TRANSFER)			return new HandlerSessionTransfer( mEnv, mSvc,std::string(uri));
		STRCASE(URLRULE_HLS)				return new HandlerHLS( mEnv,mSvc,std::string(uri));
		STRCASE("scs/queryassetinfo")		return new HandlerAssetAttributeQuery( mEnv, mSvc );
		STRCASE("/cacheserver")			    return new HandlerC2Locate( mEnv, mSvc );
		STRCASE("/vodadi.cgi")			    return new HandlerC2Locate( mEnv, mSvc );
		STRCASE("/processinformation/")		return new HandlerDiagnosis( mEnv, mSvc );
	STRENDCASE()
	return 0;
}

void HttpHanlderFactory::destroy( ZQHttp::IRequestHandler* req )
{
	if( req )
	{
		delete req;
		req = NULL;
	}
}




}//namespace C2Streamer

