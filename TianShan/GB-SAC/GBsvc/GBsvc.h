#if !defined(__ZQTIANSHAN_GBSVC_H__)
#define __ZQTIANSHAN_GBSVC_H__

#include "ZQ_common_conf.h"
#include "GBsvcConfig.h"

#ifdef ZQ_OS_MSWIN
#include "BaseZQServiceApplication.h"
#else
#include "ZQDaemon.h"
#endif

namespace ZQTianShan {
namespace GBServerNS {
	
class ServiceEnv;

class GBsvcUtil
{
public:
	static bool fsCreatePath( const std::string& strPath);
	static std::string fsFixupPath( const std::string& strPath);
	static void replaceCharacter( int iLen, char* p, const char* pSrc, int& iPos, bool& bLastSlash);
	static void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress);
};


class GBService : public ZQ::common::BaseZQServiceApplication
{
public:
	GBService ();
	virtual ~GBService ();

	HRESULT OnStart(void);
	HRESULT OnStop(void);
	HRESULT OnInit(void);
	HRESULT OnUnInit(void);

private:
	bool initializeCrashDumpLocation(void);
	friend void WINAPI GBsvcUtil::CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress);

private:
	ServiceEnv *      _pGBSvcEnv;
};


class ServiceEnv : public ZQ::common::NativeThread
{
public:
	explicit ServiceEnv(std::string localIp, std::string localPort, ZQ::common::Log & _svcLog);
	~ServiceEnv();

	virtual int run(); 

private:
	int          _running;
	std::string	 _localIp;
	std::string  _localPort;

	ZQ::common::Log & _svcLog; 
};

}//GBServerNS
}//ZQTianShan 

#endif// __ZQTIANSHAN_GBSVC_H__