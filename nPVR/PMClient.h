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
// Desc  : define the soap interface of AssetGear to query ZQ Program Manager
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/nPVR/PMClient.h 1     10-11-12 16:01 Admin $
// $Log: /ZQProjs/nPVR/PMClient.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// ============================================================================================

#ifndef __PMClient_H__
#define __PMClient_H__

#ifdef PMCLIENT_EXPORTS
#define	PMClientAPI  __declspec(dllexport)
#else
#define	PMClientAPI  __declspec(dllimport)
#endif // PMCLIENT_EXPORTS
#pragma warning (disable:4251)

#define METADATA_VALUE_MAXLEN 12 * 1024

#include <vector>

class PMClientAPI PMClient;

class PMClient
{
public:

	typedef struct _MetaData
	{
		char		name[40];
		int         type;
		char		value[METADATA_VALUE_MAXLEN];
	} MetaData;

	typedef std::vector <MetaData> MetaDataCollection;

	PMClient(const char* pmUrl = NULL);
	~PMClient();
	
	virtual const int queryMetaData(int scheduleID, const char* programId, int assetID, MetaData* pMetaData, const int maxSize);

protected:
	std::string _pmUrl;
};

#endif // __PMClient_H__
