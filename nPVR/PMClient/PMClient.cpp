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
// Desc  : impl the soap interface of AssetGear to query ZQ Program Manager
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/nPVR/PMClient/PMClient.cpp 1     10-11-12 16:01 Admin $
// $Log: /ZQProjs/nPVR/PMClient/PMClient.cpp $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 5     06-01-06 19:15 Jie.zhang
// 
// 4     05-12-28 18:52 Jie.zhang
// 
// 3     05-11-22 19:13 Jie.zhang
// 
// 2     11/04/05 3:35p Hui.shao
// ============================================================================================

#include "PMClient.h"
#include "./soapProgramManagementServiceSoapBindingProxy.h"	// get proxy
#include "./ProgramManagementServiceSoapBinding.nsmap"		// get namespace bindings


typedef ProgramManagementServiceSoapBinding proxy_object;

PMClient::PMClient(const char* pmUrl)
{
	if (NULL != pmUrl)
		_pmUrl = pmUrl;
}

PMClient::~PMClient()
{
}

const int PMClient::queryMetaData(int scheduleID, const char* programId, int assetID, MetaData* pMetaData, const int maxSize)
{
	if (NULL == pMetaData || maxSize <=0)
		return 0;

	std::auto_ptr<proxy_object> pObject(new proxy_object);

	if (!_pmUrl.empty())
		pObject->endpoint = _pmUrl.c_str();

	if (NULL == programId)
		return 0;

	//prepare the schedule model
	pm2__ScheduleModel* pSchdmodel = soap_new_pm2__ScheduleModel(pObject->soap, -1);
	if (NULL == pSchdmodel)
		return 0;

	pSchdmodel->scheduleID = scheduleID;	// required attribute
	// optional attributes
	pSchdmodel->channelID = NULL;
	pSchdmodel->channelName = NULL;
	pSchdmodel->startTime = NULL;
	pSchdmodel->endTime = NULL;
	pSchdmodel->priority = NULL;

	// prepare pAssetmodel
	pm2__AssetModel* pAssetmodel = soap_new_pm2__AssetModel(pObject->soap, -1);
	if (NULL == pAssetmodel)
		return 0;

	// required attribute
	pAssetmodel->programID = programId;
	pAssetmodel->Schedule = pSchdmodel;

	// optional attribute
	pAssetmodel->ListOfMetaData = NULL;
	pAssetmodel->assetName = NULL;
	pAssetmodel->assetID = &assetID;
	pAssetmodel->url = NULL;
	pAssetmodel->bitRate = NULL;
	pAssetmodel->provider = NULL;

	pm2__queryMetaDataResponse response;
	response._serviceReturn=NULL;

	MetaData* pCurrentMetaData = pMetaData;

//#define DUMY_PM
#ifdef DUMY_PM

	static MetaData dummymeta[] = {
		{"Genre",				1, "0"},
		{"Sub-Genre",			1, "0"},
		{"Description",	4, "dummy"},
		{"LongDescription",	4, "dummy"},
		{"CreationDate",	5, "20051011"},
		{"Provider",	4, "dummy"},
		{"ProviderId",	4, "dummy"},
		{"ProviderAssetId",	4, ""},
		{"ProfileId",	4, "dummy"},
		{"NumberInProfile",	1, "1"},
		{"SeriesId",	4, "321"},
		{"SeasonNumberInSeries",	1, "1"},
		{"EpisodeInSeries",		1, "1"},
		{"Title",	4, "æ˜¥å¤©ç¾Žä¸½çš„Girl"},
		{"TitleBrief",	4, "dummy"},
		{"TitleSortName",	4, "dummy"},
		{"PosterBoard",	4, "dummy"},
		{"Actors",	4, "dummy"},
		{"Directors",	4, "dummy"},
		{"Producers",	4, "dummy"},
		{"Writers",	4, "dummy"},
		{"YearOfRelease",	1, "2004"},
		{"CountryOfOrigin",	4, "republic of dummy"},
		{"ScreenFormat",	1, "1"},
		{"AudioType",	1, "1"},
		{"Rating",	1, "1"},
		{"Language",	1, "0"},
		{"SubtitleLanguage",	1, "1"},
		{"DubbedLanguage",	1, "2"},
		{"price",	3, "1"},
		{"RentalTime",	2, "1.0"},
		{"ExtendRentalPrice",	3, "1"}
	};

#if 0
	char pp[] = "´ºÌìÃÀÀöµÄGirl";
	wchar_t dd[256];
	printf("ansi :%d\n", strlen(pp));
	printf("unicode :%d\n", MultiByteToWideChar(CP_ACP, 0, pp, -1, dd, sizeof(dd)/sizeof(wchar_t))); ///---> in dll, this sentence return error, len is 15, but should be 10
	printf("utf8 :%d\n", WideCharToMultiByte(CP_UTF8, 0, dd, -1, dummymeta[13].value, sizeof(dummymeta[13].value), 0, 0));
#endif

	int dummymsz = sizeof(dummymeta) / sizeof(const MetaData);

	for (int i =0; i< sizeof(dummymeta) / sizeof(const MetaData) && i<maxSize; i++)
		memcpy(pCurrentMetaData++, &(dummymeta[i]), sizeof(MetaData));

	return i;
#else
	if (0 != pObject->pm2__queryMetaData(pAssetmodel, response) || NULL == response._serviceReturn)
		return 0;
#endif

	if (NULL == response._serviceReturn->ListOfMetaData || response._serviceReturn->ListOfMetaData->MetaData.size()<=0)
	{
		return 0;
	}

	//TODO prepare the result metadata
	int idx=0;
	for (std::vector<class pm2__MetaDataModel * > ::iterator it =response._serviceReturn->ListOfMetaData->MetaData.begin();
	it < response._serviceReturn->ListOfMetaData->MetaData.end() && idx <maxSize; ++it)
	{
		strcpy(pCurrentMetaData[idx].name, (*it)->mdName.c_str());
		pCurrentMetaData[idx].type = (*it)->mdType;
		strcpy(pCurrentMetaData[idx].value, (*it)->__item.c_str());

		switch(pCurrentMetaData[idx].type)
		{
		case 1: // integer
		case 2: // real
		case 3: // float
		case 4: // string
		case 5: // datatime
			idx ++;
			break;
		case 6: // binary
		case 7: // boolean
		case 8: // asni
		default:
			break;
		}
	}

	return idx;
}

#ifdef __TEST
int main()
{
	PMClient pmc;
	PMClient::MetaDataCollection metedatas;
	return pmc.queryMetaData(1,1, metedatas);
}
#endif // __TEST

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}
