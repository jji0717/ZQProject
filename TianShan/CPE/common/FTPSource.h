

#ifndef _FTP_SOURCE_FILTER_
#define _FTP_SOURCE_FILTER_

#include "BaseClass.h"
#include <string>
#include <memory>

#define	SOURCE_TYPE_FTP	"FTPSrc"


namespace ZQTianShan {
	namespace ContentProvision {

class FTPClient;

class FTPIOSource : public BaseSource
{
protected:
	friend class SourceFactory;
	FTPIOSource();
	
public:

	virtual ~FTPIOSource();

	virtual bool Init();
	
	virtual void Stop();
	
	virtual void Close();
	
	void setLocalNetworkInterface(const char* szLocalNetIf);
	void setURL(const char* szURL){_srcURL=szURL;}
	void setConnectionMode(bool mode = true){_bPassiveMode = mode;}
	void setConnectionInterval(int interval){_connectionInterval = interval;}
	
	virtual void endOfStream();
	
	virtual const char* GetName();
	
	virtual int64 getProcessBytes();
	
	virtual MediaSample* GetData(int nOutputIndex = 0);
	
	void setMaxBandwidth(unsigned int nBandwidthBps);

	bool readbuf(char* pBuf, unsigned int bufLen, unsigned int& rcvLen);
	bool seek(int64 offset, int pos);

	void setDecodeSourceURL(bool bDecodeSourceURL);

	void setSourceUrlUTF8(bool bIsUTF8);
protected:

	std::string		              _srcURL;

	std::string		              _strLocalNetIf;

	unsigned int				  _nBandwidthBps;
	int64                       _fileSize;
	
	//for bitrate control
	BitrateControlor			 _bitrateCtrl;
	bool                         _bPassiveMode;
	int                          _connectionInterval;

	int64                        _llReadOffset;
	std::string                  _strOrgFile;

	bool                         _bDecodeSourceURL;
	bool						 _bSourceUrlUTF8;
protected:
	
	std::auto_ptr<FTPClient>	 _pFTPDownloader; 
};


}}

#endif

