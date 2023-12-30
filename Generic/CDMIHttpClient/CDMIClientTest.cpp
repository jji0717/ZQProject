// CDMIHttpClient.cpp : Defines the entry point for the console application.
//
#include <stdio.h>
#include <string>
#include <sstream>
#include "FileLog.h"
#include <sys/stat.h>
#include "CDMIHttpClient.h"
#include "CdmiFuseOps.h"
using namespace std;
using namespace ZQ::common;
using namespace ZQ::CDMIClient;
std::vector<CURLClient::Ptr> _curlClients;
static ZQ::common::NativeThreadPool* _thrdpool = NULL;
void CDMIPutFile(ZQ::common::Log& log)
{
	CDMIHttpClient::Ptr cdmiPtr;
//	std::string url = "http://10.50.5.180:8080/aqua/test1.txt";
	std::string url = "http://10.15.10.50:2364/test1.txt";
//    std::string url = "http://www163.com/test1.txt";
	int flags = 0;
	flags = CURLClient::sfEnableDebugInfo | CURLClient::sfEnableOutgoingDataCB;
	cdmiPtr =  new CDMIHttpClient((char*)url.c_str(),log, *_thrdpool, flags,CURLClient::HTTP_PUT, "putFile"); 
	if(cdmiPtr && !cdmiPtr->init())
	{
		cdmiPtr = NULL;
		return ;
	}
	cdmiPtr->setLocalIpPort("192.168.80.224");
	std::string  transferBuf = "This is the Value of this Data Object";
	cdmiPtr->setHeader("Content-Type", "text/plain;charset=utf-8");
	cdmiPtr->setReqData(transferBuf.c_str(), transferBuf.size());
	cdmiPtr->sendRequest();
	_curlClients.push_back(cdmiPtr);
}
void CDMIReadFile(ZQ::common::Log& log)
{
	CDMIHttpClient::Ptr cdmiPtr;
	std::string url = "http://10.50.5.180:8080/aqua/test1.txt";
	//std::string url = "http://10.15.10.85:10080/aqua/test1.txt";

	int flags = 0;
	flags = CURLClient::sfEnableDebugInfo;
	cdmiPtr =  new CDMIHttpClient((char*)url.c_str(),log, *_thrdpool, flags,CURLClient::HTTP_GET, "GetFile"); 
	if(cdmiPtr && !cdmiPtr->init())
	{
		cdmiPtr = NULL;
		return ;
	}
	cdmiPtr->setLocalIpPort("192.168.80.224");
	cdmiPtr->sendRequest();
	_curlClients.push_back(cdmiPtr);
}
void CDMIDeleteFile(ZQ::common::Log& log)
{
	CDMIHttpClient::Ptr cdmiPtr;
	std::string url = "http://10.50.5.180:8080/aqua/test1.txt";
	//std::string url = "http://10.15.10.85:10080/aqua/test1.txt";

	int flags = 0;
	flags = CURLClient::sfEnableDebugInfo;
	cdmiPtr =  new CDMIHttpClient((char*)url.c_str(),log, *_thrdpool, flags,CURLClient::HTTP_DEL, "DeleteFile"); 
    
	if(cdmiPtr && !cdmiPtr->init())
	{
		cdmiPtr = NULL;
		return ;
	}
	cdmiPtr->setLocalIpPort("192.168.80.224");
	cdmiPtr->sendRequest();
	_curlClients.push_back(cdmiPtr);
}

///cdmi_CreateContainer
void CreateCDMIContainer(CdmiFuseOps& cdmifuse, const std::string uri)
{	
	Json::Value result;
	CdmiFuseOps::Properties metadata;

	Json::Value exports;

	{
		Json::Value v;
		Json::Value::Members member;
		member.push_back("http://example.com/compute/0/");
		member.push_back("http://example.com/compute/1/");
		v["identifier"] = "AABRbgAVPcgvTXlDb250YWluZXIv";
        v["permissions"].append("http://example.com/compute/0/");
		v["permissions"].append("http://example.com/compute/1/");
		exports["OCCI/iSCSI"] = v;
	}
	{
		Json::Value v;
		v["identifier"]= "/users";
		v["permissions"] = "domain";
		exports["Network/NFSv4"] = v;
	}

	std::string domainURI="";
	std::string deserialize="";
	std::string copy="";
	std::string move="";
	std::string reference="";
	std::string deserializevalue="";

	cdmifuse.cdmi_CreateContainer(result, uri,metadata,exports,domainURI,
		deserialize, copy, move, reference, deserializevalue);
}
void CreateNonCDMIContainer(CdmiFuseOps& cdmifuse, const std::string uri)
{
	cdmifuse.nonCdmi_CreateContainer(uri);
}

void ReadCDMIContainer(CdmiFuseOps& cdmifuse, const std::string uri)
{
	Json::Value result;
	cdmifuse.cdmi_ReadContainer(result, uri);
}

void UpdateCDMIContainer(CdmiFuseOps& cdmifuse, const std::string uri)
{
	Json::Value result;
	std::string location;
	CdmiFuseOps::Properties metadata;
	Json::Value exports;
	{
		Json::Value v;
		Json::Value::Members member;
		member.push_back("http://example.com/compute/0/");
		member.push_back("http://example.com/compute/1/");
		v["identifier"] = "AABRbgAVPcgvTXlDb250YWluZXIv";
		v["permissions"].append("http://example.com/compute/0/");
		v["permissions"].append("http://example.com/compute/1/");
		exports["OCCI/iSCSI"] = v;
	}
	{
		Json::Value v;
		v["identifier"]= "/users";
		v["permissions"] = "domain";
		exports["Network/NFSv4"] = v;
	}
	std::string domainURI="";
	std::string deserialize="";
	std::string copy="";
	std::string snapshot="";
	std::string deserializevalue="";

	cdmifuse.cdmi_UpdateContainer(result, location,uri,  metadata, exports,
		domainURI, deserialize,copy,snapshot, deserializevalue);
}

void DeleteCDMIContainer(CdmiFuseOps& cdmifuse, const std::string uri)
{
	Json::Value result;
	cdmifuse.cdmi_DeleteContainer(result, uri);
}

void DeleteNonCDMIContainer(CdmiFuseOps& cdmifuse, const std::string uri)
{
	cdmifuse.noncdmi_DeleteContainer(uri);
}

void CreateCDMIDataObject(CdmiFuseOps& cdmifuse, const std::string uri)
{	
	Json::Value result;
	std::string mimetype = "text/plain";
//	std::string mimetype = "application/cdmi-object";
	CdmiFuseOps::Properties metadata;
	std::string value = "This is the Value of this Data Object";
	CdmiFuseOps::StrList valuetransferencoding = CdmiFuseOps::StrList();

	std::string domainURI="";
	std::string deserialize="";
	std::string serialize="";
	std::string copy="";
	std::string move="";
	std::string reference="";
	std::string deserializevalue="";
	cdmifuse.cdmi_CreateDataObject(result, uri, mimetype, metadata, value, 
		valuetransferencoding,domainURI, deserialize,
		serialize,copy,move,reference,deserializevalue);
}
void CreateNonCDMIDataObject(CdmiFuseOps& cdmifuse, const std::string uri)
{
	std::string contentType = "text/plain;charset=utf-8" ;
	std::string value = "This is the Value of this Data Object";
	cdmifuse.nonCdmi_CreateDataObject(uri, contentType, (void*)value.c_str(), (uint)value.size());
}
void ReadCDMIDataObject(CdmiFuseOps& cdmifuse, const std::string uri)
{
	Json::Value result;
	std::string location = "";
	cdmifuse.cdmi_ReadDataObject(result, uri, location);
}

void ReadNonCDMIDataObject(CdmiFuseOps& cdmifuse, const std::string uri)
{
	std::string contentType = "";
	std::string location;
	uint64 startOffset = 0;
	uint len = 1024;
	char recvbuff[1024] = "";
	cdmifuse.nonCdmi_ReadDataObject(uri,contentType,location, startOffset, len, recvbuff);
}

void UpdateCDMIDataObject(CdmiFuseOps& cdmifuse, const std::string uri)
{
	Json::Value result;
	std::string location;
	CdmiFuseOps::Properties metadata;
	uint64 startOffset = 0;
	std::string value = "This is the Value of this Data Object";
	bool partial = false;
	CdmiFuseOps::StrList valuetransferencoding = CdmiFuseOps::StrList();
	std::string domainURI="";
	std::string deserialize="";
	std::string copy="";
	std::string deserializevalue="";
	cdmifuse.cdmi_UpdateDataObject(result,location,uri,metadata,
		startOffset, value, partial, valuetransferencoding,
		domainURI, deserialize, copy,deserializevalue);
}

void UpdateNonCDMIDataObject(CdmiFuseOps& cdmifuse, const std::string uri)
{
	std::string location;
	std::string contentType = "text/plain;charset=utf-8";
	uint64 startOffset = 0;
	char buff[11]= "0123456789";
	int64 objectSize=-1;
	bool partial=false;

	cdmifuse.nonCdmi_UpdateDataObject(uri,location, contentType, startOffset, sizeof(buff), buff, objectSize, partial);
}
void DeleteCDMIDataObject(CdmiFuseOps& cdmifuse, const std::string uri)
{
	Json::Value result;
	cdmifuse.cdmi_DeleteDataObject(result, uri);

}
void DeleteNonCDMIDataObject(CdmiFuseOps& cdmifuse, const std::string uri)
{
	cdmifuse.noncdmi_DeleteDataObject(uri);
}
int main(int argc, char* argv[])
{
	_thrdpool = new ZQ::common::NativeThreadPool(20);
	if(!_thrdpool)
		return 0;

	CURLClient::startCurlClientManager();
    
	ZQ::common::FileLog testLogFile("c:\\CDMIHttpClient.log" , 7);

	//CdmiFuseOps cdmifuse(testLogFile, "http://10.50.5.180:8080", *_thrdpool);

	CdmiFuseOps cdmifuse(testLogFile, "http://10.15.10.50:2364", *_thrdpool);

	CreateCDMIContainer(cdmifuse, "MyContainerCDMI/");
	CreateNonCDMIContainer(cdmifuse, "MyContainerNonCDMI/");

	CreateCDMIDataObject(cdmifuse, "MyContainerCDMI/MyDataObject.txt");
	CreateNonCDMIDataObject(cdmifuse, "MyContainerNonCDMI/MyDataObject.txt");
	ReadCDMIDataObject(cdmifuse, "MyContainerCDMI/MyDataObject.txt");
	ReadNonCDMIDataObject(cdmifuse, "MyContainerNonCDMI/MyDataObject.txt");
	UpdateCDMIDataObject(cdmifuse, "MyContainerCDMI/MyDataObject.txt");
	UpdateNonCDMIDataObject(cdmifuse, "MyContainerNonCDMI/MyDataObject.txt");

	ReadCDMIContainer(cdmifuse, "MyContainerCDMI/");
	UpdateCDMIContainer(cdmifuse, "MyContainerCDMI/");

	DeleteCDMIDataObject(cdmifuse, "MyContainerCDMI/MyDataObject.txt");
	DeleteNonCDMIDataObject(cdmifuse, "MyContainerNonCDMI/MyDataObject.txt");

	DeleteCDMIContainer(cdmifuse, "MyContainerCDMI/");
	DeleteNonCDMIContainer(cdmifuse, "MyContainerNonCDMI/");

/*
//	CDMIPutFile(testLogFile);
//  CDMIReadFile(testLogFile);
//	CDMIDeleteFile(testLogFile);
	size_t size = CURLClient::getCURLClientSize();
	while(size)
	{
		printf("Current CDMIHttpClient Count: %d\n", size);
		Sleep(1000);
		size = CURLClient::getCURLClientSize();
	}
	for(int i = 0 ;i< _curlClients.size();i++)
	{
		//if(curlClients[i]->getStatusCode() == 0)
		std::string strResStatus;
		printf("***%05d*** ERROR:[%s] StatusCode:[%d]\n", i+1, _curlClients[i]->getErrorMessage().c_str(), _curlClients[i]->getStatusCode(strResStatus));	
		_curlClients[i] =  NULL;
	}*/

	CURLClient::stopCurlClientManager();
	return 0;
}