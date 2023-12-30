#include "RTIRawSource.h"
#include "assert.h"
#include "ErrorCode.h"
#include "BaseClass.h"
#include "TimeUtil.h"


#define MOLOG (*_pLog)

namespace ZQTianShan {
	namespace ContentProvision {
		RTIRawSource::RTIRawSource()
		{
			_llProcBytes = 0;
			_bDriverModule = true;
			_offset = 0;
			_bIsEndOfData = false;
			_quit = false;
			//_nDelayMilliseconds = 0;		//default to disable it, because the test result is not 
			_dwNotifyStamp = 0;
			_timeoutInterval = 30000;
			_pCapture = NULL;
			_scheduleTime = 0;
			_startGetTime = 0;
			_endGetTime = 0;
		}

		RTIRawSource::~RTIRawSource(void)
		{
			Close();
		}
		bool RTIRawSource::Init()
		{
			MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(RTIRawSource, "[%s] init()"), _strLogHint.c_str());
			_pCapture = new MCastCapture();
			_pCapture->setLog(_pLog);
			_pCapture->setMediaSamplePool(this);
			if(!_pCapture->open(_localBindIp, _multiCastIp, _multiCastPort)) 
			{
				std::string errmsg = _pCapture->getLastError();
				SetLastError(std::string("RTIRawSource: open wincap failed with error: ") + errmsg, ERRCODE_PARAMS_MISSING);
				MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(RTIRawSource, "[%s] failed to open wincap, localIp is %s, multicastIp is %s, mcport is %d, error msg:%s"), 
					_strLogHint.c_str(), _localBindIp.c_str(), _multiCastIp.c_str(), _multiCastPort, errmsg.c_str());
				return false;
			}	
			_dwNotifyStamp = SYS::getTickCount();
			MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(RTIRawSource, "[%s] initialized at ip[%s] port[%d]"), _strLogHint.c_str(), _multiCastIp.c_str(), _multiCastPort);
			return true;
		}
		bool RTIRawSource::Start()
		{
			assert(_pCapture!=0);
			if (!_pCapture)
				return false;
			_pCapture->start();
			//(new RTIRawProc(this,*_pool))->execute();
			return true;
		}
		void RTIRawSource::Stop()
		{
			if (!_pCapture)
				return;
			MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(RTIRawSource, "[%s] Stop() called"), _strLogHint.c_str());
			_pCapture->stop();
			_quit = true;
			endOfStream();
		}
		void RTIRawSource::Close()
		{
			if (!_pCapture)
				return;
			MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(RTIRawSource, "[%s] Close() enter"), _strLogHint.c_str());		
			_pCapture->stop();
			_pCapture->close();
			delete _pCapture;
			_pCapture = NULL;
			{
				ZQ::common::Guard<ZQ::common::Mutex> op(_lock);
				if (_captured.size())
				{
					MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(RTIRawSource, "[%s] there is still [%d] captured data sample not processed"), _strLogHint.c_str(), _captured.size());
					while(_captured.size()>0)
					{
						MediaSample* pSample = _captured.front();		
						_captured.pop_front();
						GetGraph()->freeMediaSample(pSample);
					}
				}
			}
			//_targetFilter->Close();
			MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(RTIRawSource, "[%s] Closed"), _strLogHint.c_str());		
		}
		void RTIRawSource::endOfStream()
		{
			MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(RTIRawSource, "[%s] endOfStream() called"), _strLogHint.c_str());
			_bIsEndOfData = true;
			_hDataNotify.signal();
		}
		const char* RTIRawSource::GetName()
		{
			return SOURCE_TYPE_RTIRAW;
		}
		bool RTIRawSource::getSample(MediaSample*& pSample)
		{	
			ZQ::common::Guard<ZQ::common::Mutex> op(_lock);
			if (_captured.size()>0)
			{
				pSample = _captured.front();		
				_captured.pop_front();
				return true;
			}
			pSample = NULL;
			return !_bIsEndOfData;
		}

		MediaSample* RTIRawSource::GetData(int nOutputIndex)
		{
			MediaSample* pSample;
			bool bError = false;
			while(getSample(pSample)&&!bError)
			{
				if (pSample)
				{					
					_llProcBytes += pSample->getDataLength();
					if(!_targetFilter->Receive(pSample,nOutputIndex))
					{
						GetGraph()->freeMediaSample(pSample);
						pSample = NULL;
					}
					return pSample;
				}

				//MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(RTIRawSource, "[%s] wait begin()"), _strLogHint.c_str());
				int64 dwWait = SYS::getTickCount();
				do
				{
					SYS::SingleObject::STATE dwRet = _hDataNotify.wait(1000);

					if (dwRet == SYS::SingleObject::SIGNALED)
						break;
					if (dwRet != SYS::SingleObject::TIMEDOUT)
					{
						bError = true;
						GetGraph()->SetLastError("capturing buffer event wait error" , 0);
						break;
					}
					if (SYS::getTickCount()-dwWait>=_timeoutInterval)
					{
						//MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(RTIRawSource, "[%s] wait end took %dms"), _strLogHint.c_str(), SYS::getTickCount() - dwWait);
						bError = true;
						GetGraph()->SetLastError("capturing multicast stream time out" , 0);
						break;
					}			
				}while(1);
			};
			MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(RTIRawSource, "[%s] capturing multicast stoped"), _strLogHint.c_str());
			return NULL;
		}
		bool RTIRawSource::setInspectPara(const std::string& multicastIp, int multicastPort,uint32 timeoutInterval,const std::string& localIp)
		{
			MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(RTIRawSource, "[%s] setInspectParam() mcastip[%s] port[%d] localip[%s] timeout[%d]s"),
				_strLogHint.c_str(), multicastIp.c_str(), multicastPort, localIp.c_str(), timeoutInterval);
			if (multicastIp.size() != 0 && multicastPort != 0)
			{
				_multiCastIp = multicastIp;
				_multiCastPort = multicastPort;
				_timeoutInterval = timeoutInterval*1000;
				if (_timeoutInterval == 0)
					_timeoutInterval = 30*1000;
				_localBindIp = localIp;
				return true;
			}
			else
				return false;	
		}
// 		void RTIRawSource::setDelayDataNotify( int nDelayMilliseconds )
// 		{
// 			_nDelayMilliseconds = nDelayMilliseconds;
// 			MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(RTIRawSource, "[%s] setDelayDataNotify() to %d ms"), _strLogHint.c_str(), nDelayMilliseconds);
// 		}
		MediaSample* RTIRawSource::acquireOutputBuffer()
		{
			MediaSample* pSample = GetGraph()->allocMediaSample();
			if (!pSample)
			{
				MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(RTIRawSource, "[%s] acquireOutputBuffer() failed to allocate mediasample"), _strLogHint.c_str());		
				SetLastError(std::string("RTIRawSource: failed to allocate mediasample"), ERRCODE_BUFFERQUEUE_FULL);
			}
			return pSample;
		}
		void RTIRawSource::releaseOutputBuffer( MediaSample* pSample )
		{
			//static int count = 0; 
			if (!pSample)
			{
				MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(RTIRawSource, "[%s] releaseOutputBuffer() with NULL pointer"), _strLogHint.c_str());
				return;
			}
			if (pSample->getDataLength())
			{
				pSample->setOffset(_offset);
				_offset += pSample->getDataLength();
				_lock.enter();
				_captured.push_back(pSample);
				_lock.leave();

				//count++;
				//MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(RTIRawSource, "[%s] releaseOutputBuffer() ************* count[%d] DataNotify"), _strLogHint.c_str(), count);		

				_hDataNotify.signal();
			}
			else
			{
				MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(RTIRawSource, "[%s] In releaseOutputBuffer(), the dataLen parameter is 0"), _strLogHint.c_str());		
				GetGraph()->freeMediaSample(pSample);
			}
		}
		bool RTIRawSource::Run()
		{
/*			MediaInfo mInfo;
			memset(&mInfo, 0 , sizeof(mInfo));
			GetGraph()->OnMediaInfoParsed(mInfo);
*/
			MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(RTIRawSource, "[%s] RTIRawSource Run enter()"), _strLogHint.c_str());

			while (!_quit)
			{
				if (GetGraph()->IsErrorOccurred())
				{
					MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(RTIRawSource, "[%s] capture the data with error[%s]"), _strLogHint.c_str(), GetGraph()->GetLastError().c_str());
					return false;
				}

				MediaSample* pSample = GetData(0);
				if (!pSample)
					break;
				if (_startGetTime == 0)
				{
					_startGetTime = ZQ::common::TimeUtil::now();
				}
			}
			_endGetTime = ZQ::common::TimeUtil::now();
			int64 dataGetTime = 0;
			if (_startGetTime != 0)
				dataGetTime = _endGetTime - _startGetTime;
			if (dataGetTime < 5000 || _endGetTime < _scheduleTime)
			{
				MOLOG(ZQ::common::Log::L_ERROR,CLOGFMT(RTIRawSource,"[%s]fail to capture the data with error[%s] ,captureTime[%d]s"),
					_strLogHint.c_str(), GetGraph()->GetLastError().c_str(), (int)(dataGetTime/1000));
				return false;
			}
			return true;
		}

		bool RTIRawSource::readbuf(char* pBuf, unsigned int bufLen, unsigned int& rcvLen)
		{
			return false;
		}
		bool RTIRawSource::seek(int64 offset, int pos)
		{
			return false;
		}
		int64 RTIRawSource::getProcessBytes()
		{
			return _llProcBytes;
		}
	}}//namespace
