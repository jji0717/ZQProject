
#include "AquaReader.h"
#include <IndexFileParser.h>
#include "C2SessionHelper.h"

namespace C2Streamer {

	AquaReadRunner::AquaReadRunner( AquaReader& reader, const std::vector<BufferUser>& bufs )
	:
	ZQ::common::ThreadRequest(reader.threadPool()),
	mEnv(reader.getEnv()),
	mReader(reader),
	mBufs(bufs)
	{
	}

	int AquaReadRunner::run( ) {
		if(mEnv.getConfig().aquaReader.aquaReaderHlsEnableMDataQuery >= 1) {
			const std::string& filename = mBufs[0]->filename();
			int64 fileSize = 0;
			if(mReader.getFileSize( filename, fileSize)) {
				std::vector<BufferUser>::iterator it = mBufs.begin();
				for( ; it != mBufs.end() ; it ++ ) {
					(*it)->fileSize( fileSize );
				}
			}
		}
		mReader.doRead(mBufs);
		return 0;
	}

	AquaQueryIndexRunner::AquaQueryIndexRunner( AquaReader& reader, const std::string& filename, AssetAttribute::Ptr attr )
	:ZQ::common::ThreadRequest(reader.threadPool()),
	mEnv(reader.getEnv()),
	mReader(reader),
	mFileName(filename),
	mAttr(attr) 
	{
	}

	int AquaQueryIndexRunner::run() {
		mReader.doQueryIndexInfo( mFileName, mAttr );
		return 0;
	}

	AquaReader::AquaReader( C2StreamerEnv& env, IReaderCallback* cb, const FuseOpsConf& opsConf)
		:CdmiFuseOps(*env.getLogger(), mCdmiOpsThreadPool, 
					env.getConfig().aquaReader.aquaReaderRootUrl,
					env.getConfig().aquaReader.aquaReaderUserDomain,
					env.getConfig().aquaReader.aquaReaderHomeContainer,
					(uint32)env.getConfig().aquaReader.aquaReaderFlags,
					opsConf),
		mEnv(env),
		mReaderCB(cb),
		mIdxParser(mIdxEnv)
	{
		mIdxEnv.AttchLogger(mEnv.getLogger());
		mCdmiOpsThreadPool.resize( env.getConfig().aquaReader.aquaReaderCdmiOpsReadThreadPoolSize );
		mPool.resize( (int)env.getConfig().aquaReader.aquaReaderIoThreadPoolSize );

		static std::string contentNameFormat = mEnv.getConfig().aquaReader.aquaReaderContentNameFormat;
		static std::string mainFileExtension = mEnv.getConfig().aquaReader.aquaReaderMainFileExtension;

		mA3AquaBase = new CRM::A3Message::A3AquaContentStatus(*mEnv.getLogger(), *this, 30, 30, 500, mainFileExtension, contentNameFormat);
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(AquaReader,"init with: rootUrl:%s userDomain:%s homeContainer:%s flags[%d], threadPool[%d/%d]"),
				env.getConfig().aquaReader.aquaReaderRootUrl.c_str(),
				env.getConfig().aquaReader.aquaReaderUserDomain.c_str(),
				env.getConfig().aquaReader.aquaReaderHomeContainer.c_str(),
				(int)env.getConfig().aquaReader.aquaReaderFlags,
				(int)env.getConfig().aquaReader.aquaReaderIoThreadPoolSize,
				(int)env.getConfig().aquaReader.aquaReaderCdmiOpsReadThreadPoolSize);
	}

	AquaReader::~AquaReader( ) {
		mCdmiOpsThreadPool.stop();
		mPool.stop();
	}

	bool AquaReader::read( const std::vector<BufferUser>& bufs ) {
		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(AquaReader,"read() entry with bufs[%d]"), bufs.size());
		checkPendingRequest();
		for( int i = 0; i < bufs.size(); i++ )
		{
			std::vector<BufferUser> buffers;
			buffers.push_back(bufs[i]);
			( new AquaReadRunner( *this, buffers) )->start();
		}
		return true;
	}

	bool AquaReader::queryIndexInfo( const std::string& filename, AssetAttribute::Ptr attr ) {
		checkPendingRequest();
		(new AquaQueryIndexRunner(*this, filename, attr))->start();
		return true;
	}

	void AquaReader::checkPendingRequest( ) {
		int pending = mCdmiOpsThreadPool.pendingRequestSize();
		int active = mCdmiOpsThreadPool.activeCount();
		static int size = mCdmiOpsThreadPool.size();
		if( pending >= size ) {
			MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(AquaReader,"threadpool[%d/%d], pending[%d]"),active, size, pending );
		}
	}

	void splitFileName(const std::string& pathname, std::string& pid, std::string& paid, std::string& ext)
	{
		std::string filename = pathname;
		size_t pos = filename.find_last_of('.');
		ext = "";
		if (std::string::npos != pos)
		{
			ext = filename.substr(pos + 1);
			if (0 != ext.compare("index") && std::string::npos == ext.find_first_of("XFRVC0123456789ABCDEF")) // make sure if it is the reserved extname
				ext = "";
			else filename = filename.substr(0, pos);
		}

		paid = filename;
		pid = "";
		pos = filename.find('_');
		if (std::string::npos != pos)
		{
			paid = filename.substr(0, pos);
			pid = filename.substr(pos + 1);
		}
		else if (filename.length() >20)
		{
			paid = filename.substr(0, 20);
			pid = filename.substr(20);
		}
	}

	std::string fixupFilename( const std::string& filename ) {
		return filename;
		/*
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
		*/
	}

	int AquaReader::doReadData( const std::string& filename, int64 startOffset, char* buf, int size ) {
		std::string url = pathToUri(filename);
		fixupPathname(url);
		std::string contentType, location;
		uint32 dataSize = size;
		StopWatch ts; ts.start();
		CdmiRetCode rc = nonCdmi_ReadDataObject( url, contentType, location, startOffset, dataSize, buf  ); 
		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(AquaReader,"doReadData() read data took %ldus"), ts.stop());
		if( CdmiRet_FAIL(rc) )
			return -rc;

		return dataSize;
	}

	void AquaReader::doRead( const std::vector<BufferUser>& bufs ) {
		if( bufs.empty())
			return;
		StopWatch ts; ts.start();
		std::string filename = fixupFilename( bufs[0]->filename());
		int64 startOffset = bufs[0]->offsetInFile();
		std::vector<BufferUser>::const_iterator it = bufs.begin();
		int okCount = 0;
		int lastError = 0;
		for( ; it != bufs.end(); it ++ ) {
			BufferUser buf = *it;
			StopWatch tss; tss.start();
			int rc = doReadData( filename, buf->offsetInFile(), buf->buffer(), buf->bufSize() );
			MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(AquaReader,"doRead() read buf[%ld] us[%ld]us"), buf->reqId(), ts.stop());
			if( rc < 0 ) {
				lastError = rc;
				break;
			}
			buf->setDataSize(rc);
			okCount ++;
		}
		for( ; it != bufs.end(); it ++ ) {
			BufferUser buf = *it;
			buf->setLastError(lastError, Buffer::ECATE_SOCKET);
		}
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(AquaReader,"doRead() read for[%s] startAt[%ld], ok[%d] fail[%d]. time cost [%ld]us"),
				filename.c_str(), startOffset, okCount, (int)bufs.size()-okCount, ts.stop() );
		mReaderCB->onRead(bufs);
	}

	void AquaReader::doQueryIndexInfo( const std::string& filename, AssetAttribute::Ptr attr ) {
		StopWatch ts; ts.start();
		std::string pid, paid, ext;
		
		
		std::string indexFileName = fixupFilename( filename );
		splitFileName( indexFileName, pid, paid, ext );

		/*
		// TEST COED, TO BE REMOVED
		{
			attr->lastError(AssetAttribute::ASSET_NOTFOUND);
			mReaderCB->onIndexInfo(attr);
			return;
		}
		*/

		TianShanIce::Properties props;
		if(! mA3AquaBase->getMetadataInfo(paid, pid,  props) ) {
			bool isNotFound = (props["sys.State"] == "NotFound");
			MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(AquaReader,"doQueryIndexInfo() failed to getMetadataInfo for [%s] req[%ld] %s"),
					indexFileName.c_str(),attr->reqId(), isNotFound ? "due to not found" : "");
			attr->lastError( isNotFound ? AssetAttribute::ASSET_NOTFOUND : AssetAttribute::ASSET_DATAERR );
			mReaderCB->onIndexInfo(attr);
			return;
		}
		std::string assetState = props["sys.State"];
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(AquaReader,"doQueryIndexInfo() got sys.State for[%s] req[%ld] : %s"), indexFileName.c_str(), attr->reqId(), assetState.c_str());
		bool bPwe = !assetState.empty() && assetState != "InService";
		attr->pwe( bPwe );
		char buffer[4*1024];
		int rc = doReadData( indexFileName, 0, buffer, sizeof(buffer));
		if( rc < 0 ) {
			MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(AquaReader,"failed to read data for[%s] req[%ld], error[%d]"),
					indexFileName.c_str(),attr->reqId(), rc);
			attr->lastError( rc == -404 ? AssetAttribute::ASSET_NOTFOUND : AssetAttribute::ASSET_DATAERR);
			mReaderCB->onIndexInfo( attr );
			return;
		}
		ZQ::IdxParser::IndexData idxData;
		if( !mIdxParser.ParseIndexFromMemory( indexFileName, idxData, buffer, rc ) ) {
			MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(AquaReader,"doQueryIndexInfo() failed to parse index data for[%s] req[%ld] size[%d]"),
					indexFileName.c_str(),attr->reqId(), rc);
			attr->lastError( AssetAttribute::ASSET_DATAERR );
			mReaderCB->onIndexInfo(attr);
			return;
		}

		attr->assetBaseInfo( idxData.baseInfoToXML() );
		attr->assetMemberInfo( idxData.memberFileToXML() );
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(AquaReader,"doQueryIndexInfo() got indexinfo for[%s] req[%ld] time cost[%ld]ms"), 
				indexFileName.c_str(), attr->reqId(), ts.stop());
		mReaderCB->onIndexInfo( attr );
	}

	bool AquaReader::getFileSize( const std::string& name, int64& fileSize )
	{
		fileSize = 0;
		//std::string filename = pathToUri(fixupFilename( name ) );
		FileInfo fi;
		StopWatch sw;sw.start();
		CdmiRetCode retCode = getFileInfo( name, fi, false);
		if(!CdmiRet_SUCC(retCode))
		{
			MLOG.warning( CLOGFMT(AquaReader,"getFileSize() failed to get fileSize for file[%s]"), name.c_str());
			return false;
		}

		fileSize = fi.filestat.st_size;
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(AquaReader,"getFileSize() got fileSize[%lld] for[%s], time cost[%ld]us"), 
				 fileSize, name.c_str(), sw.stop());

		return true;
	}

}//namespace C2Streamer



