

#include "string.h"
#include "SystemMd.h"

const char* _strSystemMDs[] = {
	"ActivateTime",
	"Advisory",
	"AnalogCopy",
	"AnalogCopyCharge",
	"Audience",
	"AudioType",
	"BillingName",
	"BillingId",
	"ClosedCaption",
	"ContractName",
	"CountryOfOrigin",
	"CreationDate",
	"DeactivateTime",
	"DeleteTime",
	"DigitalCopy",
	"DigitalCopyCharge",
	"DisplayGroup",
	"Distributor",
	"Distributor	RoyaltyMinimum",
	"Distributor RoyaltyFlatRate",
	"Distributor RoyaltyPercent",
	"Description",
	"LocationClass",
	"Language",
	"SubtitleLanguage",
	"DubbedLanguage",
	"Length",
	"MSORating",
	"NagraCasProductId",
	"NagraCaSystemId",
	"NagraAccessCriteria",
	"NagraNaspCA",
	"NagraNaspPPV",
	"NagraNaspIEMM",
	"PlayBandwidth",
	"Priority",
	"Product",
	"Provider",
	"ProviderId",
	"ProviderAssetId",
	"ProviderQAContact",
	"PurchaseCount",
	"Rating",
	"Studio",
	"StudioRoyaltyMinimum",
	"StudioRoyaltyFlatRate",
	"StudioRoyaltyPercent",
	"ScreenFormat",
	"Scrambled",
	"ScrambledTimeType",
	"ScrambledTime",
	"SplicingEnabled",
	"ToldPsToReplicate",
	"UsageClass",
	"Version",
	"YearOfRelease",
};

int _nSystemMDs = sizeof(_strSystemMDs)/sizeof(const char*);

bool IsSystemMetadata(const char* szMd)
{
	for(int i=0;i<_nSystemMDs;i++)
	{
		if (!stricmp(_strSystemMDs[i], szMd))
			return true;
	}

	return false;
}