
#include "AquaReader.h"
#include <AquaUpdate.h>
#include <IndexFileParser.h>
#include "C2SessionHelper.h"

namespace C2Streamer {

AquaReadRunner::AquaReadRunner( AquaReader& reader, const std::vector<Buffer*>& bufs ):
ZQ::common::ThreadRequest(reader.threadPool()),
mReader(reader),
mBufs(bufs)
{
}

int AquaReadRunner::run( ) {
	mReader.doRead(mBufs);
	return 0;
}

AquaQueryIndexRunner::AquaQueryIndexRunner( AquaReader& reader, const std::string& filename, AssetAttribute::Ptr attr )
:ZQ::common::ThreadRequest(reader.threadPool()),
mReader(reader),
mFileName(filename),
mAttr(attr) 
{
}

int AquaQueryIndexRunner::run() {
	mReader.doQueryIndexInfo( mFileName, mAttr );
	return 0;
}

AquaReader::AquaReader( C2StreamerEnv& env, IReaderCallback* cb )
:CdmiFuseOps(*env.getLogger(), mCdmiOpsThreadPool, 
		env.getConfig().aquaReaderRootUrl,
		env.getConfig().aquaReaderUserDomain,
		env.getConfig().aquaReaderHomeContainer,
		env.getConfig().aquaReaderFlags ),
mEnv(env),
mReaderCB(cb),
mIdxParser(mIdxEnv){
	mIdxEnv.AttchLogger(mEnv.getLogger());
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(AquaReader,"init with: rootUrl:%s userDomain:%s homeContainer:%s flags[%d]"),
		env.getConfig().aquaReaderRootUrl.c_str(),
		env.getConfig().aquaReaderUserDomain.c_str(),
		env.getConfig().aquaReaderHomeContainer.c_str(),
		env.getConfig().aquaReaderFlags	);
}

AquaReader::~AquaReader( ) {
	mCdmiOpsThreadPool.stop();
	mPool.stop();
}

bool AquaReader::read( const std::vector<Buffer*>& bufs ) {
	( new AquaReadRunner( *this, bufs) )->start();
	return true;
}

bool AquaReader::queryIndexInfo( const std::string& filename, AssetAttribute::Ptr attr ) {
	(new AquaQueryIndexRunner(*this, filename, attr))->start();
	return true;
}

void splitFileName( const std::string& filename, std::string& pid, std::string& paid, std::string& ext ) {
	if( filename.length() < 20 )
		return;
	paid = filename.substr(0,20);
	std::string::size_type posDot = filename.find_last_of('.');
	std::string::size_type posPid = 20;
	if( filename.at(20) == '_') {
		posPid = 21;
	}
	pid = filename.substr(posPid, posDot - posPid );
	if( posDot != std::string::npos )
		ext = filename.substr(posDot+1);
}

std::string fixupFilename( const std::string& filename ) {
	std::string::size_type pos1 = filename.find_last_of('/');
	std::string::size_type pos2 = filename.find_last_of('\\');

	std::string::size_type pos = std::string::npos;
	if( pos1 == std::string::npos ) {
		pos = pos2;
	} else if ( pos2 == std::string::npos ) {
		pos = pos1;
	} else {
		pos = pos1 > pos2 ? pos1 : pos2;
	}
	if( pos == std::string::npos ) {
		return filename;
	}
	return filename.substr(pos+1);
}

int AquaReader::doReadData( const std::string& filename, int64 startOffset, char* buf, int size ) {
	std::string url = pathToUri(filename);
	fixupPathname(url);
	std::string contentType, location;
	uint32 dataSize = size;
	CdmiRetCode rc = nonCdmi_ReadDataObject( url, contentType, location, startOffset, dataSize, buf  ); 
	if( CdmiRet_FAIL(rc) ) {
		return -1;
	} else {
		return dataSize;
	}
}

void AquaReader::doRead( const std::vector<Buffer*>& bufs ) {
	if( bufs.empty())
		return;
	StopWatch ts; ts.start();
	std::string filename = fixupFilename( bufs[0]->filename());
	int64 startOffset = bufs[0]->offsetInFile();
	std::vector<Buffer*>::const_iterator it = bufs.begin();
	int okCount = 0;
	int lastError = 0;
	for( ; it != bufs.end(); it ++ ) {
		Buffer* buf = *it;
		int rc = doReadData( filename, buf->offsetInFile(), buf->buffer(), buf->bufSize() );
		if( rc < 0 ) {
			lastError = rc;
			break;
		}
		buf->setDataSize(rc);
		okCount ++;
	}
	for( ; it != bufs.end(); it ++ ) {
		Buffer* buf = *it;
		buf->setLastError(lastError, Buffer::ECATE_SOCKET);
	}
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(AquaReader,"doRead() read for[%s] startAt[%ld], ok[%d] fail[%d]. time cost [%ld]ms"),
		filename.c_str(), startOffset, okCount, (int)bufs.size()-okCount, ts.stop() );
	mReaderCB->onRead(bufs);
}

void AquaReader::doQueryIndexInfo( const std::string& filename, AssetAttribute::Ptr attr ) {
	StopWatch ts; ts.start();
	using namespace CRM::A3Message;
	std::string pid, paid, ext;
	std::string indexFileName = fixupFilename( filename );
	splitFileName( indexFileName, pid, paid, ext );
	A3AquaBase::Ptr assetInfo = new A3AquaBase( *mEnv.getLogger(), *this);
	TianShanIce::Properties props;
	if(!assetInfo->getMetadataInfo(paid, pid,  props ) ) {
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(AquaReader,"doQueryIndexInfo() failed to getMetadataInfo for %s"),indexFileName.c_str());
		attr->lastError( AssetAttribute::ASSET_DATAERR );
		mReaderCB->onIndexInfo(attr);
		return;
	}
	std::string assetState = props["sys.State"];
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(AquaReader,"doQueryIndexInfo() got sys.State for[%s] : %s"), indexFileName.c_str(), assetState.c_str());
	bool bPwe = assetState != "InService";
	attr->pwe( bPwe );
	char buffer[4*1024];
	int rc = doReadData( indexFileName, 0, buffer, sizeof(buffer));
	if( rc < 0 ) {
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(AquaReader,"failed to read data for[%s], error[%d]"),
				indexFileName.c_str(), rc);
		attr->lastError(AssetAttribute::ASSET_DATAERR);
		mReaderCB->onIndexInfo( attr );
		return;
	}
	ZQ::IdxParser::IndexData idxData;
	if( !mIdxParser.ParseIndexFromMemory( indexFileName, idxData, buffer, rc ) ) {
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(AquaReader,"doQueryIndexInfo() failed to parse index data from[%s] size[%d]"),
			indexFileName.c_str(), rc);
		attr->lastError( AssetAttribute::ASSET_DATAERR );
		mReaderCB->onIndexInfo(attr);
		return;
	}
	attr->assetBaseInfo( idxData.baseInfoToXML() );
	attr->assetMemberInfo( idxData.memberFileToXML() );
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(AquaReader,"doQueryIndexInfo() got indexinfo for[%s]  time cose[%ld]ms"), indexFileName.c_str(), ts.stop());
	mReaderCB->onIndexInfo( attr );
}

}//namespace C2Streamer



