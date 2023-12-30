// OTEAuthorization.h: interface for the OTEAuthorization class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LSMSAUTHORIZATION_H__04BE222C_810A_4F2B_A78C_1394C87D0CE1__INCLUDED_)
#define AFX_LSMSAUTHORIZATION_H__04BE222C_810A_4F2B_A78C_1394C87D0CE1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
namespace ZQTianShan {
	namespace Application{
namespace MOD{
#define	LSMS_Authorization_NAME		"AuthorizationOnLSMS"

class LSMSForMODAuthorization :  public ::Ice::LocalObject, 
				    	public ZQAPPMOD::IAuthorization  
{
public:
	LSMSForMODAuthorization(::Ice::CommunicatorPtr& _ic);
	virtual ~LSMSForMODAuthorization();
	
	int OnAuthPurchase(AuthorInfo& authorInfo, ::TianShanIce::ValueMap& privData);		
	int OnDestroyPurchase(AuthorInfo& authorInfo, const ::TianShanIce::Properties& prop);

public:
	::Ice::CommunicatorPtr& _iceComm;
private:
	ZQ::common::Mutex _mutex;
	Ice::Long  _lLoop;
};
}}}//end namespace


#endif // !defined(AFX_LSMSAUTHORIZATION_H__04BE222C_810A_4F2B_A78C_1394C87D0CE1__INCLUDED_)
