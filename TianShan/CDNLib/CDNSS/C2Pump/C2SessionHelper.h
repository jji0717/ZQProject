
#ifndef _tianshan_cdnss_c2streamer_c2session_helper_header_h__
#define _tianshan_cdnss_c2streamer_c2session_helper_header_h__

#include <ZQ_common_conf.h>
#include <string>
#include <sys/stat.h>
#include "C2StreamerLib.h"

namespace C2Streamer
{

//C2SessionFile is a utility to get the file attribute
class C2StreamerEnv;

#define GIGABYTES ( 1024LL * 1024LL * 1024LL)

class C2SessFile
{
public:
	C2SessFile( C2StreamerEnv& env, int readerType );
	virtual ~C2SessFile();
	
	void process(const std::string& filepath , const std::string& sessId );

public:
	
	/// check if target file is exist and good to be read
	/// @return true if target file is good, vice versa	
	bool		isValid( ) const;
	
	bool		checkRequestRange( const TransferRange& request );	

	int64		fileDataSize() const; // this should be the real file data size in byte
	
	int64		dataStartPos( ) const;

	int64		dataEndPos( ) const;
	
	const std::string& getFileFullPath() const;	

protected:
	
	bool		isRangeStand( int64 requestPos );

	bool		checkFileAttr( );

	bool 		findDataStartPos(  int64 startPos , int64 reservedSize );
	
private:
	std::string					mFileFullPath;
	C2StreamerEnv&				mEnv;
	int64						mFilePosEnd;
	int64						mFilePosStart;	
	struct stat					mFileStat;
	bool						mbValid;
	std::string					mSessId;
	std::string					mRequestFilename;
	int 						mReaderType;
};

std::string			fsFixupPath( const std::string& strPath );

std::string			fsConcatPath( const std::string& parentPath , const std::string& subPath );

bool				fsCreatePath( const std::string& strPath );

bool				fsCreateFile( const std::string& strFile );

template<typename T> 
void				mergeVector( T& a, const T& b)
{
	typename T::const_iterator itB = b.begin();
	for( ; itB != b.end() ; itB++ )
	{
		a.push_back( *itB );
	}
}
std::string			showStringVector( const std::vector<std::string>& strs );

TransferRange composeAvailRange( const TransferRange& requestRange ,C2SessFile& file );
TransferRange composeFileRange( C2SessFile& file, int readerType = CLIENT_TYPE_DISKAIO );


class StopWatch
{
public:
	StopWatch(bool accumulative = true);
	virtual ~StopWatch();
public:
	void		start();
	uint64		stop();//count in microsecond
	uint64		span();//count in microsecond
private:
	uint64		mStart;
	uint64		mRecord;
	bool		mbAccumulative;
};

class AddrInfoHelper
{
public:
	AddrInfoHelper(int type = SOCK_STREAM)
	{
		memset(&mHints, 0, sizeof(mHints) );
		mInfo = 0;
		mHints.ai_family    = AF_UNSPEC;
		mHints.ai_socktype  = type;
		mHints.ai_protocol  = 0;
		mHints.ai_flags     = AI_CANONNAME|AI_PASSIVE;
	}

	virtual ~AddrInfoHelper()
	{
		freeResource();
	}

	struct addrinfo*   getinfo(const std::string& addr="", const std::string& port="" )
	{
		if( addr.empty() && port.empty())	return mInfo;
		freeResource();
		int rc = getaddrinfo( addr.c_str() , port.c_str(), &mHints, &mInfo);
		if( rc != 0 )
		{
			mInfo = 0 ;
			return 0;
		}
		char szIp[128]={0}, szPort[128]={0};
		getnameinfo(mInfo->ai_addr, mInfo->ai_addrlen, szIp, sizeof(szIp)-1, szPort,sizeof(szPort)-1, NI_NUMERICHOST|NI_NUMERICSERV );
		mIpString = szIp; mPortString = szPort;
		return mInfo;
	}
	const std::string&	ip() const { return mIpString; }
	const std::string& port() const {return mPortString ; } 
protected:
	void    freeResource()
	{
		if( mInfo)
		{
			freeaddrinfo( mInfo );
			mInfo = 0;
		}
	}
private:
	struct addrinfo mHints;
	struct addrinfo* mInfo;
	std::string mIpString, mPortString;
};

template<typename T>
class AtomicInteger  {
public:
	AtomicInteger()
		:mI(0){
	}
	AtomicInteger( const T& data)
	:mI(0){
		store(data);
	}

	AtomicInteger( const AtomicInteger& rhs)
	:mI(0){
		store(rhs.load());
	}

	AtomicInteger& operator=( const T& data ) {
		store(data);
		return this;
	}

	AtomicInteger& operator += ( const T& data ) {
		store(load()+data);
		return *this;
	}

	void store(const T& data) {
		T old = mI;
		while( !__sync_bool_compare_and_swap(&mI, old, data) ) {
			old = mI;
		}
	}

	T load() {
		return __sync_add_and_fetch(&mI, 0);
	}

private:
	T	mI;
};

}//namespace C2Streamer

std::string errormsg( int err );

#endif//_tianshan_cdnss_c2streamer_c2session_helper_header_h__
