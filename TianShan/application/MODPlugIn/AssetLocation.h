// AAAQuery.h: interface for the AAAQuery class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LAM_AESSTLOCATION__B__CD3B3833_49F6_46E2_8542_3DE700AAD2EE__INCLUDED_)
#define AFX_LAM_AESSTLOCATION__B__CD3B3833_49F6_46E2_8542_3DE700AAD2EE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#define R1_AssetLocation_Name "AesstLocation"
namespace ZQTianShan {
	namespace Application{
		namespace MOD{			
class AssetLocation : public ZQTianShan::Application::MOD::IAssetLocation
{
public:
	AssetLocation(void);
	~AssetLocation(void);
protected:
	typedef  struct  
	{
		std::string assetId;
		std::string pid;
		std::vector<std::string> volumes;
	}AssetInfo;

	typedef std::map<std::string, AssetInfo>AssetInfos;

	typedef struct
	{
		int retCode;
		std::string onDemandSessionID;
		AssetInfos assetInfos;
	}ResponseInfo;
public:
	virtual int getAssetLocation(AssetLocationInfo& alinfo, AEReturnData& aedata, ::TianShanIce::ValueMap& privData);
private:
	bool parserAssetLocationResponse(const std::string& strResponse, AssetLocationInfo& alinfo, ResponseInfo& responseinfo);

};
}}}//end namespace

#endif // !defined(AFX_LAM_AESSTLOCATION__B__CD3B3833_49F6_46E2_8542_3DE700AAD2EE__INCLUDED_)
/*
<?xml version="1.0" encoding="utf-8"?>
<LocateAssetsResponse ODSessionID=¡± be074250-cc5a-11d9-8cd5-0800200c9a66¡±>
<Asset ProviderID=¡±comcast.com¡± AssetID=¡±CokeAd1¡±>
<Location fileLocation=¡±/vodContent/CokeAd1.Mpg¡±
volumeName=¡±detroit.GrossePointe.volume1¡±/>
</Asset>
<Asset ProviderID=¡±comcast.com¡± AssetID=¡±flyers20041103¡±>
<Location fileLocation=¡°/vodContent/flyers20041103¡±
volumeName=¡±detroit.GrossePointe.volume2¡±/>
<Location fileLocation=¡°/vodContent/flyers20041103¡±
volumeName=¡±detroit.GrossePointe.volume1¡±/>
</Asset>
<Asset ProviderID=¡±comcast.com¡± AssetID=¡±ComcastPromo¡±>
</Asset>
</LocateAssetsResponse>
*/