

#ifndef _VSTRMFILESET_TARGET_FILTER_
#define _VSTRMFILESET_TARGET_FILTER_

#include "BaseClass.h"
#include "MD5CheckSumUtil.h"

#define	TARGET_TYPE_VSTRMFS	"VSMFSIO"
#define VSTRM_IO_SIZE		64*1024


#ifndef OBJECT_ID
#define OBJECT_ID ULONG
#endif

namespace ZQTianShan {
	namespace ContentProvision {


class VstrmFsTarget : public BaseTarget
{
protected:
	friend class TargetFactory;
	VstrmFsTarget();
	
public:
	virtual ~VstrmFsTarget();

	virtual void InitPins();

	virtual bool Init();
	
	virtual void Stop();
	
	virtual void Close();
	
	virtual void endOfStream();
	
	virtual const char* GetName();
	
	virtual void setFilename(const char* szFile);

	void setBandwidth(unsigned int uBandwidthBps);

	void enablePacing(bool bEnable = true){_bPacing = bEnable;};

	void enablePacingTrace(bool bTrace = true){
		_bPacingTrace = bTrace;
	}

	void setTypeH264(){_bTypeH264=true;}

	void disableBufDrvThrottle(bool bDisable)
	{
		_disableBufDrvThrottle = bDisable;
	}

	void setVstrmBwClientId(unsigned int vstrmBwClientId)
	{
		_bwmgrClientId = vstrmBwClientId;
	}

	void enableMD5(bool bEnable = true);
	
	virtual MediaSample* GetData(int nOutputIndex = 0){return NULL;}
	
	bool Receive(MediaSample* pSample, int nInputIndex=0);

	/// set cache path
	void setCacheDirectory(const std::string& path);

	void setTrickFile(std::map<std::string, int>& temp){_SpeedAndFileExt = temp;}
	
	// get the cache path
	std::string getCacheDirectory() { return _cachePath; };
	

	/// initialize vstrm handle
	static bool initVstrm(ULONG bwClientId, std::string& errmsg);
	
	/// uninitialize vstrm handle
	static void uninitVstrm(HANDLE hvstrm);
	static void uninitVstrm();

	/// copy a file from NTFS to Vstrm without speed limitation
	static bool copyFileToVstrm(const std::string& sourceFile, const std::string& desFile, std::string& errmsg, bool disableBufDrvThrottle = false, HANDLE hVstrm = INVALID_HANDLE_VALUE);
	
	bool reserveVstrmBW(std::string& errmsg);
	bool releaseVstrmBW();

	static void getVstrmErrorWithVStatus(HANDLE hVstrm, unsigned int status, std::string& strErr);
	static void getVstrmError(HANDLE hVstrm, unsigned int& status, std::string& strErr);
	static bool disableBufDrvThrottle(HANDLE vstrmHandle, HANDLE fileHandle, OBJECT_ID objectId, std::string& errMsg); 

	bool copyFileToVstrm(const std::string& sourceFile, HANDLE hVstrm, HANDLE targetHandle, std::string& errmsg);

	void getMD5(std::string& strMD5);
	void getSupportFileSize(LONGLONG& supportFilesize);
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
		char			cacheBuffer[VSTRM_IO_SIZE];
		unsigned int    cacheCurLength;
		std::string     strPacename;
		
		ULONG64         bwTicket;
		DWORD           bwPercentageN;   // numerator for calculating bandwidth
		DWORD           bwPercentageD;   // denominator for calculating bandwidth
		DWORD           reservedBW;      // _maxbps * (_bwPercentageN / _bwPercentageD)
		bool			bIndexFile;		//true is index file
		const void*		pacingIndexCtx;	//context of pacing
		VstrmFsTarget*	pThis;			//point to VstrmFsTarget object

		LONGLONG		llProcOffset;
		std::vector<MediaSample*>	samples;
	};

	SubFile			_subFiles[10];
	
	HANDLE          _ntfsCacheHandle;	 // for index file
	LARGE_INTEGER   _posCacheRead;		// used for pacing, read data
	__int64         _vvxByteRead;
	char            _tmpbuffer[VSTRM_IO_SIZE];
	DWORD           _lastReadSize;

	bool			_bDeleteOnFail;
	bool			_bCleanup;

	static HANDLE                 _hVstrm;
	DWORD                         _bwmgrClientId;
	std::string	                  _cachePath;
	bool			_disableBufDrvThrottle;

	std::string		_strFilename;
	DWORD			_dwBandwidth;
	bool			_bTypeH264;
	bool			_bPacing;
	static bool		_bPacingTrace;
	bool			_bEnableMD5;
	ZQ::common::MD5ChecksumUtil _md5ChecksumUtil;

	std::map<std::string, int> _SpeedAndFileExt;//used to specify speed num
};

}}


#endif