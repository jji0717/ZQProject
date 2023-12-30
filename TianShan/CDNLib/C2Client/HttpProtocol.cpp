
#include "HttpProtocol.h"
#include <sstream>
#include <strHelper.h>

HttpMessage::HttpMessage(void)
:
mParseStage(HTTP_PARSE_STAGE_NULL),
mbChunkedData(false),
mBodyContentSize(0),
mCurrentUsedBodyContentSize(0),
mChunkState(CHUNK_STATE_HEADER)
{
}

HttpMessage::~HttpMessage(void)
{
}

std::string HttpMessage::getUrl() const
{
	return mUrl;
}

std::string HttpMessage::getMethod() const
{
	return mMethod.toString();
}

std::string HttpMessage::getProtocol() const
{
	return mProto.toString();
}

std::string HttpMessage::getHeader(  const std::string& key ) const
{
	HEADERKVMAP::const_iterator it = mHeaders.find(key);
	if( it == mHeaders.end() )
		return std::string();
	else
		return it->second;
}
void HttpMessage::updateMethod( const std::string& methodStr )
{
	mMethod.fromString(methodStr);
}
void HttpMessage::updateMethod( HttpMethod m )
{
	mMethod = m;
}
// void HttpMessage::updateUrl( const ZQ::common::URLStr& url )
// {	
// 	assert(false);//not implement yet
// }
void HttpMessage::updateUrl( const std::string& url )
{
	mUrl = url;
}

void HttpMessage::updateProtocol( const std::string& protocol )
{
	mProto.fromString(protocol);
}
void HttpMessage::updateProtocol( HttpProto p )
{
	mProto = p;
}

#define CONTENT_LENGTH_KEY "Content-Length"
#define TRANSFER_CODING "transfer-encoding"
void HttpMessage::updateHeader( const std::string& key , const std::string& value )
{
	if(key.empty()) return;
	if(value.empty())
	{
		mHeaders.erase( key );
	}
	else
	{
		mHeaders[key] = value;
	}
	std::string tmpKey = key;
	ZQ::common::stringHelper::TrimExtra(tmpKey);
	if( stricmp(CONTENT_LENGTH_KEY,key.c_str()) == 0 )
	{
		sscanf( value.c_str(),"%lld", &mBodyContentSize);
	}
	else if( stricmp( TRANSFER_CODING,  key.c_str() ) == 0 )
	{
		std::string tmpValue = value;
		ZQ::common::stringHelper::TrimExtra(tmpValue);
		mbChunkedData = stricmp(tmpValue.c_str() , "chunked") == 0;
	}
}

bool HttpMessage::addContentData( const char*& data , size_t& size , BodyContentDataEvent sender)
{
	bool bComplete = false;
	if( mbChunkedData )
	{
		char szBuf[128];
		sprintf( szBuf , "%x\r\n" , size );
		sender( szBuf , strlen(szBuf) , false );
		if( size > 0 )
		{
			sender( data , size ,false );
		}
		else
		{
			sender( "\r\n" , 2 , true );
			bComplete = true;
		}
	}
	else
	{
		size_t delta = size ;
		if( (size + mCurrentUsedBodyContentSize) >= mBodyContentSize )
		{
			delta = (size_t)(mBodyContentSize - mCurrentUsedBodyContentSize);
			bComplete = true;
		}
		sender( data , delta , bComplete );
		data += delta;
		size -= (size_t)delta;
	}
	if( bComplete )
	{
		mParseStage = HTTP_PARSE_STAGE_BODY;//message construct completely
	}
	return true;
}

size_t findLineTerm( const char* data ,size_t size )
{
	size_t pos = 0 ;
	while( (pos + 1)< size )
	{
		if( memcmp(data+pos , "\r\n" , 2 ) == 0  )
			return (pos + 2);
		++pos;
	}
	return size_t(-1);
}

bool HttpMessage::getContentData( const char*& data , size_t& size , BodyContentDataEvent receiver )
{
	bool bComplete = false;
	
	if( mbChunkedData )
	{
		while( size > 0 )
		{
			size_t delta = 0;
			switch(mChunkState)
			{
			case CHUNK_STATE_HEADER:
				{//find header
					size_t nextPos = findLineTerm( data , MIN(128,size) );
					if( nextPos == size_t(-1) )
						return false;//not enough data

					size_t chunkSize = 0;
					sscanf( data , "%x", &chunkSize );

					if( chunkSize <= 0 )
					{						
						size -= (nextPos + 2 );
						data += (nextPos + 2 );
						mChunkState = CHUNK_STATE_TAIL;
						mParseStage = HTTP_PARSE_STAGE_BODY;
						bComplete = true;						
					}
					else
					{
						mChunkState = CHUNK_STATE_BODY;
						size -= nextPos;
						data += nextPos;						
						mBodyContentSize = chunkSize;
						continue;
					}					
				}
				break;
			case CHUNK_STATE_BODY:
				{
					if( mBodyContentSize <= 0)
					{
						mChunkState = CHUNK_STATE_TAIL;
						continue;
					}
					delta = (size_t)MIN( (int64)size , mBodyContentSize );
					mBodyContentSize -= delta;
				}
				break;
			case CHUNK_STATE_TAIL:
				{
					size_t nextPos = findLineTerm( data , MIN(128,size) );
					if( nextPos == size_t(-1) )
						return false;//not enough data					
					size -= nextPos;
					data += nextPos;
					delta = 0;
					mChunkState = CHUNK_STATE_HEADER;
					continue;
				}
				break;
			default:
				assert(false);
				break;
			}		
			
			receiver( data , delta , bComplete);			

			data += delta;
			size -= delta;	
			if( bComplete )
				return true;
		}
	}
	else
	{
		size_t delta = size;
		if( (mCurrentUsedBodyContentSize + delta) >= mBodyContentSize )
		{
			delta = (size_t)( mBodyContentSize - mCurrentUsedBodyContentSize );
			mParseStage = HTTP_PARSE_STAGE_BODY;
			bComplete = true;
		}		
		receiver( data , delta , bComplete );
		data += delta;
		size -= delta;
	}
	return true;
}
#define LINETERM "\r\n"
std::string HttpMessage::toString( )
{
	std::ostringstream oss;
	oss << mMethod.toString() << " " << mUrl << " " << mProto.toString() << LINETERM;
	HEADERKVMAP::const_iterator itHeader = mHeaders.begin();
	while( itHeader != mHeaders.end() )
	{
		oss << itHeader->first << ": "<< itHeader->second << LINETERM;
		itHeader++;
	}

	oss << LINETERM;

	return oss.str();
}
