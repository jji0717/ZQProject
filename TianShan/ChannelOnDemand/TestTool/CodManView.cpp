// CodManView.cpp : implementation of the CCodManView class
//

#include "stdafx.h"
#include "CodMan.h"

#include "CodManDoc.h"
#include "CodManView.h"
#include "ChildFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCodManView

IMPLEMENT_DYNCREATE(CCodManView, CView)

BEGIN_MESSAGE_MAP(CCodManView, CView)
	//{{AFX_MSG_MAP(CCodManView)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCodManView construction/destruction

CCodManView::CCodManView()
{
	// TODO: add construction code here

}

CCodManView::~CCodManView()
{
}

BOOL CCodManView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CCodManView drawing

void CCodManView::OnDraw(CDC* pDC)
{
	CCodManDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CCodManView printing

BOOL CCodManView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CCodManView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CCodManView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CCodManView diagnostics

#ifdef _DEBUG
void CCodManView::AssertValid() const
{
	CView::AssertValid();
}

void CCodManView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CCodManDoc* CCodManView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCodManDoc)));
	return (CCodManDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CCodManView message handlers
