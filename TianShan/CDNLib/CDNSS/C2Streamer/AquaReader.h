#ifndef __cdnss_c2streamer_aquareader_header_file__
#define __cdnss_c2streamer_aquareader_header_file__

#include "AioFile.h"
#include "../IndexFileParser.h"
#include "C2StreamerEnv.h"
#include <NativeThreadPool.h>
#include <CdmiFuseOps.h>


namespace C2Streamer {

class AquaReader;

class AquaReadRunner : public ZQ::common::ThreadRequest {
public:
	AquaReadRunner( AquaReader& reader, const std::vector<Buffer*>& bufs );
	virtual ~AquaReadRunner() {
	}
protected:
	int		run();
	void 	final( int,bool ) {
		delete this;
	}
private:
	AquaReader&				mReader;
	std::vector<Buffer*>	mBufs;
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
	AquaReader&				mReader;
	std::string				mFileName;
	AssetAttribute::Ptr		mAttr;
};
	
class AquaReader: public IDataReader,  public CdmiFuseOps{
public:
	AquaReader(C2StreamerEnv& env, IReaderCallback* cb);
	virtual ~AquaReader();

	typedef ZQ::common::Pointer<AquaReader>	Ptr;

	virtual bool read( const std::vector<Buffer*>& bufs );
	virtual bool queryIndexInfo( const std::string& filename, AssetAttribute::Ptr attr );

	void		doRead( const std::vector<Buffer*>& bufs );
	void		doQueryIndexInfo( const std::string& filename, AssetAttribute::Ptr attr );

	ZQ::common::NativeThreadPool&		threadPool() {
		return mPool;
	}
protected:
	int			doReadData( const std::string& filename, int64 startOffset, char* buf, int size);

private:
	C2StreamerEnv&						mEnv;
	ZQ::common::NativeThreadPool		mCdmiOpsThreadPool;
	ZQ::common::NativeThreadPool		mPool;
	IReaderCallback*					mReaderCB;
	ZQ::IdxParser::IdxParserEnv			mIdxEnv;
	ZQ::IdxParser::IndexFileParser		mIdxParser;
};

}


#endif//__cdnss_c2streamer_aquareader_header_file__

