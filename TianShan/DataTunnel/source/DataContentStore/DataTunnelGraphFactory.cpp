#include "DataTunnelGraphFactory.h"

#include "DataTunnelDataSource.h"
#include "DataTsWrapperRender.h"
#include "NTFSFileIORender.h"


ZQ::Content::Process::Graph* DODGraphFactory::create()
{
	// create graph
	ZQ::Content::Process::Graph* graph = new ZQ::Content::Process::Graph(_graphLog, false, _buffPoolSize, _buffSize);	

	// create filters
	ZQ::Content::Process::DODDataSource* dodsrc = new ZQ::Content::Process::DODDataSource(*graph, _yieldTime);
	ZQ::Content::Process::DataTsWrapperRender* dodWrapper = new ZQ::Content::Process::DataTsWrapperRender(*graph);
	ZQ::Content::Process::NTFSFileIORender* ntfsIO = new ZQ::Content::Process::NTFSFileIORender(*graph);

	// DOD wrapper as the progress reporter
	graph->setProgressReporter(dodWrapper);
	
	// set IO's home directory
	ntfsIO->setHomeDirectory(_homeDirectory);

	// connect filters, without copying data between filters
	dodsrc->connectTo(dodWrapper, false);
	dodWrapper->connectTo(ntfsIO, false);

	return graph;
}


DODProvisionRequest::DODProvisionRequest(ZQ::common::Log* pLog, 
										 ZQ::Content::Process::GraphPool& pool, bool syncReq, 
										 std::string srcURL, std::string cntName, DWORD maxbps,
										 DWORD progressRptInterval)
: ZQ::Content::Process::ProvisionRequest(pool, syncReq, srcURL, cntName, 0, 0, maxbps, progressRptInterval, false),
_pLog(pLog)
{
	
}

DODProvisionRequest::~DODProvisionRequest()
{
}

bool DODProvisionRequest::init(ZQ::Content::Process::Graph& graph)
{	
	ZQ::Content::Process::DODDataSource* dodSource = (ZQ::Content::Process::DODDataSource*)graph.getSource();

	if(NULL == dodSource)
	{
		return false;
	}

	dodSource->setObjectParam(_basePID, _streamType, _streamCount, _sendIndexTable, 
								_objTag, _encryptType, _versionNum, _dataType);
	
	return true;
}

void DODProvisionRequest::setParameters(WORD pid, WORD streamType, WORD streamCount, bool sendIndexTable, 
										DWORD tag, int encryptType, BYTE versionNum, int dataType)
{
	_basePID = pid;
	_streamType = streamType;
	_encryptType = encryptType;
	_streamCount = streamCount;
	_sendIndexTable = sendIndexTable;
	_objTag = tag;
	_versionNum = versionNum;
	_dataType = dataType;
}

void DODProvisionRequest::OnProvisionStart()
{
	printf("%s provision start.\n", _contentName.c_str());
	if(_pLog != NULL)
	{
		(*_pLog)(ZQ::common::Log::L_DEBUG, "%s provision start.", _contentName.c_str());
	}
}

void DODProvisionRequest::OnProvisionStreamable()
{
	printf("%s provision streamable.\n", _contentName.c_str());
	if(_pLog != NULL)
	{
		(*_pLog)(ZQ::common::Log::L_DEBUG, "%s provision streamable.", _contentName.c_str());
	}
}

void DODProvisionRequest::OnProvisionProcess(__int64 processed, __int64 total)
{
	printf("%s provision objects progress %lld / %lld.\n", _contentName.c_str(), processed, total);
	if(_pLog != NULL)
	{
		(*_pLog)(ZQ::common::Log::L_DEBUG, "%s provision progress %lld / %lld.", _contentName.c_str(), processed, total);
	}
}

void DODProvisionRequest::OnPropertyUpdate(std::map<std::string, ZQ::common::Variant> property)
{
	std::map<std::string, ZQ::common::Variant>::iterator it = property.begin();
	for(; it != property.end(); it++)
	{
		printf("%s provision property %s updated\n", _contentName.c_str(), it->first.c_str());
		if(_pLog != NULL)
		{
			(*_pLog)(ZQ::common::Log::L_DEBUG, "%s provision property %s updated", _contentName.c_str(), it->first.c_str());
		}
	}
}

void DODProvisionRequest::OnProvisionCompleted(bool bSuccess, int errCode, std::string errStr)
{
	printf("%s provision completed.\n", _contentName.c_str());
	if(_pLog != NULL)
	{
		(*_pLog)(ZQ::common::Log::L_DEBUG, "%s provision completed.", _contentName.c_str());
	}
}
