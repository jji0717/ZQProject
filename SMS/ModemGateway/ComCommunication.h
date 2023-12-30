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
// Name  : ComCommunication.h
// Author : Ken Qian(ken.qian@i-zq.com  Xuezheng Qian)
// Date  : 2005-9-19
// Desc  : This class is inherited from CCommunication to implement the RS422 interface
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/SMS/ModemGateway/ComCommunication.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 1     06-11-14 18:23 Shuai.chen
// ===========================================================================

#if !defined(COMCOMUNICATION_H)
#define COMCOMUNICATION_H

#include "Communication.h"

class CCommunication;

class CComCommunication : public CCommunication
{
public:
	CComCommunication();
	CComCommunication(int nComport);
	virtual ~CComCommunication();

public:
	/// Open COM port, and initialize the COM communication with proper parameters
	bool Initialize(void);
	/// Close COM port
	bool Uninitialize(void);

	/// Read Data from COM port by OVERLAP way
	bool ReadData(DWORD dwBytesToRead, unsigned char* data, DWORD* dwCount);
	/// Send data to COM port by OVERLAP
	bool WriteData(unsigned char* data, DWORD dwCount);

	/// Get the handle of reading data for blocking, it overlap's event
	void GetBlockHandleForReading(HANDLE* pHandles, int* nCount);
	/// Get the handle of sending data for blocking, it is not implemented
	void GetBlockHandleForWriting(HANDLE* pHandles, int* nCount);

	/// Get the data from overlap port and checking the received data.
	virtual bool CheckOverlappedResult(DWORD* nReceivedBytes);

	/// To clear the receiving buffer when the data met problem
	virtual bool ClearRevBuffer();

	// just for test
	bool ReadComm(void* pData, DWORD dwLength);
	bool WriteComm(void* pData, DWORD dwLength);
	////////////////////////////////////////

private:
	int          m_nComport;		/// COM port

	HANDLE       m_hComm;			/// Handle of COM PORT
		
	OVERLAPPED   m_ovRead;			/// Overlap for reading data from COM Port
	OVERLAPPED   m_ovWrite;			/// Overlap for writing data to COM Port
};

#endif // !defined(COMCOMUNICATION_H)
