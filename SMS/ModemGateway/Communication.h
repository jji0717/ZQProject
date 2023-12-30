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
// Name  : Communication.h
// Author : Ken Qian(ken.qian@i-zq.com  Xuezheng Qian)
// Date  : 2005-9-19
// Desc  : This class is an abstract class to provider the interface between 
//         VDCPService and Automation. The inherited class only need to implement
//         virtual functions.
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/SMS/ModemGateway/Communication.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 1     06-11-14 18:23 Shuai.chen
// ===========================================================================

#if !defined(COMUNICATION_H)
#define COMUNICATION_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <wtypes.h>

class CCommunication  
{
public:
	CCommunication();
	virtual ~CCommunication();

public:
	/// initialize the communication between VDCPService and Automation
	virtual bool Initialize(void) = 0;
	/// uninitialize the communication between VDCPService and Automation
	virtual bool Uninitialize(void) = 0;

	/// read data through current communication way from Automation
	virtual bool ReadData(DWORD dwBytesToRead, unsigned char* data, DWORD* dwCount) = 0;
	/// send data through current communication way to Automation
	virtual bool WriteData(unsigned char* data, DWORD dwCount) = 0;

	/// Get the handle of reading data for blocking, 
	/// if no blocking, just set *nCount = 0
	virtual void GetBlockHandleForReading(HANDLE* pHandles, int* nCount);
	/// Get the handle of sending data for blocking, 
	/// if no blocking, just set *nCount = 0
	virtual void GetBlockHandleForWriting(HANDLE* pHandles, int* nCount);

	/// only for overlapped IO communication operation, 
	/// if NO overlap, just return true without doing anything
	virtual bool CheckOverlappedResult(DWORD* nReceivedBytes);

	/// To clear the receiving buffer when the data met problem
	virtual bool ClearRevBuffer() = 0;

	/// get last error with description and code to descript the detail information.
	void GetLastError(int* errCode, char* errDes, int nSize);
protected:
	int		m_nLastError;
	char    m_chLastError[1024];

	void SetLastError(int nErrCode, const char* lpszFmt, ...);
};

#endif // !defined(COMUNICATION_H)
