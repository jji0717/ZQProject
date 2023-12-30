

#include "McastCapSrc.h"
#include "Log.h"
#include "ErrorCode.h"
#include "urlstr.h"
#include "assert.h"
#include "SystemUtils.h"


//if this macro defined, the stream dump will be in capture thread
//if not, dump will be in trick generation thread
//the diffrent is:
//1. if disk io is not quick enough, when in capture thread, maybe it would infect the stream capture
//2. dump in capture thread would get more data
#define DUMP_IN_CAPTURE_THREAD


#define McastCap			"McastCap"

using namespace ZQ::common;

#define MOLOG (*_pLog)

#ifdef ZQ_OS_MSWIN
#pragma comment(lib, "ws2_32.lib")
#endif

namespace ZQTianShan {
	namespace ContentProvision {


McastCapSource::McastCapSource()	
{
	_nOutputCount = 1;

	int i;
	for(i=0;i<_nInputCount;i++)
	{
		InputPin pin;
		pin.nPrevPin = 0;
		pin.pPrevFilter = 0;		
		_inputPin.push_back(pin);
	}
	for(i=0;i<_nOutputCount;i++)
	{
		OutputPin pin;
		pin.nNextPin = 0;
		pin.pNextFilter = 0;		
		_outputPin.push_back(pin);
	}

	_llProcBytes = 0;
	_bDriverModule = false;
	_offset = 0;
	_bIsEndOfData = false;
    _quit = false;

	_nDelayMilliseconds = 0;		//default to disable it, because the test result is not 
	_dwNotifyStamp = 0;
	_timeoutInterval = 30000;

	_pCapture = NULL;
}

McastCapSource::~McastCapSource()
{	
	Close();
}

bool McastCapSource::Init()
{
	MOLOG(Log::L_INFO, CLOGFMT(McastCap, "[%s] init()"), _strLogHint.c_str());

	_pCapture = new MCastCapture();
	_pCapture->setLog(_pLog);
	_pCapture->setMediaSamplePool(this);
	if(!_pCapture->open(_localBindIp, _multiCastIp, _multiCastPort)) 
	{
		std::string errmsg = _pCapture->getLastError();
		SetLastError(std::string("McastCap: open wincap failed with error: ") + errmsg, ERRCODE_PARAMS_MISSING);

		MOLOG(Log::L_ERROR, CLOGFMT(McastCap, "[%s] failed to open wincap, localIp is %s, multicastIp is %s, mcport is %d, error msg:%s"), 
			_strLogHint.c_str(), _localBindIp.c_str(), _multiCastIp.c_str(), _multiCastPort, errmsg.c_str());
		return false;
	}	
	MOLOG(Log::L_INFO, CLOGFMT(McastCap, "[%s] multicast ip[%s] port[%d]"), _strLogHint.c_str(), _multiCastIp.c_str(), _multiCastPort);

#ifdef _ENABLE_DUMP_MPEG_FILE_
	//just use the log hint as dump file name
	_dumper.setFile(_strLogHint.c_str());
	if (!_dumper.init())
	{
		MOLOG(Log::L_WARNING, CLOGFMT(McastCap, "[%s] failed to initialize capture dumper, no dump will be generated"), _strLogHint.c_str());
	}
#endif

	_dwNotifyStamp = SYS::getTickCount();

	MOLOG(Log::L_INFO, CLOGFMT(McastCap, "[%s] initialized"), _strLogHint.c_str());
	return true;
}

bool McastCapSource::Start()
{
	assert(_pCapture!=0);
	if (!_pCapture)
		return false;

	_pCapture->start();
	return true;
}


void McastCapSource::Stop()
{
	if (!_pCapture)
		return;

	MOLOG(Log::L_INFO, CLOGFMT(McastCap, "[%s] Stop() called"), _strLogHint.c_str());
  	_pCapture->stop();
	endOfStream();
}

void McastCapSource::Close()
{
	if (!_pCapture)
		return;

	MOLOG(Log::L_INFO, CLOGFMT(McastCap, "[%s] Close() enter"), _strLogHint.c_str());		

	_pCapture->stop();
	_pCapture->close();
	delete _pCapture;
	_pCapture = NULL;

	{
		ZQ::common::Guard<ZQ::common::Mutex> op(_lock);
		if (_captured.size())
		{
			MOLOG(Log::L_WARNING, CLOGFMT(McastCap, "[%s] there is still [%d] captured data sample not processed"), _strLogHint.c_str(), _captured.size());

			while(_captured.size()>0)
			{
				MediaSample* pSample = _captured.front();		
				_captured.pop_front();
				GetGraph()->freeMediaSample(pSample);
			}
		}
	}	

#ifdef _ENABLE_DUMP_MPEG_FILE_
	_dumper.close(!GetGraph()->IsErrorOccurred());
#endif

	MOLOG(Log::L_INFO, CLOGFMT(McastCap, "[%s] Closed"), _strLogHint.c_str());		
}

void McastCapSource::endOfStream()
{
	MOLOG(Log::L_INFO, CLOGFMT(McastCap, "[%s] endOfStream() called"), _strLogHint.c_str());
	
	_bIsEndOfData = true;
    _hDataNotify.signal();
}

const char* McastCapSource::GetName()
{
	return SOURCE_TYPE_MCASTCAPSRC;
}

int64 McastCapSource::getProcessBytes()
{
	return _llProcBytes;
}

bool McastCapSource::getSample(MediaSample*& pSample)
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

MediaSample* McastCapSource::GetData(int nOutputIndex)
{
	MediaSample* pSample;
	bool bError = false;

	while(getSample(pSample)&&!bError)
	{
		if (pSample)
		{		
#ifdef _ENABLE_DUMP_MPEG_FILE_
			_dumper.dump((char*)pSample->getPointer(), pSample->getDataLength());
#endif
			return pSample;
		}

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
				bError = true;
				GetGraph()->SetLastError("capturing multicast stream time out" , 0);
				break;
			}			
		}while(1);
	};

	MOLOG(Log::L_INFO, CLOGFMT(McastCap, "[%s] Get the end of data"), _strLogHint.c_str());
	return NULL;
}

bool McastCapSource::setInspectPara(const std::string& multicastIp, int multicastPort,uint32 timeoutInterval,const std::string& localIp)
{
	MOLOG(Log::L_INFO, CLOGFMT(McastCap, "[%s] setInspectParam() mcastip[%s] port[%d] localip[%s] timeout[%d]s"),
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

void McastCapSource::setDelayDataNotify( int nDelayMilliseconds )
{
	_nDelayMilliseconds = nDelayMilliseconds;

	MOLOG(Log::L_INFO, CLOGFMT(McastCap, "[%s] setDelayDataNotify() to %d ms"), _strLogHint.c_str(), nDelayMilliseconds);
}

MediaSample* McastCapSource::acquireOutputBuffer()
{
	MediaSample* pSample = GetGraph()->allocMediaSample();
	if (!pSample)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(McastCap, "[%s] acquireOutputBuffer() failed to allocate mediasample"), _strLogHint.c_str());		
		SetLastError(std::string("McastCap: failed to allocate mediasample"), ERRCODE_BUFFERQUEUE_FULL);
	}

	return pSample;
}

void McastCapSource::releaseOutputBuffer( MediaSample* pSample )
{
		if (!pSample)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(McastCap, "[%s] releaseOutputBuffer() with NULL pointer"), _strLogHint.c_str());
		return;
	}
	if (pSample->getDataLength())
	{
		pSample->setOffset(_offset);
		_offset += pSample->getDataLength();

		_lock.enter();
		_captured.push_back(pSample);
		_lock.leave();

		_hDataNotify.signal();
	}
	else
	{
		MOLOG(Log::L_INFO, CLOGFMT(McastCap, "[%s] In releaseOutputBuffer(), the dataLen parameter is 0"), _strLogHint.c_str());		
		GetGraph()->freeMediaSample(pSample);
	}
}

//if (_nDelayMilliseconds)
//{
//	if (GetTickCount() - _dwNotifyStamp>= (DWORD)_nDelayMilliseconds)
//	{		
//		SetEvent(_hDataNotify);		
//		_dwNotifyStamp = GetTickCount();
//		//			MOLOG(Log::L_DEBUG, CLOGFMT(McastCap, "[%s] SetEvent(_hDataNotify)"), _strLogHint.c_str());
//	}
//}
//else
//{
//	SetEvent(_hDataNotify);		
//}



#ifdef _ENABLE_DUMP_MPEG_FILE_	

void McastCapSource::enableDump(bool bEnable)
{
	_dumper.enable(bEnable);
}

void McastCapSource::setDumpPath(const char* szPath)
{
	_dumper.setPath(szPath);
}

void McastCapSource::deleteDumpOnSuccess(bool bDelOnSucc)
{
	_dumper.deleteOnSuccess(bDelOnSucc);
}
bool McastCapSource::readbuf(char* pBuf, unsigned int bufLen, unsigned int& rcvLen)
{
	return false;
}
bool McastCapSource::seek(int64 offset, int pos)
{
	return false;
}
#endif


}}
