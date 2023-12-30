

#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include "HttpDialog.h"

#include <strHelper.h>
#include <assert.h>

HttpDialog::HttpDialog( HttpSessionFactoryPtr fac )
:mReservedMsg(NULL),
mSessionFactory(fac)
{
}

HttpDialog::~HttpDialog(void)
{
}

SimpleHttpSessionPtr HttpDialog::getSession()
{
	assert( mSessionFactory != NULL );
	if( !mSession )
	{
		mSession = mSessionFactory->createSession();
		mSession->onCreated( mComm );
	}
	assert( mSession != NULL );
	return mSession;

}

void HttpDialog::onCommunicatorSetup( ZQ::DataPostHouse::IDataCommunicatorPtr communicator )
{
	mComm = communicator;	
}

void HttpDialog::onCommunicatorDestroyed( ZQ::DataPostHouse::IDataCommunicatorPtr communicator ) 
{
	if(mSession)
	{
		mSession->onMessageComplete();
	}
	mComm = NULL;
}

bool HttpDialog::onRead( const int8* buffer , size_t bufSize ) 
{
	while( bufSize > 0 )
	{
		parseHttpMessage( buffer , bufSize );
	}
	return true;
}

void HttpDialog::onWritten( size_t bufSize ) 
{
	///FIXME: implement later
}

void HttpDialog::onError( ) 
{
	///FIXME: implement later
}


char*		getLine( const char*& buf , size_t& bufSize , size_t& pos ) 
{	

	const char* p			= buf;
	const char* pStartPos = p;
	pos = 0;
	while( *p !='\n' && pos < bufSize )
	{
		++pos;
		++p;
	}

	if( *p != '\n' )
	{
		return NULL;
	}

	buf			+=	( pos + 1 );
	bufSize		-=	( pos + 1 );
	return (char*)pStartPos;	
}

bool			getHeaderKeyValue(  char*& buf , size_t& bufSize , char*& pKey ,size_t& keySize , char*& pValue , size_t& valueSize )
{	
	char* p	= reinterpret_cast<char*>(buf);
	char* pEnd	= p + bufSize;
	pKey = p;
	//step 1 , find ':'
	size_t	pos			= 0;
	bool	bFoundKey	= false;
	bool	bGetKeySize = false;

	while( p < pEnd && *p != ':' )
	{
		if( !bFoundKey && *p == ' ')
		{
			++pKey;
		}
		else
		{
			bFoundKey = true;
			if( *p == ' ' )
			{
				bGetKeySize = true;
				keySize = p - pKey;
			}
		}
		++p;		
	}
	if( !bGetKeySize )
	{
		keySize = p - pKey;
	}

	if( p == pEnd )
		return false;	

	if( ++p  > pEnd )
		return false;

	pValue		= p ;

	//find the end of value , that should be the \r\n
	if( *(pEnd - 1) =='\r' )
		p = pEnd - 1;
	else 
		p = pEnd ;

	valueSize	= p > pValue  ? (p - pValue ) : 0 ;
	return true;
}

void HttpDialog::findStartline( const char*& data , size_t& size )
{
	HttpMessagePtr p = NULL;
	assert( mReservedMsg != NULL );	
	p = mReservedMsg;

	size_t lineSize = 0;
	const char* pLine = getLine( data , size , lineSize );
	
	if( !pLine )
		return;		
	
	std::string startline;
	if( mReservedData.empty() )
	{
		startline.assign(pLine,lineSize);
	}
	else
	{
		startline	= mReservedData + std::string( pLine , lineSize );
		mReservedData = "";
	}
	std::vector<std::string> tmp;
	ZQ::common::stringHelper::SplitString( startline , tmp, " " , " \r\n");
	if( tmp.size() < 3 )
		return;
	p->updateMethod(tmp[0]);
	p->updateUrl(tmp[1]);
	p->updateProtocol(tmp[2]);
	p->mParseStage	=	HttpMessage::HTTP_PARSE_STAGE_STARTLINE;	
}

void HttpDialog::findHeaders( const char*& data , size_t& size )
{
	HttpMessagePtr p = mReservedMsg;
	assert( p != NULL );
	size_t lineSize = 0;
	if( !mReservedData.empty() )
	{
		const char* pLine = getLine( data , size , lineSize );
		if( pLine )
		{
			if( size <= 1 )
			{//empty line
				p->mParseStage	=	HttpMessage::HTTP_PARSE_STAGE_HEADER;
				return;
			}
			else
			{
				mReservedData.append( pLine , lineSize );
				size_t sz = mReservedData.size();
				char* pString = (char*)mReservedData.c_str();
				char* pKey		=	NULL;
				size_t	keySize			=	0;
				char* pValue	=	NULL;
				size_t	valueSize		=	0;
				if( getHeaderKeyValue(pString , sz , pKey , keySize , pValue , valueSize ))
				{				
					p->updateHeader( std::string(pKey,keySize) , std::string(pValue,valueSize) );
				}
				else
				{//just ignore the invalid header line					
				}
				mReservedData = "";
			}
		}
	}	
	unsigned char* pKey		=	NULL;
	size_t	keySize			=	0;
	unsigned char* pValue	=	NULL;
	size_t	valueSize		=	0;
	do 
	{
		size_t lineSize = 0;
		char* pLine = getLine( data , size , lineSize );
		if ( pLine )
		{
			if( lineSize <= 1 )
			{
				p->mParseStage	=	HttpMessage::HTTP_PARSE_STAGE_HEADER;
				return ;
			}
			else
			{		
				char* pKey		=	NULL;
				size_t	keySize			=	0;
				char* pValue	=	NULL;
				size_t	valueSize		=	0;
				if( getHeaderKeyValue( pLine , lineSize , pKey , keySize , pValue , valueSize ))
				{
					p->updateHeader( std::string(pKey,keySize) , std::string(pValue,valueSize) );
				}
				else
				{//just ignore the invalid header line
					//return false;
				}
			}
		}
		else
		{
			break;
		}

	} while ( true );
}

void HttpDialog::parseHttpMessage( const char*& data , size_t& size )
{
	HttpMessagePtr p = NULL;
	if( !mReservedMsg )
	{
		mReservedMsg = new HttpMessage();
		assert( mReservedMsg != NULL );
	}
	p = mReservedMsg;
	
	while( size > 0 )
	{
		HttpMessage::MessageParseStage lastStage = p->mParseStage;
		switch ( p->mParseStage )
		{
		case HttpMessage::HTTP_PARSE_STAGE_NULL:
			{
				findStartline( data , size );
			}
			break;
		case HttpMessage::HTTP_PARSE_STAGE_STARTLINE:
			{
				findHeaders(  data , size );
				if( p->mParseStage == HttpMessage::HTTP_PARSE_STAGE_HEADER )
				{//header parsed
					getSession()->onHeadersComplete( p );
				}
			}
			break;
		case HttpMessage::HTTP_PARSE_STAGE_HEADER:
			{//tring to parse the content data
				//receiver( data , delta , false );
				if(mSession)
				{
					mSession->onBodyContent(data,size,false);
					size = 0;
				}
				break;

				{
					if( mReservedData.size() > 0 && size > 0 )
					{
						size_t delta = MIN(128,size);
						data += delta;
						size -= delta;
						mReservedData.append(data,delta);
						if( mReservedData.find("\r\n") == std::string::npos )
						{
							if( mReservedData.size() >= 128 )
							{
								if( mSession)
								{
									mSession->onBadBodyContent( data , size );
								}
								mReservedData.clear();
							}
						}
						else
						{
							const char* rdata =mReservedData.c_str();
							size_t rsize = mReservedData.length();
							if( !p->getContentData( rdata , rsize , boost::bind(&SimpleHttpSession::onBodyContent, mSession.get(),_1,_2,_3 ) ) )
							{
								mReservedData.append(rdata,rsize);
								if(mReservedData.size() > 256 * 1024)
								{
									if(mSession)
										mSession->onBadBodyContent(rdata,0);
									mReservedData.clear();
								}
								break;
							}
						}
					}
					if( !p->getContentData( data , size , boost::bind(&SimpleHttpSession::onBodyContent, mSession.get(),_1,_2,_3 ) ) )
					{					
						if( size > 0 )
						{
							if( size >= 128 )
							{
								if( mSession)
								{
									mSession->onBadBodyContent( data , size );
								}
								//wrong format, consume all data
								data += size;
								size = 0;
							}
							else
							{
								mReservedData.append(data,size);
							}
						}
					}
				}
			}
			break;
		case HttpMessage::HTTP_PARSE_STAGE_BODY:
			{
				mReservedMsg = NULL;
				return;
			}
			break;
		default:
			{
				assert(false);
				return ;
			}
		}
		if( (p->mParseStage <= lastStage) && (p->mParseStage < HttpMessage::HTTP_PARSE_STAGE_HEADER) )
		{
			mReservedData.append(data,size);
			return ;
		}
	}
}

