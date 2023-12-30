

#ifndef _NTFS_FILESET_SOURCE_FILTER_
#define _NTFS_FILESET_SOURCE_FILTER_

#include "BaseClass.h"
#include "NativeThread.h"
#include <fstream>

#define	SOURCE_TYPE_NTFS_FILESET	"NTFSFileSetSource"
#define NTFSFS_IO_SIZE		64*1024

#ifndef OBJECT_ID
#define OBJECT_ID ULONG
#endif

namespace ZQTianShan {
	namespace ContentProvision {


class NTFSFileSetSource : public BaseSource
{
protected:
	friend class SourceFactory;
	NTFSFileSetSource();
	~NTFSFileSetSource();

public:
	virtual bool Init();

	virtual void Stop();
	
	virtual void Close();
	
	virtual void endOfStream();
	
	virtual const char* GetName();

    virtual void setFilename(const char* szFile);

	void setSourceDirectory(const char* szSourceDir);
	void setCachDir(const char* cachpath);
	void setWaitTime(int waittime);
	
	virtual LONGLONG getProcessBytes();

	virtual MediaSample* GetData(int nOutputIndex = 0);

	void setMaxBandwidth(unsigned int nBandwidthBps);

	void setBandwidthCtrlInterval(int nIntervalMs);

	bool WriteIndexFile(unsigned long& readbytes);
	virtual bool Run();

	virtual bool readbuf(char* pBuf, unsigned int bufLen, unsigned int& rcvLen);
	virtual bool seek(int64 offset, int pos);
protected:

	unsigned int				_nBandwidthBps;
	std::ifstream            	_fstrm;
	std::string		            _strFilename;
	std::string                 _cachePath;
	std::string                 _sourcePath;
	std::vector<std::ifstream>  _fstrmVec;

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
		NTFSFileSetSource*	pThis;			//point to NtfsFsTarget object
	};

	SubFile			_subFiles[4];

	//for bitrate control
	unsigned int				_nBytesPro;
	unsigned int				_nLastTime;
	unsigned int				_nBwCtlBytes;
	unsigned int				_nBwCtlMilliSec;
	unsigned int                _nBufSize;
	bool                        _quit;
	bool                        _bEndOfVvx;
	bool                        _bEndOfFF;
	bool                        _bEndOfFR;
	bool                        _bEndOfRegin;
    LONGLONG                    _llProFF;
    LONGLONG                    _llProFR;
	LONGLONG                    _llProVVX;
	LONGLONG                    _llProRegin;
	float                       _fColRate;

	HANDLE          _hFileOrign;
	HANDLE          _hFileff;
	HANDLE          _hFilefr;
	HANDLE          _hFilevvx;
	HANDLE          _ntfsCacheHandle;	 // for index file
	LARGE_INTEGER   _posCacheRead;		// used for pacing, read data
	__int64         _vvxByteRead;
	char            _tmpbuffer[NTFSFS_IO_SIZE];
	DWORD           _lastReadSize;
	DWORD           _filesize;
	int             _failNum;
	int             _waittime;
};


}}

#endif