#ifndef _C2_AIOFILE_WIN_H
#define _C2_AIOFILE_WIN_H

namespace C2Streamer
{
    class Buffer {
    public:
        Buffer();
        ~Buffer();

        enum ErrorCategory {
            ECATE_FILEIO    = 0,
            ECATE_HTTP      = 1,
            ECATE_SOCKET	= 2,
            ECATE_TIMEOUT   = 3, //server timed out while sending data
            ECATE_CLIENTTIMEOUT = 4 //timed out while waiting for new buffer
        };

        void 	setLastError( int error, ErrorCategory category ) { }
        void	setDataSize( size_t size ) { }
        char*	buffer() { return NULL;}
        size_t	bufSize( ) const { return 0;}
        uint64  offsetInFile( ) const { return 0; }
        const std::string& filename() const { return ""; }
        size_t  getDataSize() {return 0;}
        void	attach( const std::string& filename, int fd,  uint64 offsetInFile ){};
    };

    class AssetAttribute : public ZQ::common::SharedObject {
    public:
        enum LASTERR{
            ASSET_SUCC    = 0,
            ASSET_HTTP      = 1,
            ASSET_SOCKET	= 2,
            ASSET_TIMEOUT   = 3,
            ASSET_DATAERR = 4
        };

        typedef ZQ::common::Pointer<AssetAttribute>	Ptr;

        AssetAttribute()
            :mOpenForWrite(false),
            mRangeStart(-1),
            mRangeEnd(-1){              
        }

        void			lastError( int error ){}
        int				lastError() const { return mLastError; }
        bool			pwe( ) const { return mOpenForWrite; }
        void			pwe( bool bPwe ) { mOpenForWrite = bPwe; }
        void			range( int64 rangeStart, int64 rangeEnd) {}
        int64			rangeStart( ) const { return mRangeStart; }
        int64			rangeEnd( ) const { return mRangeEnd; }
        void			assetBaseInfo( const std::string& info ) { mAssetBaseInfo = info; }
        const std::string& assetBaseInfo() const { return mAssetBaseInfo; }
        void			assetMemberInfo( const std::string& info ) { mAssetMemberInfo = info; }
        const std::string& assetMemberInfo() const { return mAssetMemberInfo; }

    private:
        bool				mOpenForWrite;
        int64				mRangeStart;
        int64				mRangeEnd;
        std::string			mAssetBaseInfo;
        std::string 		mAssetMemberInfo;
        int64				mTimestamp;
        int					mLastError;
    };

    class IDataReader {
    public:
        virtual ~IDataReader( ) { }
        virtual bool read( const std::vector<Buffer*>& bufs ) = 0;
        virtual bool queryIndexInfo( const std::string& filename, C2Streamer::AssetAttribute::Ptr attr ) = 0;
    };

    class IReaderCallback {
    public:
        virtual ~IReaderCallback() { }
        virtual void onRead( const std::vector<Buffer*>& bufs ) = 0;
        virtual void onIndexInfo( C2Streamer::AssetAttribute::Ptr attr ) = 0;
        virtual void onError( int err ) = 0;
        virtual void onLatency(std::string& fileName, int64 offset, int64 time) = 0;
    };
}

#endif // _C2_AIOFILE_WIN_H