#include <sys/stat.h> 
#include <sys/types.h>
#include <map>
#include <sstream>
#include <strHelper.h>
#include "IndexFileParser.h"
#include <XMLPreferenceEx.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "SimpleXMLParser.h"
#include <algorithm>
#include <cmath>

#ifndef EXCLUDE_VSTRM_API
	#include <vstrmfp.h>
	#include <vsiolib.h>
#endif

#ifdef ZQ_OS_LINUX
#include <unistd.h>
#endif

// too danger
#define IDXPARSERLOG	if( _idxParserEnv.GetLogger() ) (*_idxParserEnv.GetLogger())



namespace ZQ
{
namespace IdxParser
{

std::string IndexData::baseInfoToXML() const
{
	std::ostringstream result;
	char buf[200];
	memset(buf,0,sizeof(buf));
	//sprintf(buf,"<Residential recording=\"%d\" />\n",bPWE);
	//result<<buf;
	memset(buf,0,sizeof(buf));
	float playTime = (float)mainFilePlayTime/1000;
	sprintf(buf,"<EncodingInfo playTime=\"%.3f\" muxBitrate=\"%d\" videoBitrate=\"%d\" audioBitrate=\"%d\" horizonalResolution=\"%d\" verticalResolution=\"%d\" frameRate=\"%s\" />\n",playTime,muxBitRate,videoBitRate,audioBitRate,horizonResolution,verticalResolution,getFrameRateString());
	result<<buf;
	return result.str();
}

std::string IndexData::memberFileToXML() const
{
	std::ostringstream result;
	result << "<Members>\n";
	char buf[200];
	int i = 0;
	float spd = 0.0;
	while(i < getSubFileCount())
	{
		memset(buf,0,sizeof(buf));
		SubFileInformation info;
		getSubFileInfo( i , info);
		std::string extName = info.fileExtension.empty() ? "" : 
			( (info.fileExtension.at(0) == '.') ? info.fileExtension.substr(1) : info.fileExtension);
		if(info.speed.denominator)
			spd = (float)(getSubFileSpeed(i).numerator)/getSubFileSpeed(i).denominator;		
		sprintf(buf,"<SubFile extName=\"%s\" speed=\"%.1f\" startOffset=\"%llu\" endOffset=\"%llu\" />\n",
			extName.c_str(), spd, info.startingByte, info.endingByte);
		result << buf;
		i++;
	}
	result << "</Members>";
	return result.str();
}

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
mFileName(fileName),
mFileSize(0)
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

int64 MemFileReader::seek( int64 offset , SeekPosition begin ) 
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
	return mFileSize;
}
void MemFileReader::setRealFileSize( int64 size )
{
	mFileSize = size;
}

const char* MemFileReader::getLastErrorDesc( char* buf , size_t  )
{
	buf[0]=0;// no error
	return buf;
}

const std::string&	MemFileReader::getFileName( ) const
{
	return mFileName;
}

bool MemFileReader::open( const std::string&  ) 
{
	mCurPos = 0;
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
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(FileReaderVstrmI,"Can't open  file[%s] because[%s] with VsOpenEx"),
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
uint32 FileReaderVstrmI::convertBeginPos( SeekPosition pos )
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

int64 FileReaderVstrmI::seek( int64 offset , SeekPosition begin ) 
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
	_fileHandle = NULL;
}

FileReaderNative::~FileReaderNative()
{
	close();
}
uint32 FileReaderNative::convertBeginPos( SeekPosition pos )
{
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
}

bool FileReaderNative::open( const std::string& fileName )
{
	assert( !fileName.empty() );
	close();
	_fileHandle = fopen(fileName.c_str(),"rb");
	if(_fileHandle == NULL)
	{
		IDXPARSERLOG(ZQ::common::Log::L_WARNING, CLOGFMT(FileReaderNative,"failed to open [%s] because [%s]"),
			fileName.c_str() , 
			getLastErrorDesc(_szBuf,sizeof(_szBuf)-1  ) );
		return false;
	}
	_fileName = fileName;
	return true;
}

bool FileReaderNative::close( )
{
	if(_fileHandle != NULL ) {
		fclose(_fileHandle);
		_fileHandle = NULL;
	}
	return true;
}
int32 FileReaderNative::read( void* buf , size_t count ,size_t size/* = 1 */) 
{
	if( _fileHandle == NULL )
	{
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(FileReaderNative,"please open the index file first before use it") );
		return -1;
	}

	size_t ret = fread(buf,1,count,_fileHandle);
	if(ret != count)
	{
		if(!feof(_fileHandle)){
			int err = ferror(_fileHandle);
			if( err != 0) {
				IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(FileReaderNative,"failed to read file [%s] because [%s]"),
					_fileName.c_str(),
					getLastErrorDesc(_szBuf,sizeof(_szBuf)-1));
				return -1;
			}
		}
	}
	return static_cast<int32>( ret );
}

int32 FileReaderNative::write( const void* buf ,size_t count , size_t size/* = 1 */) 
{
	if( _fileHandle == NULL )
	{
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(FileReaderVstrmI,"please open the index file first before use it") );
		return -1;
	}
	size_t ret = fwrite(buf,1,count,_fileHandle);
	if( ret != count )
	{
		int err = ferror(_fileHandle);
		if( err != 0 ) {
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(FileReaderNative,"failed to write file because [%s]"),
				getLastErrorDesc(_szBuf,sizeof(_szBuf)-1));
			return -1;
		}
	}
	return static_cast<int32>(ret);
}

int64 FileReaderNative::seek( int64 offset , SeekPosition begin )
{
	if(_fileHandle == NULL)
		return 0;
#ifdef ZQ_OS_MSWIN
	_fseeki64(_fileHandle, offset, convertBeginPos(begin));	
#else
	fseek(_fileHandle, offset, convertBeginPos(begin));
#endif
	return tell();
}

int64 FileReaderNative::tell( )
{
	if(_fileHandle == NULL)
		return 0;
#ifdef ZQ_OS_MSWIN
	return _ftelli64(_fileHandle);
#else
	return ftell(_fileHandle);
#endif//
}
const char* FileReaderNative::getLastErrorDesc( char* buf , size_t bufSize ) 
{
	if( bufSize <= 0 )return NULL;
	buf[0] = '\0';
	buf[bufSize-1] = '\0';

	int err = 0;
	if(_fileHandle == NULL) {
		err = errno;
	} else {
		err = ferror(_fileHandle);
	}
#ifdef ZQ_OS_MSWIN
	return strerror(err);
#elif defined ZQ_OS_LINUX
	strerror_r(err,buf,bufSize);
	return buf;
#endif
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
	struct __stat64 st;
	if(_stat64(filePathName.c_str(),&st)!=0)
		return 0;
#else
	struct stat st;
	if(stat(filePathName.c_str(),&st)!=0)
		return 0;
#endif//
	return (int64)st.st_size;
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
	assert( LoadMethod != NULL );	

	//
	ULONG	bitRate = 0;
	ULONG	duration = 0;
	VOD_ASSET_INDEX_TYPE type = VOD_ASSET_INDEX_IS_VVX;
	VOD_ASSET_LOCATION location = bLocalContent ? VOD_ASSET_LOCATION_LOCAL : VOD_ASSET_LOCATION_REMOTE;


	VSTATUS ret = LoadMethod->LoadBriefAssetInfo( _idxParserEnv.GetVstrmHandle() , 
																mainFileName.c_str() ,
																&bitRate ,
																&duration,
																&type,
																location );
	if( ret != VSTRM_SUCCESS )
	{
		IDXPARSERLOG(ZQ::common::Log::L_WARNING,CLOGFMT(VstrmClassLoadBriefAssetInfo,"failed to parse[%s]' index ,use LoadFullAssetInfo"),mainFileName.c_str() );
		return parseIndexUsingVstrmAPILoadAll( reader , mainFileName , idxData );
		
		/*
		char szErrorBuf[1024]={0};
		VstrmClassGetErrorText( _idxParserEnv.GetVstrmHandle() , ret , szErrorBuf , sizeof(szErrorBuf) );
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(PVstrmClassLoadBriefAssetInfo,"failed to parse [%s]'s index data because[%s]"),
			mainFileName.c_str() ,
			szErrorBuf );
		_lastError = szErrorBuf;
#if defined _DEBUG || defined DEBUG
		printf("failed to parse index data for [%s] because [%s]\n",
			mainFileName.c_str() ,
			szErrorBuf );
#endif
		return false;
		*/
	}
	idxData.muxBitRate			= bitRate;
	idxData.mainFilePlayTime	= duration;
	if( bitRate <= 0 || duration <= 0 )
	{
		IDXPARSERLOG(ZQ::common::Log::L_WARNING,
			CLOGFMT(PVstrmClassLoadBriefAssetInfo,"invalid index data for [%s], playTime[%u] muxBitrate[%u]"),
			mainFileName.c_str() ,
			duration,
			bitRate );
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
	
	return true;
}

bool IndexFileParser::parseIndexUsingVstrmAPILoadAll(  FileReader*  , const std::string& mainFileName , IndexData& idxData )
{
	VstrmLoadAssetInfo* LoadMethod = _idxParserEnv.getLoadInfoMethod();
	assert( LoadMethod != NULL );	

	//
	VOD_ASSET_INFO assetInfo;
	memset(&assetInfo,0,sizeof(assetInfo));
	ULONG			infoSize =sizeof(assetInfo);
	
	VSTATUS ret = LoadMethod->LoadFullAssetInfo( _idxParserEnv.GetVstrmHandle() , 
												mainFileName.c_str() ,
												&assetInfo ,
												&infoSize );
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
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,
			CLOGFMT(parseIndexUsingVstrmAPILoadAll,"incorrect index data for [%s], no subfile is found"),
			mainFileName.c_str() );		
		return false;
	}
	if( assetInfo.subFile[0].playTimeInMilliSeconds <= 0 ||
		assetInfo.subFile[0].bitRate <= 0 )
	{
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,
			CLOGFMT(parseIndexUsingVstrmAPILoadAll,"invalid index data for [%s], playTime[%u] muxBitrate[%u]"),
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
	const char*					_indexExtension;
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
		{ ".index" ,	&IndexFileParser::parseCsicoIndex ,		IndexData::INDEX_TYPE_VVC },
//		{ ".INDEX" ,	&IndexFileParser::parseCsicoIndex ,		IndexData::INDEX_TYPE_VVC },
		{ ".VVX" ,		&IndexFileParser::parseVVX ,			IndexData::INDEX_TYPE_VVX },
//		{ ".vvx" ,		&IndexFileParser::parseVVX ,			IndexData::INDEX_TYPE_VVX },
		{ ".VV2" ,		&IndexFileParser::parseVV2 ,			IndexData::INDEX_TYPE_VV2 },
//		{ ".vv2" ,		&IndexFileParser::parseVV2 ,			IndexData::INDEX_TYPE_VV2 },
	};

	IDXPARSERLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(parseIndexFile,"Parse Content[%s] with indexFile[%s]"),
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
			for( int id = 0 ;id < (int)(sizeof(funcRoute)/sizeof(funcRoute[0])) ; id ++ )
			{
				if( strcmp(funcRoute[id]._indexExtension,extName.c_str() ) == 0 )
				{
					if( _idxParserEnv.skipZeroByteFile() && ( reader->getFileSize(indexFileName , true ) <= 0))
					{
						IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(parseIndexFile,"Parse Content[%s] with indexFile[%s] , but index's size is 0"),
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
		else
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,
				CLOGFMT(IndexFileParser,"failed to parse index file [%s]"),indexFileName.c_str());
			return false;
		}
	}
	else
	{
		
		std::string idxFileName;
		for( int id = 0 ;id < (int)(sizeof(funcRoute)/sizeof(funcRoute[0])) ;id ++ )
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
					IDXPARSERLOG(ZQ::common::Log::L_DEBUG,
						CLOGFMT(IndexFileParser,"failed to parse index file [%s] for content[%s]"),
						idxFileName.c_str() , mainFileName.c_str() );
					return false;
				}
			}
		}
		IDXPARSERLOG(ZQ::common::Log::L_WARNING,
			 				CLOGFMT(IndexFileParser,"can't parse index file for content[%s]"),
			 				mainFileName.c_str() );
		return false;
	}
	return false;
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
			IDXPARSERLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(ParseIndexFileFromVstrm,"Parse Content[%s] using API{VstrmClassLoadBriefAssetInfo}"),mainFileName.c_str());
			return parseIndexUsingVstrmAPILoadBrief( &reader , mainFileName , idxData , bLocalContent );
		}
		else
		{
			IDXPARSERLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(ParseIndexFileFromVstrm,"Parse Content[%s] using API{VstrmClassLoadFullAssetInfo}"),mainFileName.c_str());
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

bool IndexFileParser::ParseIndexFromMemory( const std::string& mainFilePathName, IndexData& idxData, const char* buffer, size_t size)
{
	MemFileReader reader((const unsigned char*)buffer,size,mainFilePathName);
	return parseIndexFile( &reader, mainFilePathName, idxData , false , "" );
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
	if( reader->read( pSubInfo , readSize ) != (int)readSize )
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
	if( reader->read( pSubFileInfo , readSize ) != (int)readSize )
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
	if( reader->read( pSubFileInfo , readSize ) != (int)readSize )
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
	VVX_INDEX_HEADER vvxHeader;
	memset(&vvxHeader , 0 , sizeof(vvxHeader));	
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
			IDXPARSERLOG(ZQ::common::Log::L_WARNING, CLOGFMT(IndexFileParser,"can't get [%s]' size"),_mainFileName.c_str());
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

bool IndexFileParser::parseCsicoIndex(ZQ::IdxParser::FileReader *iReader, ZQ::IdxParser::IndexData &idxData)
{
	CsicoIndexFileParser parser(_idxParserEnv,_mainFileName);
	
	unsigned char szBuffer[64*1024];
	int32 readbufSize = iReader->read(szBuffer,sizeof(szBuffer));

	if( readbufSize <= 0 )
	{
		IDXPARSERLOG(ZQ::common::Log::L_INFO,CLOGFMT(IndexFileParser,"failed to read from file[%s]"), iReader->getFileName().c_str() );
		return false;
	}

	IDXPARSERLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(IndexFileParser,"read[%d] bytes from file[%s]"), readbufSize , iReader->getFileName().c_str() );

	MemFileReader memreader( szBuffer , readbufSize , iReader->getFileName() );	

	MemFileReader* reader = &memreader;

	//get main file name
	
	 if( !parser.parseCsicoINDEX( reader , idxData) )
		return false;
	
	 if( idxData.subFileInfos.size() > 0 )
	 {
		 idxData.mainFileSize = iReader->getFileSize( _mainFileName + idxData.subFileInfos[0].fileExtension );
		 if( idxData.muxBitRate > 0 && idxData.mainFilePlayTime <= 0 )
		 {
			 idxData.mainFilePlayTime = (uint32)(idxData.mainFileSize * 8000 / idxData.muxBitRate);
		 }
		 for( int i = 1; i < (int)idxData.subFileInfos.size() ; i++ )
		 {
			 idxData.subFileInfos[i].fileSize = iReader->getFileSize( _mainFileName + idxData.subFileInfos[0].fileExtension );
		 }
	 }
	 return true;
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
				IDXPARSERLOG(ZQ::common::Log::L_WARNING, CLOGFMT(IndexFileParser,"can't get [%s]' size"),_mainFileName.c_str());
				return false;
			}
			if( idxData.muxBitRate <= 0 )
			{
				IDXPARSERLOG(ZQ::common::Log::L_ERROR, CLOGFMT(IndexFileParser,"invalid vv2 bitrate[%d]"),idxData.muxBitRate);
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

bool CsicoIndexFileParser::getTag( FileReader* reader , IndexRecordHeader& tal ) {
	do {
		if( reader->read(&tal,sizeof(tal)) != sizeof(tal))
		{
			return false;
		}
		if(tal.type != 0x00)
			return true;
		do 
		{
			if(tal.length != 0x00) {
				reader->seek(-1,FileReader::SEEK_FROM_CUR);
				if( reader->read(&tal,sizeof(tal)) != sizeof(tal))
				{
					return false;
				}
				return true;
			}
			// it's not effciently to read data like this.
			// may using it at memory file reader
			if(reader->read(&tal.length,sizeof(tal.length))!= sizeof(tal.length))
				return false;
		} while (true);
	}while(true);	
	return true;
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

#define	CSICOINDEX_TAG_SECTIONHEADERTAG								0x11000001

#define CSICOINDEX_TAG_INDEXHEADERCHECK                             0x13000002

#define CSICOINDEX_TAG_ASSETINFOSECTION								0x110001FF

#define CSICOINDEX_TAG_TRANSPORT_STREAM_INFO                        0x110002FF

#define CSICOINDEX_TAG_ELEMSTREAMINFO                               0x110003FF

#define CSICOINDEX_TAG_PSIINFO                                      0x110004FF

#define CSICOINDEX_TAG_SUBFILESECTION								0x110005FF

#define CSICOINDEX_TAG_ZEROMOTION                                   0x110006FF

#define CSICOINDEX_TAG_RANDOM_INDEX                                 0x110007FF

#define CSICOINDEX_TAG_INDEX_HEADER_CHECKSUM                        0x13000002
#define CSICOINDEX_TAG_CREATION_VENDOR_ID                           0x13000101
#define CSICOINDEX_TAG_ASSET_INGEST_TIME                            0x15000102
#define CSICOINDEX_TAG_INDEX_CREATION_TIME                          0x15000103
#define CSICOINDEX_TAG_PROGRAM_VERSION                              0x15000104
#define CSICOINDEX_TAG_ASSER_IDENTIFIER                             0x1500011D
#define CSICOINDEX_TAG_SOURCE_FILE_NAME                             0x15000105
#define CSICOINDEX_TAG_FILE_WRAPPER_TYPE                            0x12000106
#define CSICOINDEX_TAG_TRANSPORT_PACKET_SIZE                        0x13000107
#define CSICOINDEX_TAG_SYNC_BYTE_OFFSET                             0x13000108
#define CSICOINDEX_TAG_TRANSPORT_STREAM_TYPE                        0x1300010A
#define CSICOINDEX_TAG_VIDEO_BIT_RATE                               0x1300010B
#define CSICOINDEX_TAG_FRAME_RATE_TICKS                             0x1300010E
#define CSICOINDEX_TAG_FRAME_RATE_TIME_SCALE                        0x1300010F
#define CSICOINDEX_TAG_MAX_GOP_SIZE                                 0x13000110
#define CSICOINDEX_TAG_FIXED_PTS_DTS_INTERVAL                       0x13000111
#define CSICOINDEX_TAG_PTS_DTS_CALCULATION                          0x13000112
#define CSICOINDEX_TAG_INITIAL_DECODING_DELAY                       0x13000113
#define CSICOINDEX_TAG_TRANSPORT_STREAM_COUNT                       0x13000115
#define CSICOINDEX_TAG_ELEMENTARY_STREAM_COUNT                      0x13000116
#define CSICOINDEX_TAG_PCR_PACKET_INTERVAL                          0x13000118
#define CSICOINDEX_TAG_SCTE35_RECORD_COUNT                          0x13000119
#define CSICOINDEX_TAG_OPEN4WRITE                                   0x1400011B

#define CSICOINDEX_TAG_MUXBITRATE									0x13000109
#define CSICOINDEX_TAG_HORIZONTALSIZE								0x1300010C
#define CSICOINDEX_TAG_VERTICALSIZE									0x1300010D

#define CSICOINDEX_TAG_SUBFILECOUNT									0x13000117

#define CSICOINDEX_TAG_SUBFILEPLAYTIME								0x1300050A
#define CSICOINDEX_TAG_SUBFILEDIRECTION                             0x1300050B
#define CSICOINDEX_TAG_SUBFILESPEEDNUMERATOR						0x1200050C
#define CSICOINDEX_TAG_SUBFILESPEEDDENOMINATOR						0x1300050D
#define CSICOINDEX_TAG_SUBFILEIFRAMECOUNT                           0x1300050E
#define CSICOINDEX_TAG_SUBFILEPFRAMECOUNT                           0x1300050F
#define CSICOINDEX_TAG_SUBFILEBFRAMECOUNT                           0x13000510
#define CSICOINDEX_TAG_SUBFILETYPE                                  0x13000503
#define CSICOINDEX_TAG_SUBFILENAME									0x15000502
#define CSICOINDEX_TAG_SUBFILEINDEX									0x13000501
#define CSICOINDEX_TAG_SUBFILE_STARTBYTE							0x13000504
#define CSICOINDEX_TAG_SUBFILE_ENDBYTE								0x13000505
#define CSICOINDEX_TAG_SUBFILE_STARTPCR                             0x13000506
#define CSICOINDEX_TAG_SUBFILE_ENDPCR                               0x13000507
#define CSICOINDEX_TAG_SUBFILE_STARTCONTINUITY                      0x13000508
#define CSICOINDEX_TAG_SUBFILE_ENDCONTINUITY                        0x13000509
#define CSICOINDEX_TAG_SUBFILE_CHECKSUM                             0x13000511

#define CSICOINDEX_TAG_IDXRATE                                      0x13000701
#define CSICOINDEX_TAG_WORSTCASE                                    0x13000702

#define CSICOINDEX_TAG_TRANSSTREAM_PROGRAM_NUMBER                   0x13000201
#define CSICOINDEX_TAG_TRANSSTREAM_PROGRAMMAPTABLE                  0x13000202
#define CSICOINDEX_TAG_TRANSSTREAM_PCR_PID                          0x13000203
#define CSICOINDEX_TAG_TRANSSTREAM_VIDEO_PID                        0x13000204
#define CSICOINDEX_TAG_TRANSSTREAM_AUDIO_PID                        0x13000205

#define CSICOINDEX_TAG_ZEROMOTIONPFRAME                             0x16000602
#define CSICOINDEX_TAG_ZEROMOTIONBFRAME                             0x16000601

#define CSICOINDEX_TAG_ELEMSTREAMTYPE                               0x13000301
#define CSICOINDEX_TAG_ELEMSTREAPID                                 0x13000303
#define CSICOINDEX_TAG_ELEMSTREAMSEQ                                0x16000305 
#define CSICOINDEX_TAG_ELEMSTREAMEXTSEQ                             0x16000306

#define CSICOINDEX_TAG_PROGASSOCIATIONTB                            0x16000401
#define CSICOINDEX_TAG_PROGCondTB                                   0x16000402
#define CSICOINDEX_TAG_PROGMAPTB                                    0x16000403

bool CsicoIndexFileParser::skipFileHeader(FileReader* reader ) {
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

	if ( fileHeader.majorVersion != 1  )
	{
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,	CLOGFMT(IndexFileParser,"Unsupport version[%d.%d] for file[%s]"),
			fileHeader.majorVersion, fileHeader.minorVersion, reader->getFileName().c_str() );
		return false;	
	}
	if( !(fileHeader.minorVersion == 0 || fileHeader.minorVersion == 1) )
	{
		IDXPARSERLOG(ZQ::common::Log::L_WARNING, CLOGFMT(IndexFileParser,"unexpect version[%d.%d] for file[%s]"),
			fileHeader.majorVersion, fileHeader.minorVersion, reader->getFileName().c_str() );
	}
	CsicoIndexFileTagAndLength tal;
	if( !getTag( reader,tal ) )
	{
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"insufficient data to parse of [%s]"),reader->getFileName().c_str());
		return false;
	}
	if(tal.tagId != CSICOINDEX_TAG_SECTIONHEADERTAG ) {
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"[%s] invalid tagId,except 0x11000001, got 0x%x"),
			reader->getFileName().c_str(),tal.tagId);
		return false;
	}
	reader->seek( tal.length ,FileReader::SEEK_FROM_CUR  );
	return true;
}

bool CsicoIndexFileParser::parseIndexRecord( FileReader* reader, IndexRecord& record ) {
	IndexData indexHeader;
	if( record.lastParsedOffset() == 0 ) {
		if(!parseCsicoINDEX(reader,indexHeader))
			return false;
		reader->seek(0, FileReader::SEEK_FROM_BEGIN);
		size_t count = indexHeader.getSubFileCount();
		if(count>1)
			count -= 1; //exclude normal play file
		record.subfileCount(count);
		record.setRecSize(count);
        record.setMainPlayTime(indexHeader.getPlayTime());
        record.setMainExtension(indexHeader.getSubFileName(0));//get mainfile extension
        IDXPARSERLOG(ZQ::common::Log::L_INFO,CLOGFMT(IndexFileParser,"mainfilesize[%lld],[%x]"),indexHeader.getMainFileSize(),indexHeader.getMainFileSize());
	}
	size_t subFileCount = record.subfileCount();
	if( record.lastParsedOffset() == 0) {
		if(!skipFileHeader(reader)) {
			return false;
		}
	} else {
		if( record.lastParsedOffset() != reader->seek(record.lastParsedOffset(),FileReader::SEEK_FROM_BEGIN) ) {
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"failed to seek to [%llu], file content may corrupt"),
				reader->getFileName().c_str() );
			return false;
		}
	}
	// now we are in the index record territory
	char tempBuffer[0x1B], tmpBufTrick[0x09];
	while(true) {
		IndexRecordHeader recHeader;
		if( !getTag(reader, recHeader) ) {
			break; // we may encounter EOF, so return true is OK
		}
		if( recHeader.type != 0x02 ){
			reader->seek(recHeader.length, FileReader::SEEK_FROM_CUR);
			continue;
		}
		if(recHeader.length > 0x1B) {
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,
				CLOGFMT(IndexFileParser,"bad index record data, IFrame record should not bigger than  0x1B, file[%s]"),
				reader->getFileName().c_str() );
			return false;
		}
		if(reader->read(tempBuffer,recHeader.length) != recHeader.length) {
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,
				CLOGFMT(IndexFileParser,"insufficient data, file[%s]"),
				reader->getFileName().c_str() );
			return false;
		}
		
		// we only care about normal play file index record
		memset(&tempBuffer[17],0,5);
		uint32 timeOffset = (uint32)((*(uint64*)&tempBuffer[11])/27000);
		record.addRecord(*(uint32*)tempBuffer, timeOffset);

		size_t subRecords = 0;
		
		do {
			if( !getTag(reader, recHeader) ) {
				break;
			}
			reader->seek(recHeader.length, FileReader::SEEK_FROM_CUR);
			if( recHeader.type != 0x03){
				continue;				
			}
			else if (recHeader.type == 0x03)
			{
				if(recHeader.length > 0x09) {
					IDXPARSERLOG(ZQ::common::Log::L_ERROR,
						CLOGFMT(IndexFileParser,"bad index record data, IFrame record should not bigger than  0x09, file[%s]"),
						reader->getFileName().c_str() );
					return false;}
				reader->seek(-recHeader.length,FileReader::SEEK_FROM_CUR);
				if (reader->read(tmpBufTrick,recHeader.length) != recHeader.length){
					IDXPARSERLOG(ZQ::common::Log::L_ERROR,
						CLOGFMT(IndexFileParser,"insufficient data, file[%s]"),
						reader->getFileName().c_str() );
					return false;
				}
				uint8 SubFileIdx = (uint8)(*(uint8*)tmpBufTrick);
				uint32 PkgOffset = (uint32)(*(uint32*)&tmpBufTrick[1]);
				SPEED_IND speed = indexHeader.getSubFileSpeed(SubFileIdx);
				record.addRecTrick(PkgOffset,timeOffset,speed, SubFileIdx,indexHeader.getSubFileName(SubFileIdx), indexHeader.getAllFilePlayTime(SubFileIdx));
			}
			subRecords ++;
		}while(subRecords < subFileCount);
	}
	record.sortRecordTrcik();
    record.printIndexfile();
//	int idx = -1;unsigned long offset = -1;
//    uint64 time = 1000;
//	offset = record.findNextIFrame(time,idx, 7.5);
    float sp = 0.0;uint32 playtime = -1;
    std::string ext = record.getSubfileExt(sp,playtime);
    IDXPARSERLOG(ZQ::common::Log::L_INFO,CLOGFMT(IndexFileParser,"scale[%f],ext[%s],playtime[%d]"),sp,ext.c_str(),playtime);
	record.lastParsedOffset(reader->tell());
	return true;
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
	printf("File Header\n");
	if( strcmp( CSICOINDEXFILESIGNATURE, fileHeader.signature ) != 0 )
	{
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,
			CLOGFMT(IndexFileParser,"bad signature for file[%s]"),
			reader->getFileName().c_str() );
		return false;
	}else{
		printf("Signature: \t\t%s\n", fileHeader.signature);
	}
	IDXPARSERLOG(ZQ::common::Log::L_INFO, CLOGFMT(parseCsicoINDEX,"signature:[%s]"),fileHeader.signature);
	
	if ( fileHeader.majorVersion == 1  )
	{
		printf("Version: \t\tV%d.%d.%d\n",fileHeader.generation,fileHeader.majorVersion,fileHeader.minorVersion);
		printf("Creator: \t\t%s\n",fileHeader.creator);
		printf("Checksum Value: \t\t0x%02x ",fileHeader.checksum);
		if( !(fileHeader.minorVersion == 0 || fileHeader.minorVersion == 1) )
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,	CLOGFMT(IndexFileParser,"unexpect version[%d.%d] for file[%s]"),
				fileHeader.majorVersion, fileHeader.minorVersion, reader->getFileName().c_str() );
		}
		return parseCsicoINDEX11( reader , idxData );
	}
	else
	{
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,	CLOGFMT(IndexFileParser,"Unsupport version[%d.%d] for file[%s]"),
			fileHeader.majorVersion, fileHeader.minorVersion, reader->getFileName().c_str() );
		return false;
	}
	int i = 0;
}


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
			if( pSubType->content.length() > 0 )
			{
				if( pSubType->content.at(0) == '.' )
					return pSubType->content;
				else
					return "." + pSubType->content;
			}
			else
			{
				return pSubType->content;
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

bool CsicoIndexFileParser::parseSubFiles( FileReader* reader  , IndexData& idxData, uint32 len)
{
	CsicoIndexFileTagAndLength tal;
    uint64 posEnd = reader->tell() + len;
	int	subFileIndex = -1;
	IndexData::SubFileInformation subInfo;

		bool bIndexOk				= false;
		bool bSpeedNumeratorOk		= false;
		bool bSpeedDenominatorOk	= false;
		bool bNameOk				= false;
		bool bPlayTimeOK			= false;
		bool bStartingByteOK		= false;
		bool bEndinByteOK			= false;
		bool bTypeOK = false, bDirectionOK = false, bIFrameCount = false, bPFrameCount = false, bBFrameCount = false, bStartPCR = false,
			 bEndPCR = false, bStartContinuty = false, bEndContinuty = false, bChecksum = false;
		while( reader->tell() < posEnd && !( bIndexOk && bSpeedNumeratorOk && bSpeedDenominatorOk && bNameOk && bPlayTimeOK && bEndinByteOK && bStartingByteOK && bDirectionOK
			      && bIFrameCount && bPFrameCount && bBFrameCount && bStartPCR && bEndPCR && bStartContinuty && bEndContinuty && bChecksum && bTypeOK) )
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
					printf("Index: \t\t %d\n",subFileIndex);
					bIndexOk = true;
				}
				break;
			case CSICOINDEX_TAG_SUBFILEPLAYTIME:
				{
					subInfo.playtime	= readAndReinterpretAs<uint32>( reader , tal.length );
					printf("Play Time: \t\t %d\n",subInfo.playtime);
					bPlayTimeOK = true;
				}
				break;
			case CSICOINDEX_TAG_SUBFILETYPE:
				{
					uint32 type = readAndReinterpretAs<uint32>( reader , tal.length );
					switch (type)
					{
					case 0:
						{
							printf("Type: \t\t %d %s\n",type, "SPTS");
						}
						break;
					case 1:
						{
							printf("Type: \t\t %d %s\n",type, "Elementary Stream");
						}
						break;
					default:{}
					}
					
					bTypeOK = true;
				}
				break;
			case CSICOINDEX_TAG_SUBFILESPEEDDENOMINATOR:
				{
					subInfo.speed.denominator = (USHORT)readAndReinterpretAs<uint32>( reader , tal.length );
					printf("Speed Numerator: \t\t %d\n",subInfo.speed.denominator);
					 bSpeedDenominatorOk = true;
				}
				break;
			case CSICOINDEX_TAG_SUBFILESPEEDNUMERATOR:
				{
					subInfo.speed.numerator = (SHORT)readAndReinterpretAs<uint32>( reader , tal.length );
					printf("Speed Denominator: \t\t %d\n",subInfo.speed.numerator);
					bSpeedNumeratorOk = true;
				}
				break;
			case CSICOINDEX_TAG_SUBFILEDIRECTION:
				{
					uint8 direc = readAndReinterpretAs<uint8>( reader , tal.length );
					switch (direc)
					{
					case 1:
						{
							printf("Play Direction: \t\t %d  %s\n",direc, "Forward");
						}
						break;
					case 2:
						{
							printf("Play Direction: \t\t %d  %s\n",direc, "Reverse");
						}
						break;
					case 3:
						{
							printf("Play Direction: \t\t %d  %s\n",direc, "Bidirectional");
						}
						break;
					default:
						{}
					}
					printf("Play Direction: \t\t %d\n",direc);
					bDirectionOK = true;
				}
				break;
			case CSICOINDEX_TAG_SUBFILEIFRAMECOUNT:
				{
					uint32 countI = readAndReinterpretAs<uint8>( reader , tal.length );
					printf("I Picture Count: \t\t %d\n",countI);
					bIFrameCount = true;
				}
				break;
			case CSICOINDEX_TAG_SUBFILEPFRAMECOUNT:
				{
					uint32 countP = readAndReinterpretAs<uint8>( reader , tal.length );
					printf("P Picture Count: \t\t %d\n",countP);
					bPFrameCount = true;
				}
				break;
			case CSICOINDEX_TAG_SUBFILEBFRAMECOUNT:
				{
					uint32 countB = readAndReinterpretAs<uint8>( reader , tal.length );
					printf("B Picture Count: \t\t %d\n",countB);
					bBFrameCount = true;
				}
				break;
			case CSICOINDEX_TAG_SUBFILE_STARTPCR:
				{
					uint32  pcr = readAndReinterpretAs<uint32>( reader , tal.length );
					printf("Starting PCR: \t\t %x\n",pcr);
					bStartPCR = true;
				}
				break;
			case CSICOINDEX_TAG_SUBFILE_ENDPCR:
				{
					uint32 pcr = readAndReinterpretAs<uint32>( reader , tal.length );
					printf("Ending PCR: \t\t %x\n",pcr);
					bEndPCR = true;
				}
				break;
			case CSICOINDEX_TAG_SUBFILE_STARTCONTINUITY:
				{
					uint8 startcont = readAndReinterpretAs<uint8>( reader , tal.length );
					printf("Starting Continuity Ctr: \t\t %d\n",startcont);
					bStartContinuty = true;
				}
				break;
			case CSICOINDEX_TAG_SUBFILE_ENDCONTINUITY:
				{
					uint8 endcont = readAndReinterpretAs<uint8>( reader , tal.length );
					printf("Ending Continuity Ctr: \t\t %d\n",endcont);
					bEndContinuty = true;
				}
				break;
			case CSICOINDEX_TAG_SUBFILE_CHECKSUM:
				{
					uint32 checksum = readAndReinterpretAs<uint32>( reader , tal.length );
					printf("Checksum Value: \t\t %x\n",checksum);
					bChecksum = true;
				}
				break;
			case CSICOINDEX_TAG_SUBFILENAME:
				{
					//std::string name = readString(reader,tal.length);
					subInfo.fileExtension = getSubType( reader , readString( reader , tal.length));
					printf("Name: \t\t %s\n",subInfo.fileExtension.c_str());
					bNameOk = true;
				}
				break;
			case CSICOINDEX_TAG_SUBFILE_STARTBYTE:
				{
					subInfo.startingByte = read5byteInt(reader);
					printf("Starting Byte: \t\t %x\n",subInfo.startingByte);
					bStartingByteOK = true;
				}
				break;
			case CSICOINDEX_TAG_SUBFILE_ENDBYTE:
				{
					subInfo.endingByte = read5byteInt( reader);
					printf("Ending Byte: \t\t %x\n",subInfo.endingByte);
					bEndinByteOK = true;
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

            idxData.mainFileSize        = subInfo.endingByte;
		}		
		idxData.subFileInfos.push_back(subInfo);
	
	return true;
}

bool CsicoIndexFileParser::parseTransStreamInfo(FileReader* reader, IndexData& idxData, uint32 len)
{
	CsicoIndexFileTagAndLength tal;
    uint64 posEnd = reader->tell() + len;
	bool bProgramNumber = false, bProgMap = false, bPCRPid = false, bVideo = false, bAudio = false;
	while (reader->tell() < posEnd && !( bProgramNumber && bProgMap && bPCRPid && bVideo && bAudio) )
	{
		if (!getTag( reader, tal) )
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"insufficient data to parse of [%s]"),reader->getFileName().c_str());
			return false;
		}
		switch ( tal.tagId )
		{
		case CSICOINDEX_TAG_TRANSSTREAM_PROGRAM_NUMBER:
			{
				//idxData.programnumber = readAndReinterpretAs<uint32>(reader, tal.length);
				uint32 programnumber = readAndReinterpretAs<uint32>(reader, tal.length);
				printf("Program Number: \t\t %d\n",programnumber);
				bProgramNumber = true;
			}
			break;
		case CSICOINDEX_TAG_TRANSSTREAM_PROGRAMMAPTABLE:
			{
				uint16 mapid = readAndReinterpretAs<uint16>(reader, tal.length);
				printf("Program Map Table PID: \t\t 0x%02x\n",mapid);
				bProgMap = true;
			}
			break;
		case CSICOINDEX_TAG_TRANSSTREAM_PCR_PID:
			{
				uint16 PCRID = readAndReinterpretAs<uint16>(reader, tal.length);
				printf("Program Clock Reference PID: \t\t 0x%02x\n",PCRID);
				bPCRPid = true;
			}
			break;
		case CSICOINDEX_TAG_TRANSSTREAM_VIDEO_PID:
			{
				uint16 VideoPCR = readAndReinterpretAs<uint16>(reader, tal.length);
				printf("Video Elementary Stream PID: \t\t 0x%02x\n",VideoPCR);
				bVideo = true;
			}
			break;
		case CSICOINDEX_TAG_TRANSSTREAM_AUDIO_PID:
			{
				uint16 AudioPCR = readAndReinterpretAs<uint16>(reader, tal.length);
				printf("Audio Elementarty Stream PID: \t\t 0x%02x\n",AudioPCR);
				bAudio = true;
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

bool CsicoIndexFileParser::parseElemStreamInfo(FileReader* reader, IndexData& idxData, uint32 len)
{
	CsicoIndexFileTagAndLength tal;
	bool bStreamType = false, bStreamPID = false, bSeq = false, bExtSeq = false, flag = false;
	uint32 count = 0;
	CsicoIndexFileTagAndLength lastTal;
	int64 posEnd = reader->tell()+ len;	
	while ( reader->tell() < posEnd  && !( bStreamType && bStreamPID && bExtSeq && bSeq) )
	{
		if (!getTag( reader, tal) )
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"insufficient data to parse of [%s]"),reader->getFileName().c_str());
			return false;
		}
		
		switch ( tal.tagId )
		{
		case CSICOINDEX_TAG_ELEMSTREAMTYPE:
			{
				uint8 type = readAndReinterpretAs<uint8>(reader, tal.length);
				printf("Stream Type: \t\t 0x%02x\n", type);
				bStreamType = true;
			}
			break;
		case CSICOINDEX_TAG_ELEMSTREAPID:
			{
				uint16 pid = readAndReinterpretAs<uint16>(reader, tal.length);
				printf("Stream PID: \t\t 0x%02x\n",pid);
				bStreamPID = true;
			}
			break;
		case CSICOINDEX_TAG_ELEMSTREAMEXTSEQ:
			{
						printf("Extend Sequence Header Length: \t\t %d\n",tal.length);
						unsigned char buff[1024];
						memset(buff,0,1024);
						reader->read(buff,tal.length);
						printf("\t\t");int j = 0;
						for (int i = 0;i < tal.length; i++, j++)
						{
                            if (j % 16 == 0){printf("\n\t\t");}
							printf("%02x ", buff[i]);
						}
						printf("\n");
						bExtSeq = true;
			}
			break;
		case CSICOINDEX_TAG_ELEMSTREAMSEQ:
			{

				/*uint64 seq = read8byteInt(reader);*/
				printf("Sequence Header Length: \t\t %d\n",tal.length);
				unsigned char buff[1024];
				memset(buff,0,1024);
				reader->read(buff,tal.length);
				printf("\t\t"); int j = 0;
				for (int i = 0;i < tal.length; i++,j++)
				{
                    if ( j % 16 == 0){printf("\n\t\t");}
					printf("%02x ", buff[i]);
				}
				printf("\n");
				bSeq = true;
			}
			break;
		default:
			{			
				reader->seek( tal.length ,FileReader::SEEK_FROM_CUR  );				
			}
			break;
		}
		lastTal = tal;
	}
	return true;
}

bool CsicoIndexFileParser::parseRandomIndexInfo(FileReader* reader, IndexData& idxData, uint32 len)
{
	CsicoIndexFileTagAndLength tal;
    uint64 posEnd = reader->tell() + len;
	bool bIdxBitRate = false, bWorst = false;
	while (reader->tell() < posEnd && !(bIdxBitRate && bWorst))
	{
		if (!getTag(reader, tal))
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"insufficient data to parse of [%s]"),reader->getFileName().c_str());
			return false;
		}
		switch ( tal.tagId)
		{
		case CSICOINDEX_TAG_IDXRATE:
			{
				uint64 idxrate = readAndReinterpretAs<uint64>(reader, tal.length);
				printf("Index Bit Rate: \t\t%ld\n",idxrate);
				bIdxBitRate = true;
			}

			break;
		case CSICOINDEX_TAG_WORSTCASE:
			{
				uint64 uWorst =  readAndReinterpretAs<uint64>(reader, tal.length);
				printf("Worst Case: \t\t%ld\n",uWorst);
				bWorst = true;
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

bool CsicoIndexFileParser::parsePSIInfo(FileReader* reader, IndexData& idxData, uint32 len)
{
	CsicoIndexFileTagAndLength tal;
    uint64 posEnd = reader->tell() + len;
	bool bProgAssociation = false, bProgMap = false;
	while (reader->tell() < posEnd && !( bProgAssociation && bProgMap) )
	{
		if (!getTag( reader, tal) )
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"insufficient data to parse of [%s]"),reader->getFileName().c_str());
			return false;
		}
		switch ( tal.tagId )
		{
		case CSICOINDEX_TAG_PROGASSOCIATIONTB:
			{
				printf("Program Association Table (PAT)");
				unsigned char buff[1024];
				memset(buff,0,1024);
				reader->read(buff,tal.length);
				for (int i = 0;i < tal.length; i++)
				{
                    if (i % 16 == 0)
                    {
                        printf("\n\t\t");
                    }
					printf("%02x ", buff[i]);
				}
				printf("\n");
				bProgAssociation = true;
			}
			break;
		case CSICOINDEX_TAG_PROGMAPTB:
			{
				printf("Program Map Table (PMT)");
				unsigned char buff[1024];
				memset(buff,0,1024);
				reader->read(buff,tal.length);
				for (int i = 0;i < tal.length; i++)
				{
                    if ( i % 16 == 0)
                    {
                        printf("\n\t\t");
                    }
					printf("%02x ", buff[i]);
				}
				printf("\n");
				bProgMap = true;
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
//bool CsicoIndexFileParser::parsePSIAssociation(FileReader* reader, IndexData& idxData)
//{
//	CsicoIndexFileTagAndLength tal;
//	bool bProgNum = false, bProgMaptb = false;
//	//uint8 num = readAndReinterpretAs<uint8>(reader, tal.length);
//	//int i = 0;
//	while (!( bProgNum) )
//	{
//		if (!getTag( reader, tal) )
//		{
//			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"insufficient data to parse of [%s]"),reader->getFileName().c_str());
//			return false;
//		}
//		switch ( tal.tagId )
//		{
//		case CSICOINDEX_TAG_TRANSSTREAM_PROGRAM_NUMBER:
//			{
//				printf("Program Association Table (PAT)\n");
//				uint32 programnumber = readAndReinterpretAs<uint32>(reader, tal.length);
//				printf("Program Number: \t\t %d\n",programnumber);
//				bProgNum = true;
//			}
//			break;
//		default:
//			{
//				reader->seek( tal.length ,FileReader::SEEK_FROM_CUR  );
//			}
//			break;
//		}
//	}
//	return true;
//}

bool CsicoIndexFileParser::parseIdxHeaderInfo(FileReader* reader, IndexData& idxData)
{
	CsicoIndexFileTagAndLength tal;
	bool bSignatureOK = false;
	while (!(bSignatureOK))
	{
		if( !getTag( reader,tal ) )
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"insufficient data to parse of [%s]"),reader->getFileName().c_str());
			return false;
		}
	}
	return true;
}
bool CsicoIndexFileParser::parseAssetInfo(ZQ::IdxParser::FileReader *reader, ZQ::IdxParser::IndexData &idxData, uint32 len)
{
	CsicoIndexFileTagAndLength tal;
    uint64 posEnd = reader->tell() + len;
	bool bMuxRateOk = false;
	bool bVerticalSizeOk = false;
	bool bHorizontalSizeOk = false;
	bool bSubFileCountOk = false;
	bool bAssetIngestOK = false, bidxCreationtime = false, bprogramVersion = false, bassetid = false, bsrcfilename = false, bWrapperType = false,
		 bTransPkgSize = false, bSyncOffset = false, bStreamType = false,bVideoRate = false, bVideoRateTicks = false, bVideoRateTimeScale = false,
		 bMaxGOPSize = false, bFixedPTSInterval = false, bPTSCalculation = false, bInitDecoding = false, bstreamcount = false, belemstreamcount = false,
		 bPCKPkg = false, bscte35count = false, bOpen4Write = false, bVendorID = false;

	while( reader->tell() < posEnd && !(bMuxRateOk && bVerticalSizeOk && bHorizontalSizeOk && bSubFileCountOk && bAssetIngestOK && bidxCreationtime && bprogramVersion && bassetid
		     && bsrcfilename && bTransPkgSize && bSyncOffset && bStreamType && bVideoRate && bVideoRateTicks && bVideoRateTimeScale && bMaxGOPSize && bFixedPTSInterval
			 && bPTSCalculation && bInitDecoding && bstreamcount && belemstreamcount && bPCKPkg && bscte35count && bOpen4Write && bVendorID) )
	{
		if( !getTag( reader,tal ) )
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"insufficient data to parse of [%s]"),reader->getFileName().c_str());
			return false;
		}
		switch( tal.tagId )
		{
		case CSICOINDEX_TAG_CREATION_VENDOR_ID:
			{
				//idxData.vendorID = read6byteInt(reader);
				std::string str = getSubType( reader, readString( reader, tal.length));//readString( reader , tal.length );
				//printf("Creation Vendor Id:  ");
				bVendorID = true;
			}
			break;
		case CSICOINDEX_TAG_ASSET_INGEST_TIME:
			{
				idxData.assetIngestTime = readString( reader , tal.length );
				printf("Asset Ingest Time:  \t\t %s\n",idxData.assetIngestTime.c_str());
				bAssetIngestOK = true;
			}
			break;
		case CSICOINDEX_TAG_INDEX_CREATION_TIME:
			{
				idxData.idxcreationtime = readString( reader, tal.length );
				printf("Index Creation Time:  \t\t %s\n",idxData.idxcreationtime.c_str());
				bidxCreationtime = true;
			}
			break;
		case CSICOINDEX_TAG_PROGRAM_VERSION:
			{
				idxData.programversion = readString( reader, tal.length );
				printf("Program Version:  \t\t %s\n",idxData.programversion.c_str());
				bprogramVersion = true;
			}
			break;
		case CSICOINDEX_TAG_ASSER_IDENTIFIER:
			{
				idxData.assetId = readString( reader, tal.length );
				printf("Asset Identifier:  \t\t %s\n",idxData.assetId.c_str());
				bassetid = true;
			}
			break;
		case CSICOINDEX_TAG_SOURCE_FILE_NAME:
			{
				idxData.sourcefilename = readString( reader, tal.length );
				printf("Source File Name:  \t\t %s\n",idxData.sourcefilename.c_str());
                bsrcfilename = true;
			}
			break;
		/*case CSICOINDEX_TAG_FILE_WRAPPER_TYPE:
			{	
				idxData.filewrappertype = readAndReinterpretAs<uint8>(reader,tal.length);
				bWrapperType = true;
			}
			break;*/
		case CSICOINDEX_TAG_TRANSPORT_PACKET_SIZE:
			{
				idxData.transportpacketsize = readAndReinterpretAs<uint8>(reader, tal.length);
				printf("Transport Packet Size:  \t\t %d\n",idxData.transportpacketsize);
				bTransPkgSize = true;
			}
			break;
		case CSICOINDEX_TAG_SYNC_BYTE_OFFSET:
			{
				idxData.syncbyteoffset = readAndReinterpretAs<uint8>(reader, tal.length);
				printf("Sync Byte Offset:  \t\t %d\n",idxData.syncbyteoffset);
				bSyncOffset = true;
			}
			break;
		case CSICOINDEX_TAG_MUXBITRATE:
			{
				idxData.muxBitRate = readAndReinterpretAs<uint32>(reader,tal.length);
				printf("Transport Bit Rate:  \t\t %d\n",idxData.muxBitRate);
				bMuxRateOk = true;
			}
			break;
		case CSICOINDEX_TAG_TRANSPORT_STREAM_TYPE:
			{
				idxData.transstreamtype = readAndReinterpretAs<uint8>(reader, tal.length);
				switch (idxData.transstreamtype)
				{
				case 1:
					{
						printf("Transport Stream Type:  \t %d  %s\n",idxData.transstreamtype,"MPEG-2");
					}
					break;
				}
				bStreamType = true;
			}
			break;
		case CSICOINDEX_TAG_VIDEO_BIT_RATE:
			{
				idxData.videobyterate = readAndReinterpretAs<uint32>(reader, tal.length);
				printf("Video Bit Rate:  \t\t %d\n",idxData.videobyterate);
				bVideoRate = true;
			}
			break;
		case CSICOINDEX_TAG_HORIZONTALSIZE:
			{
				idxData.horizonResolution = readAndReinterpretAs<uint16>(reader,tal.length);
				printf("Video Horizontal Size:  \t\t %d\n",idxData.horizonResolution);
				bHorizontalSizeOk = true;
			}
			break;
		case CSICOINDEX_TAG_VERTICALSIZE:
			{
				idxData.verticalResolution = readAndReinterpretAs<uint16>(reader , tal.length);
				printf("Video Vertical Size:  \t\t %d\n",idxData.verticalResolution);
				bVerticalSizeOk = true;
			}
			break;
		case CSICOINDEX_TAG_FRAME_RATE_TICKS:
			{
				idxData.videorateticks = readAndReinterpretAs<uint32>(reader, tal.length);
				printf("Video Frame Rate Ticks:  \t\t %d\n",idxData.videorateticks);
				bVideoRateTicks = true;
			}
			break;
		case CSICOINDEX_TAG_FRAME_RATE_TIME_SCALE:
			{
				idxData.videoratetimescale = readAndReinterpretAs<uint32>(reader, tal.length);
				printf("Video Frame Rate Time Scale:  \t\t %d\n",idxData.videoratetimescale);
				bVideoRateTimeScale = true;
			}
			break;
		case CSICOINDEX_TAG_MAX_GOP_SIZE:
			{
				idxData.maxGOPsize = readAndReinterpretAs<uint32>(reader, tal.length);
				printf("Maximum GOP Size in Frames:  \t\t %d\n",idxData.maxGOPsize);
				bMaxGOPSize = true;
			}
			break;
		case CSICOINDEX_TAG_FIXED_PTS_DTS_INTERVAL:
			{
				idxData.fixedPTSInterval = readAndReinterpretAs<uint32>(reader, tal.length);
				printf("Fixed PTS-DTS Interval:  \t\t %d\n",idxData.fixedPTSInterval);
				bFixedPTSInterval = true;
			}
			break;
		case CSICOINDEX_TAG_PTS_DTS_CALCULATION:
			{
				idxData.PTSCalculation = readAndReinterpretAs<uint32>(reader, tal.length);
				printf("PTS-DTS Calculation Method:  \t\t %d\n",idxData.PTSCalculation);
				bPTSCalculation = true;
			}
			break;
		case CSICOINDEX_TAG_INITIAL_DECODING_DELAY:
			{			
				idxData.initdecodingdelay = readAndReinterpretAs<uint32>(reader, tal.length);
				printf("Initial Decoding Delay:  \t\t %d\n",idxData.initdecodingdelay);
				bInitDecoding = true;
			}
			break;
		case CSICOINDEX_TAG_TRANSPORT_STREAM_COUNT:
			{
				idxData.transstreamcount = readAndReinterpretAs<uint32>(reader, tal.length);
				printf("Transport Stream Count:  \t\t %d\n",idxData.transstreamcount);
				bstreamcount = true;
			}
			break;
		case CSICOINDEX_TAG_ELEMENTARY_STREAM_COUNT:
			{
				idxData.elemstreamcount = readAndReinterpretAs<uint8>(reader, tal.length);
				printf("Elementary Stream Count:  \t\t %d\n",idxData.elemstreamcount);
				belemstreamcount = true;
			}
			break;
		case CSICOINDEX_TAG_SUBFILECOUNT:
			{
				idxData.subFileCount = readAndReinterpretAs<uint8>(reader,tal.length);
				printf("Sub File Count:  \t\t %d\n",idxData.subFileCount);
				bSubFileCountOk = true;
				_subFileCount	= idxData.subFileCount;
			}
			break;
		case CSICOINDEX_TAG_PCR_PACKET_INTERVAL:
			{
				idxData.PCRPkgInterval = readAndReinterpretAs<uint16>(reader,tal.length);
				printf("PCR Packet Interval:  \t\t %d\n",idxData.PCRPkgInterval);
				bPCKPkg = true;
			}
			break;
		case CSICOINDEX_TAG_SCTE35_RECORD_COUNT:
			{
				idxData.scte35count = readAndReinterpretAs<uint32>(reader, tal.length);
				printf("SCTE-35 Record Count:  \t\t %d\n",idxData.scte35count);
				bscte35count = true;
			}
			break;
		case CSICOINDEX_TAG_OPEN4WRITE:
			{
				idxData.bOpen4Write = readAndReinterpretAs<uint8>(reader, tal.length);
				printf("Open For Write:  \t\t %d\n",idxData.bOpen4Write);
				bOpen4Write = true;
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

bool CsicoIndexFileParser::parseZeroMotion(FileReader* reader, IndexData& idxData,uint32 len )
{
	CsicoIndexFileTagAndLength tal;
    uint64 posEnd = reader->tell() + len;
	bool bPFrame = false, bBFrame = false;
	while (reader->tell() < posEnd && !( bPFrame && bBFrame) )
	{
		if (!getTag( reader, tal) )
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"insufficient data to parse of [%s]"),reader->getFileName().c_str());
			return false;
		}
		switch ( tal.tagId )
		{
		case CSICOINDEX_TAG_ZEROMOTIONPFRAME:
			{
				unsigned char buff[512];
				memset(buff,0,512);
				reader->read(buff, tal.length);
				unsigned char flag = buff[0];
				printf("Zero Motion P-Frame length: %d",tal.length);
				for (int i = 0;i < tal.length;i++)
				{
                    if (i % 16 == 0)
                    {
                        printf("\n\t\t");
                    }
					printf("%02x ",buff[i]);
				}
				printf("\n");
				bPFrame = true;
			}
			break;
		case CSICOINDEX_TAG_ZEROMOTIONBFRAME:
			{
				unsigned char buff[1024];
				memset(buff, 0, 1024);
				reader->read(buff,tal.length);
				unsigned char flag = buff[0];
				printf("Zero Motion B-Frame Length: %d",tal.length);
				for (int i = 0;i < tal.length; i++)
				{
                    if ( i % 16 == 0)
                    {
                        printf("\n\t\t");
                    }
					printf("%02x ", buff[i]);
				}
				printf("\n");
				bBFrame = true;
			}
			break;
		}
	}

	return true;
}
bool CsicoIndexFileParser::parseSection(  FileReader* reader , IndexData& idxData, int len )
{
	CsicoIndexFileTagAndLength tal;

	int subFileCount = 0, count = 0;
	bool bAssetInfoOk = false;
	bool bSubFileInfoOk = false;
	bool bTransStreamOK = false;
	bool bZeroMotionOK = false, bElemInfoOK = false, bPSIOK = false, bRandomOK = false, bCheckvalue = false;
	while( !bAssetInfoOk  || !bSubFileInfoOk || !bZeroMotionOK || !bTransStreamOK || !bRandomOK || !bElemInfoOK || !bPSIOK || !bCheckvalue)
	{
		if( !getTag( reader,tal ) )
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"insufficient data to parse of [%s]"),reader->getFileName().c_str());
			return false;
		}
		
		switch ( tal.tagId )
		{
		case CSICOINDEX_TAG_INDEXHEADERCHECK:
			{
				uint32 checksum	= readAndReinterpretAs<uint32>( reader , tal.length );
				printf("Index Header Checksum Value:\t\t%x\n",checksum);
				bCheckvalue = true;
			}
			break;
		case CSICOINDEX_TAG_ASSETINFOSECTION:
			{
				printf("Index Header Information Block\n\n");
				if( !parseAssetInfo(reader , idxData, tal.length ))
					return false;
				else
					bAssetInfoOk = true;

			}
			break;

		case CSICOINDEX_TAG_SUBFILESECTION:
			{
				printf("Sub File Information Block\n\n");
				if( !parseSubFiles( reader , idxData , tal.length))
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
		case CSICOINDEX_TAG_ZEROMOTION:
			{
				printf("Zero Motion Frame Information Block\n\n");
				if (!parseZeroMotion( reader, idxData, tal.length))
				{
					return false;
				}else{
					bZeroMotionOK = true;
				}
			}
			break;
		case CSICOINDEX_TAG_TRANSPORT_STREAM_INFO:
			{
				printf("Transport Stream Information Block\n\n");
				if (! parseTransStreamInfo(reader,idxData, tal.length))
				{
					return false;
				}else{
					bTransStreamOK = true;
				}
			}
			break;
		case CSICOINDEX_TAG_ELEMSTREAMINFO:
			{
				printf("Elementary Stream Information Block\n\n");
				if (! parseElemStreamInfo( reader, idxData, tal.length))
				{
					return false;
				}else{
					bElemInfoOK = true;
				}
			}
			break;
		case CSICOINDEX_TAG_RANDOM_INDEX:
			{
				printf("Random Index Access Information Block\n\n");
				if (!parseRandomIndexInfo( reader, idxData, tal.length))
				{
					return false;
				}else{
					bRandomOK = true;
				}
			}
			break;
		case CSICOINDEX_TAG_PSIINFO:
			{
				printf("Program Specific Information (PSI) Block\n\n");
				if (! parsePSIInfo(reader, idxData, tal.length))
				{
					return false;
				}else{
					bPSIOK = true;
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
				if(! parseSection( reader, idxData, tal.length) )
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

	return true;
}

bool IndexFileParser::ParseIndexRecodFromMemory( const std::string& mainFilePathName,
												IndexRecord& record, 
												const char* buffer, 
												size_t size) {
	MemFileReader reader((const unsigned char*)buffer,size,mainFilePathName);
	return parserCsicoIndexRecord(&reader,mainFilePathName,record);
}

bool IndexFileParser::ParseIndexRecordFromCommonFS( const std::string& mainFilePathName, 
												 IndexRecord& record, 
												 const std::string& indexFileName ) {
	 FileReaderNative reader(_idxParserEnv);
	 if(!reader.open(indexFileName)) {
		 IDXPARSERLOG(ZQ::common::Log::L_ERROR, CLOGFMT(IndexFileParser,"failed to open[%s]"),indexFileName.c_str());
		 return false;
	 }

	 int64 filesize = reader.seek(0,FileReader::SEEK_FROM_END);
	 reader.seek(0,FileReader::SEEK_FROM_BEGIN);
	 char* p = (char*)malloc(filesize*sizeof(char));
	 assert(p);
	 int32 dataSize = reader.read(p,(int32)filesize);
	 if(dataSize != (int32)filesize) {
		 free(p);
		 IDXPARSERLOG(ZQ::common::Log::L_ERROR, CLOGFMT(IndexFileParser,"failed to read content from [%s]" ),
				 indexFileName.c_str());
		 return false;
	 }
	 IDXPARSERLOG(ZQ::common::Log::L_INFO, CLOGFMT(ParseIndexRecordFromCommonFS, "filesize:[%d],datasize[%d]"),filesize,dataSize);
	 MemFileReader memreader((const unsigned char*)p,dataSize, mainFilePathName);
	 bool bOK =  parserCsicoIndexRecord(&memreader,mainFilePathName,record);
	 free(p);
	 return bOK;
}

bool IndexFileParser::parserCsicoIndexRecord(ZQ::IdxParser::FileReader *reader, 
										const std::string &mainFileName,
										ZQ::IdxParser::IndexRecord &record) {
	CsicoIndexFileParser parser(_idxParserEnv, mainFileName);
	return parser.parseIndexRecord(reader, record);
	//return parser.parseIndexTrickRecord(reader,record);
}


//////////////////////////////////////////////////////////////////////////
IndexRecord::IndexRecord()
:mLastOffset(0),
mPwe(false),
mSubFileCount(0){
}

IndexRecord::~IndexRecord(){

}

uint64 IndexRecord::findNextIFrame( uint64& timeOffset , int& rateIdx, float speed) const {
	if (speed == 0.0){
		if(mRecords.size() == 0)
			return 0;
		if(mRecords.size() == 1)
			return packet2offset(mRecords[0].packetOffset);
		const RecordInfo* low = firstItem();
		const RecordInfo* high = lastItem();
		assert( low != NULL && high != NULL );
		uint32 packet = (uint32)(timeOffset/188);//TS packet alignment

		while( (low+1) < high ) {
			if( packet < low->packetOffset )
				return packet2offset(low->packetOffset);
			else if(packet > high->packetOffset)
				return packet2offset(high->packetOffset);
			const RecordInfo* middle = low+(high-low)/2;
			if( packet < middle->packetOffset ) {
				high = middle;
			} else if( packet > middle->packetOffset ){
				low = middle;
			} else {
				return packet2offset(middle->packetOffset);
			}
		}
		return packet2offset(high->packetOffset);
	}
	else
	{
		size_t size = mRecTrickInfo.size();
		int beg,next,i = 0;
		bool flag = false;
		//float arr[]= {-15.0,-7.5,-3.5,3.5,7.5};
		if (speed >= 0.0)
		{
			if (speed <=1.00)//normal rate
			{
				int idx = -1;
				return findNextIFrame(timeOffset, idx);
			}

			for (i = 0;i<size;i++)
			{
				if (mRecTrickInfo[i].scale >=1.0 )
					break;
			}
			beg = i;//找起始比较点
			next = i+1;
			while (true)
			{
				if ( speed <= mRecTrickInfo[beg].scale || next >= size){
					rateIdx = beg;
					break;
				}
				if( speed >= mRecTrickInfo[beg].scale && speed <= mRecTrickInfo[next].scale)
				{
					flag = true;
					break;
				}
				beg++;
				if (beg >= size)
				{
					rateIdx = beg -1;
					break;
				}
				next++;
			}
			if (flag){
				if (fabs(speed-mRecTrickInfo[beg].scale) <= fabs(speed-mRecTrickInfo[next].scale))
				{
					rateIdx = beg;
				}else
				{
					rateIdx = next;
				}
			}

		}
		////float arr[]= {-15.0,-7.5,-3.5,3.5,7.5};
		else if (speed < 0.0)
		{
			beg = 0;
			if ( speed < mRecTrickInfo[beg].scale )
			{
				rateIdx = beg;
			}else{
				next = beg + 1;
				while (true)
				{
					if (mRecTrickInfo[next].scale > 0.0 ){  //下一个数据为正
						rateIdx = beg;
						break;
					}
					if (speed >= mRecTrickInfo[beg].scale && speed < mRecTrickInfo[next].scale){
						flag = true;
						break;
					}
					beg ++;
					next ++;
				}
				if (flag){
					if ( fabs(speed - mRecTrickInfo[beg].scale) < fabs(speed - mRecTrickInfo[next].scale) ){
						rateIdx = beg;
					}else {
						rateIdx = next;
					}
				}
			}
		}
		if (rateIdx == -1) return -1;
		if ( mRecTrickInfo[rateIdx].info.size() == 0 )
		{
			return 0;
		}
		if ( mRecTrickInfo[rateIdx].info.size() == 1 )
			return mRecTrickInfo[rateIdx].info[0].packetOffset;
		const RecordInfo* low = firstTrickItem(mRecTrickInfo[rateIdx]);
		const RecordInfo* high = lastTrickItem(mRecTrickInfo[rateIdx]);
		assert( low != NULL && high != NULL);

		while( (low+1) < high ) {
			if( timeOffset < low->timeOffset )
				return low->packetOffset;
			else if(timeOffset > high->timeOffset)
				return high->packetOffset;
			const RecordInfo* middle = low+(high-low)/2;
			if( timeOffset < middle->timeOffset ) {
				high = middle;
			} else if( timeOffset > middle->timeOffset ){
				low = middle;
			} else {
				return middle->packetOffset;
			}
		}
		return high->packetOffset;
	}
}

void IndexRecord::addRecord( uint32 packetOffset, uint32 timeOffset) {
	RecordInfo r;
	r.packetOffset = packetOffset;
	r.timeOffset = timeOffset;
	mRecords.push_back(r);
}

void IndexRecord::addRecTrick(uint32 pkgOffset, uint32 timeOffset, SPEED_IND speed, uint8 subfileidx, const std::string ext,uint32 playtime)
{

	RecordInfo recinfo;
	recinfo.packetOffset = pkgOffset;
	recinfo.timeOffset = timeOffset;
	mRecTrickInfo[subfileidx-1].info.push_back(recinfo);
	mRecTrickInfo[subfileidx-1].speed = speed;
	mRecTrickInfo[subfileidx-1].scale = (float)speed.numerator / speed.denominator;
    mRecTrickInfo[subfileidx-1].fileExt = ext;
    mRecTrickInfo[subfileidx-1].playtime = playtime;
}

void IndexRecord::sortRecordTrcik()
{
	std::sort(mRecTrickInfo.begin(), mRecTrickInfo.end(),RecordTrickInfo());
}

void IndexRecord::printIndexfile()
{
    std::vector<RecordInfo>::iterator it1 = mRecords.begin(); 
    int i = 0;
    printf("MainFile\n");
    for (; it1 != mRecords.end(); ++it1, i++ )
    {
        if ( i % 4 == 0)
        {
          //  printf("\n\t\t");
            printf("\n");
        }
        printf("time: %ld, Pkg: %ld\t", it1->timeOffset, it1->packetOffset);
    }
    printf("\n\n");
    printf("SubIndexFile\n");
    std::vector<RecordTrickInfo>::iterator it = mRecTrickInfo.begin();
    for (;it != mRecTrickInfo.end();it++)
    {
        it1 = (it->info).begin(); i = 0;
        printf("scale: %f\n\n",it->scale);
        for (;it1 != it->info.end();++it1,i++)
        {
            if ( i % 4 ==0)
            {
                printf("\n");
            }
        printf("time: %ld, Pkg: %ld\t",it1->timeOffset, it1->packetOffset);
        }
        printf("\n\n");
    }
}

int IndexRecord::getSubFileIdx(float& speed)
{
	int rateIdx= 0;
	size_t size = mRecTrickInfo.size();
	int beg,next,i = 0;
	bool flag = false;
	if (speed >= 0.0)
	{
		if (speed <=1.00)//normal rate
		{
			int idx = -1;
            speed = 1.0;
			return 0;
		}

		for (i = 0;i<size;i++)
		{
			if (mRecTrickInfo[i].scale >=1.0 )
				break;
		}
		beg = i;//找起始比较点
		next = i+1;
		while (true)
		{
			if ( speed <= mRecTrickInfo[beg].scale || next >= size){
				rateIdx = beg;
				break;
			}
			if( speed >= mRecTrickInfo[beg].scale && speed <= mRecTrickInfo[next].scale)
			{
				flag = true;
				break;
			}
			beg++;
			if (beg >= size)
			{
				rateIdx = beg -1;
				break;
			}
			next++;
		}
		if (flag){
			if (fabs(speed-mRecTrickInfo[beg].scale) <= fabs(speed-mRecTrickInfo[next].scale))
			{
				rateIdx = beg;
			}else
			{
				rateIdx = next;
			}
		}

	}
	////float arr[]= {-15.0,-7.5,-3.5,3.5,7.5};
	else if (speed < 0.0)
	{
		beg = 0;
		if ( speed < mRecTrickInfo[beg].scale )
		{
			rateIdx = beg;
		}else{
			next = beg + 1;
			while (true)
			{
				if (mRecTrickInfo[next].scale > 0.0 ){  //下一个数据为正
					rateIdx = beg;
					break;
				}
				if (speed >= mRecTrickInfo[beg].scale && speed < mRecTrickInfo[next].scale){
					flag = true;
					break;
				}
				beg ++;
				next ++;
			}
			if (flag){
				if ( fabs(speed - mRecTrickInfo[beg].scale) < fabs(speed - mRecTrickInfo[next].scale) ){
					rateIdx = beg;
				}else {
					rateIdx = next;
				}
			}
		}
	}
    speed = mRecTrickInfo[rateIdx].scale;
	return rateIdx;
}
std::string IndexRecord::getSubfileExt(float& speed, uint32& playtime)
{
    if ( IsNormalRate(speed) )
    {
        playtime = mainPlayTime;
        speed = 1.0;
        return mainExtension;
    }
    int rateIdx = getSubFileIdx(speed);
    playtime = mRecTrickInfo[rateIdx].playtime;
    speed = mRecTrickInfo[rateIdx].scale;
    return mRecTrickInfo[rateIdx].fileExt;
  
}

bool IndexRecord::IsNormalRate(float& speed)
{
    if ( speed >= 0.0 && speed <= 1.0)
        return true;
    return false;
}
//std::string IndexRecord::getSubfileExt(float& speed)
//{
//        int rateIdx = getSubFileIdx(speed);
//            if (rateIdx == 0 ) return mainExtension; 
//                return mRecTrickInfo[rateIdx].fileExt;
//}

}}//namespace ZQ::IdxParser

















