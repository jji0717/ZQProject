#ifndef _RTIRAW_SOURCE_H
#define _RTIRAW_SOURCE_H
#pragma once
#include "BaseClass.h"
#include <deque>
#include "Locks.h"
#include "SystemUtils.h"
#include  "MCastCapture.h"

#define	 SOURCE_TYPE_RTIRAW		"RTIRawSource"

namespace ZQTianShan {
	namespace ContentProvision {
		class RTIRawSource : public BaseSource, protected MediaSamplePool
		{
		friend class SourceFactory;
		friend class MCastCapture;

		public:
			RTIRawSource();
			virtual ~RTIRawSource(void);
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

			//void setDelayDataNotify(int nDelayMilliseconds);

			virtual bool readbuf(char* pBuf, unsigned int bufLen, unsigned int& rcvLen);
			virtual bool seek(int64 offset, int pos);

			void setFilter(BaseFilter* pTarget){_targetFilter = pTarget;}
			void setScheduleTime(int64 time){_scheduleTime = time;}
			bool getQuit(){return _quit;}
//			void setPool(ZQ::common::NativeThreadPool& pool){_pool = &pool;}
		protected:
			virtual MediaSample* acquireOutputBuffer();
			virtual void releaseOutputBuffer(MediaSample* pSample);

			// return false if no sample return and end of data, which means the capture stopped, and not more data in the queue.
			// else return true
			inline bool getSample(MediaSample*& pSample);
			bool Run();

			std::deque<MediaSample*>	_captured;
			ZQ::common::Mutex			_lock;
			SYS::SingleObject				_hDataNotify;
			int								_dwNotifyStamp;
			bool							_bIsEndOfData;
			int64							 _offset;
			std::string						 _localBindIp;
			std::string						 _multiCastIp;
			int								 _multiCastPort;
			uint32							_timeoutInterval;
			bool							 _quit;

			int64							_scheduleTime;
			int64							_startGetTime;
			int64							_endGetTime;

			MCastCapture*			_pCapture;
			BaseFilter* _targetFilter;
//			ZQ::common::NativeThreadPool*  _pool;
		};


	}}//namespace
#endif