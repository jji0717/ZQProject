#pragma once

#define WM_CLOSE_CHILD WM_USER+100

#include "afxwin.h"
#include "afxcmn.h"
#include "FileLog.h"
#include "NativeThreadPool.h"
#include <vector>
#include <map>
#include "CdmiDokan.h"

// CCdmiFuseDlg dialog

class CCdmiFuseDlg : public CDialog
{
	DECLARE_DYNAMIC(CCdmiFuseDlg)

public:
	CCdmiFuseDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCdmiFuseDlg();

	// Dialog Data
	enum { IDD = IDD_POPUP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedLogin();
	afx_msg void OnBnClickedApply();
	afx_msg void OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult);
public:
	CComboBox m_cbContainer;
	CComboBox m_cbDrive;
	CListCtrl m_listContainer;
	CButton m_buttonSave;
	CButton m_buttonAutomount;

	afx_msg void OnClose();
	afx_msg LRESULT OnExit(WPARAM wParam,LPARAM lParam); //message of exit dialog/application
	afx_msg LRESULT OnGetDefID(WPARAM wParam,LPARAM lParam);
	afx_msg void OnNcPaint();

	CString m_strUserName;
	CString m_strPassWord;
	CString m_strCdmiServer;

public:
	ZQ::common::FileLog m_log;
	ZQ::common::NativeThreadPool m_thpool;

protected:
	virtual void refreshContainerList(std::vector <std::string> &Containers);
	void initListCtrl();
public:
	afx_msg void OnEnChangeEdit1();
	void saveUserInfor();
	void inforBox(CString inforData,CString titleData="Warn");
	std::string findUser(std::string &userName);
	void addFirstUser();
	void deleteUser(void);
	std::string getRootURL();
	void clearDokanFuse();
	void addDokanFuse(std::string container,std::string drive);
	void deleteDokanFuse(std::string container);
	std::string getPath(std::string pathType,std::string driveName="");
	bool driveJudge(std::string &drive);
public:
	std::vector < HWND > m_handleChild;
	std::string m_strUserInfor;
	std::map <std::string , CdmiDokan*> m_ptrDokanFuse;
	std::map <std::string , ZQ::common::FileLog*> m_ptrMountLog;
	uint32 dokanOptions;
	HANDLE hSingleMutex;
public:
	afx_msg void OnNMRclickList1(NMHDR *pNMHDR, LRESULT *pResult);
public:
	afx_msg void OnEnChangeEditUser();
public:
	afx_msg void OnMenuDelete();
public:
	afx_msg void OnMenuConnect();
public:
	afx_msg void OnMenuAutomount();
};



// CDialogInfo dialog

class CDialogInfo : public CDialog
{
	DECLARE_DYNAMIC(CDialogInfo)

public:
	CDialogInfo(CString &infor,CString title="Warn",CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogInfo();
	virtual BOOL OnInitDialog();
private:
	CString inforData;
	CString titleData;

	// Dialog Data
	enum { IDD = IDD_INFO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
public:
	void OnClose(void);
public:
	afx_msg void OnPaint();
public:
	afx_msg LRESULT OnCloseChild(WPARAM,LPARAM  );
};


#pragma once