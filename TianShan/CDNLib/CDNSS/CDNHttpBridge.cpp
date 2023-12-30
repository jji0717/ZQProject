#include <boost/thread.hpp>
#include "CdnStreamerManager.h"
#include "CDNHttpBridge.h"
#include <sstream>
#include <string>
#include <assert.h>
#include <urlstr.h>
#include "CdnEnv.h"

namespace ZQ
{
namespace StreamService
{
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//CDNHttpFactory
CDNHttpFactory::CDNHttpFactory( CdnSsEnvironment* environment )
:env(environment)
{
}

CDNHttpFactory::~CDNHttpFactory( )
{
}

#define STRSWITCH() if(0){
#define STRCASE(x)	} else if(::strncmp( uri , x ,strlen(x) ) == 0 ){
#define STRENDCASE() }

ZQHttp::IRequestHandler* CDNHttpFactory::create( const char* uri)
{
	STRSWITCH()
		STRCASE("/c2cp/transferingresscapacityupdate")		return new TranferUpdateIngressCapacity(env);
		STRCASE("/c2cp/transferstateupdate")				return new TransferUpdateState(env);
		STRCASE("/c2cp/transferresourceupdate")			return new TranferUpdateResource(env);
	STRENDCASE()
	return NULL;
}

void CDNHttpFactory::destroy( ZQHttp::IRequestHandler* req)
{
	if( req )
	{
		delete req;
	}
}

/////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//CDNMiniXmlBuilder
CDNMiniXmlBuilder::CDNMiniXmlBuilder( )
{
}

CDNMiniXmlBuilder::~CDNMiniXmlBuilder( )
{
}

void CDNMiniXmlBuilder::create( const std::string& rootNode )
{
	mRootName	= rootNode;
}

void CDNMiniXmlBuilder::addParameter( const std::string& key , const std::string& value )
{
	Parameter p;
	if( key.empty() || value.empty() )
		return;
	p.nodeName	=	key;
	p.value		=	value;
	mParas.push_back(p);
}

void CDNMiniXmlBuilder::reset( )
{
	mParas.clear();
	mRootName= "";
}

std::string CDNMiniXmlBuilder::exportXml( )
{
	std::ostringstream oss;
	oss<<"<" << mRootName<< ">\r\n";
	ParameterS::const_iterator it = mParas.begin();
	for( ; it != mParas.end() ; it++ )
	{
		oss<< "\t<" << it->nodeName << ">" << it->value << "</" << it->nodeName <<">\r\n";
	}
	oss<< "</" << mRootName << ">";
	return oss.str();
}

//////////////////////////////////////////////////////////////////////////
///C2HttpClient
C2HttpClient::C2HttpClient( CdnSsEnvironment* environment ,const std::string& contextKey )
:ZQ::common::HttpClient(environment->getHttpLog() ),
env(environment),
mContextKey(contextKey)
{

}
C2HttpClient::~C2HttpClient( )
{
}

void C2HttpClient::setProxyUrl( const std::string& url ) {
	ZQ::common::URLStr urlstr(url.c_str());
	setProxy( urlstr.getHost(), urlstr.getPort());
}

void C2HttpClient::setPeerAddress( const std::string& peerIp, const std::string& peerPort )
{
	mPeerIp = peerIp;
	mPeerPort = peerPort;
}
void C2HttpClient::initHttpHeader( )
{
	setHeader("User-Agent","SeaChange C2 Locate Server");
	setHeader("Host","C2Streamer");
	setHeader("Content-Type","text/xml");	
}

int	C2HttpClient::lastErrorCode( )
{
	return getStatusCode();
}

const std::string&	C2HttpClient::getLastError( ) const
{
	return mLastError;
}

int32 C2HttpClient::sendRequest( const std::string& uri, bool getOrPost )
{
	init( );
	//	int iRet = -1;
	bool bOk = false; 
	mUri = uri;
	do
	{

		if(!sendHeader())
		{
			break;
		}

		if(! sendBodyContent() ) 
		{
			break;
		}

		httpEndSend();

		//receive data from peer
		if( httpBeginRecv() )
		{
			HTTPLOG(ZQ::common::Log::L_ERROR,CLOGFMT(sendRequest,"failed to received data from %s:%s with erroCode[%d]"),
				mPeerIp.c_str(),mPeerPort.c_str(),getErrorcode() );
			break;
		}
		while(!isEOF()) 
		{
			if(httpContinueRecv()) 
			{
				HTTPLOG(ZQ::common::Log::L_ERROR,CLOGFMT(sendRequest,"failed to received data from %s:%s with errorCode[%d]"),
					mPeerIp.c_str(),mPeerPort.c_str(),getErrorcode() );				
				break;
			}
		}

		if(getBodyContent() )
			bOk = true;
		else
			bOk = false;

	}while(0);
	uninit();
	if( !bOk )
		return -1;
	if(!processBodyContent() )
	{
		return -1;
	}
	return getStatusCode();
}

//////////////////////////////////////////////////////////////////////////
///CDNIndexGetter

CDNIndexGetter::CDNIndexGetter( CdnSsEnvironment* environment ,const std::string& contextKey ,
							   const std::string& upstreamUrl, size_t sizeWanted )
							   :C2HttpClient(environment,contextKey),
							   mUpstreamUrl(upstreamUrl),
							   mBuffer(0),
							   mWantedSize(sizeWanted),
							   mDataSize(0)
{
	if(mWantedSize>0) {
		std::ostringstream oss;
		oss<<"bytes=0-"<<mWantedSize-1;
		setHeader((char*)"Range",(char*)oss.str().c_str());
	}
}

CDNIndexGetter::~CDNIndexGetter()
{
	if(mBuffer) {
		free(mBuffer);
	}
}

int32 CDNIndexGetter::invoke( const std::string& )
{
	int status = sendRequest("",true);
	return status;
}

bool CDNIndexGetter::sendHeader( ) 
{	
	if( httpConnect( mUpstreamUrl.c_str() , HTTP_GET ) )
	{
		HTTPLOG(ZQ::common::Log::L_ERROR,CLOGFMT(sendRequest,"failed to connect %s:%s"),
			mPeerIp.c_str(),mPeerPort.c_str());
		return false;
	}
	return true;
}

bool CDNIndexGetter::sendBodyContent( ) 
{
	return true;
}
bool CDNIndexGetter::getBodyContent( )
{
	if( mBuffer ) { free( mBuffer); }
	size_t contentLength = getContentLength();
	mDataSize = mWantedSize > contentLength ? contentLength : mWantedSize;
	mBuffer = (char*)malloc(mDataSize);
	assert(mBuffer);
	mDataSize = getBodyData(mBuffer,mDataSize);
	if(mDataSize == 0)
		return false;
	return true;
}

bool CDNIndexGetter::processBodyContent( )
{
	return true;// do nothing
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//CDNHttpClient
CDNHttpClient::CDNHttpClient( CdnSsEnvironment* environment ,const std::string& contextKey  )
:C2HttpClient(environment,contextKey)
{
	initHttpHeader();
	setPeerAddress( env->TransferServerHttpIp(), env->TransferServerHttpPort());	
}

CDNHttpClient::~CDNHttpClient( )
{	
}

bool CDNHttpClient::sendHeader( )
{
	std::string url = mPeerIp+":"+ mPeerPort +"/" + mUri;
	if( httpConnect( url.c_str() , HTTP_POST ) )
	{
		HTTPLOG(ZQ::common::Log::L_ERROR,CLOGFMT(sendRequest,"failed to connect %s:%s"),
			mPeerIp.c_str(),mPeerPort.c_str());
		return false;
	}
	return true;
}

bool CDNHttpClient::sendBodyContent( )
{
	std::string xmlContent = mXmlBuilder.exportXml();
	if( httpSendContent( xmlContent.c_str() , xmlContent.length() ) || httpEndSend() )
	{
		HTTPLOG(ZQ::common::Log::L_ERROR,CLOGFMT(sendRequest,"failed to send data to %s:%s with errorCode[%d]"),
			mPeerIp.c_str() , mPeerPort.c_str(),getErrorcode() );
		return false;
	}
	return true;
}

bool CDNHttpClient::getBodyContent( ) 
{
	getContent(mResult);
	return true;
}

bool CDNHttpClient::processBodyContent( )
{
	return parseResultXml(mResult);
}

bool CDNHttpClient::parseResultXml( const std::string& xml )
{
	try
	{
		mXmlParser.parse( xml.c_str() ,static_cast<int>( xml.length() ), 1 );
	}
	catch( ZQ::common::ExpatException& ex)
	{
		HTTPLOG(ZQ::common::Log::L_ERROR,CLOGFMT(CDNHttpClient,"failed to parse xml result because [%s]"),
			ex.what() );
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/// C2LocateRequest
C2LocateRequest::C2LocateRequest(CdnSsEnvironment* environment , 
							   const std::string& contextKey,
							   const std::string& peerIp, const std::string& peerPort,
							   const std::string& pid, const std::string& paid,
							   const std::string& subtype, 
							   const std::string& clientTransfer, 
							   const std::string& transferRate, 
							   const std::string& ingressCapacity, 
							   const std::string& range, 
							   const std::string& delay):
	CDNHttpClient(environment,contextKey)
{
	std::ostringstream oss;
	oss<<"<LocateRequest>"<<std::endl;
	oss<<"   <Object>"<<std::endl;
	oss<<"       <Name>"<<std::endl;
	oss<<"          <ProviderID>"<<pid<<"</ProviderID>"<<std::endl;
	oss<<"          <AssetID>"<<paid<<"</AssetID>"<<std::endl;
	oss<<"       </Name>"<<std::endl;
	oss<<"       <SubType>"<<subtype<<"/Subtype>"<<std::endl;
	oss<<"   </Object>"<<std::endl;
	oss<<"   <ClientTransfer>"<<clientTransfer<<"<ClientTransfer>"<<std::endl;
	oss<<"   <TransferRate>"<<transferRate<<"</TransferRate>"<<std::endl;
	oss<<"   <IngressCapacity>"<<ingressCapacity<<"</IngressCapacity>"<<std::endl;
	oss<<"   <Range>"<<range<<"</Range>"<<std::endl;
	oss<<"   <TransferDelay>"<<delay<<"</TransferDelay>"<<std::endl;
	oss<<"   "<<std::endl;
	oss<<"</LocateRequest>";
	mRequestBody = oss.str();
	setPeerAddress(peerIp,peerPort);
}
C2LocateRequest::~C2LocateRequest()
{
}

int32 C2LocateRequest::invoke( const std::string& uri )
{
	return sendRequest(uri);
}

bool C2LocateRequest::sendBodyContent( )
{
	if( httpSendContent( mRequestBody.c_str() , mRequestBody.length() ) || httpEndSend() )
	{
		HTTPLOG(ZQ::common::Log::L_ERROR,CLOGFMT(sendRequest,"failed to send data to %s:%s with errorCode[%d]"),
			mPeerIp.c_str() , mPeerPort.c_str(),getErrorcode() );
		return false;
	}
	return true;
}
///get transfer id generated by Http server
const std::string&	C2LocateRequest::getTransferId( ) const
{
	return mTransferId;
}

const std::string&	C2LocateRequest::getDownloadServerIp( ) const
{
	return mDownloadServerIp;
}

const std::string&	C2LocateRequest::getDownloadServerPort( ) const
{
	return mDownloadServerPort;
}

bool C2LocateRequest::processXML( const std::string& result )
{
	if( !parseResultXml(result) )
		return false;
	const SimpleXMLParser::Node& root = mXmlParser.document();
	if( getStatusCode()/100 != 2 )
	{
		const SimpleXMLParser::Node* lastError = findNode(&root,"TransferInitiateResponse/ErrorText");
		if( lastError )
		{
			mLastError = lastError->content;
		}
		else
		{
			HTTPLOG(ZQ::common::Log::L_ERROR,CLOGFMT(TransferInitiate,"return code/100 != 2, and missing ErrorText"));
		}
		return false;
	}
	else
	{
		const SimpleXMLParser::Node* transferId = findNode(&root , "LocateResponse/TransferID");
		if( transferId )
		{
			mTransferId = transferId->content;
		}
		else
		{
			HTTPLOG(ZQ::common::Log::L_ERROR,CLOGFMT(TransferInitiate,"return success , but misssing TransferID in LocateResponse"));
			return false;
		}

		const SimpleXMLParser::Node* serverAddres =  findNode( &root,"LocateResponse/TransferPort");
		if( serverAddres )
		{
			mDownloadServerIp = serverAddres->content;
		}
		else
		{
			HTTPLOG(ZQ::common::Log::L_ERROR,CLOGFMT(TransferInitiate,"return success , but misssing TransferPort in LocateResponse"));
			return false;
		}

		return true;
	}
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
///
TransferInitiate::TransferInitiate(CdnSsEnvironment* environment ,
								   const std::string& contextKey,
									const std::string& clientTranfer , 
								   const std::string& transferAddress ,
								   const std::string& ingressCapacity, 
								   const std::string& allocatedTRansferRate,
								   const std::string& fileName,
								   const std::string& transferRate,
								   const std::string& transferTimeout,
								   const std::string& range,
								   const std::string& transferDelay)
								   :CDNHttpClient(environment,contextKey)
{
	mXmlBuilder.reset();
	mXmlBuilder.create("TransferInitiate");
	mXmlBuilder.addParameter( "ClientTransfer" ,clientTranfer );
	mXmlBuilder.addParameter( "TransferAddress", transferAddress );
	mXmlBuilder.addParameter( "IngressCapacity", ingressCapacity );
	
	mXmlBuilder.addParameter( "AllocatedTransferRate", allocatedTRansferRate );

	mXmlBuilder.addParameter( "Filename", fileName);
	mXmlBuilder.addParameter( "TransferRate", transferRate);
	mXmlBuilder.addParameter( "TransferTimeout",transferTimeout );

	if( !range.empty() )
	{
		mXmlBuilder.addParameter("Range" , range );
	}
	if(! transferDelay.empty() )
	{
		mXmlBuilder.addParameter("TransferDelay" , transferDelay );
	}
}
TransferInitiate::~TransferInitiate()
{

}

int32 TransferInitiate::invoke( const std::string& uri ) 
{
	//return sendRequest("c2cp/transferinitiate");
	return sendRequest(uri);
}

bool TransferInitiate::processXML( const std::string& result )
{
	if( !parseResultXml(result) )
		return false;
	const SimpleXMLParser::Node& root = mXmlParser.document();
	if( getStatusCode() != 200 )
	{
		const SimpleXMLParser::Node* lastError = findNode(&root,"TransferInitiateResponse/ErrorText");
		if( lastError )
		{
			mLastError = lastError->content;
		}
		else
		{
			HTTPLOG(ZQ::common::Log::L_ERROR,CLOGFMT(TransferInitiate,"return code != 200, but missing ErrorText in TransferInitiateResponse"));
		}
		return false;
	}
	else
	{
		const SimpleXMLParser::Node* transferId = findNode(&root , "TransferInitiateResponse/TransferID");
		if( transferId )
		{
			mTransferId = transferId->content;
		}
		else
		{
			HTTPLOG(ZQ::common::Log::L_ERROR,CLOGFMT(TransferInitiate,"return code == 200 , but misssing TransferID in TransferInitiateResponse"));
			return false;
		}
		
		const SimpleXMLParser::Node* availRange = findNode(&root , "TransferInitiateResponse/AvailableRange");
		if(availRange)
		{
			mAvailRange = availRange->content;
		}
		//OpenForWrite
		const SimpleXMLParser::Node* openForWrite = findNode(&root , "TransferInitiateResponse/OpenForWrite");
		if(openForWrite)
		{
			mOpenForWrite = openForWrite->content;
// 			if( stricmp(openForWrite->content.c_str() , "yes") == 0 )
// 			{
// 				mOpenForWrite = true;
// 			}
// 			else
// 			{
// 				mOpenForWrite = false;
// 			}
		}
		else
		{
			mOpenForWrite = "";
		}

		return true;
	}
}
const std::string&	TransferInitiate::getTransferId( ) const
{
	return mTransferId;
}

const std::string&	TransferInitiate::getAvailRange( ) const
{
	return mAvailRange;
}

const std::string&	TransferInitiate::getOpenForWrite( ) const
{
	return mOpenForWrite;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
TransferTerminate::TransferTerminate(CdnSsEnvironment* environment , 
									 const std::string& contextKey,
									const std::string& tranferId,
									const std::string& clientTransfer )
:CDNHttpClient(environment,contextKey)
{
	mXmlBuilder.create("TransferTerminate");
	mXmlBuilder.addParameter( "TransferID", tranferId );
	if( !clientTransfer.empty() )
	{
		mXmlBuilder.addParameter( "ClientTransfer", clientTransfer );
	}
}
TransferTerminate::~TransferTerminate()
{

}

int32 TransferTerminate::invoke( const std::string& uri )
{
	//return sendRequest("c2cp/transferterminate");
	return sendRequest(uri);
}

bool TransferTerminate::processXML(const std::string &result)
{
	if( !parseResultXml(result) )
		return false;
	const SimpleXMLParser::Node& root = mXmlParser.document();
	if( getStatusCode() != 200 )
	{
		const SimpleXMLParser::Node* lastError = findNode(&root,"TransferTerminateResponse/ErrorText");
		if( lastError )
		{
			mLastError = lastError->content;
		}
		else
		{
			HTTPLOG(ZQ::common::Log::L_ERROR,CLOGFMT(TransferInitiate,"return code != 200, but missing ErrorText in TransferTerminate"));
		}
		return false;
	}
	else
	{
		return true;
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
IngressCapacityUpdate::IngressCapacityUpdate( CdnSsEnvironment* environment, 
											 const std::string& contextKey,
												const std::string& clientTransfer , 
												const std::string& ingressCapacity, 
												const std::string& allocatedIngressCapacity )
												:CDNHttpClient(environment,contextKey)
{
	mXmlBuilder.create("IngressCapacityUpdate");
	mXmlBuilder.addParameter( "ClientTransfer" , clientTransfer );
	mXmlBuilder.addParameter( "IngressCapacity", ingressCapacity );
	mXmlBuilder.addParameter( "AllocatedIngressCapacity",allocatedIngressCapacity );
}
IngressCapacityUpdate::~IngressCapacityUpdate()
{

}

int32 IngressCapacityUpdate::invoke( const std::string& ip , const std::string& port ) 
{
	mPeerIp = ip;
	mPeerPort = port;
	return sendRequest("c2cp/ingresscapacityupdate");
}
bool IngressCapacityUpdate::processXML(const std::string &result)
{
	if( !parseResultXml(result) )
		return false;
	const SimpleXMLParser::Node& root = mXmlParser.document();
	if( getStatusCode() != 200 )
	{
		const SimpleXMLParser::Node* lastError = findNode(&root,"IngressCapacityUpdateResponse/ErrorText");
		if( lastError )
		{
			mLastError = lastError->content;
		}
		else
		{
			HTTPLOG(ZQ::common::Log::L_ERROR,CLOGFMT(TransferInitiate,"return code != 200, but missing ErrorText in TransferTerminate"));
		}
		return false;
	}
	else
	{
		return true;
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//SessionStatusQuery
SessionStatusQuery::SessionStatusQuery( CdnSsEnvironment* environment ,const std::string& contextKey,const std::string& clientAddress)
:CDNHttpClient(environment,contextKey)
{
	setLog(NULL);
	mXmlBuilder.create("Status");
	if( !clientAddress.empty() )
	{
		//do not add IncludeAggregate , this will make http server report aggregate automatically
		mXmlBuilder.addParameter( "Client",clientAddress);
	}
}

SessionStatusQuery::~SessionStatusQuery( )
{

}
int32 SessionStatusQuery::invoke( const std::string& uri )
{
	//return sendRequest("c2cp/status");
	return sendRequest(uri);
}

bool SessionStatusQuery::processXML( const std::string& result )
{
	if( !parseResultXml(result) )
		return false;
	const SimpleXMLParser::Node& root = mXmlParser.document();
	if( getStatusCode() != 200 )
	{
		const SimpleXMLParser::Node* lastError = findNode(&root,"StatusResponse/ErrorText");
		if( lastError )
		{
			mLastError = lastError->content;
		}
		else
		{
			HTTPLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SessionStatusQuery,"return code != 200, but missing ErrorText in SessionStatusQuery"));
		}
		return false;
	}
	else
	{
		const SimpleXMLParser::Node* pResponse = findNode( &root , "StatusResponse" );
		if( !pResponse )
		{
			HTTPLOG(ZQ::common::Log::L_ERROR,CLOGFMT(SessionStatusQuery,"return code == 200, but missing StatusResponse"));
			return false;
		}
		else
		{
			SiblingNode nodes = childNodes( pResponse , "Session");
			const SimpleXMLParser::Node* pChildSession = NULL;
			for( pChildSession = nodes.first() ; pChildSession != NULL ; pChildSession = nodes.next() )
			{
				SessionAttr attr;
				if( !getValue( pChildSession , "TransferID" , attr.transferId ) )	continue;
				if( !getValue( pChildSession , "Filename" , attr.fileName))			continue;
				if( !getValue( pChildSession , "ClientTransfer" , attr.clientTransfer )) continue;
				if( !getValue( pChildSession , "TransferAddress" , attr.transferAddress )) continue;
				if( !getValue( pChildSession , "TransferPort" ,attr.transferPort )) continue;
				if( !getValue( pChildSession , "State", attr.transferState )) continue;
				if( !getValue( pChildSession , "TimeInState" , attr.timeInState )) continue;
				if( !getValue( pChildSession , "TransferRate" , attr.transferRate )) continue;
				if( !getValue( pChildSession , "BytesTransferred" , attr.transferBytes )) continue;
				mSessions.push_back(attr); 
			}
		}
		return true;
	}
	return false;
}

const SessionStatusQuery::SessionAttrS&	SessionStatusQuery::getSessions( ) const
{
	return mSessions;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
ResourceStatusQuery::ResourceStatusQuery( CdnSsEnvironment* environment, const std::string& contextKey)
:CDNHttpClient(environment,contextKey)
{
	mXmlBuilder.create("ResourceStatus");	
}
ResourceStatusQuery::~ResourceStatusQuery()
{
}

void ResourceStatusQuery::addPortName(const std::string& portName)
{
	mPortNames.push_back( portName );
}

bool ResourceStatusQuery::getPortAttrs( PortAttrs& attrs ) const
{
	attrs = mPortAttrs;
	return true;
}

int32 ResourceStatusQuery::invoke( const std::string& uri )
{
	std::vector<std::string>::const_iterator it = mPortNames.begin();
	for( ; it != mPortNames.end() ; it ++ )
	{
		mXmlBuilder.addParameter( "PortName" , *it );
	}
	//return sendRequest("c2cp/resourcestatus");
	return sendRequest(uri);
}

bool ResourceStatusQuery::getValue( const SimpleXMLParser::Node* pParent , const std::string& nodeName , std::string& value)
{
	if( !pParent || nodeName.empty() )	return false;
	const SimpleXMLParser::Node* pNode = findNode( pParent , nodeName );
	if( !pNode )
	{
		HTTPLOG(ZQ::common::Log::L_ERROR,CLOGFMT(ResourceStatusQuery,"failed to get node[%s] in [%s]"),nodeName.c_str(), pParent->name.c_str());
		return false;
	}
	value = pNode->content;
	return !value.empty();
}

bool ResourceStatusQuery::processXML(const std::string &result)
{
	if( !parseResultXml(result) )
		return false;
	
	const SimpleXMLParser::Node& root = mXmlParser.document();
	if( getStatusCode() != 200 )
	{
		const SimpleXMLParser::Node* lastError = findNode(&root,"IngressCapacityUpdateResponse/ErrorText");
		if( lastError )
		{
			mLastError = lastError->content;
		}
		else
		{
			HTTPLOG(ZQ::common::Log::L_ERROR,CLOGFMT(ResourceStatusQuery,"return code != 200, but missing ErrorText in TransferTerminate"));
		}
		return false;
	}
	else
	{
		const SimpleXMLParser::Node* resouceStatusResponse = findNode( &root , "ResourceStatusResponse");
		if(!resouceStatusResponse)
		{
			HTTPLOG(ZQ::common::Log::L_ERROR,CLOGFMT(ResourceStatusQuery,"can't find node [%s] in response"),"ResourceStatusResponse");
			return false;
		}
		SiblingNode nodes =  childNodes( resouceStatusResponse , "Port" );
		const SimpleXMLParser::Node* pPortNode = NULL;
		for( pPortNode = nodes.first() ; pPortNode != NULL ; pPortNode = nodes.next() )
		{
			PortAttr attr;
			std::string value;
			if( !getValue( pPortNode , "Name" , value) )	return false;//get port name
			attr.name		=	value;

			if(!getValue( pPortNode , "State" , value))		return false;
			attr.bUp	=	( stricmp(value.c_str() , "up") == 0 );
			if( attr.bUp)
			{
				///get address both ipv4 && ipv6
				SiblingNode addressNodes =  childNodes( pPortNode , "Address" );
				for( const SimpleXMLParser::Node* pAddress = addressNodes.first() ; pAddress != NULL ; pAddress = addressNodes.next() )
				{
					if( pAddress->content.find(":") == std::string::npos )
					{//can't find : , IPV4
						attr.transferAddressIpv4.push_back( pAddress->content );
					}
					else
					{
						attr.transferAddressIpv6.push_back( pAddress->content );
					}
				}

				if(!getValue( pPortNode ,  "Capacity" , value) )	return false;
				sscanf(value.c_str(), FMT64 ,&attr.capacity );
			}		

			if(!getValue( pPortNode , "ActiveTransferCount" , value ) ) return false;
			sscanf(value.c_str(),"%d",&attr.activeTransferCount );
			
			if(!getValue( pPortNode , "ActiveBandwidth" , value)) return false;
			sscanf(value.c_str(), FMT64, &attr.activeBandwidth );

			mPortAttrs.push_back( attr );
		}

		return mPortAttrs.size() > 0 ;
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
CDNHttpHandler::CDNHttpHandler( CdnSsEnvironment* environment  )
:mResponse(NULL), mRequest(NULL), env(environment)
{
	mXmlContent.clear();
	mLogger = &environment->getMainLogger();
}

CDNHttpHandler::~CDNHttpHandler()
{
}

void CDNHttpHandler::generateReponseHeader( int code , size_t contentLength , const char* reason  )
{
	mResponse->setStatus( code , reason );
	mResponse->setHeader( "Server","C2Streamer/1.0 SeaChange International");
	mResponse->setHeader( "Cache-Control" ,"no-cache" );
	mResponse->setHeader( "Content-Type","text/xml");
	std::ostringstream oss;
	oss<<contentLength;
	mResponse->setHeader( "Content-Length", oss.str().c_str() );
	mResponse->headerPrepared();
}

bool CDNHttpHandler::onRequest(const ZQHttp::IRequest& req, ZQHttp::IResponse& resp) 
{
	mResponse	= &resp;
	mRequest	= &req;
	
	return true;

}
bool CDNHttpHandler::onPostData(const ZQHttp::PostDataFrag& frag) 
{
	mXmlContent.append( frag.data , frag.len );
	return true;
}
bool CDNHttpHandler::onPostDataEnd() 
{
	assert( mResponse != NULL );
	assert( mRequest != NULL );

	//parse the xml
	try
	{
#if defined _DEBUG || defined DEBUG
		printf("\n*************************************\n");
		printf(mXmlContent.c_str());
#endif
		mXmlParser.parse(mXmlContent.c_str() , static_cast<int>(mXmlContent.length()) , 1);
	}
	catch( ZQ::common::ExpatException& ex)
	{
		HTTPLOG(ZQ::common::Log::L_ERROR,CLOGFMT(CDNHttpHandler,"failed to parse xml result because [%s]"),
			ex.what() );
		return true;
	}

	run();
	
	return true;
}
void CDNHttpHandler::onRequestEnd()
{
	assert( mResponse != NULL );
	assert( mRequest != NULL );

}
void CDNHttpHandler::onBreak() 
{
}

bool getValueEx( const SimpleXMLParser::Node* pParent , const std::string& nodeName , std::string& value)
{
	if( !pParent || nodeName.empty() )	return false;
	const SimpleXMLParser::Node* pNode = findNode( pParent , nodeName );
	if( !pNode )
	{
		//HTTPLOG(ZQ::common::Log::L_ERROR,CLOGFMT(CDNHttpHandler,"failed to get node[%s] in [%s]"),nodeName.c_str(), pParent->name.c_str());
		return false;
	}
	value = pNode->content;
	return !value.empty();
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
	if(!getValue(pParent , nodeName, result))	return false;
	sscanf(result.c_str(), FMT64, &value);
	return true;
}
bool getValue( const SimpleXMLParser::Node* pParent , const std::string& nodeName , float& value)
{
	std::string result;
	if(!getValue(pParent , nodeName, result))	return false;
	sscanf(result.c_str(),"%f",&value);
	return true;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
TransferUpdateState::TransferUpdateState( CdnSsEnvironment* environment  )
:CDNHttpHandler(environment)
{

}
TransferUpdateState::~TransferUpdateState( )
{

}
bool TransferUpdateState::run( )
{
	const SimpleXMLParser::Node& root = mXmlParser.document();
	const SimpleXMLParser::Node* pTransferStateUpdate = findNode( &root, "TransferStateUpdate");
	if( !pTransferStateUpdate )
	{
		HTTPLOG(ZQ::common::Log::L_ERROR,CLOGFMT(TransferUpdateState,"Node [%s] is not found"),"TransferStateUpdate");
		return false;
	}
	SiblingNode nodes = childNodes( pTransferStateUpdate , "Transfer");
	const SimpleXMLParser::Node* pNode = NULL;
	for( pNode = nodes.first() ; pNode != NULL ; pNode = nodes.next() )
	{
		std::string clientAddress;
		std::string transferId;
		std::string state;
		///get client
		if(!getValue(pNode , "ClientTransfer" , clientAddress))	continue;
		if(!getValue(pNode , "TransferID" , transferId )) continue;
		if(!getValue(pNode , "State" , state)) continue;
		
		TransferSessionStateUpdateEvent(env).post( clientAddress , transferId , state );
	}
	
	CDNMiniXmlBuilder builder;
	builder.create("TransferStateUpdateResponse");
	std::string xml =builder.exportXml();
	generateReponseHeader(200,xml.length());
	mResponse->addContent(xml.c_str() , xml.length() );
	mResponse->complete();

	return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//TranferUpdateResource
TranferUpdateResource::TranferUpdateResource( CdnSsEnvironment* environment )
:CDNHttpHandler(environment)
{

}
TranferUpdateResource::~TranferUpdateResource()
{
	
}

bool TranferUpdateResource::run( )
{
	const SimpleXMLParser::Node& root = mXmlParser.document();
	const SimpleXMLParser::Node* pTransferResourceUpdate = findNode( &root, "TransferResourceUpdate");
	if( !pTransferResourceUpdate )
	{
		HTTPLOG(ZQ::common::Log::L_ERROR,CLOGFMT(TransferUpdateState,"Node [%s] is not found"),"TransferResourceUpdate");
		return false;
	}
	SiblingNode nodes = childNodes( pTransferResourceUpdate , "Port");
	const SimpleXMLParser::Node* pNode = NULL;
	for( pNode = nodes.first() ; pNode != NULL ; pNode = nodes.next() )
	{
		CdnStreamerManager::StreamerAttr attr;
		std::string value;
		if(!getValue(pNode , "Name" , attr.portName	))	continue;
		{
			SiblingNode addressNodes = childNodes( pNode , "Address");
			for( const SimpleXMLParser::Node* pAddress = addressNodes.first() ; pAddress != NULL ; pAddress = addressNodes.next() )
			{
				if( pAddress->content.find(":") == std::string::npos )
				{//IPv4
					attr.transferAddressIpv4.push_back(pAddress->content);
				}
				else
				{//Ipv6
					attr.transferAddressIpv6.push_back(pAddress->content);
				}
			}
		}
		if(!getValue(pNode , "Capacity" , attr.capacity )) continue;
		if(!getValue(pNode , "State" , value ))	continue;
		attr.bUp = (stricmp(value.c_str() , "up") == 0 );
		if(!getValue(pNode , "ActiveTransferCount" , attr.activeTransferCount) ) continue;
		if(!getValue(pNode , "ActiveBandwidth", attr.activeBandwidth)) continue;

		CdnStreamerManager& manager = env->getStreamerManager();
		manager.reportStreamerState(attr);
	}
	
	CDNMiniXmlBuilder builder;
	builder.create("ResourceStatusResponse");
	std::string xml =builder.exportXml();
	generateReponseHeader(200,xml.length());
	mResponse->addContent(xml.c_str() , xml.length() );
	mResponse->complete();

	return true;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//TranferUpdateIngressCapacity
TranferUpdateIngressCapacity::TranferUpdateIngressCapacity( CdnSsEnvironment* environment )
:CDNHttpHandler(environment)
{
}

TranferUpdateIngressCapacity::~TranferUpdateIngressCapacity( )
{

}
bool TranferUpdateIngressCapacity::run( )
{
	const SimpleXMLParser::Node& root = mXmlParser.document();
	const SimpleXMLParser::Node* pIngressCapacityUpdate = findNode( &root, "TransferIngressCapacityUpdate");
	if( !pIngressCapacityUpdate )
	{
		HTTPLOG(ZQ::common::Log::L_ERROR,CLOGFMT(TransferUpdateState,"Node [%s] is not found"),"IngressCapacityUpdate");
		return false;
	}

	std::string client;
	int64		ingressCapacity = 0;
	
	if(!getValue(pIngressCapacityUpdate , "ClientTransfer" , client	))	return false;
	if(!getValue(pIngressCapacityUpdate , "IngressCapacity" , ingressCapacity )) return false;
	
	IngressCapacityUpdateEvent(env).post( client , ingressCapacity );

	
	CDNMiniXmlBuilder builder;
	builder.create("TransferIngressCapacityUpdateResponse");
#pragma message(__MSGLOC__"TODO:Should I add the sub node to comfirm the IngressCapacity?")
	std::string xml =builder.exportXml();
	generateReponseHeader(200,xml.length());
	mResponse->addContent(xml.c_str() , xml.length() );
	mResponse->complete();

	return true;
}

}}
