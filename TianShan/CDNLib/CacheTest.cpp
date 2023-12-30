#include "DiskCache.h"
#include "FileLog.h"
#include <stdio.h>
using namespace XOR_Media::DiskCache;

void io_test()
{
/*
XOR_Media::DiskCache::CacheDir::hkey_t hkey1, hkey2;
	std::string s ="asfsafsafsadfsa", s2="324124321421";
	XOR_Media::DiskCache::CacheDir::_calcHashKey(hkey1, s.c_str(), s.length());
	XOR_Media::DiskCache::CacheDir::_calcHashKey(hkey2, s2.c_str(), s2.length());
	uint64 dist = XOR_Media::DiskCache::CacheDir::_distance(hkey1, hkey1);
	dist = XOR_Media::DiskCache::CacheDir::_distance(hkey1, hkey2);
*/
	char rawdata[1024];
	for (int i =0; i< sizeof(rawdata); i++)
		rawdata[i] =i &0xff;

	XOR_Media::DiskCache::BufferPtr cont = new ZQ::common::BufferList();
	cont->fill((uint8*)rawdata, 0, sizeof(rawdata));

	CacheCopy::Ptr ccp = new CacheCopy();
	ccp->_content.cont = cont;
	ccp->_content.url  = "http://sadfsafdas/dsf";
	ccp->_content.stampAsOfOrigin = ZQ::common::now();
	ccp->_pathName     = "e:\\aaaa.seg";
	ccp->flush();

	ccp = new CacheCopy();
	ccp->load("e:\\aaaa.seg");

	ccp->unlink();
}
class DiskCacheSinkImpl :public DiskCacheSink
{
public:
	DiskCacheSinkImpl(ZQ::common::Log& log):_log(log)
	{

	}
	// errCode takes those of DiskCache::Error
	virtual void OnRead(Error errCode, CacheCopy::Ptr ccopy)
	{
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(DiskCacheSinkImpl, "OnRead ErrorCode[%d]URL[%s]"), errCode, ccopy->_content.url.c_str());
	}

	// errCode : cacheErr_OK - wrote success, cacheErr_Hit - cache already exists, cacheErr_Timeout - writting timeout
	virtual void OnWrote(Error errCode, const std::string& url)
	{
	   _log(ZQ::common::Log::L_DEBUG, CLOGFMT(DiskCacheSinkImpl, "OnWrote ErrorCode[%d]URL[%s]"), errCode, url.c_str());
	}
protected:
	ZQ::common::Log& _log;
};

std::string randUrl(std::vector<std::string>& strURLList) 
{ 
	int size = strURLList.size();
	srand(time(NULL) + rand());    

	return strURLList[rand() % size];
} 
bool _bQuit = false;

class WriteTest: public ZQ::common::ThreadRequest
{
public:
	WriteTest(ZQ::common::NativeThreadPool& thpool,ZQ::common::Log& log, std::vector<std::string>& strURLlist):
		ZQ::common::ThreadRequest(thpool),_log(log),_strURLlist(strURLlist)
	{

	}
	~WriteTest()
	{

	}
public:
	virtual int run()
	{
		char rawdata[10240];
		for (int i =0; i< sizeof(rawdata); i++)
			rawdata[i] =i &0xff;

		XOR_Media::DiskCache::BufferPtr cont = new ZQ::common::BufferList();
		cont->fill((uint8*)rawdata, 0, sizeof(rawdata));

		DiskCacheSink* cbWriteCache = new DiskCacheSinkImpl(_log);

         while(!_bQuit)
		 {
			 CacheDir::write_async(randUrl(_strURLlist), cont, 0, cbWriteCache);
			 Sleep(100);
		 }

		 if(cbWriteCache)
		 {
			 delete cbWriteCache;
			 cbWriteCache = NULL;
		 }
		 return -1;
	}

	virtual void final(int retcode =0, bool bCancelled =false)
	{
		delete this;
	}
protected:
	ZQ::common::Log& _log;
	std::vector<std::string>& _strURLlist;
};


class ReadTest: public ZQ::common::ThreadRequest
{
public:
	ReadTest(ZQ::common::NativeThreadPool& thpool, ZQ::common::Log& log, std::vector<std::string>& strURLlist):
	  ZQ::common::ThreadRequest(thpool), _log(log), _strURLlist(strURLlist)
	  {

	  }
	~ReadTest()
	{

	}
public:
	virtual int run()
	{
		char rawdata[10240];
		for (int i =0; i< sizeof(rawdata); i++)
			rawdata[i] =i &0xff;

		XOR_Media::DiskCache::BufferPtr cont = new ZQ::common::BufferList();
		cont->fill((uint8*)rawdata, 0, sizeof(rawdata));

		DiskCacheSink* cbReadCache = new DiskCacheSinkImpl(_log);

		while(!_bQuit)
		{
			CacheDir::read_async(randUrl(_strURLlist),cbReadCache);
			Sleep(100);
		}

		if(cbReadCache)
		{
			delete cbReadCache;
			cbReadCache = NULL;
		}
		return -1;
	}

	virtual void final(int retcode =0, bool bCancelled =false)
	{
		delete this;
	}
protected:
	ZQ::common::Log& _log;
	std::vector<std::string>& _strURLlist;
};
ZQ::common::FileLog testlog("E:\\temp\\CacheTest.log", ZQ::common::Log::L_DEBUG);

#define LRUMapSize				(1000)
#define MaxDirtyCopies	        (50)
#define MaxLoadingCopies		(50)
#define CacheDirTimeOut			(500)
#define URLCount				(10000)
#define URLPrefix				"http://192.168.81.2/"
char* pstr="1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ/";

std::string randstring(int strLen) 
{ 
	int len = strlen(pstr);
	std::string buffer;
	srand(time(NULL) + rand());    
	for(int i = 0; i  < strLen; i++) 
	{ 
		buffer.push_back(pstr[rand()%len]);
	} 
	return buffer;
} 

void main(int argc, char* argv[])
{
	///Ð´ÈëData
	char rawdata[1024];
	for (int i =0; i< sizeof(rawdata); i++)
		rawdata[i] =i &0xff;

	//×éÖ¯URL
	std::vector<std::string>strURLList;
	strURLList.push_back("http://192.168.81.2/Test");
	for(int j = 0; j < URLCount; ++j)
	{
		std::string strRandomString = randstring(20);
		std::string url = URLPrefix + strRandomString;
		strURLList.push_back(url);
	}

	// io_test();
	CacheDir::Ptr dir1 = CacheDir::addCacheDir(testlog, "E:\\temp\\CacheTest\\dir1", 100);
	CacheDir::Ptr dir2 = CacheDir::addCacheDir(testlog, "E:\\temp\\CacheTest\\dir2", 100);
	CacheDir::Ptr dir3 = CacheDir::addCacheDir(testlog, "E:\\temp\\CacheTest\\dir3", 100);
	CacheDir::Ptr dir4 = CacheDir::addCacheDir(testlog, "E:\\temp\\CacheTest\\dir4", 100);
	CacheDir::Ptr dir5 = CacheDir::addCacheDir(testlog, "E:\\temp\\CacheTest\\dir5", 100);

	// dirX.setXXX();
	CacheDir::DirMap dirMap =  CacheDir::listCacheDir();
	for(CacheDir::DirMap::iterator itorCacheMap =  dirMap.begin();  itorCacheMap != dirMap.end(); itorCacheMap++)
	{
       itorCacheMap->second->setLRUsize(LRUMapSize);
	   itorCacheMap->second->setMaxPending(MaxDirtyCopies, MaxLoadingCopies);
	   itorCacheMap->second->setTimeout(CacheDirTimeOut);
	   itorCacheMap->second->subDirs(16, 16);
	}
/*
#define WriteCount				(10000)
	XOR_Media::DiskCache::BufferPtr cont = new ZQ::common::BufferList();
	cont->fill((uint8*)rawdata, 0, sizeof(rawdata));

	DiskCacheSink* cbWriteCache = new DiskCacheSinkImpl();
	for(int j = 0; j < WriteCount; ++j)
	{
		CacheDir::write_async(randUrl(strURLList), cont, 0, cbWriteCache);
	}

#define ReadCount				(10000)
	DiskCacheSink* cbReadCache = new DiskCacheSinkImpl();
	for(int j = 0; j < ReadCount; ++j)
	{
		CacheDir::read_async(randUrl(strURLList),cbReadCache);

	}

	while(getchar() != 'q')
	{
		Sleep(5000);
	}

	if(cbWriteCache)
	{
		delete cbWriteCache;
		cbWriteCache = NULL;
	}

	if(cbReadCache)
	{
		delete cbReadCache;
		cbReadCache = NULL;
	}
*/
#define WriteThreadCount  10
#define ReadThreadCount   10
	ZQ::common::NativeThreadPool  testPool;
	testPool.resize(WriteThreadCount +  ReadThreadCount +1);

	for(int i =0; i < WriteThreadCount; i++)
	{
		WriteTest* pWrite = new WriteTest(testPool,testlog ,strURLList);
		if(pWrite)
			pWrite->start();
	}

	for(int i =0; i < ReadThreadCount; i++) 
	{
		ReadTest* pRead = new ReadTest(testPool,testlog,strURLList);
		if(pRead)
			pRead->start();
	}

	while(getchar() != 'q')
	{
		Sleep(5000);
	}
	_bQuit = true;

}
