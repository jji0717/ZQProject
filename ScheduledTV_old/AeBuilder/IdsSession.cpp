// ===========================================================================
// Copyright (c) 1997, 1998 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
// Ident : $Id: AeBuidler.cpp
// Author: Kaliven Lee
// Desc  : IDS session manager
// Revision History: 
// ---------------------------------------------------------------------------
// $Revision: 1 $
// $Log: /ZQProjs/ScheduledTV_old/AeBuilder/IdsSession.cpp $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 2     04-10-08 11:19 Kaliven.lee
// add function
// 
// 1     04-10-08 10:43 Kaliven.lee
// file create
// IdsSession.cpp: implementation of the IdsSession class.
//
//////////////////////////////////////////////////////////////////////
#include "Log.h"
//#include "stdafx.h"
#include "IdsSession.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


#pragma comment(lib,"idsapi"LIBEXTERN)


using ZQ::common::Log;
IdsSession::IdsSession():
m_isUnIntialized(false)
{

}

IdsSession::~IdsSession()
{
	if(!m_isUnIntialized)
		unInitialize();
}
bool IdsSession::initialize(wchar_t* wszServer,wchar_t* wszUser,DWORD dwTimeOut,WORD IDSVer)
{
	LogEvent(Log::L_DEBUG,"AeBuilder start initializing.\n");

	m_wVersion = IDSVer;

	ITVVERSION Version; 
	Version.wVersion = IDSVer;

	RTNDEF rtn = IdsInitialize(&Version,NULL);
	if(ITV_INVALID_VERSION == rtn)
	{		
		LogEvent(Log::L_ERROR,"Invalid IDS version.AeBuilder initialized failed!\n");
		return false;
	}


	LogEvent(Log::L_DEBUG,"bind Ids session to %s.\n",wszServer);
	wcscpy(m_wszServer,wszServer);
	wcscpy(m_wszUser,wszUser);
	m_dwTimeOut = dwTimeOut;
	ITVSTATUSBLOCK ItvSb;
	// here 7th parameter must be set as NULL to call the function synchronized
	rtn = IdsBind (m_wszServer,m_wszUser,IDS_READ_ONLY, m_dwTimeOut,
             &m_IdsSess,&ItvSb,NULL,NULL);
	if(rtn != ITV_SUCCESS)
	{
		LogEvent(Log::L_ERROR,"Bind to IDS server failed\n");
		return false;
	}
	
	return true;
}
void IdsSession::unInitialize(void)
{
	LogEvent(Log::L_DEBUG,"AeBuilder start unInitialized.\n");
	ITVSTATUSBLOCK ItvSb;	
	RTNDEF rtn = IdsUnbind(&m_IdsSess,IDS_READ_ONLY,&ItvSb,NULL);
	rtn = IdsUninitialize();
	m_isUnIntialized = true;
}
HRESULT IdsSession::GetAppMetaData(const DWORD appUid, const WCHAR* metaDataName, METADATA& metaData)
{
	if (metaDataName == NULL || *metaDataName == 0x00)
		return S_FALSE;

	DWORD dwNumMeta;
	METADATA* Mds2Read[2], * pMDs, Md2Read;
	
	// init the query input
	memset(&Md2Read, 0x00, sizeof(Md2Read));
	Md2Read.Version.wVersion = m_wVersion;

	BOOL succ = S_FALSE;

	wcscpy(Md2Read.wszMdName, L"*");
	
	Mds2Read[0] = &Md2Read;
	Mds2Read[1] = NULL;

	ITVSTATUS status  = IdsReadApplication(&m_IdsSess, appUid, Mds2Read, 1, &pMDs, 
							&dwNumMeta, NULL, NULL); // performed synchronously
														//IdsCallBack);
	if(status == IDS_NETWORK_ERROR)
	{
		if(reConnect())
		{
			status = IdsReadApplication(&m_IdsSess, appUid, Mds2Read, 1, &pMDs, 
							&dwNumMeta, NULL, NULL);
		}		
		else 
		{
			LogEvent(Log::L_ERROR,"net disconnected\n");
			return status;
		}
	}
	if (ITV_SUCCESS != status)	// && ITV_PENDING != status)
	{
		LogEvent(Log::L_ERROR, "IdsSession::GetAppMeta, IdsReadApplication Fail\n");
		return status;
	}
	else
	{
		for (DWORD i = 0; i < dwNumMeta; i++)
		{
			if (wcscmp(metaDataName, pMDs[i].wszMdName) ==0)
			{
				succ = S_OK;
				memcpy(&metaData, &(pMDs[i]), sizeof(metaData));
				break;
			}
		}
		
		IdsFreeMd(pMDs, dwNumMeta);
	} 
	
	return succ;
}


HRESULT IdsSession::getElementList(PASSETS pAssetList)
{
	HRESULT hr = S_OK;
	DWORD i = 0;
	DWORD AssetNum = pAssetList->size();
	if(AssetNum == 0) 
		return S_FALSE;
	OBJECTLIST * pObjList = new OBJECTLIST[AssetNum];
	for(i =0; i < AssetNum; i++)
	{
		pObjList[i].dwUid = pAssetList->at(i).dwAssetUID;
		pObjList[i].Version.wVersion = IDS_VERSION_1_0;
	}

	METADATA * metaData[3];
	metaData[0] = new METADATA();
	memset(metaData[0],0,sizeof(METADATA));	
	wcscpy(metaData[0]->wszMdName , L"BitRate");
	metaData[0]->Version.wVersion = IDS_VERSION_1_0;	
	metaData[0]->wMdType = IDS_INTEGER;	

	metaData[1] = new METADATA();
	memset(metaData[1],0,sizeof(METADATA));	
	wcscpy(metaData[1]->wszMdName , L"PlayTime");
	metaData[1]->Version.wVersion = IDS_VERSION_1_0;	
	metaData[1]->wMdType = IDS_INTEGER;	

	metaData[2] = new METADATA();
	memset(metaData[2],0,sizeof(METADATA));	
	wcscpy(metaData[2]->wszMdName , L"PlayTimeFraction");
	metaData[2]->Version.wVersion = IDS_VERSION_1_0;	
	metaData[2]->wMdType = IDS_INTEGER;		
	
	

	AEMD* pAeMetaData = NULL;
	DWORD AeNum = 0;
	ITVSTATUSBLOCK itvSb;
	

		
	RTNDEF rtn = IdsGetAssetAndMd(&m_IdsSess,
									pObjList,
									AssetNum,
									metaData,
									3,
									&pAeMetaData,
									&AeNum,
									&itvSb,
									NULL);
	

		
	if(rtn == ITV_TCP_STATE_ERROR)
	{		
		if(reConnect())
		{
			rtn = IdsGetAssetAndMd(&m_IdsSess,
								pObjList,
								AssetNum,
								metaData,
								3,
								&pAeMetaData,
								&AeNum,
								&itvSb,
								NULL);
		
		}
		else
		{
			LogEvent(Log::L_ERROR,"net disconnected\n");
			return rtn;
		}
	}

	if(rtn !=ITV_SUCCESS)
	{
		LogEvent(Log::L_ERROR,"Can not get data\n");
		return S_FALSE;
	}
	for(i = 0; i < 3 ; i++)
	{
		if(metaData[i] == NULL)
			free(metaData[i]);
	}

	if(rtn != ITV_SUCCESS)
	{
		free(pObjList);
		return rtn;
	}
	
	for(DWORD j = 0; j < AeNum ; j++)
	{	ASSETELEMENT AElement;
		AElement.dwAEUID = pAeMetaData[j].dwAeUid;
		DWORD dwAssetPlayTime = 0;
		long duration= 0;
		int nAttr = 0;
		for(i = 0; i < pAeMetaData[j].dwNumMd; i ++)
		{
			if(wcscmp(pAeMetaData[j].pMd[i].wszMdName,L"BitRate") == 0)
			{
				AElement.dwBitRate = pAeMetaData[j].pMd[i].iVal;
				nAttr++;
			}
			if(wcscmp(pAeMetaData[j].pMd[i].wszMdName,L"PlayTime") == 0)
			{
				AElement.dwPlayTime = pAeMetaData[j].pMd[i].iVal;
				nAttr++;
			}
			if(wcscmp(pAeMetaData[j].pMd[i].wszMdName,L"PlayTimeFraction") == 0)
			{
				AElement.dwPlayTime += pAeMetaData[j].pMd[i].iVal /1000;				
				nAttr++;
			}
			

			if(3 == nAttr)
			{
				nAttr = 0;
				break;
			}
		}
		dwAssetPlayTime += AElement.dwPlayTime;	
		for(i = 0;i < pAssetList->size(); i ++)
		{
			if(pAssetList->at(i).dwAssetUID == pAeMetaData[j].dwAssetUid)
			{
				bool flag = false;
				for(DWORD j = 0; j < pAssetList->at(i).AssetElements.size(); j ++)
				{
					if(pAssetList->at(i).AssetElements.at(j).dwAEUID == AElement.dwAEUID)
					{
						flag = true;
						break;
					}
				}
				if(flag)
					continue;

				pAssetList->at(i).dwPlayTime = dwAssetPlayTime;
				pAssetList->at(i).AssetElements.push_back(AElement);
			}
		}
	}
	
	if(pObjList)
	{
		free(pObjList);
	}
	IdsFreeAeMd(pAeMetaData,AeNum);

	return hr;
}

bool IdsSession::reConnect(void)
{
	/// rebind server
	ITVSTATUSBLOCK ItvSb;
	RTNDEF Rtn = IdsUnbind(&m_IdsSess,IDS_READ_ONLY,&ItvSb,NULL);
//	Rtn = IdsUninitialize();
	for(int i = 0; i< 2 ; i ++)
	{
		Rtn = IdsBind(m_wszServer,m_wszUser,IDS_READ_ONLY,m_dwTimeOut,&m_IdsSess,&ItvSb,NULL,NULL);
		if (Rtn == ITV_SUCCESS)
			return true;
	}
	
	return false;	
}

HRESULT IdsSession::ListApplications(APPNAMES &apps)
{
	APPNAME* pApps = NULL;
	DWORD dwNumFound = 0;
	LogEvent(Log::L_DEBUG, "Enter IdsSession::ListApplications");
	ITVSTATUS status = IdsListApplications(&m_IdsSess, &pApps, &dwNumFound, 
				NULL, NULL); //IdsCallBack);
	/// retry if disconnected
	if(status == ITV_TCP_STATE_ERROR)
	{
		if(reConnect())
		{
			status = IdsListApplications(&m_IdsSess, &pApps, &dwNumFound, 
				NULL, NULL); 
		}
		else
		{
			LogEvent(Log::L_ERROR,"net disconnected\n");
			return status;
		}
	}
	
	if (ITV_SUCCESS != status ) //&& ITV_PENDING != status)
	{
		LogEvent(Log::L_ERROR, "IdsSession IdsListAppilcation fail");
		return status;
	}
	else // success
	{
			// copy the site infos to local and free the memory in ITV api
		for (DWORD i = 0; i < dwNumFound; i++)
			apps.push_back(pApps[i]);
		if (dwNumFound > 0)
			IdsFree(pApps);			
	}
	return S_OK;
}

HRESULT IdsSession::GetAppMetaData(const DWORD appUid, const WCHAR* metaDataName, DWORD& dwVal)
{
	METADATA result;

	HRESULT hr = GetAppMetaData(appUid, metaDataName, result);
	
	if (hr != S_OK || result.wMdType != 1)
		return S_FALSE;

	dwVal = result.iVal;
	
	return S_OK;
}

HRESULT IdsSession::GetAppMetaData(const DWORD appUid, const WCHAR* metaDataName, std::wstring& wstrVal)
{
	METADATA result;

	HRESULT succ = GetAppMetaData(appUid, metaDataName, result);

	if (succ!=S_OK || result.wMdType != 1)
		return S_FALSE;

	
	WCHAR buf[32];

	switch (result.wMdType)
	{
	case 1: // integer value, then convert it to wstring
		swprintf(buf, L"%08x", result.iVal);
		wstrVal = buf;
		break;
		
	case 4: // string value
		wstrVal = result.sVal;
		break;
		
	default:
		swprintf(buf, L"other valuetype: %08x", result.wMdType);
		wstrVal = buf;
		succ= FALSE;
		break;
	}
	
	return S_OK;
}
void IdsSession::LogEvent(int loglevel,char* msg,...)
{
	char newMsg[2048];
	
	va_list args;
	va_start(args, msg);
	vsprintf(newMsg, msg, args);
	va_end(args);

	printf(newMsg);
}

HRESULT IdsSession::getElementList(DWORD dwAssetID,PASSETELEMENTS pElementList)
{
	HRESULT hr = S_OK;
	LogEvent(Log::L_ERROR,"start build process!\n");	

	///		get Elements' list of a Asset
	//@1	initialize the arguments of IdsGetAsset function
	
	IDSUIDUPDATESTAMP assetStamp;
	COMPLEXASSET *complexAsset= NULL;
	ITVSTATUSBLOCK ItvSb;
	DWORD dwNum = 0;	
	//@2	get Elements' id of a Asset
	RTNDEF rtn = -1;
	rtn = IdsGetAsset(&m_IdsSess,dwAssetID,&assetStamp,&complexAsset,&dwNum,&ItvSb,NULL);
	

	if((rtn != ITV_SUCCESS)&&(rtn != ITV_PENDING))
	{
		LogEvent(Log::L_ERROR,"Error when call IdsGetAsset()\n");	
		return rtn;
	}
	if(0 == dwNum)
	{
		LogEvent(Log::L_DEBUG,"No Asset element matched\n");
		return rtn;
	}
	
	/// insert the element id into playlist
	//@1	insert the element id	
	

	DWORD i = 0;
	for (i = 0; i < dwNum; i++)
	{
		ASSETELEMENT element;
		element.dwAEUID = complexAsset[i].dwUid;
		// get element's metadata
		METADATA * metaData[3];
		metaData[0] = new METADATA();
		memset(metaData[0],0,sizeof(METADATA));	
		wcscpy(metaData[0]->wszMdName , L"BitRate");
		metaData[0]->Version.wVersion = IDS_VERSION_1_0;	
		metaData[0]->wMdType = IDS_INTEGER;	

		metaData[1] = new METADATA();
		memset(metaData[1],0,sizeof(METADATA));	
		wcscpy(metaData[1]->wszMdName , L"PlayTime");
		metaData[1]->Version.wVersion = IDS_VERSION_1_0;	
		metaData[1]->wMdType = IDS_INTEGER;	

		metaData[2] = new METADATA();
		memset(metaData[2],0,sizeof(METADATA));	
		wcscpy(metaData[2]->wszMdName , L"PlayTimeFraction");
		metaData[2]->Version.wVersion = IDS_VERSION_1_0;	
		metaData[2]->wMdType = IDS_INTEGER;			
	

		DWORD metaDataNum = 0;
		METADATA* aeMetaData = NULL;
		rtn = IdsReadAtomicElement(&m_IdsSess,element.dwAEUID,metaData,3,&aeMetaData,&metaDataNum,&ItvSb,NULL);

		for(i = 0; i< 3 ; i ++)
		{
			if(metaData[i])
				free(metaData[i]);
		}

		
		if(rtn != ITV_SUCCESS)
		{
			
			LogEvent(Log::L_ERROR,"Read Asset Element error\n");
			
			IdsFree(complexAsset);
			return rtn;
		}
		for(DWORD j = 0 ; j < metaDataNum ; j++)
		{
			long duration = 0;
			if(wcscmp(aeMetaData[j].wszMdName,L"BitRate") == 0)
			{
				element.dwBitRate = aeMetaData[j].iVal ;
			}
			if(wcscmp(aeMetaData[j].wszMdName,L"PlayTime")==0)
			{
				element.dwPlayTime = aeMetaData[j].iVal;
			}
			if(wcscmp(aeMetaData[j].wszMdName,L"PlayTimeFraction")==0)
			{
				element.dwPlayTime+= aeMetaData[j].iVal /1000;			
			}
		}	
		pElementList->push_back(element);
		rtn = IdsFreeMd(aeMetaData,metaDataNum);		
	}
	//@2r	free the memory allocate by IdsGetAsset  
	// here is a must. Note!
	rtn = IdsFree(complexAsset);
	if(rtn!=ITV_SUCCESS)
	{
		LogEvent(Log::L_ERROR,"Error to free memory allocated by IDS\n");

		return rtn;
	}
	
	return hr;
}