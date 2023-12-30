// CNAVTbl.h : Declaration of the CNAVTbl

#pragma once

// code generated on 2004Äê12ÔÂ14ÈÕ, 17:50

class CNAVTbl : public CRecordset
{
public:
	CNAVTbl(CDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(CNAVTbl)

// Field/Param Data

// The string types below (if present) reflect the actual data type of the
// database field - CStringA for ANSI datatypes and CStringW for Unicode
// datatypes. This is to prevent the ODBC driver from performing potentially
// unnecessary conversions.  If you wish, you may change these members to
// CString types and the ODBC driver will perform all necessary conversions.
// (Note: You must use an ODBC driver version that is version 3.5 or greater
// to support both Unicode and these conversions).

	CString	m_Hierarchy_UID;
	CString	m_Local_entry_UID;
	CString	m_Entry_name;
	double	m_Entry_type;
	double	m_Sequence;
	CString	m_Parent_HUID;
	double	m_Parent_entry_type;
	CString	m_Product_offering_UID;
	double	m_Is_manual;
	double	m_Is_invisible;
	double	m_entry_count;
	CString	m_Target_HUID;
	double	m_Target_entry_type;

// Overrides
	// Wizard generated virtual function overrides
	public:
	virtual CString GetDefaultConnect();	// Default connection string

	virtual CString GetDefaultSQL(); 	// default SQL for Recordset
	virtual void DoFieldExchange(CFieldExchange* pFX);	// RFX support

// Implementation
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

};


