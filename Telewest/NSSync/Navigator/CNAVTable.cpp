// ===========================================================================
// Copyright (c) 2004 by
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
// Name  : navWorkerProvider.cpp
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2004-12-13
// Desc  : implementation of the CNAVTable class
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/NSSync/Navigator/CNAVTable.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 3     05-12-20 11:42 Bernie.zhao
// 
// 1     05-03-25 12:47 Bernie.zhao
// ===========================================================================

#include "stdafx.h"
#include "CNAVTable.h"
IMPLEMENT_DYNAMIC(CNAVTable, CRecordset)

CNAVTable::CNAVTable(CDatabase* pdb, CString connectStr)
	: CRecordset(pdb)
{
	m_nFields = 0;
	
	m_nDefaultType = dynaset;
	//m_nDefaultType = snapshot;	// dynaset will cause deadlock? not sure!

	_connectStr = connectStr;
}

CNAVTable::~CNAVTable()
{
	if(IsOpen())			// Added 2005.12.16.  Las Vegas Airport.  Bernie.Zhao
	{						// Close record set in destructor, just for the sake of connection or resource
        Close();			// leak when exception is caught during db operations.
	}
}

//#error Security Issue: The connection string may contain a password
// The connection string below may contain plain text passwords and/or
// other sensitive information. Please remove the #error after reviewing
// the connection string for any security related issues. You may want to
// store the password in some other form or use a different user authentication.
CString CNAVTable::GetDefaultConnect()
{
	return _connectStr;
}

CString CNAVTable::GetDefaultSQL()
{
	return _T("");
}

void CNAVTable::DoFieldExchange(CFieldExchange* pFX)
{
	pFX->SetFieldType(CFieldExchange::outputColumn);
// Macros such as RFX_Text() and RFX_Int() are dependent on the
// type of the member variable, not the type of the field in the database.
// ODBC will try to automatically convert the column value to the requested type

}
/////////////////////////////////////////////////////////////////////////////
// CNAVTable diagnostics

#ifdef _DEBUG
void CNAVTable::AssertValid() const
{
	CRecordset::AssertValid();
}

void CNAVTable::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG


