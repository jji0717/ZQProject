// DataDialog.h: interface for the CDataDialog class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __ZQDATADIALOG_H
#define __ZQDATADIALOG_H

#include "SortListViewCtrl.h"
#include "ButtonXP.h"
#include "resource.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDataDialog : public CDialogImpl<CDataDialog>  
{
public:
	CDataDialog(LPCTSTR szTabName =_T(""),BOOL bShowAboveCtrl = FALSE);
	virtual ~CDataDialog();

	enum { IDD = IDD_DATADIALOG };

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		pMsg;
		return IsDialogMessage(pMsg);
	}

	BEGIN_MSG_MAP(CDataDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SIZE,OnSize)
		COMMAND_HANDLER(IDC_SEARCHBTN, BN_CLICKED, OnSearch)
		COMMAND_HANDLER(IDC_DATACHECK, BN_CLICKED, OnDataBtn)
		REFLECT_NOTIFICATIONS()
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()
	
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnSearch(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnDataBtn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

public:
	static int CALLBACK  GetProgressProc( const int & iTotalNum, const int  & iCurNum);
	static int CALLBACK  GetPlayListStateProc(const string & strUid, const string & strMsg, const string & strStateValue, const int & iCurCtrlNum );
	static bool IsExist(const string & strUid, int * iIndex);
	static CProgressBarCtrl    m_ProgressCtrl;
	static CSortListViewCtrl*  m_pListCtrl;

protected:
	void InitContentListCtrl();
	void InitPlayListListCtrl();
	void InitOtherListCtrl();
	void RefreshListCtrl();
	void FreeMemoryData();
	
protected:
	char  m_strTabName[ITEMLEN];
	
	CXPButton           m_SearchBtn;
	CEditXP             m_ConditionEdit;
	CButton             m_CheckBtn;
	CStatic             m_StaticCtrl;
	
	BOOL				m_bShowAboveCtrl;
	BOOL                m_bShowAllData;
	int                 m_iNumColumns;
	ITEMDATA    **      m_pCellsData;
	int                 m_iNumRows;
	int                 m_iBtnSearchNum;
};

#endif // !defined(AFX_DATADIALOG_H__C7B897FC_2584_41F2_A1BC_08EBE902AD2D__INCLUDED_)
