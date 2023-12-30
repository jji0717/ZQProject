

#include "PushSource.h"
#include "IPushTrigger.h"
#include "Log.h"

#define PushSrc			"PushSrc"

using namespace ZQ::common;

#define MOLOG (*_pLog)


namespace ZQTianShan {
	namespace ContentProvision {


PushSource::PushSource()
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
	_nBandwidthBps = 0;
	_pPushSess = NULL;
}

bool PushSource::Init()
{
	_bitrateCtrl.setBitrate(_nBandwidthBps);
	_bitrateCtrl.start();

	MOLOG(Log::L_INFO, CLOGFMT(PushSrc, "[%s] initialized"), _strLogHint.c_str());
	return true;
}

void PushSource::Stop()
{
	BaseFilter::Stop();
	MOLOG(Log::L_INFO, CLOGFMT(PushSrc, "[%s] Stop() called"), _strLogHint.c_str());
}

void PushSource::Close()
{
	if (_pPushSess)
	{
		_pPushSess->close(true);
		_pPushSess = NULL;

		// set the total bytes to processed bytes
		GetGraph()->setTotalBytes(_llProcBytes);
	}
}

void PushSource::endOfStream()
{

}

const char* PushSource::GetName()
{
	return SOURCE_TYPE_PUSHSRC;
}

LONGLONG PushSource::getProcessBytes()
{
	return _llProcBytes;
}

MediaSample* PushSource::GetData(int nOutputIndex)
{
	if (nOutputIndex != 0)
		return NULL;

	if (!_pPushSess)
	{
		// set the total bytes to processed bytes
		GetGraph()->setTotalBytes(_llProcBytes);
		return NULL;
	}
	
	if (_bStop)
	{
		_pPushSess->close(true);
		_pPushSess = NULL;
		MOLOG(Log::L_INFO, CLOGFMT(PushSrc, "[%s] require to stop while GetData()"), _strLogHint.c_str());
		
		// set the total bytes to processed bytes
		GetGraph()->setTotalBytes(_llProcBytes);
		return NULL;
	}
	
	MediaSample* pSample = GetGraph()->allocMediaSample();
	if (!pSample)
	{
		_pPushSess->close(false, "failed to allocate memory");
		_pPushSess = NULL;
		MOLOG(Log::L_ERROR, CLOGFMT(PushSrc, "[%s] failed to alloc meida sample"), _strLogHint.c_str());

		// set the total bytes to processed bytes
		GetGraph()->setTotalBytes(_llProcBytes);

		return NULL;
	}
	
	unsigned int nRead = _pPushSess->read(pSample->getPointer(), pSample->getBufSize());
	if (!nRead)
	{
		//no more data
		GetGraph()->freeMediaSample(pSample);
		_pPushSess->close(true);
		_pPushSess = NULL;
		MOLOG(Log::L_INFO, CLOGFMT(PushSrc, "[%s] read return 0 bytes, no more data available"), _strLogHint.c_str());

		// set the total bytes to processed bytes
		GetGraph()->setTotalBytes(_llProcBytes);
		return NULL;
	}

	pSample->setDataLength(nRead);
	pSample->setOffset(_llProcBytes);

	_llProcBytes += nRead;

	//bandwidth control
	uint32 nRet = _bitrateCtrl.control(_llProcBytes);
	if (nRet > 1000)
	{
		MOLOG(Log::L_DEBUG, CLOGFMT(NTFSSrc, "[%s] bitrate control waiting time %dms"), _strLogHint.c_str(), nRet);
	}

	return pSample;
}

void PushSource::setPushSrcI(IPushSource* pPushI)
{
	_pPushSess = pPushI;
}

void PushSource::setMaxBandwidth(unsigned int nBandwidthBps)
{
	_nBandwidthBps = nBandwidthBps;
	_bitrateCtrl.setBitrate(nBandwidthBps);
	MOLOG(Log::L_INFO, CLOGFMT(PushSrc, "[%s] set MaxBandwidth=%d bps"), _strLogHint.c_str(), nBandwidthBps);
}

void PushSource::setBandwidthCtrlInterval(int nIntervalMs)
{
	_bitrateCtrl.setInterval(nIntervalMs);
	MOLOG(Log::L_INFO, CLOGFMT(PushSrc, "[%s] set BandwidthControlInterval=%d ms"), _strLogHint.c_str(), nIntervalMs);
}
bool PushSource::readbuf(char* pBuf, unsigned int bufLen, unsigned int& rcvLen)
{
	return false;
}
bool PushSource::seek(int64 offset, int pos)
{
	return false;
}	
}}