

#ifndef _NTFSFILESET_TARGET_FILTER_
#define _NTFSFILESET_TARGET_FILTER_

#include "BaseClass.h"


#define	TARGET_TYPE_NTFSFS	"NTFSFSIO"
#define NTFSFS_IO_SIZE		64*1024

#ifndef OBJECT_ID
#define OBJECT_ID ULONG
#endif

namespace ZQTianShan {
	namespace ContentProvision {
		

class NtfsFsTarget : public BaseTarget
{
protected:
	friend class TargetFactory;
	NtfsFsTarget();
	
public:
	virtual ~NtfsFsTarget();

	virtual void InitPins();

	virtual bool Init();
	
	virtual void Stop();
	
	virtual void Close();
	
	virtual void endOfStream();
	
	virtual const char* GetName();
	
	virtual LONGLONG getProcessBytes();
	
	virtual void setFilename(const char* szFile);

	void setBandwidth(unsigned int uBandwidthBps);

	void setTypeH264(){_bTypeH264=true;}

	void enablePacing(bool bEable = true){_bPacing = bEable;};
	
	virtual MediaSample* GetData(int nOutputIndex = 0){return NULL;}
	
	bool Receive(MediaSample* pSample, int nInputIndex=0);

	/// set cache path
	void setCacheDirectory(std::string path);
	
	// get the cache path
	std::string getCacheDirectory() { return _cachePath; };
	

	/// initialize vstrm handle
	static HANDLE initVstrm(std::string& errmsg);
	
	/// uninitialize vstrm handle
	static void uninitVstrm(HANDLE hvstrm);
	
	/// copy a file from NTFS to Vstrm without speed limitation
	static bool copyFileToVstrm(std::string sourceFile, std::string desFile, std::string& errmsg, bool disableBufDrvThrottle = false, HANDLE hVstrm = INVALID_HANDLE_VALUE);
	
	bool initMyVstrm(std::string& errmsg);
	bool uninitMyVstrm();

	bool reserveVstrmBW(std::string& errmsg);
	bool releaseVstrmBW();

	static void getVstrmError(HANDLE hVstrm, std::string& strErr);
	static bool disableBufDrvThrottle(HANDLE vstrmHandle, HANDLE fileHandle, OBJECT_ID objectId, std::string& errMsg); 

	void delOutput();

	bool flushCacheBuffer();
protected:

	bool	writeVstrmData(int nIndex, char* pBuf, unsigned int nLen);

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
		char			cacheBuffer[NTFSFS_IO_SIZE];
		unsigned int    cacheCurLength;
		
		ULONG64         bwTicket;
		DWORD           bwPercentageN;   // numerator for calculating bandwidth
		DWORD           bwPercentageD;   // denominator for calculating bandwidth
		DWORD           reservedBW;      // _maxbps * (_bwPercentageN / _bwPercentageD)
		bool			bIndexFile;		//true is index file
		const void*		pacingIndexCtx;	//context of pacing
		NtfsFsTarget*	pThis;			//point to NtfsFsTarget object
	};

	SubFile			_subFiles[4];
	
	HANDLE          _ntfsCacheHandle;	 // for index file
	LARGE_INTEGER   _posCacheRead;		// used for pacing, read data
	__int64         _vvxByteRead;
	char            _tmpbuffer[NTFSFS_IO_SIZE];
	DWORD           _lastReadSize;

	bool			_bDeleteOnFail;
	bool			_bFailed;

	static HANDLE                 _hVstrm;
	DWORD                         _bwmgrClientId;
	std::string	                  _cachePath;
	bool			_disableBufDrvThrottle;

	std::string		_strFilename;
	DWORD			_dwBandwidth;

	bool			_bPacing;
	bool			_bTypeH264;

};

}}


#endif