

#ifndef _VSTRM_TARGET_FILTER_
#define _VSTRM_TARGET_FILTER_

#include "BaseClass.h"


#define	TARGET_TYPE_VSTRM	"VSTRMIO"
#define VSTRM_IO_SIZE		64*1024

#ifndef OBJECT_ID
#define OBJECT_ID ULONG
#endif


namespace ZQTianShan {
	namespace ContentProvision {

class VstrmTarget : public BaseTarget
{
protected:
	friend class TargetFactory;
	VstrmTarget();
	
public:
	virtual bool Init();
	
	virtual void Stop();
	
	virtual void Close();
	
	virtual void endOfStream();
	
	virtual const char* GetName();
	
	virtual LONGLONG getProcessBytes();
	
	virtual void setFilename(const char* szFile){
		_strFilename = szFile;
	}

	void setBandwidth(unsigned int uBandwidthBps){_reservedBW = uBandwidthBps;}
	
	virtual MediaSample* GetData(int nOutputIndex = 0){return NULL;}
	
	bool Receive(MediaSample* pSample, int nInputIndex=0);

	/// set if it's vvx index file, default is no
	void setIsIndexFile(bool bYes){_bIndexFile = bYes;}

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

	bool delOutput();
protected:

	bool	writeData(char* pBuf, unsigned int nLen);

	bool	writeIndexData(char* pBuf, unsigned int nLen, unsigned int uOffetLow, unsigned int uOffetHigh);

	
	LONGLONG		_llProcBytes;
	OBJECT_ID		_objectId;	

	HANDLE          _hOutputFile;
	std::string		_strFilename;
	char			_cacheBuffer[VSTRM_IO_SIZE];
	unsigned int        _cacheCurLength;

	ULONG64         _bwTicket;
	DWORD           _bwPercentageN;   // numerator for calculating bandwidth
	DWORD           _bwPercentageD;   // denominator for calculating bandwidth
	DWORD           _reservedBW;      // _maxbps * (_bwPercentageN / _bwPercentageD)

	// for index file
	bool			_bIndexFile;
	HANDLE          _ntfsCacheHandle;


	bool			_bDeleteOnFail;
	bool			_bFailed;

	static HANDLE                 _hVstrm;
	DWORD                         _bwmgrClientId;
	std::string	                  _cachePath;
	bool			_disableBufDrvThrottle;



};


}}

#endif