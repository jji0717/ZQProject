

#ifndef _NTFS_SOURCE_FILTER_
#define _NTFS_SOURCE_FILTER_

#include "BaseClass.h"

#define	SOURCE_TYPE_NTFSSRC	"NTFSSrc"


namespace ZQTianShan {
	namespace ContentProvision {

class NTFSIOSource : public BaseSource
{
protected:
	friend class SourceFactory;
	NTFSIOSource();
	
public:
	virtual bool Init();
	
	virtual void Stop();
	
	virtual void Close();
	
	void setFileName(const char* szFile){_strFilename=szFile;}
	void setUtfFlag(bool utfFlag){_bUtf8Flag = utfFlag;}
	
	virtual void endOfStream();
	
	virtual const char* GetName();
	
	virtual LONGLONG getProcessBytes();
	
	virtual MediaSample* GetData(int nOutputIndex = 0);
	
	void setMaxBandwidth(unsigned int nBandwidthBps);

	virtual bool readbuf(char* pBuf, unsigned int bufLen, unsigned int& rcvLen);
	virtual bool seek(int64 offset, int pos);
protected:
	std::string		_strFilename;
	HANDLE			_hFile;

	__int64                       _fileSize;
	unsigned int				_nBandwidthBps;
	
	//for bitrate control
	BitrateControlor			_bitrateCtrl;
	bool _bUtf8Flag;

	__int64   _llReadOffset;
};


}}

#endif