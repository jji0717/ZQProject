

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
// Ident : $Id: AeBuilderTest.cpp
// Author: Kaliven Lee
// Desc  : Test for AeBuilder
// Revision History: 
// ---------------------------------------------------------------------------
// $Revision: 1 $
// $Log: /ZQProjs/ScheduledTV_old/AeBuilder/AeBuilderTest.cpp $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 9     04-10-08 11:19 Kaliven.lee
// move to idsSession
// 
// 8     04-09-29 9:57 Kaliven.lee
// avoid repeat push the same assetelement to list
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
// 4     04-09-20 18:15 Kaliven.lee
// 
// 3     04-09-16 17:18 Kaliven.lee



#include "IdsSession.h"

int main(int argc, char* argv[])
{
	IdsSession builder;
	
	if(!builder.initialize(L"192.168.12.12",L"ITV"))
		return -1;	
	
	

	DWORD 	dwAssetId  = 0x0560000B;
	
	ASSETELEMENTS ElementList;
	builder.getElementList(dwAssetId,&ElementList);
	
	DWORD dwElementNum = ElementList.size();
	if(dwElementNum == 0 )
	{
		return -1;
	}
	for(DWORD i = 0; i < dwElementNum; i++)
	{
		wprintf(L"element %lu ID: %X BitRate:%ld PlayTime:%d s \n",
				i,
				ElementList.at(i).dwAEUID,
				ElementList.at(i).dwBitRate,
				ElementList.at(i).dwPlayTime
				);
	}
	//////////////////////////////////////////////////////////////////////////
	// build Asset list  you wana get the detail information
	ASSETS AssetList;
	ASSET Asset;
	Asset.dwAssetUID  = 0x0560000B;
	AssetList.push_back(Asset);
	AssetList.push_back(Asset);

	builder.getElementList(&AssetList);
	DWORD elementNum ;
	ASSETELEMENT Element;
	for(i= 0; i < AssetList.size() ; i ++)
	{
		wprintf(L"Asset ID: 0x%X \n",AssetList.at(i).dwAssetUID);
		elementNum = AssetList.at(i).AssetElements.size();		
		for(DWORD j =0; j < elementNum; j ++)
		{
			Element = AssetList.at(i).AssetElements.at(j);
			wprintf(L"element %lu ID: 0x%X BitRate:%ld PlayTime:%d s \n",
				j,
				Element.dwAEUID,
				Element.dwBitRate,
				Element.dwPlayTime);
		}
	}
	
	builder.unInitialize();
	return 0;
}
