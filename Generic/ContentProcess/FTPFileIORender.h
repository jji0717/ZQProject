// FTPFileIORender.h: interface for the FTPFileIORender class.
//
//////////////////////////////////////////////////////////////////////
#ifndef __ZQ_FTPFileIORender_Process_H__
#define __ZQ_FTPFileIORender_Process_H__

#include "GraphFilter.h"
#include "MD5ChecksumUtil.h"

class	CInternetSession;
class	CFtpConnection;
class   CInternetFile;

#define DEFAULT_RENDER_BUFF_COUNT	30
#define MAX_READ_SIZE               (1024*64)     // do not change this, coz to VStrm, it is fixed
#define DEFAULT_FTP_PORT            21

namespace ZQ {
namespace Content {
namespace Process {
	
class FTPFileIORender : public Filter
{
	friend class Graph;
public:

	FTPFileIORender(ZQ::Content::Process::Graph& graph, bool enableMD5Checksum = false, std::string myName="");
	virtual ~FTPFileIORender();
public:
	/// receiving the buffer coming from previous content process object
	/// The received Content Process Filter is required to re allocate buffer from pool
	/// and copy it.
	///@param[in]  upObj   the obj who call this function
	///@param[in]  buff      the buff in the pool which allocated in above Filter
	virtual bool receive(Filter* upObj, ZQ::Content::BufferData* buff);
	
	/// start to process the incoming BufferData
	///@return true if it start successfully
	virtual bool begin(void);
	
	/// pause to process the incoming BufferData
	///@return true if it pause successfully
	virtual bool pause(void);
	
	/// abort current buffer processing, generally, this is invoked by the Graph
	/// in case of any Filter obj failed during the processing, and this failure require all object need to aborted.
	/// @return true if it abort successfully
	virtual bool abort(void);
		
	/// stop content processing, just a little bit different with abort(), 
	/// it is a normal stopping, but abort() is abnormal.
	virtual void stop(void);
	
	/// stop the processing and exit the Filter thread
    virtual void quit(void);

	/// this virtual function MUST be implemented by source filter, to get know 
	/// how many process stuff in the whole processing, this could be source file total
	/// bytes or something else. 
	/// Currently seems only the source could provide the total number
	virtual __int64 getTotalStuff() { return _fileSize; };

	/// this virtual function must be render, to get know current processing progress,
	/// bytes or something else.
	virtual __int64 getProcessedStuff() { return _processedBytes; };

	virtual void endOfStream(void);

	void setAccessProperty(std::string server,std::string userName,std::string password, unsigned int port=DEFAULT_FTP_PORT);

	void enableMD5Checksum(bool enable) { _enableMD5Checksum = enable; };

public:
	void setReadSize(int readSize);
public:
	/// implementation of NativeThread virtual function
	int run(void);

private:
	void ftpConnRelease(bool removeFile=false);
	void emptyDataQueue(void);

	ZQ::common::Mutex                     _dataMutex;
	std::queue<ZQ::Content::BufferData*>  _dataQueue;

private:
	HANDLE                        _hStop;
	HANDLE                        _hNotify;
	HANDLE _hDesFile;
	
	DWORD                         _maxbps;
	DWORD                         _lasttimer;

	CInternetSession*             _pISession;
	CFtpConnection*               _pFtpConn;
	CInternetFile*	              _pFtpFile;

	DWORD                         _tidAbortBy;
	
	__int64                       _fileSize;

	DWORD                         _readSize;
	bool                          _bEndOfStream;

	HANDLE                        _hThdEnter;
	std::string                   _homeDirectory;

	std::string                   _szDesFileName; 
    std::string                   _szServer;
	std::string                   _szUserName;
	std::string                   _szPassword;
	unsigned int                  _nPort;
	// total processed bytes
	__int64                       _processedBytes;

	bool                          _enableMD5Checksum;
	ZQ::common::MD5ChecksumUtil   _md5ChecksumUtil;
};


} } }

#endif // !defined(AFX_FTPFILEIORENDER_H__F4A3F373_33E9_4259_B62A_9F7A535847FA__INCLUDED_)
