#ifndef __HSNTree_Authorization_H__
#define __HSNTree_Authorization_H__

#include "Locks.h"

#define LSMS
#ifdef LSMS
	#include "LSMSForMoD.h"
	#define NAMESPACE(_SD)	::com::izq::lsms::integration::mod::##_SD
	#define INTERFACEPRX	NAMESPACE(LSMSForMoD)##Prx
	#define TEARDOWNCB		NAMESPACE(AMI_LSMSForMoD_sessionTeardown)
#else
	#include "ote.h"
	#define NAMESPACE(_SD)	::com::izq::ote::tianshan::##_SD
	#define INTERFACEPRX	NAMESPACE(MoDIceInterface)##Prx
	#define TEARDOWNCB		NAMESPACE(AMI_MoDIceInterface_sessionTeardown)
#endif


const char* AuthorizationGetErrorDesc(int nErrorCode);

class TeardownCB : public TEARDOWNCB
{
public:
	TeardownCB(const std::string& cltSession);
	virtual void ice_response(const NAMESPACE(SessionResultData)& rd);
	virtual void ice_exception(const ::Ice::Exception& ex);

private:
	std::string		clientSessionId;
};

typedef ::IceUtil::Handle<TeardownCB> TeardownCBPtr;

class Authorization
{
public:
	static INTERFACEPRX getAuthorization(::Ice::CommunicatorPtr communicator);
	static bool sessionSetup(::Ice::CommunicatorPtr communicator, const NAMESPACE(SessionData)& sd);
	static void sessionTeardown(::Ice::CommunicatorPtr communicator, const NAMESPACE(SessionData)& sd);
private:
	static ZQ::common::Mutex _mutex;
	static Ice::Long  _lLoop;
};

#endif //__HSNTree_Authorization_H__