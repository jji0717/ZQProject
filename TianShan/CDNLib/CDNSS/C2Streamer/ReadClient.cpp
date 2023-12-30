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

        bool ReadClient::read( const std::vector<C2Streamer::Buffer*>& bufs )
        {
            _log(ZQ::common::Log::L_DEBUG, CLOGFMT(ReadClient, "read() entry, buffer number[%ld]"), bufs.size());

            if (0 == bufs.size())
            {
                _log(ZQ::common::Log::L_ERROR, CLOGFMT(ReadClient, "read() fail to read, there is no buffer for use"));
                return false;
            }

            // check offset
            for (std::vector<C2Streamer::Buffer*>::const_iterator it = bufs.begin(); it != bufs.end(); ++it)
            {
                std::string filename = (*it)->filename();
                std::ostringstream oss;
                oss<<filename<<":"<<(*it)->offsetInFile();
                std::string key = oss.str();

                std::ostringstream ossNew;
                ossNew<<filename<<":"<<(*it)->offsetInFile() + (*it)->bufSize();
                std::string newKey = ossNew.str();

                std::vector<C2Streamer::Buffer*> readBufs;
                readBufs.push_back(*it);

                ZQ::common::MutexGuard gd(_mutex);
                // one instance per buffer      
                RequestParams requestParams = _params;
                requestParams.filename = filename;

                C2ReadFile::Ptr c2ReadPtr = new C2ReadFile(this, _log, requestParams);
                _log(ZQ::common::Log::L_INFO, CLOGFMT(ReadClient, "[%p] create a new request instance, instance id[%d], buffer offset[%ld], key[%s]"), this, c2ReadPtr->getInstanceID(), (*it)->offsetInFile(), key.c_str());

                _c2RequestMaps[newKey] = c2ReadPtr;

                timespec startTime, endTime;
                clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &startTime);
                if (!c2ReadPtr->addBuffer(readBufs))
                {
                    _log(ZQ::common::Log::L_ERROR, CLOGFMT(ReadClient, "[%p] read() failed file[%s]"), this, newKey.c_str());
                    return false;
                }
                clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &endTime);
                int64 spec = endTime.tv_sec*1000*1000*1000 + endTime.tv_nsec - (startTime.tv_sec*1000*1000*1000 + startTime.tv_nsec);
                _log(ZQ::common::Log::L_DEBUG, CLOGFMT(ReadClient, "[%p] addBuffer use time[%ld]ns"), this, spec);
            }
            return true;
        }

        // TODO: add interface for query index file attributes
        bool ReadClient::queryIndexInfo( const std::string& filename, C2Streamer::AssetAttribute::Ptr attr )
        {
            RequestParams requestParams =  _params;
            requestParams.filename = filename;

            C2QueryIndex::Ptr c2queryPtr = new C2QueryIndex(this, _log, requestParams, attr);

            return c2queryPtr->startRequest();
        }

        void ReadClient::onIndexInfo( C2Streamer::AssetAttribute::Ptr attr )
        {
            _readerCB->onIndexInfo(attr);
        }

        void ReadClient::onRead( const std::vector<C2Streamer::Buffer*>& bufs)
        {
            _log(ZQ::common::Log::L_DEBUG, CLOGFMT(ReadClient, "onRead() entry"));
            if (!bufs.empty())
            {
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
                _log(ZQ::common::Log::L_DEBUG, CLOGFMT(ReadClient, "found it [%d], prepare to delete this instance"), c2ReadPtr->getInstanceID());

                std::string filename = key.substr(0, key.find_last_of(':'));
                _log(ZQ::common::Log::L_INFO, CLOGFMT(ReadClient, "[%p] onRead() read complete, file[%s]"), c2ReadPtr._ptr, filename.c_str());
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