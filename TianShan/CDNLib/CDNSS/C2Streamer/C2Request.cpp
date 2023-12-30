#include "C2Request.h"

namespace ZQ{
namespace StreamService{
    ZQ::common::AtomicInt C2ReadFile::_atomicID;
    ZQ::common::AtomicInt C2QueryIndex::_atomicID;
    C2ReadFile::C2ReadFile(IReadCB::Ptr readClient, ZQ::common::Log& log, RequestParams params, int64 bufferReqID)
        : _log(log), _cb(readClient), _params(params), _currRetryCount(0), _instanceKey(""), _key(""), _tmpBuf(""), _bufferReqID(bufferReqID)
    {
        _atomicID.inc();
        _requestInstID = _atomicID.get();

        std::ostringstream oss;
        oss<<"C2Request "<<_atomicID.get();
        _instanceKey = oss.str();

        _phase = ZQ::StreamService::phaseInit;
        _log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2ReadFile, "[%s] bufReqID[%ld] constructor entry, enter phase [%s]"), _instanceKey.c_str(), _bufferReqID, getCurrPhaseStr().c_str());
    }

    C2ReadFile::~C2ReadFile()
    {
        _log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2ReadFile, "[%s] bufReqID[%ld] destructor entry, release resources"), _instanceKey.c_str(), _bufferReqID);
        
        if (_requestHandle)
        {
            _requestHandle = NULL;
            //_log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2ReadFile, "[%s] bufReqID[%ld] set request handle to NULL"), _instanceKey.c_str(), _bufferReqID);
        }
    }

    bool C2ReadFile::startRequest()
    {
        _log(ZQ::common::Log::L_INFO, CLOGFMT(C2ReadFile, "[%s] bufReqID[%ld] startRequest() prepare for locate request"), _instanceKey.c_str(), _bufferReqID);

        _phase = ZQ::StreamService::phaseLocate;

        std::vector<C2Streamer::Buffer*>::iterator it = _readerBufs.begin();
        if (it == _readerBufs.end())
        {
            _log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ReadFile, "[%s] bufReqID[%ld] startRequest() no buffer"), _instanceKey.c_str(), _bufferReqID);
            return false;
        }
        size_t startOffset = (*it)->offsetInFile() + (*it)->getDataSize();
        size_t endOffset   = (*it)->offsetInFile() + (*it)->bufSize() - 1;
       
        std::ostringstream rangeOss;
        rangeOss << startOffset << "-" << endOffset;
        _params.range = rangeOss.str();

        _log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2ReadFile, "[%s] bufReqID[%ld] startRequest() current retry times[%d/%d] request range[%s]"), _instanceKey.c_str(), _bufferReqID, _currRetryCount, getMaxRetry(), _params.range.c_str());
        _requestHandle = new LocateRequest(_log, this, _params, _instanceKey, _bufferReqID);

        (new AsyncRequest(_requestHandle, &_requestHandle->getLoop()))->queueWork();
        return true;        
    }

    bool C2ReadFile::retry()
    {
        _log(ZQ::common::Log::L_INFO, CLOGFMT(C2ReadFile, "[%s] bufReqID[%ld] retry(), [%d]-th retry"), _instanceKey.c_str(), _bufferReqID, ++_currRetryCount);
        return startRequest();
    }

    int C2ReadFile::getMaxRetry()
    {
        if ("index" == _params.subType)
        {
            return _params.indexRetryTimes;
        }
        return _params.mainfileRetryTimes;
    }

    C2Streamer::Buffer::ErrorCategory  C2ReadFile::convertErrorCategoty(C2RequestErrorCategory category)
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
        }
    }

    std::string C2ReadFile::getCurrPhaseStr()
    {
        switch(_phase)
        {
        case ZQ::StreamService::phaseInit:
            return "Init";
        case ZQ::StreamService::phaseLocate:
            return "Locate";
        case ZQ::StreamService::phaseGet:
            return "Get";
        case ZQ::StreamService::phaseTransferDelete:
            return "TransferDelete";
        case ZQ::StreamService::phaseDone:
            return "Done";
        }
    }

    bool C2ReadFile::checkTempBuffer()
    {
        if (!_tmpBuf.empty())
        {
            _log(ZQ::common::Log::L_INFO, CLOGFMT(C2ReadFile, "[%s] bufReqID[%ld] checkTempBuffer() there are some data[%ld] at temp buffer, move them to read buffer firstly"), _instanceKey.c_str(), _bufferReqID, _tmpBuf.size());

            std::vector<C2Streamer::Buffer*>::iterator it = _readerBufs.begin();
            for (; it != _readerBufs.end(); )
            {
                char *startWritePoint = (*it)->buffer() + (*it)->getDataSize();

                size_t bufLeftSize = (*it)->bufSize() - (*it)->getDataSize();
                size_t copySize = 0;
                if (bufLeftSize > _tmpBuf.size())
                {
                    copySize = _tmpBuf.size();
                    memcpy(startWritePoint, _tmpBuf.c_str(), copySize);
                    (*it)->setDataSize((*it)->getDataSize() + copySize);

                    _tmpBuf.clear();
                    break;
                }else{
                    copySize = bufLeftSize;
                    memcpy(startWritePoint, _tmpBuf.c_str(), copySize);
                    (*it)->setDataSize((*it)->getDataSize() + copySize);

                    std::vector<C2Streamer::Buffer*> bufsVec;
                    bufsVec.push_back(*it);

                    _log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2ReadFile, "[%s] bufReqID[%ld] checkTempBuffer() call onRead(), buffer number[%ld], first buffer offset[%ld], data size[%ld]"), _instanceKey.c_str(), _bufferReqID, bufsVec.size(), (*bufsVec.begin())->offsetInFile(), (*bufsVec.begin())->getDataSize());
                    _cb->onRead(bufsVec);

                    it = _readerBufs.erase(it);
                    _tmpBuf = _tmpBuf.substr(copySize);
                }
            }
        }

        if (_tmpBuf.size() > 0)
        {
            // still have data at temp buffer
            _log(ZQ::common::Log::L_INFO, CLOGFMT(C2ReadFile, "[%s] bufReqID[%ld] checkTmpBuffer(), there are data[%ld] at temp buffer, but no read buffer left"), _instanceKey.c_str(), _bufferReqID, _tmpBuf.size());
            return false;
        }else{
            if (_phase == ZQ::StreamService::phaseDone)
            {
                _log(ZQ::common::Log::L_INFO, CLOGFMT(C2ReadFile, "[%s] bufReqID[%ld] checkTmpBuffer(), take over all temp data, read request complete"), _instanceKey.c_str(), _bufferReqID);
                
                if (_requestHandle)
                {
                    _requestHandle->cancel();   // cancel timer
                }

                _cb->onRead(_readerBufs);
                _cb->onReadComplete(_key);
                _readerBufs.clear();
                return true;
            }
        }
        return true;
    }

    int  C2ReadFile::getInstanceID()
    {
        return _requestInstID;
    }

    bool C2ReadFile::addBuffer(const std::vector<C2Streamer::Buffer*>& bufs)
    {
        //ZQ::common::MutexGuard gd(_mutex);
        if (bufs.empty())
        {
            _log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2ReadFile, "[%s] bufReqID[%ld] addBuffer() there is no buffer to add"), _instanceKey.c_str(), _bufferReqID);
            return false;
        }

        //update key
        std::ostringstream oss;
        oss<<bufs[bufs.size()-1]->filename()<<":"<<bufs[bufs.size()-1]->offsetInFile()+bufs[bufs.size()-1]->bufSize();
        _key = oss.str();

        _readerBufs.insert(_readerBufs.end(), bufs.begin(), bufs.end());
        _log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2ReadFile, "[%s] bufReqID[%ld] addBuffer() received buffer number[%ld] first buffer offset[%ld]"), _instanceKey.c_str(), _bufferReqID, bufs.size(), bufs[0]->offsetInFile());

        if (ZQ::StreamService::phaseInit == _phase)
        {
            _log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2ReadFile, "[%s] bufReqID[%ld] addBuffer() due to current phase is [Init], start request immediately"), _instanceKey.c_str(), _bufferReqID);
            if (!startRequest())
            {
                _log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ReadFile, "[%s] bufReqID[%ld] addBuffer() locate request failed"), _instanceKey.c_str(), _bufferReqID);
                onError(ZQ::StreamService::crGeneric, -14, "locate request failed");
                return false;
            }
            return true;
        }

        if (!checkTempBuffer())
        {// also have data at temp buffer
            _log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ReadFile, "[%s] bufReqID[%ld] addBuffer() no enough buffer to receive temp data, current phase[%s]"), _instanceKey.c_str(), _bufferReqID, getCurrPhaseStr().c_str());
            if (ZQ::StreamService::phaseGet == _phase)
            {
                _requestHandle->waitBuffer(_params.waitBufferTime);
            }
            return true;
        }
        
        if (ZQ::StreamService::phaseTransferDelete == _phase || ZQ::StreamService::phaseDone == _phase)
        {
            _log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ReadFile, "[%s] bufReqID[%ld] addBuffer() couldn't accept buffer, current phase[%s]"), \
                _instanceKey.c_str(), _bufferReqID, getCurrPhaseStr().c_str());

            onError(ZQ::StreamService::crGeneric, -16, "couldn't accept buffer, request is done");
            return false;
        }
        return true;
    }

    void C2ReadFile::onLocate(const LocateResponseData& resp)
    {
        //ZQ::common::MutexGuard mg(_mutex);
        _log(ZQ::common::Log::L_INFO, CLOGFMT(C2ReadFile, "[%s] bufReqID[%ld] onLocate() locate request complete, prepare for get request"), _instanceKey.c_str(), _bufferReqID);
        _params.transferID  = resp.transferId;
        _params.getAddr = resp.transferPort;

        if (resp.portNum.empty())
        {
            _log(ZQ::common::Log::L_INFO, CLOGFMT(C2ReadFile, "[%s] bufReqID[%ld] onLocate() can't find 'PortNum' from locate response, try to use default port[%d]"), _instanceKey.c_str(), _bufferReqID, _params.defaultGetPort);
            _params.getPort = _params.defaultGetPort;
        }else{
            _params.getPort = atoi(resp.portNum.c_str());
        }

        _requestHandle = new GetRequest(_log, this, _params, _instanceKey, _bufferReqID);
        //_log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2ReadFile, "[%s] bufReqID[%ld] onLocate() set request handle to NULL"), _instanceKey.c_str(), _bufferReqID);

        _phase = ZQ::StreamService::phaseGet;

        (new AsyncRequest(_requestHandle, &_requestHandle->getLoop()))->queueWork();
        return;        
        //_requestHandle->process();
        //_log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2ReadFile, "[%s] bufReqID[%ld] onLocate() process get request"), _instanceKey.c_str(), _bufferReqID);
    }

    void C2ReadFile::onData(const char* data, const size_t& size, bool error)
    {
        //ZQ::common::MutexGuard gd(_mutex);
        _log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2ReadFile, "[%s] bufReqID[%ld] onData() received data[%ld]"), _instanceKey.c_str(), _bufferReqID, size);

        int64 alignLength = size;
        if (error)
        {
            alignLength = ((size - _params.alignment * 1024) / (_params.alignment*1024)) * _params.alignment*1024;
            alignLength = alignLength < 0 ? 0 : alignLength;
            _alignAbandonData.clear();
            _alignAbandonData.append(data + alignLength, size - alignLength);
            _log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2ReadFile, "[%s] bufReqID[%ld] onData() error occured, useful data[%ld], align abandon data[%ld]"), _instanceKey.c_str(), _bufferReqID, alignLength, size - alignLength);
        }

        size_t leftDataSize = alignLength;
        size_t allCopySize = 0;

        std::vector<C2Streamer::Buffer*>::iterator it = _readerBufs.begin();
        for (; it != _readerBufs.end(); )
        {
            if (leftDataSize <= 0) break;

            size_t currBufLeftSize = (*it)->bufSize() - (*it)->getDataSize();
            if (currBufLeftSize > leftDataSize)
            {
                char *startWritePoint = (*it)->buffer() + (*it)->getDataSize();
                memcpy(startWritePoint, data, leftDataSize);
                _log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2ReadFile, "[%s] bufReqID[%ld] onData() copy data[%ld] to buffer, buffer offset[%ld]"), _instanceKey.c_str(), _bufferReqID, leftDataSize, (*it)->offsetInFile());

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
                _log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2ReadFile, "[%s] bufReqID[%ld] onData() copy data[%d] to buffer, buffer offset[%ld]"), _instanceKey.c_str(), _bufferReqID, copySize, (*it)->offsetInFile());

                leftDataSize = leftDataSize - copySize;
                allCopySize += copySize;

                if (currBufLeftSize == 0)
                {
                    // return buffer and fill next buffer
                    std::vector<C2Streamer::Buffer*> bufs;
                    bufs.push_back(*it);
                    _log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2ReadFile, "[%s] bufReqID[%ld] onData() call onRead(), buffer number[%ld], first buffer offset[%ld], data size[%ld]"), _instanceKey.c_str(), _bufferReqID, bufs.size(), (*bufs.begin())->offsetInFile(), (*bufs.begin())->getDataSize());
                    _cb->onRead(bufs);
                    it = _readerBufs.erase(it);
                }
            }
        }

        if (leftDataSize > 0)
        {
            _tmpBuf.append(data + (alignLength - leftDataSize), leftDataSize);
            _log(ZQ::common::Log::L_INFO, CLOGFMT(C2ReadFile, "[%s] bufReqID[%ld] onData() no read buffer left, copy left data[%ld] to temp buffer"), _instanceKey.c_str(), _bufferReqID, leftDataSize);

            // #TODO add wait buffer timeout
            _requestHandle->waitBuffer(_params.waitBufferTime);
            return;
        }
    }

    void C2ReadFile::onRecvComplete()
    {
        //ZQ::common::MutexGuard mg(_mutex);

        _cb->onRead(_readerBufs); // response the rest buffer firstly
        _readerBufs.clear();

        if (_tmpBuf.empty())
        {
            _log(ZQ::common::Log::L_INFO, CLOGFMT(C2ReadFile, "[%s] bufReqID[%ld] onRecvComplete() get request complete"), _instanceKey.c_str(), _bufferReqID);
        }else{
            _log(ZQ::common::Log::L_INFO, CLOGFMT(C2ReadFile, "[%s] bufReqID[%ld] onRecvComplete() there are also have some data[%ld] at temp buffer, wait new buffer"), _instanceKey.c_str(), _bufferReqID, _tmpBuf.size());
        }

        // #TODO send transfer delete
        if (_params.transferDelete)
        {
            _log(ZQ::common::Log::L_INFO, CLOGFMT(C2ReadFile, "[%s] bufReqID[%ld] onRecvComplete() transfer delete is enabled, prepare for send transfer delete request"), _instanceKey.c_str(), _bufferReqID);

            _requestHandle = new TransferDelete(_log, this, _params, _instanceKey, _bufferReqID);
            _phase = ZQ::StreamService::phaseTransferDelete;
            _requestHandle->process();
            return;
        }

        _phase = ZQ::StreamService::phaseDone;
        if (_tmpBuf.empty())
        {
            _cb->onReadComplete(_key);
            _requestHandle = NULL;
            return;
        }

        // also have data at temp buffer
        _requestHandle->update(_params.waitBufferTime);
    }

    void C2ReadFile::onTransferDelete()
    {
        //ZQ::common::MutexGuard mg(_mutex);
        _log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2ReadFile, "[%s] bufReqID[%ld] onTransferDelete() transfer delete complete"), _instanceKey.c_str(), _bufferReqID);

        _phase = ZQ::StreamService::phaseDone;

        if (!_tmpBuf.empty())
        {
            _log(ZQ::common::Log::L_WARNING, CLOGFMT(C2ReadFile, "[%s] bufReqID[%ld] onTransferDelete() there are also have some data[%ld] at temp buffer, wait new buffer"), _instanceKey.c_str(), _bufferReqID, _tmpBuf.size());
            _requestHandle->update(_params.waitBufferTime);
            //_requestHandle = NULL;
            return;
        }

        _cb->onReadComplete(_key);
        _requestHandle = NULL;
    }

    void C2ReadFile::onError(C2RequestErrorCategory category, const int& err, const std::string& msg)
    {
        //ZQ::common::MutexGuard gd(_mutex);
        _log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ReadFile, "[%s] bufReqID[%ld] onError() error[%d:%s] current phase[%s], retry times[%d/%d]"), _instanceKey.c_str(), _bufferReqID, err, msg.c_str(), getCurrPhaseStr().c_str(), _currRetryCount, getMaxRetry());

        if (_currRetryCount < getMaxRetry() && 
            (ZQ::StreamService::crTimeout == category || ZQ::StreamService::crWaitBufferTimeout == category || ZQ::StreamService::crSocketError == category))
        {
            // clear alignment abadon data
            _log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2ReadFile, "[%s] bufReqID[%ld] start to retry, current abandon data[%ld], clean it"), _instanceKey.c_str(), _bufferReqID, _alignAbandonData.length());
            _alignAbandonData.clear();
            if (!retry())
            {
                _log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ReadFile, "[%s] bufReqID[%ld] onError() locate request failed"), _instanceKey.c_str(), _bufferReqID);
                onError(ZQ::StreamService::crGeneric, -14, "locate request failed");
            }
            return;
        }

        // do not retry, then copy alignment abadon data to buffer
        if (!_alignAbandonData.empty())
        {
            _log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2ReadFile, "[%s] bufReqID[%ld] onError() no need for retry, copy alignment abadon data[%ld] to buffer"), _instanceKey.c_str(), _bufferReqID, _alignAbandonData.length());
            onData(_alignAbandonData.c_str(), _alignAbandonData.length());
            _alignAbandonData.clear();
        }
        _phase = ZQ::StreamService::phaseDone;

        std::vector<C2Streamer::Buffer*>::iterator it = _readerBufs.begin();
        for (; it != _readerBufs.end(); it++)
        {
            (*it)->setLastError(err, convertErrorCategoty(category));
        }

        _cb->onRead(_readerBufs);
        _cb->onReadComplete(_key);
        _cb->onError(err);
        _readerBufs.clear();
       // _requestHandle = NULL;
    }

    // class C2QueryIndex
    C2QueryIndex::C2QueryIndex(IReadCB::Ptr readClient, ZQ::common::Log& log, RequestParams params, \
        C2Streamer::AssetAttribute::Ptr attr)
        : _log(log), _cb(readClient), _params(params), _assetAttr(attr)
    {
        _phase = ZQ::StreamService::phaseInit;
        _atomicID.inc();
        _queryIndexInstID = _atomicID.get();

        std::ostringstream oss;
        oss<<"QueryIndex "<<_atomicID.get();
        _instanceKey = oss.str();

        //_reqID = attr->reqId();
    }

    C2QueryIndex::~C2QueryIndex()
    {
        if (_requestHandle)
        {
            _requestHandle = NULL;
        }
    }

    bool C2QueryIndex::startRequest()
    {
        //ZQ::common::MutexGuard mg(_mutex);
        _params.range = "0-4095";

        _requestHandle = new LocateRequest(_log, this, _params, _instanceKey, _reqID);
        return _requestHandle->process();
    }

    void C2QueryIndex::onLocate(const LocateResponseData& resp)
    {
        //ZQ::common::MutexGuard mg(_mutex);
        _log(ZQ::common::Log::L_INFO, CLOGFMT(C2QueryIndex, "[%s] bufReqID[%ld] onLocate() locate request complete, prepare for get request"), _instanceKey.c_str(), _reqID);
        _locateData = resp;

        _params.transferID  = resp.transferId;
        _params.getAddr = resp.transferPort;

        if (resp.portNum.empty())
        {
            _log(ZQ::common::Log::L_INFO, CLOGFMT(C2QueryIndex, "[%s] bufReqID[%ld] onLocate() can't find 'PortNum' from locate response, try to use default port[%d]"), _instanceKey.c_str(), _reqID, _params.defaultGetPort);
            _params.getPort = _params.defaultGetPort;
        }else{
            _params.getPort = atoi(resp.portNum.c_str());
        }

        _requestHandle = new GetRequest(_log, this, _params, _instanceKey, _reqID);

        _phase = ZQ::StreamService::phaseGet;
        _requestHandle->process();
    }

    void C2QueryIndex::onData(const char* data, const size_t& size, bool error)
    {
        //ZQ::common::MutexGuard mg(_mutex);
        _indexData.append(data, size);
    }

    void C2QueryIndex::onRecvComplete()
    {
        //ZQ::common::MutexGuard mg(_mutex);
        // parse index file
        _log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2QueryIndex, "[%s] bufReqID[%ld] onRecvComplete() begin parse index file"), _instanceKey.c_str(), _reqID);

        ZQ::IdxParser::IndexData idxData;
        size_t ppos = _params.filename.find_last_of('.');
        std::string contentName = _params.filename.substr(0, ppos);
        // remove slash
        size_t lastSlashPos = contentName.find_last_of(FNSEPC);
        if (std::string::npos != lastSlashPos)
        {
            contentName = contentName.substr(lastSlashPos + 1);
        }
        bool result = parseIndex(contentName, _indexData.c_str(), _indexData.size(), idxData);
        if (result)
        {
            _log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2QueryIndex, "[%s] bufReqID[%ld] onRecvComplete() parse index success"), _instanceKey.c_str(), _reqID);

            if ("yes" == _locateData.openForWrite)
            {
                _assetAttr->pwe(true);
            }else{
                _assetAttr->pwe(false);
            }

            ZQ::IdxParser::IndexData::SubFileInformation info;
            if (idxData.getSubFileInfo(0, info))
            {
                std::ostringstream startOffsetOSS;
                uint64 startOffset =info.startingByte;
                startOffsetOSS<<startOffset;


                std::ostringstream endOffsetOSS;
                uint64 endOffset   = info.endingByte;
                endOffsetOSS<<endOffset;

                _assetAttr->range(atol(startOffsetOSS.str().c_str()), atol(endOffsetOSS.str().c_str()));
            }

            _assetAttr->assetBaseInfo(idxData.baseInfoToXML());
            _assetAttr->assetMemberInfo(idxData.memberFileToXML());
            _cb->onIndexInfo(_assetAttr);

            _log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2QueryIndex, "[%s] bufReqID[%ld] onRecvComplete() query index successful"), _instanceKey.c_str(), _reqID);
        }else{
            // parse index failed
            _assetAttr->lastError(C2Streamer::AssetAttribute::ASSET_DATAERR);
            _cb->onIndexInfo(_assetAttr);
            _log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2QueryIndex, "[%s] bufReqID[%ld] onRecvComplete() query index failed"), _instanceKey.c_str(), _reqID);
            _requestHandle = NULL;
            return;
        }

        if (_params.transferDelete)
        {
            _requestHandle = new TransferDelete(_log, this, _params, _instanceKey, _reqID);

            _phase = ZQ::StreamService::phaseTransferDelete;
            _requestHandle->process();
            return;
        }
        _requestHandle = NULL;
    }
    void C2QueryIndex::onTransferDelete()
    {
        //ZQ::common::MutexGuard mg(_mutex);
        _phase = ZQ::StreamService::phaseDone;
        _log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2QueryIndex, "[%s] bufReqID[%ld] onTransferDelete() transfer delete complete"), _instanceKey.c_str(), _reqID);
        _requestHandle = NULL;
    }

    void C2QueryIndex::onError(C2RequestErrorCategory category, const int& err, const std::string& msg)
    {
        //ZQ::common::MutexGuard gd(_mutex);
        _phase = ZQ::StreamService::phaseDone;

        _assetAttr->lastError(convertErrorCategoty(category));
        _cb->onIndexInfo(_assetAttr);
        //_requestHandle = NULL;
    }

    bool C2QueryIndex::parseIndex(std::string& contentName, const char* indexData, size_t dataSize, ZQ::IdxParser::IndexData& idxData)
    {
        ZQ::IdxParser::IdxParserEnv			idxParserEnv;
        idxParserEnv.AttchLogger(&_log);
        ZQ::IdxParser::IndexFileParser		idxParser(idxParserEnv);

        if(!idxParser.ParseIndexFromMemory( contentName, idxData, indexData, dataSize ) ) 
        {
            _log(ZQ::common::Log::L_ERROR,CLOGFMT(C2QueryIndex,"[%s] bufReqID[%ld] parseIndex() failed to parse index data for[%s], data size[%u]"), _instanceKey.c_str(), _reqID, contentName.c_str(), (uint32)dataSize);
            return false;
        }
        return true;
    }

    int  C2QueryIndex::getInstanceID()
    {
        return _queryIndexInstID;
    }

    C2Streamer::AssetAttribute::LASTERR C2QueryIndex::convertErrorCategoty(C2RequestErrorCategory category)
    {
        switch(category)
        {
        case crGeneric:
            return C2Streamer::AssetAttribute::ASSET_DATAERR;
        case crSocketError:
            return C2Streamer::AssetAttribute::ASSET_SOCKET;
        case crHttpError:
            return C2Streamer::AssetAttribute::ASSET_HTTP;
        case crParseXmlDataError:
            return C2Streamer::AssetAttribute::ASSET_DATAERR;
        case crTimeout:
            return C2Streamer::AssetAttribute::ASSET_TIMEOUT;
        case crWaitBufferTimeout:
            return C2Streamer::AssetAttribute::ASSET_TIMEOUT;
        }
    }

}}
