
#ifndef _CPH_BASE_CLASS_
#define _CPH_BASE_CLASS_


#include "Log.h"
#include "SystemUtils.h"
#include <map>
#include <string>
#include <memory>
#include <vector>
//#include "IMemAlloc.h"
#include "BufferPool.h"
namespace ZQ{
	namespace common{
		class NativeThreadPool;
	}
}

//class QueueBufMgr;

namespace ZQTianShan {
	namespace ContentProvision {


class MediaSample
{
public:
	MediaSample(void* buf, int bufsiz)
	{
		init(buf, bufsiz);
	}

	inline void init(void* buf, int bufsiz)
	{
		_pBuf = buf;
		_nBufSize = bufsiz;
		_nDataLen = 0;
		_uOffsetHigh = 0;
		_uOffsetLow = 0;
		_uRef = 0;
	}
	
public:
	inline void*			getPointer(){return _pBuf;}
	inline unsigned int		getBufSize(){return _nBufSize;}
	inline unsigned int		getDataLength(){return _nDataLen;}
	inline void				setDataLength(unsigned int nLen){_nDataLen = nLen;}
	inline unsigned int		getFreeSize(){return _nBufSize - _nDataLen;}
	inline void*			getFreeBufferPointer(){return (void*)((char*)_pBuf + _nDataLen);}
	inline void				increaseDataLength(unsigned int nLen){_nDataLen += nLen;}
	inline void				setOffset(unsigned int uOffsetLow, unsigned int uOffsetHigh)
	{
		_uOffsetHigh = uOffsetHigh;
		_uOffsetLow = uOffsetLow;
	}
	inline void				setOffset(int64 llOffset)
	{
		_uOffsetLow = (unsigned int)llOffset;
		_uOffsetHigh = *(((unsigned int*)&llOffset)+1);
	}

	inline unsigned int			getOffset(unsigned int* puOffsetHigh)
	{
		if (puOffsetHigh)
			*puOffsetHigh = _uOffsetHigh;
		
		return _uOffsetLow;
	}

	inline unsigned int ref()
	{
		return _uRef;
	}

	inline unsigned int addRef()
	{
		return ++_uRef;
	}

	inline unsigned int relRef()
	{
		if (_uRef > 0)
			_uRef--;

		return _uRef;
	}
	
private:
	void*		_pBuf;
	int			_nBufSize;
	int			_nDataLen;
	unsigned int	_uOffsetLow;
	unsigned int	_uOffsetHigh;
	unsigned int	_uRef;
};

struct MediaInfo
{
	int bitrate;
	int videoBitrate;
	int videoResolutionH;
	int videoResolutionV;
	int64	filesize;
	int	playTime;
	double framerate;
	MediaInfo(){memset(this, 0, sizeof(MediaInfo));}
};

#define  BITRATE_CONTROL_MAX_WAITMS		3000

class BitrateControlor
{
public:
	BitrateControlor()
	{
		_nBitrateBps = 0;
		_nCtrlIntervalMs = 60; //ms
		_lastCtrlBytes = 0;
		_nBytesInterval = 0;
		_nStart = 0;
	}

	void setBitrate(int nBitrateBps)
	{
		_nBitrateBps = nBitrateBps;
	}

	void setInterval(int nCtrlIntervalMs = 60)
	{
		_nCtrlIntervalMs = nCtrlIntervalMs;	
	}

	void start()
	{
		_nBytesInterval = int32(((float)_nBitrateBps) * _nCtrlIntervalMs / 8000);
		_nStart = SYS::getTickCount();
		_lastCtrlBytes = 0;
	}

	// return the sleep time in milliseconds
	uint32 control(uint64 llProcBytes)
	{
		if (!_nBitrateBps)
			return 0;

		if (llProcBytes < _lastCtrlBytes + _nBytesInterval)
		{
			//do nothing
			return 0;
		}

		_lastCtrlBytes = llProcBytes;

		uint32 nSpent =  SYS::getTickCount() - _nStart;			
		uint32 nShouldSpent = uint32(llProcBytes*8000/_nBitrateBps); //ms

		if (nShouldSpent > nSpent)
		{	
			if (nShouldSpent > nSpent)
			{
				uint32 nSleep = nShouldSpent - nSpent;
				if (nSleep > BITRATE_CONTROL_MAX_WAITMS)
					nSleep = BITRATE_CONTROL_MAX_WAITMS;

				SYS::sleep(nSleep);			
				return nSleep;
			}
		}	

		return 0;
	}	

private:

	int			_nBitrateBps;
	int			_nCtrlIntervalMs;
	uint32		_nStart;
	uint32		_nBytesInterval;
	uint64		_lastCtrlBytes;
};


class BaseGraph;
class BaseFilter
{
protected:
	BaseFilter();
	
public:
	enum
	{
		TYPE_SOURCE,
		TYPE_TARGET,
		TYPE_PROCESS,
	};

	virtual ~BaseFilter();
	
	//called before connect pins, initialize input/output pins
	virtual void InitPins() = 0;

	//called after connect pins, initialize filter
	virtual bool Init() = 0;

	//only the driver module need to implement it
	virtual bool Run(){return true;};
	
	//called after init() and before run()
	virtual bool Start(){return true;}
	virtual void Stop();

	virtual void Close() = 0;

	virtual void endOfStream() = 0;

	virtual int GetInputCount(){return _nInputCount;}
	
	virtual int GetOutputCount(){return _nOutputCount;}

	virtual int GetType() {return _nType;}

	virtual const char* GetName() = 0;

	virtual bool Next(int nOutputIndex, BaseFilter* pNextFilter, int nInputIndex);

	virtual bool Prev(int nInputIndex, BaseFilter* pPrevFilter, int nOutputIndex);

	virtual bool Receive(MediaSample* pSample, int nInputIndex = 0) = 0;
	
	virtual MediaSample* GetData(int nOutputIndex = 0) = 0;

	virtual int64 getProcessBytes(){return _llProcBytes;}

	virtual void SetLog(ZQ::common::Log* pLog){_pLog = pLog;}

	BaseGraph* GetGraph(){return _pGraph;}
	void SetGraph(BaseGraph* pGraph) {_pGraph = pGraph;}

	void SetLogHint(const char* szLogHint){_strLogHint = szLogHint;}

	virtual std::string GetLastError(){return _strLastErr;}
	virtual int GetLastErrorCode(){return _nLastErrCode;}
	void SetLastError(const std::string& strErr, int nErrorCode=0);

	bool IsFailed(){return _bFailed;}

	virtual bool IsDriverModule() {return _bDriverModule;}

	struct InputPin
	{
		int				nPrevPin;
		BaseFilter*		pPrevFilter;
	};

	struct OutputPin
	{
		int				nNextPin;
		BaseFilter*		pNextFilter;
	};

protected:
	ZQ::common::Log*	_pLog;
	BaseGraph*			_pGraph;
	std::string			_strLogHint;
	std::vector<InputPin>			_inputPin;
	std::vector<OutputPin>			_outputPin;
	int64			_llProcBytes;	
	int					_nInputCount;
	int					_nOutputCount;
	int					_nType;
	std::string			_strLastErr;
	int					_nLastErrCode;
	bool				_bFailed;
	bool				_bDriverModule;
	bool				_bStop;

};


class BaseSource : public BaseFilter
{
protected:
	friend class SourceFactory;

	BaseSource()
	{
		_nType = TYPE_SOURCE;
		_nInputCount = 0;
		_llTotalBytes = 0;
	}
public:
	enum{
		SRC_PULL,
			SRC_PUSH
	};

	virtual void InitPins(){};

	virtual int64 getTotalBytes() {return _llTotalBytes;};
	virtual bool Receive(MediaSample* pSample, int nInputIndex = 0){ return true;}

	virtual bool readbuf(char* pBuf, unsigned int bufLen, unsigned int& rcvLen) = 0;
	virtual bool seek(int64 offset, int pos) = 0;
protected:
	
	int64		_llTotalBytes;
};

class SourceFactory
{
public:
	static BaseSource* Create(const char* szName, ZQ::common::NativeThreadPool* pPool = NULL);	
	
};



class BaseTarget: public BaseFilter
{
public:
	virtual void setStreamableBytes(int nStreamableBytes);
	virtual MediaSample* GetData(int nOutputIndex = 0) {return NULL;}
	virtual bool IsStreamable(){return _bIsStreamable;}

	virtual void enableStreamableEvent(bool bEnable){_bStreamableEvent = bEnable;}
	virtual void enableProgressEvent(bool bEnable){_bProgressEvent = bEnable;}

	virtual void InitPins(){};
protected:
	BaseTarget();
	virtual void IncProcvBytes(int nBytes);

	bool	_bIsStreamable;
	int		_nStreamableBytes;
	bool	_bProgressEvent;
	bool	_bStreamableEvent;
};

class TargetFactory
{
public:
	static BaseTarget* Create(const char* szName, ZQ::common::NativeThreadPool* pPool = NULL);	
	
};

class BaseGraph
{
public:
	BaseGraph(int maxAllocSampleCount);
	BaseGraph();
	virtual ~BaseGraph();

	bool ConnectTo(BaseFilter* pFrom, int nOutputIndex, BaseFilter* pTo, int nInputIndex);
	bool AddFilter(BaseFilter* pFilter);
	bool RemoveFilter(BaseFilter* pFilter);

	//called before connect pins, initialize input/output pins
	void InitPins();

	//called after connect pins
	virtual bool Init();
	virtual bool Start();
	virtual bool Run();
	virtual void Stop();
	virtual void Close();
	virtual void Finish();

	virtual int64 getProcessBytes(){return _llProcBytes;}
	virtual int64 getTotalBytes();
	void setTotalBytes(int64 llTotal) {_llTotalBytes = llTotal;}

	virtual void SetLog(ZQ::common::Log* pLog);

	virtual ZQ::common::Log* GetLog(ZQ::common::Log* pLog){return _pLog;}

	void SetLogHint(const char* szLogHint);
//	void SetMemAlloc(IMemAlloc* pAlloc){
	void SetMemAlloc(ZQ::common::BufferPool* pAlloc){
		_pAlloc = pAlloc;
	};

	void SetMediaSampleSize(int nSampleSize);
	int GetMediaSampleSize(){return _nSampleSize;}

	void SetMaxAllocSampleCount(int nMaxSampleCount);
	int GetMaxAllocSampleCount(){return _nMaxAllocSampleCount;}

	virtual BaseSource* getSourceFilter(){return _srcFilter;};
	 
	bool IsStopped() {return _bStop;}

	MediaSample* allocMediaSample();
	void freeMediaSample(MediaSample*);
	void addRefMediaSample(MediaSample*);


	virtual std::string GetLastError(){return _strLastErr;}
	virtual int GetLastErrorCode(){return _nLastErrCode;}
	// add this function for CPH_RTI retry catapture stream
	//// TicketId: 7384, bugid:13290
	void RetSetError(bool bIsError){_bErrorOccurred = bIsError;};

	void SetLastError(const std::string& strErr, int nErrCode = 0)
	{
		if (!_bErrorOccurred)
		{
			_strLastErr = strErr;
			_nLastErrCode = nErrCode;
			_bErrorOccurred = true;
		}	
	}

	bool IsErrorOccurred(){return _bErrorOccurred;}

	virtual void OnProgress(int64& prcvBytes)
	{
		_llProcBytes = prcvBytes;
	}

	virtual void OnStreamable(bool bStreamable)
	{
		_bStreamable = bStreamable;
	}

	virtual void OnMediaInfoParsed(MediaInfo& mInfo) = 0;

	virtual bool IsStreamble(){return _bStreamable;}
protected:
	ZQ::common::Log*	_pLog;
	std::vector<BaseFilter*>	_filters;
	BaseSource*					_srcFilter;
	BaseFilter*					_driverFilter;
	std::string					_strLogHint;
//	IMemAlloc*					_pAlloc;
	ZQ::common::BufferPool*	    _pAlloc;
	int							_nSampleSize;
	bool						_bStop;
	std::string					_strLastErr;
	int							_nLastErrCode;
	bool						_bErrorOccurred;
	bool						_bStreamable;
	int64					_llProcBytes;	
	int64					_llTotalBytes;	

//	QueueBufMgr*				_pQueMediaSample;	//for MediaSample buffer
	ZQ::common::BufferPool*		_pQueMediaSample;	//for MediaSample buffer
	int							_nMaxAllocSampleCount;
	bool                        _bGraphClosed;
};

class BaseProcess : public BaseFilter
{
protected:
	friend class ProcessFactory;
	BaseProcess()
	{
		_nType = TYPE_PROCESS;		
	}
public:
	virtual void InitPins(){};

};

class ProcessFactory
{
public:
	static BaseProcess* Create(const char* szName, ZQ::common::NativeThreadPool* pPool = NULL);	
	
};

void getSystemErrorText(std::string& strErr);

}}

#endif

