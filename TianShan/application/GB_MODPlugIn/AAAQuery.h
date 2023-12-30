// AAAQuery.h: interface for the AAAQuery class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AAAQUEY_B__CD3B3833_49F6_46E2_8542_3DE700AAD2EE__INCLUDED_)
#define AFX_AAAQUEY_B__CD3B3833_49F6_46E2_8542_3DE700AAD2EE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#define LAM_AAAQUERY_Name "AAARequest"
namespace ZQTianShan {
	namespace Application{
		namespace MOD{			
class AAAQuery : public ZQTianShan::Application::MOD::IAAA
{
public:
	AAAQuery(void);
	~AAAQuery(void);
protected:
	struct ResponseInfo
	{
		std::string  transactionID;
		std::string  time; 
		std::string  opCode;
		std::string  msgType;
		int          retCode;
		std::string  errormsg;

		std::string  sessionId;
		std::string  userId;
		std::string  entitlementCode;

		int64        maxBitrate;
		double       duration;
		std::string  playType;
		double       breakpoint;

		TianShanIce::StrValues playlistItems;
	};
public:
	virtual int OnAuthorize(AAAInfo& aaaInfo, AEReturnData& aedata, ::TianShanIce::ValueMap& privData);
	virtual	int OnStatusNotice(AAAInfo& aaaInfo, const ::TianShanIce::Properties& prop);
protected:
	bool parserAuthorResponse(std::string& strResponse,AAAInfo& aaaInfo, ResponseInfo& responseInfo);
	bool parserStatusNoticeResponse(std::string& strResponse, AAAInfo& aaaInfo, ResponseInfo& responseInfo);
protected:
	ZQ::common::Mutex _mutex;
	Ice::Long  _ltransactionID;
};
}}}//end namespace

#endif // !defined(AFX_LAMGETPLAYLIST_H__CD3B3833_49F6_46E2_8542_3DE700AAD2EE__INCLUDED_)
