
#include <assert.h>
#include "SessionDataRecorder.h"

#define RECORD_BUFFER_SIZE	( 16 * 1024 )

SessionDataRecorder::SessionDataRecorder( )
{
}

SessionDataRecorder::~SessionDataRecorder( )
{
	clearResource();
}

void SessionDataRecorder::clearResource( )
{
	RECORDS recs;
	{
		ZQ::common::MutexGuard gd(mMutex);
		recs = mRecord;
	}
	RECORDS::iterator it = recs.begin();
	for(  ; it != recs.end() ; it ++ )
	{
		it->second->destroy();
	}
}

DataRecord* SessionDataRecorder::createDataRecord( const std::string& sessId , int64 transferStartTime  )
{
	DataRecord* p = new DataRecord(*this);
	assert(p != NULL );
	p->create( sessId , transferStartTime );
	
	{
		ZQ::common::MutexGuard gd(mMutex);
		mRecord.insert( RECORDS::value_type(sessId , p ) );
	}
	return p;
}

void SessionDataRecorder::destroyDataRecord( const std::string& sessId )
{
	DataRecord* p = NULL ;
	{
		ZQ::common::MutexGuard gd(mMutex);
		RECORDS::iterator it = mRecord.find(sessId);
		if( it != mRecord.end() )
			p = it->second;
	}
	if( p )
	{
		p->destroy();
	}
}
void* SessionDataRecorder::allocMemory( )
{
	void* p = malloc( RECORD_BUFFER_SIZE );
	assert( p );
	return p;
}

void SessionDataRecorder::freeMemory(  void* mem )
{
	free(mem);
}
SessionDataRecorder::RECORDS& SessionDataRecorder::getAllRecords()
{
	ZQ::common::MutexGuard gd(mMutex);
	return mRecord;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////

DataRecord::RecordInfo::RecordInfo()
{
	init();
};

void DataRecord::RecordInfo::init()
{
	hashCode	= 0;
	commitSize	= RECORD_BUFFER_SIZE - sizeof(RecordInfo);
	usedSize	= 0;			
	next		= NULL;
	buffer		= (char*)this + sizeof(RecordInfo);
}

size_t DataRecord::RecordInfo::availSize( ) const
{
	return commitSize - usedSize;
}

size_t	DataRecord::RecordInfo::dataSize( ) const
{
	return usedSize;
}

void DataRecord::RecordInfo::putData( const char* data , size_t size )
{
	if( availSize() < size )
	{
		assert( false );
		return;
	}	
	memcpy( buffer + usedSize,  data , size );
	usedSize += size;
}

//////////////////////////////////////////////////////////////////////////
DataRecord::DataRecord( SessionDataRecorder& recorder )
:mRecorder(recorder)
{
}

DataRecord::~DataRecord( )
{
	clearResource();
}

DataRecord::RecordInfo*	DataRecord::getDataHeader( )
{
	return mHeader.next;
}
void DataRecord::clearResource()
{
	//walk through the link and release buffer
	RecordInfo* p = mHeader.next;
	while( p )
	{
		RecordInfo* next = p->next;
		mRecorder.freeMemory( (char*)p );		
	}
	mHeader.next = NULL;
	mCurrentRecord = NULL;
}

void DataRecord::destroy()
{
	clearResource();
}

void DataRecord::create( const std::string& sessionId , int64 dataTransferStartTime )
{
	destroy();
	mHeader.sessionId[sizeof(mHeader.sessionId) - 1 ] = 0;
	strncpy( mHeader.sessionId , sessionId.c_str() , MIN(SESSIONID_BUF_SIZE , sessionId.length() ) );
	mHeader.dataTransferStartTime = dataTransferStartTime;
}

DataRecord::RecordInfo* DataRecord::selectRecordInfo( size_t size )
{
	if( !mCurrentRecord || (mCurrentRecord->availSize() < size ) )
	{
		//create a new one
		RecordInfo* p = (RecordInfo*)mRecorder.allocMemory();
		assert( p != NULL );
		p->init();
		if( mCurrentRecord == NULL )
		{
			mHeader.next = p;
			mCurrentRecord = p;
		}
		else
		{
			mCurrentRecord->next = p;
			mCurrentRecord = p;
		}
	}
	return mCurrentRecord;
}
void DataRecord::put( const char* data , size_t size )
{
	RecordInfo* p = selectRecordInfo( size );
	p->putData( data , size );
}

void DataRecord::put( const HttpSessionDataRecordInfo& r)
{
	if( r.size <= 0 )
		return;
	const char* p = (const char*)&r;
	put( p , sizeof(r) );
}
