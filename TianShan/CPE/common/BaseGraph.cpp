

#include "BaseClass.h"
#include "QueueBufMgr.h"
#include "assert.h"

#define BaseGph			"BaseGph"

using namespace ZQ::common;

#define MOLOG (*_pLog)



namespace ZQTianShan {
	namespace ContentProvision {



BaseGraph::BaseGraph()
:_srcFilter(0), _driverFilter(0), _pAlloc(0),_nSampleSize(64*1024), _bStop(true),_nLastErrCode(0),
_bErrorOccurred(false),_bStreamable(false),_llProcBytes(0),_llTotalBytes(0)
{
	// ZQ::common::setGlogger();

	_nMaxAllocSampleCount = 400;	//each graph can not allocate more than this sample count

	//for MediaSample buffer
// 	_pQueMediaSample = new QueueBufMgr();
// 	_pQueMediaSample->setBufferSize(sizeof(MediaSample));
// 	_pQueMediaSample->setMaxSize(_nMaxAllocSampleCount);

	_pQueMediaSample = new ZQ::common::BufferPool();
	_pQueMediaSample->initialize(sizeof(MediaSample), _nMaxAllocSampleCount, 50);
	_bGraphClosed = false;
}

BaseGraph::BaseGraph(int maxAllocSampleCount)
:_srcFilter(0), _driverFilter(0), _pAlloc(0),_nSampleSize(64*1024), _bStop(true),_nLastErrCode(0),
_bErrorOccurred(false),_bStreamable(false),_llProcBytes(0),_llTotalBytes(0)
{
	// ZQ::common::setGlogger();

	_nMaxAllocSampleCount = maxAllocSampleCount;	//each graph can not allocate more than this sample count

	//for MediaSample buffer
// 	_pQueMediaSample = new QueueBufMgr();
// 	_pQueMediaSample->setBufferSize(sizeof(MediaSample));
// 	_pQueMediaSample->setMaxSize(_nMaxAllocSampleCount);

	_pQueMediaSample = new ZQ::common::BufferPool();
	_pQueMediaSample->initialize(sizeof(MediaSample), _nMaxAllocSampleCount, 50);
	_bGraphClosed = false;
}

BaseGraph::~BaseGraph()
{
	if (!_filters.size())
		return;
	
	std::vector<BaseFilter*>::iterator it;
	for(it=_filters.begin();it!=_filters.end();it++)
	{
		if (*it != NULL)
		{
			std::string strName = (*it)->GetName();
			try
			{
				delete (*it);
				MOLOG(Log::L_INFO, CLOGFMT(BaseGph, "[%s] (%s) destroyed"), _strLogHint.c_str(), strName.c_str());
			}
			catch(...)
			{
				MOLOG(Log::L_WARNING, CLOGFMT(BaseGph, "[%s] (%s) destroy caught unknown exception"), _strLogHint.c_str(), strName.c_str());
			}			
		}
	}
	_filters.clear();
	_srcFilter = NULL;
	_driverFilter = NULL;
	
	if (_pQueMediaSample)
	{
// 		MOLOG(Log::L_DEBUG, CLOGFMT(BaseGph, "[%s] MediaSample queue available[%d], total[%d]"), 
// 			_strLogHint.c_str(), _pQueMediaSample->getAvailableSize(), _pQueMediaSample->getTotalSize());

		delete _pQueMediaSample;
		_pQueMediaSample=NULL;
	}
	
	MOLOG(Log::L_INFO, CLOGFMT(BaseGph, "[%s] Graph destroyed"), _strLogHint.c_str());
}

void BaseGraph::SetMediaSampleSize(int nSampleSize)
{
	_nSampleSize = nSampleSize;
	MOLOG(Log::L_DEBUG, CLOGFMT(BaseGph, "[%s] set MediaSampleSize=%d"), 
		_strLogHint.c_str(), nSampleSize);
}

void BaseGraph::SetMaxAllocSampleCount(int nMaxSampleCount)
{
	_nMaxAllocSampleCount = nMaxSampleCount;
// 	_pQueMediaSample->setMaxSize(_nMaxAllocSampleCount);
	if(_pQueMediaSample)
		_pQueMediaSample->initialize(sizeof(MediaSample), _nMaxAllocSampleCount, 50);
	MOLOG(Log::L_DEBUG, CLOGFMT(BaseGph, "[%s] set MaxAllocSampleCount=%d"), 
		_strLogHint.c_str(), nMaxSampleCount);
}

bool BaseGraph::ConnectTo(BaseFilter* pFrom, int nOutputIndex, BaseFilter* pTo, int nInputIndex)
{
	if (!pFrom->Next(nOutputIndex, pTo, nInputIndex))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(BaseGph, "[%s] Failed to connect next (%s)[%d] to (%s)[%d]"), 
			_strLogHint.c_str(), pFrom->GetName(), nOutputIndex, pTo->GetName(), nInputIndex);
		return false;
	}

	if (!pTo->Prev(nInputIndex, pFrom, nOutputIndex))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(BaseGph, "[%s] Failed to connect prev (%s)[%d] to (%s)[%d]"), 
			_strLogHint.c_str(), pTo->GetName(), nInputIndex, pFrom->GetName(), nOutputIndex);
		return false;
	}

#if 0
	MOLOG(Log::L_DEBUG, CLOGFMT(BaseGph, "[%s] Connect (%s)[%d] to (%s)[%d] successfully"), 
		_strLogHint.c_str(), pFrom->GetName(), nOutputIndex, pTo->GetName(), nInputIndex);
#endif

	return true;
}

bool BaseGraph::AddFilter(BaseFilter* pFilter)
{
	_filters.push_back(pFilter);
	pFilter->SetGraph(this);
	pFilter->SetLog(_pLog);
	pFilter->SetLogHint(_strLogHint.c_str());

	if (pFilter->GetType() == BaseFilter::TYPE_SOURCE)
		_srcFilter = (BaseSource*)pFilter;

	if (pFilter->IsDriverModule())
		_driverFilter = pFilter;

	return true;
}

bool BaseGraph::RemoveFilter(BaseFilter* pFilter)
{
	std::vector<BaseFilter*>::iterator it;
	for(it=_filters.begin();it!=_filters.end();it++)
	{
		if (pFilter == *it)
		{
			_filters.erase(it);
			return true;
		}
	}

	return false;
}

void BaseGraph::InitPins()
{
	std::vector<BaseFilter*>::iterator it;
	for(it=_filters.begin();it!=_filters.end();it++)
	{
		if (*it!=NULL)
		{
			(*it)->InitPins();
			MOLOG(Log::L_DEBUG, CLOGFMT(BaseGph, "[%s] (%s) Pin initialized"), _strLogHint.c_str(), (*it)->GetName());
		}
	}
}

bool BaseGraph::Init()
{
	std::vector<BaseFilter*>::iterator it;
	for(it=_filters.begin();it!=_filters.end();it++)
	{
		if (*it!=NULL)
		{
			if (!(*it)->Init())
			{
				MOLOG(Log::L_ERROR, CLOGFMT(BaseGph, "[%s] (%s) failed to initialize"), _strLogHint.c_str(), (*it)->GetName());
				return false;
			}			

			MOLOG(Log::L_INFO, CLOGFMT(BaseGph, "[%s] (%s) initialized successfully"), _strLogHint.c_str(), (*it)->GetName());
		}
	}
	
	MOLOG(Log::L_INFO, CLOGFMT(BaseGph, "[%s] Graph initialized"), _strLogHint.c_str());
	
	return true;
}

bool BaseGraph::Start()
{
	std::vector<BaseFilter*>::iterator it;
	for(it=_filters.begin();it!=_filters.end();it++)
	{
		if (*it!=NULL)
		{
			if (!(*it)->Start())
			{
				MOLOG(Log::L_ERROR, CLOGFMT(BaseGph, "[%s] (%s) failed to start"), _strLogHint.c_str(), (*it)->GetName());
				return false;
			}			

			MOLOG(Log::L_INFO, CLOGFMT(BaseGph, "[%s] (%s) started successfully"), _strLogHint.c_str(), (*it)->GetName());
		}
	}

	MOLOG(Log::L_INFO, CLOGFMT(BaseGph, "[%s] Graph started"), _strLogHint.c_str());

	return true;
}
bool BaseGraph::Run()
{
	_bStop = false;
	if (!_driverFilter)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(BaseGph, "[%s] Run() failed with error: no driver filter specifed"), _strLogHint.c_str());
		return false;
	}
	
	MOLOG(Log::L_INFO, CLOGFMT(BaseGph, "[%s] Graph Run() enter"), _strLogHint.c_str());
	bool bRet = _driverFilter->Run();
	MOLOG(Log::L_INFO, CLOGFMT(BaseGph, "[%s] Graph Run() left"), _strLogHint.c_str());

	return bRet;
}

void BaseGraph::Stop()
{
	if (_srcFilter)
	{
		_srcFilter->Stop();
		MOLOG(Log::L_INFO, CLOGFMT(BaseGph, "[%s] Graph stop() called"), _strLogHint.c_str());
	}	
}
	
void BaseGraph::Close()
{
	if (!_bGraphClosed)
	{

		std::vector<BaseFilter*>::iterator it;
		for(it=_filters.begin();it!=_filters.end();it++)
		{
			if (*it != NULL)
			{
			std::string strName = (*it)->GetName();
			(*it)->Close();			
			MOLOG(Log::L_INFO, CLOGFMT(BaseGph, "[%s] (%s) closed"), _strLogHint.c_str(), strName.c_str());
			}
		}
		_bGraphClosed = true;
		MOLOG(Log::L_INFO, CLOGFMT(BaseGph, "[%s] Graph closed"), _strLogHint.c_str());
	}

}
	
void BaseGraph::SetLogHint(const char* szLogHint)
{
	_strLogHint = szLogHint;

	std::vector<BaseFilter*>::iterator it;
	for(it=_filters.begin();it!=_filters.end();it++)
	{
		if (*it != NULL)
		{
			(*it)->SetLogHint(szLogHint);			
		}
	}
}

MediaSample* BaseGraph::allocMediaSample()
{
	assert(_pAlloc);

	MediaSample* pMediaSample;
	void* pBuf;
	ZQ::common::Buffer mediasampleBuf;
	ZQ::common::BufferPool::ErrorCode mediasampleErr;

	if (!_pQueMediaSample->allocate(mediasampleBuf, mediasampleErr))
	{
// 		MOLOG(Log::L_ERROR, CLOGFMT(BaseGph, "[%s] allocMediaSample() allocated too many buffers, exceeded %d samples"),
// 			_strLogHint.c_str(), _pQueMediaSample->getTotalSize());
		size_t nActive,  nFree, nMax;
		_pQueMediaSample->getStatus(nActive, nFree, nMax);
		MOLOG(Log::L_ERROR, CLOGFMT(BaseGph, "[%s] allocMediaSample() failed to allocate mediasample memory size [%d]from buffer manager, errorcode[%d], errormsg[%s],  BufferPool Size[Active:(%d) Free:(%d) Max:(%d)]"),
			_strLogHint.c_str(), sizeof(MediaSample), mediasampleErr, _pQueMediaSample->showError(mediasampleErr), nActive, nFree, nMax);
		return NULL;
	}

	pMediaSample = (MediaSample*) mediasampleBuf.ptr;
	

	ZQ::common::Buffer allocBuf;
	ZQ::common::BufferPool::ErrorCode allocErr;
	if (!_pAlloc->allocate(allocBuf, allocErr))
	{
	    //_pQueMediaSample->free(pMediaSample);
		_pQueMediaSample->release(mediasampleBuf, mediasampleErr);
		//MOLOG(Log::L_ERROR, CLOGFMT(BaseGph, "[%s] allocMediaSample() failed to allocate memory size[%d] from buffer manager, used count[%d], max cached[%d]"),
		//_strLogHint.c_str(), _nSampleSize, _pQueMediaSample->getUsedSize(), _pQueMediaSample->getTotalSize());

		size_t nActive,  nFree, nMax;
		_pQueMediaSample->getStatus(nActive, nFree, nMax);
		MOLOG(Log::L_ERROR, CLOGFMT(BaseGph, "[%s] allocMediaSample() failed to allocate memory size [%d]from buffer manager, errorcode[%d], errormsg[%s], BufferPool Size[Active:(%d) Free:(%d) Max:(%d)]"),
			_strLogHint.c_str(), _nSampleSize, allocErr, _pAlloc->showError(allocErr), nActive, nFree, nMax);
		return NULL;
	}

	pBuf = allocBuf.ptr;
	pMediaSample->init(pBuf, _nSampleSize);
	return pMediaSample;
}

void BaseGraph::freeMediaSample(MediaSample* pSample)
{
	assert(_pAlloc && pSample);

	if (!pSample)
		return;

	if (pSample->ref() > 0)
	{
		pSample->relRef();
		return;
	}

	void* pBuf;

	if ((pBuf = pSample->getPointer()))
	{
		ZQ::common::Buffer allocBuf;
		ZQ::common::BufferPool::ErrorCode allocErr;

		allocBuf.ptr = pBuf;
		//_pAlloc->free(pBuf);	
		_pAlloc->release(allocBuf, allocErr);
	}
	ZQ::common::Buffer medicsapleBuf;
	ZQ::common::BufferPool::ErrorCode medicsapleErr;
     medicsapleBuf.ptr = (void *)pSample;
	_pQueMediaSample->release(medicsapleBuf, medicsapleErr);	
}

void BaseGraph::Finish()
{
	_bStop = true;
	MOLOG(Log::L_INFO, CLOGFMT(BaseGph, "[%s] Graph finished"), _strLogHint.c_str());
}

void BaseGraph::SetLog(ZQ::common::Log* pLog)
{
	_pLog = pLog;
	std::vector<BaseFilter*>::iterator it;
	for(it=_filters.begin();it!=_filters.end();it++)
	{
		if (*it != NULL)
		{
			(*it)->SetLog(pLog);			
		}
	}
//	_pQueMediaSample->setLog(pLog);
}

int64 BaseGraph::getTotalBytes()
{
	if (_llTotalBytes)
		return _llTotalBytes;

	if (!_srcFilter)
		return 0;
	
	_llTotalBytes = _srcFilter->getTotalBytes();
	return _llTotalBytes;
}

}}

