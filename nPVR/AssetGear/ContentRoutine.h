

#ifndef _CONTENT_ROUTINE_
#define _CONTENT_ROUTINE_


bool contentInit();

bool createContent(const char* szObjectName,
				   const char* szAssetName,
				   const char* szStartTime,
				   const char* szEndTime,
				   int nBitrate,
				   const char* szProvider,
				   const char* szProviderId,
				   const char* szProviderAssetId,
				   int nPriority,
				   int& dwAssetID, 
				   int& dwAeID, 
				   char* szPushUrl,
				   int  nPushUrlSize,
				   char* szErrMsg,
				   int  nErrMsgSize,
				   const char* szActiveDate = "01-Dec-2000");

bool repairContent(const char* szObjectName,
				   const char* szAssetName,
				   const char* szStartTime,
				   const char* szEndTime,
				   int nBitrate,
				   const char* szProvider,
				   int nPriority,
				   int& dwAssetID, 
				   int& dwAeID, 
				   char* szPushUrl,
				   int  nPushUrlSize,
				   char* szErrMsg,
				   int  nErrMsgSize,
				   bool& bTryCreateContent,
				   const char* szActiveDate = "01-Dec-2000");

void deleteContent(const char* szObjectName);

void contentClose();


extern WCHAR                           _wszItvImportPath[256];
extern WCHAR							_wszTemplatePath[256];


#endif