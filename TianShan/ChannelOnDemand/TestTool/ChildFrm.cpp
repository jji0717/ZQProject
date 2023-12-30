// ChildFrm.cpp : implementation of the CChildFrame class
//

#include "stdafx.h"
#include "CodMan.h"

#include "ChildFrm.h"

#include "ChannelTV.h"
#include "PropertyView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CChildFrame)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChildFrame construction/destruction

CChildFrame::CChildFrame()
{
	// TODO: add member initialization code here
	
}

CChildFrame::~CChildFrame()
{
}

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	if( !CMDIChildWnd::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// override OnCreateClient
BOOL CChildFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	m_SplitWnd.CreateStatic(this, 1, 2);

	if (!m_SplitWnd.CreateView(0, 0, RUNTIME_CLASS(CChannelTV), CSize(100, 100), pContext))
	{
		AfxMessageBox("创建TreeView子视图失败");
		return FALSE;
	}

	if (!m_SplitWnd.CreateView(0, 1, RUNTIME_CLASS(CPropertyView), CSize(0, 0), pContext))
	{
		AfxMessageBox("创建ListView子视图失败");
		return FALSE;
	}

	return TRUE;
}

CChannelTV* CChildFrame::GetChannelView()
{
	CChannelTV* pTreeView = NULL;
	pTreeView = (CChannelTV*)m_SplitWnd.GetDlgItem(m_SplitWnd.IdFromRowCol(0, 0));
	ASSERT(pTreeView != NULL && pTreeView->IsKindOf(RUNTIME_CLASS(CChannelTV)));
	return pTreeView;
}

CPropertyView* CChildFrame::GetPropertyView()
{
	CPropertyView* pPropView = NULL;
	pPropView = (CPropertyView*)m_SplitWnd.GetDlgItem(m_SplitWnd.IdFromRowCol(0, 1));
	ASSERT(pPropView != NULL && pPropView->IsKindOf(RUNTIME_CLASS(CPropertyView)));
	return pPropView;
}

/////////////////////////////////////////////////////////////////////////////
// CChildFrame diagnostics

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CChildFrame message handlers
