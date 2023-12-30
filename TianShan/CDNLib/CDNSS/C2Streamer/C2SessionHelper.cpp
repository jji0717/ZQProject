
#define _FILE_OFFSET_BITS  64

#include <ZQ_common_conf.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/fs.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/io.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sstream>

#include <TimeUtil.h>
#include <SystemUtils.h>
#include "C2SessionHelper.h"
#include "C2StreamerEnv.h"
#include "LocateSparseTsHeader.h"


namespace C2Streamer
{

C2SessFile::C2SessFile( C2StreamerEnv& env )
:mEnv(env),
mFilePosEnd(0),
mFilePosStart(0),
mbValid(false)
{
}

C2SessFile::~C2SessFile()
{
}
void C2SessFile::process( const std::string& filepath, const std::string& sessId)
{
	mFilePosEnd = 0;
	mFilePosStart = 0;
	mbValid = false;

	mRequestFilename = filepath;
	mFileFullPath = fsConcatPath( mEnv.getDocumentRootFolder() , filepath );

	memset( &mFileStat, 0 ,sizeof(mFileStat) );
	StopWatch watch;watch.start();
	checkFileAttr();
	watch.stop();

	MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(C2SessFile,"checkFileAttr() session[%s], parse file %s, time cost[%lld]us regularfile[%s] isdir[%s] posStart[%lld] posEnd[%lld] valid[%s]"),
		sessId.c_str() , mFileFullPath.c_str() , watch.span(), 
		S_ISREG(mFileStat.st_mode) ? "true":"false",
		S_ISDIR(mFileStat.st_mode) ? "true":"false",
		mFilePosStart,
		mFilePosEnd,
		mbValid ? "true" : "false" );

}

bool C2SessFile::isValid( ) const
{
	///FIXME: 
	  //return true;
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(C2SessFile,"isValid() get client type [%d]."),mEnv.getConfig().clientType);
    if ( 0 == mEnv.getConfig().clientType)
	{
		  return ( mbValid && S_ISREG( mFileStat.st_mode) && !S_ISDIR( mFileStat.st_mode) && mFilePosEnd > 0 );
    }
	else if ( 1 == mEnv.getConfig().clientType || 2 == mEnv.getConfig().clientType || 3 == mEnv.getConfig().clientType )
	{
		  //mbValid = true;
		  return true;
	}
	else 
	{
		  MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(C2SessFile,"isValid() get invalid client type [%d]."),mEnv.getConfig().clientType);
		  return false;
	}
}

int64 C2SessFile::fileDataSize() const
{
	if( mFilePosEnd < mFilePosStart )
		return 0;
	return mFilePosEnd - mFilePosStart;
}


bool C2SessFile::isRangeStand( int64 requestPos )
{
	//FIXME: not implemented yet
	return false;
}

const std::string& C2SessFile::getFileFullPath() const
{	
	return mFileFullPath;
}

bool C2SessFile::findDataStartPos(  int64 startPos , int64 reservedSize )
{	
	if( mFilePosEnd <  (20LL *  GIGABYTES) )
	{
		mbValid = true;
		mFilePosStart = 0;
		return true;
	}
	IAttrBridge* attrbridge = mEnv.getAttrBridge();
	if(!attrbridge)
	{
		mbValid = true;
		return true;//should never happen
	}

	if(attrbridge->getFileDataRange(mRequestFilename,mSessId,mFilePosStart,mFilePosEnd))
	{
		mbValid = true;
		return true;
	}

	StopWatch watch;
	watch.start();
	mFilePosStart = locateSparseTsHeader( mFileFullPath.c_str() , startPos , 32 * 1024 , reservedSize );
	if( mFilePosStart < 0 )
	{			
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(C2SessFile,"checkFileAttr() session[%s], failed to get start pos from locateSparseTsHeader for %s"),
				mSessId.c_str() , mFileFullPath.c_str() );
		mFilePosStart = 0;
		return false;
	}
	else
	{
		watch.stop();			
		MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(C2SessFile,"checkFileAttr() session[%s], it cost [%lld]us to locate the head of file content from file[%s]"),
			mSessId.c_str() , watch.span()  , mFileFullPath.c_str() );
		mbValid = true;
		return true;
	}
}

bool C2SessFile::checkFileAttr( )
{
	  MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(C2SessFile,"checkFileAttr() get client type [%d]."),mEnv.getConfig().clientType);
	  if ( 0 != mEnv.getConfig().clientType){
			mFilePosStart = 0;
			mFilePosEnd = 100000000000;
			mbValid = true;
			return true;
	  }
	int fd = ::open( mFileFullPath.c_str() , O_RDONLY );
	if( fd < 0 ) {
		char errbuf[256] = {0};
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(C2SessFile,"checkFileAttr() failed to open file [%s] error[%s]"), 
				mFileFullPath.c_str(), strerror_r(errno, errbuf, sizeof(errbuf)) );
		return false;
	}

	if (fstat( fd, &mFileStat ))
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(C2SessFile,"checkFileAttr() failed to invoke fstat for file%s],error[%s]"),
			 mFileFullPath.c_str() , SYS::getErrorMessage().c_str() );
		::close(fd);
		return false;
	}
	
	mFilePosStart	= 0;
	mFilePosEnd		= mFileStat.st_size;
		
	if( !S_ISREG( mFileStat.st_mode) || S_ISDIR( mFileStat.st_mode) )
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(C2SessFile,"checkFileAttr() file[%s] is not a regular file"),
			 mFileFullPath.c_str() );
		::close(fd);
		return false;
	}
	close(fd);
	return findDataStartPos( 0LL , 0LL );
}

int64 C2SessFile::dataStartPos( ) const
{
	return mFilePosStart;
}

int64 C2SessFile::dataEndPos( ) const
{
	return mFilePosEnd - 1;//actually mFilePosEnd is the file size
}

bool C2SessFile::checkRequestRange( const TransferRange& range )
{
//     if ( 0 != mEnv.getConfig().clientType)
// 		  return true;
	if( !mbValid )
	{
		MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(C2SessFile,"failed to get file[%s]'s attribute"), mFileFullPath.c_str() );
		return false;
	}
	if( mFilePosEnd <= mFilePosStart )
	{
		MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(C2SessFile,"invalid file attribute: data range[%lld-%lld] of file[%s]"),
			mFilePosStart , mFilePosEnd , mFileFullPath.c_str()	);
		return false;
	}
	if( range.bStartValid && range.bEndValid )
	{
		if( range.startPos > range.endPos )
		{
			return false;
		}
	}
	if( range.bStartValid )
	{
		if( range.startPos >= mFilePosEnd )
		{
			return isRangeStand( range.startPos );
		}
		else if( range.startPos < mFilePosStart )
		{
			return false;
		}
	}
	return true;
}




#ifdef ZQ_OS_MSWIN
#define		PATHDELIMETER '\\'

void		replaceCharacter( int iLen , char* p , const char* pSrc , int& iPos, bool& bLastSlash )
{
	for ( int i = 0 ;i < iLen ; i ++ )
	{
		if( pSrc[i] == '/' )
		{
			if( bLastSlash	)
			{
				continue;			
			}
			else
			{
				p[iPos++] = '\\';
			}
			bLastSlash = true;
		}
		else if ( bLastSlash && pSrc[i] == '\\'  )
		{
			continue;
		}
		else
		{
			bLastSlash = pSrc[i] == '\\';
			p[iPos++]=pSrc[i];
		}
	}
}

bool				fsCreatePath( const std::string& strPath )
{
	if( !CreateDirectoryA(fsFixupPath(strPath).c_str(),NULL) )
	{
		if ( ERROR_ALREADY_EXISTS == GetLastError() )
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	return true;
}
bool				fsCreateFile( const std::string& strFile )
{
	return false;
}

#endif

#ifdef ZQ_OS_LINUX
#define		PATHDELIMETER '/'
void		replaceCharacter( int iLen , char* p , const char* pSrc , int& iPos, bool& bLastSlash )
{
	for ( int i = 0 ;i < iLen ; i ++ )
	{
		if( pSrc[i] == '\\' )
		{
			if( bLastSlash	)
			{
				continue;			
			}
			else
			{
				p[iPos++] = '/';
			}
			bLastSlash = true;
		}
		else if ( bLastSlash && pSrc[i] == '/'  )
		{
			continue;
		}
		else
		{
			bLastSlash = pSrc[i] == '/';
			p[iPos++]=pSrc[i];
		}
	}
}
bool				fsCreatePath( const std::string& strPath )
{
	return mkdir(fsFixupPath(strPath).c_str(), 0644) == 0;
}
bool				fsCreateFile( const std::string& strFile )
{
	return false;
}
#endif

std::string			fsFixupPath( const std::string& strPath )
{
	int iLen = static_cast<int>(strPath.length());
	const char* pSrc = strPath.c_str();
	char* p = new char[iLen + 1 ];
	p[iLen] = 0;
	assert( p != NULL );
	bool bLastSlash = false;
	int iPos = 0;
	replaceCharacter(iLen,p,pSrc,iPos, bLastSlash);
	p[iPos] = 0;
	std::string result(p);
	delete[] p;
	return result;
}
std::string			fsConcatPath( const std::string& parentPath , const std::string& subPath )
{
	int iLen = static_cast<int>(parentPath.length());
	const char* pSrc = parentPath.c_str();
	char* p = new char[parentPath.length() + subPath.length() + 3 ];
	p[iLen] = 0;
	assert( p != NULL );
	bool bLastSlash = false;
	int iPos = 0;
	replaceCharacter( iLen , p , pSrc , iPos , bLastSlash );
	pSrc = subPath.c_str() ;
	iLen = static_cast<int>(subPath.length());
	if( pSrc[0] != '/' || pSrc[0] != '\\')
	{
		if( !bLastSlash )
			p[iPos++] = PATHDELIMETER;
		bLastSlash = true;
	}
	replaceCharacter( iLen, p, pSrc, iPos ,bLastSlash );
	p[iPos] = 0;
	std::string result(p);
	delete[] p;
	return result;
}
std::string			showStringVector( const std::vector<std::string>& strs )
{
	std::ostringstream oss;
	std::vector<std::string>::const_iterator it = strs.begin();
	for( ; it != strs.end() ; it ++ )
	{
		oss << *it << " ";
	}
	return oss.str();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

StopWatch::StopWatch(bool accumulative)
:mStart(0),
mRecord(0),
mbAccumulative(accumulative)
{
}

StopWatch::~StopWatch()
{
}

uint64 currentTime() {
	struct timeval v;
	gettimeofday( &v , NULL );	 
	return (uint64)v.tv_sec * 1000 * 1000 + v.tv_usec;
}

void StopWatch::start()
{
	mStart = currentTime();
}

uint64 StopWatch::stop()
{
	uint64 cur = currentTime();
	if( cur < mStart)
		return 0;
	cur -= mStart;
	if( mbAccumulative )
		mRecord += cur;
	else
		mRecord = cur;
	return cur;
}

uint64 StopWatch::span()
{
	return mRecord;
}

TransferRange composeFileRange( C2SessFile& file )
{
	TransferRange range;

	range.bStartValid = true;
	range.startPos = file.dataStartPos();	

	range.bEndValid = true;
	range.endPos = file.dataEndPos();

	return range;

}
TransferRange composeAvailRange( const TransferRange& requestRange ,C2SessFile& file )
{
	TransferRange availRange = requestRange;
	//TransferRange availRange ;
	
	if( !requestRange.bStartValid)
	{
		availRange.bStartValid	= true;
		availRange.startPos		= file.dataStartPos();	
	}
	
	if( !requestRange.bEndValid)
	{
		availRange.bEndValid	= true;
		availRange.endPos		= file.dataEndPos();
	}

	return availRange;
}

}//namespace C2Streamer

std::string errormsg( int err )
{
	char errmsg[1024] = {0};
	strerror_r( err, errmsg, 1024);
	return errmsg;
}

