#include "GraphPool.h"

class DODGraphFactory : public ZQ::Content::Process::GraphFactory
{
public:
	DODGraphFactory(int graphCount, ZQ::common::Log* pLog = NULL, 
		            std::string homeDir = "", DWORD yieldTime = 0, 
					int buffPoolSize = DEFAULT_BUFFER_POOL_SIZE, int buffSize = DEFAULT_POOL_BUFFER_SIZE)
					: ZQ::Content::Process::GraphFactory(graphCount, pLog, buffPoolSize, buffSize), 
					_homeDirectory(homeDir), _yieldTime(yieldTime) {};
	virtual ~DODGraphFactory(){};

public:
	virtual ZQ::Content::Process::Graph* create();

protected:
	std::string _homeDirectory;
	DWORD       _yieldTime;
};


class DODProvisionRequest : public ZQ::Content::Process::ProvisionRequest
{
public:
	DODProvisionRequest(ZQ::common::Log* pLog, 
		                ZQ::Content::Process::GraphPool& pool, bool syncReq, 
						std::string srcURL, std::string cntName, DWORD maxbps=0,
						DWORD progressRptInterval = DEFAULT_PROV_PROGRESS_RPT_INTERVAL);

protected:
	virtual ~DODProvisionRequest();

public:	
	// this function must invoked before start()
	void setParameters(WORD pid, WORD streamType, WORD streamCount, bool sendIndexTable, 
						DWORD tag, int encryptType, BYTE versionNum, int dataType);

	bool init(ZQ::Content::Process::Graph& graph);

public:
	// virtual function for asynchronised callback 
	virtual void OnProvisionStart();
	virtual void OnProvisionStreamable();
	virtual void OnProvisionProcess(__int64 processed, __int64 total);
	virtual void OnPropertyUpdate(std::map<std::string, ZQ::common::Variant> property);
    virtual void OnProvisionCompleted(bool bSuccess, int errCode, std::string errStr);
	
private:
	ZQ::common::Log* _pLog;

	WORD                          _basePID;
	WORD                          _streamType;
	DWORD                         _encryptType;
	WORD                          _streamCount;
	bool                          _sendIndexTable;
	DWORD                         _objTag;
	BYTE                          _versionNum;

	int                           _dataType;
};