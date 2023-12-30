// OTEAuthorization.h: interface for the OTEAuthorization class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OTEAUTHORIZATION_H__04BE222C_810A_4F2B_A78C_1394C87D0CE1__INCLUDED_)
#define AFX_OTEAUTHORIZATION_H__04BE222C_810A_4F2B_A78C_1394C87D0CE1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
namespace ZQTianShan {
namespace Application{
namespace MOD{
#define	OTE_Authorization_NAME		"AuthorizationOnOTE"

class OTEAuthorization :  public ::Ice::LocalObject, 
				    	public ZQAPPMOD::IAuthorization  
{
public:
	OTEAuthorization(::Ice::CommunicatorPtr& _ic);
	virtual ~OTEAuthorization();
	
	int OnAuthPurchase(AuthorInfo& authorInfo, ::TianShanIce::ValueMap& privData);		
	int OnDestroyPurchase(AuthorInfo& authorInfo, const ::TianShanIce::Properties& prop);

public:
	::Ice::CommunicatorPtr& _iceComm;
private:
	ZQ::common::Mutex _mutex;
	Ice::Long  _lLoop;
};
}}}//end namespace


#endif // !defined(AFX_OTEAUTHORIZATION_H__04BE222C_810A_4F2B_A78C_1394C87D0CE1__INCLUDED_)
