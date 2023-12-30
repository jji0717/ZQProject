#include "urlstr.h"
#include "ReadHttpManger.h"
#include "C2SessionHelper.h"

namespace C2Streamer{

	uint64 currentTimeC2() {
		struct timeval v;

		gettimeofday( &v , NULL );
		return (uint64)v.tv_sec * 1000 * 1000 + v.tv_usec;
	}

	//////class ReadHttpManger//////
	ReadHttpManger::ReadHttpManger(IReaderCallback* callBack,ZQ::StreamService::ReadClient::Ptr c2client,ZQ::common::Log& log,const std::string& proxyUrl, const std::string& segmenterUrl, const int mapsize, const int savecount, const int timeout,int retrytime, bool replacec2):
		_callBack(callBack),
		_log(log),
		_proxyUrl(proxyUrl),
		_segmenterUrl(segmenterUrl),
		_clientMap(FileClientMap(mapsize)),
		_saveCount(savecount),
		_timeOut(timeout),
		_retryTimes(retrytime),
		_replaceC2(replacec2),
		_c2client(c2client)
	{
		_params =_c2client->getParams();
	}

	ReadHttpManger::~ReadHttpManger()
	{

	}

	bool  ReadHttpManger::newArequest(const std::string filename,BufferUser buf)
	{
		ZQ::common::MutexGuard gd(_clientMapLock);
		{
			FileClientMap::iterator it = _clientMap.find(filename);
			if(it == _clientMap.end())//not found
			{
				_log(ZQ::common::Log::L_DEBUG, CLOGFMT(ReadHttpManger, "newArequest() will new locate Client with file:[%s] bufReqId[%ld]"), filename.c_str(),buf->reqId());
				LocateClient::Ptr locateclient = new LocateClient(filename,_params,_log,this,_proxyUrl,_segmenterUrl,buf);
				return locateclient->startRequest();
			}
			else
			{
				uint64 curtime = currentTimeC2();
				_log(ZQ::common::Log::L_DEBUG, CLOGFMT(ReadHttpManger, "newArequest() bufReqId[%ld] file:[%s] is EXIST,savecount[%d] read count[%d] locatPath[%s] [%lu:%lu:%lu:%d]"),buf->reqId(),filename.c_str(),_saveCount,it->second->_count,it->second->_locatePath.c_str(),it->second->_creatTime,curtime,curtime - it->second->_creatTime,_timeOut*1000);
				if(((curtime - it->second->_creatTime) > _timeOut * 1000) ||
						(it->second->_count > _saveCount))//add config
				{
					_log(ZQ::common::Log::L_INFO, CLOGFMT(ReadHttpManger, "newArequest() timeout [%lu] will delete file:[%s] from clientMap bufReqId[%ld]"), (curtime - it->second->_creatTime)/1000, filename.c_str()),buf->reqId();
					delClient(it->first,buf->reqId());//delete the httpfetcher
					//try get again
					LocateClient::Ptr locateclient = new LocateClient(filename,_params,_log,this,_proxyUrl,_segmenterUrl,buf);
					return locateclient->startRequest();
				}
				else // increase the count
				{
					it->second->_count++;
					//replace IpPort
					ZQ::common::URLStr srcUrl(_segmenterUrl.c_str());
					std::string strServer = srcUrl.getHost();
					std::string strOrgFile = srcUrl.getPath();
					int nPort = srcUrl.getPort();
					srcUrl.setHost(it->second->_locatePath.c_str());
					srcUrl.setPort(it->second->_Port);
					std::string newUpSessUrl = std::string(srcUrl.generate());

					std::string  segUrl  = newUpSessUrl + "/" + it->second->_sessionId;// filename; 
					int64 origBitrate = buf->getBitrate();
					int64 reqBitrate = origBitrate * _params.bitrateInflate / 100;
					if(_params.minTransferRate > 0 && reqBitrate < _params.minTransferRate) {
						reqBitrate = _params.minTransferRate;
					}
					std::ostringstream oss;
					oss<<segUrl<<"-"<<buf->offsetInFile()<<"L"<<buf->bufSize()
						<<"?ic="<<1000L*1000*1000*1000<<"&rate="<<reqBitrate<<"&sid=true";
					std::string resUrl = oss.str();
					/*
					char buffer[1024];
					memset(buffer, '\0', sizeof(buffer));
		            snprintf(buffer, sizeof(buffer), "%s-%ldL%d?ic=%ld&rate=%d&sid=true",
							segUrl.c_str(),buf->offsetInFile( ), buf->bufSize(),
							1000L*1000*1000*1000, reqBitrate);
					//snprintf(buffer, sizeof(buffer), "%s-%ldL%d", segUrl.c_str(), buf->offsetInFile( ), buf->bufSize());
					std::string resUrl(buffer);
					*/
					_log(ZQ::common::Log::L_INFO, CLOGFMT(ReadHttpManger, "newArequest() increase count[%d], will get data from old resUrl[%s] bufReqId[%ld], file[%s] timeout[%d]"),
							it->second->_count,resUrl.c_str(),buf->reqId(),filename.c_str(), _timeOut);
					HttpFetcher::Ptr httpPtr = new HttpFetcher(this, _proxyUrl,resUrl, _log, _timeOut, _retryTimes);
					httpPtr->read(buf);
				}
				return false;
			}
		}
	}

	bool ReadHttpManger::read( const std::vector<BufferUser>& bufs )
	{
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(ReadHttpManger, "read() entry with bufs[%ld]."), bufs.size());
		for (int i = 0; i < bufs.size(); i++ )
		{
			BufferUser buf = bufs[i];
			if ( !buf.valid() ) {
				assert(false && "get the null buffer from cache center.");
			}
			newArequest(buf->filename(),buf);//if locate OK,will read by HttpFetcher
		}
		return true;
	}

	bool ReadHttpManger::queryIndexInfo( const std::string& filename, AssetAttribute::Ptr attr )
	{
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(ReadHttpManger, "queryIndexInfo() entry with file[%s] segmenterurl[%s]."), filename.c_str(),_segmenterUrl.c_str());
		_c2client->queryIndexInfo(filename, attr);
		return true;
	}

	HttpFetcher::Ptr ReadHttpManger::addClient(const std::string& fileName,std::string sessionId, int64 offset, std::string resUrl, int retry /*= 0*/, int type /*=0*/)
	{
		uint64 start = currentTimeC2();
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(ReadHttpManger, "addClient() try to create a new client."));
		HttpFetcher::Ptr httpPtr = NULL;
		ZQ::common::MutexGuard gd(_clientMapLock);
		FileClientMap::iterator it = _clientMap.find(fileName);
		if(it == _clientMap.end())//not found,will new a client
		{
			//get IpPort
			ZQ::common::URLStr srcUrl(resUrl.c_str());
			std::string strServer = srcUrl.getHost();
			int nPort = srcUrl.getPort();

			uint64 curtime = currentTimeC2();
			httpPtr = new HttpFetcher(this, _proxyUrl, resUrl, _log, _timeOut, retry);
			ReadController::Ptr     readctrler = new ReadController(strServer,nPort,sessionId,curtime);
			{
				_clientMap[fileName] = readctrler;
			}
		}
		else//return exist value
		{
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(ReadHttpManger, "addClient() filename is Exist,will use old client bufReqId[%ld] resUrl[%s]"),offset,resUrl.c_str());
			httpPtr = new HttpFetcher(this, _proxyUrl, resUrl, _log, _timeOut, retry);
		}
		uint64 step = currentTimeC2();
		int a = (int)(step - start);
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(ReadHttpManger, "addClient() create a new client to request file[%s], bufReqId[%ld] usint[%d]."), fileName.c_str(), offset, a);
		return httpPtr;
	}

	void ReadHttpManger::delClient(const std::string& fileName, int64 offset, int type)
	{
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(ReadHttpManger, "delClient() try to delete client for file[%s], bufReqId[%ld]."), fileName.c_str(), offset);
		{
			ZQ::common::MutexGuard gd(_clientMapLock);
			_clientMap[fileName] = NULL;
			_clientMap.erase(fileName);
		}
	}

	void ReadHttpManger::onRead( BufferUser buf, bool retry /*=false*/ )
	{
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(ReadHttpManger, "onRead() will reading... bufReqId[%d]"),buf->reqId());
		std::vector<C2Streamer::BufferUser> bufs;
		bufs.push_back(buf);
		_callBack->onRead(bufs);
		return;
	}

	void ReadHttpManger::onLatency(std::string& fileName, int64 offset,int64 time)
	{
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(ReadHttpManger, "onLatency(), filename[%s] offset[%lld] time[%lld]"),fileName.c_str(),offset,time);
		_callBack->onLatency(fileName, offset, time);
	}

	void ReadHttpManger::onError( int err )
	{
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(ReadHttpManger, "onError(), error code[%d]"),err);
		_callBack->onError(err);
	}

	void ReadHttpManger::onIndexInfo( AssetAttribute::Ptr attr , bool retry /*=false*/)
	{
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(ReadHttpManger, "onIndexInfo()..."));
		_callBack->onIndexInfo(attr);
		return;
	}


	//////LocateClient//////
	LocateClient::LocateClient(const std::string filename,RequestParams params,ZQ::common::Log& log,ReadHttpManger* readHttpManger,const std::string& proxyUrl, const std::string& segmenterUrl,BufferUser buf):
		_filename(filename),
		_params(params),
		_log(log),
		_currRetryCount(0),
		_readHttpManger(readHttpManger),
		_proxyUrl(proxyUrl),
		_segmenterUrl(segmenterUrl),
		_buf(buf){
		_log.debug("LocateClient created with buf bufReqId[%ld]",_buf.reqId());
	}

	LocateClient::~LocateClient() {
		_log.debug("LocateClient destroied, buf bufReqId[%ld]", _buf.reqId());
	}

	bool LocateClient::startRequest()
	{
		_log(ZQ::common::Log::L_INFO, CLOGFMT(LocateClient, "ReadHttpManger a request: startRequest() prepare for locate request"));

		_phase = phaseLocate;

		_params.filename = _filename;
		_requestHandle = new LocateRequest(_log, this, _params, "ReadHttpManger", _buf->reqId());

		//return _requestHandle->process();
		(new AsyncRequest(_requestHandle, &_requestHandle->getLoop()))->queueWork();
		return true;
	}

	void LocateClient::onLocate(const LocateResponseData& resp)
	{
		ZQ::common::MutexGuard mg(_mutex);
		_log(ZQ::common::Log::L_INFO, CLOGFMT(LocateClient, "onLocate() locate request complete, prepare for get request"));
		_params.transferID  = resp.transferId;
		_params.getAddr = resp.transferPort;
		_params.isPwe = resp.openForWrite == "yes";

		std::string::size_type pDotPos = _params.filename.rfind(".");
		if( pDotPos != std::string::npos) {
			_params.subType = _params.filename.substr(pDotPos+1);
		}

		if (resp.portNum.empty())
		{
			_log(ZQ::common::Log::L_INFO, CLOGFMT(LocateClient, " onLocate() can't find 'PortNum' from locate response, try to use default port[%d]"), _params.defaultGetPort);
			_params.getPort = _params.defaultGetPort;
		}else{
			_params.getPort = atoi(resp.portNum.c_str());
		}
		/*
		_requestHandle = new GetRequest(_log, this, _params, "LocateClient", _buf->reqId());
		//_log(ZQ::common::Log::L_DEBUG, CLOGFMT(LocateClient, "[%s] bufReqID[%ld] onLocate() set request handle to NULL"), _instanceKey.c_str(), _bufferReqID);

		_phase = C2Streamer::phaseGet;

		//(new AsyncRequest(_requestHandle, &_requestHandle->getLoop()))->queueWork();
		//return;
		_requestHandle->process();
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(LocateClient, "bufReqID[%d] onLocate() process get request"), _buf->reqId());
		*/

		//add a new httpFetcher
		char Port[10] = {0};
		itoa(_params.getPort,Port,10);
		std::string IpPort = _params.getAddr + ":" + Port;
		//replace IpPort
		ZQ::common::URLStr srcUrl(_segmenterUrl.c_str());
		std::string strServer = srcUrl.getHost();
		std::string strOrgFile = srcUrl.getPath();
		int nPort = srcUrl.getPort();
		srcUrl.setHost(_params.getAddr.c_str());
		if(resp.portNum.empty())//respose no portNum
		{
			srcUrl.setPort(nPort);
		}
		else
		{
			srcUrl.setPort(_params.getPort);
		}
		std::string newUpSessUrl = std::string(srcUrl.generate());

		std::string sessionId = "";
		size_t pos = _params.transferID.rfind("/");
		if(pos != std::string::npos)
		{
			sessionId = _params.transferID.substr(pos+1);
		}
		else
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(LocateClient, "onLocate() bufReqID[%ld] get sessionId from [%s] id fail"), _buf->reqId(),_params.transferID.c_str());
		}
		//_log(ZQ::common::Log::L_DEBUG, CLOGFMT(LocateClient, "OnLocate()  URLStr Server[%s],OrgFile[%s],Port[%d] newUpSessUrl[%s]"),strServer.c_str(),strOrgFile.c_str(),nPort,newUpSessUrl.c_str());
		std::string  segUrl  = newUpSessUrl + "/" + sessionId ;//_params.filename; 
		int64 origBitrate = _buf->getBitrate();
		int64 reqBitrate = origBitrate * _params.bitrateInflate / 100;
		if(_params.minTransferRate > 0 && reqBitrate < _params.minTransferRate) {
			reqBitrate = _params.minTransferRate;
		}

		std::ostringstream oss;
		oss<<segUrl<<"-"<<_buf->offsetInFile()<<"L"<<_buf->bufSize()
			<<"?ic="<<1000L*1000*1000*1000<<"&rate="<<reqBitrate<<"&sid=true";
		std::string resUrl = oss.str();
		/*
		char buffer[1024];
		memset(buffer, '\0', sizeof(buffer));
		snprintf(buffer, sizeof(buffer), "%s-%ldL%d?ic=%ld&rate=%d&sid=true",
				segUrl.c_str(), _buf->offsetInFile( ), _buf->bufSize(),
				1000L*1000*1000*1000, reqBitrate);
		std::string resUrl(buffer);
		*/
		_log(ZQ::common::Log::L_INFO, CLOGFMT(LocateClient, "OnLocate() A new IpPort[%s],contentName[%s] filename[%s] resUrl[%s]"), IpPort.c_str(),_params.contentName.c_str(),_params.filename.c_str(),resUrl.c_str());
		HttpFetcher::Ptr pt = _readHttpManger->addClient(_params.filename,sessionId,_buf->reqId(),resUrl,0);
		if(pt == NULL)
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(LocateClient, "OnLocate() addclient is fail, bufReqId[%ld] resUrl[%s]"),_buf->reqId(),resUrl.c_str());
		}
		else
		{
			pt->read(_buf);
		}

		return;
	}

	void LocateClient::onData(const char* data, const size_t& size, bool error)
	{
		//_log(ZQ::common::Log::L_DEBUG, CLOGFMT(LocateClient, "onData() received data[%ld] error code[%d]"),size,error);
		/*
		//ZQ::common::MutexGuard gd(_mutex);
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(LocateClient, "[%s] bufReqID[%ld] onData() received data[%ld]"), _instanceKey.c_str(), _bufferReqID, size);

		int64 alignLength = size;
		if (error)
		{
		alignLength = ((size - _params.alignment * 1024) / (_params.alignment*1024)) * _params.alignment*1024;
		alignLength = alignLength < 0 ? 0 : alignLength;
		_alignAbandonData.clear();
		_alignAbandonData.append(data + alignLength, size - alignLength);
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(LocateClient, "[%s] bufReqID[%ld] onData() error occured, useful data[%ld], align abandon data[%ld]"), _instanceKey.c_str(), _bufferReqID, alignLength, size - alignLength);
		}

		size_t leftDataSize = alignLength;
		size_t allCopySize = 0;

		std::vector<C2Streamer::BufferUser>::iterator it = _readerBufs.begin();
		for (; it != _readerBufs.end(); )
		{
		if (leftDataSize <= 0) break;

		size_t currBufLeftSize = (*it)->bufSize() - (*it)->getDataSize();
		if (currBufLeftSize > leftDataSize)
		{
		char *startWritePoint = (*it)->buffer() + (*it)->getDataSize();
		memcpy(startWritePoint, data, leftDataSize);
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(LocateClient, "[%s] bufReqID[%ld] onData() copy data[%ld] to buffer, buffer offset[%ld], dataSize[%d]"), _instanceKey.c_str(), _bufferReqID, leftDataSize, (*it)->offsetInFile(), (*it)->getDataSize());

		(*it)->setDataSize((*it)->getDataSize() + leftDataSize);
		leftDataSize = 0;
		break;
		}else{
		//the free space less than received data size
		char *startDstPoint = (*it)->buffer() + (*it)->getDataSize();
		char *startSrcPoint =  (char*)data + allCopySize;

		int copySize;
		if (leftDataSize > currBufLeftSize)
		{
		copySize = currBufLeftSize;
		currBufLeftSize = 0;
		}else{
		copySize = leftDataSize;
		currBufLeftSize = currBufLeftSize - copySize;
		}

		memcpy(startDstPoint, startSrcPoint, copySize);
		(*it)->setDataSize((*it)->bufSize() - currBufLeftSize);
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(LocateClient, "[%s] bufReqID[%ld] onData() copy data[%d] to buffer, buffer offset[%ld]"), _instanceKey.c_str(), _bufferReqID, copySize, (*it)->offsetInFile());

		leftDataSize = leftDataSize - copySize;
		allCopySize += copySize;

		if (currBufLeftSize == 0)
		{
		// return buffer and fill next buffer
		std::vector<C2Streamer::BufferUser> bufs;
		bufs.push_back(*it);
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(LocateClient, "[%s] bufReqID[%ld] onData() call onRead(), buffer number[%ld], first buffer offset[%ld], data size[%ld]"), _instanceKey.c_str(), _bufferReqID, bufs.size(), (*bufs.begin())->offsetInFile(), (*bufs.begin())->getDataSize());
		_cb->onRead(bufs);
		it = _readerBufs.erase(it);
		}
		}
		}

		if (leftDataSize > 0)
		{
		_tmpBuf.append(data + (alignLength - leftDataSize), leftDataSize);
		_log(ZQ::common::Log::L_INFO, CLOGFMT(LocateClient, "[%s] bufReqID[%ld] onData() no read buffer left, copy left data[%ld] to temp buffer"), _instanceKey.c_str(), _bufferReqID, leftDataSize);

		// #TODO add wait buffer timeout
		_requestHandle->waitBuffer(_params.waitBufferTime);
		return;
	}
	*/
	}

	void LocateClient::onRecvComplete()
	{
		/*
		//ZQ::common::MutexGuard mg(_mutex);

		_cb->onRead(_readerBufs); // response the rest buffer firstly
		_readerBufs.clear();

		if (_tmpBuf.empty())
		{
		_log(ZQ::common::Log::L_INFO, CLOGFMT(LocateClient, "[%s] bufReqID[%ld] onRecvComplete() get request complete"), _instanceKey.c_str(), _bufferReqID);
		}else{
		_log(ZQ::common::Log::L_INFO, CLOGFMT(LocateClient, "[%s] bufReqID[%ld] onRecvComplete() there are also have some data[%ld] at temp buffer, wait new buffer"), _instanceKey.c_str(), _bufferReqID, _tmpBuf.size());
		}

		// #TODO send transfer delete
		if (_params.transferDelete)
		{
		_log(ZQ::common::Log::L_INFO, CLOGFMT(LocateClient, "[%s] bufReqID[%ld] onRecvComplete() transfer delete is enabled, prepare for send transfer delete request"), _instanceKey.c_str(), _bufferReqID);

		_requestHandle = new TransferDelete(_log, this, _params, _instanceKey, _bufferReqID);
		_phase = C2Streamer::phaseTransferDelete;
		_requestHandle->process();
		return;
		}

		_phase = C2Streamer::phaseDone;
		if (_tmpBuf.empty())
		{
		_cb->onReadComplete(_key);
		_requestHandle = NULL;
		return;
		}

		// also have data at temp buffer
		_requestHandle->update(_params.waitBufferTime);
		*/
	}

	void LocateClient::onTransferDelete()
	{
		/*
		//ZQ::common::MutexGuard mg(_mutex);
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(LocateClient, "[%s] bufReqID[%ld] onTransferDelete() transfer delete complete"), _instanceKey.c_str(), _bufferReqID);

		_phase = C2Streamer::phaseDone;

		if (!_tmpBuf.empty())
		{
		_log(ZQ::common::Log::L_WARNING, CLOGFMT(LocateClient, "[%s] bufReqID[%ld] onTransferDelete() there are also have some data[%ld] at temp buffer, wait new buffer"), _instanceKey.c_str(), _bufferReqID, _tmpBuf.size());
		_requestHandle->update(_params.waitBufferTime);
		//_requestHandle = NULL;
		return;
		}

		_cb->onReadComplete(_key);
		_requestHandle = NULL;
		*/
	}

	void LocateClient::onError(C2RequestErrorCategory category, const int& err, const std::string& msg)
	{
		ZQ::common::MutexGuard gd(_mutex);
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(LocateClient, " onError() bufReqId[%ld] error[%d:%s] current phase[%s], retry times[%d/%d]"), _buf->reqId(), err, msg.c_str(), getCurrPhaseStr().c_str(), _currRetryCount, getMaxRetry());

		if (_currRetryCount < getMaxRetry() && 
				(err == 503 ||
				 ZQ::StreamService::crTimeout == category || 
				 ZQ::StreamService::crWaitBufferTimeout == category || 
				 ZQ::StreamService::crSocketError == category))
		{
			if (!retry())
			{
				_log(ZQ::common::Log::L_ERROR, CLOGFMT(LocateClient, "onError() locate request failed"));
				onError(ZQ::StreamService::crGeneric, -14, "locate request failed");
			}
			return;
		}
		_buf->setLastError(err, convertErrorCategoty(category));
		_phase = C2Streamer::phaseDone;
		_readHttpManger->onRead(_buf);
		_readHttpManger->onError(err);
	}

	bool LocateClient::retry()
	{
		_log(ZQ::common::Log::L_INFO, CLOGFMT(LocateClient, "retry(),bufReqId[%ld] [%d]-th retry"),_buf->reqId(), ++_currRetryCount);
		return startRequest();
	}

	int LocateClient::getMaxRetry()
	{
		if ("index" == _params.subType)
		{
			return _params.indexRetryTimes;
		}
		return _params.mainfileRetryTimes;
	}

	C2Streamer::Buffer::ErrorCategory  LocateClient::convertErrorCategoty(C2RequestErrorCategory category)
	{
		switch(category)
		{
			case crGeneric:
				return C2Streamer::Buffer::ECATE_FILEIO;
			case crSocketError:
				return C2Streamer::Buffer::ECATE_SOCKET;
			case crHttpError:
				return C2Streamer::Buffer::ECATE_HTTP;
			case crParseXmlDataError:
				return C2Streamer::Buffer::ECATE_HTTP;
			case crTimeout:
				return C2Streamer::Buffer::ECATE_TIMEOUT;
			case crWaitBufferTimeout:
				return C2Streamer::Buffer::ECATE_CLIENTTIMEOUT;
			case crNotFound:
				return C2Streamer::Buffer::ECATE_NOTFOUND;
			case crServiceUnavailable:
				return C2Streamer::Buffer::ECATE_HTTP;
			default:
				assert(false);
				return C2Streamer::Buffer::ECATE_FILEIO;
		}
	}

	std::string LocateClient::getCurrPhaseStr()
	{
		switch(_phase)
		{
			case C2Streamer::phaseInit:
				return "Init";
			case C2Streamer::phaseLocate:
				return "Locate";
			case C2Streamer::phaseGet:
				return "Get";
			case C2Streamer::phaseTransferDelete:
				return "TransferDelete";
			case C2Streamer::phaseDone:
				return "Done";
		}
		return "";
	}

}
