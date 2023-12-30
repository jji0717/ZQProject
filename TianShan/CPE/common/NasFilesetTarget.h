

#ifndef _NASFILESET_TARGET_FILTER_
#define _NASFILESET_TARGET_FILTER_

#include "BaseClass.h"
#include "MD5CheckSumUtil.h"

#define	TARGET_TYPE_NAS	"NASFSIO"
#define NAS_IO_SIZE		64*1024


#ifndef OBJECT_ID
#define OBJECT_ID ULONG
#endif

namespace ZQTianShan {
	namespace ContentProvision {


class NasFsTarget : public BaseTarget
{
protected:
	friend class TargetFactory;
	NasFsTarget();
	
public:
	virtual ~NasFsTarget();

	virtual bool Init();
	
	virtual void Stop();
	
	virtual void Close();
	
	virtual void endOfStream();
	
	virtual const char* GetName();
	
	virtual void setFilename(const char* szFile);
	
	virtual void setFilePath(const char* szFilePath);

	void setBandwidth(unsigned int uBandwidthBps);

	void enablePacing(bool bEable = true){_bPacing = bEable;};


	void enableMD5(bool bEnable = true);
	void getMD5(std::string& strMD5);

	void getSupportFileSize(LONGLONG& supportFilesize);

	virtual LONGLONG getProcessBytes();

	virtual MediaSample* GetData(int nOutputIndex = 0){return NULL;}

	bool Receive(MediaSample* pSample, int nInputIndex=0);

	/// set cache path
	void setCacheDirectory(const std::string& path);

	// get the cache path
	std::string getCacheDirectory() { return _cachePath; };


	/// copy a file from NTFS to Nas without speed limitation
	static bool copyFileToNas(std::string sourceFile, std::string desFile, std::string& errmsg); 

	void delOutput();

	bool flushCacheBuffer();
protected:

	bool	writeNasData(int nIndex, char* pBuf, unsigned int nLen);

	bool	writeIndexData(char* pBuf, unsigned int nLen, unsigned int uOffetLow, unsigned int uOffetHigh);

	bool	pacingWriteIndex();

	bool	pacingWriteData(int nIndex, char* pBuf, unsigned int nLen);



	static void pacingAppLogCbk(const char * const pMsg);
	static int pacingAppWrite(const void * const pCbParam, const int len, const char buf[]);
	static int pacingAppSeek(const void * const pCbParam, const LONGLONG offset);
	static int pacingAppSetEOF(const void * const pCbParam, const LONGLONG offset);
	static void pacingAppReportOffsets(const void * const pCbParam, const LONGLONG offset1, const LONGLONG offset2);
	static const char* DecodePacedIndexError(const unsigned long err);


	struct SubFile
	{
		LONGLONG		llProcBytes;		
		OBJECT_ID		objectId;	

		HANDLE          hOutputFile;
		std::string		strFilename;
		char			cacheBuffer[NAS_IO_SIZE];
		unsigned int    cacheCurLength;

		ULONG64         bwTicket;
		DWORD           bwPercentageN;   // numerator for calculating bandwidth
		DWORD           bwPercentageD;   // denominator for calculating bandwidth
		DWORD           reservedBW;      // _maxbps * (_bwPercentageN / _bwPercentageD)
		bool			bIndexFile;		//true is index file
		const void*		pacingIndexCtx;	//context of pacing
		NasFsTarget*	pThis;			//point to VstrmFsTarget object

		LONGLONG		llProcOffset;
		std::vector<MediaSample*>	samples;
	};

	SubFile			_subFiles[4];

	HANDLE          _ntfsCacheHandle;	 // for index file
	LARGE_INTEGER   _posCacheRead;		// used for pacing, read data
	__int64         _vvxByteRead;
	char            _tmpbuffer[NAS_IO_SIZE];
	DWORD           _lastReadSize;

	bool			_bDeleteOnFail;
	bool			_bCleanup;

	DWORD                         _bwmgrClientId;
	std::string	                  _cachePath;

	bool			_bPacing;
	static bool		_bPacingTrace;
	std::string                   _filePath;

	bool			_bEnableMD5;
	ZQ::common::MD5ChecksumUtil _md5ChecksumUtil;

};

}}


#endif