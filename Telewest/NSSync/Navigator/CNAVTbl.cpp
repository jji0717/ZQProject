// CNAVTbl.h : Implementation of the CNAVTbl class



// CNAVTbl implementation

// code generated on 2004Äê12ÔÂ14ÈÕ, 17:50

#include "stdafx.h"
#include "CNAVTbl.h"
IMPLEMENT_DYNAMIC(CNAVTbl, CRecordset)

CNAVTbl::CNAVTbl(CDatabase* pdb)
	: CRecordset(pdb)
{
	m_Hierarchy_UID = "";
	m_Local_entry_UID = "";
	m_Entry_name = "";
	m_Entry_type = 0.0;
	m_Sequence = 0.0;
	m_Parent_HUID = "";
	m_Parent_entry_type = 0.0;
	m_Product_offering_UID = "";
	m_Is_manual = 0.0;
	m_Is_invisible = 0.0;
	m_entry_count = 0.0;
	m_Target_HUID = "";
	m_Target_entry_type = 0.0;
	m_nFields = 13;
	m_nDefaultType = dynaset;
}
#error Security Issue: The connection string may contain a password
// The connection string below may contain plain text passwords and/or
// other sensitive information. Please remove the #error after reviewing
// the connection string for any security related issues. You may want to
// store the password in some other form or use a different user authentication.
CString CNAVTbl::GetDefaultConnect()
{
	return _T("DSN=LAM;Description=local AM DB;UID=am;PWD=am;APP=Microsoft\x00ae Visual Studio .NET;WSID=BZHAO;DATABASE=am");
}

CString CNAVTbl::GetDefaultSQL()
{
	return _T("[dbo].[NAV_Hierarchy]");
}

void CNAVTbl::DoFieldExchange(CFieldExchange* pFX)
{
	pFX->SetFieldType(CFieldExchange::outputColumn);
// Macros such as RFX_Text() and RFX_Int() are dependent on the
// type of the member variable, not the type of the field in the database.
// ODBC will try to automatically convert the column value to the requested type
	RFX_Text(pFX, _T("[Hierarchy_UID]"), m_Hierarchy_UID);
	RFX_Text(pFX, _T("[Local_entry_UID]"), m_Local_entry_UID);
	RFX_Text(pFX, _T("[Entry_name]"), m_Entry_name);
	RFX_Double(pFX, _T("[Entry_type]"), m_Entry_type);
	RFX_Double(pFX, _T("[Sequence]"), m_Sequence);
	RFX_Text(pFX, _T("[Parent_HUID]"), m_Parent_HUID);
	RFX_Double(pFX, _T("[Parent_entry_type]"), m_Parent_entry_type);
	RFX_Text(pFX, _T("[Product_offering_UID]"), m_Product_offering_UID);
	RFX_Double(pFX, _T("[Is_manual]"), m_Is_manual);
	RFX_Double(pFX, _T("[Is_invisible]"), m_Is_invisible);
	RFX_Double(pFX, _T("[entry_count]"), m_entry_count);
	RFX_Text(pFX, _T("[Target_HUID]"), m_Target_HUID);
	RFX_Double(pFX, _T("[Target_entry_type]"), m_Target_entry_type);

}
/////////////////////////////////////////////////////////////////////////////
// CNAVTbl diagnostics

#ifdef _DEBUG
void CNAVTbl::AssertValid() const
{
	CRecordset::AssertValid();
}

void CNAVTbl::Dump(CDumpContext& dc) const
{
	CRecordset::Dump(dc);
}
#endif //_DEBUG


