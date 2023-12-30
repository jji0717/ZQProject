
#ifndef _VSTRM_INDEX_FILE_PARSER_HEADER_FILE_H__
#define _VSTRM_INDEX_FILE_PARSER_HEADER_FILE_H__

#include <string>
#include <vector>
#include <Locks.h>
#include "IdxFileParserEnvironment.h"


namespace ZQ
{
namespace IdxParser
{
const int32 errorCodeOK                     = 200;
const int32 errorCodeHeadError              = -4;
const int32 errorCodeFailToOpenFile         = -5;
const int32 errorCodeFailToReadIndex        = -6;	
const int32 errorCodeFailToReadContent      = -7;	
const int32 errorCodeSkipFileHeader         = -8;
const int32 errorCodeFailToSeek             = -9;
const int32 errorCodeExceed                 = -10;
const int32 errorCodeNotFull                = -11;
const int32 errorCodeEOF 					= -12;

class IndexData
{
	friend class IndexFileParser;
	friend class CsicoIndexFileParser;
public:
	IndexData( )
	{
		bLocal				=	true;
		hasExtensionData	=	false;
		mainFileSize		=	0;
		mainFilePlayTime	=	0;
		muxBitRate			=	0;
		videoBitRate		=	0;
		audioBitRate		=	0;
		horizonResolution	=	0;
		verticalResolution	=	0;
		subFileCount		=	0;
		frameRateCode		=	VMPEG_FRAME_RATE_NULL;
		pwe 				=	false;
		subFileInfos.clear();

		zeroPFrameLength 	= 	0;
		zeroBFrameLength	= 	0;
	}
	IndexData( const IndexData& d )
	{
		mainFilePlayTime	=	d.mainFilePlayTime;
		muxBitRate			=	d.muxBitRate;
		videoBitRate		=	d.videoBitRate;
		audioBitRate		=	d.audioBitRate;
		horizonResolution	=	d.horizonResolution;
		verticalResolution	=	d.verticalResolution;
		subFileCount		=	d.subFileCount;
		frameRateCode		=	d.frameRateCode;
		subFileInfos		=	d.subFileInfos;
		hasExtensionData	=	d.hasExtensionData;
		mainFileSize		=	d.mainFileSize;
		pwe					=	d.pwe;

		zeroPFrameLength 	= 	d.zeroBFrameLength;
		zeroBFrameLength 	= 	d.zeroPFrameLength;
	}
	IndexData& operator=( const IndexData& d  ) 
	{
		mainFilePlayTime	=	d.mainFilePlayTime;
		muxBitRate			=	d.muxBitRate;
		videoBitRate		=	d.videoBitRate;
		audioBitRate		=	d.audioBitRate;
		horizonResolution	=	d.horizonResolution;
		verticalResolution	=	d.verticalResolution;
		subFileCount		=	d.subFileCount;
		frameRateCode		=	d.frameRateCode;
		subFileInfos		=	d.subFileInfos;
		hasExtensionData	=	d.hasExtensionData;
		mainFileSize		=	d.mainFileSize;
		pwe					=	d.pwe;

		zeroPFrameLength 	= 	d.zeroBFrameLength;
		zeroBFrameLength 	= 	d.zeroPFrameLength;

		return *this;
	}
	~IndexData( )
	{

	}

	enum IndexType
	{
		INDEX_TYPE_VVX,
		INDEX_TYPE_VV2,
		INDEX_TYPE_VVC
	};
	
	enum FrameRate
	{
		VMPEG_FRAME_RATE_NULL	= 0,
		VMPEG_FRAME_RATE_23_976 = 0x01,
		VMPEG_FRAME_RATE_24,
		VMPEG_FRAME_RATE_25,
		VMPEG_FRAME_RATE_29_97,
		VMPEG_FRAME_RATE_30,
		VMPEG_FRAME_RATE_50,
		VMPEG_FRAME_RATE_59_94,
		VMPEG_FRAME_RATE_60
	};

	enum StreamType
	{
		MPEG_STREAM_TYPE_UNKNOWN			= 0,
		MPEG_STREAM_TYPE_MPEG2_TRANSPORT,
		MPEG_STREAM_TYPE_ES_STREAM,
	};
	

public:
	
	typedef std::vector<std::string> STRINGSET;

	typedef struct _SubFileInformation
	{
		std::string				fileExtension;
		SPEED_IND				speed;
		uint32					playtime;
		uint64					startingByte;
		uint64					endingByte;
		uint64					fileSize;//not data size, in some situation file size >= datasizea
		uint8					subFileIdx;
		_SubFileInformation()
		{
			speed.denominator	= 0;
			speed.numerator		= 0;
			playtime			= 0;
			startingByte		= 0;
			endingByte			= 0;
			fileSize			= 0;
			subFileIdx			= 0;
		}
	}SubFileInformation;
	
	bool						isLocalContent( ) const
	{
		return bLocal;
	}

	uint32						getPlayTime( ) const
	{
		return mainFilePlayTime;
	}

	int32						getSubFileCount( ) const
	{
		return static_cast<int32>(subFileInfos.size());
	}

	uint32						getMuxBitrate( ) const
	{
		return muxBitRate;
	}

	uint32						getVideoHorizontalSize( ) const
	{
		return horizonResolution;
	}

	uint32						getVideoVerticalSize( ) const
	{
		return verticalResolution;
	}

	unsigned const char*        getZeroMotionBFrame(uint16& len) const
	{
		len = zeroBFrameLength;
		return zeroBFrame;
	}

	unsigned const char*        getZeroMotionPFrame(uint16& len) const
	{
		len = zeroPFrameLength;
		return zeroPFrame;
	}

	SPEED_IND					getSubFileSpeed( int index ) const
	{
		SPEED_IND speed;
		speed.denominator = speed.numerator = 0;
		if(index<0 || index >= (int)subFileInfos.size() )
			return speed;
		const SubFileInformation& info = subFileInfos[index];
		speed.denominator = info.speed.denominator;
		speed.numerator = info.speed.numerator;
		return speed;
	}

	FrameRate					getFrameRate( ) const
	{
		return frameRateCode;
	}

	const char*					getIndexTypeString( IndexType type ) const
	{
		switch ( type )
		{
		case INDEX_TYPE_VV2:
			return "INDEX_TYPE_VV2";
		case INDEX_TYPE_VVX:
			return "INDEX_TYPE_VVX";
		case INDEX_TYPE_VVC:
			return "INDEX_TYPE_VVC";
		default:
			return "UNKNOWN";
		}
	}

	const char*					getFrameRateString( ) const
	{
		const char * szReturn;
		switch (frameRateCode)
		{
		case VMPEG_FRAME_RATE_23_976:	szReturn = "23.976";break;
		case VMPEG_FRAME_RATE_24:		szReturn = "24";break;
		case VMPEG_FRAME_RATE_25:		szReturn = "25";break;
		case VMPEG_FRAME_RATE_29_97:	szReturn = "29.97";break;
		case VMPEG_FRAME_RATE_30:		szReturn = "30";break;
		case VMPEG_FRAME_RATE_50:		szReturn = "50";break;
		case VMPEG_FRAME_RATE_59_94:	szReturn = "59.94";break;
		case VMPEG_FRAME_RATE_60:		szReturn = "60";break;
		default:						szReturn = "";break;
		}

		return szReturn;
	}
	
	bool						getSubFileInfo( size_t idx , SubFileInformation& info ) const
	{
		if( idx >= subFileInfos.size() )
			return false;
		info = subFileInfos[idx];
		return true;
	}

	const std::string			getSubFileName( int index ) const
	{
		if (index<0 || index>= (int)subFileInfos.size())
		{
			return std::string("");
		}
		return subFileInfos[index].fileExtension;
	}

	IndexType					getIndexType( ) const
	{
		return indexDataType;
	}

	int64						getMainFileSize( ) const
	{
		return mainFileSize;
	}

	bool						hasExtension( ) const
	{
		return hasExtensionData;
	}

	bool						isPwe( ) const {
		return pwe;
	}

	const STRINGSET&			getImportantFileSet( ) const
	{
		return importantFileSet;
	}

	const STRINGSET&			getReferCopyFileSet( ) const
	{
		return referenceCopySet;
	}
	const std::string&			getIndexFilePathName( ) const
	{
		return indexFileName;
	}
	const std::string&			getMainFilePathName( ) const
	{
		return mainFileName;
	}
    const uint32                getAllFilePlayTime(int idx) const
    {
        if (idx < 0 || idx >= (int)subFileInfos.size())
        {
            return 0;
        }
        return subFileInfos[idx].playtime;
    }

	uint8 						getSubFileRecordIndex(int idx) const {
		if(idx<0 || idx>=(int)subFileInfos.size()) {
			return 0;
		}
		return subFileInfos[idx].subFileIdx;
	}

	std::string					baseInfoToXML() const;
	std::string					memberFileToXML() const;
	
private:
	std::string							mainFileName;
	std::string							indexFileName;
	IndexType							indexDataType;
	int64								mainFileSize;
	uint32								mainFilePlayTime;
	uint32								muxBitRate;
	uint32								videoBitRate;
	uint32								audioBitRate;
	uint32								horizonResolution;
	uint32								verticalResolution;
	uint32								subFileCount;
	uint16                              zeroPFrameLength;
	uint16                              zeroBFrameLength;
	unsigned char                       zeroPFrame[188];
	unsigned char                       zeroBFrame[188];
	FrameRate							frameRateCode;
	std::vector<SubFileInformation>		subFileInfos;

	///reference copy show us what's else content reference to this content
	STRINGSET							referenceCopySet;
	STRINGSET							importantFileSet;
	bool								hasExtensionData;
	bool								pwe;
	bool								bLocal;
};

struct IndexRecordInfo {
	uint32		packetOffset;
	uint64		timeOffset;
};

struct RecordTrickInfo{
	bool operator()(const RecordTrickInfo& in1, const RecordTrickInfo& in2)
	{
		if (in1.speed.denominator == 0 || in2.speed.denominator == 0 || in1.speed.denominator == in2.speed.denominator)
			return in1.speed.numerator < in2.speed.numerator;
		return ((float)(in1.speed.numerator / (float)in1.speed.denominator)) < ((float)(in2.speed.numerator / (float)in2.speed.denominator));
	}
	std::vector<IndexRecordInfo>  info;
	SPEED_IND   speed;
	float       scale;
	std::string  fileExt;
	uint32      playtime;
	uint64      firstPCR;
	RecordTrickInfo() {
		firstPCR = 0;
		speed.numerator = 0;
		speed.denominator = 0;
		playtime = 0;
		scale = 0.0;
	}
};

class IndexRecord 
{
	friend class IndexFileParser;
public:
	IndexRecord();
	virtual ~IndexRecord();

public:
	
	typedef std::vector<IndexRecordInfo>	RECORDS;

	bool	pwe() const { return mPwe ;}
	void	pwe( bool bPwe ) { mPwe = bPwe; }

	int64	lastParsedOffset() const {
		return mLastParsedOffset;
	}

	void 	setLastParsedOffset(int64 offset) {
		mLastParsedOffset += offset;
	}

	size_t	subfileCount( ) const { 
		if(mRecTrickInfo.size() == 0 ) {
			return 0;
		}
		return mRecTrickInfo.size() - 1; 
	}

	int64	indexFilesize() const { return -1; }
	
	int 	getSubFileIdx(float& speed);

	int32  recordCount() const {
		if(mRecTrickInfo.empty()) {
			return 0;
		}
		return (int32)mRecTrickInfo[0].info.size();
	}

	uint32 getdatabitrate(){return mBitRate;}

	unsigned const char* getZeroMotionBFrame(uint16& len)const;

	unsigned const char* getZeroMotionPFrame(uint16& len) const;

	void   setdatabitrate(uint32 bitrate){mBitRate = bitrate;}

	void   setZeroMotionBFrame(const IndexData& idxdata);

	void   setZeroMotionPFrame(const IndexData& idxdata);

	std::string getSubfileExt(float& speed, uint32& playtime,int& idx); //fot test 
	/*
	void    setMainExtension(std::string ext){ 
		mainExtension = ext;
	}

    void    setMainPlayTime(uint32 playtime){
		mainPlayTime = playtime;
	}
	*/
	uint64	findNextIFrame( uint64 offset) const;

    uint64  findNextIFrameByTimeOffset( uint64& timeOffset, int& rateIdx, float speed = 0.0) const;
	
	uint64  binarySearch(const IndexRecordInfo* low, const IndexRecordInfo* high, uint64& timeOffset)const;
	
	bool    fabsSmaller(const uint32& low, const uint32& high, const uint32& init)const;
	
//	void	addRecord( uint32 packetOffset, uint32 timeOffset);

	//void    addRecTrick( uint32 pkgOffset, uint32 timeOffset, SPEED_IND speed, uint8 subfileidx, const std::string ext,uint32 playtime);

	void    rewriteRecords(const IndexRecord& rec);

	void   sortRecordTrcik();

	void   setHeaderErrCode(int32 errcode){HeaderErrCode = errcode;}

	int32  getHeaderErrCode(){return HeaderErrCode;}

	int     getParsedRecords(){return mParsedRecords;}//for test	

	bool    skipRecord(const IndexData& idxdata)const;

	uint32	lastTimeOffset() const { 
		if(mRecTrickInfo.size() ==0) {
			return 0;
		}
		if (mRecTrickInfo[0].info.size() == 0 ) {
			return 0;
		}
		return lastItem()->timeOffset;
	}

	const bool IsPwe() const{
		return mPwe;
	}

	bool initIndexRecords( const IndexData& header) {
		int subFileCount = header.getSubFileCount();//include normal speed file
		if (subFileCount <= 0 ) {
			return false;
		}
		mRecTrickInfo.clear();
		for (int i = 0; i < subFileCount; i++) {
			RecordTrickInfo info;
			info.speed = header.getSubFileSpeed(i);
			if(info.speed.denominator != 0) {
				info.scale = (float)info.speed.numerator/(float)info.speed.denominator;
			} else {
				info.scale = 0.0f;
			}
			info.fileExt = header.getSubFileName(i);
			info.playtime = header.getAllFilePlayTime(i);
			mRecTrickInfo.push_back(info);
			mSubFileIdx2RecordSlot[header.getSubFileRecordIndex(i)] = (int)(mRecTrickInfo.size()-1);
		}
		return true;
	}

	bool addIndexRecord(IndexRecordInfo& record, int subFileIdx);

	void getTemporaryZone(char*& tempRecordBuffer, char**& tempRecordData, size_t*& tempRecordBufferSize) {
		tempRecordBuffer = (char*)mTempRecordBuffer;
		tempRecordData = &mTempRecord;
		tempRecordBufferSize = &mTempRecordDataSize;
	}

	void getLastTimeOffsetPointer(uint64*& lastTimeOffset) {
		lastTimeOffset = &mLastTimeOffset;
	}

private:

	int32       HeaderErrCode;

    bool IsNormalRate(const float& speed);

	inline uint64	packet2offset(uint64 packet) const {
		return packet * 188;//TS packete align with 188 bytes
	}

	inline const IndexRecordInfo*	firstItem() const {
		if(mRecTrickInfo.empty() || mRecTrickInfo[0].info.empty()) {
			return NULL;
		}
		return &(mRecTrickInfo[0].info[0]);
	}

	inline const IndexRecordInfo*	lastItem() const {
		if(mRecTrickInfo.empty() || mRecTrickInfo[0].info.empty()) {
			return NULL;
		}
		return &(mRecTrickInfo[0].info[ mRecTrickInfo[0].info.size()-1]);
	}

	inline const IndexRecordInfo* firstTrickItem(const RecordTrickInfo& rec) const {
		if(rec.info.empty()) {
			return NULL;
		}
		return  &rec.info[0];
	}

	inline const IndexRecordInfo* lastTrickItem(const RecordTrickInfo& rec) const{
		if(rec.info.empty()){
			return NULL;
		}
		return &rec.info[rec.info.size()-1];
	}

private:
	typedef std::vector<RecordTrickInfo> RECTRICKINFO;
	//PartRecInfo     mPartRecInfo;
	//RECORDS			mRecords;
	RECTRICKINFO    mRecTrickInfo;
	std::map<int,int> mSubFileIdx2RecordSlot;

	char*			mTempRecordBuffer[256];
	char*			mTempRecord;
	unsigned const char*  mpZeroBFrame;
	unsigned const char*  mpZeroPFrame;
	size_t			mTempRecordDataSize;
	uint64			mLastTimeOffset;

	int64 			mLastParsedOffset;
	int64           mLastOffset;

	bool			mPwe;
	bool            mbReverseDir;
	size_t			mSubFileCount;//exclude mainfile
	int             mParsedRecords;
	uint32          mBitRate;
	uint16          mZeroBFrameLen;
	uint16          mZeroPFrameLen;
	float           mTargetScale;
	ZQ::common::Mutex   mLocker;

private:
	const RECTRICKINFO& getTrickInfo() const {
		return mRecTrickInfo; 
	}

	const size_t getSubFileCount() const{
		if(mRecTrickInfo.size() == 0 ) {
			return 0;
		}
		return mRecTrickInfo.size() - 1;
	}
};

class FileReader
{
public:
	virtual ~FileReader() {}

	enum SeekPosition {
		SEEK_FROM_BEGIN = 0, 
		SEEK_FROM_END = 1,
		SEEK_FROM_CUR = 2,
	};

	virtual int32 read( void* buf , size_t count ,size_t size = 1 ) = 0;

	virtual const char* getLine( char* buf ,size_t size ) = 0;

	virtual int32 write( const void* buf ,size_t count , size_t size = 1 ) = 0;

	virtual int64 seek( int64 offset , SeekPosition begin ) = 0;

	virtual int64 tell( ) = 0;

	virtual int64 getMsize() = 0;
	virtual int64 getFileSize( const std::string& file = "" ,bool bLog = true ) = 0;

	virtual const char* getLastErrorDesc( char* buf , size_t bufSize ) = 0;

	virtual const std::string&	getFileName( ) const = 0;

	virtual bool	open( const std::string& filePathName ) = 0 ;

	virtual bool	close( ) = 0;

};
class CsicoIndexFileParser
{
private:
	CsicoIndexFileParser& operator=( const CsicoIndexFileParser&);
public:
	CsicoIndexFileParser( IdxParserEnv& env , const std::string& mainFileName );
	virtual ~CsicoIndexFileParser( );
	
#pragma pack(1)
	typedef struct  _CsicoIndexFileHeader
	{
		char				signature[24];
		uint8				majorVersion;
		uint8				minorVersion;
		uint8				generation;
		char				creator[32];
		uint32				checksum;
		uint8				align;
	}CsicoIndexFileHeader;
	typedef struct _CsicoIndexFileTagAndLength 
	{
		uint32				tagId;
		uint16				length;
	}CsicoIndexFileTagAndLength;

	struct IndexRecordHeader {
		uint8 type;
		uint8 length;
	};

#pragma pack()



public:

	bool			parseCsicoINDEX( FileReader* reader , IndexData& idxData );

	int32	        parseIndexRecord( FileReader* reader, IndexRecord& record );


private:

	bool			skipFileHeader(FileReader* reader );

	//read next record into buffer, if data for that record is insufficient
	//store the in-complete data into temporary buffer and return error code
	int32			readRecordBody(FileReader* reader, const IndexRecordHeader& recordHeader, char* buf, size_t size);
	int32			readRecordHeader(FileReader* reader, IndexRecordHeader& recordHeader);
	int32			skipRecordBody(FileReader* reader, const IndexRecordHeader& recordHeader);

protected:

	bool			parseCsicoINDEX11( FileReader* reader , IndexData& idxData );

	bool			parseSubFiles( FileReader* reader  , IndexData& idxData);

	bool			parseAssetInfo( FileReader* reader , IndexData& idxData );

	bool			parseSection(  FileReader* reader , IndexData& idxData );

	bool            parseZeroMotion( FileReader* reader, IndexData& idxData);

	bool			getTag( FileReader* reader , CsicoIndexFileTagAndLength& tal );
	bool 			getTag( FileReader* reader , IndexRecordHeader& tal,IndexRecord& rec, bool IsNormalRec);
	
	std::string		getSubType( FileReader* reader  , const std::string& xmlContent );
	
	template<typename T> T readAndReinterpretAs( FileReader* reader , size_t size = sizeof(T) )
	{	
		assert( size < sizeof(_tmpBuffer) );

		if( reader->read( _tmpBuffer , size ) != (int)size )
		{			
			throw "not enough data";
		}
		return *reinterpret_cast<T*>(_tmpBuffer);	
	}
	
	uint64 read5byteInt( FileReader* reader  )
	{
		memset(_tmpBuffer,0,10);//reset first 10 bytes to zero so that we can transcode the data directly
		if( reader->read( _tmpBuffer , 5 ) != 5 )
		{			
			throw "not enough data";
		}
		return *((uint64*)_tmpBuffer);
	}
	std::string			readString( FileReader* reader , size_t size );

	int32			readRecordData(FileReader* reader, char* buf, size_t count);
	void 			storeRecordData(const char* header, int32 headerSize, const char* body, int32 bodySize);
	
private:
	IdxParserEnv&		_idxParserEnv;
	std::string			_mainFileName;
	std::string			_lastError;
	std::string			_tmpString;
	char				_tmpBuffer[256];
	int					_subFileCount;

	char*				mTempRecordBuffer;
	char**				mTempRecord;
	size_t*				mTempRecordDataSize;
	uint64*				mLastTimeOffset;
};

#define ERROR_CODE_GENERC_ERROR			-1
#define ERROR_CODE_OBJECT_NOT_FOUND		-2


class IndexFileParser
{
public:
	IndexFileParser( IdxParserEnv& env );
	virtual ~IndexFileParser( );

public:

	bool			parseExtesionFileFromVstrm( const std::string& filePath , IndexData& idxData );

	bool			parseExtensionFileFromCommonFS( const std::string& filePath , IndexData& idxData  );

	
	bool			ParseIndexFileFromVstrm( const std::string& mainFileName ,
												IndexData& idxData , 
												bool bIndexFile = false ,
												const std::string& indexFileName = "",
												 bool bLoadBrief = false , bool bLocalContent = true );

	bool			ParserIndexFileFromCommonFS( const std::string& mainFilePathName, 
													IndexData& idxData , 													
													bool bIndexFile = false ,
													const std::string& indexFileName = "");

	bool			ParseIndexFromMemory( const std::string& mainFilePathName, IndexData& idxData, const char* buffer, size_t size);


	bool	        ParseIndexRecodFromMemory( const std::string& mainFilePathName, IndexRecord& record, const char* buffer, size_t size);

	int32			ParseIndexRecodFromMemoryPWE(const std::string& mainFilePathName, IndexRecord& record, const char* buffer, int64 offset, int64 blocksize);

	bool             ParseIndexRecordFromCommonFS( const std::string& mainFilePathName, IndexRecord& record, const std::string& indexFileName="");

	std::string		getLastError( );

	int				getLastErrorCode( ) const
	{
		return _lastErrorCode;
	}

protected:

	bool			parseIndexFile( FileReader* reader ,
									const std::string&	mainFileName,
									IndexData& idxData , 
									bool bIndexFile = false ,
									const std::string& indexFileName = "" );

	bool			parseVVX( FileReader* reader ,IndexData& idxData );

	bool			parseVV2( FileReader* reader ,IndexData& idxData );

	bool			parseCsicoIndex( FileReader* reader ,IndexData& idxData );

	bool			parseExtension( FileReader* reader , IndexData& idxData );

	bool			parseIndexUsingVstrmAPILoadAll( FileReader* reader , const std::string& mainFileName , IndexData& idxData );

	bool			parseIndexUsingVstrmAPILoadBrief( FileReader* reader , const std::string& mainFileName , IndexData& idxData ,bool bLocalContent);

	bool	        parserCsicoIndexRecord( FileReader* reader, const std::string& mainFileName, IndexRecord& record);

	int32	        parserCsicoIndexRecordPWE( FileReader* reader, const std::string& mainFileName, IndexRecord& record);
private:

	bool			parserVVxVersion5( FileReader* reader , const VVX_INDEX_HEADER& header ,IndexData& idxData );
	bool			parserVVxVersion6( FileReader* reader , const VVX_INDEX_HEADER& header ,IndexData& idxData );
	bool			parserVVxVersion7( FileReader* reader , const VVX_INDEX_HEADER& header ,IndexData& idxData );

private:

	IndexData::FrameRate		convertFrameRateCode( ULONG framRateCode );

	void						setLastErrorCode( int errCode )
	{
		_lastErrorCode = errCode;
	}
private:

	IdxParserEnv&		_idxParserEnv;
	std::string			_mainFileName;
	std::string			_lastError;
	int					_lastErrorCode;
};




class CommonFileReader : public FileReader
{
public:
	CommonFileReader( );	
	
	virtual const char*		getLine( char* buf ,size_t size ) ;

public:
	
	void					resetBufferPointer( );

private:
	unsigned char		szLocalBuffer[4096];
	unsigned char*		pHeader;
	unsigned char*		pTail;	
};

class MemFileReader : public CommonFileReader
{
public:
	MemFileReader( const unsigned char* pData , size_t size , const std::string& fileName, int64 blockEnd = 0 );
	virtual ~MemFileReader( );
public:

	virtual int32 read( void* buf , size_t count ,size_t size = 1 );

	virtual int32 write( const void* buf ,size_t count , size_t size = 1 ) ;

	virtual int64 seek( int64 offset , SeekPosition begin ) ;

	virtual int64 tell( ) ;

	virtual int64 getFileSize( const std::string& file = "" ,bool bLog = true ) ;

	virtual int64 getMsize(){return mSize;}

	virtual const char* getLastErrorDesc( char* buf , size_t bufSize ) ;

	virtual const std::string&	getFileName( ) const ;

	virtual bool	open( const std::string& filePathName ) ;

	virtual bool	close( ) ;

private:

	const unsigned char*		mpBuffer;
	size_t						mSize; //block size
	int64 						mCurPos; //relative to start of current block(mpBuffer)
	std::string					mFileName;
	int64						mBlockBegin;
	int64						mBlockEnd;

};

#ifndef EXCLUDE_VSTRM_API
class FileReaderVstrmI : public CommonFileReader
{
public:	
	FileReaderVstrmI( IdxParserEnv& env );
	~FileReaderVstrmI( );
public:

	virtual int32		read( void* buf , size_t count ,size_t size = 1 ) ;	

	virtual int32		write( const void* buf ,size_t count , size_t size = 1 ) ;

	virtual int64		seek( int64 offset , SeekPosition begin ) ;

	virtual int64		tell( ) ;

	virtual int64		getFileSize( const std::string& file = "" ,bool bLog = true  ) ;
	
	virtual const char* getLastErrorDesc( char* buf , size_t bufSize ) ;	

	virtual bool		open( const std::string& filePathName );

	virtual bool		close( );

	virtual const std::string&	getFileName( ) const;


protected:

	uint32			convertBeginPos( SeekPosition pos );

private:

	VHANDLE			_fileHandle;
	
	OBJECT_ID		_FileId;

	IdxParserEnv&	_idxParserEnv;

	char			_szBuf[1024];

	size_t			_bufSize;

	std::string		_fileName;
};
#endif//#ifndef EXCLUDE_VSTRM_API

#ifdef ZQ_OS_MSWIN
typedef HANDLE	file_t;
#elif defined ZQ_OS_LINUX
typedef int	file_t;
#endif

class FileReaderNative : public CommonFileReader
{
public:
	FileReaderNative( IdxParserEnv& env );
	~FileReaderNative( );

	virtual int32	read( void* buf , size_t count ,size_t size = 1 );	

	virtual int32	write( const void* buf ,size_t count , size_t size = 1 );

	virtual int64	seek( int64 offset , SeekPosition begin );

	virtual int64	tell( );

	virtual int64	getFileSize( const std::string& file = "" ,bool bLog = true ) ;

	virtual int64   getMsize(){return 0;}
	virtual const char* getLastErrorDesc( char* buf , size_t bufSize ) ;

	virtual const std::string&	getFileName( ) const ;

	virtual bool	open( const std::string& filePathName );

	virtual bool	close( );

public:	

	uint32			convertBeginPos( SeekPosition pos );

private:

	FILE*			_fileHandle;

	IdxParserEnv&	_idxParserEnv;

	char			_szBuf[1024];

	size_t			_bufSize;

	std::string		_fileName;

};



}}//namespace ZQ::IdxParser

#endif //_VSTRM_INDEX_FILE_PARSER_HEADER_FILE_H__

