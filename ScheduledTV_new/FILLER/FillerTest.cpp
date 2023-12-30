
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
// Ident : $Id: FillerTest.CPP
// Author: Kaliven Lee
// Desc  : Test Filler  
// Revision History: 
// ---------------------------------------------------------------------------
// $Revision: 1 $
// $Log: /ZQProjs/ScheduledTV_new/FILLER/FillerTest.cpp $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 1     05-08-30 18:29 Bernie.zhao
// 
// 1     05-08-30 18:28 Bernie.zhao
// 
// 5     04-09-29 11:49 Kaliven.lee
// output point list
// 
// 4     04-09-29 9:05 Kaliven.lee
// compatible with aebuilder
// 
// 3     04-09-23 17:13 Kaliven.lee
// 
// 2     04-09-16 17:15 Kaliven.lee

#include "autofiller.h"
#include "barkerfiller.h"

void resetPlayList(ASSETS *playlist)
{
	playlist->clear();
	ASSET node;
	node.dwAssetUID = 11;
	node.dwWeight = 3;
	playlist->push_back(node);

	node.dwAssetUID = 12;
	node.dwWeight = 7;
	playlist->push_back(node);

	node.dwAssetUID = 13;
	node.dwWeight = 2;
	playlist->push_back(node);

	node.dwAssetUID = 14;
	node.dwWeight = 7;
	playlist->push_back(node);

	node.dwAssetUID = 16;
	node.dwWeight = 5;
	playlist->push_back(node);

	node.dwAssetUID = 16;
	node.dwWeight = 3;
	playlist->push_back(node);
}

int main(int argv,char* argc[])
{
	ASSETS filler ;	
	ASSET node;

	node.dwAssetUID = 1001;
	node.dwPlayTime = 30;
	node.dwWeight = 6;
	filler.push_back(node);

	node.dwAssetUID = 1002;
	node.dwPlayTime = 30;
	node.dwWeight = 2;
	filler.push_back(node);

	node.dwAssetUID = 1003;
	node.dwPlayTime = 30;
	node.dwWeight = 5;
	filler.push_back(node);

	node.dwAssetUID = 1004;
	node.dwPlayTime = 30;
	node.dwWeight = 4;
	filler.push_back(node);

	node.dwAssetUID = 1005;
	node.dwPlayTime = 30;
	node.dwWeight = 1;
	filler.push_back(node);

	node.dwAssetUID = 1006;
	node.dwPlayTime = 30;
	node.dwWeight = 9;
	filler.push_back(node);

	


	ASSETS playlist;
	resetPlayList(&playlist);


//////////////////////////////////////////////////////////////////////////

	AutoFiller autoFiller;
	autoFiller.setFillers(&filler);
	PASSETLIST autoFillRList;
	autoFiller.fill(&playlist,&autoFillRList,810,FILLTYPE_RANDOM);
	DWORD i =0;
	printf("Autofiller result ------Random\n");
	for(i = 0; i < autoFillRList.size(); i++)
	{
		
		printf("\t%d\t%d\n",autoFillRList[i]->dwAssetUID,autoFillRList[i]->dwWeight);
	}
	
	resetPlayList(&playlist);
	PASSETLIST autoFillSList;
	autoFiller.fill(&playlist,&autoFillSList,690,FILLTYPE_SERIAL);
	printf("Autofiller result ------Serial\n");
	for(i = 0; i < autoFillSList.size(); i++)
	{
		
		printf("\t%d\t%d\n",autoFillSList[i]->dwAssetUID,autoFillSList[i]->dwWeight);
	}

	resetPlayList(&playlist);
	BarkerFiller barkerFiller;
	barkerFiller.setFillers(&filler);
	PASSETLIST BarkerList;
	barkerFiller.fill(&playlist,&BarkerList,1020);

	printf("Barkfiller result\n");
	for(i = 0; i < BarkerList.size(); i++)
	{
		ASSET* pAsset = BarkerList.at(i);
		printf("\t%d\t%d\n",pAsset->dwAssetUID,pAsset->dwWeight);
	}
	return 0;
}
