// ===========================================================================
// Copyright (c) 2005 by
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
// Name  : Communication.cpp
// Author : Ken Qian(ken.qian@i-zq.com  Xuezheng Qian)
// Date  : 2005-9-19
// Desc  : This class is an abstract class to provider the interface between 
//         VDCPService and Automation. The inherited class only need to implement
//         virtual functions.
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/SMS/ModemGateway/Communication.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 1     06-11-14 18:23 Shuai.chen
// 
// 1     06-01-06 10:40 Shuai.chen
// 
// 1     05-10-13 17:11 Ken.qian
// ===========================================================================

#include "stdafx.h"
#include "Communication.h"
#include <stdio.h> 
#include <stdarg.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCommunication::CCommunication()
{

}

CCommunication::~CCommunication()
{

}

void CCommunication::GetLastError(int* errCode, char* errDes, int nSize)
{
	*errCode = m_nLastError;

	if(errDes != NULL && nSize != 0)
	{
		strncpy(errDes, m_chLastError, nSize);
	}
}

void CCommunication::SetLastError(int nErrCode, const char* lpszFmt, ...)
{
	m_nLastError = nErrCode;
	
    va_list	marker;

	// Initialize access to variable arglist
	va_start(marker, lpszFmt);

	// Expand message
	_vsnprintf(m_chLastError, 1023, lpszFmt, marker);
}

void CCommunication::GetBlockHandleForReading(HANDLE* pHandles, int* nCount)
{
	nCount = 0;
}

void CCommunication::GetBlockHandleForWriting(HANDLE* pHandles, int* nCount)
{
	nCount = 0;
}

bool CCommunication::CheckOverlappedResult(DWORD* nReceivedBytes)
{
	return true;
}
