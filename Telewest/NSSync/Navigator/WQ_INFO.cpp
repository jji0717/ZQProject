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
// Desc  : implementation of the CWQ_INFO class
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/NSSync/Navigator/WQ_INFO.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 4     08-12-24 14:40 Ken.qian
// fix the wq fetch sql
// 
// 3     08-09-04 2:02 Ken.qian
// new codes for new logic
// 
// 1     05-03-25 12:48 Bernie.zhao
// ===========================================================================
#include "stdafx.h"
#include "WQ_INFO.h"
IMPLEMENT_DYNAMIC(CWQ_INFO, CRecordset)

CWQ_INFO::CWQ_INFO(CDatabase* pdb, CString connectStr, int mode, DWORD waitTime)
	: CRecordset(pdb)
{
	m_QueueUID = "";
	m_Source_type = 0;
	m_local_entry_UID = "";
	m_Parent_HUID = "";
	m_Entry_Name = "";
	m_Entry_type = 0;
	m_Operation_type = 0;
	m_Operation_time;
	m_MD_name = "";
	
	m_nFields = 9;
	m_nDefaultType = dynaset;

	_connectStr = connectStr;

	_waitTime = waitTime;

	switch(mode)
	{
	// add by KenQ to support the new Navigation asset/bundle build
	// condition: If data from DBSync(SourceType=0), Asset(entryType=1)/bundle(entryType=11)(actually only two types), 
	//            Update(operation_type=6) which need to be hold for specified time
	// 
	case MODE_SQL_WITH_WAITTIME:
		{
			time_t now;
			time(&now);
			CTime updateTime = CTime(now) - CTimeSpan(_waitTime);
			CString strUpdateTime = updateTime.Format("%Y-%m-%d %H:%M:%S");
			
			_defaultSQLString.Format( _T(" SELECT QueueUID, Source_type, local_entry_UID, Parent_HUID, Entry_Name, Entry_type, Operation_type, Operation_time, MD_name FROM WQ_INFO ")
									  _T(" WHERE (Source_Type=0 AND Operation_type=6 AND Operation_time<=%s) " )
									  _T(" OR    (Source_Type=0 AND Operation_type<>6 ) ")
									  _T(" OR     Source_Type <> 0 ORDER BY QueueUID"), (LPCTSTR)strUpdateTime);
		}
		break;
	case MODE_SQL_DEFAULT:
	default:
		_defaultSQLString = _T("[WQ_INFO]");
		break;
	}
}
//#error Security Issue: The connection string may contain a password
// The connection string below may contain plain text passwords and/or
// other sensitive information. Please remove the #error after reviewing
// the connection string for any security related issues. You may want to
// store the password in some other form or use a different user authentication.
CString CWQ_INFO::GetDefaultConnect()
{
	return _connectStr;
}

CString CWQ_INFO::GetDefaultSQL()
{
	return _defaultSQLString;
}

void CWQ_INFO::SetTimeout(DWORD dwTimeout)
{
	m_pDatabase->SetQueryTimeout(dwTimeout);
}

void CWQ_INFO::DoFieldExchange(CFieldExchange* pFX)
{
	pFX->SetFieldType(CFieldExchange::outputColumn);
// Macros such as RFX_Text() and RFX_Int() are dependent on the
// type of the member variable, not the type of the field in the database.
// ODBC will try to automatically convert the column value to the requested type
	RFX_Text(pFX, _T("[QueueUID]"), m_QueueUID);
	RFX_Int(pFX, _T("[Source_type]"), m_Source_type);
	RFX_Text(pFX, _T("[local_entry_UID]"), m_local_entry_UID);
	RFX_Text(pFX, _T("[Parent_HUID]"), m_Parent_HUID);
	RFX_Text(pFX, _T("[Entry_Name]"), m_Entry_Name);
	RFX_Int(pFX, _T("[Entry_type]"), m_Entry_type);
	RFX_Int(pFX, _T("[Operation_type]"), m_Operation_type);
	RFX_Date(pFX, _T("[Operation_time]"), m_Operation_time);
	RFX_Text(pFX, _T("[MD_name]"), m_MD_name);

}
/////////////////////////////////////////////////////////////////////////////
// CWQ_INFO diagnostics

#ifdef _DEBUG
void CWQ_INFO::AssertValid() const
{
	CRecordset::AssertValid();
}

void CWQ_INFO::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG


