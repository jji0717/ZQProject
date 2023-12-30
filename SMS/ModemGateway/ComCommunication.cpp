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
// Name  : ComCommunication.cpp
// Author : Ken Qian(ken.qian@i-zq.com  Xuezheng Qian)
// Date  : 2005-9-19
// Desc  : This class is inherited from CCommunication to implement the RS422 interface
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/SMS/ModemGateway/ComCommunication.cpp $
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
#include <afx.h>
#include "ComCommunication.h"

#define COMPORT_BUFF_SIZE	1024

#define FC_DTRDSR       0x01
#define FC_RTSCTS       0x02
#define FC_XONXOFF      0x04

// ascii definitions

#define ASCII_BEL       0x07
#define ASCII_BS        0x08
#define ASCII_LF        0x0A
#define ASCII_CR        0x0D
#define ASCII_XON       0x11
#define ASCII_XOFF      0x13

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CComCommunication::CComCommunication() : CCommunication()
{

}

CComCommunication::CComCommunication(int nComport)
{
	m_nComport = nComport;

	// clear the Read/Write OVERLAPPED structures
	memset (&m_ovRead, 0x00, sizeof (OVERLAPPED));
	memset (&m_ovWrite, 0x00, sizeof (OVERLAPPED));
}

CComCommunication::~CComCommunication()
{
}

bool CComCommunication::Initialize(void)
{
	wchar_t wszComPort[24];
	swprintf(wszComPort, L"\\\\.\\COM%d", m_nComport);

	DWORD dwMode = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED;

	m_hComm = CreateFile( wszComPort,
			GENERIC_READ | GENERIC_WRITE,
			0,				
			NULL,
			OPEN_EXISTING,
			dwMode,	
			NULL);			

	if(m_hComm == INVALID_HANDLE_VALUE) 
	{
		SetLastError(::GetLastError(), "Failed to open COM%d", m_nComport);
		return false;		
	}
	
	if(SetupComm(m_hComm, COMPORT_BUFF_SIZE, COMPORT_BUFF_SIZE) == FALSE)
	{
		CloseHandle(m_hComm);
		SetLastError(::GetLastError(), "Failed to Setup COM%d", m_nComport);
		return false;
	}

	COMMTIMEOUTS  CommTimeOuts;
	// can NOT be 0, cosz if the actual bytes in IO input buffer is less than ToRead bytes(in ReadFile)
	// the read overlap event will not be signal and the thread will be halt
	CommTimeOuts.ReadIntervalTimeout = 200;	

	// following two MUST be 0. That make only data arrived at IO input buffer, 
	// the read overlap event will be signal.
	CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
	CommTimeOuts.ReadTotalTimeoutConstant = 0;

	CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
	CommTimeOuts.WriteTotalTimeoutConstant = 200;
	SetCommTimeouts(m_hComm, &CommTimeOuts) ;

	// get dcb 
	DCB dcb;

	GetCommState(m_hComm, &dcb); 

	dcb.DCBlength = sizeof(DCB);
	dcb.BaudRate = CBR_9600;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;//ODDPARITY; 
	dcb.StopBits = ONESTOPBIT;

//	dcb.fBinary = TRUE ;
//	dcb.fParity = FALSE ;	
//	dcb.fOutxDsrFlow = dcb.fOutxCtsFlow = 0;
//	dcb.fInX = dcb.fOutX = 0;
//
//	dcb.fDtrControl = DTR_CONTROL_DISABLE;  
//	dcb.fRtsControl = RTS_CONTROL_DISABLE;  
	// set dcb 
	if(SetCommState(m_hComm, &dcb) == FALSE)
	{
		CloseHandle(m_hComm);
		SetLastError(::GetLastError(), "Failed to SetCommState COM%d", m_nComport);
		return false;
	}

	DWORD           CommErrs;
	COMSTAT         CommStat;
	ClearCommError(m_hComm, &CommErrs, &CommStat);

	if (PurgeComm( m_hComm, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR) == FALSE)
	{
		CloseHandle(m_hComm);
		SetLastError(::GetLastError(), "Failed to PurgeComm COM%d", m_nComport);
		return false;
	}

	m_ovRead.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (m_ovRead.hEvent == NULL)
	{
		CloseHandle(m_hComm);
		SetLastError(::GetLastError(), "Failed to CreateEvent for Read OVERLAP");
		return false;
	}
	m_ovWrite.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (m_ovWrite.hEvent == NULL)
	{
		CloseHandle(m_hComm);
		SetLastError(::GetLastError(), "Failed to CreateEvent for Write OVERLAP");
		return false;
	}

	return true;
}

bool CComCommunication::Uninitialize(void)
{
	PurgeComm(m_hComm, PURGE_TXABORT | PURGE_RXABORT |
		PURGE_TXCLEAR | PURGE_RXCLEAR) ;
	
    if(m_hComm != INVALID_HANDLE_VALUE && m_hComm != NULL)
		CloseHandle(m_hComm);

	if(m_ovRead.hEvent != INVALID_HANDLE_VALUE && m_ovRead.hEvent != NULL)
	{
		CloseHandle(m_ovRead.hEvent);
		m_ovRead.hEvent = NULL;
	}

	if(m_ovWrite.hEvent != INVALID_HANDLE_VALUE && m_ovWrite.hEvent != NULL)
	{
		CloseHandle(m_ovWrite.hEvent);
		m_ovWrite.hEvent = NULL;
	}

	return true;
}

void CComCommunication::GetBlockHandleForReading(HANDLE* pHandles, int* nCount)
{
	pHandles[0] = m_ovRead.hEvent;
	*nCount = 1;
	return;
}

void CComCommunication::GetBlockHandleForWriting(HANDLE* pHandles, int* nCount)
{
	nCount = 0;
}

bool CComCommunication::ReadData(DWORD dwBytesToRead, unsigned char* data, DWORD* dwCount)
{
	if(data == NULL)
		return false;

	ASSERT(m_hComm != INVALID_HANDLE_VALUE);
	ASSERT(m_hComm != NULL);

	bool bResult = false;
	int iStatus;
	iStatus = ReadFile(m_hComm, data, dwBytesToRead, dwCount, &m_ovRead);

	if (iStatus == false)
	{
		int nLastError = ::GetLastError();
		// ERROR_IO_PENDING is expected for overlapped IO
		int temp = nLastError;
		if (nLastError != ERROR_IO_PENDING)
		{
			SetLastError(nLastError, "Failed to ReadData() from COM%d", m_nComport);
		    return false;
		}
	}
	return true;
}

bool CComCommunication::WriteData(unsigned char* data, DWORD dwCount)
{
	ASSERT(m_hComm != INVALID_HANDLE_VALUE);
	ASSERT(m_hComm != NULL);

	DWORD dwBytesWritten;
	BOOL bStatus;

	bStatus = PurgeComm(m_hComm, PURGE_TXCLEAR) ;
	if (!bStatus)
	{
		SetLastError(::GetLastError(), "Error calling PurgeComm() before WriteData() on COM%d", m_nComport);
		return false;
	}

	bStatus = WriteFile(m_hComm, data, dwCount, &dwBytesWritten, &m_ovWrite); 

	if (!bStatus)
	{
		int nLastError = ::GetLastError();
		if (nLastError == ERROR_IO_PENDING)
		{
			//  give it time for the write to complete
			DWORD dwStatus ;
			for (int i=0; i < 10; i++ )
			{
				dwStatus = WaitForSingleObject(m_ovWrite.hEvent, 100) ;
				if (dwStatus == WAIT_OBJECT_0)
					break;
			}
			if ( i>= 9)   // did it take 10 or more iterations
			{
				nLastError = ::GetLastError();
				SetLastError(nLastError, "Exhausted iterations waiting for successful WriteFile() on COM%d", m_nComport);
				return false;
			}
		}
		else   // error other than IO_PENDING
		{
			SetLastError(nLastError, "Error calling WriteData() on COM%d", m_nComport);
			return false;
		}
	}
	return true;
}

bool CComCommunication::CheckOverlappedResult(DWORD* nReceivedBytes)
{
	if (GetOverlappedResult(m_hComm, 
						   &m_ovRead, 
						   nReceivedBytes, 
						   FALSE) == FALSE)
	{
		int nLastError = ::GetLastError();
		// ERROR_IO_PENDING is expected for overlapped IO
		if (nLastError != ERROR_IO_PENDING)
		{
			SetLastError(::GetLastError(), "Error returned from GetOverlappedResult() On COM%d", m_nComport);
			return false;
		}
	}

	return true;
}

bool CComCommunication::ClearRevBuffer()
{
	return PurgeComm(m_hComm, PURGE_RXCLEAR) ;
}


///////// just for test //////////////////////////////////////
bool CComCommunication::ReadComm(void* pData, DWORD dwLength)
{
	if(pData == NULL)
		return false;

	ASSERT(m_hComm != INVALID_HANDLE_VALUE);
	ASSERT(m_hComm != NULL);

	bool bResult = false;
	int iStatus;
	DWORD dwNumRead;
	
	iStatus = ReadFile(m_hComm, pData, dwLength, &dwNumRead, &m_ovRead);

	if (iStatus == false)
	{
		int nLastError = ::GetLastError();
		// ERROR_IO_PENDING is expected for overlapped IO
		int temp = nLastError;
		if (nLastError != ERROR_IO_PENDING)
		{
			SetLastError(nLastError, "Failed to ReadData() from COM%d", m_nComport);
		    return false;
		}
	}
	return true;
}

bool CComCommunication::WriteComm(void* pData, DWORD dwLength)
{
	DWORD dwNumWrite=0;	

	ASSERT(m_hComm != INVALID_HANDLE_VALUE);
	ASSERT(m_hComm != NULL);

	DWORD dwBytesWritten;
	BOOL bStatus;

	bStatus = PurgeComm(m_hComm, PURGE_TXCLEAR) ;
	if (!bStatus)
	{
		SetLastError(::GetLastError(), "Error calling PurgeComm() before WriteData() on COM%d", m_nComport);
		return false;
	}

	bStatus = WriteFile(m_hComm, (unsigned char*)pData, dwLength, &dwBytesWritten, &m_ovWrite); 

	if (!bStatus)
	{
		int nLastError = ::GetLastError();
		if (nLastError == ERROR_IO_PENDING)
		{
			//  give it time for the write to complete
			DWORD dwStatus ;
			for (int i=0; i < 10; i++ )
			{
				dwStatus = WaitForSingleObject(m_ovWrite.hEvent, 15) ;
				if (dwStatus == WAIT_OBJECT_0)
					break;
			}
			if ( i>= 9)   // did it take 10 or more iterations
			{
				nLastError = ::GetLastError();
				SetLastError(nLastError, "Exhausted iterations waiting for successful WriteFile() on COM%d", m_nComport);
				return false;
			}
		}
		else   // error other than IO_PENDING
		{
			SetLastError(nLastError, "Error calling WriteData() on COM%d", m_nComport);
			return false;
		}
	}
	return true;
}
