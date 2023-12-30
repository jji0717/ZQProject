#ifndef __FTPGraphFactory_H__
#define __FTPGraphFactory_H__

#define  TEST_CLIENT_ID   10000

#include "GraphPool.h"


class FTPGraphFactory : public ZQ::Content::Process::GraphFactory
{
public:
	FTPGraphFactory(int graphCount, ZQ::common::Log* pLog, 
		            std::string ftpServer, std::string userName, std::string password, 
					int buffPoolSize = DEFAULT_BUFFER_POOL_SIZE, int buffSize = DEFAULT_POOL_BUFFER_SIZE)
					: ZQ::Content::Process::GraphFactory(graphCount, pLog, buffPoolSize, buffSize), 
					_ftpServer(ftpServer), _userName(userName), _password(password) {};
	virtual ~FTPGraphFactory(){};

public:
	virtual ZQ::Content::Process::Graph* create();

protected:
	std::string _ftpServer;
	std::string _userName;
	std::string _password;
};

class VstrmGraphFactory : public ZQ::Content::Process::GraphFactory
{
public:
	VstrmGraphFactory(int graphCount, ZQ::common::Log* pLog, 
					int buffPoolSize = DEFAULT_BUFFER_POOL_SIZE, int buffSize = DEFAULT_POOL_BUFFER_SIZE)
					: ZQ::Content::Process::GraphFactory(graphCount, pLog, buffPoolSize, buffSize)
					{};
	virtual ~VstrmGraphFactory(){};

public:
	virtual ZQ::Content::Process::Graph* create();
};

class RTFNTFSGraphFactory: public ZQ::Content::Process::GraphFactory
{
public:
	RTFNTFSGraphFactory(int graphCount, ZQ::common::Log* pLog, 
					int buffPoolSize = DEFAULT_BUFFER_POOL_SIZE, int buffSize = DEFAULT_POOL_BUFFER_SIZE)
					: ZQ::Content::Process::GraphFactory(graphCount, pLog, buffPoolSize, buffSize)
					{};
	virtual ~RTFNTFSGraphFactory(){};

public:
	virtual ZQ::Content::Process::Graph* create();

};

class RTFVstrmGraphFactory : public ZQ::Content::Process::GraphFactory
{
public:
	RTFVstrmGraphFactory(int graphCount, ZQ::common::Log* pLog, HANDLE vstrmHandle = INVALID_HANDLE_VALUE,
					int buffPoolSize = DEFAULT_BUFFER_POOL_SIZE, int buffSize = DEFAULT_POOL_BUFFER_SIZE)
					: ZQ::Content::Process::GraphFactory(graphCount, pLog, buffPoolSize, buffSize),
					_vstrmHandle(vstrmHandle)
					{};
	virtual ~RTFVstrmGraphFactory(){};

public:
	virtual ZQ::Content::Process::Graph* create();

private:
	HANDLE _vstrmHandle;
};

class TestProvisionRequest : public ZQ::Content::Process::ProvisionRequest
{
public:
	TestProvisionRequest(ZQ::common::Log* pLog, 
		                ZQ::Content::Process::GraphPool& pool, bool syncReq, 
						std::string srcURL, std::string cntName, 
						DWORD progressRptInterval = DEFAULT_PROV_PROGRESS_RPT_INTERVAL, DWORD expectedExecTime = INFINITE, 
						DWORD maxbps=0, bool autoFree=true);

protected:
	virtual ~TestProvisionRequest();

public:	

	bool init(ZQ::Content::Process::Graph& graph);

public:
	// virtual function for asynchronised callback 
	virtual void OnProvisionStart();
	virtual void OnProvisionStreamable();
	virtual void OnProvisionProcess(__int64 processed, __int64 total);
	virtual void OnPropertyUpdate(std::map<std::string, ZQ::common::Variant> property);

	virtual void OnProvisionCompleted(bool bSuccess, int errCode, std::string errStr);

public:
	__int64 getContentSize() { return _fileSize; };
private:
	__int64                       _fileSize;

private:
	ZQ::common::Log* _pLog;

	WORD                          _basePID;
	WORD                          _streamType;
	DWORD                         _encryptType;
	WORD                          _streamCount;
	bool                          _sendIndexTable;
	DWORD                         _objTag;

};

#endif // __ZQ_VstrmIORender_Process_H__
