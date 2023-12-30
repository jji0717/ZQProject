// NodeGeneralInfo.cpp : implementation file
//

#include "stdafx.h"
#include "SysTray.h"
#include "NodeGeneralInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNodeGeneralInfo dialog


CNodeGeneralInfo::CNodeGeneralInfo(CWnd* pParent /*=NULL*/)
	: CInfoDlg(CNodeGeneralInfo::IDD, pParent)
{
	//{{AFX_DATA_INIT(MN_GeneralInfo)
	m_edCpu = _T("");
	m_edInt = _T("");
	m_edMem = _T("");
	m_edNet = _T("");
	m_edNid = _T("");
	m_edOs = _T("");
	m_edPnm = _T("");
	m_edVer = _T("");
	//}}AFX_DATA_INIT
}


void CNodeGeneralInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNodeGeneralInfo)
	DDX_Text(pDX, IDC_EDIT_CPU, m_edCpu);
	DDX_Text(pDX, IDC_EDIT_INT, m_edInt);
	DDX_Text(pDX, IDC_EDIT_MEM, m_edMem);
	DDX_Text(pDX, IDC_EDIT_NET, m_edNet);
	DDX_Text(pDX, IDC_EDIT_NID, m_edNid);
	DDX_Text(pDX, IDC_EDIT_OS, m_edOs);
	DDX_Text(pDX, IDC_EDIT_PNM, m_edPnm);
	DDX_Text(pDX, IDC_EDIT_VER, m_edVer);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNodeGeneralInfo, CDialog)
	//{{AFX_MSG_MAP(CNodeGeneralInfo)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNodeGeneralInfo message handlers

RpcValue& CNodeGeneralInfo::GenRpcValue()
{
	params.setStruct(INFOTYPE_KEY, RpcValue(INFOTYPE_GENERAL));
	params.setStruct(INFOPARAM_KEY, RpcValue(NULL));
	return params;
}

void CNodeGeneralInfo::UpdateDlgList(RpcValue& _result)
{
	char buf[1024];
	m_edOs = _T(_result[INFO_OS_KEY].ToString(buf, 1024));
	m_edCpu = _T(_result[INFO_CPU_KEY].ToString(buf,1024));

	// split free/total mem
	CString strMem(_T(_result[INFO_MEMORY_KEY].ToString(buf,1024))); 
	int nfreePos1 = strMem.Find('/');
	int nfreePos2 = strMem.Find('(');
	CString strFreeMem(strMem.Left(nfreePos1));
	CString strTotalMem(strMem.Mid(nfreePos1+1, (nfreePos2-nfreePos1)-1));
	FormatMemoryStyle(strFreeMem);
	FormatMemoryStyle(strTotalMem);
	strMem.Empty();
	strMem = strFreeMem + "/" + strTotalMem + " (FREE/TOTAL)";
	
	m_edMem = strMem;
	m_edNet = _T("Now Reserved!");
	m_edInt = _T(_result[INFO_INTERFACE_KEY].ToString(buf,1024));
	m_edPnm = _T(_result[INFO_PROCESS_KEY].ToString(buf,1024));
	m_edNid = _T(_result[INFO_NODEID_KEY].ToString(buf,1024));
	m_edVer = _T(_result[INFO_MPFVERSION_KEY].ToString(buf,1024));
}

void CNodeGeneralInfo::FormatMemoryStyle(CString& mem)
{
	CString strMem(mem);
	mem.Empty();
	int nSubStrLen = strMem.GetLength();
	while (nSubStrLen > 3)
	{
		strMem.Insert(nSubStrLen-3,',');
		mem.Insert(0,strMem.Right(4));
		strMem = strMem.Left(nSubStrLen-3);
		nSubStrLen -= 3;
	}
	mem.Insert(0,strMem);
}
