#include "ReadClient.h"
#include <time.h>

namespace ZQ{
    namespace StreamService{
        ReadClient::ReadClient(C2Streamer::IReaderCallback* readerCB, ZQ::common::Log& log, RequestParams& params)
            :_log(log), _params(params), _readerCB(readerCB)
        {
            _log(ZQ::common::Log::L_DEBUG, CLOGFMT(ReadClient, "ReadClient() constructor entry [%p]"), this);

        }

        ReadClient::~ReadClient()
        {
            _log(ZQ::common::Log::L_DEBUG, CLOGFMT(ReadClient, "ReadClient() destructor entry [%p]"), this);
        }

        bool ReadClient::read( const std::vector<C2Streamer::BufferUser>& bufs )
        {
            _log(ZQ::common::Log::L_DEBUG, CLOGFMT(ReadClient, "read() entry, buffer number[%ld]"), bufs.size());

            if (0 == bufs.size())
            {
                _log(ZQ::common::Log::L_ERROR, CLOGFMT(ReadClient, "read() fail to read, there is no buffer for use"));
                return false;
            }

            ZQ::common::MutexGuard gd(_mutex);

            std::vector<C2Streamer::BufferUser>::const_iterator beginIter = bufs.begin();
            std::string filename = (*beginIter)->filename();
            
            size_t bufIndex = 0;
            size_t maxBufferCountPerRequest = _params.maxBufferCountPerRequest;
            size_t increment = maxBufferCountPerRequest != 0 ? maxBufferCountPerRequest : bufs.size();
            while(bufIndex < bufs.size())
            {
                size_t bufferCount = MIN(bufs.size() - bufIndex, increment);
                std::vector<C2Streamer::BufferUser> readBufs;
                readBufs.insert(readBufs.begin(), bufs.begin() + bufIndex, bufs.begin() + bufIndex + bufferCount);
                bufIndex += bufferCount;

                std::ostringstream oss;
                oss<<filename<<":"<<readBufs[0]->offsetInFile();
                std::string key = oss.str();

                std::ostringstream ossNew;
                ossNew<<filename<<":"<<readBufs[readBufs.size() - 1]->offsetInFile() + readBufs[readBufs.size() - 1]->bufSize();
                std::string newKey = ossNew.str();

				int64 origBitrate = bufs[0]->getBitrate();
				int64 reqBitrate = origBitrate * _params.bitrateInflate / 100;
				if(_params.minTransferRate > 0 && reqBitrate < _params.minTransferRate) {
					reqBitrate = _params.minTransferRate;
				}

				if( (_params.maxBitrate > 0) && (reqBitrate > _params.maxBitrate) ) {
					reqBitrate = _params.maxBitrate;
				}

                RequestParams requestParams = _params;
                requestParams.filename = filename;
				if( origBitrate > 0 ) {
					std::ostringstream ossBitrate;
					ossBitrate << reqBitrate;
					requestParams.transferRate = ossBitrate.str();
					if( reqBitrate > 0 ) {
						requestParams.mainfileTimeout =(int)((int64)bufferCount * bufs[0].bufferSize() * 8000 / reqBitrate * 130 /100) + 100;
					}
				}

                // create a new c2request instance
                C2ReadFile::Ptr c2ReadPtr = new C2ReadFile(this, _log, requestParams, readBufs[0]->reqId());
                _log(ZQ::common::Log::L_INFO, CLOGFMT(ReadClient, "read() [%p] create a new request instance, instance id[%d], buffer number[%ld] buffer size[%ld] bufReqId[%ld] first buffer offset[%ld], key[%s] mainFileTimeout[%d] bitrate[%ld/%ld]"),
						this, c2ReadPtr->getInstanceID(), readBufs.size(), readBufs[0]->bufSize(), c2ReadPtr->getBufferReqID(), readBufs[0]->offsetInFile(), key.c_str(), requestParams.mainfileTimeout, origBitrate, reqBitrate ) ;
                _c2RequestMaps[newKey] = c2ReadPtr;

                timespec startTime, endTime;
                clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &startTime);
                if (!c2ReadPtr->addBuffer(readBufs))
                {
                    _log(ZQ::common::Log::L_ERROR, CLOGFMT(ReadClient, "read() [%p]failed file[%s] instance id[%d], bufReqId[%ld]"), this, newKey.c_str(), c2ReadPtr->getInstanceID(), c2ReadPtr->getBufferReqID());
                    return false;
                }
                clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &endTime);
                int64 spec = endTime.tv_sec*1000*1000*1000 + endTime.tv_nsec - (startTime.tv_sec*1000*1000*1000 + startTime.tv_nsec);
                _log(ZQ::common::Log::L_DEBUG, CLOGFMT(ReadClient, "read() [%p]addBuffer use time[%ld]ns, instance id[%d], bufReqId[%ld]"), this, spec, c2ReadPtr->getInstanceID(), c2ReadPtr->getBufferReqID());
            }

            return true;
        }

        // TODO: add interface for query index file attributes
        bool ReadClient::queryIndexInfo( const std::string& filename, C2Streamer::AssetAttribute::Ptr attr )
        {
            RequestParams requestParams =  _params;
            requestParams.filename = filename;

            C2QueryIndex::Ptr c2queryPtr = new C2QueryIndex(this, _log, requestParams, attr);

            _log(ZQ::common::Log::L_INFO, CLOGFMT(ReadClient, "queryIndexInfo() [%p] create a new request instance, instance id[%d], bufReqId[%ld]"), this, c2queryPtr->getInstanceID(), c2queryPtr->getReqID());
            return c2queryPtr->startRequest();
        }

        void ReadClient::onIndexInfo( C2Streamer::AssetAttribute::Ptr attr )
        {
            _log(ZQ::common::Log::L_DEBUG, CLOGFMT(ReadClient, "onIndexInfo() entry, bufReqId[%ld]"), attr->reqId());
            _readerCB->onIndexInfo(attr);
        }

        void ReadClient::onRead( const std::vector<C2Streamer::BufferUser>& bufs)
        {
            if (!bufs.empty())
            {
                _log(ZQ::common::Log::L_DEBUG, CLOGFMT(ReadClient, "onRead() entry, buffer number[%ld] bufReqId[%ld]"), bufs.size(), bufs[0]->reqId());
                _readerCB->onRead(bufs);
            }
        }

        void ReadClient::onReadComplete( const std::string& key)
        {
            ZQ::common::MutexGuard gd(_mutex);
            if (_c2RequestMaps.find(key) != _c2RequestMaps.end())
            {
                std::map<std::string, C2ReadFile::Ptr>::iterator it = _c2RequestMaps.find(key);
                C2ReadFile::Ptr c2ReadPtr = it->second;
                _log(ZQ::common::Log::L_DEBUG, CLOGFMT(ReadClient, "found it [%d], bufReqId[%ld] prepare to delete this instance"), c2ReadPtr->getInstanceID(), c2ReadPtr->getBufferReqID());

                std::string filename = key.substr(0, key.find_last_of(':'));
                _log(ZQ::common::Log::L_INFO, CLOGFMT(ReadClient, "[%p] onReadComplete() read complete, file[%s] bufReqId[%ld]"), c2ReadPtr._ptr, filename.c_str(), c2ReadPtr->getBufferReqID());
                c2ReadPtr = NULL;
                {
                    _c2RequestMaps.erase(key);
                }
            }
        }

        void ReadClient::onLatency(std::string& fileName, int64 offset, int64 time)
        {
            ZQ::common::MutexGuard gd(_mutex);
            _readerCB->onLatency(fileName, offset, time);
        }

        void ReadClient::onError( int err )
        {
            ZQ::common::MutexGuard gd(_mutex);
            _readerCB->onError(err);
        }

    }//namespace ZQ
} //namespace StreamService
