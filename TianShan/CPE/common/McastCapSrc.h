

#ifndef _MULTICAST_CAPTURE_SOURCE_FILTER_
#define _MULTICAST_CAPTURE_SOURCE_FILTER_

#include <deque>
#include "BaseClass.h"
#include  "MCastCapture.h"
#include "Locks.h"
#include "SystemUtils.h"


#define _ENABLE_DUMP_MPEG_FILE_

#ifdef _ENABLE_DUMP_MPEG_FILE_	
#include "StreamDataDumper.h"
#endif


#define	SOURCE_TYPE_MCASTCAPSRC	"McastCapSrc"


namespace ZQTianShan {
	namespace ContentProvision {
		
		class McastCapSource: public BaseSource, protected MediaSamplePool
		{
		protected:
			friend class SourceFactory;
			friend class MCastCapture;

			McastCapSource();
			~McastCapSource();
			
		public:
			virtual bool Init();
			virtual bool Start();
			
			virtual void Stop();
			
			virtual void Close();
			
			virtual void endOfStream();
			
			virtual const char* GetName();
			
			virtual int64 getProcessBytes();
			
			virtual MediaSample* GetData(int nOutputIndex = 0);

			bool setInspectPara(const std::string& multicastIp, int multicastPort, uint32 timeoutInterval, const std::string& localIp);

			void setDelayDataNotify(int nDelayMilliseconds);

			virtual bool readbuf(char* pBuf, unsigned int bufLen, unsigned int& rcvLen);
			virtual bool seek(int64 offset, int pos);

#ifdef _ENABLE_DUMP_MPEG_FILE_	

			// for dump data
			void enableDump(bool bEnable=true);
			void setDumpPath(const char* szPath);
			void deleteDumpOnSuccess(bool bDelOnSucc=true);
#endif
			
		protected:
			virtual MediaSample* acquireOutputBuffer();
			virtual void releaseOutputBuffer(MediaSample* pSample);

			// return false if no sample return and end of data, which means the capture stopped, and not more data in the queue.
			// else return true
			inline bool getSample(MediaSample*& pSample);
			
			std::deque<MediaSample*>	_captured;
			ZQ::common::Mutex			_lock;

			SYS::SingleObject			_hDataNotify;
			int					_dwNotifyStamp;
			int							_nDelayMilliseconds;	//

			bool				_bIsEndOfData;
			int64            _offset;

			std::string         _localBindIp;
			std::string         _multiCastIp;
			int                 _multiCastPort;

			uint32  _timeoutInterval;
			bool _quit;

#ifdef _ENABLE_DUMP_MPEG_FILE_	
			StreamDataDumper		_dumper;
#endif

			MCastCapture*			_pCapture;
		};
		
		
}}

#endif
