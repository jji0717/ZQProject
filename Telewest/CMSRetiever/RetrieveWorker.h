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
//
// Ident : $Id: RetrieveWorker.h,v 1.8 2004/08/09 10:08:56 wli Exp $
// Branch: $Name:  $
// Author: kaliven lee
// Desc  : define worker service class
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/CMSRetiever/RetrieveWorker.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 2     05-04-15 11:15 Kaliven.lee
// 
// 1     05-03-10 11:12 Kaliven.lee
// file create
// RetrieveWorker.h: interface for the RetrieveWorker class.
//
//////////////////////////////////////////////////////////////////////
#pragma once
#include "../Common/NativeThread.h"
#include "xmlpreference.h"
#include <string>
#define MAXFILELEN		256
using ZQ::common::XMLPrefDoc;

class RetrieveWorker : public ZQ::common::NativeThread  
{
public:
	RetrieveWorker(char* fileName,DWORD dwTimeOut);
	virtual ~RetrieveWorker();
public:
	int run(void);	
	void stopWorker(void);
	void setLoadFileName(char* fileName);
public:
	void readLoadFile(void);
	bool readInstance(ZQ::common::IPreference* parent);
	bool readHead(ZQ::common::IPreference* parent,char* sVer,char* sTime,char* sInterval);
	bool readCMGroup(ZQ::common::IPreference* parent,char* sAppType,char* sNodeGroup,char* sPGLevel);
private:
	std::string m_sFileName1;
	std::string m_sFileName2;
	CDatabase m_LocalDB;
	ZQ::common::XMLPrefDoc* m_pXMLDoc;
	HANDLE m_hEvent;
	DWORD m_dwTimeOut;
	bool m_bQuit; 
	ZQ::common::ComInitializer* m_pComInit;
};


