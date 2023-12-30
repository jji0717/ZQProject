#include "MCastGraph.h"
#include "RTFLibFilter.h"
#include "TrickFileGenFilter.h"
#include "NTFSFileIORender.h"
#include "VstrmIORender.h"
#include "MCastIOSource.h"


ZQCP::Graph* MCastGraphFactory::create() 
{
	ZQCP::Graph* graph = new ZQCP::Graph(_graphLog, false, _buffPoolSize, _buffSize);	
	
	ZQCP::MCastIOSource* mcastSource = new ZQCP::MCastIOSource(*graph, _timeout*1000);

	if(_dumpFile) {
		ZQCP::NTFSFileIORender* fileRender = new ZQCP::NTFSFileIORender(*graph);
		mcastSource->connectTo(fileRender, true);
		fileRender->setFileExtension(".dump");
	}
	
	if(_oldTrickType)
	{
		// the buffer read size must be X * MPEG_TS_PACKET_SIZE
		DWORD readTSCount = _buffSize / MPEG_TS_PACKET_SIZE;
		DWORD dataReadSize = readTSCount * MPEG_TS_PACKET_SIZE;

		// reset the read size, since to old trick library, the buffer size must be n * 188
		mcastSource->setReadSize(dataReadSize);

		// old trick library
		ZQCP::TrickFileGenFilter* trickFilter = new ZQCP::TrickFileGenFilter(*graph, true, 1, dataReadSize, _streamablePlaytime, false);
		graph->setProgressReporter(trickFilter);
		mcastSource->connectTo(trickFilter, false);

		if(_runningOnNode)
		{
			ZQCP::VstrmIORender* vstrmRender = new ZQCP::VstrmIORender(
							*graph, INVALID_HANDLE_VALUE, DEFAULT_BW_CLIENT_ID, true, 10);
			trickFilter->connectTo(vstrmRender, false);
			
			char extension[16];
			int fileCount = trickFilter->getOutputFileCount();
			vstrmRender->setSubFileCount(fileCount);  

			for(int i = 0; i < fileCount; ++i) 
			{
				ZQCP::TrickFileGenFilter::OutputFileType fileType;
				DWORD speedNumerator = 0;
				DWORD speedDenominator = 1;
				
				trickFilter->getOutputFileInfo(
						i, extension, 16, speedNumerator, speedDenominator, fileType);

				if(ZQCP::TrickFileGenFilter::OPTFT_VVX == fileType) {
					vstrmRender->setSubFileInfo(i, 0, false, extension, 0, 1);
				}
				else 
				{
					bool enableMD5 = false;
					if(ZQCP::TrickFileGenFilter::OPTFT_MAIN == fileType) 
					{
						enableMD5 =true;
					}

					vstrmRender->setSubFileInfo(
							i, 
							DEF_VSTRM_WRITE_SIZE, 
							enableMD5, 
							extension, 
							speedDenominator, 
							speedNumerator, 
							false);
				}
			}
		}  // if(_runningOnNode)
		else
		{
			char extension[16];
			int fileCount = trickFilter->getOutputFileCount();

			for(int i=0; i<fileCount; i++)
			{
				ZQCP::NTFSFileIORender* trickFileRen= new ZQCP::NTFSFileIORender(*graph, false);

				trickFilter->connectTo(trickFileRen, false);
				
				DWORD speedNumerator = 0;
				DWORD speedDenominator = 1;
				ZQCP::TrickFileGenFilter::OutputFileType fileType;
				trickFilter->getOutputFileInfo(i, extension, 16, speedNumerator, speedDenominator, fileType);

				if(ZQCP::TrickFileGenFilter::OPTFT_VVX == fileType)
				{
					trickFileRen->vvxFileByTrickGen();
				}
				trickFileRen->setFileExtension(extension);
			}		
		}
	}
	else
	{
		ZQCP::RTFLibFilter* trickFilter = new ZQCP::RTFLibFilter(*graph);
		graph->setProgressReporter(trickFilter);
		mcastSource->connectTo(trickFilter, false);

		if(_runningOnNode)
		{
			ZQCP::VstrmIORender* vstrmRender = new ZQCP::VstrmIORender(
							*graph, INVALID_HANDLE_VALUE, DEFAULT_BW_CLIENT_ID, true, _streamablePlaytime);
			trickFilter->connectTo(vstrmRender, false);
			
			char extension[16];
			int fileCount = trickFilter->getOutputFileCount();
			vstrmRender->setSubFileCount(fileCount);  

			for(int i = 0; i < fileCount; ++i) {
				ZQCP::RTFLibFilter::OutputFileType fileType;
				DWORD speedNumerator = 0;
				DWORD speedDenominator = 1;
				
				trickFilter->getOutputFileInfo(
						i, extension, 16, speedNumerator, speedDenominator, fileType);

				if(ZQCP::RTFLibFilter::OPTFT_VVX == fileType || ZQCP::RTFLibFilter::OPTFT_VV2 == fileType) {
					vstrmRender->setSubFileInfo(i, 0, false, extension, 0, 1);
				}
				else {
					bool enableMD5 = false;
					if(ZQCP::RTFLibFilter::OPTFT_MAIN == fileType) {
						enableMD5 =true;
					}

					vstrmRender->setSubFileInfo(
							i, 
							DEF_VSTRM_WRITE_SIZE, 
							enableMD5, 
							extension, 
							speedDenominator, 
							speedNumerator, 
							false);
				}
			}
		}// if(_runningOnNOde)
		else
		{
			// add trick files io render
			char extension[16];
			int fileCount = trickFilter->getOutputFileCount();

			for(int i=0; i<fileCount; i++)
			{
				ZQCP::RTFLibFilter::OutputFileType fileType;
				DWORD speedNumerator = 0;
				DWORD speedDenominator = 1;
				trickFilter->getOutputFileInfo(i, extension, 16, speedNumerator, speedDenominator, fileType);

				ZQCP::NTFSFileIORender* trickFileRen= new ZQCP::NTFSFileIORender(*graph, false);
				
				trickFileRen->setFileExtension(extension);

				trickFilter->connectTo(trickFileRen, false);
			}

		}
	}
	return graph;
}


void MCastRequest::OnProvisionStart() {
	printf("[%s] provision started\n", _contentName.c_str());
}

void MCastRequest::OnProvisionStreamable() {
	printf("[%s] provision streamable\n", _contentName.c_str());
}

void MCastRequest::OnProvisionProcess(__int64 processed, __int64 total) {
	printf("[%s] provision progress %I64d / %I64d \n", 
		_contentName.c_str(), processed, total);
}

void MCastRequest::OnPropertyUpdate(std::map<std::string, ZQ::common::Variant> property) {

	int i=0;
	std::map<std::string, ZQ::common::Variant>::iterator it = property.begin();
	for(; it != property.end(); it++)
	{
		char propertyvalue[256];
		propertyvalue[0] = '\0';
		
		switch(it->second.type())
		{
		case ZQ::common::Variant::T_STRING:
			{
				std::string str = std::string(it->second);
				sprintf(propertyvalue, "%s", str.c_str());
			}
			break;
		case ZQ::common::Variant::T_LONGLONG:
			{
				sprintf(propertyvalue, "%I64d", (__int64)(it->second));
			}
			break;
		case ZQ::common::Variant::T_ULONG:
			{
				sprintf(propertyvalue, "%d", (unsigned long)(it->second));
			}
			break;
		case ZQ::common::Variant::T_DOUBLE:
			{
				sprintf(propertyvalue, "%f", (double)(it->second));
			}
			break;
		default:
			return;
		}

		printf("[%s] provision property updated, [%s] = [%s]\n", 
			_contentName.c_str(), it->first.c_str(), propertyvalue);
	}
}

void MCastRequest::OnProvisionCompleted(bool bSuccess, int code, std::string errStr) {
	printf("[%s] provision completed\n", _contentName.c_str());
}
