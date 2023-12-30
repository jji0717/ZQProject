
#ifndef RTF_PROCESS_H
#define RTF_PROCESS_H


#include "Locks.h"
#include "BaseClass.h"
#include <list>

extern "C" 
{
#include "CTFLib.h"
}



#define PROCESS_TYPE_RTF	"TrickGeneration"
#define MAX_GROUPS_PER_SEQUENCE			1
#ifdef CTF_MAX_GOP_PICS
#define MAX_PICTURES_PER_GROUP			CTF_MAX_GOP_PICS
#else
#define MAX_PICTURES_PER_GROUP			128
#endif


#define RTF_MAX_TRICKSPEED_NUMBER	        8
#define RTF_MAX_OUTPUT_FILE_COUNT           2 * RTF_MAX_TRICKSPEED_NUMBER + 1 + 1 + 1 // trick files + index file + main file + splice file
			
#define MAX_AUGMENTATION_PIDS			    4

namespace ZQTianShan {
	namespace ContentProvision {
typedef struct 
{
	std::string ext;
	std::string extForCisco;
	int position;
}FileExtension;
//typedef std::vector<FileExtension>  FileExtensions;
typedef std::map<std::string, FileExtension>  FileExtensions;
class RTFProcess : public BaseProcess
{
public:

	virtual void InitPins();
	virtual bool Init();
	
	virtual void Close();
	
	virtual void endOfStream(){}
	
	virtual const char* GetName() {return PROCESS_TYPE_RTF;}
	
	virtual bool Receive(MediaSample* pSample, int nInputIndex = 0){return false;}
		
	virtual MediaSample* GetData(int nOutputIndex = 0){return NULL;}

	RTFProcess();
	virtual ~RTFProcess();

	virtual bool Start();
	virtual bool Run();
	void setRetryCount(int retryCount);
	void setCsicoFileExt(bool bTestForCsico){_bTestForCsico = bTestForCsico;};
	void setCsicoMainFileExt(std::string& mainFileExt){_mainFileExt = mainFileExt;};
protected:
	
public:
 	typedef enum {OPTFT_MAIN, OPTFT_VVX, OPTFT_VV2,OPTFT_VVC,OPTFT_FF, OPTFT_FR,OPTFT_FF1, OPTFT_FR1, OPTFT_FF2, OPTFT_FR2,OPTFT_FF3, OPTFT_FR3,OPTFT_FFR,OPTFT_FFR1,OPTFT_FFR2,OPTFT_FFR3} OutputFileType;

	void setTrickGenParam(CTF_INDEX_TYPE indexType=CTF_INDEX_TYPE_VVX, CTF_VIDEO_CODEC_TYPE codecType=CTF_VIDEO_CODEC_TYPE_MPEG2);
	void setTrickFile(std::map<std::string, int>& temp);
	void setTrickFileEx(FileExtensions& temp){_trickSpeedAndFileExt = temp;}

 	static bool initRTFLib(uint32 maxSession, ZQ::common::Log* pLog, 
						   uint32 inputBufferSize, 
						   uint32 maxInputBuffersPerSession,
						   uint32 sessionFailThreshold, 
						   bool trickKeyFrame = true, bool rtfWarningTolerance = true);
	static void uninitRTFLib();

	int trickFileCloseOutputProcess(int fileNo);
    void settrickSpeedNumerator(std::list<float>& temp);
	void settrickSpeedNumeratorHD( std::list<float>& temp);
	void setAudioOnly(bool bAudioOnly=false){_bAudioOnly = bAudioOnly;}
	void setLegacyAudioOnly();
	void getOutputFileExtCol(std::string& ext);
	void getOutputFileInfo(std::map<std::string, std::string>& fileMap);
	void getIndexType(std::string& type);
    void setProviderId();
	void setAssetInfo(const std::string& providerId, const std::string& provAssetId);
    void setAugmentationPids(uint16* pIDs, int pidcount);
	bool getPreEncrypt(){return _bPreEncryptFile;};
	void getPreEncryptBitRate(	int& augmentedBitRate, int& originalBitRate);
	void getAugmentationPids(std::string& augmentionpids);
	void setUnifiedTrickFile(bool bUnifiedTrickFile){_unifiedTrickFile = bUnifiedTrickFile;};
protected:

	static bool isRTFLibLoaded();
	static void setRTFLibLoaded(bool bLoaded);

	static void* appAlloc(void* hAppHeap, int32_t bytes);
	static void  appFree(void* hAppHeap, void *ptr);
	static void  sessionErrorNotifier(void* hAppSession, char *pMessage);

#ifdef ZQ_OS_MSWIN
	static int32_t readInputStream( void* hAppSession,unsigned char *pBuffer, unsigned long bufSize,
		unsigned long *pBufOccupancy,CTF_BUFSEEK bufSeek, INT64 bufSeekOffset );
	
	static int32_t trickFilePutOutputBuffer(void *hAppSession, void *hAppFile, void *hAppBuffer,unsigned char *pBuffer, unsigned long occupancy, CTF_BUFSEEK bufSeek, INT64 bufSeekOffset);

	static int32_t trickFileGetOutputBuffer( void *hAppSession, void *hAppFile, void **phAppBuffer,unsigned char **ppBuffer, unsigned long *pCapacity);
#else
	static int32_t readInputStream( void* hAppSession, uint8_t* pBuffer, uint32_t bufSize,
		uint32_t* pBufOccupancy, CTF_BUFSEEK bufSeek, int64_t bufSeekOffset );

	static int32_t trickFilePutOutputBuffer(void *hAppSession, void *hAppFile, void *hAppBuffer,uint8 *pBuffer, uint32_t occupancy, CTF_BUFSEEK bufSeek, int64_t bufSeekOffset);

	static int32_t trickFileGetOutputBuffer( void *hAppSession, void *hAppFile, void **phAppBuffer,uint8_t **ppBuffer, uint32_t *pCapacity);
#endif

	int augmentationScan();

	static int trickFilePutOutputBuffer_unite(void *hAppSession, void *hAppFile, void *hAppBuffer,uint8 *pBuffer, uint32 occupancy, CTF_BUFSEEK bufSeek, int64 bufSeekOffset);

	static int trickFileGetOutputBuffer_unite( void *hAppSession, void *hAppFile, void **phAppBuffer,uint8 **ppBuffer, uint32 *pCapacity);
	
	static void  appLog(void *hAppSession, const char *pShortString, char *pLongString);
	static void  rtfLibErrorLog(char *pMessage);

	static int32_t releaseInputBuffer(void *hAppSession, void *hAppFile, void *hAppBuffer, unsigned char *pBuffer);
	static int32_t trickFileCloseOutput( void *hAppSession, void *hAppFile );
	
 	bool openSession();
 	bool closeSession();

	void initOutputFileInfo();
 	void getTrickExt( int speedNo, char* ext1, char* ext2);
	void getUnifiedTrickExt(int speedNo, char* ext);

	static double getFrameRateByCode(uint16 framecode);
public:
	static void getCTFLibVersion(int& major, int& Minor);
	static int _ctfLibVerMajor;
	static int _ctfLibVerMinor;
private:
	typedef struct _OUTPUTFILEINFO
	{	
		int             fileNo;
		OutputFileType  fileType;
		int             speedNo;
		char            extension[32];
		int64			filesize;
		int64			fileOffset;
		int64           begOffset;
		int64           endOffset;
		uint32           speedDirection;
		uint32           speedNumerator;
		uint32           speedDenominator;
		uint32           speedNumeratorHD;
		uint32           speedDenominatorHD;
	}OutputFileInfo, *POutputFileInfo;
// 
	static bool                  _rtfInited;
	// static ZQ::common::Log*      _rtfLogger;
	static bool                  _trickKeyFrame;

	static CTF_WARNING_TOLERANCE _rtfTolerance;
	CTF_VIDEO_CODEC_TYPE    _vcdType;
	CTF_INDEX_TYPE          _indexType;
	CTF_INDEX_MODE          _indexMode;
	CTF_INDEX_OPTION        _indexOption;
	CTF_SES_HANDLE			_sessionHandle;

	static ZQ::common::Mutex	 _lock;			//for rtflib

	bool                    _unifiedTrickFile;
 	int                     _outputFileCount;

	OutputFileInfo          _outputInfo[RTF_MAX_OUTPUT_FILE_COUNT];

	int                     _eosSubFileCount;  // end of stream sub file count
	bool                    _sessClosed;
	
	bool                    _bFirstReleaseSrc;

	FileExtensions         _trickSpeedAndFileExt;//used to specify speed num
	std::map<int, int> _indexExMap;
	unsigned long           _newTrickSpeedNumerator[4];
	unsigned long           _newTrickSpeedNumeratorHD[4];
	bool                    _bAudioOnly;
	bool                    _bfrontToBackFR;
std::string             _strprovId;
	std::string             _strprovAssetId;

	int32_t _augmentationPidCount;
	uint16 _augmentationPid[ MAX_AUGMENTATION_PIDS ];
	int32_t _augmentedBitRate;
	int32_t _originalBitRate;
	int                     _retryCount;
	int						_alreadyRetryCount;
	bool                    _bRetry;

	bool                    _bPreEncryptFile;
	bool                    _bTestForCsico;
	std::string             _mainFileExt;

	int						_lingerAtFailed;
};


}}

#endif
