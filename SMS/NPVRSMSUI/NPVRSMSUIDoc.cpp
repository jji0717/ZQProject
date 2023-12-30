// NPVRSMSUIDoc.cpp : implementation of the CNPVRSMSUIDoc class
//

#include "stdafx.h"
#include "NPVRSMSUI.h"

#include "NPVRSMSUIDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNPVRSMSUIDoc

IMPLEMENT_DYNCREATE(CNPVRSMSUIDoc, CDocument)

BEGIN_MESSAGE_MAP(CNPVRSMSUIDoc, CDocument)
	//{{AFX_MSG_MAP(CNPVRSMSUIDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNPVRSMSUIDoc construction/destruction

CNPVRSMSUIDoc::CNPVRSMSUIDoc()
{
	// TODO: add one-time construction code here

}

CNPVRSMSUIDoc::~CNPVRSMSUIDoc()
{
}

BOOL CNPVRSMSUIDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CNPVRSMSUIDoc serialization

void CNPVRSMSUIDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CNPVRSMSUIDoc diagnostics

#ifdef _DEBUG
void CNPVRSMSUIDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CNPVRSMSUIDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CNPVRSMSUIDoc commands
