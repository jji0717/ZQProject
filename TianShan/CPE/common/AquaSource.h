#ifndef _AQUA_SOURCE_FILTER_
#define _AQUA_SOURCE_FILTER_

#include "BaseClass.h"
#include "FileIo.h"

#define	SOURCE_TYPE_AQUA	"AQUASrc"


namespace ZQTianShan {
	namespace ContentProvision {


class AquaIOSource : public BaseSource
{
protected:
	friend class SourceFactory;
	AquaIOSource();
	
public:

	virtual ~AquaIOSource();

	virtual bool Init();
	
	virtual void Stop();
	
	virtual void Close();
	
	void setURL(const char* szURL){_srcURL=szURL;}
	void setFileName(const char* szFile){_strFilename=szFile;}
	void setIOFactory(FileIoFactory* pFac){_pFileIoFac = pFac;}

	virtual void endOfStream();
	
	virtual const char* GetName();
	
	virtual int64 getProcessBytes();
	
	virtual MediaSample* GetData(int nOutputIndex = 0);
	
	void setMaxBandwidth(unsigned int nBandwidthBps);

	bool readbuf(char* pBuf, unsigned int bufLen, unsigned int& rcvLen);
	bool seek(int64 offset, int pos);

protected:

	std::string		              _srcURL;
	std::string                   _strFilename;

	std::auto_ptr<FileIo>	      _pFileIo;
	FileIoFactory*		          _pFileIoFac;

	unsigned int				  _nBandwidthBps;
	int64                         _fileSize;
	
	//for bitrate control
	BitrateControlor			 _bitrateCtrl;
};


}}

#endif

