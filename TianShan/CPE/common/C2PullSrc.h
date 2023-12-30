#ifndef _C2PULL_CAPTURE_SOURCE_FILTER_
#define _C2PULL_CAPTURE_SOURCE_FILTER_

#include <deque>
#include "BaseClass.h"
#include "Locks.h"
#include "SystemUtils.h"
#include "HTTPClientFactory.h"

#define _ENABLE_DUMP_MPEG_FILE_

#ifdef _ENABLE_DUMP_MPEG_FILE_	
#include "StreamDataDumper.h"
#endif


#define	SOURCE_TYPE_C2PULL	"C2PullSrc"

namespace ZQTianShan {
	namespace ContentProvision {

		typedef struct 
		{
			std::string filename;
			int64 firstOffset;
			int64 finalOffset;
			int64 totalFilesize;
			bool  bIsSparseFile;
		}MainFileInfo;

		class MediaSample;

		class MediaSamplePool
		{
		public:
			virtual ~MediaSamplePool() {}
			virtual MediaSample* acquireOutputBuffer() = 0;
			virtual void releaseOutputBuffer(MediaSample* pSample) = 0;
		};

		class C2PullSource: public BaseSource, protected MediaSamplePool, public ZQ::common::NativeThread
		{
		protected:
			friend class SourceFactory;
//			friend class MCastCapture;

			C2PullSource();
			~C2PullSource();
		public:
			int run(void);
		public:
			virtual bool Init();
            virtual bool Start();
			virtual void Stop();

			virtual void Close();

			virtual void endOfStream();

			virtual const char* GetName();

			virtual int64 getProcessBytes();

			virtual MediaSample* GetData(int nOutputIndex = 0);

			virtual bool readbuf(char* pBuf, unsigned int bufLen, unsigned int& rcvLen);
			virtual bool seek(int64 offset, int pos);

		public:
			void setFilename(const char* szFilename){_strFilename = szFilename;};
			void setCacheDir(const char* szCachDir);
			void setURL(const char* szURL){_srcURL=szURL;}
	        void setIngressCapcaity(int IngressCapcaity){_IngressCapcaity  = IngressCapcaity;}
			void setPIDAndPAID(std::string PID, std::string PAID){_strProvideId = PID; _strProvideAssetId = PAID;}
			void setLocateFileIP(std::string bindIP, std::string transferip){_bindIP = bindIP; _transferip = transferip;};
			void setTransferServerPort(int nport){_transferServerPort = nport;};
			void setSpeed(int nspeed){_nspeed = nspeed;};
			void setTransferDelay(int transferdelay){_transferDelay = transferdelay;};
			void setTimeout(int timeout){_timeout = timeout;};
			void setMaxBandwidth(unsigned int nBandwidthBps);

			void setHttpClientFac(HTTPClientFactory*_pHttpClientFactory){__pHttpClientFactory = _pHttpClientFactory;};

			static bool processData(void* pCtx, char*pBuf, int64 len);
		protected:
			std::string                   _tempDir;
			std::string		              _srcURL;
			std::string					  _strServer;
			std::string                   _strPort;
			std::string					  _strProvideAssetId;
			std::string                   _strProvideId;
			std::string                   _bindIP;
			std::string                   _transferip;
			int                           _transferServerPort;
			int64                         _IngressCapcaity;
			int                           _nspeed; 
			int                           _transferDelay;
			int                           _timeout;

			unsigned int				  _nBandwidthBps;

			std::string                   _strFilename;		//target file name 
			std::string                   _strTempIndexFile;

			MainFileInfo                  _mainFileset;
            MediaInfo					  _info;
			std::auto_ptr<ZQTianShan::ContentProvision::C2HttpClient>	_pHttpDownloader;
			HTTPClientFactory*			  __pHttpClientFactory;

			MediaSample*		          _pMediaSample;

			bool                          _bProcessDataError;
		protected:
			virtual MediaSample* acquireOutputBuffer();
			virtual void releaseOutputBuffer(MediaSample* pSample);

			// return false if no sample return and end of data, which means the capture stopped, and not more data in the queue.
			// else return true
			inline bool getSample(MediaSample*& pSample);

			std::deque<MediaSample*>	_downloaded;
			ZQ::common::Mutex			_lock;

			bool				_bIsEndOfData;
			int64            _offset;

			bool _quit;

			SYS::SingleObject  _hDownLoadComplete;

		protected:
				bool downloadIndexFile();
				bool downloadMainFile();

		};


	}}

#endif
