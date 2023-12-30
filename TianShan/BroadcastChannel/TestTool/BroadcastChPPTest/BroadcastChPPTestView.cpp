// CodManView.cpp : implementation of the BroadcastChPPTestView class
//

#include "stdafx.h"
#include "BroadcastChPPTest.h"

#include "BroadcastChPPTestDoc.h"
#include "BroadcastChPPTestView.h"
#include "ChildFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// BroadcastChPPTestView

IMPLEMENT_DYNCREATE(BroadcastChPPTestView, CView)

BEGIN_MESSAGE_MAP(BroadcastChPPTestView, CView)
	//{{AFX_MSG_MAP(BroadcastChPPTestView)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// BroadcastChPPTestView construction/destruction

BroadcastChPPTestView::BroadcastChPPTestView()
{
	// TODO: add construction code here

}

BroadcastChPPTestView::~BroadcastChPPTestView()
{
}

BOOL BroadcastChPPTestView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// BroadcastChPPTestView drawing

void BroadcastChPPTestView::OnDraw(CDC* pDC)
{
	BroadcastChPPTestDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// BroadcastChPPTestView printing

BOOL BroadcastChPPTestView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void BroadcastChPPTestView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void BroadcastChPPTestView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// BroadcastChPPTestView diagnostics

#ifdef _DEBUG
void BroadcastChPPTestView::AssertValid() const
{
	CView::AssertValid();
}

void BroadcastChPPTestView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

BroadcastChPPTestDoc* BroadcastChPPTestView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(BroadcastChPPTestDoc)));
	return (BroadcastChPPTestDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// BroadcastChPPTestView message handlers
