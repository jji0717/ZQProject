// CodManDoc.h : interface of the CCodManDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CODMANDOC_H__697E29EB_52DE_4445_859D_6B226ACDFD09__INCLUDED_)
#define AFX_CODMANDOC_H__697E29EB_52DE_4445_859D_6B226ACDFD09__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "OpenCodDlg.h"
#include "ChannelOnDemandEx.h"

//#define USE_OLD_NS  // The switch to use the old namespace

#ifdef USE_OLD_NS
#  define  NS_PREFIX(_CLS) _CLS
#  define  CI_NS_PREFIX(_CLS) ChannelOnDemand::_CLS
#else
#  define  NS_PREFIX(_CLS) TianShanIce::Application::_CLS
#  define  CI_NS_PREFIX(_CLS) TianShanIce::Application::_CLS
#endif // USE_OLD_NS

#define TreeItem_Root 0
#define TreeItem_Channel 1
#define TreeItem_ChannelItem 2
#define ListItem_Channel 1
#define ListItem_ChannelItem 2
class CCodManDoc : public CDocument
{
protected: // create from serialization only
	CCodManDoc();
	DECLARE_DYNCREATE(CCodManDoc)

// Attributes
public:
	NS_PREFIX(ChannelOnDemand::ChannelPublisherPrx) _publisherPrx;
	CString _codEndPoint;
	TianShanIce::StrValues _chnlIds;
	std::string _curChnl;

// Operations
public:
	bool RefreshChannels();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCodManDoc)
	public:
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CCodManDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	COpenCodDlg _openCod;

// Generated message map functions
protected:
	//{{AFX_MSG(CCodManDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CODMANDOC_H__697E29EB_52DE_4445_859D_6B226ACDFD09__INCLUDED_)
