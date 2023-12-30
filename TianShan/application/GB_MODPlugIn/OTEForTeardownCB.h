// OTEForTeardownCB.h: interface for the OTEForTeardownCB class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OTEFORTEARDOWNCB_H__03CFADFD_9E5E_41BD_B579_387E1426D706__INCLUDED_)
#define AFX_OTEFORTEARDOWNCB_H__03CFADFD_9E5E_41BD_B579_387E1426D706__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define _WINSOCK2API_
#include "ote.h"
#include <IceUtil/IceUtil.h>
namespace ZQTianShan {
namespace Application{
namespace MOD{
	const char* oteGetErrorDesc(int nErrorCode);
	
#if  ICE_INT_VERSION / 100 >= 306
	class OTEForStateCB : public IceUtil::Shared
	{
	public:
		OTEForStateCB(const std::string& cltSession);
 	private:
    	void handleException(const Ice::Exception&);
 	public:
		void sessionTeardown(const Ice::AsyncResultPtr&);   
 	protected:
		std::string     clientSessionId;
	};      
	typedef IceUtil::Handle<OTEForStateCB> OTEForStateCBPtr;	
#else
	class OTEForTeardownCB : public com::izq::ote::tianshan::AMI_MoDIceInterface_sessionTeardown
	{
	public:
		OTEForTeardownCB(const std::string& cltSession);
		virtual void ice_response(const ::com::izq::ote::tianshan::SessionResultData& rd);
		virtual void ice_exception(const ::Ice::Exception& ex);
		
	private:
		std::string		clientSessionId;
	};
	
	typedef ::IceUtil::Handle<OTEForTeardownCB> OTEForTeardownCBPtr;
#endif	
#endif // !defined(AFX_OTEFORTEARDOWNCB_H__03CFADFD_9E5E_41BD_B579_387E1426D706__INCLUDED_)
	
}}}//end namespace
