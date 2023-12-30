
#include <map>
#include <strHelper.h>
#include "IndexFileParser.h"
#include <XMLPreferenceEx.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "SimpleXMLParser.h"
#include <TianShanIceHelper.h>

#ifndef EXCLUDE_VSTRM_API
	#include <vstrmfp.h>
	#include <vsiolib.h>	
#endif

#define IDXPARSERLOG	if( _idxParserEnv.GetLogger() ) (*_idxParserEnv.GetLogger())

#define VSTRMAPICALLSTART(x) ZQTianShan::Util::TimeSpan __localSpan##x;__localSpan##x.start();
#define VSTRMAPICALLEND(x) if(__localSpan##x.stop() > 500 ){ IDXPARSERLOG(ZQ::common::Log::L_INFO,CLOGFMT(x,"vstrm api "#x" time cost [%lld]"),__localSpan##x.span() ); }


namespace ZQ
{
namespace IdxParser
{
CommonFileReader::CommonFileReader()
{
	pTail	=	szLocalBuffer + sizeof( szLocalBuffer );
	pHeader	=	pTail;
}

void CommonFileReader::resetBufferPointer( )
{
	pTail = pHeader = szLocalBuffer;
}

const char* CommonFileReader::getLine( char* buf ,size_t size ) 
{
	register char* p = buf;
	assert( buf != NULL && size != 0 );

	while ( --size )
	{
		if( pHeader >= pTail )
		{
			//no buffered data
			int32	ret = read( szLocalBuffer , sizeof(szLocalBuffer)  );
			if( ret < 0 ) 
			{//should I return NULL directly
				return NULL;
			}				
			pHeader = szLocalBuffer;
			pTail	= pHeader + ret;
			if( ret == 0 )
				break;
		}
		if ( (*p++ = *pHeader++) == ('\n') )
			break;
	}

	*p = '\0';

	return		buf == p ? NULL : buf ;
}


MemFileReader::MemFileReader(  const unsigned char* pData , size_t sz , const std::string& fileName )
:mpBuffer(pData),
mSize(sz),
mCurPos(0),
mFileName(fileName)
{
}

MemFileReader::~MemFileReader( )
{
}

int32 MemFileReader::read( void* buf , size_t count ,size_t size )
{
	size_t szLeft = mSize - mCurPos ;
	size_t szCopy = (size*count) > szLeft ? szLeft : (size*count);

	memcpy( buf , mpBuffer+mCurPos , szCopy );
	mCurPos += szCopy;

	return  static_cast<int32>(szCopy);
}

int32 MemFileReader::write( const void*  ,size_t  , size_t  ) 
{
	return -1;
}

int64 MemFileReader::seek( int64 offset , SeekPos begin ) 
{
	int64 realOff = 0;
	switch ( begin )
	{
	case SEEK_FROM_BEGIN:
		{
			realOff = offset;
			realOff = realOff > (int64)mSize ? mSize : realOff;
			realOff = realOff < 0 ? 0 : realOff;
		}
		break;
	case SEEK_FROM_END:
		{
			int64 tmp = mSize;
			realOff = tmp + offset;
			realOff = realOff > (int64)mSize ? mSize : realOff;			
			realOff = realOff < 0 ? 0 : realOff;
		}
		break;
	case SEEK_FROM_CUR:
		{
			realOff = offset + mCurPos;
			realOff = realOff > (int64)mSize ? mSize : realOff;
			realOff = realOff < 0 ? 0 : realOff;
		}
		break;
	default:
		{

		}
		break;
	}
	mCurPos = (size_t)realOff;
	return realOff;
}

int64 MemFileReader::tell( ) 
{
	return mCurPos;
}

int64 MemFileReader::getFileSize( const std::string&   ,bool   ) 
{
	return mSize;
}

const char* MemFileReader::getLastErrorDesc( char* buf , size_t  )
{
	buf[0]=0;
	return buf;
}

const std::string&	MemFileReader::getFileName( ) const
{
	return mFileName;
}

bool MemFileReader::open( const std::string&  ) 
{
	return true;
}

bool MemFileReader::close( ) 
{
	return true;
}
//////////////////////////////////////////////////////////////////////////

#ifndef EXCLUDE_VSTRM_API

FileReaderVstrmI::FileReaderVstrmI(  IdxParserEnv& env )
:_idxParserEnv(env)
{
	_fileHandle = INVALID_HANDLE_VALUE;
}
FileReaderVstrmI::~FileReaderVstrmI()
{
	close( );
}

bool FileReaderVstrmI::open( const std::string& fileName )
{
	assert( !fileName.empty() );
	close( );

	if( _idxParserEnv.useVsOpenAPI() )
	{
		VSTATUS ret = VsOpenEx( &_fileHandle , (char*)fileName.c_str() ,
			GENERIC_READ , FILE_SHARE_READ | FILE_SHARE_WRITE,
			OPEN_EXISTING,
			FILE_FLAG_CACHED,
			0,
			&_FileId);
		if( !(IS_VSTRM_SUCCESS(ret)) )
		{	
			IDXPARSERLOG(ZQ::common::Log::L_WARNING,CLOGFMT(FileReaderVstrmI,"Can't open  file[%s] because[%s] with VsOpenEx"),
				fileName.c_str() ,getLastErrorDesc(_szBuf,sizeof(_szBuf)-1)  );
			return false;		
		}	
	}	
	else
	{
		_fileHandle	= VstrmCreateFile( _idxParserEnv.GetVstrmHandle() , fileName.c_str() ,
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			0,
			OPEN_EXISTING,
			0,
			0);
		if( _fileHandle == INVALID_HANDLE_VALUE )
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(FileReaderVstrmI,"Can't open file[%s], because [%s] with VstrmCreateFile"),
				fileName.c_str() , 
				getLastErrorDesc( _szBuf , sizeof(_szBuf)-1 )	);
			return false;
		}
	}

	_fileName = fileName;
	return true;
}

bool FileReaderVstrmI::close( )
{

	if(_idxParserEnv.useVsOpenAPI() )
	{
		VsClose( _fileHandle , _FileId );
	}
	else
	{
		if( _fileHandle != INVALID_HANDLE_VALUE )
		{
			VstrmCloseHandle( _idxParserEnv.GetVstrmHandle() , _fileHandle );
			_fileHandle = INVALID_HANDLE_VALUE;
			_fileName	= "";
		}
	}
	return true;
}
int32 FileReaderVstrmI::read( void* buf , size_t count ,size_t size /*= 1*/ )
{
	DWORD	readByte = 0;
	if(_idxParserEnv.useVsOpenAPI() )

	{
		if(FALSE == VsRead(  _fileHandle, (char*) buf, static_cast<int>(count*size), &readByte, 0))
		{		
			IDXPARSERLOG(ZQ::common::Log::L_ERROR, 
				CLOGFMT(FileReaderVstrmI,"(%s) VsRead failed and last error[%s]"),
				_fileName.c_str(),
				getLastErrorDesc(_szBuf,sizeof(_szBuf)-1 ) );
			return -1;
		}
	}
	else
	{
		if( _fileHandle == INVALID_HANDLE_VALUE )
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(FileReaderVstrmI,"please open the index file first before read it") );
			return -1;
		}

		if( FALSE == VstrmReadFile( _idxParserEnv.GetVstrmHandle() , _fileHandle , buf , (DWORD)(count*size) ,  &readByte , NULL  ) )
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(FileReaderVstrmI,"can't read the file[%s] because [%s]"),
				_fileName.c_str()  , 
				getLastErrorDesc( _szBuf,sizeof(_szBuf)-1 ) );
			return -1;
		}
	}
	return static_cast<int32>(readByte);
}
uint32 FileReaderVstrmI::convertBeginPos( SeekPos pos )
{
#ifdef ZQ_OS_MSWIN
	switch(pos)
	{
	case SEEK_FROM_BEGIN:
		return FILE_BEGIN;
		break;
	case SEEK_FROM_END:
		return FILE_END;
		break;
	case SEEK_FROM_CUR:
		return FILE_CURRENT;
		break;
	default:
		return FILE_BEGIN;
		break;
	}
#else
	switch(pos)
	{
	case SEEK_FROM_BEGIN:
		return SEEK_SET;
	case SEEK_FROM_END:
		return SEEK_END;
	case SEEK_FROM_CURRENT:
		return SEEK_CUR;
	default:
	return SEEK_SET;
	}
#endif
}
int32 FileReaderVstrmI::write( const void* buf ,size_t count , size_t size /* = 1 */ )
{
	DWORD writtenByte = 0;
	if(_idxParserEnv.useVsOpenAPI() )
	{
		if( FALSE == VsWrite( _fileHandle , static_cast<int>(count*size) , (char*)buf , &writtenByte , NULL ) )
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(FileReaderVstrmI,"write to file[%s] failed") ,_fileName.c_str() );
			return -1;
		}
	}
	else
	{
		if( _fileHandle == INVALID_HANDLE_VALUE )
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(FileReaderVstrmI,"please open the index file first before write it") );
			return -1;
		}	
		if( FALSE == VstrmWriteFile(_idxParserEnv.GetVstrmHandle() , _fileHandle , buf , (DWORD)(count* size) , &writtenByte , 0 ))
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(FileReaderVstrmI,"can't write the file[%s] because [%s]"),
				_fileName.c_str()  , 
				getLastErrorDesc( _szBuf,sizeof(_szBuf)-1 ) );
			return -1;
		}
	}

	return static_cast<int32>(writtenByte);
}

int64 FileReaderVstrmI::seek( int64 offset , SeekPos begin ) 
{
	DWORD	beginPos	= convertBeginPos(begin);
	if(_idxParserEnv.useVsOpenAPI() )
	{
		LARGE_INTEGER li;
		li.QuadPart = offset;
		VSTATUS status = VsSeek(_fileHandle,_FileId,&li,(int)beginPos);
		if( !IS_VSTRM_SUCCESS(status) )
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(FileReaderVstrmI,"failed to seek file[%s]"),_fileName.c_str() );
			return -1;
		}
		return li.QuadPart;
	}
	else
	{
		int64 ret = 0;
		DWORD	moveLow		= static_cast<DWORD>( offset & 0xFFFFFFFF );
		LONG	moveHigh	= static_cast<LONG>( offset >> 32 );	
		DWORD	apiRet = 0 ;

		if( _fileHandle == INVALID_HANDLE_VALUE )
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(FileReaderVstrmI,"please open the index file first before use it") );
			return -1;
		}
		if ( INVALID_SET_FILE_POINTER == ( apiRet = VstrmSetFilePointer(_idxParserEnv.GetVstrmHandle() , _fileHandle , moveLow, &moveHigh, beginPos ) ))
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(FileReaderVstrmI,"can't seek with pos[%d] offset[%lld] because[%s]"),
				begin , offset ,
				getLastErrorDesc(_szBuf,sizeof(_szBuf)-1));
			return -1;
		}

		ret =  static_cast<int64>(moveHigh) << 32 ;
		ret |= apiRet;
		return ret;
	}
}

int64 FileReaderVstrmI::tell( )
{
	return seek(0,SEEK_FROM_CUR);	
}

const char* FileReaderVstrmI::getLastErrorDesc( char* buf , size_t bufSize ) 
{
	assert( _idxParserEnv.GetVstrmHandle() != NULL );
	VstrmClassGetErrorText( _idxParserEnv.GetVstrmHandle() , VstrmGetLastError() , buf , (ULONG)bufSize );
	return buf;
}
const std::string& FileReaderVstrmI::getFileName( ) const
{
	return _fileName;
}

int64 FileReaderVstrmI::getFileSize( const std::string& file /*= "" */ ,bool bLog  )
{
	std::string filePathName ;
	if ( file.empty() )
	{
		filePathName = _fileName;
	}
	else
	{
		filePathName = file;
	}
	
	DLL_FIND_DATA_LONG findData = {0};
	VHANDLE fileHandle =VstrmFindFirstFileEx( _idxParserEnv.GetVstrmHandle(), filePathName.c_str() , &findData );
	if(fileHandle != INVALID_HANDLE_VALUE)
	{
		VstrmFindClose( _idxParserEnv.GetVstrmHandle() , fileHandle );
		LARGE_INTEGER fileSize = {findData.w.nFileSizeLow, findData.w.nFileSizeHigh};
		int64 retSize = static_cast<int64>(fileSize.QuadPart);;
		IDXPARSERLOG(ZQ::common::Log::L_INFO,CLOGFMT(FileReaderVstrmI,"get file[%s]'s size [%lld]"),filePathName.c_str() , retSize );
		return retSize;
	}
	else
	{
		if(bLog)
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(FileReaderVstrmI,"can't get file[%s]'s size"),filePathName.c_str() );
		return 0;
	}
	return 0;
}
#endif//#ifndef EXCLUDE_VSTRM_API

//////////////////////////////////////////////////////////////////////////
#ifndef INVALID_HANDLE_VALUE
	#define INVALID_HANDLE_VALUE ( -1)
#endif

FileReaderNative::FileReaderNative( IdxParserEnv& env )
:_idxParserEnv(env)
{
	_fileHandle = INVALID_HANDLE_VALUE;
}

FileReaderNative::~FileReaderNative()
{
	close();
}
uint32 FileReaderNative::convertBeginPos( SeekPos pos )
{
#ifdef ZQ_OS_MSWIN
	switch(pos)
	{
	case SEEK_FROM_BEGIN:
		return FILE_BEGIN;
		break;
	case SEEK_FROM_END:
		return FILE_END;
		break;
	case SEEK_FROM_CUR:
		return FILE_CURRENT;
		break;
	default:
		return FILE_BEGIN;
		break;
	}
#else
	switch(pos)
	{
	case SEEK_FROM_BEGIN:
		return SEEK_SET;
	case SEEK_FROM_END:
		return SEEK_END;
	case SEEK_FROM_CUR:
		return SEEK_CUR;
	default:
		return SEEK_SET;
	}
#endif
}

bool FileReaderNative::open( const std::string& fileName )
{
	assert( !fileName.empty() );
	close();
#ifdef ZQ_OS_MSWIN
	_fileHandle = CreateFile( fileName.c_str() , GENERIC_READ , FILE_SHARE_READ , 0, OPEN_EXISTING , 0 , 0 );
	if( _fileHandle == INVALID_HANDLE_VALUE )	
#else
	_fileHandle = ::open(fileName.c_str() , O_RDONLY );
	if( _fileHandle < 0 )	
#endif
	{
		IDXPARSERLOG(ZQ::common::Log::L_WARNING,CLOGFMT(FileReaderNTFSI,"failed to open [%s] because [%s]"),
			fileName.c_str() , 
			getLastErrorDesc(_szBuf,sizeof(_szBuf)-1  ) );
		return false;
	}
	_fileName = fileName;
	return true;
}

bool FileReaderNative::close( )
{
#ifdef ZQ_OS_MSWIN
	if( _fileHandle != INVALID_HANDLE_VALUE )
	{
		CloseHandle( _fileHandle );
		_fileHandle = INVALID_HANDLE_VALUE;
		_fileName = "";
	}
#else
	if( _fileHandle >= 0 )
	{
		::close(_fileHandle);
		_fileHandle = -1;
		_fileName = "";
	}
#endif
	return true;
}
int32 FileReaderNative::read( void* buf , size_t count ,size_t size/* = 1 */) 
{
	if( _fileHandle == INVALID_HANDLE_VALUE )
	{
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(FileReaderVstrmI,"please open the index file first before use it") );
		return -1;
	}

#ifdef ZQ_OS_MSWIN
	DWORD readByte = 0;
	if( !ReadFile(_fileHandle , buf , (DWORD)(count*size) ,&readByte , NULL ) )
	
#else
	ssize_t readByte = ::read(_fileHandle , buf  , count*size );
	if( readByte < 0 )
#endif
	{
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(FileReaderNative,"failed to read file [%s] because [%s]"),
			_fileName.c_str(),
			getLastErrorDesc(_szBuf,sizeof(_szBuf)-1));
		return -1;
	}

	return static_cast<int32>( readByte );
}

int32 FileReaderNative::write( const void* buf ,size_t count , size_t size/* = 1 */) 
{
	if( _fileHandle == INVALID_HANDLE_VALUE )
	{
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(FileReaderVstrmI,"please open the index file first before use it") );
		return -1;
	}
#ifdef ZQ_OS_MSWIN
	DWORD writtenByte = 0;
	if( !WriteFile( _fileHandle , buf , (DWORD)(count* size) , &writtenByte , NULL ) )	
#else 
	ssize_t writtenByte = ::write( _fileHandle , buf , count*size);
	if( writtenByte < 0 )
#endif
	{
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(FileReaderNative,"failed to write file because [%s]"),
			getLastErrorDesc(_szBuf,sizeof(_szBuf)-1));
		return -1;
	}

	return writtenByte ;
}

int64 FileReaderNative::seek( int64 offset , SeekPos begin )
{
	if( _fileHandle == INVALID_HANDLE_VALUE )
	{
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(FileReaderNative,"please open the index file first before use it") );
		return -1;
	}
#ifdef ZQ_OS_MSWIN
	DWORD	moveLow		= static_cast<uint32>( offset & 0xFFFFFFFF );
	LONG	moveHigh	= static_cast<uint32>( offset >> 32 );
	DWORD	beginPos	= convertBeginPos(begin);
	DWORD	apiRet = 0 ;
	if ( INVALID_SET_FILE_POINTER == ( apiRet = SetFilePointer(_fileHandle , moveLow, &moveHigh, beginPos ) ) )
	{
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(FileReaderNative,"can't seek with pos[%d] offset[%lld] because[%s]"),
			begin , offset ,
			getLastErrorDesc(_szBuf,sizeof(_szBuf)-1));
		return -1;
	}
	int64 ret = static_cast<int64>( moveHigh ) << 32 ;
	ret |= apiRet;
#else
	off_t mv = (off_t)offset;
	int beginPos = convertBeginPos(begin);
	off_t ret = lseek( _fileHandle , mv , beginPos );
	if( ret < 0 )
	{
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(FileReaderNative,"can't seek with pos[%d] offset[%lld] because[%s]"),
			begin , offset ,
			getLastErrorDesc(_szBuf,sizeof(_szBuf)-1));
		return -1;
	}
#endif

	return static_cast<int64>( ret );
}

int64 FileReaderNative::tell( )
{
	if( _fileHandle == INVALID_HANDLE_VALUE )
	{
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(FileReaderNative,"please open the index file first before use it") );
		return -1;
	}
	return seek(0,SEEK_FROM_CUR);
}
const char* FileReaderNative::getLastErrorDesc( char* buf , size_t bufSize ) 
{
#ifdef ZQ_OS_MSWIN
	DWORD dwErr = GetLastError();
	::FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM ,
						0,
						dwErr,
						LANG_SYSTEM_DEFAULT,
						buf,
						(DWORD)bufSize,
						NULL);
#else
	strerror_r(errno,buf,bufSize);
#endif

	return buf;
}

const std::string& FileReaderNative::getFileName( ) const
{
	return _fileName;
}
int64 FileReaderNative::getFileSize( const std::string& file /*= "" */,bool bLog /*= true */ )
{
	std::string filePathName ;
	if ( file.empty() )
	{
		filePathName = _fileName;
	}
	else
	{
		filePathName = file;
	}

#ifdef ZQ_OS_MSWIN
	WIN32_FIND_DATA findData = {0};
	HANDLE fileHandle =FindFirstFile( filePathName.c_str() , &findData );
	if(fileHandle != INVALID_HANDLE_VALUE )
	{
		FindClose(fileHandle);
		LARGE_INTEGER fileSize = {findData.nFileSizeLow, findData.nFileSizeHigh};
		return static_cast<int64>(fileSize.QuadPart);
	}
	else
	{
		if(bLog)
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(FileReaderNative,"can't get file[%s]'s size"),filePathName.c_str() );
		return 0;
	}
#else
	struct stat fst;
	if( lstat( filePathName.c_str(),&fst) < 0 )
	{
		if(bLog)
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(FileReaderNative,"can't get file[%s]'s size"),filePathName.c_str() );
		return 0;
	}
	return (int64)fst.st_size;
#endif
	return 0;
}


//////////////////////////////////////////////////////////////////////////
IndexFileParser::IndexFileParser( IdxParserEnv& env )
:_idxParserEnv(env),
_lastErrorCode(0)
{

}
IndexFileParser::~IndexFileParser( )
{

}
#ifndef EXCLUDE_VSTRM_API
bool IndexFileParser::parseIndexUsingVstrmAPILoadBrief( FileReader* reader , const std::string& mainFileName , IndexData& idxData ,bool bLocalContent)
{
	VstrmLoadAssetInfo* LoadMethod = _idxParserEnv.getLoadInfoMethod();
	VstrmLoadAssetInfoEx* LoadExMethod = _idxParserEnv.getLoadInfoExMethod();
	assert( LoadMethod != NULL );

	//
	ULONG	bitRate = 0;
	ULONG	duration = 0;
	VOD_ASSET_INDEX_TYPE type = VOD_ASSET_INDEX_IS_VVX;
	VOD_ASSET_LOCATION location = bLocalContent ? VOD_ASSET_LOCATION_LOCAL : VOD_ASSET_LOCATION_REMOTE;
	UCHAR	flag = 0;


	/// NOTE: refer to Jira NGOD-274
	/// 

	VSTATUS ret = 0;
	if( LoadExMethod)
	{
		IDXPARSERLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(ParseIndexFileFromVstrm,"Parse Content[%s] using API{VstrmClassLoadBriefAssetInfoEx}"),mainFileName.c_str());
		VSTRMAPICALLSTART(LoadBriefAssetInfoEx);
		ret  = LoadExMethod->LoadBriefAssetInfoEx( _idxParserEnv.GetVstrmHandle() , 
													mainFileName.c_str() ,
													&bitRate ,
													&duration,
													&type,
													location ,
													&flag);
		VSTRMAPICALLEND(LoadBriefAssetInfoEx);
	}
	else
	{
		IDXPARSERLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(ParseIndexFileFromVstrm,"Parse Content[%s] using API{VstrmClassLoadBriefAssetInfo}"),mainFileName.c_str());
		VSTRMAPICALLSTART(LoadBriefAssetInfo);
		ret  = LoadMethod->LoadBriefAssetInfo( _idxParserEnv.GetVstrmHandle() , 
												mainFileName.c_str() ,
												&bitRate ,
												&duration,
												&type,
												location );
		VSTRMAPICALLEND(LoadBriefAssetInfo);
	}

	if( ret != VSTRM_SUCCESS )
	{
		if( ret != VSTRM_OBJECT_NAME_NOT_FOUND )
		{
			IDXPARSERLOG(ZQ::common::Log::L_WARNING,CLOGFMT(VstrmClassLoadBriefAssetInfo,"failed to parse[%s]' index ,use LoadFullAssetInfo"),mainFileName.c_str() );
		}
		return parseIndexUsingVstrmAPILoadAll( reader , mainFileName , idxData );		
	}
#ifdef VOD_IS_CDN_PWE
	idxData.bPWE				= VOD_IS_CDN_PWE(flag);
#else
	idxData.bPWE				= false;
#endif
	idxData.muxBitRate			= bitRate;
	idxData.mainFilePlayTime	= duration;

	if( bitRate <= 0 /*|| duration <= 0 */)
	{
		IDXPARSERLOG(ZQ::common::Log::L_WARNING, CLOGFMT(PVstrmClassLoadBriefAssetInfo,"invalid index data for [%s], playTime[%lu] muxBitrate[%lu]"),
			mainFileName.c_str() , duration, bitRate );
		return parseIndexUsingVstrmAPILoadAll( reader , mainFileName , idxData );		
	}

	switch ( type )
	{
	case VOD_ASSET_INDEX_IS_VVX:
		{
			idxData.indexDataType	=	IndexData::INDEX_TYPE_VVX;			
			idxData.indexFileName	=	mainFileName+".VVX";			
		}
		break;

	case VOD_ASSET_INDEX_IS_VV2:
		{
			idxData.indexDataType	=	IndexData::INDEX_TYPE_VV2;
			idxData.indexFileName	=	mainFileName+".VV2";			
		}
		break;
	case VOD_ASSET_INDEX_IS_VVC:
		{
			idxData.indexDataType	=	IndexData::INDEX_TYPE_VVC;
			idxData.indexFileName	=	mainFileName+".INDEX";						
		}
	default:
		break;
	}
	IDXPARSERLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(PVstrmClassLoadBriefAssetInfo,"got information for [%s]: indexfile[%s], playtime[%lu] bitrate[%lu] pwe[%s]"),
		mainFileName.c_str() , idxData.indexFileName.c_str() , idxData.mainFilePlayTime, idxData.muxBitRate, idxData.bPWE?"true":"false");
	return true;
}

bool IndexFileParser::parseIndexUsingVstrmAPILoadAll(  FileReader*  , const std::string& mainFileName , IndexData& idxData )
{
	VstrmLoadAssetInfo* LoadMethod = _idxParserEnv.getLoadInfoMethod();
	VstrmLoadAssetInfoEx* LoadExMethod = _idxParserEnv.getLoadInfoExMethod();
	assert( LoadMethod != NULL );	

	//
	VOD_ASSET_INFO assetInfo;
	memset(&assetInfo,0,sizeof(assetInfo));
	ULONG			infoSize =sizeof(assetInfo);
	UCHAR			flag = 0;	

	VSTATUS ret = 0;
	if(LoadExMethod)
	{
		IDXPARSERLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(ParseIndexFileFromVstrm,"Parse Content[%s] using API{VstrmClassLoadFullAssetInfoEx}"),mainFileName.c_str());
		VSTRMAPICALLSTART(LoadFullAssetInfoEx);
		ret = LoadExMethod->LoadFullAssetInfoEx( _idxParserEnv.GetVstrmHandle() , 
											mainFileName.c_str() ,
											&assetInfo ,
											&infoSize,
											&flag);
		VSTRMAPICALLEND(LoadFullAssetInfoEx);
	}
	else
	{
		IDXPARSERLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(ParseIndexFileFromVstrm,"Parse Content[%s] using API{VstrmClassLoadFullAssetInfo}"),mainFileName.c_str());
		VSTRMAPICALLSTART(LoadFullAssetInfo);
		ret = LoadMethod->LoadFullAssetInfo( _idxParserEnv.GetVstrmHandle() , 
											mainFileName.c_str() ,
											&assetInfo ,
											&infoSize );
		VSTRMAPICALLEND(LoadFullAssetInfo);
	}
	if( ret != VSTRM_SUCCESS )
	{
		if( ret == VSTRM_OBJECT_NAME_NOT_FOUND )
		{//set object not found error code
			setLastErrorCode( ERROR_CODE_OBJECT_NOT_FOUND );
		}

		char szErrorBuf[1024]={0};
		VstrmClassGetErrorText( _idxParserEnv.GetVstrmHandle() , ret , szErrorBuf , sizeof(szErrorBuf) );
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(parseIndexUsingVstrmAPILoadAll,
			"failed to parse [%s]'s index data because[%s]"),
			mainFileName.c_str() ,
			szErrorBuf );
		_lastError = szErrorBuf;
#if defined _DEBUG || defined DEBUG
		printf("failed to parse index data for [%s] because [%s]\n",
			mainFileName.c_str() ,
			szErrorBuf );
#endif
		return false;
	}
	if( assetInfo.numberOfSubFiles < 1 )
	{
		IDXPARSERLOG(ZQ::common::Log::L_ERROR, CLOGFMT(parseIndexUsingVstrmAPILoadAll,"incorrect index data for [%s], no subfile is found"),
			mainFileName.c_str() );		
		return false;
	}
	if( /*assetInfo.subFile[0].playTimeInMilliSeconds <= 0 ||*/
		assetInfo.subFile[0].bitRate <= 0 )
	{
		IDXPARSERLOG(ZQ::common::Log::L_ERROR, CLOGFMT(parseIndexUsingVstrmAPILoadAll,"invalid index data for [%s], playTime[%u] muxBitrate[%u]"),
			mainFileName.c_str() ,
			assetInfo.subFile[0].playTimeInMilliSeconds,
			assetInfo.subFile[0].bitRate );		
		return false;
	}
	idxData.subFileCount	=	assetInfo.numberOfSubFiles;
	idxData.mainFileSize	=	static_cast<int64>(assetInfo.subFile[0].fileLength);
	idxData.mainFilePlayTime=	static_cast<uint32>(assetInfo.subFile[0].playTimeInMilliSeconds);
	idxData.muxBitRate		=	static_cast<uint32>(assetInfo.subFile[0].bitRate);
	idxData.bLocal			=	( assetInfo.indexLocation != VOD_ASSET_LOCATION_REMOTE );

#ifdef VOD_IS_CDN_PWE
	idxData.bPWE			=	VOD_IS_CDN_PWE(flag);
#else
	idxData.bPWE			=	false;
#endif

	switch ( assetInfo.indexType )
	{
	case VOD_ASSET_INDEX_IS_VVX:
		{
			idxData.indexDataType	=	IndexData::INDEX_TYPE_VVX;			
			idxData.indexFileName	=	mainFileName+".VVX";
			idxData.mainFileName	=	mainFileName + assetInfo.subFile[0].extension ;
		}
		break;

	case VOD_ASSET_INDEX_IS_VV2:
		{
			idxData.indexDataType	=	IndexData::INDEX_TYPE_VV2;
			idxData.indexFileName	=	mainFileName+".VV2";
			idxData.mainFileName	=	mainFileName + assetInfo.subFile[0].extension ;
		}
		break;
	case VOD_ASSET_INDEX_IS_VVC:
		{
			idxData.indexDataType	=	IndexData::INDEX_TYPE_VVC;
			idxData.indexFileName	=	mainFileName+".INDEX";
			idxData.mainFileName	=	mainFileName + "." + assetInfo.subFile[0].extension ;			
			for( int i = 0 ;i < assetInfo.numberOfSubFiles ; i ++ )
			{
				IndexData::SubFileInformation info;			
				info.fileExtension	=	std::string(".") + assetInfo.subFile[i].extension;
				info.playtime		=	assetInfo.subFile[i].playTimeInMilliSeconds;		
				info.speed.denominator=	static_cast<USHORT>(assetInfo.subFile[i].speedDenominator);
				info.speed.numerator=	static_cast<SHORT>(assetInfo.subFile[i].speedNumerator);
				idxData.subFileInfos.push_back(info);
			}
			return true;
		}
	default:
		break;
	}

	for( int i = 0 ;i < assetInfo.numberOfSubFiles ; i ++ )
	{
		IndexData::SubFileInformation info;
		
		info.fileExtension	=	assetInfo.subFile[i].extension;
		

		info.playtime		=	assetInfo.subFile[i].playTimeInMilliSeconds;		
		info.speed.denominator=	static_cast<USHORT>(assetInfo.subFile[i].speedDenominator);
		info.speed.numerator=	static_cast<SHORT>(assetInfo.subFile[i].speedNumerator);
		idxData.subFileInfos.push_back(info);
	}
	IDXPARSERLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(parseIndexUsingVstrmAPILoadAll,"got information for [%s]: indexfile[%s], playtime[%lu] bitrate[%lu] pwe[%s]"),
		mainFileName.c_str() , idxData.indexFileName.c_str() , idxData.mainFilePlayTime, idxData.muxBitRate, idxData.bPWE?"true":"false");
	return true;
}
#endif//EXCLUDE_VSTRM_API

bool IndexFileParser::parseExtesionFileFromVstrm( const std::string& filePath , IndexData& idxData )
{
#ifndef EXCLUDE_VSTRM_API
	FileReaderVstrmI reader(_idxParserEnv);	
	std::string extFileName = filePath;
	bool bExtensionFile = false;
	if( extFileName.length() > 4 )
	{
		std::string	strTemp = extFileName.substr(extFileName.length() -4 );
		if( stricmp(strTemp.c_str(),".xml") != 0 )
		{
			extFileName = extFileName + ".xml";
		}
		bExtensionFile = true;
	}

	if( _idxParserEnv.skipZeroByteFile() )
	{
		if( reader.getFileSize(extFileName, true ) <= 0 )
		{
			return false;
		}
	}

	if( bExtensionFile && reader.open(extFileName) ) 
	{
		if( parseExtension( &reader , idxData ) )
		{
			idxData.hasExtensionData = true;
			idxData.importantFileSet.push_back( extFileName );		
		}
	}
#endif//#ifndef EXCLUDE_VSTRM_API
	return false;
}

bool IndexFileParser::parseExtensionFileFromCommonFS( const std::string& filePath , IndexData& idxData  )
{
	FileReaderNative reader(_idxParserEnv);
	std::string extFileName = filePath;
	bool bExtensionFile = false;
	if( extFileName.length() > 4 )
	{
		std::string	strTemp = extFileName.substr(extFileName.length() -4 );
		if( stricmp(strTemp.c_str(),".xml") != 0 )
		{
			extFileName = extFileName + ".xml";
		}
		bExtensionFile = true;
	}

	if( _idxParserEnv.skipZeroByteFile())
	{
		if( reader.getFileSize(extFileName, true ) <= 0 )
		{
			return false;
		}
	}

	if( bExtensionFile && reader.open(extFileName) ) 
	{
		if( parseExtension( &reader , idxData ) )
		{
			idxData.hasExtensionData = true;
			idxData.importantFileSet.push_back( extFileName );		
		}
	}
	return false;
}

typedef		bool (IndexFileParser::*PARSEFUNC)( FileReader* reader ,IndexData& idxData  );

typedef		struct  _PARSEROUTE	
{
	char*					_indexExtension;
	PARSEFUNC				_parser;
	IndexData::IndexType	_type;
}PARSEROUTE;



bool IndexFileParser::parseIndexFile( FileReader* reader ,
									 const std::string&	mainFileName,
									 IndexData& idxData , 
									 bool bIndexFile , 
									 const std::string& indexFileName )
{
	idxData.mainFileName = mainFileName;
	_mainFileName = mainFileName;

	PARSEROUTE funcRoute[] = 
	{
		{ ".VVX" ,		&IndexFileParser::parseVVX ,			IndexData::INDEX_TYPE_VVX },
		{ ".vvx" ,		&IndexFileParser::parseVVX ,			IndexData::INDEX_TYPE_VVX },
		{ ".VV2" ,		&IndexFileParser::parseVV2 ,			IndexData::INDEX_TYPE_VV2 },
		{ ".vv2" ,		&IndexFileParser::parseVV2 ,			IndexData::INDEX_TYPE_VV2 },
		{ ".index" ,	&IndexFileParser::parseCsicoIndex ,		IndexData::INDEX_TYPE_VVC },
		{ ".INDEX" ,	&IndexFileParser::parseCsicoIndex ,		IndexData::INDEX_TYPE_VVC },
	};

	IDXPARSERLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(parseIndexFile,"Parse Content[%s] by StreamSmith with indexFile[%s]"),
		mainFileName.c_str(),indexFileName.c_str() );
	bool bParseOK = true;
	if( bIndexFile )
	{
		//find the extension name
		std::string::size_type  dotPos = indexFileName.rfind(".");
		std::string		extName;
		if( dotPos != std::string::npos )
		{
			extName = indexFileName.substr(dotPos);
			for( int id = 0 ;id < sizeof(funcRoute)/sizeof(funcRoute[0]) ; id ++ )
			{
				if( strcmp(funcRoute[id]._indexExtension,extName.c_str() ) == 0 )
				{
					if( _idxParserEnv.skipZeroByteFile() && ( reader->getFileSize(indexFileName , true ) <= 0))
					{
						IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(parseIndexFile,"Parse Content[%s] by StreamSmith with indexFile[%s] , but index's size is 0"),
							mainFileName.c_str(),indexFileName.c_str() );
						bParseOK = false;
						return false;
					}
					if( (reader->open( indexFileName )) )
					{
						if( (this->*funcRoute[id]._parser)(reader,idxData))
						{
							idxData.indexDataType = funcRoute[id]._type;
							idxData.indexFileName = indexFileName;
							return true;
						}						
					}
				}
			}
		}		
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,
				CLOGFMT(IndexFileParser,"failed to parse index file [%s]"),indexFileName.c_str());
			return false;
		}
	}
	else
	{
		
		std::string idxFileName;
		for( int id = 0 ;id < sizeof(funcRoute)/sizeof(funcRoute[0]) ;id ++ )
		{
			idxFileName = mainFileName + funcRoute[id]._indexExtension;

			if( ( _idxParserEnv.skipZeroByteFile()  ? ( reader->getFileSize( idxFileName , true )  > 0 )  : true ) && 
				reader->open( idxFileName ) )
			{
				if( (this->*funcRoute[id]._parser)(reader,idxData))
				{
					IDXPARSERLOG(ZQ::common::Log::L_DEBUG,
						CLOGFMT(IndexFileParser,"parse index file [%s] for [%s] ok , "
						"and mainFileSize[%lld] bitRate[%u] playTime[%u]"),
						reader->getFileName().c_str(),
						mainFileName.c_str() , idxData.getMainFileSize(), idxData.getMuxBitrate() ,idxData.getPlayTime() );
					idxData.indexDataType = funcRoute[id]._type;
					idxData.indexFileName = idxFileName;
					return true;
				}
				else
				{
					IDXPARSERLOG(ZQ::common::Log::L_WARNING,
						CLOGFMT(IndexFileParser,"failed to parse index file [%s] for content[%s]"),
						idxFileName.c_str() , mainFileName.c_str() );
				}
			}
		}
		IDXPARSERLOG(ZQ::common::Log::L_WARNING,
			 				CLOGFMT(IndexFileParser,"can't parse index file for content[%s]"),
			 				mainFileName.c_str() );
		return false;
	}

}


bool IndexFileParser::ParseIndexFileFromVstrm( const std::string& mainFileName ,
											  IndexData& idxData ,
											  bool bIndexFile,
											  const std::string& indexFileName ,
											  bool bLoadBrief , bool bLocalContent )
{	
#ifndef EXCLUDE_VSTRM_API
	FileReaderVstrmI reader(_idxParserEnv);	
	if( _idxParserEnv.canUseVstrmIndexParseAPI() && _idxParserEnv.isVstrmLoadInfoMethodAvailable() )
	{
		if( bLoadBrief)
		{			
			return parseIndexUsingVstrmAPILoadBrief( &reader , mainFileName , idxData , bLocalContent );
		}
		else
		{			
			return parseIndexUsingVstrmAPILoadAll( &reader,  mainFileName , idxData );
		}
	}
	else
	{
		return parseIndexFile( &reader, mainFileName , idxData , bIndexFile , indexFileName );
	}
#else
	return false;
#endif//#ifndef EXCLUDE_VSTRM_API
}
std::string	IndexFileParser::getLastError( )
{
	return _lastError;
}


bool IndexFileParser::ParserIndexFileFromCommonFS( const std::string& mainFilePathName ,IndexData& idxData ,
												  bool bIndexFile , const std::string& indexFileName )
{
	//now we can only support NTFS
	FileReaderNative reader(_idxParserEnv);
	return parseIndexFile( &reader, mainFilePathName, idxData , bIndexFile , indexFileName );
}

IndexData::FrameRate IndexFileParser::convertFrameRateCode( ULONG framRateCode )
{
	return (IndexData::FrameRate)framRateCode;
}

bool IndexFileParser::parserVVxVersion5( FileReader* reader , const VVX_INDEX_HEADER& header ,IndexData& idxData )
{
	assert( reader != NULL );
	idxData.muxBitRate			=	static_cast<uint32>( header.mpegBitRate );
	idxData.horizonResolution	=	static_cast<uint32>( header.horizontalSize );
	idxData.verticalResolution	=	static_cast<uint32>( header.verticalSize );
	idxData.videoBitRate		=	static_cast<uint32>( header.videoBitRate );
	idxData.subFileCount		=	static_cast<uint32>( header.subFileInformationCount );
	idxData.frameRateCode		=	convertFrameRateCode( header.frameRateCode );
	
	assert(idxData.subFileCount < 1000 );
	
	int64 subFileOffset = header.subFileInformationOffset;
	if( reader->seek( subFileOffset , FileReader::SEEK_FROM_BEGIN) != subFileOffset )
	{
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"[%s] can't seek to [%lld]"), 
			reader->getFileName().c_str(),subFileOffset);
		return false;
	}
	
	VVX_SUBFILE_INFORMATION* pSubInfo = new VVX_SUBFILE_INFORMATION[idxData.subFileCount];
	size_t	readSize = sizeof(VVX_SUBFILE_INFORMATION ) *  idxData.subFileCount;
	if( reader->read( pSubInfo , readSize ) != readSize )
	{
		delete[] pSubInfo;
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"read [%s] sub file information failed"), 
			reader->getFileName().c_str() );
		return false;
	}

	IndexData::SubFileInformation info;
	for( uint32 i = 0; i < idxData.subFileCount ; i ++ )
	{
		info.fileExtension		=	reinterpret_cast<const char*>( pSubInfo[i].fileExtension );
		info.playtime			=	static_cast<uint32>( pSubInfo[i].playTimeInMilliSeconds );
		info.speed.numerator	=	static_cast<SHORT>( pSubInfo[i].speed.numerator);
		info.speed.denominator	=	static_cast<SHORT>( pSubInfo[i].speed.denominator);
		idxData.subFileInfos.push_back( info );
	}

	delete[] pSubInfo;
	pSubInfo = NULL;

	
	
	return true;
}

bool IndexFileParser::parserVVxVersion6( FileReader* reader , const VVX_INDEX_HEADER& header ,IndexData& idxData )
{
	assert( reader != NULL );
	//sizeof(VVX_V6_INDEX_HEADER) < sizeof(VVX_INDEX_HEADER)
	// so we can perform this conversion
	VVX_V6_INDEX_HEADER* v6Header = reinterpret_cast<VVX_V6_INDEX_HEADER*>( (VVX_INDEX_HEADER*)&header );
	
	idxData.muxBitRate			=	static_cast<uint32>(v6Header->streamBitRate);
	idxData.horizonResolution	=	static_cast<uint32>(v6Header->horizontalSize);
	idxData.verticalResolution	=	static_cast<uint32>(v6Header->verticalSize);
	idxData.frameRateCode		=	convertFrameRateCode(v6Header->frameRateCode);
	idxData.subFileCount		=	static_cast<uint32>(v6Header->subFileInformationCount);

	assert(idxData.subFileCount < 1000 );

	int64 subFileOffset = v6Header->subFileInformationOffset;
	if( reader->seek( subFileOffset , FileReader::SEEK_FROM_BEGIN) != subFileOffset )
	{
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"[%s] can't seek to [%lld]"), 
					reader->getFileName().c_str(),subFileOffset);
		return false;
	}
	//read sub file information
	VVX_V6_SUBFILE_INFORMATION* pSubFileInfo = new VVX_V6_SUBFILE_INFORMATION[v6Header->subFileInformationCount];
	assert( pSubFileInfo != NULL );

	size_t	readSize = sizeof(VVX_V6_SUBFILE_INFORMATION ) *  v6Header->subFileInformationCount;
	if( reader->read( pSubFileInfo , readSize ) != readSize )
	{
		delete[] pSubFileInfo;
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"read [%s] sub file information failed"), reader->getFileName().c_str() );
		return false;
	}

	IndexData::SubFileInformation info;
	for( uint32 i = 0; i < idxData.subFileCount ; i ++ )
	{
		info.fileExtension		=	reinterpret_cast<const char*>( pSubFileInfo[i].fileExtension );
		info.playtime			=	static_cast<uint32>( pSubFileInfo[i].playTimeInMilliSeconds );
		info.speed.numerator	=	static_cast<SHORT>( pSubFileInfo[i].speed.numerator);
		info.speed.denominator	=	static_cast<SHORT>( pSubFileInfo[i].speed.denominator);
		idxData.subFileInfos.push_back( info );
	}
	delete[] pSubFileInfo;

	return true;
}

bool IndexFileParser::parserVVxVersion7( FileReader* reader , const VVX_INDEX_HEADER& header ,IndexData& idxData )
{
	assert( reader != NULL );
	//sizeof(VVX_V7_INDEX_HEADER) < sizeof(VVX_INDEX_HEADER)
	//so we can perform this conversion
	VVX_V7_INDEX_HEADER* v7Header = reinterpret_cast<VVX_V7_INDEX_HEADER*>( (VVX_INDEX_HEADER*)(&header));
	idxData.muxBitRate			=	static_cast<uint32>(v7Header->streamBitRate);
	idxData.horizonResolution	=	static_cast<uint32>(v7Header->horizontalSize);
	idxData.verticalResolution	=	static_cast<uint32>(v7Header->verticalSize);
	idxData.videoBitRate		=	static_cast<uint32>(v7Header->videoBitRate);
	idxData.frameRateCode		=	convertFrameRateCode(v7Header->frameRateCode);
	idxData.subFileCount		=	static_cast<uint32>(v7Header->subFileInformationCount);

	assert( idxData.subFileCount < 1000 );
	int64	subFileOffset = v7Header->subFileInformationOffset;
	if( reader->seek( subFileOffset , FileReader::SEEK_FROM_BEGIN ) !=  subFileOffset )
	{
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"[%s] can't seek to [%lld]"),
			reader->getFileName().c_str(), subFileOffset );
		return false;
	}

	//read sub file information
	VVX_V6_SUBFILE_INFORMATION* pSubFileInfo = new VVX_V6_SUBFILE_INFORMATION[v7Header->subFileInformationCount];
	assert( pSubFileInfo != NULL );

	size_t	readSize = sizeof(VVX_V6_SUBFILE_INFORMATION ) *  v7Header->subFileInformationCount;
	if( reader->read( pSubFileInfo , readSize ) != readSize )
	{
		delete[] pSubFileInfo;
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"read [%s] sub file information failed"), reader->getFileName().c_str() );
		return false;
	}
	
	IndexData::SubFileInformation info;
	for( uint32 i = 0; i < idxData.subFileCount ; i ++ )
	{
		info.fileExtension		=	reinterpret_cast<const char*>( pSubFileInfo[i].fileExtension );
		info.playtime			=	static_cast<uint32>( pSubFileInfo[i].playTimeInMilliSeconds );
		info.speed.numerator	=	static_cast<SHORT>( pSubFileInfo[i].speed.numerator);
		info.speed.denominator	=	static_cast<SHORT>( pSubFileInfo[i].speed.denominator);
		idxData.subFileInfos.push_back( info );
	}

	delete[] pSubFileInfo;

	return true;
}

bool IndexFileParser::parseVVX( FileReader* iReader ,IndexData& idxData  )
{
	unsigned char szBuffer[64*1024];
	int32 readbufSize = iReader->read(szBuffer,sizeof(szBuffer));

	
	if( readbufSize <= 0 )
	{
		IDXPARSERLOG(ZQ::common::Log::L_INFO,CLOGFMT(IndexFileParser,"failed to read from file[%s]"),
			iReader->getFileName().c_str() );
		return false;
	}

	IDXPARSERLOG(ZQ::common::Log::L_INFO,CLOGFMT(IndexFileParser,"read[%d] bytes from file[%s]"),
		readbufSize , iReader->getFileName().c_str() );

	MemFileReader memreader( szBuffer , readbufSize , iReader->getFileName() );

	MemFileReader* reader = & memreader;

	assert( reader != NULL );
	VVX_INDEX_HEADER vvxHeader = {0};
	if( reader->read( &vvxHeader , sizeof(vvxHeader) ) != sizeof(vvxHeader) )
	{
		return false;
	}
	if( strcmp( reinterpret_cast<const char*>(vvxHeader.signature) , VVX_INDEX_SIGNATURE) !=0 )
	{
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"file[%s] Invalid Vvx Header Signature"),reader->getFileName().c_str());
		return false;
	}

	switch ( vvxHeader.majorVersion )
	{
	case 5:
		if( !parserVVxVersion5( reader , vvxHeader , idxData ) )
			return false;
		break;
	case 6:
		if( !parserVVxVersion6( reader , vvxHeader , idxData ) )
			return false;
		break;
	case 7:
		if( !parserVVxVersion7( reader , vvxHeader , idxData ) )
			return false;
		break;
	default:
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"invalid header version[%d] for file[%s]" ),
				vvxHeader.majorVersion , reader->getFileName().c_str() );
			return false;
		}
	}
	if(idxData.subFileInfos.size() == 0 )
	{
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"invalid vvx data, no subfile playtime information"));
		return false;
	}

	idxData.mainFilePlayTime= idxData.subFileInfos[0].playtime;

	int64 lFileSize = iReader->getFileSize( _mainFileName );
	idxData.mainFileSize = lFileSize;
	if( idxData.mainFilePlayTime <= 0 )
	{
#if 0		
		if( lFileSize <= 0 )
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"can't get [%s]' size"),_mainFileName.c_str());
			return false;
		}
#endif
		if( idxData.muxBitRate <= 0 )
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"invalid vvx bitrate[%d]"),idxData.muxBitRate);
			return false;
		}
		idxData.mainFilePlayTime	=	static_cast<uint32>( lFileSize* 8000 / idxData.muxBitRate );
		IDXPARSERLOG(ZQ::common::Log::L_DEBUG,
			CLOGFMT(IndexFileParser,"calculate playtime [%u] with fileSize[%lld] bitRate[%d]"),
			idxData.mainFilePlayTime,
			lFileSize, idxData.muxBitRate);

	}
	return true;
}

bool IndexFileParser::parseCsicoIndex(ZQ::IdxParser::FileReader *reader, ZQ::IdxParser::IndexData &idxData)
{
	CsicoIndexFileParser parser(_idxParserEnv,_mainFileName);
	return parser.parseCsicoINDEX( reader , idxData);
}



typedef struct _Vv2SubFileInfo 
{
	char			szFileName[256];
	int				numerator;
	unsigned int	denominator;
	short			direction;
	unsigned int	muxBitrate;
}Vv2SubFileInfo ;
typedef std::map< int , Vv2SubFileInfo > Vv2SubFileInfoMap;


static const char*		pHeaderSOH				=	"SOH";
static const size_t		sizeSOH					=	strlen( pHeaderSOH );

static const char*		pHeaderDRIVERTSOVERIP	=	"DRIVERFLAVOR:TSOVERIP";
static const size_t		sizeDRIVERTSOVERIPR		=	strlen( pHeaderDRIVERTSOVERIP );

static const char*		pHeaderNOEMBEDDEDHINTS	=	"NO_EMBEDDED_HINTS";
static const size_t		sizeNOEMBEDDEDHINTS		=	strlen( pHeaderNOEMBEDDEDHINTS );

static const char*		pHeaderINDXRECFMT2		=	"INDXRECFMT:2";
static const size_t		sizeINDXRECFMT2			=	strlen(pHeaderINDXRECFMT2);

static const char*		pHeaderINDXRECFMT3		=	"INDXRECFMT:3";
static const size_t		sizeINDXRECFMT3			=	strlen(pHeaderINDXRECFMT3);

static const char*		pHeaderFILE				=	"FILE";
static const size_t		sizeFILE				=	strlen(pHeaderFILE);

static const char*		pHeaderBITRATE0			=	"BITRATE_HEX:0";
static const size_t		sizeBITRATE0			=	strlen(pHeaderBITRATE0);

static const char*		pHeaderBITRATE			=	"BITRATE_HEX";
static const size_t		sizeBITRATE				=	strlen(pHeaderBITRATE);

static const char*		pHeaderPlayDuration		=	"PLAYDURATIONMSECS_HEX";
static const size_t		sizePlayDuration		=	strlen(pHeaderPlayDuration);

static const char*		pHeaderTBD				=	"TBD";
static const size_t		sizeTBD					=	strlen(pHeaderTBD);

static const char*		pVersion				= "VERSION:";
static const size_t		sizeVersion				= strlen(pVersion);

#ifdef ZQ_OS_MSWIN
#define GET_LINE_STRING(x,y)	if( reader->getLine( szLine , lineStrLen ) == NULL ) \
{\
	IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,#x));\
	##y;\
}
#else
#define GET_LINE_STRING(x,y)	if( reader->getLine( szLine , lineStrLen ) == NULL ) \
{\
	IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,#x));\
	y;\
}
#endif

#define LINE_IS_VERSION(x) ( memcmp( x , pVersion , sizeVersion ) == 0 )

#define LINE_IS_SOH(x)	( memcmp( x , pHeaderSOH , sizeSOH ) == 0 )

#define LINE_IS_DRIVERTSOVERIP(x) ( memcmp( x , pHeaderDRIVERTSOVERIP , sizeDRIVERTSOVERIPR ) == 0 )

#define LINE_IS_NOEMBEDDEDHINTS(x) ( memcmp( x , pHeaderNOEMBEDDEDHINTS , sizeNOEMBEDDEDHINTS ) == 0 )

#define LINE_IS_INDXRECFMT2(x)	( memcmp( x , pHeaderINDXRECFMT2 , sizeINDXRECFMT2 ) == 0 )

#define LINE_IS_INDXRECFMT3(x) ( memcmp( x , pHeaderINDXRECFMT3 , sizeINDXRECFMT3 ) == 0 )

#define LINE_IS_FILE(x)	( memcmp( x , pHeaderFILE , sizeFILE ) == 0 )

#define LINE_IS_BITRATE0(x) ( memcmp( x , pHeaderBITRATE0 , sizeBITRATE0 ) == 0 )

#define LINE_IS_BITRATE(x)	( memcmp( x , pHeaderBITRATE , sizeBITRATE ) == 0 )

#define LINE_IS_DURATION(x)	( memcmp( x , pHeaderPlayDuration , sizePlayDuration ) == 0 )

#define LINE_IS_TBD(x)	( memcmp( x , pHeaderTBD , sizeTBD ) == 0 )

bool IndexFileParser::parseVV2( FileReader* reader ,IndexData& idxData  )
{
	assert( reader != NULL );
	//parse the vv2 file line by line
	char	szLine[1024];
	size_t	lineStrLen = sizeof(szLine);
	
	GET_LINE_STRING ( "Invalid vv2 file ,can't get SOH" , return false );
	if( !LINE_IS_SOH(szLine) )	return false;
	
	GET_LINE_STRING( "Invalid vv2 file,get driverflavor" , return false);
	if( !LINE_IS_DRIVERTSOVERIP(szLine) ) return false;

	for( int i = 0 ;i < 3 ;i ++ )
	{//try to skip no useful information
		GET_LINE_STRING("Invalid vv2 file , can't get more data", return false );
		if( LINE_IS_NOEMBEDDEDHINTS(szLine ) ||
			LINE_IS_INDXRECFMT3(szLine)||
			LINE_IS_INDXRECFMT2(szLine) )
			break;
	}	
	//GET_LINE_STRING("Invalid vv2 file , can't get more data", return false );
	if( LINE_IS_NOEMBEDDEDHINTS(szLine) )
	{
		GET_LINE_STRING("Invalid vv2 file , can't get INDEXFORMAT3 ", return false );
		if( !LINE_IS_INDXRECFMT3(szLine) ) return false;
	}
	else
	{
		if( !LINE_IS_INDXRECFMT2(szLine) ) return false;		
	}

	
	Vv2SubFileInfoMap	subFiles;
	Vv2SubFileInfo		infoSub;

	int subFileId = 0;
	while ( reader->getLine(szLine, lineStrLen) )
	{//read sub file information		
		if( !LINE_IS_FILE(szLine) )
			break;
		std::vector<std::string>	tempVec;
		if(!ZQ::common::stringHelper::SplitString( szLine , tempVec, ":",":","","") && tempVec.size() != 6 )
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"Invalid vv2 subfile information"));
			return false;
		}
		//ignore the leading char '#'
		strncpy( infoSub.szFileName , tempVec[1].c_str() + 1 , sizeof(infoSub.szFileName) -1 );
		sscanf(tempVec[3].c_str(),"%X",&infoSub.numerator);			
		sscanf(tempVec[4].c_str(),"%X",&infoSub.denominator);		
		infoSub.direction	=	stricmp( tempVec[5].c_str() , "F" ) == 0 ? 1 : -1;
		subFileId			=	static_cast<int>( atoi(tempVec[2].c_str()));
		
		if( subFiles.find(subFileId) != subFiles.end() )
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"duplicated sub file id [%d]"),subFileId );
			return false;
		}
		subFiles.insert( Vv2SubFileInfoMap::value_type(subFileId,infoSub) );		
	}
	//GET_LINE_STRING("Invalid vv2 file header",return false);	
	if( !LINE_IS_BITRATE0(szLine) ) return false;
	if( sscanf(szLine,"BITRATE_HEX:0:%X",&idxData.muxBitRate) != 1 )
		return false;

	while ( reader->getLine(szLine, lineStrLen) )
	{
		if( !LINE_IS_BITRATE(szLine) )
			break;
	}
	//about to get the play duration
	//GET_LINE_STRING("Invalid vv2 file header",return false);
	if(!LINE_IS_DURATION(szLine) ) return false;

	char szPlayDuration[256] = {0};
	sscanf(szLine,"PLAYDURATIONMSECS_HEX:%s",szPlayDuration);
	
	if( LINE_IS_TBD(szPlayDuration ) )
	{
		int64 lFileSize = reader->getFileSize( _mainFileName );
		idxData.mainFileSize = lFileSize;
		if( idxData.muxBitRate <= 0 )
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"invalid vv2 bitrate[%d]"),idxData.muxBitRate);
			return false;
		}
		idxData.mainFilePlayTime	=	static_cast<uint32>( lFileSize* 8000 / idxData.muxBitRate );
	}
	else
	{
		sscanf( szPlayDuration , "%X" , &idxData.mainFilePlayTime );
		int64 lFileSize = reader->getFileSize( _mainFileName );
		idxData.mainFileSize = lFileSize;
		if(idxData.mainFilePlayTime <= 0 )
		{			
			if( lFileSize <= 0 )
			{
				IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"can't get [%s]' size"),_mainFileName.c_str());
				return false;
			}
			if( idxData.muxBitRate <= 0 )
			{
				IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"invalid vv2 bitrate[%d]"),idxData.muxBitRate);
				return false;
			}
			idxData.mainFilePlayTime	=	static_cast<uint32>( lFileSize* 8000 / idxData.muxBitRate );
		}
	}

	{
		Vv2SubFileInfoMap::const_iterator it = subFiles.begin();
		for( ; it != subFiles.end() ; it ++)
		{
			IndexData::SubFileInformation sInfo;
			sInfo.fileExtension			= it->second.szFileName;
			sInfo.speed.numerator		= (SHORT)it->second.numerator;
			sInfo.speed.denominator		= (USHORT)it->second.denominator;
			idxData.subFileInfos.push_back( sInfo );
		}
	}
	
	return true;
}

bool IndexFileParser::parseExtension( FileReader* reader , IndexData& idxData )
{
	assert( reader!= NULL );
	char szXmlData[4096];
	int readSize = 0;

	ZQ::common::XMLPreferenceDocumentEx		xmlDoc;
	do 
	{
		readSize = reader->read(szXmlData,sizeof(szXmlData));
		try
		{
			if(!xmlDoc.read(szXmlData, readSize, readSize == sizeof(szXmlData) ? 0 : 1 ))
			{
				IDXPARSERLOG(ZQ::common::Log::L_ERROR,
					CLOGFMT(IndexFileParser,"can't parse xml data for file [%s]'s extension file"),
					_mainFileName.c_str());
				return false;
			}
		}
		catch(ZQ::common::XMLException&)
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,
				CLOGFMT(IndexFileParser,"invalid xml data for file[%s]'s exdtension file"),
				_mainFileName.c_str() );
			return false;
		}

		if(readSize < 0 )
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,
				CLOGFMT(IndexFileParser, "failed to read data for file [%s]'s extension file"),
				_mainFileName.c_str());
			return false;
		}
	} while ( readSize == sizeof(szXmlData) );
	
	ZQ::common::XMLPreferenceEx* preRoot = (ZQ::common::XMLPreferenceEx*)xmlDoc.getRootPreference();
	if(!preRoot )
	{
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,
			CLOGFMT(IndexFileParser,"bad xml schema , can't get root node from the xml for file[%s]'s extension file"),
			_mainFileName.c_str());
		return false;
	}

	bool bParseOk = true;

	ZQ::common::XMLPreferenceEx* pNode = NULL;
	
	char	szTemp[1024];
	for( pNode = preRoot->firstChild("SubFile") ; pNode ; pNode=preRoot->nextChild() )
	{
		if( pNode )
		{
			//get data
			memset( szTemp , 0 , sizeof(szTemp));
			pNode->get("requiredInLeadCopy",szTemp,"0",sizeof(szTemp));
			if ( atoi(szTemp) >= 1 )
			{
				memset( szTemp , 0 , sizeof(szTemp));
				pNode->get("name",szTemp,"",sizeof(szTemp));
				if(strlen(szTemp) > 0 )
				{
					idxData.importantFileSet.push_back(std::string(szTemp));
				}
			}
		}
		else
		{
			// no data
			bParseOk = false;
			break;
		}
		pNode->free();
	}
	
	if(!bParseOk)
	{
		idxData.importantFileSet.clear();
		idxData.referenceCopySet.clear();
		return false;
	}	

	for( pNode = preRoot->firstChild("ReferenceCopy") ; pNode ; pNode=preRoot->nextChild() )
	{
		if(pNode )
		{
			memset( szTemp , 0 , sizeof(szTemp));
			pNode->get("contentName",szTemp,"",sizeof(szTemp));
			if( strlen(szTemp) > 0 )
			{
				idxData.referenceCopySet.push_back( std::string(szTemp) );
			}
		}
		else
		{
			bParseOk = false;
		}
	}
	if(!bParseOk)
	{
		idxData.importantFileSet.clear();
		idxData.referenceCopySet.clear();
		return false;
	}
	
	return true;
}



#define CSICOINDEXFILESIGNATURE	"Video Index File"

CsicoIndexFileParser::CsicoIndexFileParser(IdxParserEnv& env, const std::string& mainFileName )
:_idxParserEnv(env),
_mainFileName(mainFileName)
{//actually mainFileName is a content name
}

CsicoIndexFileParser::~CsicoIndexFileParser( )
{

}

bool CsicoIndexFileParser::getTag( FileReader* reader , CsicoIndexFileTagAndLength& tal )
{
	if( reader->read(&tal,sizeof(tal)) != sizeof(tal))
	{
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"insufficient data to parse of [%s]"),reader->getFileName().c_str());
		return false;
	}
	while( *((char*)&tal) == 0x00 )
	{
		CsicoIndexFileTagAndLength tmpTal;
		unsigned char c;
		if(reader->read( &c , sizeof(c))!=sizeof(c))
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"insufficient data to parse of [%s]"),reader->getFileName().c_str());
			return false;
		}
		char* pTemp = (char*)&tal;
		pTemp ++;
		memcpy( &tmpTal ,pTemp , sizeof(tal) -1 );
		pTemp = (char*)&tmpTal;
		pTemp += sizeof(tmpTal) -1;
		memcpy( pTemp,&c ,sizeof(c));
		memcpy( &tal,&tmpTal,sizeof(tal));
	}
	return true;
}

std::string	CsicoIndexFileParser::readString( FileReader* reader , size_t size )
{
	_tmpString.clear();
	size_t originalSize = size;
	do
	{
		size_t readSize = size > sizeof(_tmpBuffer) ? sizeof(_tmpBuffer) : size;
		size_t retSize = 0;
		if( (retSize = reader->read(_tmpBuffer,readSize)) != readSize )
		{
			throw "not enough data";
		}
		size -= retSize;
		_tmpString.append(_tmpBuffer,retSize);
	}while( size> 0 );
	if( originalSize > 0 )
		_tmpString.resize( _tmpString.length()-1);
	return _tmpString;
}

bool CsicoIndexFileParser::parseCsicoINDEX( FileReader* reader , IndexData& idxData )
{
	CsicoIndexFileHeader	fileHeader;
	memset(&fileHeader , 0, sizeof(fileHeader));
	if( reader->read(&fileHeader,sizeof(fileHeader)) != (int32)sizeof(fileHeader))
	{
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,
			CLOGFMT(IndexFileParser,"failed to read CsicoIndexFileHeader for file[%s]"),
			reader->getFileName().c_str() );
		return false;
	}
	//check signature
	if( strcmp( CSICOINDEXFILESIGNATURE, fileHeader.signature ) != 0 )
	{
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,
			CLOGFMT(IndexFileParser,"bad signature for file[%s]"),
			reader->getFileName().c_str() );
		return false;
	}
	
	if ( fileHeader.majorVersion == 1 && fileHeader.minorVersion == 0 )
	{
		return parseCsicoINDEX11( reader , idxData );
	}
	else
	{
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,
			CLOGFMT(IndexFileParser,"Unsupport version[%d.%d] for file[%s]"),
			fileHeader.majorVersion,
			fileHeader.minorVersion,
			reader->getFileName().c_str() );
		return false;
	}
}

#define	CSICOINDEX_TAG_SECTIONHEADERTAG								0x11000001

#define CSICOINDEX_TAG_ASSETINFOSECTION								0x110001FF

#define CSICOINDEX_TAG_SUBFILESECTION								0x110005FF

#define CSICOINDEX_TAG_MUXBITRATE									0x13000109
#define CSICOINDEX_TAG_HORIZONTALSIZE								0x1300010C
#define CSICOINDEX_TAG_VERTICALSIZE									0x1300010D

#define CSICOINDEX_TAG_SUBFILECOUNT									0x13000117

#define CSICOINDEX_TAG_SUBFILEPLAYTIME								0x1300050A
#define CSICOINDEX_TAG_SUBFILESPEEDNUMERATOR						0x1200050C
#define CSICOINDEX_TAG_SUBFILESPEEDDENOMINATOR						0x1300050D
#define CSICOINDEX_TAG_SUBFILENAME									0x15000502
#define CSICOINDEX_TAG_SUBFILEINDEX									0x13000501

std::string CsicoIndexFileParser::getSubType( FileReader* reader ,const std::string &sxmlContent)
{
	SimpleXMLParser parser;
	std::string xmlContent = "<root>" ;
	xmlContent = xmlContent + sxmlContent;
	//xmlContent.resize( strlen(xmlContent.c_str() ) );
	xmlContent = xmlContent + "</root>";
	try
	{
		parser.parse( xmlContent.c_str() ,static_cast<int>( xmlContent.length() ), 1 );
		const SimpleXMLParser::Node& root = parser.document();
		const SimpleXMLParser::Node* pSubType = findNode(&root,"root/SubType");
		if( pSubType )
		{
			if( pSubType->content.length() > 0 && pSubType->content.at(0) == '.' )
			{
				return pSubType->content;
			}
			else
			{
				return "." + pSubType->content;
			}
		}
		else
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"failed to parse subfile extension for [%s] with [%s] , can't get subtype"),
				reader->getFileName().c_str() ,xmlContent.c_str() );
		}
	}
	catch( ZQ::common::ExpatException& )
	{
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"failed to parse subfile extension for [%s] with [%s]"),
			reader->getFileName().c_str() ,xmlContent.c_str() );
	}
	return "";
}

bool CsicoIndexFileParser::parseSubFiles( FileReader* reader  , IndexData& idxData)
{
	CsicoIndexFileTagAndLength tal;

	int	subFileIndex = -1;
	IndexData::SubFileInformation subInfo;

		bool bIndexOk			= false;
		bool bSpeedNumeratorOk	= false;
		bool bSpeedDenominatorOk = false;
		bool bNameOk			= false;
		bool bPlayTimeOK = false;
		while( !(bIndexOk && bSpeedNumeratorOk && bSpeedDenominatorOk && bNameOk && bPlayTimeOK ) )
		{
			if( !getTag( reader,tal ) )
			{
				IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"insufficient data to parse of [%s]"),reader->getFileName().c_str());
				return false;
			}
			switch ( tal.tagId )
			{
			case CSICOINDEX_TAG_SUBFILEINDEX:
				{
					subFileIndex = readAndReinterpretAs<uint32>( reader , tal.length );					
					bIndexOk = true;
				}
				break;
			case CSICOINDEX_TAG_SUBFILEPLAYTIME:
				{
					subInfo.playtime	= readAndReinterpretAs<uint32>( reader , tal.length );
					bPlayTimeOK = true;
				}
				break;
			case CSICOINDEX_TAG_SUBFILESPEEDDENOMINATOR:
				{
					subInfo.speed.denominator = (USHORT)readAndReinterpretAs<uint32>( reader , tal.length );
					 bSpeedDenominatorOk = true;
				}
				break;
			case CSICOINDEX_TAG_SUBFILESPEEDNUMERATOR:
				{
					subInfo.speed.numerator = (SHORT)readAndReinterpretAs<uint32>( reader , tal.length );
					bSpeedNumeratorOk = true;
				}
				break;
			case CSICOINDEX_TAG_SUBFILENAME:
				{
					subInfo.fileExtension = getSubType( reader , readString( reader , tal.length));
					bNameOk = true;
				}
				break;
			default:
				{
					reader->seek( tal.length ,FileReader::SEEK_FROM_CUR  );
				}
				break;
			}
		}
		if ( subFileIndex == 0 )
		{
			idxData.mainFilePlayTime	= subInfo.playtime;

			idxData.mainFileName		= idxData.mainFileName + subInfo.fileExtension;
			
		}
		idxData.subFileInfos.push_back(subInfo);
	
	return true;
}
bool CsicoIndexFileParser::parseAssetInfo(ZQ::IdxParser::FileReader *reader, ZQ::IdxParser::IndexData &idxData)
{
	CsicoIndexFileTagAndLength tal;
	bool bMuxRateOk = false;
	bool bVerticalSizeOk = false;
	bool bHorizontalSizeOk = false;
	bool bSubFileCountOk = false;

	while( !( bMuxRateOk && bVerticalSizeOk && bHorizontalSizeOk && bSubFileCountOk ) )
	{
		if( !getTag( reader,tal ) )
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"insufficient data to parse of [%s]"),reader->getFileName().c_str());
			return false;
		}
		switch( tal.tagId )
		{
		case CSICOINDEX_TAG_MUXBITRATE:
			{
				idxData.muxBitRate = readAndReinterpretAs<uint32>(reader,tal.length);
				bMuxRateOk = true;
			}
			break;
		case CSICOINDEX_TAG_HORIZONTALSIZE:
			{
				idxData.horizonResolution = readAndReinterpretAs<uint16>(reader,tal.length);
				bHorizontalSizeOk = true;
			}
			break;
		case CSICOINDEX_TAG_VERTICALSIZE:
			{
				idxData.verticalResolution = readAndReinterpretAs<uint16>(reader , tal.length);
				bVerticalSizeOk = true;
			}
			break;
		case CSICOINDEX_TAG_SUBFILECOUNT:
			{
				idxData.subFileCount = readAndReinterpretAs<uint8>(reader,tal.length);
				bSubFileCountOk = true;
				_subFileCount	= idxData.subFileCount;
			}
			break;
		default:
			{
				reader->seek( tal.length ,FileReader::SEEK_FROM_CUR  );
			}
		}
	}
	return true;
}

bool CsicoIndexFileParser::parseSection(  FileReader* reader , IndexData& idxData )
{
	CsicoIndexFileTagAndLength tal;
	int subFileCount = 0;
	bool bAssetInfoOk = false;
	bool bSubFileInfoOk = false;
	while( !bAssetInfoOk  || !bSubFileInfoOk )
	{
		if( !getTag( reader,tal ) )
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"insufficient data to parse of [%s]"),reader->getFileName().c_str());
			return false;
		}
		
		switch ( tal.tagId )
		{
		case CSICOINDEX_TAG_ASSETINFOSECTION:
			{
				if( !parseAssetInfo(reader , idxData ))
					return false;
				else
					bAssetInfoOk = true;

			}
			break;

		case CSICOINDEX_TAG_SUBFILESECTION:
			{
				if( !parseSubFiles( reader , idxData ))
				{
					return false;
				}
				else
				{
					if( ++subFileCount >= _subFileCount )
						bSubFileInfoOk = true;
				}
			}
			break;
		default:
			{
				reader->seek( tal.length ,FileReader::SEEK_FROM_CUR  );
			}
			break;
		}
	}
	return true;
}

bool CsicoIndexFileParser::parseCsicoINDEX11(ZQ::IdxParser::FileReader *reader, ZQ::IdxParser::IndexData &idxData)
{
	bool bContinue = true;
	CsicoIndexFileTagAndLength tal;
	
	while ( bContinue)
	{
		//get the tag and length
		if( !getTag( reader,tal ) )
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"insufficient data to parse of [%s]"),reader->getFileName().c_str());
			return false;
		}
		try
		{
			switch ( tal.tagId )
			{
			case CSICOINDEX_TAG_SECTIONHEADERTAG:
				if(! parseSection( reader, idxData ) )
					return false;
				bContinue = false;
				break;
			default:
				reader->seek( tal.length ,FileReader::SEEK_FROM_CUR  );
				break;
			}
		}
		catch( const char* reason )
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"failed to parse[%s] because [%s]"),
				reader->getFileName().c_str(),reason);
			return false;
		}
	}
	//get main file name
	if( idxData.subFileInfos.size() > 0 )
	{
		idxData.mainFileSize = reader->getFileSize( _mainFileName + idxData.subFileInfos[0].fileExtension );
	}
	

	return true;
}


}}//namespace ZQ::IdxParser
