#ifndef __cdnss_c2streamer_aquareader_header_file__
#define __cdnss_c2streamer_aquareader_header_file__

#include "AioFile.h"
#include "../IndexFileParser.h"
#include "C2StreamerEnv.h"
#include <AquaUpdate.h>
#include <NativeThreadPool.h>
#include <CdmiFuseOps.h>


namespace C2Streamer {

class AquaReader;

class AquaReadRunner : public ZQ::common::ThreadRequest {
public:
	AquaReadRunner( AquaReader& reader, const std::vector<BufferUser>& bufs );
	virtual ~AquaReadRunner() {
	}
protected:
	int		run();
	void 	final( int,bool ) {
		delete this;
	}
private:
	C2StreamerEnv&			mEnv;
	AquaReader&				mReader;
	std::vector<BufferUser>	mBufs;
};

class AquaQueryIndexRunner : public ZQ::common::ThreadRequest {
public:
	AquaQueryIndexRunner( AquaReader& reader, const std::string& filename, AssetAttribute::Ptr attr );
	virtual ~AquaQueryIndexRunner() {
	}
protected:
	int		run();

	void 	final( int,bool ) {
		delete this;
	}
private:
	C2StreamerEnv&			mEnv;
	AquaReader&				mReader;
	std::string				mFileName;
	AssetAttribute::Ptr		mAttr;
};
	
class AquaReader: public IDataReader,  public CdmiFuseOps{
public:
	AquaReader(C2StreamerEnv& env, IReaderCallback* cb, const FuseOpsConf&);
	virtual ~AquaReader();

	typedef ZQ::common::Pointer<AquaReader>	Ptr;

	virtual bool read( const std::vector<BufferUser>& bufs );
	virtual bool queryIndexInfo( const std::string& filename, AssetAttribute::Ptr attr );
	virtual bool getFileSize( const std::string& filename, int64& fileSize );

	void		doRead( const std::vector<BufferUser>& bufs );
	void		doQueryIndexInfo( const std::string& filename, AssetAttribute::Ptr attr );

	ZQ::common::NativeThreadPool&		threadPool() {
		return mPool;
	}

	C2StreamerEnv&		getEnv() const {
		return mEnv;
	}
protected:
	int			doReadData( const std::string& filename, int64 startOffset, char* buf, int size);

	void		checkPendingRequest( );


private:
	C2StreamerEnv&						mEnv;
	ZQ::common::NativeThreadPool		mCdmiOpsThreadPool;
	ZQ::common::NativeThreadPool		mPool;
	IReaderCallback*					mReaderCB;
	ZQ::IdxParser::IdxParserEnv			mIdxEnv;
	ZQ::IdxParser::IndexFileParser		mIdxParser;

	CRM::A3Message::A3AquaBase::Ptr			mA3AquaBase;
};

}


#endif//__cdnss_c2streamer_aquareader_header_file__

