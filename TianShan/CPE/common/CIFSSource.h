

#ifndef _CIFS_SOURCE_FILTER_
#define _CIFS_SOURCE_FILTER_

#include "BaseClass.h"
#include "FileIo.h"

#define	SOURCE_TYPE_CIFS	"CIFSSrc"


namespace ZQTianShan {
	namespace ContentProvision {


class CIFSIOSource : public BaseSource
{
protected:
	friend class SourceFactory;
	CIFSIOSource();
	
public:

	virtual ~CIFSIOSource();

	virtual bool Init();
	
	virtual void Stop();
	
	virtual void Close();
	
	void setURL(const char* szURL){_srcURL=szURL;}
	void setFileName(const char* szFile){_strFilename=szFile;}
	void setIOFactory(FileIoFactory* pFac){_pFileIoFac = pFac;}
	void setSystemType(std::string systype){_strSysType = systype;}
	void setMountOpt(std::string opt){_szOpt = opt;}
	void setSharePath(std::string strpath){_szSharePath = strpath;}
	void setLocalSourceFlag(bool localFlag){_localSourceFlag = localFlag;}
	void setMountPath(std::string mountpath){_szMountPoint = mountpath;};
	virtual void endOfStream();
	
	virtual const char* GetName();
	
	virtual int64 getProcessBytes();
	
	virtual MediaSample* GetData(int nOutputIndex = 0);
	
	void setMaxBandwidth(unsigned int nBandwidthBps);

	bool readbuf(char* pBuf, unsigned int bufLen, unsigned int& rcvLen);
	bool seek(int64 offset, int pos);

	void setSourceUrlUTF8(bool bIsUTF8);

protected:

	std::string		              _srcURL;
	std::string                   _strFilename;

	std::auto_ptr<FileIo>	      _pFileIo;
	FileIoFactory*		          _pFileIoFac;

	unsigned int				  _nBandwidthBps;
	int64                         _fileSize;
	
	//for bitrate control
	BitrateControlor			 _bitrateCtrl;
	std::string                  _szMountPoint;
	std::string                  _strSysType;
	std::string                  _szOpt;
	std::string                  _szSharePath;
	bool                         _localSourceFlag;
	bool						 _bSourceUrlUTF8;
};


}}

#endif

