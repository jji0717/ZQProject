// EventsDialog.h: interface for the CEventsDialog class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __ZQEVENTSDIALOG_H
#define __ZQEVENTSDIALOG_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "resource.h"
#include <Locks.h>
#include "EventAddDataThread.h"
#include "EventLog.h"
#include "SortListViewCtrl.h"
#include "ButtonXP.h"

typedef struct _tagColumnLenData
{
	int  iColumn0;
	int  iColumn1;
	int  iColumn2;
	int  iColumn3;
	int  iColumn4;
}COLUMNLENDATA,*PCOLUMNLENDATA;

typedef struct _tagAllItemsData
{
	int iIndex;
	string strDataTime;
	string strCateGory;
	string strEventData;
}ALLITEMSDATA,*PALLITEMSDATA;

typedef struct _tagOptionData
{
	int iErrors;
	int iWarnings;
	int iInformation;
	int iShowLines;
}OPTIONDATA,*POPTIONDATA;

typedef CSimpleArray<ALLITEMSDATA> SIMPLEARRAY;


#define THREADMODE 1

class CEventsDialog : public CDialogImpl<CEventsDialog> 
{
public:
	CEventsDialog();
	virtual ~CEventsDialog();

	enum { IDD = IDD_EVENTDIALOG };

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		pMsg;
		return IsDialogMessage(pMsg);
	}

	BEGIN_MSG_MAP(CEventsDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SIZE,OnSize)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnOK)
		COMMAND_HANDLER(IDC_PLAYBUTTON,BN_CLICKED,OnStopEvent)
		REFLECT_NOTIFICATIONS()
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()
	
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnStopEvent(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	CListViewCtrl & getListViewCtrl();
public:
	static int AddListCtrlData(const string & strCategory, const int &iLevel, const string & strCurTime,const  string & strMessage);
	static int CALLBACK   OnStreamEvent(string & strCategory, int &iLevel, string & strCurTime, string & strMessage);
//	static CListViewCtrl  m_ListCtrl;
	static CSortListViewCtrl*  m_pListCtrl;

	
	static int            m_iIndex;
	static SIMPLEARRAY    m_DataArray;			
private:
	void SaveColumnLenData();
	void InitData();
	void GetCateGoryFromDll();
	void RefreshListCtrl();
protected:
	CBitmap        m_bkbmp;
	CButton        m_playBtn;
	CXPButton      m_OKBtn;
	int            m_iNumColumns;
	static int     m_iCateGoryCount;
	static int     m_iErrors;
	static int     m_iWarnings;
	static int     m_iInformation;
	static int     m_iShowLines;
	static ZQ::common::Mutex		m_Mutex;
	static CEventFileLog * m_pEventLogFile;
	COLUMNLENDATA  m_iColumnData;
	static ITEMDATA *   m_pCategorys;
	static int*         m_pCateCheck;
	static bool         m_bEventStop;
	
#ifdef THREADMODE
//	static CEventAddDataThread *m_pAddDataThread;
	CEventAddDataThread *m_pAddDataThread;
#endif
};

class COptionsDialog : public CDialogImpl<COptionsDialog> 
{
public:
	COptionsDialog(OPTIONDATA &OptionData,ITEMDATA **pCategorys,int *iCount,int *pCateCheck);
	virtual ~COptionsDialog();

	enum { IDD = IDD_OPTIONDIALOG };

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		pMsg;
		return IsDialogMessage(pMsg);
	}

	BEGIN_MSG_MAP(COptionsDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SIZE,OnSize)
		COMMAND_HANDLER(IDC_ERRORCHECK, BN_CLICKED, OnErrorClick)
		COMMAND_HANDLER(IDC_WARNCHECK, BN_CLICKED, OnWarnClick)
		COMMAND_HANDLER(IDC_INFOCHECK, BN_CLICKED, OnInfoClick)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnOK)
		REFLECT_NOTIFICATIONS()
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnErrorClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnWarnClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnInfoClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

public:
	void GetOptionData(OPTIONDATA * pValue,int *pCateCheck);
private:
	OPTIONDATA  m_OptionData;
	CButton     m_ErrorBtn;
	CButton     m_WarningBtn;
	CButton     m_InforamtionBtn;
//	CEdit       m_LineEdit;
	CEditXP     m_LineEdit;
	ITEMDATA ** m_pCategorys;
	int         m_iCateGoryCount;
	int     *   m_pCateCheck;
	CButton     *m_pCheckButton;
	CXPButton   m_OKBtn;

};

#endif // !defined(AFX_EVENTSDIALOG_H__2FAD1CD9_4682_4822_BEDE_BB9B442BAE72__INCLUDED_)
