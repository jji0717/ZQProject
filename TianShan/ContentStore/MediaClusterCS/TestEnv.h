
#ifndef _TEST_ENV_HEADER_
#define _TEST_ENV_HEADER_

#include "Log.h"
#include "ContentImpl.h"


using namespace ZQ::common;
using namespace ZQTianShan::ContentStore;

class TestEnv
{
public:
	//only one instance
	static TestEnv* getInstance();
	static void deleteInstance();

	TestEnv();
	~TestEnv();

	Ice::CommunicatorPtr ic;
	NativeThreadPool threadpool;
	ZQADAPTER_DECLTYPE adapter;
	::ZQTianShan::ContentStore::ContentStoreImpl::Ptr store;
	std::auto_ptr<Log> log, eventlog, icelog;

protected:
	
	static std::auto_ptr<TestEnv> env;
	static bool bQuit;
	static BOOL WINAPI ConsoleHandler(DWORD CEvent);
};

#endif