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
#include <math.h>

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
		getSubFileInfo( i, info);
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

const char* CommonFileReader::getLine( char* buf, size_t size ) 
{
	register char* p = buf;
	assert( buf != NULL && size != 0 );

	while ( --size )
	{
		if( pHeader >= pTail )
		{
			//no buffered data
			int32	ret = read( szLocalBuffer, sizeof(szLocalBuffer)  );
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


MemFileReader::MemFileReader( const unsigned char* pData, size_t size, const std::string& fileName, int64 blockEnd )
:mpBuffer(pData),
mSize(size),
mCurPos(blockEnd - size),
mFileName(fileName),
mBlockBegin(blockEnd - size),
mBlockEnd(blockEnd)
{
}

MemFileReader::~MemFileReader( )
{
}

int32 MemFileReader::read( void* buf, size_t count, size_t size )
{
	size_t szLeft = mBlockEnd - mCurPos ;
	size_t szCopy = (size*count) ;
	szCopy = MIN(szCopy, szLeft);
	memcpy( buf, mpBuffer + mCurPos, szCopy );
	mCurPos += szCopy;

	return  static_cast<int32>(szCopy);
}

int32 MemFileReader::write( const void*, size_t, size_t  ) 
{
	return -1;
}

int64 MemFileReader::seek( int64 offset, SeekPosition begin ) 
{
	int64 realOff = 0;
	switch ( begin )
	{
	case SEEK_FROM_BEGIN:
		{
			realOff = offset;
			realOff = realOff > (int64)mBlockEnd ? mBlockEnd : realOff;
			realOff = realOff < mBlockBegin ? mBlockBegin : realOff;
		}
		break;
	case SEEK_FROM_END:
		{
			//int64 tmp = mSize;
			int64 tmp = mBlockEnd;
			realOff = tmp + offset;
			realOff = realOff > (int64)mBlockEnd ? mBlockEnd : realOff;			
			realOff = realOff < mBlockBegin ? mBlockBegin: realOff;
		}
		break;
	case SEEK_FROM_CUR:
		{
			realOff = offset + mCurPos;
			realOff = realOff < mBlockBegin? mBlockBegin: realOff;
			realOff = realOff > (int64)mBlockEnd ? mBlockEnd: realOff;
		}
		break;
	default:
		{
			return -1;
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

int64 MemFileReader::getFileSize( const std::string&, bool   ) 
{
	return mBlockEnd;
}

const char* MemFileReader::getLastErrorDesc( char* buf, size_t  )
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
		VSTATUS ret = VsOpenEx( &_fileHandle, (char*)fileName.c_str(), 
			GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
			OPEN_EXISTING,
			FILE_FLAG_CACHED,
			0,
			&_FileId);
		if( !(IS_VSTRM_SUCCESS(ret)) )
		{	
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(FileReaderVstrmI,"Can't open  file[%s] because[%s] with VsOpenEx"),
				fileName.c_str(), getLastErrorDesc(_szBuf,sizeof(_szBuf)-1)  );
			return false;		
		}	
	}	
	else
	{
		_fileHandle	= VstrmCreateFile( _idxParserEnv.GetVstrmHandle(), fileName.c_str(), 
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			0,
			OPEN_EXISTING,
			0,
			0);
		if( _fileHandle == INVALID_HANDLE_VALUE )
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(FileReaderVstrmI,"Can't open file[%s], because [%s] with VstrmCreateFile"),
				fileName.c_str(), 
				getLastErrorDesc( _szBuf, sizeof(_szBuf)-1 )	);
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
		VsClose( _fileHandle, _FileId );
	}
	else
	{
		if( _fileHandle != INVALID_HANDLE_VALUE )
		{
			VstrmCloseHandle( _idxParserEnv.GetVstrmHandle(), _fileHandle );
			_fileHandle = INVALID_HANDLE_VALUE;
			_fileName	= "";
		}
	}
	return true;
}
int32 FileReaderVstrmI::read( void* buf, size_t count, size_t size /*= 1*/ )
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

		if( FALSE == VstrmReadFile( _idxParserEnv.GetVstrmHandle(), _fileHandle, buf, (DWORD)(count*size), &readByte, NULL  ) )
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(FileReaderVstrmI,"can't read the file[%s] because [%s]"),
				_fileName.c_str(), 
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
int32 FileReaderVstrmI::write( const void* buf, size_t count, size_t size /* = 1 */ )
{
	DWORD writtenByte = 0;
	if(_idxParserEnv.useVsOpenAPI() )
	{
		if( FALSE == VsWrite( _fileHandle, static_cast<int>(count*size), (char*)buf, &writtenByte, NULL ) )
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(FileReaderVstrmI,"write to file[%s] failed"), _fileName.c_str() );
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
		if( FALSE == VstrmWriteFile(_idxParserEnv.GetVstrmHandle(), _fileHandle, buf, (DWORD)(count* size), &writtenByte, 0 ))
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(FileReaderVstrmI,"can't write the file[%s] because [%s]"),
				_fileName.c_str(), 
				getLastErrorDesc( _szBuf,sizeof(_szBuf)-1 ) );
			return -1;
		}
	}

	return static_cast<int32>(writtenByte);
}

int64 FileReaderVstrmI::seek( int64 offset, SeekPosition begin ) 
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
		if ( INVALID_SET_FILE_POINTER == ( apiRet = VstrmSetFilePointer(_idxParserEnv.GetVstrmHandle(), _fileHandle, moveLow, &moveHigh, beginPos ) ))
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(FileReaderVstrmI,"can't seek with pos[%d] offset[%lld] because[%s]"),
				begin, offset, 
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

const char* FileReaderVstrmI::getLastErrorDesc( char* buf, size_t bufSize ) 
{
	assert( _idxParserEnv.GetVstrmHandle() != NULL );
	VstrmClassGetErrorText( _idxParserEnv.GetVstrmHandle(), VstrmGetLastError(), buf, (ULONG)bufSize );
	return buf;
}
const std::string& FileReaderVstrmI::getFileName( ) const
{
	return _fileName;
}

int64 FileReaderVstrmI::getFileSize( const std::string& file /*= "" */, bool bLog  )
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
	VHANDLE fileHandle =VstrmFindFirstFileEx( _idxParserEnv.GetVstrmHandle(), filePathName.c_str(), &findData );
	if(fileHandle != INVALID_HANDLE_VALUE)
	{
		VstrmFindClose( _idxParserEnv.GetVstrmHandle(), fileHandle );
		LARGE_INTEGER fileSize = {findData.w.nFileSizeLow, findData.w.nFileSizeHigh};
		int64 retSize = static_cast<int64>(fileSize.QuadPart);;
		IDXPARSERLOG(ZQ::common::Log::L_INFO,CLOGFMT(FileReaderVstrmI,"get file[%s]'s size [%lld]"),filePathName.c_str(), retSize );
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
			fileName.c_str(), 
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
int32 FileReaderNative::read( void* buf, size_t count, size_t size/* = 1 */) 
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

int32 FileReaderNative::write( const void* buf, size_t count, size_t size/* = 1 */) 
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

int64 FileReaderNative::seek( int64 offset, SeekPosition begin )
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
const char* FileReaderNative::getLastErrorDesc( char* buf, size_t bufSize ) 
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
bool IndexFileParser::parseIndexUsingVstrmAPILoadBrief( FileReader* reader, const std::string& mainFileName, IndexData& idxData, bool bLocalContent)
{
	VstrmLoadAssetInfo* LoadMethod = _idxParserEnv.getLoadInfoMethod();
	assert( LoadMethod != NULL );	

	//
	ULONG	bitRate = 0;
	ULONG	duration = 0;
	VOD_ASSET_INDEX_TYPE type = VOD_ASSET_INDEX_IS_VVX;
	VOD_ASSET_LOCATION location = bLocalContent ? VOD_ASSET_LOCATION_LOCAL : VOD_ASSET_LOCATION_REMOTE;


	VSTATUS ret = LoadMethod->LoadBriefAssetInfo( _idxParserEnv.GetVstrmHandle(), 
																mainFileName.c_str(), 
																&bitRate, 
																&duration,
																&type,
																location );
	if( ret != VSTRM_SUCCESS )
	{
		IDXPARSERLOG(ZQ::common::Log::L_WARNING,CLOGFMT(VstrmClassLoadBriefAssetInfo,"failed to parse[%s]' index, use LoadFullAssetInfo"),mainFileName.c_str() );
		return parseIndexUsingVstrmAPILoadAll( reader, mainFileName, idxData );
		
		/*
		char szErrorBuf[1024]={0};
		VstrmClassGetErrorText( _idxParserEnv.GetVstrmHandle(), ret, szErrorBuf, sizeof(szErrorBuf) );
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(PVstrmClassLoadBriefAssetInfo,"failed to parse [%s]'s index data because[%s]"),
			mainFileName.c_str(), 
			szErrorBuf );
		_lastError = szErrorBuf;
#if defined _DEBUG || defined DEBUG
		printf("failed to parse index data for [%s] because [%s]\n",
			mainFileName.c_str(), 
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
			mainFileName.c_str(), 
			duration,
			bitRate );
		return parseIndexUsingVstrmAPILoadAll( reader, mainFileName, idxData );		
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

bool IndexFileParser::parseIndexUsingVstrmAPILoadAll(  FileReader*, const std::string& mainFileName, IndexData& idxData )
{
	VstrmLoadAssetInfo* LoadMethod = _idxParserEnv.getLoadInfoMethod();
	assert( LoadMethod != NULL );	

	//
	VOD_ASSET_INFO assetInfo;
	memset(&assetInfo,0,sizeof(assetInfo));
	ULONG			infoSize =sizeof(assetInfo);
	
	VSTATUS ret = LoadMethod->LoadFullAssetInfo( _idxParserEnv.GetVstrmHandle(), 
												mainFileName.c_str(), 
												&assetInfo, 
												&infoSize );
	if( ret != VSTRM_SUCCESS )
	{
		if( ret == VSTRM_OBJECT_NAME_NOT_FOUND )
		{//set object not found error code
			setLastErrorCode( ERROR_CODE_OBJECT_NOT_FOUND );
		}

		char szErrorBuf[1024]={0};
		VstrmClassGetErrorText( _idxParserEnv.GetVstrmHandle(), ret, szErrorBuf, sizeof(szErrorBuf) );
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(parseIndexUsingVstrmAPILoadAll,
			"failed to parse [%s]'s index data because[%s]"),
			mainFileName.c_str(), 
			szErrorBuf );
		_lastError = szErrorBuf;
#if defined _DEBUG || defined DEBUG
		printf("failed to parse index data for [%s] because [%s]\n",
			mainFileName.c_str(), 
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
			mainFileName.c_str(), 
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

bool IndexFileParser::parseExtesionFileFromVstrm( const std::string& filePath, IndexData& idxData )
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
		if( parseExtension( &reader, idxData ) )
		{
			idxData.hasExtensionData = true;
			idxData.importantFileSet.push_back( extFileName );		
		}
	}
#endif//#ifndef EXCLUDE_VSTRM_API
	return false;
}

bool IndexFileParser::parseExtensionFileFromCommonFS( const std::string& filePath, IndexData& idxData  )
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
		if( parseExtension( &reader, idxData ) )
		{
			idxData.hasExtensionData = true;
			idxData.importantFileSet.push_back( extFileName );		
		}
	}
	return false;
}

typedef		bool (IndexFileParser::*PARSEFUNC)( FileReader* reader, IndexData& idxData  );

typedef		struct  _PARSEROUTE	
{
	const char*					_indexExtension;
	PARSEFUNC				_parser;
	IndexData::IndexType	_type;
}PARSEROUTE;



bool IndexFileParser::parseIndexFile( FileReader* reader, 
									 const std::string&	mainFileName,
									 IndexData& idxData, 
									 bool bIndexFile, 
									 const std::string& indexFileName )
{
	idxData.mainFileName = mainFileName;
	_mainFileName = mainFileName;

	PARSEROUTE funcRoute[] = 
	{
		{ ".index", 	&IndexFileParser::parseCsicoIndex, 		IndexData::INDEX_TYPE_VVC },
//		{ ".INDEX", 	&IndexFileParser::parseCsicoIndex, 		IndexData::INDEX_TYPE_VVC },
		{ ".VVX", 		&IndexFileParser::parseVVX, 			IndexData::INDEX_TYPE_VVX },
//		{ ".vvx", 		&IndexFileParser::parseVVX, 			IndexData::INDEX_TYPE_VVX },
		{ ".VV2", 		&IndexFileParser::parseVV2, 			IndexData::INDEX_TYPE_VV2 },
//		{ ".vv2", 		&IndexFileParser::parseVV2, 			IndexData::INDEX_TYPE_VV2 },
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
					if( _idxParserEnv.skipZeroByteFile() && ( reader->getFileSize(indexFileName, true ) <= 0))
					{
						IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(parseIndexFile,"Parse Content[%s] with indexFile[%s], but index's size is 0"),
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

			if( ( _idxParserEnv.skipZeroByteFile()  ? ( reader->getFileSize( idxFileName, true )  > 0 )  : true ) && 
				reader->open( idxFileName ) )
			{
				if( (this->*funcRoute[id]._parser)(reader,idxData))
				{
					IDXPARSERLOG(ZQ::common::Log::L_DEBUG,
						CLOGFMT(IndexFileParser,"parse index file [%s] for [%s] ok, "
						"and mainFileSize[%lld] bitRate[%u] playTime[%u]"),
						reader->getFileName().c_str(),
						mainFileName.c_str(), idxData.getMainFileSize(), idxData.getMuxBitrate(), idxData.getPlayTime() );
					idxData.indexDataType = funcRoute[id]._type;
					idxData.indexFileName = idxFileName;
					return true;
				}
				else
				{
					IDXPARSERLOG(ZQ::common::Log::L_DEBUG,
						CLOGFMT(IndexFileParser,"failed to parse index file [%s] for content[%s]"),
						idxFileName.c_str(), mainFileName.c_str() );
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


bool IndexFileParser::ParseIndexFileFromVstrm( const std::string& mainFileName, 
											  IndexData& idxData, 
											  bool bIndexFile,
											  const std::string& indexFileName, 
											  bool bLoadBrief, bool bLocalContent )
{	
#ifndef EXCLUDE_VSTRM_API
	FileReaderVstrmI reader(_idxParserEnv);	
	if( _idxParserEnv.canUseVstrmIndexParseAPI() && _idxParserEnv.isVstrmLoadInfoMethodAvailable() )
	{
		if( bLoadBrief)
		{
			IDXPARSERLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(ParseIndexFileFromVstrm,"Parse Content[%s] using API{VstrmClassLoadBriefAssetInfo}"),mainFileName.c_str());
			return parseIndexUsingVstrmAPILoadBrief( &reader, mainFileName, idxData, bLocalContent );
		}
		else
		{
			IDXPARSERLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(ParseIndexFileFromVstrm,"Parse Content[%s] using API{VstrmClassLoadFullAssetInfo}"),mainFileName.c_str());
			return parseIndexUsingVstrmAPILoadAll( &reader, mainFileName, idxData );
		}
	}
	else
	{
		return parseIndexFile( &reader, mainFileName, idxData, bIndexFile, indexFileName );
	}
#else
	return false;
#endif//#ifndef EXCLUDE_VSTRM_API
}
std::string	IndexFileParser::getLastError( )
{
	return _lastError;
}


bool IndexFileParser::ParserIndexFileFromCommonFS( const std::string& mainFilePathName, IndexData& idxData, 
												  bool bIndexFile, const std::string& indexFileName )
{
	//now we can only support NTFS
	FileReaderNative reader(_idxParserEnv);
	return parseIndexFile( &reader, mainFilePathName, idxData, bIndexFile, indexFileName );
}

bool IndexFileParser::ParseIndexFromMemory( const std::string& mainFilePathName, IndexData& idxData, const char* buffer, size_t size)
{
	MemFileReader reader((const unsigned char*)buffer,size,mainFilePathName,size);//test
	return parseIndexFile( &reader, mainFilePathName, idxData, false, "" );
}

IndexData::FrameRate IndexFileParser::convertFrameRateCode( ULONG framRateCode )
{
	return (IndexData::FrameRate)framRateCode;
}

bool IndexFileParser::parserVVxVersion5( FileReader* reader, const VVX_INDEX_HEADER& header, IndexData& idxData )
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
	if( reader->seek( subFileOffset, FileReader::SEEK_FROM_BEGIN) != subFileOffset )
	{
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"[%s] can't seek to [%lld]"), 
			reader->getFileName().c_str(),subFileOffset);
		return false;
	}
	
	VVX_SUBFILE_INFORMATION* pSubInfo = new VVX_SUBFILE_INFORMATION[idxData.subFileCount];
	size_t	readSize = sizeof(VVX_SUBFILE_INFORMATION ) *  idxData.subFileCount;
	if( reader->read( pSubInfo, readSize ) != (int)readSize )
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

bool IndexFileParser::parserVVxVersion6( FileReader* reader, const VVX_INDEX_HEADER& header, IndexData& idxData )
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
	if( reader->seek( subFileOffset, FileReader::SEEK_FROM_BEGIN) != subFileOffset )
	{
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"[%s] can't seek to [%lld]"), 
					reader->getFileName().c_str(),subFileOffset);
		return false;
	}
	//read sub file information
	VVX_V6_SUBFILE_INFORMATION* pSubFileInfo = new VVX_V6_SUBFILE_INFORMATION[v6Header->subFileInformationCount];
	assert( pSubFileInfo != NULL );

	size_t	readSize = sizeof(VVX_V6_SUBFILE_INFORMATION ) *  v6Header->subFileInformationCount;
	if( reader->read( pSubFileInfo, readSize ) != (int)readSize )
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

bool IndexFileParser::parserVVxVersion7( FileReader* reader, const VVX_INDEX_HEADER& header, IndexData& idxData )
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
	if( reader->seek( subFileOffset, FileReader::SEEK_FROM_BEGIN ) !=  subFileOffset )
	{
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"[%s] can't seek to [%lld]"),
			reader->getFileName().c_str(), subFileOffset );
		return false;
	}

	//read sub file information
	VVX_V6_SUBFILE_INFORMATION* pSubFileInfo = new VVX_V6_SUBFILE_INFORMATION[v7Header->subFileInformationCount];
	assert( pSubFileInfo != NULL );

	size_t	readSize = sizeof(VVX_V6_SUBFILE_INFORMATION ) *  v7Header->subFileInformationCount;
	if( reader->read( pSubFileInfo, readSize ) != (int)readSize )
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

bool IndexFileParser::parseVVX( FileReader* iReader, IndexData& idxData  )
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
		readbufSize, iReader->getFileName().c_str() );

	MemFileReader memreader( szBuffer, readbufSize, iReader->getFileName(),iReader->getMsize() );

	MemFileReader* reader = & memreader;

	assert( reader != NULL );
	VVX_INDEX_HEADER vvxHeader;
	memset(&vvxHeader, 0, sizeof(vvxHeader));	
	if( reader->read( &vvxHeader, sizeof(vvxHeader) ) != sizeof(vvxHeader) )
	{
		return false;
	}
	if( strcmp( reinterpret_cast<const char*>(vvxHeader.signature), VVX_INDEX_SIGNATURE) !=0 )
	{
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"file[%s] Invalid Vvx Header Signature"),reader->getFileName().c_str());
		return false;
	}

	switch ( vvxHeader.majorVersion )
	{
	case 5:
		if( !parserVVxVersion5( reader, vvxHeader, idxData ) )
			return false;
		break;
	case 6:
		if( !parserVVxVersion6( reader, vvxHeader, idxData ) )
			return false;
		break;
	case 7:
		if( !parserVVxVersion7( reader, vvxHeader, idxData ) )
			return false;
		break;
	default:
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"invalid header version[%d] for file[%s]" ),
				vvxHeader.majorVersion, reader->getFileName().c_str() );
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

	IDXPARSERLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(IndexFileParser,"read[%d] bytes from file[%s]"), readbufSize, iReader->getFileName().c_str() );

	MemFileReader memreader( szBuffer, readbufSize, iReader->getFileName(),iReader->getMsize() );	

	MemFileReader* reader = &memreader;

	//get main file name
	
	 if( !parser.parseCsicoINDEX( reader, idxData) )
		return false;
	
	 if( idxData.subFileInfos.size() > 0 )
	 {
		 //idxData.mainFileSize = iReader->getFileSize( _mainFileName + idxData.subFileInfos[0].fileExtension );
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
typedef std::map< int, Vv2SubFileInfo > Vv2SubFileInfoMap;


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
#define GET_LINE_STRING(x,y)	if( reader->getLine( szLine, lineStrLen ) == NULL ) \
{\
	IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,#x));\
	##y;\
}
#else
#define GET_LINE_STRING(x,y)	if( reader->getLine( szLine, lineStrLen ) == NULL ) \
{\
	IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,#x));\
	y;\
}
#endif

#define LINE_IS_VERSION(x) ( memcmp( x, pVersion, sizeVersion ) == 0 )

#define LINE_IS_SOH(x)	( memcmp( x, pHeaderSOH, sizeSOH ) == 0 )

#define LINE_IS_DRIVERTSOVERIP(x) ( memcmp( x, pHeaderDRIVERTSOVERIP, sizeDRIVERTSOVERIPR ) == 0 )

#define LINE_IS_NOEMBEDDEDHINTS(x) ( memcmp( x, pHeaderNOEMBEDDEDHINTS, sizeNOEMBEDDEDHINTS ) == 0 )

#define LINE_IS_INDXRECFMT2(x)	( memcmp( x, pHeaderINDXRECFMT2, sizeINDXRECFMT2 ) == 0 )

#define LINE_IS_INDXRECFMT3(x) ( memcmp( x, pHeaderINDXRECFMT3, sizeINDXRECFMT3 ) == 0 )

#define LINE_IS_FILE(x)	( memcmp( x, pHeaderFILE, sizeFILE ) == 0 )

#define LINE_IS_BITRATE0(x) ( memcmp( x, pHeaderBITRATE0, sizeBITRATE0 ) == 0 )

#define LINE_IS_BITRATE(x)	( memcmp( x, pHeaderBITRATE, sizeBITRATE ) == 0 )

#define LINE_IS_DURATION(x)	( memcmp( x, pHeaderPlayDuration, sizePlayDuration ) == 0 )

#define LINE_IS_TBD(x)	( memcmp( x, pHeaderTBD, sizeTBD ) == 0 )

bool IndexFileParser::parseVV2( FileReader* reader, IndexData& idxData  )
{
	assert( reader != NULL );
	//parse the vv2 file line by line
	char	szLine[1024];
	size_t	lineStrLen = sizeof(szLine);
	
	GET_LINE_STRING ( "Invalid vv2 file, can't get SOH", return false );
	if( !LINE_IS_SOH(szLine) )	return false;
	
	GET_LINE_STRING( "Invalid vv2 file,get driverflavor", return false);
	if( !LINE_IS_DRIVERTSOVERIP(szLine) ) return false;

	for( int i = 0 ;i < 3 ;i ++ )
	{//try to skip no useful information
		GET_LINE_STRING("Invalid vv2 file, can't get more data", return false );
		if( LINE_IS_NOEMBEDDEDHINTS(szLine ) ||
			LINE_IS_INDXRECFMT3(szLine)||
			LINE_IS_INDXRECFMT2(szLine) )
			break;
	}	
	//GET_LINE_STRING("Invalid vv2 file, can't get more data", return false );
	if( LINE_IS_NOEMBEDDEDHINTS(szLine) )
	{
		GET_LINE_STRING("Invalid vv2 file, can't get INDEXFORMAT3 ", return false );
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
		if(!ZQ::common::stringHelper::SplitString( szLine, tempVec, ":",":","","") && tempVec.size() != 6 )
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"Invalid vv2 subfile information"));
			return false;
		}
		//ignore the leading char '#'
		strncpy( infoSub.szFileName, tempVec[1].c_str() + 1, sizeof(infoSub.szFileName) -1 );
		sscanf(tempVec[3].c_str(),"%X",&infoSub.numerator);			
		sscanf(tempVec[4].c_str(),"%X",&infoSub.denominator);		
		infoSub.direction	=	stricmp( tempVec[5].c_str(), "F" ) == 0 ? 1 : -1;
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
		sscanf( szPlayDuration, "%X", &idxData.mainFilePlayTime );
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

bool IndexFileParser::parseExtension( FileReader* reader, IndexData& idxData )
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
			CLOGFMT(IndexFileParser,"bad xml schema, can't get root node from the xml for file[%s]'s extension file"),
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
			memset( szTemp, 0, sizeof(szTemp));
			pNode->get("requiredInLeadCopy",szTemp,"0",sizeof(szTemp));
			if ( atoi(szTemp) >= 1 )
			{
				memset( szTemp, 0, sizeof(szTemp));
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
			memset( szTemp, 0, sizeof(szTemp));
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

bool CsicoIndexFileParser::getTag( FileReader* reader, IndexRecordHeader& tal, IndexRecord& rec, bool IsNormalRec) {
	int32 nsize = -1;
	do {
		if( (nsize = reader->read(&tal,sizeof(tal))) != sizeof(tal))
		{
			//get last part of record tag
			reader->seek(-nsize,FileReader::SEEK_FROM_CUR);
			//reader->read(rec.mPartRecInfo.mPartRecTag,nsize);
			//rec.mPartRecInfo.state = IndexRecord::HEADER;
			//rec.mPartRecInfo.LenOfRecTag = nsize;
			IDXPARSERLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(IndexFileParser,"getTag NOTFULL nsize[%d], curpos[%ld] mSize[%ld]"),nsize, reader->tell(),reader->getMsize());
			//rec.mPartRecInfo.IsNormalRecord = IsNormalRec;
			//rec.lastParsedOffset(reader->tell());
			return false;
		}
		if(tal.type != 0x00)
			return true;
		do 
		{
			if(tal.length != 0x00) {
				reader->seek(-1,FileReader::SEEK_FROM_CUR);
				if( (nsize = reader->read(&tal,sizeof(tal))) != sizeof(tal))
				{
					return false;
				}
				return true;
			}
			// it's not effciently to read data like this.
			// may using it at memory file reader
			if( (nsize = reader->read(&tal.length,sizeof(tal.length))) != sizeof(tal.length))
			{
				return  errorCodeNotFull; //false;
			}
		} while (true);
	}while(true);	
	return true;
}

bool CsicoIndexFileParser::getTag( FileReader* reader, CsicoIndexFileTagAndLength& tal )
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
		if(reader->read( &c, sizeof(c))!=sizeof(c))
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"insufficient data to parse of [%s]"),reader->getFileName().c_str());
			return false;
		}
		char* pTemp = (char*)&tal;
		pTemp ++;
		memcpy( &tmpTal, pTemp, sizeof(tal) -1 );
		pTemp = (char*)&tmpTal;
		pTemp += sizeof(tmpTal) -1;
		memcpy( pTemp,&c, sizeof(c));
		memcpy( &tal,&tmpTal,sizeof(tal));
	}
	return true;
}

std::string	CsicoIndexFileParser::readString( FileReader* reader, size_t size )
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

#define CSICOINDEX_TAG_ASSETINFOSECTION								0x110001FF

#define CSICOINDEX_TAG_SUBFILESECTION								0x110005FF
#define CSICOINDEX_TAG_ZEROMOTION                                   0x110006FF
#define CSICOINDEX_TAG_ZEROMOTIONPFRAME                             0x16000602
#define CSICOINDEX_TAG_ZEROMOTIONBFRAME                             0x16000601

#define CSICOINDEX_TAG_MUXBITRATE									0x13000109
#define CSICOINDEX_TAG_HORIZONTALSIZE								0x1300010C
#define CSICOINDEX_TAG_VERTICALSIZE									0x1300010D

#define CSICOINDEX_TAG_SUBFILECOUNT									0x13000117

#define CSICOINDEX_TAG_SUBFILEPLAYTIME								0x1300050A
#define CSICOINDEX_TAG_SUBFILESPEEDNUMERATOR						0x1200050C
#define CSICOINDEX_TAG_SUBFILESPEEDDENOMINATOR						0x1300050D
#define CSICOINDEX_TAG_SUBFILENAME									0x15000502
#define CSICOINDEX_TAG_SUBFILEINDEX									0x13000501
#define CSICOINDEX_TAG_SUBFILE_STARTBYTE							0x13000504
#define CSICOINDEX_TAG_SUBFILE_ENDBYTE								0x13000505

#define CSICOINDEX_TAG_PWE											0x1400011B



bool CsicoIndexFileParser::skipFileHeader(FileReader* reader ) {
	CsicoIndexFileHeader	fileHeader;
	memset(&fileHeader, 0, sizeof(fileHeader));
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
	reader->seek( tal.length, FileReader::SEEK_FROM_CUR  );
	return true;
}

void CsicoIndexFileParser::storeRecordData(const char* header, int32 headerSize, const char* body, int32 bodySize) {
	headerSize = MAX(0,headerSize);
	bodySize = MAX(0, bodySize);
	if(headerSize > 0 ) {
		memcpy(mTempRecordBuffer, header, headerSize);
	}
	if(bodySize > 0) {
		memcpy(mTempRecordBuffer + headerSize, body, bodySize);
	}
	*mTempRecordDataSize = headerSize + bodySize;
	*mTempRecord = mTempRecordBuffer;
}

int32 CsicoIndexFileParser::readRecordData(FileReader *reader, char *buf, size_t count ) {
	size_t pos = 0;
	if(*mTempRecordDataSize >0) {
		size_t delta = MIN((*mTempRecordDataSize), count);
		memcpy(buf + pos, *mTempRecord, delta);
		count -= delta;
		pos += delta;
		*mTempRecord += delta;
		*mTempRecordDataSize -= delta;
		if (*mTempRecordDataSize == 0) {
			*mTempRecord = mTempRecordBuffer;
		}
	}
	if(count > 0) {
		int32 rc = reader->read(buf+pos,count);
		pos += rc;
	}
	return (int32)pos;
}

int32 CsicoIndexFileParser::readRecordHeader(FileReader* reader, IndexRecordHeader& recordHeader ) {
	size_t bytesGotten = 0;
	while(true) {
		size_t bytesToRead = sizeof(recordHeader) - bytesGotten;
		int32 rc = readRecordData(reader, (char*)&recordHeader + bytesGotten, bytesToRead);
		if(rc > 0) {
			bytesGotten += rc;
		}
		if(rc != bytesToRead) {
			storeRecordData((const char*)&recordHeader, bytesGotten, NULL, 0);
			return errorCodeNotFull;
		}
		if(recordHeader.type == 0x00) {
			if(recordHeader.length == 0x00) {
				bytesGotten = 0;
			} else {
				recordHeader.type = recordHeader.length;
				bytesGotten = 1;
			}
			continue;
		}
		break;
	};
	return bytesGotten;
}

int32 CsicoIndexFileParser::readRecordBody(FileReader* reader, const IndexRecordHeader& recordHeader, char *buf, size_t size) {
	int32 rc = readRecordData(reader, buf, recordHeader.length);
	if(rc != (int32)recordHeader.length) {
		storeRecordData((const char*)&recordHeader, sizeof(recordHeader), buf, rc);
		return errorCodeNotFull;
	}
	return rc;
}

int32 CsicoIndexFileParser::skipRecordBody(FileReader* reader, const IndexRecordHeader& recordHeader) {
	char buf[256];
	return readRecordBody(reader, recordHeader, buf, sizeof(buf));
}

int32 CsicoIndexFileParser::parseIndexRecord( FileReader* reader, IndexRecord& record ) {
	IndexData indexData;
    SPEED_IND speed;
	if( record.lastParsedOffset() == 0 ) {
		if(!parseCsicoINDEX(reader, indexData))
		{
			record.setHeaderErrCode(errorCodeHeadError);

			return errorCodeHeadError;   //false;
		}
		record.setHeaderErrCode(errorCodeOK);
		reader->seek(0, FileReader::SEEK_FROM_BEGIN);
		/*
		size_t count = IndexData.getSubFileCount();
		if(count>1)
			count -= 1; //exclude normal play file
		record.subfileCount(count);
		record.setRecSize(count);
    	record.setMainExtension(IndexData.getSubFileName(0));//get mainfile extension
        record.setMainPlayTime(IndexData.getPlayTime());
		record.mPartRecInfo.GetSubInfo(IndexData);
		*/

		record.setdatabitrate(indexData.getMuxBitrate());
		record.initIndexRecords(indexData);
		record.setZeroMotionBFrame(indexData);
		record.setZeroMotionPFrame(indexData);

        IDXPARSERLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(parseIndexRecord,"MainExtension[%s], PlayTime[%d] AssetName[%s]"), indexData.getSubFileName(0).c_str(), indexData.getPlayTime(),reader->getFileName().c_str() );
	} else {
		//IDXPARSERLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(IndexFileParser,"record.lastParseOffset[%ld],curPos[%ld],mSize[%ld] AssetName[%s]"),record.lastParsedOffset(),reader->tell(),reader->getMsize(),reader->getFileName().c_str());
	}
	size_t subFileCount = record.subfileCount();
	if( record.lastParsedOffset() == 0) {
		if(!skipFileHeader(reader)) {
			return errorCodeSkipFileHeader;//false;
		}
//	} else {
//		if( record.lastParsedOffset() != reader->seek(record.lastParsedOffset(),FileReader::SEEK_FROM_BEGIN) ) {
//			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"failed to seek to [%llu], file content may corrupt"),
//				reader->getFileName().c_str() );
//			return errorCodeFailToSeek;//false;
//		}
	}
	record.setLastParsedOffset(reader->getMsize());

	record.getTemporaryZone(mTempRecordBuffer, mTempRecord, mTempRecordDataSize);
	record.getLastTimeOffsetPointer(mLastTimeOffset);

//	IDXPARSERLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(parseIndexRecord,"begin record parse curpos[%ld] AssetName[%s]"), reader->tell(),reader->getFileName().c_str() );
	// now we are in the index record territory
	char tempBuffer[256];
	IndexRecordHeader recordHeader;
	IndexRecordInfo	 recordBody;
	int subFileIndex = 0;
	while(true) {
		int32 rc = readRecordHeader(reader, recordHeader);
		if(rc<0) {
			IDXPARSERLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(parseIndexRecord,"readRecordHeader failed errCode[%d] AssetName[%s]"), rc,reader->getFileName().c_str() );
			return rc; //log message goes here
		}
		switch(recordHeader.type) {
			case 0x02:
			case 0x03:
			case 0x0E:
			case 0x0F:
				break;
			default:
			{
				rc = skipRecordBody(reader, recordHeader);
				if (rc<0) {
					IDXPARSERLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(parseIndexRecord,"skipRecordBody failed errCode[%d] AssetName[%s] curpos[%ld]"), rc,reader->getFileName().c_str(), reader->tell());
					return rc;//log message goes here
				}
				continue;
			}
		}
		rc = readRecordBody(reader, recordHeader, tempBuffer, sizeof(tempBuffer));
		if(rc <0) {
			IDXPARSERLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(parseIndexRecord,"readRecordBody failed errCode[%d] AssetName[%s]"), rc,reader->getFileName().c_str() );
			return rc;//log message goes here
		}
		switch(recordHeader.type) {
		case 0x02:
			{
				recordBody.packetOffset = *(uint32*)tempBuffer;
				memset(&tempBuffer[17], 0, 10);
				recordBody.timeOffset = (*(uint64*)(&tempBuffer[11]))/27000;//PCR at offset 17 and is 6bytes
				*mLastTimeOffset = recordBody.timeOffset;
				subFileIndex = 0;
			}
			break;
		case 0x03:
			{
				subFileIndex = *(uint8*)(&tempBuffer[0]);
				recordBody.packetOffset = *(uint32*)(&tempBuffer[1]);
				recordBody.timeOffset = *mLastTimeOffset;
			}
			break;
		case 0x0E:
			{
				uint8 flag = *(uint8*)(&tempBuffer[6]);
				if (record.skipRecord(indexData))
				{
					//if((flag & 0x08)==0){
					//	continue;  //skip if it is not start of a frame
					//}
					//if ((flag & 0x80) == 0){
					//	continue; //skip non-IDR picture
					//}
				}
				subFileIndex = 0;
				recordBody.packetOffset = *(uint32*)(&tempBuffer[0]);
				memset(&tempBuffer[16], 0, 10);
				recordBody.timeOffset = (*(uint64*)(&tempBuffer[10]))/27000;
				*mLastTimeOffset = recordBody.timeOffset;
			}
			break;
		case 0x0F:
			{
				uint8 flag = *(uint8*)(&tempBuffer[5]);
				if (record.skipRecord(indexData))
				{
					//if((flag & 0x08) == 0) {
					//	continue; //skip if it is not start of a frame
					//}
					//if((flag & 0x80) == 0) {
					//	continue; //skip non-IDR picture
					//}
				}
				subFileIndex = *(uint8*)(&tempBuffer[0]);
				recordBody.packetOffset = *(uint32*)(&tempBuffer[1]);
				recordBody.timeOffset = *mLastTimeOffset;
			}
			break;
		default:
			{
				assert(false&&"should never be here");
				//log an error message here
			}
		}
		//store record body
		record.addIndexRecord(recordBody, subFileIndex);
		//IDXPARSERLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(parseIndexRecord,"addIndexRecord type[0X%02x] length[%d] timeoffset[%lu] packet[%010x] sub[%d] curpos[%ld] AssetName[%s]"), recordHeader.type, recordHeader.length,recordBody.timeOffset,recordBody.packetOffset,subFileIndex,reader->tell(),reader->getFileName().c_str() );
	}
	record.sortRecordTrcik();
	return errorCodeOK;
}



bool CsicoIndexFileParser::parseCsicoINDEX( FileReader* reader, IndexData& idxData )
{
	CsicoIndexFileHeader	fileHeader;
	memset(&fileHeader, 0, sizeof(fileHeader));
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
	
	if ( fileHeader.majorVersion == 1  )
	{
		if( !(fileHeader.minorVersion == 0 || fileHeader.minorVersion == 1) )
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,	CLOGFMT(IndexFileParser,"unexpect version[%d.%d] for file[%s]"),
				fileHeader.majorVersion, fileHeader.minorVersion, reader->getFileName().c_str() );
		}
		return parseCsicoINDEX11( reader, idxData );
	}
	else
	{
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,	CLOGFMT(IndexFileParser,"Unsupport version[%d.%d] for file[%s]"),
			fileHeader.majorVersion, fileHeader.minorVersion, reader->getFileName().c_str() );
		return false;
	}
}


std::string CsicoIndexFileParser::getSubType( FileReader* reader, const std::string &sxmlContent)
{
	SimpleXMLParser parser;
	std::string xmlContent = "<root>" ;
	xmlContent = xmlContent + sxmlContent;
	//xmlContent.resize( strlen(xmlContent.c_str() ) );
	xmlContent = xmlContent + "</root>";
	try
	{
		parser.parse( xmlContent.c_str(), static_cast<int>( xmlContent.length() ), 1 );
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
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"failed to parse subfile extension for [%s] with [%s], can't get subtype"),
				reader->getFileName().c_str(), xmlContent.c_str() );
		}
	}
	catch( ZQ::common::ExpatException& )
	{
		IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"failed to parse subfile extension for [%s] with [%s]"),
			reader->getFileName().c_str(), xmlContent.c_str() );
	}
	return "";
}

bool CsicoIndexFileParser::parseSubFiles( FileReader* reader, IndexData& idxData)
{
	CsicoIndexFileTagAndLength tal;
	int	subFileIndex = -1;
	IndexData::SubFileInformation subInfo;

		bool bIndexOk				= false;
		bool bSpeedNumeratorOk		= false;
		bool bSpeedDenominatorOk	= false;
		bool bNameOk				= false;
		bool bPlayTimeOK			= false;
		bool bStartingByteOK		= false;
		bool bEndinByteOK			= false;
		while( !( bIndexOk && bSpeedNumeratorOk && bSpeedDenominatorOk && bNameOk && bPlayTimeOK && bEndinByteOK && bStartingByteOK ) )
		{
			if( !getTag( reader,tal) )
			{
				IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"insufficient data to parse of [%s]"),reader->getFileName().c_str());
				return false;
			}
			switch ( tal.tagId )
			{
			case CSICOINDEX_TAG_SUBFILEINDEX:
				{
					subInfo.subFileIdx = readAndReinterpretAs<uint8>( reader, tal.length );					
					subFileIndex = subInfo.subFileIdx;
					bIndexOk = true;
				}
				break;
			case CSICOINDEX_TAG_SUBFILEPLAYTIME:
				{
					subInfo.playtime	= readAndReinterpretAs<uint32>( reader, tal.length );
					bPlayTimeOK = true;
				}
				break;
			case CSICOINDEX_TAG_SUBFILESPEEDDENOMINATOR:
				{
					subInfo.speed.denominator = (USHORT)readAndReinterpretAs<uint32>( reader, tal.length );
					 bSpeedDenominatorOk = true;
				}
				break;
			case CSICOINDEX_TAG_SUBFILESPEEDNUMERATOR:
				{
					subInfo.speed.numerator = (SHORT)readAndReinterpretAs<uint32>( reader, tal.length );
					bSpeedNumeratorOk = true;
				}
				break;
			case CSICOINDEX_TAG_SUBFILENAME:
				{
					subInfo.fileExtension = getSubType( reader, readString( reader, tal.length));
					bNameOk = true;
				}
				break;
			case CSICOINDEX_TAG_SUBFILE_STARTBYTE:
				{
					subInfo.startingByte = read5byteInt(reader);
					bStartingByteOK = true;
				}
				break;
			case CSICOINDEX_TAG_SUBFILE_ENDBYTE:
				{
					subInfo.endingByte = read5byteInt( reader);
					bEndinByteOK = true;
				}
				break;
			default:
				{
					reader->seek( tal.length, FileReader::SEEK_FROM_CUR  );
				}
				break;
			}
		}
		if ( subFileIndex == 0 )
		{
			idxData.mainFilePlayTime	= subInfo.playtime;

			idxData.mainFileName		= idxData.mainFileName + subInfo.fileExtension;

            idxData.mainFileSize        = subInfo.endingByte + 1;
			
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
    bool bPweOK = false;

	while( !( bMuxRateOk && bVerticalSizeOk && bHorizontalSizeOk && bSubFileCountOk && bPweOK ) )
	{
		if( !getTag( reader,tal))
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
				idxData.verticalResolution = readAndReinterpretAs<uint16>(reader, tal.length);
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
		case CSICOINDEX_TAG_PWE:
			{
				idxData.pwe = readAndReinterpretAs<uint8>(reader, tal.length) != 0;
                bPweOK = true;
			}
			break;
		default:
			{
				reader->seek( tal.length, FileReader::SEEK_FROM_CUR  );
			}
		}
	}
	return true;
}

bool CsicoIndexFileParser::parseZeroMotion(FileReader* reader, IndexData& idxData)
{
	CsicoIndexFileTagAndLength tal;
	bool bPFrame = false, bBFrame = false;
	while (!bPFrame || !bBFrame)
	{
		if (!getTag(reader,tal))
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"insufficient data to parse of [%s]"),reader->getFileName().c_str());
			return false;
		}
		switch (tal.tagId)
		{
			case CSICOINDEX_TAG_ZEROMOTIONPFRAME:
				{
					if ( tal.length < 188){
						reader->seek(tal.length,FileReader::SEEK_FROM_CUR);
						IDXPARSERLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(IndexFileParser,"zeromotionpframe length[%d] parse of [%s]"),tal.length,reader->getFileName().c_str());
					}
					else{
						idxData.zeroPFrameLength = 188 ;
						reader->read(idxData.zeroPFrame,188);
						reader->seek(tal.length - 188,FileReader::SEEK_FROM_CUR);
					}
					bPFrame = true;
				}
				break;
			case CSICOINDEX_TAG_ZEROMOTIONBFRAME:
				{
					if (tal.length < 188){
						reader->seek(tal.length,FileReader::SEEK_FROM_CUR);
						IDXPARSERLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(IndexFileParser,"zeromotionbframe length[%d] parse of [%s]"),tal.length,reader->getFileName().c_str());

					}
					else{
						idxData.zeroBFrameLength = 188;
						reader->read(idxData.zeroBFrame,188);
						reader->seek(tal.length - 188,FileReader::SEEK_FROM_CUR);
					}
					bBFrame = true;
				}
				break;
		}
	}
	return true;
}

bool CsicoIndexFileParser::parseSection(  FileReader* reader, IndexData& idxData )
{
	CsicoIndexFileTagAndLength tal;
	int subFileCount = 0;
	bool bAssetInfoOk = false;
	bool bSubFileInfoOk = false, bZeroMotionOk = false;
	while( !bAssetInfoOk  || !bSubFileInfoOk || !bZeroMotionOk)
	{
		if( !getTag( reader,tal) )
		{
			IDXPARSERLOG(ZQ::common::Log::L_ERROR,CLOGFMT(IndexFileParser,"insufficient data to parse of [%s]"),reader->getFileName().c_str());
			return false;
		}
		
		switch ( tal.tagId )
		{
		case CSICOINDEX_TAG_ASSETINFOSECTION:
			{
				if( !parseAssetInfo(reader, idxData ))
					return false;
				else
					bAssetInfoOk = true;

			}
			break;

		case CSICOINDEX_TAG_SUBFILESECTION:
			{
				if( !parseSubFiles( reader, idxData ))
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
				if(!parseZeroMotion(reader, idxData))
				{
					return false;
				}else{
					bZeroMotionOk = true;
				}
			}
			break;
		default:
			{
				reader->seek( tal.length, FileReader::SEEK_FROM_CUR  );
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
		if( !getTag( reader,tal) )
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
				reader->seek( tal.length, FileReader::SEEK_FROM_CUR  );
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
	MemFileReader reader((const unsigned char*)buffer,size,mainFilePathName,size);//test
	return parserCsicoIndexRecord(&reader,mainFilePathName,record);
}

int32 IndexFileParser::ParseIndexRecodFromMemoryPWE( const std::string& mainFilePathName,
												IndexRecord& record, 
												const char* buffer, 
												int64 sz, int64 blocksize) {
	IDXPARSERLOG(ZQ::common::Log::L_INFO, CLOGFMT(IndexFileParser,"ParseIndexRecordFromMemoryPWE offset[%ld]"),blocksize);
	MemFileReader reader((const unsigned char*)buffer,sz,mainFilePathName,sz+blocksize);
	return parserCsicoIndexRecordPWE(&reader,mainFilePathName,record);
}

bool IndexFileParser::ParseIndexRecordFromCommonFS( const std::string& mainFilePathName, 
												 IndexRecord& record, 
												 const std::string& indexFileName ) {
	 FileReaderNative reader(_idxParserEnv);
	 if(!reader.open(indexFileName)) {
		 IDXPARSERLOG(ZQ::common::Log::L_ERROR, CLOGFMT(IndexFileParser,"failed to open[%s]"),indexFileName.c_str());
		 return  false;
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
	 MemFileReader memreader((const unsigned char*)p,dataSize, mainFilePathName,filesize);
	 bool bOK =  parserCsicoIndexRecord(&memreader,mainFilePathName,record);
	 free(p);
	 return bOK;
}

bool IndexFileParser::parserCsicoIndexRecord(ZQ::IdxParser::FileReader *reader, 
										const std::string &mainFileName,
										ZQ::IdxParser::IndexRecord &record) {
	CsicoIndexFileParser parser(_idxParserEnv, mainFileName);
	return parser.parseIndexRecord(reader, record);
}

int32 IndexFileParser::parserCsicoIndexRecordPWE(ZQ::IdxParser::FileReader *reader, 
										const std::string &mainFileName,
										ZQ::IdxParser::IndexRecord &record) {
	CsicoIndexFileParser parser(_idxParserEnv, mainFileName);
	return parser.parseIndexRecord(reader, record);
}


//////////////////////////////////////////////////////////////////////////
IndexRecord::IndexRecord()
:mLastOffset(0),
mLastParsedOffset(0),
mLastTimeOffset(0),
mPwe(false),
mSubFileCount(0),
mParsedRecords(0),
mZeroBFrameLen(0),
mZeroPFrameLen(0),
mTargetScale(1.0),
mbReverseDir(false),
mpZeroBFrame(NULL),
mpZeroPFrame(NULL){
	mRecTrickInfo.reserve(20 * 1000);
	//mRecords.reserve( 20 * 1000 );
	mTempRecord = mTempRecordBuffer[0];
	mTempRecordDataSize = 0;
}

IndexRecord::~IndexRecord(){
}

void IndexRecord::setZeroMotionBFrame(const IndexData& idxdata)
{
	mpZeroBFrame = idxdata.getZeroMotionBFrame(mZeroBFrameLen); 
}

void IndexRecord::setZeroMotionPFrame(const IndexData& idxdata)
{
	mpZeroPFrame = idxdata.getZeroMotionPFrame(mZeroPFrameLen);
}

unsigned const char* IndexRecord::getZeroMotionPFrame(uint16& len) const
{
	len = mZeroPFrameLen;
	return mpZeroPFrame;
}

unsigned const char* IndexRecord::getZeroMotionBFrame(uint16& len) const
{
	len = mZeroBFrameLen;
	return mpZeroBFrame;
}

int IndexRecord::getSubFileIdx(float& speed)
{
	ZQ::common::MutexGuard gd(mLocker);
	int rateIdx= 0;
	size_t size = mRecTrickInfo.size();
	int beg,next,i = 1;
	bool flag = false;
	if (speed >= 0.0)
	{
		if (speed <=1.00)//normal rate
		{
			int idx = -1;
            speed = 1.0;
			return 0;
		}

		for (i = 1;i<size;i++) //i = 0
		{
			if (mRecTrickInfo[i].scale >=1.0 )
				break;
		}
		beg = i;//����ʼ�Ƚϵ�
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
			if (fabs((float) speed-mRecTrickInfo[beg].scale) <= fabs((float) speed-mRecTrickInfo[next].scale))
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
		beg = 1; //beg = 0
		if ( speed < mRecTrickInfo[beg].scale )
		{
			rateIdx = beg;
		}else{
			next = beg + 1;
			while (true)
			{
				if (mRecTrickInfo[next].scale > 0.0 ){  //��һ������Ϊ��
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
				if ( fabs((float)(speed - mRecTrickInfo[beg].scale)) < fabs((float)(speed - mRecTrickInfo[next].scale)) ){
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

std::string IndexRecord::getSubfileExt(float& speed, uint32& playtime, int& idx)
{
	
    if ( IsNormalRate(speed) )
    {
		if (mRecTrickInfo.empty() || mRecTrickInfo[0].info.empty())
			return "";
    	playtime = mRecTrickInfo[0].info.back().timeOffset;//mRecords.back().timeOffset;
        speed = 1.0;
        return mRecTrickInfo[0].fileExt;//mainExtension;
    }
	
	ZQ::common::MutexGuard gd(mLocker);
	int rateIdx = getSubFileIdx(speed);
	idx = rateIdx;
	if (mRecTrickInfo.empty() || mRecTrickInfo[rateIdx].info.empty())
		return "";
	playtime = mRecTrickInfo[rateIdx].info.back().timeOffset;
    speed = mRecTrickInfo[rateIdx].scale;
	mbReverseDir = (mTargetScale * speed >0)? false:true;
	mTargetScale = speed ;
	return mRecTrickInfo[rateIdx].fileExt;
}

uint64 IndexRecord::findNextIFrame( uint64 Offset)const {
    //if(mRecords.size() == 0)
	//	return 0;
	//if(mRecords.size() == 1)
	//	return packet2offset(mRecords[0].packetOffset);

	ZQ::common::MutexGuard gd(mLocker);
	if (mRecTrickInfo.empty())
		return 0;
	if (mRecTrickInfo.size() == 1 && !mRecTrickInfo[0].info.empty())
		return packet2offset(mRecTrickInfo[0].info[0].packetOffset);
	const IndexRecordInfo* low = firstItem();
	const IndexRecordInfo* high = lastItem();
	assert( low != NULL && high != NULL );
	uint32 packet = (uint32)(Offset/188);//TS packet alignment

	while( (low+1) < high ) {
		if( packet < low->packetOffset ){
			return packet2offset(low->packetOffset);
        }
		else if(packet > high->packetOffset){
			return packet2offset(high->packetOffset);
        }
		const IndexRecordInfo* middle = low+(high-low)/2;
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

bool IndexRecord::addIndexRecord(IndexRecordInfo& record, int subFileIdx) {
	ZQ::common::MutexGuard gd(mLocker);
	int idx = mSubFileIdx2RecordSlot[subFileIdx];
	if (mRecTrickInfo.empty())
		return false;
	if(idx < 0 || idx > (int)mRecTrickInfo.size()) {
		return false;
	}
	RecordTrickInfo& subfileRecord = mRecTrickInfo[idx];
	RECORDS& records = subfileRecord.info;
	if(records.empty()) {
		subfileRecord.firstPCR = record.timeOffset;
		record.timeOffset -= subfileRecord.firstPCR;
		records.push_back(record);
		if (idx == 0) mParsedRecords ++;
	} else {
		if(record.timeOffset < subfileRecord.firstPCR ) {
		static uint64 pcrEnd = 2L^48 / 27000;
		int64 delta = (pcrEnd - subfileRecord.firstPCR);
		record.timeOffset += (pcrEnd - subfileRecord.firstPCR);
		}
		record.timeOffset -= subfileRecord.firstPCR;
		uint64 lastTimeOffset = records[records.size()-1].timeOffset;
		if(record.timeOffset < lastTimeOffset) {
			record.timeOffset += lastTimeOffset;
		}
		records.push_back(record);
		if (idx ==0) mParsedRecords++;
	}
	return true;
}

//void IndexRecord::addRecord( uint32 packetOffset, uint32 timeOffset) {
//	IndexRecordInfo r;
//	r.packetOffset = packetOffset;
//	r.timeOffset = timeOffset;
//	mRecords.push_back(r);
//}

/*
void IndexRecord::addRecTrick(uint32 pkgOffset, uint32 timeOffset, SPEED_IND speed, uint8 subfileidx, const std::string ext,uint32 playtime)
{

	IndexRecordInfo recinfo;
	recinfo.packetOffset = pkgOffset;
	recinfo.timeOffset = timeOffset;
	//for test 
	int idx;
	if (subfileidx == 1) idx = 1; //FF
	else if (subfileidx == 2) idx = 0;//FR
	mRecTrickInfo[idx].info.push_back(recinfo);
	mRecTrickInfo[idx].speed = speed;
	mRecTrickInfo[idx].scale = (float)speed.numerator / (float)speed.denominator;
	mRecTrickInfo[idx].fileExt = ext;
    mRecTrickInfo[idx].playtime = playtime;
	//mRecTrickInfo[subfileidx-1].info.push_back(recinfo);
	//mRecTrickInfo[subfileidx-1].speed = speed;
	//mRecTrickInfo[subfileidx-1].scale = (float)speed.numerator / (float)speed.denominator;
	//mRecTrickInfo[subfileidx-1].fileExt = ext;
    //mRecTrickInfo[subfileidx-1].playtime = playtime;
}
*/

void IndexRecord::sortRecordTrcik()
{
	ZQ::common::MutexGuard gd(mLocker);
	if (mRecTrickInfo.empty())
		return;
	RECTRICKINFO::iterator it = mRecTrickInfo.begin();
	if (++it != mRecTrickInfo.end())
		std::sort(it, mRecTrickInfo.end(),RecordTrickInfo());
}

void IndexRecord::rewriteRecords(const IndexRecord& rec)
{
	ZQ::common::MutexGuard gd(mLocker);
	mRecTrickInfo.resize(rec.getTrickInfo().size());
	if (mRecTrickInfo.empty()) return ;
	memcpy(&mRecTrickInfo[0],&rec.getTrickInfo()[0],rec.getTrickInfo().size() * sizeof(rec.getTrickInfo()[0]));
	sortRecordTrcik();
	mPwe = rec.IsPwe();
}

bool IndexRecord::IsNormalRate(const float& speed)
{
    if ( speed >= 0.0 && speed <= 1.0)
        return true;
    return false;
}

bool IndexRecord::skipRecord(const IndexData& idxdata)const
{
	size_t subcount = subfileCount();
	for(int i = 0; i<subcount;i++)
	{
		if ( ".FF" == idxdata.getSubFileName(i) || ".FR" == idxdata.getSubFileName(i))
		{
			return false;
		}
	}
	return true;
}

uint64 IndexRecord::findNextIFrameByTimeOffset( uint64& timeOffset , int& rateIdx, float speed) const {
	ZQ::common::MutexGuard gd(mLocker);
    uint32 initTime = timeOffset;
	if (speed == 0.0){
		if(mRecTrickInfo.empty() || mRecTrickInfo[0].info.empty())
			return 0;
		if(mRecTrickInfo.size() == 1 && !mRecTrickInfo[0].info.empty())
			return packet2offset(mRecTrickInfo[0].info[0].packetOffset);
		//if(mRecords.size() == 0)
		//	return 0;
		//if(mRecords.size() == 1)
		//	return packet2offset(mRecords[0].packetOffset);
		const IndexRecordInfo* low = firstItem();
		const IndexRecordInfo* high = lastItem();
		assert( low != NULL && high != NULL );
		return binarySearch(low,high,timeOffset);
	}
	else
	{
		size_t size = mRecTrickInfo.size();
		int beg,next,i = 1; //i=0
		bool flag = false;
		//float arr[]= {-15.0,-7.5,-3.5,3.5,7.5};
		if (speed >= 0.0)
		{
			if (speed <=1.00)//normal rate
			{
				int idx = -1;
				return findNextIFrameByTimeOffset(timeOffset, idx);
			}

			for (i = 1;i<size;i++)
			{
				if (mRecTrickInfo[i].scale >=1.0 )
					break;
			}
			beg = i;//����ʼ�Ƚϵ�
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
				if (fabs((float)speed-mRecTrickInfo[beg].scale) <= fabs((float)speed-mRecTrickInfo[next].scale))
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
			beg = 1; //beg = 0;
			if ( speed < mRecTrickInfo[beg].scale )
			{
				rateIdx = beg;
			}else{
				next = beg + 1;
				while (true)
				{
					if (mRecTrickInfo[next].scale > 0.0 ){  //��һ������Ϊ��
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
					if ( fabs((float)(speed - mRecTrickInfo[beg].scale)) < fabs((float)(speed - mRecTrickInfo[next].scale)) ){
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
		const IndexRecordInfo* low = firstTrickItem(mRecTrickInfo[rateIdx]);
		const IndexRecordInfo* high = lastTrickItem(mRecTrickInfo[rateIdx]);
		assert( low != NULL && high != NULL);
		return binarySearch(low, high, timeOffset);
	}
}

uint64 IndexRecord::binarySearch(const IndexRecordInfo* low, const IndexRecordInfo* high, uint64& timeOffset) const
{
    const IndexRecordInfo* low1 = NULL;
	uint32 inittime = timeOffset;
	while( (low+1) < high ) {
			if( timeOffset < low->timeOffset )
            {
                timeOffset = low->timeOffset;
                return packet2offset(low->packetOffset);
            }
			else if(timeOffset > high->timeOffset)
            {
                timeOffset = high->timeOffset;
                return packet2offset(high->packetOffset);
            }
			const IndexRecordInfo* middle = low+(high-low)/2;
			if( timeOffset < middle->timeOffset ) {
				high = middle;
			} else if( timeOffset > middle->timeOffset ){
				low = middle;
			} else {
                timeOffset = middle->timeOffset;
				return middle->packetOffset;
			}
		}
		timeOffset = high->timeOffset;
        low1 = high -1;
		if (mbReverseDir)//both left and right all ok
		{
			if (fabsSmaller(low1->timeOffset,high->timeOffset,inittime))
			{
				timeOffset = low1->timeOffset;
				return packet2offset(low1->packetOffset);
			}
			else{
				timeOffset = high->timeOffset;
				return packet2offset(high->packetOffset);
			}
		}
		else if (mTargetScale > 0)//same direction and scale >0
		{
			timeOffset = high->timeOffset;
			return packet2offset(high->packetOffset);
		}
		else if (mTargetScale < 0)//same direction and scale <0
		{
			timeOffset = low1->timeOffset;
			return packet2offset(low1->packetOffset);
		}
}
bool IndexRecord::fabsSmaller(const uint32& low, const uint32& high, const uint32& init)const
{
    uint32 left = 0, right = 0;
   if ( init >= low ){
       left = init - low;
   }else{
       left = low - init;
   }

   if ( init >= high ){
       right = init - high;
   }else{
       right = high - init;
   }
   if (left < right){
       return true;
   }else{
       return false;
   }
}
}}//namespace ZQ::IdxParser
