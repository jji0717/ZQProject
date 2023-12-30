// NPVRSMSUIDoc.h : interface of the CNPVRSMSUIDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_NPVRSMSUIDOC_H__F876C110_D3D7_4C19_82F5_CDD3892AA69D__INCLUDED_)
#define AFX_NPVRSMSUIDOC_H__F876C110_D3D7_4C19_82F5_CDD3892AA69D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CNPVRSMSUIDoc : public CDocument
{
protected: // create from serialization only
	CNPVRSMSUIDoc();
	DECLARE_DYNCREATE(CNPVRSMSUIDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNPVRSMSUIDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CNPVRSMSUIDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CNPVRSMSUIDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NPVRSMSUIDOC_H__F876C110_D3D7_4C19_82F5_CDD3892AA69D__INCLUDED_)
