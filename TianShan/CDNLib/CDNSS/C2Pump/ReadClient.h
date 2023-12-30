#ifndef READ_CLIENT_H
#define READ_CLIENT_H
#include "C2Request.h"

namespace ZQ{
namespace StreamService{
    class ReadClient : public C2Streamer::IDataReader, public IReadCB
    {
    public:
        typedef ZQ::common::Pointer<ReadClient> Ptr;
        ReadClient(C2Streamer::IReaderCallback* readerCB, ZQ::common::Log& log, RequestParams& params);
        virtual ~ReadClient();
	RequestParams getParams(){return _params;}

    public:
        virtual bool read( const std::vector<C2Streamer::BufferUser>& bufs );
        virtual bool queryIndexInfo( const std::string& filename, C2Streamer::AssetAttribute::Ptr attr );

    public:
        virtual void onRead( const std::vector<C2Streamer::BufferUser>& bufs );
        virtual void onReadComplete( const std::string& key);
        virtual void onIndexInfo( C2Streamer::AssetAttribute::Ptr attr );
        virtual void onLatency(std::string& fileName, int64 offset, int64 time); 
        virtual void onError( int err );

    private:
        ZQ::common::Log&            _log;
        RequestParams               _params;
        ZQ::common::Mutex           _mutex;

        C2Streamer::IReaderCallback*                _readerCB;
        std::map<std::string, C2ReadFile::Ptr>      _c2RequestMaps;

    };//class ReadClient
}//namespace ZQ
} //namespace StreamService
#endif
