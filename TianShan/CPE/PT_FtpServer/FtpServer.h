
#ifndef _CPE_FTP_SERVER_
#define _CPE_FTP_SERVER_


#include "IPushTrigger.h"

class FtpSite;

namespace ZQ{
	namespace common{
		class NativeThreadPool;
		class Log;
	}
}

namespace ZQTianShan{
	namespace ContentProvision{
		class IPushSource;
	}
}

using namespace ZQTianShan::ContentProvision;

namespace ZQTianShan {
	namespace CPE {
		class CPEEnv;
	}
}

using namespace ZQTianShan::CPE;


class FtpServer
{
public:
	FtpServer(CPEEnv& env);
	~FtpServer();
	// read the configuration xml, 
	bool Initialize();
	
	// start ftp server
	bool Start();
	
	// stop ftp server
	bool Stop();
	
	// do the uninitializations
	void Uninitialize();

	IPushSource* findPushSource(const char* contentStoreNetId, 
		const char* volume,
		const char* content);

	static bool validatePush(void* pCtx, const char* szNetId, const char* szVolume, const char* szContent);
	bool validatePush(const char* szNetId, const char* szVolume, const char* szContent);
private:	
	FtpSite*			_site;
	
	ZQ::common::NativeThreadPool*	_threadPool;
	CPEEnv&				_env;
};


#endif
