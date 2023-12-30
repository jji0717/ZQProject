#include <ZQ_common_conf.h>
#include <sys/types.h>
#include "C2StreamerEnv.h"
#include "C2StreamerService.h"
#include "C2SessionHelper.h"
#include "C2SessionUdpQuery.h"

#include <libasync/http.h>

#if defined ZQ_OS_MSWIN
    #define REQFMT(x,y)     CLOGFMT(x, " REQUEST[%s]\t"##y), request->requestHint.c_str()
#elif defined ZQ_OS_LINUX
    #define REQFMT(x,y)     CLOGFMT(x, " REQUEST[%s]\t"y), request->requestHint.c_str()
#endif

namespace C2Streamer{

class IndexDataReader;

class IndexDataGotten : public IAsyncNotifySinker {
	public:
		typedef ZQ::common::Pointer<IndexDataGotten> Ptr;
		IndexDataGotten( ZQ::common::Semaphore& sem, IdxReaderPtr idxReader )
		:mSema(sem),mIdxReader(idxReader) {
			assert(mIdxReader != NULL);
		}
		virtual ~IndexDataGotten() {}
	protected:
		virtual void onNotified() {
		//	MLOG.INFO(CLOGFMT(IndexDataGotten,"onNotified()"));
			mIdxReader->onDataReceived();
		}
	private:
		ZQ::common::Semaphore&      mSema;
		IdxReaderPtr                mIdxReader;
};

IndexDataReader::IndexDataReader( C2StreamerEnv& env, C2Service& svc )
:LibAsync::AsyncWork( LibAsync::getLoopCenter().getLoop()),
mEnv(env),
mSvc(svc),
mIndexData(NULL),
mbParsed(false),
mbHeadFinish(false),
mbFirstTry(true),
mOffset(0),
mDuration(0),
mScale(0),
mLastErrorCode(errorCodeOK),
mAioFile(NULL)
{
  mIdxParserEnv.AttchLogger(env.getLogger());
  mLoop = &getLoop();
  assert(mLoop != NULL);
}

IndexDataReader::~IndexDataReader()
{
  if(mIndexData){
      free(mIndexData);
      mIndexData = NULL;
  }
  if(mLoop) {
	  mLoop->decreateSockCount();
  }
}

bool IndexDataReader::findSubfileInfo( INOUT float& scale, OUT std::string& subFileExt, int64& playTime) {
  uint32 duration = 0;
  int idx = -1;
  MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(IndexDataReader,"before findSubfileInfo scale[%f]"),scale);
  subFileExt = mIdxRecPtr->getSubfileExt(scale, duration,idx);
  mDuration = playTime = duration;
  mScale = scale;
  MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(IndexDataReader,"after findSubfileInfo idx[%d] duration[%ld]"),idx, mDuration);
  return true;
}

int64 IndexDataReader::findDataOffset( INOUT int64& timeOffset, float scale ) {
	uint64 tmpTimeOffset = timeOffset;
	int idx = 0;
    MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(IndexDataReader,"before findDataOffset timeOffset[%ld] scale[%f] idx[%d]"),timeOffset,scale,idx);
	int64 dataOffset = (int64)mIdxRecPtr->findNextIFrameByTimeOffset( tmpTimeOffset, idx, scale );
    MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(IndexDataReader,"after findDataOffset dataOffset[%ld] timeOffset[%ld] idx[%d]"),dataOffset,tmpTimeOffset,idx);
	timeOffset = (int64)tmpTimeOffset;
	return dataOffset;
}

int64 IndexDataReader::getDataBitrate() const
{
	return mIdxRecPtr->getdatabitrate();
}

void  IndexDataReader::rewriteRecords(const IdxRecPtr& ptr)
{
	mIdxRecPtr->rewriteRecords(*ptr);
}

int32 IndexDataReader::parse( const std::string& assetName, int readertype, const std::string& sessionId )
{
	mSessionId = sessionId;
	bool bNew = true;

	if(!mIdxRecPtr) {
		mIdxRecPtr = mSvc.getCacheCenter().getIdxRecByIdxName(assetName, bNew);
	} else {
		bNew = false;
	}

	if (mIdxRecPtr->IsParsed()){
		if(bNew) {
			mIdxRecPtr->signal();
		}
		return mLastErrorCode;
	}

	if (!bNew){
		mIdxRecPtr->wait(); //wait for completion of index parsing
		return mLastErrorCode;
	}
	std::string idxfilename = assetName + ".index";

	mAioFile = mSvc.getCacheCenter().open(idxfilename, readertype, mSessionId);
	if (!mAioFile){
		mIdxRecPtr->signal();
		mLastErrorCode = errorCodeBadRequest;
		return mLastErrorCode;
	}
	mAioFile->setBitrate(4000 * 1000); //4mbps
	mAssetName = assetName;
	parseIndex( );
	mIdxRecPtr->wait();
	MLOG.debug(CLOGFMT(IndexDataReader, "finish index parse lasterror[%d] idxfile[%s] sessid[%s]"),mLastErrorCode, assetName.c_str(),mSessionId.c_str());
	return mLastErrorCode;
}

static int64 max_index_size = 8 * 1024 * 1024; 

void IndexDataReader::onAsyncWork() {
	/*
	if(mbFirstTry) {
		if(mIndexData) {
			free(mIndexData);
			mIndexData = NULL;
		}
		mIndexData = (char*)malloc( max_index_size);
		mOffsetInBuffer = 8 * 1024;// 8k for the first read
		mbFirstTry = false;
	}
	if(!mIndexData) {
		MLOG.error(CLOGFMT(IndexDataReader,"parseIndex not enough memory"));
		mLastErrorCode = errorCodeInternalError;
		return;
	}
	*/

	//the first we trying to parse index file data
	MLOG.debug(CLOGFMT(onAsyncWork, "AsyncWork begin"));
	mOffset = 0;
	mBu = mAioFile->read(mOffset);
	if(!mBu.valid()) {
		mLastErrorCode = errorCodeInternalError;
		mIdxRecPtr->signal();
		mAioFile->close();
		return;

	}
	IndexDataGotten::Ptr cb = new IndexDataGotten(mSema,this);
	if(mBu.asyncWait(cb, 8*1024)) {
		MLOG.debug(CLOGFMT(onAsyncWork, "readNext() waiting data ..."));
	} else {
		MLOG.debug(CLOGFMT(onAsyncWork, "onDataReceived() begin"));
		onDataReceived(); //parsing data immediately if index file data is available
	}
}

void IndexDataReader::onDataReceived() {
	while(true) {
		///version 2.0
		int64 dataAvail = mBu.dataLeft();
		if ( mBu.lastError() != 0 ||  dataAvail == 0 ) {
			mIdxRecPtr->sortRecordTrcik();
			MLOG.info(CLOGFMT(IndexDataReader,"maybe reach the end of asset[%s]'s index or encounter error[%d] dataAvail[%ld] sessid[%s] parsedrecords[%d]"),
					mAssetName.c_str(), mBu.lastError(), dataAvail,mSessionId.c_str(),mIdxRecPtr->getParsedRecords());
			if (mBu.lastError() != 0 )
				mLastErrorCode = errorCodeEOF; 
			break;
		}
		ZQ::IdxParser::IndexFileParser parser(mIdxParserEnv);
		MLOG.debug(CLOGFMT(IndexDataReader,"begin to parse index, mOffset[%ld] avail[%ld] idx[%s] sessid[%s] mBu[%p]"), mOffset, dataAvail,mAssetName.c_str(),mSessionId.c_str(),mBu.getBuffer());
		int32 errorCode = errorCodeOK;
		if( ( errorCode = parser.ParseIndexRecodFromMemoryPWE(mAssetName, *mIdxRecPtr, mBu.getBuffer(), dataAvail, mOffset%mBu.bufferSize())) != errorCodeOK) {
			if (errorCodeOK != mIdxRecPtr->getHeaderErrCode()) {
				mLastErrorCode = errorCode;
				MLOG.error(CLOGFMT(IndexDataReader,"failed to parse index header idx[%s] sessid[%s], parsedrecords[%d]"),mAssetName.c_str(),mSessionId.c_str(),mIdxRecPtr->getParsedRecords());
				break;
			}
			if (errorCodeNotFull == errorCode) {
				//rewriteRecords(mIdxRecPtr);
				//mLastErrorCode = errorCode;
				MLOG.debug(CLOGFMT(IndexDataReader,"idx[%s] data not full, wait another data sessid[%s],parsedrecords[%d]"),mAssetName.c_str(),mSessionId.c_str(),mIdxRecPtr->getParsedRecords());
			} else {
				mLastErrorCode = errorCode;
				MLOG.error(CLOGFMT(IndexDataReader,"sth wrong with parse idx[%s] errcode [%d] sessid[%s],parsedrecords[%d]"),mAssetName.c_str(),errorCode, mSessionId.c_str(),mIdxRecPtr->getParsedRecords());
				break;
			}
		}
		mIdxRecPtr->signal();


		mBu.advance(dataAvail);
		mOffset += dataAvail;

		//read next block of data for this index file
		mBu = mAioFile->read(mOffset);
		if(!mBu.valid()) {
			mLastErrorCode = errorCodeInternalError;
			break;
		}
		IndexDataGotten::Ptr cb = new IndexDataGotten(mSema,this);
		if(mBu.asyncWait(cb, 0x7E)) {
			MLOG.debug(CLOGFMT(IndexDataReader, "readNext() waiting data ..."));
			return; //waiting for next block of data
		}
	}
	if(mAioFile) {
		mAioFile->close();
		mAioFile = NULL;
	}
	mIdxRecPtr->signal();
}

int32 IndexDataReader::parseIndex(){
	mLastErrorCode = errorCodeOK;
	postAsyncWork();
	return errorWorkingInProcess;
}


C2UdpInfoQuery::C2UdpInfoQuery(C2StreamerEnv& env, C2Service& svc)
:mEnv(env),
mSvc(svc)
{}

C2UdpInfoQuery::~C2UdpInfoQuery()
{
}

IdxReaderPtr C2UdpInfoQuery::findReaderByLRUMap(const std::string& assetname, bool& bNew)
{
	IdxReaderPtr pIdxReader = NULL;
	ZQ::common::MutexGuard gd(mLocker);
	pIdxReader = IdxReaderMap[assetname];
	if (!pIdxReader){
		bNew = true;
	    pIdxReader = new IndexDataReader(mEnv,mSvc);
	    assert(pIdxReader);
		pIdxReader->SetUnParsed();
	    IdxReaderMap[assetname] = pIdxReader;
	}else{
		bNew = false;
		return pIdxReader;
	}
	return pIdxReader;
}

IdxReaderPtr  C2UdpInfoQuery::getIdxReaderByAssetName(const std::string& assetname, bool& bNew)
{
	IdxReaderPtr readerPtr = findReaderByLRUMap(assetname,bNew);
	assert(readerPtr != NULL);
	if (!bNew){
		readerPtr->wait();
	}
	return readerPtr;
}

int32   C2UdpInfoQuery::UdpInfoQuery(const SessionUdpControlRequestParamPtr request, SessionUdpControlResponseParamPtr response)
{
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(UdpInfoQuery,"C2Service::UdpInfoQuery"));
	if (request->subSessionId == 0){
		IdxReaderPtr reader = NULL;
		bool bNew = true;
	    int readerType = request->getConfUrlRule()->readerType;
	    int32 result = errorCodeOK;
	    response->filename = request->assetName;
	    reader = getIdxReaderByAssetName(request->assetName,bNew);
		assert(reader != NULL);
		if (bNew){
			result = reader->parse(response->filename, readerType, request->sessionId);
			reader->signal();
		}else{
	          ///
		}
		if (result != errorCodeOK){
			return result;
		}
		std::string ext;
		response->scale = 1.0;
	    reader->findSubfileInfo(response->scale,ext,response->duration);
	    MLOG.debug(CLOGFMT(udpInfoQuery, "get info for unloaded item: filename[%s] scale[%f] duration[%ld]"),response->filename.c_str(),response->scale, response->duration);
	    return errorCodeOK;
	}
	return 0;
}

IdxReaderPtr C2UdpInfoQuery::parse( const std::string& assetName, int32 readerType, const std::string& sessionId) {
	bool bNew = true;
	int32 errcode = errorCodeOK;
	IdxReaderPtr reader = getIdxReaderByAssetName(assetName, bNew);
	assert(reader != NULL);
	if(bNew) {
		errcode = reader->parse(assetName, readerType, sessionId);
		reader->signal();
	}
	if (errcode == errorCodeOK || errcode == errorCodeNotFull )
		return reader;
	return NULL;
}

}//namespace C2Streamer
