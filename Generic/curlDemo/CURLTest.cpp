#include <stdio.h>
#include <string>
#include <sstream>
#include "CURLClient.h"
#include "FileLog.h"
#include <sys/stat.h>
using namespace std;
using namespace ZQ::common;

static ZQ::common::NativeThreadPool* _thrdpool = NULL;

////////////////////////////////////////////////////////////////////////////////
/////////////////// class CURLClientChunk //////////////////////////////////////
////////////////DownLoad File by chunk mode ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class CURLClientChunk: public CURLClient
{
public:
	typedef ZQ::common::Pointer < CURLClientChunk > Ptr;
	CURLClientChunk(std::string &filePath, char* url, ZQ::common::Log& log, ZQ::common::NativeThreadPool& thrdpool, int flag = 0, HTTPMETHOD method = HTTP_GET, std::string clientId ="", char* bindIp = "", int port = 0);
	virtual ~CURLClientChunk(void){};
public:
	   bool init();
protected:
	virtual int  OnDataArrived(char *data, size_t size);
	virtual void OnTxnCompleted(CURLcode code);

private:
	FILE* _pWriteFile;
	std::string _filepath;
};

#define CLOG  (_log)
#define CURLFMT( _X) CLOGFMT(CURLClientChunk, "ClientId[%s]Socket[%d] " _X), _clientId.c_str(),_fd

CURLClientChunk::CURLClientChunk(std::string& filePath,char* url, ZQ::common::Log& log, ZQ::common::NativeThreadPool& thrdpool, int flag , HTTPMETHOD method, std::string clientId, char* bindIp, int port):
CURLClient(url, log, thrdpool,flag,method, clientId, bindIp, port), _pWriteFile(NULL), _filepath(filePath)
{

}
bool CURLClientChunk::init()
{
	_pWriteFile = fopen(_filepath.c_str(), "wb");
	if(!_pWriteFile)
	{ 
		CLOG(ZQ::common::Log::L_ERROR, CURLFMT("failed to create output file[%s]"), _filepath.c_str());
		return false;
	}
	return CURLClient::init();
}
int  CURLClientChunk::OnDataArrived(char *data, size_t size)
{
	if(!_pWriteFile)
	{
		CLOG(ZQ::common::Log::L_ERROR, CURLFMT("NULL output file[%s] handle"), _filepath.c_str());
		return 0;
	}
	unsigned int nWrite = 0;
	nWrite = fwrite(data, 1 , size, _pWriteFile);
	return nWrite;
}
void CURLClientChunk::OnTxnCompleted(CURLcode code)
{
	CURLClient::OnTxnCompleted(code);
	if(code == CURLE_OK)
	{
		CLOG(ZQ::common::Log::L_INFO, CURLFMT("[%s]DownLoad file[%s] successful"), _url.c_str(), _filepath.c_str());
	}
	else
		CLOG(ZQ::common::Log::L_INFO, CURLFMT("[%s]DownLoad file[%s] failed"), _url.c_str(), _filepath.c_str());

	if(_pWriteFile)
		fclose(_pWriteFile);
	_pWriteFile = NULL;
}


CURLClientChunk::Ptr createCURLClientChunk(ZQ::common::Log& log)
{
	CURLClientChunk::Ptr curlclient = NULL;
	std::string url = "http://10.15.10.85:12000/scs/getfile?file=cdntest1234567895521zq.com.FF&ic=1000000000&rate=16000000";
	static int i = 1;
	char buf[65] = "";
	itoa(i, buf, 10);

	std::string filepath = std::string("f:\\testCURLClientChuck\\") + buf;
	int flags = 0;
//	flags = CURLClient::sfEnableDebugInfo;
	curlclient =  new CURLClientChunk(filepath,(char*)url.c_str(),log, *_thrdpool, flags,CURLClient::HTTP_GET, buf); 
	if(!curlclient->init())
	{
		curlclient = NULL;
		return NULL;
	}
	curlclient->setHeader("User-Agent", "Test User Agent");
	curlclient->sendRequest();
	i++;
	return  curlclient;
}
///////////////////////////////////////////////////////////////////////////////////////
///////////////////End class CURLClientChuck //////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////
/////////////////// class CURLClientPost //////////////////////////////////////////////
////////////////post content by read callback /////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
class CURLClientPost: public CURLClient
{
public:
	typedef ZQ::common::Pointer < CURLClientPost > Ptr;
	CURLClientPost(std::string &filePath, char* url, ZQ::common::Log& log, ZQ::common::NativeThreadPool& thrdpool, int flag = 0, HTTPMETHOD method = HTTP_GET, std::string clientId ="", char* bindIp = "", int port = 0);
	virtual ~CURLClientPost(void){};
public:
	bool init();
protected:
	virtual int  OnPrepareOutgoingData(void *data, size_t size);
	virtual int  OnDataArrived(char *data, size_t size);
	virtual void OnTxnCompleted(CURLcode code);

private:
	FILE* _pReadFile;
	std::string _filepath;
	std::string _resPonseBody;
};

#define CLOG  (_log)
#define CURLFMT( _X) CLOGFMT(CURLClientPost, "ClientId[%s]Socket[%d] " _X), _clientId.c_str(),_fd

CURLClientPost::CURLClientPost(std::string& filePath,char* url, ZQ::common::Log& log, ZQ::common::NativeThreadPool& thrdpool, int flag , HTTPMETHOD method, std::string clientId, char* bindIp, int port):
CURLClient(url, log, thrdpool,flag,method, clientId, bindIp, port), _pReadFile(NULL), _filepath(filePath)
{
	_resPonseBody = "";
}

bool CURLClientPost::init()
{
	 if(!CURLClient::init())
		 return false;

	_pReadFile = fopen(_filepath.c_str(), "rb");
	if(!_pReadFile)
	{ 
		CLOG(ZQ::common::Log::L_ERROR, CURLFMT("failed to read file[%s]"), _filepath.c_str());
		return false;
	}
	struct _stat file_info;
	_stat(_filepath.c_str(),  &file_info);

	return setReqDataSize(file_info.st_size);
}

int  CURLClientPost::OnPrepareOutgoingData(void *data, size_t size)
{
	if(!_pReadFile)
	{
		CLOG(ZQ::common::Log::L_ERROR, CURLFMT("NULL read file[%s] handle"), _filepath.c_str());
		return 0;
	}
	unsigned int nRead = 0;
	nRead = fread(data, 1 , size, _pReadFile);
	return nRead;
}
int  CURLClientPost::OnDataArrived(char *data, size_t size)
{
    _resPonseBody.append(data, size);
	return (int)size;
}
void CURLClientPost::OnTxnCompleted(CURLcode code)
{
	CURLClient::OnTxnCompleted(code);
	if(code == CURLE_OK)
	{
		CLOG(ZQ::common::Log::L_INFO, CURLFMT("[%s]post file[%s] by read callback successful"), _url.c_str(), _filepath.c_str());
		CLOG(ZQ::common::Log::L_INFO, CURLFMT("ResponseMessage: %s"), _resPonseBody.c_str());
	}
	else
		CLOG(ZQ::common::Log::L_INFO, CURLFMT("[%s]post file[%s] by read callback failed"), _url.c_str(), _filepath.c_str());

	if(_pReadFile)
		fclose(_pReadFile);
	_pReadFile = NULL;

}
CURLClientPost::Ptr createCURLClientPost(ZQ::common::Log& log)
{
  /*FILE * hd_src ;
	char *file = "F:\\c2LocateRequest";
	hd_src = fopen(file, "wb");

	std::string transBuf;
	transferBuf(transBuf);
	if(hd_src)
		fwrite((char*)transBuf.c_str(), 1, transBuf.size(), hd_src);

	if(hd_src)
		fclose(hd_src);*/

	CURLClientPost::Ptr curlclient = NULL;

	std::string strServer = "10.15.10.85";
	std::string strPort = "10080";
	std::string url = std::string("http://") + strServer + ":" + strPort + "/vodadi.cgi";

	static int i = 1;
	char buf[65] = "";
	itoa(i, buf, 10);

	std::string filepath = std::string("F:\\c2LocateRequest");
	int flags = 0;
	flags = CURLClient::sfEnableOutgoingDataCB |CURLClient::sfEnableDebugInfo;
	curlclient =  new CURLClientPost(filepath,(char*)url.c_str(),log, *_thrdpool, flags,CURLClient::HTTP_POST, buf); 
	if(!curlclient->init())
	{
		curlclient = NULL;
		return NULL;
	}
	curlclient->setHeader("User-Agent", "Test User Agent");
	curlclient->setHeader("Content-Type", "application/atom+xml");
	curlclient->sendRequest();
	i++;
	return  curlclient;
}
///////////////////////////////////////////////////////////////////////////////////////
///////////////////End class CURLClientPost //////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////
/////////////////// class CURLClientPostChunk //////////////////////////////////////////////
////////////////post content by chunk mode   /////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
class CURLClientPostChunk: public CURLClient
{
public:
	typedef ZQ::common::Pointer < CURLClientPostChunk > Ptr;
	CURLClientPostChunk(std::string &filePath, char* url, ZQ::common::Log& log, ZQ::common::NativeThreadPool& thrdpool, int flag = 0, HTTPMETHOD method = HTTP_GET, std::string clientId ="", char* bindIp = "", int port = 0);
	virtual ~CURLClientPostChunk(void){};
public:
	bool init();
protected:
	virtual int  OnPrepareOutgoingData(void *data, size_t size);
	virtual int  OnDataArrived(char *data, size_t size);
	virtual void OnTxnCompleted(CURLcode code);
	virtual long OnChunkBegin(const void* transfer_info, int remains);
	virtual long OnChunkEnd();
private:
	FILE* _pReadFile;
	std::string _filepath;
	std::string _resPonseBody;
};

#define CLOG  (_log)
#define CURLFMT( _X) CLOGFMT(CURLClientPostChunk, "ClientId[%s]Socket[%d] " _X), _clientId.c_str(),_fd

CURLClientPostChunk::CURLClientPostChunk(std::string& filePath,char* url, ZQ::common::Log& log, ZQ::common::NativeThreadPool& thrdpool, int flag , HTTPMETHOD method, std::string clientId, char* bindIp, int port):
CURLClient(url, log, thrdpool,flag,method, clientId, bindIp, port), _pReadFile(NULL), _filepath(filePath)
{
	_resPonseBody = "";
}

bool CURLClientPostChunk::init()
{
	if(!CURLClient::init())
		return false;

	_pReadFile = fopen(_filepath.c_str(), "rb");
	if(!_pReadFile)
	{ 
		CLOG(ZQ::common::Log::L_ERROR, CURLFMT("failed to read file[%s]"), _filepath.c_str());
		return false;
	}
	struct _stat file_info;
	_stat(_filepath.c_str(),  &file_info);

//	return true;
	return setReqDataSize(file_info.st_size);
}

int  CURLClientPostChunk::OnPrepareOutgoingData(void *data, size_t size)
{
	if(!_pReadFile)
	{
		CLOG(ZQ::common::Log::L_ERROR, CURLFMT("NULL read file[%s] handle"), _filepath.c_str());
		return 0;
	}
	unsigned int nRead = 0;
	nRead = fread(data, 1 , size, _pReadFile);
	return nRead;
}
int  CURLClientPostChunk::OnDataArrived(char *data, size_t size)
{
	_resPonseBody.append(data, size);
	return (int)size;
}
void CURLClientPostChunk::OnTxnCompleted(CURLcode code)
{
	CURLClient::OnTxnCompleted(code);
	if(code == CURLE_OK)
	{
		CLOG(ZQ::common::Log::L_INFO, CURLFMT("[%s]post file[%s] by chunkd successful"), _url.c_str(), _filepath.c_str());
		CLOG(ZQ::common::Log::L_INFO, CURLFMT("ResponseMessage: %s"), _resPonseBody.c_str());
	}
	else
		CLOG(ZQ::common::Log::L_INFO, CURLFMT("[%s]post file[%s] by chunkd  failed"), _url.c_str(), _filepath.c_str());

	if(_pReadFile)
		fclose(_pReadFile);
	_pReadFile = NULL;

}

long CURLClientPostChunk::OnChunkBegin(const void* transfer_info, int remains)
{
	CLOG(ZQ::common::Log::L_INFO, CURLFMT("[%s]OnChunkBegin[%d]"),_url.c_str(), remains);
	return remains;

}
long CURLClientPostChunk::OnChunkEnd()
{
	CLOG(ZQ::common::Log::L_INFO, CURLFMT("[%s]OnChunkEnd"), _url.c_str());
    return 0;
}
CURLClient::Ptr createCURLClientBySendChunkMode(ZQ::common::Log& log)
{
	CURLClientPostChunk::Ptr curlclient = NULL;
	std::string strServer = "10.15.10.85";
	std::string strPort = "10080";
//	std::string url = std::string("http://") + strServer + ":" + strPort + "/vodadi.cgi";
	std::string url  = std::string("http://www.taobao.com");

//	std::string filepath = std::string("F:\\c2LocateRequest");
	std::string filepath = "e:\\AliIM2012_taobao(7.10.07C).exe";

	static int i = 1;
	char buf[65] = "";
	itoa(i, buf, 10);
	int flags = 0;
	flags = CURLClient::sfEnableChunkdPost | CURLClient::sfEnableOutgoingDataCB | CURLClient::sfEnableDebugInfo;
	curlclient =  new CURLClientPostChunk(filepath,(char*)url.c_str(),log, *_thrdpool, flags,CURLClient::HTTP_POST, buf); 
	if(!curlclient->init())
	{
		curlclient = NULL;
		return NULL;
	}
	curlclient->setHeader("User-Agent", "Test User Agent");
	curlclient->setHeader("Content-Type", "application/atom+xml");
	curlclient->sendRequest();
	i++;
	return  curlclient;
}
///////////////////////////////////////////////////////////////////////////////////////
///////////////////End class CURLClientPostChunk //////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
const std::string XML_HEADER = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n";
void transferBuf(std::string& buffer);

/////////////////////////////////////////////////////////////////////////
////////////////   post content by buf     //////////////////////////////
/////////////////////////////////////////////////////////////////////////
CURLClient::Ptr createCURLClient(ZQ::common::Log& log)
{
  CURLClient::Ptr curlclient = NULL;
  std::string transBuf;
  transferBuf(transBuf);	

  std::string strServer = "10.15.10.85";
  std::string strPort = "10080";
  std::string url = std::string("http://") + strServer + ":" + strPort + "/vodadi.cgi";
//  std::string url = "http://www.qq.com";
  static int i = 1;
  char buf[65] = "";
  itoa(i, buf, 10);
  int flags = 0;
  flags = CURLClient::sfEnableDebugInfo;
  curlclient =  new CURLClient((char*)url.c_str(),log, *_thrdpool, flags,CURLClient::HTTP_POST, buf); 
  if(!curlclient->init())
  {
	  curlclient = NULL;
	  return NULL;
  }
  curlclient->setHeader("User-Agent", "Test User Agent");
  curlclient->setHeader("Content-Type", "application/atom+xml");
  curlclient->setReqData(transBuf.c_str(), transBuf.size());
  curlclient->sendRequest();
  i++;
  return  curlclient;
}

const int testCount = 1;
int main(int argc, char **argv)
{	
	_thrdpool = new ZQ::common::NativeThreadPool(20);
	if(!_thrdpool)
		return 0;

	CURLClient::startCurlClientManager();

	ZQ::common::FileLog testLogFile("c:\\curlDemo" , 7);
	std::vector<CURLClient::Ptr> curlClients;

	createCURLClient(testLogFile);
/*	for(int i  = 0; i < testCount; i++)
	{
		CURLClient::Ptr curlclient =  createCURLClientChunk(testLogFile);  
		if(curlclient)
			curlClients.push_back(curlclient);
		Sleep(20);
	}

	for(int i  = 0; i < testCount; i++)
	{
		CURLClient::Ptr curlclient =  createCURLClientPost(testLogFile);  
		if(curlclient)
			curlClients.push_back(curlclient);
		Sleep(20);
	}

	for(int i  = 0; i < testCount; i++)
	{
		CURLClient::Ptr curlclient =  createCURLClientBySendChunkMode(testLogFile);  
		if(curlclient)
			curlClients.push_back(curlclient);
		Sleep(20);
	}*/

	size_t size = CURLClient::getCURLClientSize();
	while(size)
	{
		printf("Current CURLClient Count: %d\n", size);
		Sleep(1000);
		size = CURLClient::getCURLClientSize();
	}
	for(int i = 0 ;i< curlClients.size();i++)
	{
		//if(curlClients[i]->getStatusCode() == 0)
		std::string strResStatus;
		printf("***%05d*** ERROR:[%s] StatusCode:[%d]\n", i+1, curlClients[i]->getErrorMessage().c_str(), curlClients[i]->getStatusCode(strResStatus));	
		curlClients[i] =  NULL;
	}

  CURLClient::stopCurlClientManager();

  return 0;
}

void transferBuf(std::string& buffer)
{
	std::ostringstream buf;
	std::string paid = "cdntest1234567895521";
	std::string pid = "zq.com";
	std::string subFile= "index";
	int  bitRate = 1;
	int  IngressCapacity = 1000000000;
	std::string _exclustionlist = "";
	int beginPos = 0;
	int endPos = -1;
	int transferDelay = 100;

	buf << XML_HEADER ;
	buf << "<LocateRequest>\n";
	buf << "  <Object>\n" ;
	buf << "    <Name>\n";
	buf << "     <AssetID>" << paid << "</AssetID>\n";
	buf << "     <ProviderID>" << pid << "</ProviderID>\n";
	buf << "    </Name>\n";
	buf << "    <SubType>"<< subFile << "</SubType>\n";
	buf << "  </Object>\n";
	buf << "  <TransferRate>"<< bitRate<< "</TransferRate>\n";
	buf << "  <IngressCapacity>" << IngressCapacity << "</IngressCapacity>\n";
	buf << "  <ExclusionList>";
	buf << _exclustionlist;
	buf << "  </ExclusionList>\n";
	buf << "  <TransferDelay>";
	buf << transferDelay;
	buf << "  </TransferDelay>\n";
	if(beginPos >= 0)
	{
		buf << "  <Range> ";
		buf << beginPos << " - ";
		if(endPos > 0)
			buf<< endPos ;
		buf << "\n  </Range>\n";
	}
	buf << "</LocateRequest>\n";
	buffer = buf.str();
	//	printf("Body:\n%s \n", buffer.c_str());
}
//	curlclient->setHeader("Content-Type", "application/atom+xml");
//	curlclient->setHeader("Transfer-Encoding","chunked");
//	curlclient->setReqData(transBuf.c_str(), transBuf.size());

//	int code = curl_easy_setopt(curlclient->getCurl(), CURLOPT_POSTFIELDS, curlclient);
//	    code = curl_easy_setopt(curlclient->getCurl(), CURLOPT_POSTFIELDSIZE, (curl_off_t)100);
//	 int code = curl_easy_setopt(curlclient->getCurl(), CURLOPT_WILDCARDMATCH, 1L);
//	int code = curl_easy_setopt(curlclient->getCurl(), CURLOPT_INFILESIZE_LARGE,(curl_off_t)100);
