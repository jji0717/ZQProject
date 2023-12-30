// ============================================================================================
// Copyright (c) 1997, 1998 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved. Unpublished rights reserved under the copyright laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the confidential
// technology of ZQ Interactive, Inc. Possession, use, duplication or dissemination of the
// software and media is authorized only pursuant to a valid written license from ZQ Interactive,
// Inc.
// This source was copied from shcxx, shcxx's copyright is belong to Hui Shao
//
// This software is furnished under a  license  and  may  be used and copied only in accordance
// with the terms of  such license and with the inclusion of the above copyright notice.  This
// software or any other copies thereof may not be provided or otherwise made available to any
// other person.  No title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and should not be
// construed as a commitment by ZQ Interactive, Inc.
// --------------------------------------------------------------------------------------------
// Author: Hui Shao
// Desc  : impl the soap interface of AssetGear for ZQ integration with ISA components
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/nPVR/AssetGear/AssetGear_soap.cpp 1     10-11-12 16:01 Admin $
// $Log: /ZQProjs/nPVR/AssetGear/AssetGear_soap.cpp $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 15    07-10-23 16:30 Jie.zhang
// assetID from int to hex string
// 
// 14    06-07-07 16:56 Jie.zhang
// 
// 13    06-07-07 14:58 Jie.zhang
// support providerId and providerAssetId
// 
// 12    06-07-07 12:31 Jie.zhang
// 
// 11    06-07-06 17:07 Jie.zhang
// repair also need update metadata
// 
// 10    06-04-29 11:49 Jie.zhang
// 
// 9     06-04-19 16:34 Jie.zhang
// System Metadata and App Metadata itv file issue
// 
// 8     06-03-24 15:52 Jie.zhang
// 
// 7     06-02-14 14:06 Jie.zhang
// 
// 6     05-12-28 18:46 Jie.zhang
// 
// 5     05-11-24 15:48 Jie.zhang
// 
// 4     05-11-22 19:12 Jie.zhang
// 
// 3     11/04/05 10:38a Hui.shao
// ============================================================================================

#include "AssetGear_soap.h"
#include "./AssetGearServiceSoapBinding.nsmap"
#include "ContentRoutine.h"
#include "SystemMd.h"


#define STATUS_OK				0				//Success
#define STATUS_ASSET_EXIST		1				//Asset name already exist
#define	STATUS_NO_RESOURCE		2				//Fail to book resource
#define STATUS_METADATA			3				//Fail to update metadata
#define STATUS_TIME_ERROR		4				//Time parameter error( format / startime<now / endtime<startime )

#define STATUS_OTHER_ERROR		10				//Other error need retry

const char* _szStatusDesc[] = {
	"Success",
	"Asset name already exist",
	"Fail to book resource",
	"Fail to update metadata",
	"Time parameter error(format/startime<now/endtime<=startime)",
};

#define _USE_UNICODE_ITV_


// from isa.h, for compile easier, so just 
#include "ReporterTypes.h"
extern bool 
       LogMsg       ( DWORD         dwTraceLevel, 
                      LPCTSTR       lpszFmt, ... );

extern bool
       LogMsg       ( LPCTSTR       lpszFmt, ... );

extern std::wstring getErrMsg();

extern char			_szMetadataGatewayUrl[256];
extern WCHAR		_wszApplicationName[256];
extern char			_szApplicationName[256];
extern char			_szProvider[256];
extern DWORD		_dwTimeWindowSecs;
extern DWORD		_dwFailCAWhenMdFail;
extern DWORD		_dwMaxMetaDataCount;

// -----------------------------
// class AgSoapRequest
// -----------------------------
class AgSoapRequest : public ZQ::common::ThreadRequest
{
public:
	AgSoapRequest(AssetGearService& AgSvc) : _pSoap(NULL), ThreadRequest(AgSvc)
	{
		_pSoap = soap_copy(&AgSvc);
	}
	
	~AgSoapRequest()
	{
		if (NULL != _pSoap)
		{
			soap_destroy(_pSoap);
			soap_end(_pSoap);
			soap_done(_pSoap);
			free(_pSoap);
		}
		_pSoap = NULL;
	}
	
protected:
	virtual bool init(void)
	{
		return (NULL != _pSoap);
	}
	
	virtual int run(void)
	{
		if (NULL != _pSoap)
			return soap_serve(_pSoap);
		
		return -1;
	}
	
	virtual void final(void)
	{
		if (NULL != _pSoap)
		{
			soap_destroy(_pSoap);
			soap_end(_pSoap);
			soap_done(_pSoap);
		}
		delete this;
	}
	
	struct soap* _pSoap;
};

// -----------------------------
// class AssetGearService
// -----------------------------
AssetGearService::AssetGearService(const char* localIp, const int port, const int thpoolsize)
: NativeThreadPool(thpoolsize), _localIP(NULL), _port(port), _bQuit(false)
{
	soap_init2(this, SOAP_IO_KEEPALIVE, SOAP_IO_KEEPALIVE);
	if (!this->namespaces)
		this->namespaces = namespaces;
	accept_timeout = TIMEOUT;
	bind_flags |= SO_REUSEADDR;	/* don't use this in unsecured environments */
	/* socket_flags = MSG_NOSIGNAL; */	/* use this to disable SIGPIPE */
	/* bind_flags |= SO_NOSIGPIPE; */	/* or use this to disable SIGPIPE */
	
	if (NULL != localIp)
	{
		_localIP = soap_new_std__string(this, -1);
		if (NULL != _localIP)
			(*_localIP) = localIp;
	}
	if (_port <= 0)
		_port = DEFAULT_PORT;
}

AssetGearService::~AssetGearService()
{
	soap_destroy(this);
	soap_end(this);
	soap_done(this);
}

bool AssetGearService::init(void)
{
	const char* IpAddr = (NULL != _localIP) ? _localIP->c_str() : NULL;
	
	int ret = soap_bind(this, IpAddr, _port, BACKLOG);
	if (ret < 0)
	{
		soap_print_fault(this, stderr);
		return false;
	}

	return true;
}

int AssetGearService::run(void)
{
	while (!_bQuit)
	{
		int ret = soap_accept(this);
		if (ret < 0)
		{
			if (errnum)
				soap_print_fault(this, stderr);
			else
				fprintf(stderr, "timed out\n");	// should really wait for threads to terminate
		}
		
		AgSoapRequest* s = new AgSoapRequest(*this);
		s->start();
	}
	
	return 0;
}

#define soap_set_string(_Soap, _Var, _Value) { _Var = soap_new_std__string(_Soap, -1); *(_Var) = _Value; }
#define soap_set_atomic(_Soap, _Var, _Type, _Value) { _Var = (_Type*) soap_malloc(_Soap, sizeof(_Type)); *(_Var) = _Value; }

bool updateMetadata_unicode_itv(int scheduleID, const char* programId, const char* assetName, DWORD dwAssetID)
{
	//
	// query metadata
	//
	PMClient pmc(_szMetadataGatewayUrl);
	
	PMClient::MetaData* metaDatas = new PMClient::MetaData[_dwMaxMetaDataCount];
	int count =pmc.queryMetaData(scheduleID, programId, dwAssetID, metaDatas, _dwMaxMetaDataCount);
	if (count <=0)
	{
		LogMsg(REPORT_SEVERE, L"Fail to invoke PMClient::queryMetaData");
		delete metaDatas;
		return false;
	}
	
	//
	// write second itv file
	//
	{
		wchar_t wszText[100*1024];
		wszText[0] = 0xFEFF;
		wchar_t * pwPtr = wszText + 1;

		wchar_t wszAssetName[256];
		wchar_t wszKeyName[256];
		wchar_t wszKeyValue[256];

		MultiByteToWideChar(CP_UTF8, 0, assetName, -1, wszAssetName, sizeof(wszAssetName));
		
		pwPtr += swprintf(pwPtr, L"; Content import itv file. Generated by AssetGear interface"
				   L"\r\n\r\n[version]"
				   L"\r\nVersion_Major=1"
				   L"\r\nVersion_Minor=4"
				   L"\r\n\r\n[uid]"
				   L"\r\n1001=1,%s"
				   L"\r\n1002=2, %s"
				   L"\r\n\r\n[metadata_1002]\r\n\r\n",
				   _wszApplicationName,
				   wszAssetName);

		for (int i=0; i< count; i++)
			//		PMClient::MetaDataCollection::iterator it = metaDatas.begin();
			//	it < metaDatas.end(); it++)
		{
			bool bIsSystemMD = IsSystemMetadata(metaDatas[i].name);
			MultiByteToWideChar(CP_UTF8, 0, metaDatas[i].name, -1, wszKeyName, sizeof(wszKeyName));
			MultiByteToWideChar(CP_UTF8, 0, metaDatas[i].value, -1, wszKeyValue, sizeof(wszKeyValue));

			LogMsg(L"[%08x]: %s = %d, %s", dwAssetID, wszKeyName, metaDatas[i].type, wszKeyValue);
			
			if (bIsSystemMD)
			{
				pwPtr += swprintf(pwPtr, L"%s=0,%d,%s\r\n\r\n", wszKeyName, metaDatas[i].type, wszKeyValue);
			}
			else
			{
				pwPtr += swprintf(pwPtr, L"%s=1001,%d,%s\r\n\r\n", wszKeyName, metaDatas[i].type, wszKeyValue);
			}			
		}

		// add 1 to save a 0 to file end, 
		// and the unicode itv file shows like this
		int bufsize = (char*)pwPtr - (char*)wszText + 1;	

		// create itv file at local directory _wszTemplatePath
		WCHAR wszSrcPath [256];
		wsprintf (wszSrcPath, L"%s\\import\\%08x_u.itv", _wszTemplatePath, dwAssetID);

		{// write source file
			HANDLE hFile = ::CreateFile ( wszSrcPath, GENERIC_WRITE, FILE_SHARE_READ,
									  NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
			if (hFile == INVALID_HANDLE_VALUE)
			{
				LogMsg( REPORT_SEVERE, L"[%08x] Fail to create itv file %s. Reason : %s",
						  dwAssetID, wszSrcPath, getErrMsg().c_str());
				delete metaDatas;
				return false;
			}

			DWORD dwWrited;
			WriteFile(hFile, wszText, bufsize, &dwWrited, NULL);
			if (dwWrited != bufsize)
			{
				LogMsg( REPORT_SEVERE, L"[%08x] Fail to write itv file %s. Reason : %s",
						  dwAssetID, wszSrcPath, getErrMsg().c_str());
				CloseHandle(hFile);
				delete metaDatas;
				return false;
			}
			CloseHandle(hFile);
		}

		WCHAR wszDestPath [256];
		wsprintf (wszDestPath, L"%s\\%08x_u.itv", _wszItvImportPath, dwAssetID);
    
		if ( ! ::CopyFile( wszSrcPath, wszDestPath, FALSE ) )
		{
			LogMsg( L"Fail to upload itv file %s. Reason : %s", wszDestPath, getErrMsg().c_str() );
			delete metaDatas;
			return false;
		}
		else
		{
			if ( ! ::DeleteFile (wszSrcPath))
			{
				LogMsg( REPORT_SEVERE, L"Fail to delete file \'%s\' Reason : %s", 
						  wszSrcPath, getErrMsg().c_str() );
			}
			LogMsg( L"[%08x]: Request to update metadata", dwAssetID);
		}
	}
	delete metaDatas;
	
	return true;
}

bool updateMetadata(int scheduleID, const char* programId, const char* assetName, DWORD dwAssetID)
{
	//
	// query metadata
	//
	PMClient pmc(_szMetadataGatewayUrl);
	PMClient::MetaData metaDatas[40];
	int count =pmc.queryMetaData(scheduleID, programId, dwAssetID, metaDatas, sizeof(metaDatas)/sizeof(PMClient::MetaData));
	if (count <=0)
	{
		LogMsg(REPORT_SEVERE, L"Fail to invoke PMClient::queryMetaData");
		return false;
	}
	
	//
	// write second itv file
	//
	{
		// create itv file at local directory _wszTemplatePath
		WCHAR wszSrcPath [256];
		wsprintf (wszSrcPath, L"%s\\import\\%08x_a.itv", _wszTemplatePath, dwAssetID);

		char szBuf[256];
		wstr2str( wszSrcPath, szBuf, sizeof(szBuf) );
		FILE* f = fopen(szBuf, "w");
		if (!f)
		{
			LogMsg( REPORT_SEVERE, L"[%08x] Fail to create itv file %s. Reason : %s",
					  dwAssetID, wszSrcPath, getErrMsg().c_str());
			return false;
		}

		fprintf(f, "; Content import itv file. Generated by AssetGear interface"
				   "\n\n[version]"
				   "\nVersion_Major=1"
				   "\nVersion_Minor=4"
				   "\n\n[uid]"
				   "\n1001=1,%s"
				   "\n1002=2, %s"
				   "\n\n[metadata_1002]\n",
				   _szApplicationName,
				   assetName);

		for (int i=0; i< count; i++)
			//		PMClient::MetaDataCollection::iterator it = metaDatas.begin();
			//	it < metaDatas.end(); it++)
		{
			LogMsg(L"[%08x]: %S = %d, %S", dwAssetID, metaDatas[i].name, metaDatas[i].type, metaDatas[i].value);
			
			fprintf(f, "%s=1001,%d,%s\n", metaDatas[i].name, metaDatas[i].type, metaDatas[i].value);
		}

		fclose(f);
		f=NULL;		

		WCHAR wszDestPath [256];
		wsprintf (wszDestPath, L"%s\\%08x_a.itv", _wszItvImportPath, dwAssetID);
    
		if ( ! ::CopyFile( wszSrcPath, wszDestPath, FALSE ) )
		{
			LogMsg( L"Fail to upload itv file %s. Reason : %s", wszDestPath, getErrMsg().c_str() );
			return false;
		}
		else
		{
			if ( ! ::DeleteFile (wszSrcPath))
			{
				LogMsg( REPORT_SEVERE, L"Fail to delete file \'%s\' Reason : %s", 
						  wszSrcPath, getErrMsg().c_str() );
			}

			LogMsg( L"[%08x]: Request to update metadata", dwAssetID);
		}
	}
	
	return true;
}

bool exchangeAssetElement(int assetIDA, const char* assetA, const char* elementA, int assetIDB, const char* assetB, const char* elementB)
{
	//
	// write exchange ae itv file
	//
	{
		// create itv file at local directory _wszTemplatePath
		WCHAR wszSrcPath [256];
		wsprintf (wszSrcPath, L"%s\\import\\%08x_%08x.itv", _wszTemplatePath, assetIDA, assetIDB);

		char szBuf[256];
		wstr2str( wszSrcPath, szBuf, sizeof(szBuf) );
		FILE* f = fopen(szBuf, "w");
		if (!f)
		{
			LogMsg( REPORT_SEVERE, L"Fail to create itv file %s. Reason : %s",
					  wszSrcPath, getErrMsg().c_str());
			return false;
		}

		fprintf(f, "; Content import itv file. Generated by AssetGear interface"
				   "\n\n[version]"
				   "\nVersion_Major=1"
				   "\nVersion_Minor=4"
				   "\n\n[uid]"
				   "\n1001=2,%s"
				   "\n1002=3,%s"
				   "\n1003=3,%s"
				   "\n1004=2,%s"
				   "\n\n[component_1001]\n"
				   "1002=1"
				   "\n\n[component_1004]\n"
				   "1003=1\n",
				   assetA,
				   elementB,
				   elementA,
				   assetB);

		fclose(f);
		f=NULL;		

		WCHAR wszDestPath [256];
		wsprintf (wszDestPath, L"%s\\%08x_%08x.itv", _wszItvImportPath, assetIDA, assetIDB);
    
		if ( ! ::CopyFile( wszSrcPath, wszDestPath, FALSE ) )
		{
			LogMsg( L"Fail to upload itv file %s. Reason : %s", wszDestPath, getErrMsg().c_str() );
			return false;
		}
		else
		{
			if ( ! ::DeleteFile (wszSrcPath))
			{
				LogMsg( REPORT_SEVERE, L"Fail to delete file \'%s\' Reason : %s", 
						  wszSrcPath, getErrMsg().c_str() );
			}

			LogMsg( L"Request to exchange asset element, Asset A {ID[%08x] Name[%S] Ae[%S]} and Asset B {ID[%08x] Name[%S] Ae[%S]}", 
				assetIDA, assetA, elementA, assetIDB, assetB, elementB);
		}
	}
	
	return true;
}

void SystemTimeToTimet(SYSTEMTIME * pSt, time_t* t)
{
	FILETIME ft;
	SystemTimeToFileTime(pSt, &ft);
	LONGLONG ll = *((LONGLONG*)(&ft));
	ll -= 116444736000000000;
	*t =  (time_t)(ll/10000000);    
}

bool TWCUTCTimeStrToTimet(const char* sTime, time_t* t)
{
	*t = 0;

	int nYear,nMon,nDay, nHour, nMin, nSec;
	if (sscanf(sTime, "%d-%d-%dT%d:%d:%d", &nYear, &nMon, &nDay, &nHour, &nMin, &nSec)<6)
		return false;

	if (nYear < 1970 || nYear > 2100 || nMon < 1 || nMon > 12 || nDay < 1 || nDay > 31 || 
		nHour < 0 || nHour > 23 || nMin < 0 || nMin > 59 || nSec < 0 || nSec > 59)
		return false;

	// convert UTC to time_t
	SYSTEMTIME st_utc;
	memset(&st_utc, 0, sizeof(st_utc));
	st_utc.wYear = nYear;
	st_utc.wMonth = nMon;
	st_utc.wDay = nDay;
	st_utc.wHour = nHour;
	st_utc.wMinute = nMin;
	st_utc.wSecond = nSec;

	SystemTimeToTimet(&st_utc, t);
	
	return true;
}

void ParseErrorFromString(const char* szErrMsg, int& nStatusCode)
{
	if (strstr(szErrMsg, "Object name") && strstr(szErrMsg, "already exist"))
	{
		nStatusCode = STATUS_ASSET_EXIST;
	}
	else if (strstr(szErrMsg, "book RDS fail"))
	{
		nStatusCode = STATUS_NO_RESOURCE;
	}
	else
		nStatusCode = STATUS_OTHER_ERROR;
}

SOAP_FMAC5 int SOAP_FMAC6 AssetGear2__createAsset(struct soap*pSoap, AssetGear2__AssetModel *pAssetmodel, struct AssetGear2__createAssetResponse& response)
{
	 AssetGear2__AssetModel * & pReturn = response._serviceReturn;
	 pReturn = soap_new_AssetGear2__AssetModel(pSoap, -1);
	 pReturn->Schedule = NULL;
	 pReturn->ListOfMetaData = NULL;
	 pReturn->assetName = NULL;
	 pReturn->programID = pAssetmodel->programID;
	 pReturn->assetID = NULL;
	 pReturn->provider = NULL;
	 pReturn->bitRate = NULL;
	 pReturn->soap = pSoap;
	 pReturn->url = NULL;
	 pReturn->retryFlag = _0;

	 // optional attribute
	 pReturn->ListOfMetaData = NULL;
	 pReturn->Schedule = NULL;
	 pReturn->Status = soap_new_AssetGear2__StatusModel(pSoap, -1);

	if (NULL == pAssetmodel || NULL == pReturn)
		return SOAP_NULL;
	
	// read the input parameters from _message
	std::string programId = pAssetmodel->programID;
	std::string assetName;
	int scheduleID;
	std::string startTime, endTime;
	int bitRate  =0;
	int priority =0;
	std::string strProvider = "SeaChange";
	int nRetryFlag = 0;
	std::string strProviderId;
	std::string strProviderAssetId;
	
	if (NULL == pAssetmodel->assetName)
	{
		LogMsg(REPORT_SEVERE, L"Asset name is empty");
		return SOAP_REQUIRED;
	}

	assetName = *(pAssetmodel->assetName);

	if (pAssetmodel->ListOfMetaData)
	{
		for (std::vector<class AssetGear2__MetaDataModel * > ::iterator it = pAssetmodel->ListOfMetaData->MetaData.begin();
		it < pAssetmodel->ListOfMetaData->MetaData.end(); ++it)
		{
			if (!stricmp("ProviderAssetId", (*it)->mdName.c_str()))
			{
				strProviderAssetId = (*it)->__item;
			}
			else if ( !stricmp("ProviderId", (*it)->mdName.c_str()))
			{
				strProviderId = (*it)->__item; 
			}
		}
	}

	if (NULL != pAssetmodel->bitRate)
		bitRate = *(pAssetmodel->bitRate);

	nRetryFlag = pAssetmodel->retryFlag;

	if (NULL != pAssetmodel->provider)
	{
		strProvider = *(pAssetmodel->provider);
	}
	else
	{
		strProvider = _szProvider;
	}
	
	if (NULL == pAssetmodel->Schedule)
	{
		LogMsg(REPORT_SEVERE, L"Schedule is NULL");
		return SOAP_REQUIRED;
	}
	
	scheduleID = pAssetmodel->Schedule->scheduleID;
	if (NULL != pAssetmodel->Schedule->startTime)
		startTime = *(pAssetmodel->Schedule->startTime);
	if (NULL != pAssetmodel->Schedule->endTime)
		endTime = *(pAssetmodel->Schedule->endTime);
	if (NULL != pAssetmodel->Schedule->priority)
		priority = *(pAssetmodel->Schedule->priority);

	//priority always set to 55
	priority = 55;
	
	if (nRetryFlag)
		LogMsg(L"Request to repair asset[%S], st[%S],et[%S],"
			L"bt[%d],provier[%S],priority[%d],programid[%S],scheduleID[%d],providerId[%S],providerAssetId[%S]",
			assetName.c_str(),
			startTime.c_str(),
			endTime.c_str(),
			bitRate,
			strProvider.c_str(),
			priority,
			programId.c_str(),
			scheduleID,
			strProviderId.c_str(),
			strProviderAssetId.c_str());
	else
		LogMsg(L"Request to create asset[%S], st[%S],et[%S],"
			L"bt[%d],provier[%S],priority[%d],programid[%S],scheduleID[%d],providerId[%S],providerAssetId[%S]",
			assetName.c_str(),
			startTime.c_str(),
			endTime.c_str(),
			bitRate,
			strProvider.c_str(),
			priority,
			programId.c_str(),
			scheduleID,
			strProviderId.c_str(),
			strProviderAssetId.c_str());

	{// adjust time, if the time is 0
		if (startTime == "0")
			startTime = "";

		if (endTime == "0")
			endTime = "";
	}

	if (!startTime.empty() && !endTime.empty())
	{
		// check time
		time_t tStart, tEnd;
		bool bTimeError = true;
		
		if (TWCUTCTimeStrToTimet(startTime.c_str(), &tStart) && TWCUTCTimeStrToTimet(endTime.c_str(), &tEnd))
		{
			if ((DWORD)tEnd>(DWORD)tStart)
			{
				if ((DWORD)tStart+_dwTimeWindowSecs>(DWORD)time(0))
				{
					bTimeError = false;
				}
				else
				{
					LogMsg(REPORT_SEVERE, L"Time error, StartTime(%S)<Now", startTime.c_str());
				}
			}
			else
			{
				LogMsg(REPORT_SEVERE, L"Time error, EndTime(%S)<= StartTime(%S)", endTime.c_str(),startTime.c_str());
			}
		}
		else
		{
			LogMsg(REPORT_SEVERE, L"Time format error");
		}
		
		if (bTimeError)
		{
			soap_set_string(pReturn->soap, pReturn->url, "");
			soap_set_string(pReturn->soap, pReturn->assetID, "");
			soap_set_string(pReturn->soap, pReturn->Status->errorMessage, _szStatusDesc[STATUS_TIME_ERROR]);
			pReturn->Status->errorCode = STATUS_TIME_ERROR;
			
			return SOAP_OK;			
		}
	}

	char szContentName[256];
	strcpy(szContentName, assetName.c_str());
		
	int dwAssetID, dwAeID;
	char szPushUrl[256];
	char szErrMsg[256];
	
	bool bAssetCreated = false;
	bool bNeedCreateContent = true;

	//
	// check retry flag
	//
	if(nRetryFlag)
	{
		//
		// repair not to update Metadata
		//
		bNeedCreateContent = false;

		if (repairContent(szContentName,
					   assetName.c_str(),
					   startTime.c_str(),
					   endTime.c_str(),
					   bitRate,
					   strProvider.c_str(),
					   priority,
					   dwAssetID, 
					   dwAeID, 
					   szPushUrl,
					   sizeof(szPushUrl),
					   szErrMsg,
					   sizeof(szErrMsg),
					   bNeedCreateContent))
		{
			LogMsg(L"Asset [%S] repaired, asset id [%08x], aeid[%08x], pushUrl [%S]", szContentName,
				dwAssetID, dwAeID, szPushUrl);
		}
		else
		{
			if (!bNeedCreateContent)
			{
				LogMsg(REPORT_SEVERE, L"Fail to repairContent() with error [%S]", szErrMsg);

				{
					// parse the szErrMsg to 
					int nStatus;
					ParseErrorFromString(szErrMsg, nStatus);
					
					soap_set_string(pReturn->soap, pReturn->url, "");
					soap_set_string(pReturn->soap, pReturn->assetID, "");

					if (nStatus == STATUS_OTHER_ERROR)
					{
						soap_set_string(pReturn->soap, pReturn->Status->errorMessage, szErrMsg);
						pReturn->Status->errorCode = STATUS_OTHER_ERROR;
					}
					else
					{
						soap_set_string(pReturn->soap, pReturn->Status->errorMessage, _szStatusDesc[nStatus]);
						pReturn->Status->errorCode = nStatus;
					}
				}

				return SOAP_OK;
			}
			else
			{
				LogMsg(L"[%S]: Content not exist when repairContent, try createContent", szContentName);
			}
		}
	}

	if (bNeedCreateContent)
	{
		if (!createContent(szContentName,
			assetName.c_str(),
			startTime.c_str(),
			endTime.c_str(),
			bitRate,
			strProvider.c_str(),
			strProviderId.c_str(),
			strProviderAssetId.c_str(),
			priority,
			dwAssetID, 
			dwAeID, 
			szPushUrl,
			sizeof(szPushUrl),
			szErrMsg,
			sizeof(szErrMsg)))
		{
			LogMsg(REPORT_SEVERE, L"Fail to createContent() with error [%S]", szErrMsg);

			{
				// parse the szErrMsg to 
				int nStatus;
				ParseErrorFromString(szErrMsg, nStatus);

				soap_set_string(pReturn->soap, pReturn->url, "");
				soap_set_string(pReturn->soap, pReturn->assetID, "");
				
				if (nStatus == STATUS_OTHER_ERROR)
				{
					soap_set_string(pReturn->soap, pReturn->Status->errorMessage, szErrMsg);
					pReturn->Status->errorCode = STATUS_OTHER_ERROR;
				}
				else
				{
					soap_set_string(pReturn->soap, pReturn->Status->errorMessage, _szStatusDesc[nStatus]);
					pReturn->Status->errorCode = nStatus;
				}
			}

			return SOAP_OK;
		}
		
		bAssetCreated = true;
		LogMsg(L"Asset [%S] created, asset id [%08x], aeid[%08x], pushUrl [%S]", szContentName,
			dwAssetID, dwAeID, szPushUrl);
	}


	//
	// update metadata
	//
	{
		int nTry = 2;
		bool bRet;
		while(nTry>0)
		{
#ifdef _USE_UNICODE_ITV_
			bRet = updateMetadata_unicode_itv(scheduleID, programId.c_str(), assetName.c_str(), dwAssetID);
#else
			bRet = updateMetadata(scheduleID, programId.c_str(), assetName.c_str(), dwAssetID);
#endif
			
			if (bRet)
			{
				LogMsg(L"Asset [%08x] metadata updated", dwAssetID);
				
				break;
			}
			
			nTry--;
//			Sleep(1000);
		}
		if (!bRet)
		{
			if (bAssetCreated && _dwFailCAWhenMdFail)
			{
				deleteContent(szContentName);
				soap_set_string(pReturn->soap, pReturn->url, "");
				soap_set_string(pReturn->soap, pReturn->assetID, "");
				soap_set_string(pReturn->soap, pReturn->Status->errorMessage, _szStatusDesc[STATUS_METADATA]);
				pReturn->Status->errorCode = STATUS_METADATA;
				LogMsg(L"Fail to update metadata of asset [%08x], created asset deleted", dwAssetID);		
				
				return SOAP_OK;				
			}
			else
				LogMsg(L"Fail to update metadata of asset [%08x]", dwAssetID);		
		}
	}

	//
	// return the result
	//
	soap_set_string(pReturn->soap, pReturn->url, szPushUrl);
	char szAssetId[16];
	sprintf(szAssetId, "%08X", dwAssetID);
	soap_set_string(pReturn->soap, pReturn->assetID, szAssetId);
	pReturn->Status->errorMessage = NULL;
	pReturn->Status->errorCode = STATUS_OK;	
	
	return SOAP_OK;
}

