
#ifndef _VSTRM_INDEX_FILE_PARSER_HEADER_FILE_H__
#define _VSTRM_INDEX_FILE_PARSER_HEADER_FILE_H__

#include <string>
#include <vector>
#include "IdxFileParserEnvironment.h"


extern "C"
{
}

namespace ZQ
{
namespace IdxParser
{

class IndexData
{
	friend class IndexFileParser;
	friend class CsicoIndexFileParser;
public:
	IndexData( )
	{
		bPWE				=	false;
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
		subFileInfos.clear();
	}
	IndexData( const IndexData& d )
	{
		bPWE				=	d.bPWE;
		bLocal				=	d.bLocal;
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
	}
	IndexData& operator=( const IndexData& d  ) 
	{
		bPWE				=	d.bPWE;
		bLocal				=	d.bLocal;
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
		default:						szReturn = "Unknown";break;
		}

		return szReturn;
	}

	const std::string			getSubFileName( int index ) const
	{
		if (index<0 || index>= (int)subFileInfos.size())
		{
			return std::string("");
		}
		return subFileInfos[index].fileExtension;
	}
	
	bool						isPWE( ) const
	{
		return bPWE;
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

	typedef struct _SubFileInformation
	{
		std::string				fileExtension;
		SPEED_IND				speed;
		uint32					playtime;
	}SubFileInformation;
	
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
	FrameRate							frameRateCode;
	std::vector<SubFileInformation>		subFileInfos;

	///reference copy show us what's else content reference to this content
	STRINGSET							referenceCopySet;
	STRINGSET							importantFileSet;
	bool								hasExtensionData;
	bool								bLocal;
	bool								bPWE;//is in PWE mode
};

class FileReader
{
public:
	virtual ~FileReader() {}

	enum	SeekPos
	{
		SEEK_FROM_BEGIN,
		SEEK_FROM_END,
		SEEK_FROM_CUR
	};

	virtual int32 read( void* buf , size_t count ,size_t size = 1 ) = 0;

	virtual const char* getLine( char* buf ,size_t size ) = 0;

	virtual int32 write( const void* buf ,size_t count , size_t size = 1 ) = 0;

	virtual int64 seek( int64 offset , SeekPos begin ) = 0;

	virtual int64 tell( ) = 0;

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
	
public:
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
#pragma pack()

	bool			parseCsicoINDEX( FileReader* reader , IndexData& idxData );

protected:

	bool			parseCsicoINDEX11( FileReader* reader , IndexData& idxData );

	bool			parseSubFiles( FileReader* reader  , IndexData& idxData);

	bool			parseAssetInfo( FileReader* reader , IndexData& idxData );

	bool			parseSection(  FileReader* reader , IndexData& idxData );

	bool			getTag( FileReader* reader , CsicoIndexFileTagAndLength& tal );
	
	std::string		getSubType( FileReader* reader  , const std::string& xmlContent );
	
	template<typename T> T readAndReinterpretAs( FileReader* reader , size_t size = sizeof(T) )
	{	
		assert( size < sizeof(_tmpBuffer) );

		if( reader->read( _tmpBuffer , size ) != size )
		{			
			throw "not enough data";
		}
		return *reinterpret_cast<T*>(_tmpBuffer);	
	}
	
	std::string			readString( FileReader* reader , size_t size );
	
private:
	IdxParserEnv&		_idxParserEnv;
	std::string			_mainFileName;
	std::string			_lastError;
	std::string			_tmpString;
	char				_tmpBuffer[256];
	int					_subFileCount;
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
	MemFileReader( const unsigned char* pData , size_t sz , const std::string& fileName);
	virtual ~MemFileReader( );
public:

	virtual int32 read( void* buf , size_t count ,size_t size = 1 );

	virtual int32 write( const void* buf ,size_t count , size_t size = 1 ) ;

	virtual int64 seek( int64 offset , SeekPos begin ) ;

	virtual int64 tell( ) ;

	virtual int64 getFileSize( const std::string& file = "" ,bool bLog = true ) ;

	virtual const char* getLastErrorDesc( char* buf , size_t bufSize ) ;

	virtual const std::string&	getFileName( ) const ;

	virtual bool	open( const std::string& filePathName ) ;

	virtual bool	close( ) ;

private:

	const unsigned char*		mpBuffer;
	size_t						mSize;
	size_t						mCurPos;
	std::string					mFileName;

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

	virtual int64		seek( int64 offset , SeekPos begin ) ;

	virtual int64		tell( ) ;

	virtual int64		getFileSize( const std::string& file = "" ,bool bLog = true  ) ;
	
	virtual const char* getLastErrorDesc( char* buf , size_t bufSize ) ;	

	virtual bool		open( const std::string& filePathName );

	virtual bool		close( );

	virtual const std::string&	getFileName( ) const;


protected:

	uint32			convertBeginPos( SeekPos pos );

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

	virtual int64	seek( int64 offset , SeekPos begin );

	virtual int64	tell( );

	virtual int64	getFileSize( const std::string& file = "" ,bool bLog = true ) ;

	virtual const char* getLastErrorDesc( char* buf , size_t bufSize ) ;

	virtual const std::string&	getFileName( ) const ;

	virtual bool	open( const std::string& filePathName );

	virtual bool	close( );

public:	

	uint32			convertBeginPos( SeekPos pos );

private:

	file_t			_fileHandle;

	IdxParserEnv&	_idxParserEnv;

	char			_szBuf[1024];

	size_t			_bufSize;

	std::string		_fileName;

};



}}//namespace ZQ::IdxParser

#endif //_VSTRM_INDEX_FILE_PARSER_HEADER_FILE_H__
