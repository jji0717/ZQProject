
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
// Desc  : Build an asset element list by a asset id
// Revision History: 
// ---------------------------------------------------------------------------
// $Revision: 1 $
// $Log: /ZQProjs/ScheduledTV_old/AeBuilder/AeBuilder.cpp $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 12    04-12-03 10:57 Kaliven.lee
// free change to delete
// 
// 11    04-10-08 10:44 Kaliven.lee
// 
// 10    04-09-29 9:57 Kaliven.lee
// avoid repeat push the same assetelement to list
// 
// 9     04-09-29 9:40 Kaliven.lee
// compatible with filler
// 
// 8     04-09-28 15:10 Bernie.zhao
// 
// 7     04-09-23 15:29 Kaliven.lee
// modify out put structure
// 
// 6     04-09-23 12:22 Kaliven.lee
// get the inform once
// 
// 5     04-09-22 18:51 Kaliven.lee
// get detail information of element
// 
// 4     04-09-20 10:37 Kaliven.lee
// modify the interface of getElementList 
// 
// 3     04-09-16 17:17 Kaliven.lee

// AeBuilder.cpp : Defines the entry point for the console application.
//

#include "./AeBuilder.h"


#ifndef LIBEXTERN
#	ifdef _DEBUG 
#		define	LIBEXTERN "_d.lib"
#	else
#		define	LIBEXTERN ".lib"
#	endif
#endif

#pragma comment(lib,"idsapi"LIBEXTERN)

bool AeBuilder::m_isConnected = false;

AeBuilder::AeBuilder():
m_isUnIntialized(false)
{

	m_isConnected = false;
}
AeBuilder::~AeBuilder()
{
	if(!m_isUnIntialized)
		unInitialize();
}

void cbConnectionLost(IDSSESS *pIdsSess)
{
	AeBuilder::LogEvent(Log::L_ERROR,"IDS Connection Lost!\n");
	AeBuilder::m_isConnected = false;
}
HRESULT AeBuilder::getElementList(DWORD dwAssetID,PASSETELEMENTS pElementList)
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
				delete metaData[i];
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
bool AeBuilder::initialize(wchar_t* wszServer,wchar_t* wszUser,DWORD dwTimeOut,WORD IDSVer)
{
	LogEvent(Log::L_DEBUG,"AeBuilder start initializing.\n");
	wcscpy(m_wszServer,wszServer);
	wcscpy(m_wszUser,wszUser);
	m_dwTimeOut = dwTimeOut;
	m_wVersion = IDSVer;

	ITVVERSION Version; 
	Version.wVersion = IDSVer;

	RTNDEF rtn = IdsInitialize(&Version,NULL);
	if(ITV_INVALID_VERSION == rtn)
	{		
		LogEvent(Log::L_ERROR,"Invalid IDS version.AeBuilder initialized failed!\n");
		return false;
	}
	ITVSTATUSBLOCK ItvSb;
	// here 7th parameter must be set as NULL to call the function synchronized
	rtn = IdsBind (m_wszServer,m_wszUser,IDS_READ_ONLY, m_dwTimeOut,
             &m_IdsSess,&ItvSb,NULL,NULL);
	if(rtn != ITV_SUCCESS)
	{
		LogEvent(Log::L_ERROR,"Bind to IDS server failed\n");
		return false;
	}
	m_isConnected = true;
	return true;
}

void AeBuilder::unInitialize(void)
{
	LogEvent(Log::L_DEBUG,"AeBuilder start unInitialized.\n");
	ITVSTATUSBLOCK ItvSb;	
	RTNDEF rtn = IdsUnbind(&m_IdsSess,IDS_READ_ONLY,&ItvSb,NULL);
	rtn = IdsUninitialize();
	m_isUnIntialized = true;
}
void AeBuilder::LogEvent(int logLevel,char * msg,...)
{// set your logevent here 
	char newMsg[2048];
	
	va_list args;
	va_start(args, msg);
	vsprintf(newMsg, msg, args);
	va_end(args);

	printf(newMsg);
}

HRESULT AeBuilder::getElementList(PASSETS pAssetList)
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
			return S_FALSE;
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
bool AeBuilder::reConnect(void)
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
