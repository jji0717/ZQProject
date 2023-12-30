
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
	#define	SESSFMT(x,y) 	"%s/%08X/REQUEST[%s]\t"##y, request->requestHint.c_str() , GetCurrentThreadId(),#x	
#elif defined ZQ_OS_LINUX
	#define	SESSFMT(x,y) 	"%s/%08X/REQUEST[%s]\t"y, request->requestHint.c_str() , (unsigned int) gettid(),#x	
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
	mResponse	= new C2HttpResponseHanlder(&resp,mConn);
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
	
	process();	
	postResponse();	
	
	mRequestParam	= 0;
	mResponseParam	= 0;	
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

bool C2HttpHandler::process()
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
	return true;
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

bool HandlerTransferInit::process( )
{
	assert( mRequestParam == 0 );
	assert( mResponseParam == 0 );
	mRequestParam	= new TransferInitRequestParam();
	mResponseParam	= new TransferInitResponseParam();
	assert( mRequestParam != 0 );
	assert( mResponseParam != 0 );

	C2SessionPtr c2session;
	assert(c2session == 0 );
	c2session = mSvc.getSessManager().createSession(mRequestParam->sessionId);
	if( !c2session )
	{
		mResponseParam->setLastErr( mRequestParam , errorCodeInternalError , "failed to create a new session" );
		return false;
	}
	mRequestParam->method = METHOD_TRANSFER_INIT;
	mRequestParam->sessionId = c2session->getSessId();

	if( !parseRequest() )
		return false;
	
	if( !C2HttpHandler::process() )
		return false;
	
	return true;
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

bool HanlderTransferTerminate::process()
{
	assert( mRequestParam == 0 );
	assert( mResponseParam == 0 );
	mRequestParam	= new TransferTermRequestParam();
	mResponseParam	= new TransferTermResponseParam();
	assert( mRequestParam != 0 );
	assert( mResponseParam != 0 );

	if(!parseRequest())
		return false;
	
	mSvc.getSessManager().destroySession( getSessionIdFromCompoundString(mRequestParam->sessionId) );

	return true;
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

bool HanlderSessionStatus::process()
{
	assert( mRequestParam == 0 );
	assert( mResponseParam == 0 );
	mRequestParam	= new SessionStatusRequestParam();
	mResponseParam	= new SessionStatusResponseParam();
	assert( mRequestParam != 0 );
	assert( mResponseParam != 0 );
	
	if( !parseRequest() )
		return false;
	SessionStatusRequestParamPtr request = SessionStatusRequestParamPtr::dynamicCast(mRequestParam);
	SessionStatusResponseParamPtr response = SessionStatusResponseParamPtr::dynamicCast(mResponseParam);
	return mSvc.getSessionStatus( request , response );
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
bool HandlerResourceStatus::process()
{
	assert( mRequestParam == 0 );
	assert( mResponseParam == 0 );
	mRequestParam	= new ResourceStatusRequestParam();
	mResponseParam	= new ResourceStatusResponseParam();
	assert( mRequestParam != 0 );
	assert( mResponseParam != 0 );
	if( !parseRequest() )
		return false;
	
	ResourceStatusRequestParamPtr request = ResourceStatusRequestParamPtr::dynamicCast(mRequestParam);
	ResourceStatusResponseParamPtr response = ResourceStatusResponseParamPtr::dynamicCast(mResponseParam);
	return mSvc.getResourceStatus( request , response );
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

#define URLRULE_GETFILE "scs/getfile"

HandlerGetFile::HandlerGetFile(C2StreamerEnv& env , C2Service& svc , const std::string& url)
:HandlerSessionTransfer(env,svc,url),
mLastErrorCode(200),
mGetFileUrl(url)
{
}
HandlerGetFile::~HandlerGetFile()
{
	if( !isSuccessCode(mLastErrorCode) ) {
		C2SessionPtr c2sess = mSvc.getSessManager().findSession( mSessionId );
		if( c2sess )
			c2sess->destroy();
	}
}

void HandlerGetFile::fixupFilenameAndRange( std::string& filename, TransferRange& range ) {
	// filename : xxx-offsetPsize
	std::string::size_type posDash = filename.find_last_of('-'); // the delimitor of StartOffset
	std::string::size_type posPlus = filename.find_last_of('L'); // the delimitor of Length
	if( posDash == std::string::npos || posPlus == std::string::npos || posDash > posPlus ) 
		return;//not the filename pattern we want
	sscanf(filename.c_str()+posDash+1,"%ld",&range.startPos); // read the start offset
	int64 size = 0;
	sscanf(filename.c_str()+posPlus+1,"%ld",&range.endPos); // read the length, temporarily save it in endPos
	range.endPos = range.startPos + range.endPos - 1; // calculate the real endPos
	range.bEndValid = range.bStartValid = true;
	filename = filename.substr(0,posDash);
}

bool HandlerGetFile::createSession(  )
{
	std::string clientTransfer ;
	std::string transferAddress;
	std::string fileName;
	int64 ingressCapacity = ( 2000LL * 2000 * 1000 * 1000);
	int64 transferRate = mEnv.getConfig().mDefaultTransferBitrate;
	int64 transferTimeout = 5000;
	TransferRange trange;
	int64 delay = -50;
	
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
	if( vars["range"].length() > 0 )
		parseRangeValue(vars["range"],trange);
	
	fixupFilenameAndRange( fileName, trange);

	if( fileName.empty() || transferAddress.empty() )
	{
		mSessionInitErrorCode = errorCodeBadRequest;
		return false;
	}
	
	C2SessionPtr sess =  getC2StreamerService()->getSessManager().createSession("");
	if( !sess )
	{
		mSessionInitErrorCode = errorCodeInternalError;
		return false;
	}
	mSessionId					= sess->getSessId();
	int64 sesStart = ZQ::common::TimeUtil::now();
	mEnv.setLatencyMap(fileName, trange.startPos, sesStart);
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(HandlerGetFile,"init session[%s]: client[%s] transferServer[%s] file[%s] ic[%ld] rate[%ld] timeout[%ld] range[%s] delay[%ld] "),
		 mSessionId.c_str() , 
		 clientTransfer.c_str(), transferAddress.c_str() , fileName.c_str(),
		 ingressCapacity , transferRate , transferTimeout , 
		 trange.toString().c_str() , delay );

	TransferInitRequestParamPtr request = new TransferInitRequestParam();
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
		mResponse->flushHeader(0);
		mResponse->complete();
	}
}

bool HandlerGetFile::process( )
{
	if(!createSession())
	{
		return false;
	}
	mUrl = mSessionId;

	bool bOK = C2Streamer::HandlerSessionTransfer::process();
	mLastErrorCode = mResponseParam->errorCode;
	return bOK;

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
mUrl(url)
{
}

HandlerSessionTransfer::~HandlerSessionTransfer()
{
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

bool HandlerSessionTransfer::process()
{
	assert( mRequestParam == 0 );
	assert( mResponseParam == 0 );
	mRequestParam	= new SessionTransferParam();
	mResponseParam	= new SessionTransferResponseParam();
	assert( mRequestParam != 0 );
	assert( mResponseParam != 0 );
	if( !parseRequest() )
		return false;
	
	mRequestParam->method = METHOD_TRANSFER_RUN;
	
	RequestParamPtr request =  mRequestParam;	
	
	MLOG(ZQ::common::Log::L_DEBUG,SESSFMT(HandlerSessionTransfer,"got transfer command, peer info : %s") , getConnectionPeerInfo().c_str() );
	
	return C2Streamer::C2HttpHandler::process();
}

void HandlerSessionTransfer::onBreak() {
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

		if( mEnv.getConfig().clientType == 0 ) {//read data from local filesystem
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
		//mResponse->updateHeader("Content-Range", oss.str() );
		mResponse->flushHeader( );
	} else {
		mResponse->flushHeader( 0 );
	}
	
	MLOG(ZQ::common::Log::L_DEBUG,SESSFMT(HandlerSessionTransfer,"prepareResponseMsg() header flushed"));
	
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
		if(sess)
		{
			sess->changeState( SESSION_STATE_IDLE );
		}
	}
	else
	{
		C2SessionPtr sess = mSvc.getSessManager().findSession(request->sessionId);
		if( sess )
		{
			sess->startTransfer();
		}
	}	
}

//////////////////////////////////////////////////////////////////////////////////////////
//HandlerHLS
HandlerHLS::HandlerHLS( C2StreamerEnv& env, C2Service& svc, const std::string& uri ) 
	:C2HttpHandler(env,svc),
	mUri(uri)
{
}

HandlerHLS::~HandlerHLS() {
}

bool HandlerHLS::process( ) {
	assert( mRequestParam == 0 );
	assert( mResponseParam == 0 );
	mRequestParam	= new HLSRequestParam();
	mResponseParam	= new HLSResponseParam();
	assert( mRequestParam != 0 );
	assert( mResponseParam != 0 );
	if( !parseRequest() )
		return false;
	
	mRequestParam->method = METHOD_HLS_GET;
	
	HLSRequestParamPtr request =  HLSRequestParamPtr::dynamicCast(mRequestParam);
	HLSResponseParamPtr response = HLSResponseParamPtr::dynamicCast(mResponseParam);
	assert(request!=0);
	assert(response!=0 );	
	
	mSvc.getHlsServer().process(request,response );

	return true;
}

bool HandlerHLS::parseRequest() {
	HLSRequestParamPtr request =  HLSRequestParamPtr::dynamicCast(mRequestParam);
	HLSResponseParamPtr response = HLSResponseParamPtr::dynamicCast(mResponseParam);
	assert(request!=0);
	assert(response!=0 );
	
	request->url = std::string("http://dummy.com") + mRequest->getFullUri();

	const char* uri = mRequest->uri();
	if(!uri) {
		MLOG(ZQ::common::Log::L_DEBUG,SESSFMT(HandlerHLS,"no uri is specified"));
		return false;
	}
	std::string tmp = uri;
	std::string::size_type pos = tmp.rfind(".m3u8");
	if( pos != std::string::npos && tmp.length() == pos + 5 )
		tmp = tmp.substr(0,pos);
	pos = tmp.rfind('/');
	if( pos != std::string::npos ) {
		tmp = tmp.substr(pos+1);
	}
	pos = tmp.find('_');
	if (pos == std::string::npos) {
		request->contentName = tmp;
	} else {
		request->contentName = tmp.substr(0,pos);
		request->subLevelName = tmp.substr(pos+1);
	}
	return true;
}

void HandlerHLS::prepareResponseMsg( ) {
	//prepare the message
	HLSResponseParamPtr responsePara = HLSResponseParamPtr::dynamicCast(mResponseParam);
	assert( responsePara != 0 );

	setStandardHeader( responsePara );
	mResponse->updateHeader("Content-Type","application/vnd.apple.mpegurl");
	

	//prepare content body
	const std::string& strBody = responsePara->content;
	mResponse->flushHeader( strBody.length() );
	mResponse->addBodyContent( strBody.c_str() , strBody.length() );
	
	if( !mResponse->complete() )
	{
		postResponseFaied();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//HandlerAssetAttributeQeury
HandlerAssetAttributeQeury::HandlerAssetAttributeQeury( C2StreamerEnv& env, C2Service& svc ):
C2HttpHandler(env, svc) {
}

HandlerAssetAttributeQeury::~HandlerAssetAttributeQeury() {
}

char return2whitespace(const char& t ) {
	if( t == '\r' || t == '\n')
		return ' ';
	return t;
}

bool HandlerAssetAttributeQeury::process( ) {
	std::map<std::string,std::string> args = mRequest->queryArguments();
	std::string filename = args["filename"];
	if(filename.empty()) {
		MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(HandlerAssetAttributeQeury,"filename is not found in query string"));
		mResponse->updateStartline( 400, convertErrCodeToString( 400 ) );
		mResponse->flushHeader();
		mResponse->complete();
		return false;
	}
	AssetAttribute::Ptr attr = mSvc.getSessManager().getAssetAttribute( filename );
	assert( attr != NULL);
	attr->wait();
	if( attr->lastError() != 0 ) {
		MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(HandlerAssetAttributeQeury,"get a error while query asset info for:%s"),filename.c_str());
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
		mResponse->addBodyContent( body.c_str(), body.length() );
		mResponse->complete();
	}
	return true;
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
		STRCASE("scs/transferinitiate")					return new HandlerTransferInit( mEnv, mSvc);
		STRCASE("scs/transferterminate")				return new HanlderTransferTerminate( mEnv, mSvc);
		STRCASE("scs/status")							return new HanlderSessionStatus( mEnv, mSvc);
		STRCASE("scs/resourcestatus")					return new HandlerResourceStatus( mEnv, mSvc);
		STRCASE(URLRULE_GETFILE)							return new HandlerGetFile(mEnv,mSvc,std::string(uri));
		STRCASE(TRANSFERSESSION_PREFIX)					return new HandlerSessionTransfer( mEnv, mSvc,std::string(uri));
		STRCASE("assets/")								return new HandlerHLS( mEnv,mSvc,std::string(uri));
		STRCASE("scs/queryassetinfo")					return new HandlerAssetAttributeQeury( mEnv, mSvc );
	STRENDCASE()
	return 0;
}

void HttpHanlderFactory::destroy( ZQHttp::IRequestHandler* req )
{
	if( req )
	{
		delete req;
	}
}




}//namespace C2Streamer

