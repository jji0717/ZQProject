#ifndef __c2client_session_data_record_apple_header_file__
#define __c2client_session_data_record_apple_header_file__

#include <ZQ_common_conf.h>
#include <Locks.h>
#include <map>



/*
  In order to save session record efficiently	, I am planing to make hash table.
  I put the DataRecord to hash table so that I can find the DataRecord instance through the hash value.
  In DataRecord, there are twp important concept, one is Record header and the other is Record Info
  In Record header, session Id and current data record hash code will be added so that we can use session Id to find the 
  data record information
*/

struct HttpSessionDataRecordInfo 
{
	int64	time;
	size_t	size;//data size
	size_t	holder;//not used
};

#define SESSIONID_BUF_SIZE 256

class SessionDataRecorder;
class DataRecord
{
public:
	DataRecord( SessionDataRecorder& recorder);
	virtual ~DataRecord();
	

public:	

	void		create( const std::string& sessionId , int64 dataTransferStartTime );

	void		destroy( );
	
	void		put( const HttpSessionDataRecordInfo& r);

	void		put( const char* data , size_t size );

	struct RecordInfo
	{
		RecordInfo*	next;
		size_t		hashCode;
		size_t		commitSize;
		size_t		usedSize;
		size_t		holder;
		char*		buffer;		
		RecordInfo();
		void	init();
		size_t	availSize( ) const;
		void	putData( const char* data , size_t size );
		size_t	dataSize( ) const;
	};
	struct  RecordHeader
	{
		char		sessionId[SESSIONID_BUF_SIZE];		
		int64		dataTransferStartTime;
		size_t		hashCode;
		RecordInfo*	next;
		RecordHeader()
		{
			memset(sessionId,0,sizeof(sessionId));
			dataTransferStartTime	= 0;
			hashCode				= 0;
			next					= 0;
		}
	};

	RecordInfo*	getDataHeader( );

	const RecordHeader& getRecordHeader() const
	{
		return mHeader;
	}

protected:

	void		clearResource( );

	RecordInfo* selectRecordInfo( size_t size );

	void		putData( const char* data , size_t size );

private:



	SessionDataRecorder&		mRecorder;
	RecordHeader				mHeader;
	RecordInfo*					mCurrentRecord;
};

class SessionDataRecorder
{
public:
	SessionDataRecorder( );
	virtual ~SessionDataRecorder( );

public:

	typedef std::map< std::string , DataRecord*> RECORDS;

	RECORDS&		getAllRecords();

	DataRecord*		createDataRecord( const std::string& sessId , int64 transferStartTime  );

	void			destroyDataRecord( const std::string& sessId );
	
// 	bool			fromDBFile( const std::string& dbfilePath );
// 
// 	bool			toDBFile( const std::string& dbFilePath ) const;

protected:
	///allocate fix size memory
	void*			allocMemory( );

	void			freeMemory(  void* mem );

	friend class DataRecord;

	void			clearResource( );

private:
	
	RECORDS					mRecord;
	ZQ::common::Mutex		mMutex;
};



#endif//__c2client_session_data_record_apple_header_file__
