

#ifndef _PUSH_SOURCE_FILTER_
#define _PUSH_SOURCE_FILTER_

#include "BaseClass.h"

#define	SOURCE_TYPE_PUSHSRC	"PushSrc"

namespace ZQTianShan {
	namespace ContentProvision {

class IPushSource;

class PushSource : public BaseSource
{
protected:
	friend class SourceFactory;
	PushSource();

public:
	virtual bool Init();

	virtual void Stop();
	
	virtual void Close();

	void setPushSrcI(IPushSource* pPushI);
	
	virtual void endOfStream();
	
	virtual const char* GetName();
		
	virtual LONGLONG getProcessBytes();

	virtual MediaSample* GetData(int nOutputIndex = 0);

	void setMaxBandwidth(unsigned int nBandwidthBps);

	void setBandwidthCtrlInterval(int nIntervalMs);

	virtual bool readbuf(char* pBuf, unsigned int bufLen, unsigned int& rcvLen);
	virtual bool seek(int64 offset, int pos);
protected:
	IPushSource*	_pPushSess;
	unsigned int				_nBandwidthBps;

	//for bitrate control
	BitrateControlor			 _bitrateCtrl;
};


}}

#endif