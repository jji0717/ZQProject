

#ifndef _FTPFileset_SOURCE_FILTER_
#define _FTPFileset_SOURCE_FILTER_

#include "BaseClass.h"
#include "MD5CheckSumUtil.h"
#include "FileIo.h"

#define	SOURCE_TYPE_FTPFileset	"FTPFilesetSrc"

#define DEFAULT_SOURCE_BUFF_COUNT	30
#define MAX_READ_SIZE               (1024*64)     // do not change this, coz to VStrm, it is fixed
#define DEF_FTP_CONN_TIMEOUT        5000

//#ifndef OBJECT_ID
//#define OBJECT_ID uint32
//#endif


namespace ZQTianShan {
	namespace ContentProvision {

		struct SubFileInfo
		{
			char	ext[8];
			long	numerator;
			long	denominator;
			long	direction;				
		};

class FTPDownload;


class FTPFilesetSource : public BaseSource
{
protected:
	friend class SourceFactory;
	FTPFilesetSource();
	virtual ~FTPFilesetSource();
	
public:
	virtual bool Init();
	
	virtual void Stop();
	virtual bool Run();
	virtual void Close();
	
	void setURL(const char* szURL){_srcURL=szURL;}
	void setTargetDir(const char* szDir);
	void setCacheDir(const char* szCachDir);
	void setMode(bool bNtfsTest){_bNtfsTest = bNtfsTest;}
	void setFilename(const char* szFilename);
	void setIOFactory(FileIoFactory* pFac){_pFileIoFac = pFac;}

	void getMD5(std::string& strMD5);
	void getSupportFileSize(int64& supportFilesize){supportFilesize = _supportFilesize;}
	void enableMD5(bool bEnable = true);
	void getOrigFileSize(int64& orgFileSize){orgFileSize = _OrigFileSize;}

	virtual void endOfStream();
	
	virtual const char* GetName();
	
	virtual int64 getProcessBytes();
	
	virtual MediaSample* GetData(int nOutputIndex = 0){return NULL;}
	
	void setMaxBandwidth(unsigned int nBandwidthBps);
	
	void setBandwidthCtrlInterval(int nIntervalMs);
    void setDecodeSourceURL(bool bDecodeSourceURL);
protected:

	bool openTargetFile(const std::string& targetName);
	bool openRemoteFile(const std::string& sourceName);
	bool writeTargetFile(char* pBuf, int nLen);
	void closeTargetFile();

	bool  reserveVBw();
	bool releaseVBW();

	std::string getVvxIndexFileName(const std::string& strMainFile);
	std::string getVv2IndexFileName(const std::string& strMainFile);
	
	bool isRemoteVvxIndex(const std::string& strRemoteMainFile);
	bool downloadIndexFile(const std::string& strIndexFile, const std::string& strLocalFile);

	bool readbuf(char* pBuf, unsigned int bufLen, unsigned int& rcvLen);
	bool seek(int64 offset, int pos);

	bool getIdxSubFileInfo(const char* szIdxFile, bool bVvx, std::vector<SubFileInfo>& subFiles, MediaInfo& info);

protected:

	std::string		              _srcURL;
	std::string                   _tempDir;
	std::string                   _outputDir;

	std::string                   _strFilename;		//target file name 

	std::string		              _strLocalNetIf;

	std::vector<std::string>     _srcFileset;
	std::vector<std::string>     _targetFileset;

	bool						_bEnableMD5;
	ZQ::common::MD5ChecksumUtil _md5ChecksumUtil;
	int64						_supportFilesize;		//subfile size, except for main file

	unsigned int				  _nBandwidthBps;
	int64                       _totalBytes;
	int64                       _OrigFileSize;
	
	std::auto_ptr<FTPDownload>	 _pFtpDownloader;
	//for bitrate control
	BitrateControlor			 _bitrateCtrl;
	MediaInfo         _info;

	bool                         _bNtfsTest; 
//	uint32		             _objectId;	
	uint64                      _bwTicket;
	FileIoFactory*		          _pFileIoFac;
	std::auto_ptr<FileIo>	      _pFileIo;

	bool                          _bDecodeSourceURL;
};


}}

#endif

