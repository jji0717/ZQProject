// AAAQuery.h: interface for the AAAQuery class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HeNanAAAQUEY_B__CD3B3833_49F6_46E2_8542_3DE700AAD2EE__INCLUDED_)
#define AFX_HeNanAAAQUEY_B__CD3B3833_49F6_46E2_8542_3DE700AAD2EE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#define HENAN_AAAQUERY_Name "HeNanAAARequest"
namespace ZQTianShan {
	namespace Application{
		namespace MOD{
class HeNanAAAQuery : public ZQTianShan::Application::MOD::IAAA
{
public:
	HeNanAAAQuery(void);
	~HeNanAAAQuery(void);
protected:
	struct contentRef
	{
		std::string providerID;
		std::string assetID;
		std::string startNPT;
		std::string endNPT;
	};
	typedef std::vector<contentRef> contentRefs;
	struct ResponseInfo
	{
		std::string  playlistId;
		int  bitrate;
		int  startIndex;
		std::string startNPT;
		contentRefs playlistItems;
	};
public:
	virtual int OnAuthorize(AAAInfo& aaaInfo, AEReturnData& aedata, ::TianShanIce::ValueMap& privData);
	virtual	int OnStatusNotice(AAAInfo& aaaInfo, const ::TianShanIce::Properties& prop);
protected:
	bool parserAuthorResponse(std::string& strResponse,AAAInfo& aaaInfo, ResponseInfo& responseInfo);
protected:
	ZQ::common::Mutex _mutex;
	Ice::Long  _ltransactionID;
};
}}}//end namespace

#endif // !defined(AFX_HeNanAAAQUEY_B__CD3B3833_49F6_46E2_8542_3DE700AAD2EE__INCLUDED_)
