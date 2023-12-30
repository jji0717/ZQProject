#include "TestFactoryAndRequest.h"

#include "GraphFilter.h"
#include "NTFSFileIOSource.h"
#include "NTFSFileIORender.h"
#include "FTPFileIOSource.h"
#include "FTPFileIORender.h"
#include "VstrmIORender.h"
#include "RTFLibFilter.h"

#define ZQCP  ::ZQ::Content::Process

ZQCP::Graph* FTPGraphFactory::create()
{
	// create graph
	ZQCP::Graph* graph = new ZQCP::Graph(_graphLog, false, _buffPoolSize, _buffSize);	

	// create filters
	ZQCP::NTFSFileIOSource* sourceFilter = new ZQCP::NTFSFileIOSource(*graph);
	ZQCP::FTPFileIORender* ftpIORender = new ZQCP::FTPFileIORender(*graph, true);
	
	// the IO render as the progress reporter
	graph->setProgressReporter(ftpIORender);
	
	// set IO's home directory
	ftpIORender->setAccessProperty(_ftpServer, _userName, _password);

	// connect filters
	sourceFilter->connectTo(ftpIORender, false);

	return graph;
}

ZQCP::Graph* VstrmGraphFactory::create()
{
	// create graph
	ZQCP::Graph* graph = new ZQCP::Graph(_graphLog, false, _buffPoolSize, _buffSize);	

	// create filters
	ZQCP::NTFSFileIOSource* fileSourceFilter = new ZQCP::NTFSFileIOSource(*graph);
	ZQCP::FTPFileIOSource* ftpSourceFilter = new ZQCP::FTPFileIOSource(*graph);
	// add filters to graph
	
	ZQCP::VstrmIORender* vstrmIORender = new ZQCP::VstrmIORender(*graph, INVALID_HANDLE_VALUE, TEST_CLIENT_ID);

	// the IO render as the progress reporter
	graph->setProgressReporter(vstrmIORender);
	
	// connect filters
	fileSourceFilter->connectTo(vstrmIORender, false);
	ftpSourceFilter->connectTo(vstrmIORender, false);

	return graph;
}

ZQCP::Graph* RTFNTFSGraphFactory::create()
{
	// create graph
	ZQCP::Graph* graph = new ZQCP::Graph(_graphLog, false, _buffPoolSize, _buffSize);	

	int trickSpeedsCount = 1;
	// create filters
	ZQCP::NTFSFileIOSource* ntfsSourceFilter = new ZQCP::NTFSFileIOSource(*graph);
	ZQCP::FTPFileIOSource* ftpSourceFilter = new ZQCP::FTPFileIOSource(*graph);

	ZQCP::RTFLibFilter* rtfLibFilter = new ZQCP::RTFLibFilter(*graph, false, trickSpeedsCount);

	// ZQCP::VstrmIORender* vstrmIORender = new ZQCP::VstrmIORender(*graph, INVALID_HANDLE_VALUE, TEST_CLIENT_ID);
	
	ntfsSourceFilter->setReadSize(8*1024);
	ftpSourceFilter->setReadSize(8*1024);

	// the IO render as the progress reporter
	graph->setProgressReporter(rtfLibFilter);
	
	// connect filters
	ntfsSourceFilter->connectTo(rtfLibFilter, true);
	ftpSourceFilter->connectTo(rtfLibFilter, true);
	
//	ZQCP::NTFSFileIORender* mainFileRen = new ZQCP::NTFSFileIORender(*graph);
//
//	ntfsSourceFilter->connectTo(mainFileRen, false);
//	ftpSourceFilter->connectTo(mainFileRen, false);
//	mainFileRen->setHomeDirectory("D:\\TestCS\\SourceFile\\");
	
	char extension[16];
	int fileCount = rtfLibFilter->getOutputFileCount();

	for(int i=0; i<fileCount; i++)
	{
		ZQCP::NTFSFileIORender* trickFileRen= new ZQCP::NTFSFileIORender(*graph);

		rtfLibFilter->connectTo(trickFileRen, false);
		
		DWORD speedNumerator = 0;
		DWORD speedDenominator = 1;
		bool bIndexFile;
		rtfLibFilter->getOutputFileInfo(i, extension, 16, speedNumerator, speedDenominator, bIndexFile);

		trickFileRen->setHomeDirectory("D:\\TestCS\\SourceFile\\");
		trickFileRen->setFileExtension(extension);
	}

	return graph;
}

ZQCP::Graph* RTFVstrmGraphFactory::create()
{	
	bool oneVstrmIORender = true;

	// create graph
	ZQCP::Graph* graph = new ZQCP::Graph(_graphLog, false, _buffPoolSize, _buffSize);	

	int trickSpeedsCount = 1;
	// create filters
	ZQCP::NTFSFileIOSource* ntfsSourceFilter = new ZQCP::NTFSFileIOSource(*graph);
	ZQCP::FTPFileIOSource* ftpSourceFilter = new ZQCP::FTPFileIOSource(*graph);
	ntfsSourceFilter->setReadSize(8*1024);
	ftpSourceFilter->setReadSize(8*1024);

	ZQCP::RTFLibFilter* rtfLibFilter = new ZQCP::RTFLibFilter(*graph, true, trickSpeedsCount);
	// the IO render as the progress reporter
	graph->setProgressReporter(rtfLibFilter);
		
	// connect filters
	ntfsSourceFilter->connectTo(rtfLibFilter, true);
	ftpSourceFilter->connectTo(rtfLibFilter, true);
	
	// main file output
	ZQCP::VstrmIORender* mainFileRen = new ZQCP::VstrmIORender(*graph, _vstrmHandle, TEST_CLIENT_ID);
	ntfsSourceFilter->connectTo(mainFileRen, false);
	ftpSourceFilter->connectTo(mainFileRen, false);
	mainFileRen->setSubFileCount(1);
	mainFileRen->setSubFileInfo(0, DEF_VSTRM_WRITE_SIZE, true);
	
	ZQCP::VstrmIORender* trickFileRen = new ZQCP::VstrmIORender(*graph, _vstrmHandle, TEST_CLIENT_ID);
	rtfLibFilter->connectTo(trickFileRen, false);
	
	char extension[16];
	int fileCount = rtfLibFilter->getOutputFileCount();
	trickFileRen->setSubFileCount(fileCount);  

	for(int i=0; i<fileCount; i++)
	{
		bool bIndexFile = false;
		DWORD speedNumerator = 0;
		DWORD speedDenominator = 1;
		rtfLibFilter->getOutputFileInfo(i, extension, 16, speedNumerator, speedDenominator, bIndexFile);

		if(bIndexFile)
			trickFileRen->setSubFileInfo(i, 0, false, extension, 0, 1);
		else
			trickFileRen->setSubFileInfo(i, DEF_VSTRM_WRITE_SIZE, false, extension, speedDenominator, speedNumerator, false);
	}

	return graph;
}

TestProvisionRequest::TestProvisionRequest(ZQ::common::Log* pLog, 
										 ZQCP::GraphPool& pool, bool syncReq, 
										 std::string srcURL, std::string cntName, 
										 DWORD progressRptInterval, DWORD expectedExecTime, 
										 DWORD maxbps, bool autoFree)
: ZQCP::ProvisionRequest(pool, syncReq, srcURL, cntName, progressRptInterval, expectedExecTime, maxbps, autoFree),
_pLog(pLog)
{
	
}

TestProvisionRequest::~TestProvisionRequest()
{
}

bool TestProvisionRequest::init(ZQCP::Graph& graph)
{	
	return true;
}


void TestProvisionRequest::OnProvisionStart()
{
	printf("%s provision start.\n", _contentName.c_str());
	if(_pLog != NULL)
	{
		(*_pLog)(ZQ::common::Log::L_DEBUG, "%s provision start.", _contentName.c_str());
	}
}

void TestProvisionRequest::OnProvisionStreamable()
{
	printf("%s provision streamable.\n", _contentName.c_str());
	if(_pLog != NULL)
	{
		(*_pLog)(ZQ::common::Log::L_DEBUG, "%s provision streamable.", _contentName.c_str());
	}
}

void TestProvisionRequest::OnProvisionProcess(__int64 processed, __int64 total)
{
	printf("%s provision objects progress %I64d / %I64d.\n", _contentName.c_str(), processed, total);
	if(_pLog != NULL)
	{
		(*_pLog)(ZQ::common::Log::L_DEBUG, "%s provision progress %I64d / %I64d.", _contentName.c_str(), processed, total);
	}
}

void TestProvisionRequest::OnPropertyUpdate(std::map<std::string, ZQ::common::Variant> property)
{
	std::map<std::string, ZQ::common::Variant>::iterator it = property.begin();
	for(; it != property.end(); it++)
	{
		char propertyvalue[256];
		
		memset(propertyvalue, 0x00, 256*sizeof(char));
		
		if(it->second.type() == ZQ::common::Variant::T_STRING)
		{
			std::string str = std::string(it->second);
			sprintf(propertyvalue, "%s", str.c_str());
		}
		else if(it->second.type() == ZQ::common::Variant::T_LONGLONG)
		{
			sprintf(propertyvalue, "%I64d", __int64(it->second));

			if(it->first == ZQ::Content::Process::CNTPRY_FILESIZE)
			{
				_fileSize = __int64(it->second);
			}
		}
		printf("%s provision property %s updated, value is %s \n", _contentName.c_str(), it->first.c_str(), propertyvalue);
		if(_pLog != NULL)
		{
			(*_pLog)(ZQ::common::Log::L_DEBUG, "%s provision property %s updated, value is %s \n", _contentName.c_str(), it->first.c_str(), propertyvalue);
		}
	}
}

void TestProvisionRequest::OnProvisionCompleted(bool bSuccess, int errCode, std::string errStr)
{
	printf("%s provision completed.\n", _contentName.c_str());
	if(_pLog != NULL)
	{
		if(bSuccess)
		{
			(*_pLog)(ZQ::common::Log::L_DEBUG, "%s provision completed.", _contentName.c_str());
		}
		else
		{
			(*_pLog)(ZQ::common::Log::L_DEBUG, "%s provision failed with errorcode: %d reason: %s.", 
				      _contentName.c_str(), errCode, errStr.c_str());
		}
	}
}
