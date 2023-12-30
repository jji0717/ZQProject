// LSMSForMoDForTeardownCB.h: interface for the LSMSForMoDForTeardownCB class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LSMSFORTEARDOWNCB_H__03CFADFD_9E5E_41BD_B579_387E1426D706__INCLUDED_)
#define AFX_LSMSFORTEARDOWNCB_H__03CFADFD_9E5E_41BD_B579_387E1426D706__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define _WINSOCK2API_
#include "LSMSForMoD.h"
#include <IceUtil/IceUtil.h>
namespace ZQTianShan {
namespace Application{
namespace MOD{
	const char* LSMSGetErrorDesc(int nErrorCode);
#if  ICE_INT_VERSION / 100 >= 306
class LSMSForMoDForStateCB : public IceUtil::Shared
{
public:
	LSMSForMoDForStateCB(const std::string& cltSession);
private:
	void handleException(const Ice::Exception&){}
public:
	void sessionTeardown(const Ice::AsyncResultPtr&);	
protected:
	std::string		clientSessionId;
};
typedef IceUtil::Handle<LSMSForMoDForStateCB> LSMSForMoDForStateCBPtr;
#else	
	class LSMSForMoDForTeardownCB : public ::com::izq::lsms::integration::mod::AMI_LSMSForMoD_sessionTeardown
	{
	public:
		LSMSForMoDForTeardownCB(const std::string& cltSession);
		virtual void ice_response(const com::izq::lsms::integration::mod::SessionResultData& rd);
		virtual void ice_exception(const ::Ice::Exception& ex);

	private:
		std::string		clientSessionId;
	};
	
	typedef ::IceUtil::Handle<LSMSForMoDForTeardownCB> LSMSForMoDForTeardownCBPtr;
#endif
	
#endif // !defined(AFX_LSMSFORTEARDOWNCB_H__03CFADFD_9E5E_41BD_B579_387E1426D706__INCLUDED_)
	
}}}//end namespace
