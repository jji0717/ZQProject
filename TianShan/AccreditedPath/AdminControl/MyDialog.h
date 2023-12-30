// MyDialog.h: interface for the CAddDataDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYDIALOG_H__A7B811F0_4437_4BCB_B8F6_F44DC9B63EC1__INCLUDED_)
#define AFX_MYDIALOG_H__A7B811F0_4437_4BCB_B8F6_F44DC9B63EC1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#define WM_USER_EDIT_END WM_USER+1001
#include "resource.h"

typedef struct  
{ 
	int id;
	std::string  type; 
    std::string  netId; 
    std::string  desc; 
    std::string  ifep;
	long lMaxBW;
	int  iMaxSession;
	BOOL flag;//flag update data or add data, true is update data,and false is add data;
}SData;

typedef struct  
{
	std::string storageNetId;
	std::string streamerNetId;
	int SGId;
	std::string linktype;
    int bStorage;
	TianShanIce::Transport::StorageLinkPrx storagelinkPrx;
	TianShanIce::Transport::StreamLinkPrx  streamlinkprx;
}LinkInfo;

class CListEdit : public CWindowImpl<CListEdit>
{
public:
	CListEdit();
	virtual ~CListEdit();

	BEGIN_MSG_MAP(CListEdit)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
		MESSAGE_HANDLER(WM_KEYUP,OnKeyUp)
	END_MSG_MAP()
private:
	DWORD m_dwData;
	BOOL m_bExchange;
public:
	void    SetCtrlData(DWORD dwData);
	DWORD   GetCtrlData();
	LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
};

class CAddDataDlg : public CDialogImpl<CAddDataDlg>  
{
public:	
	///Init TabCtrl
	void InitTabAction(int iFlag);

	///Init ServiceGrout Tab
	void TabServiceGroup(std::string strName="ServcieGroup");

	///Init Streamer Tab
	void TabStreamer();

	///Init Storage Tab
	void TabStorage(std::string strName="Storage");
	
	/// init StreamerLink Tab
	void TabStreamerLink(std::string strName="StreamerLink");

	///Inti StorageLink Tab
	void TabStorageLink();


	/// init all combox
    ///@param[in] nID  combox ID
	///@param[in] pFindStirng Combox string flag
	void InitCombox(UINT nID, char * pFindString);

	///Open Config file
	///@return if the config file open sucessed,return TRUE
	///        else return FALSE
	BOOL OpenConfigfile();

	///Removes all item from Combox
	///@param[in] Combox ID 
	void DeleteComboItem(UINT nComboID);
   
	///Init ListCtrl
	void InitListCtrl();

	///Set privateData in ValueMap;
	///@return if the PrivateData is correct, return TRUE;
	///        else retrun FALSE;
	BOOL SetPrivateData();

	///Judges the Variant Type; 
    ///@param[in] pstr the string of Variant Type
	///@param[out] pvar a Variant receive the Variant Type
	void VariantType(const char * pstr,TianShanIce::Variant *pvar);
 
	CAddDataDlg();
	virtual ~CAddDataDlg();

	

    CAddDataDlg(Ice::CommunicatorPtr ic,TianShanIce::Transport::PathAdminPrx client, TianShanIce::Site::SiteAdminPrx businessClient, int nflag,SData data);
	CAddDataDlg(Ice::CommunicatorPtr ic,TianShanIce::Transport::PathAdminPrx client, TianShanIce::Site::SiteAdminPrx businessClient,int nflag, LinkInfo linkinfo);
	enum {PPG_UPSTORAGE, PPG_UPSTREAMER, PPG_UPSG,PPG_LINKSTORAGE,PPG_LINKSTREAMER,PPG_UPDATESITE,PPG_UPDATEAPP,PPG_MOUNTAPP};
	enum { IDD = IDD_ADDDATA };

	
	BEGIN_MSG_MAP(CAddDataDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	    MESSAGE_HANDLER(WM_USER_EDIT_END,OnEditEnd)
//      NOTIFY_HANDLER(IDC_TAB, TCN_SELCHANGE, OnSelchangeTab)
		
		COMMAND_HANDLER(IDC_LINKSTORAGEBTN, BN_CLICKED, OnClickedlinkstoragebtn)
		COMMAND_HANDLER(IDC_LINKSTREAMERBTN, BN_CLICKED, OnClickedLinkstreamerbtn)
		COMMAND_HANDLER(IDC_UPSGBTN, BN_CLICKED, OnClickedUpsgbtn)
		COMMAND_HANDLER(IDC_UPSTORAGEBTN, BN_CLICKED, OnClickedUpstoragebtn)
		COMMAND_HANDLER(IDC_UPSTREAMERBTN, BN_CLICKED, OnClickedUpstreamerbtn)
		NOTIFY_HANDLER(IDC_LIST, NM_DBLCLK, OnDblclkList)
		COMMAND_HANDLER(IDC_UPSTORAGELINKBTN, BN_CLICKED, OnClickedUpstoragelinkbtn)
		COMMAND_HANDLER(IDC_UPSTREAMERLINKBTN, BN_CLICKED, OnClickedUpstreamerlinkbtn)
		COMMAND_HANDLER(IDM_APP_EXIT, BN_CLICKED, OnClickAppExitbtn)
		COMMAND_HANDLER(IDC_STORAGELINKCOMBO, CBN_SELCHANGE, OnSelchangeStoragelinkcombo)
		COMMAND_HANDLER(IDC_STREAMERLINKCOMBO, CBN_SELCHANGE, OnSelchangeStreamerlinkcombo)
		COMMAND_HANDLER(IDC_GETSTORAGEDATA, BN_CLICKED, OnClickedGetstoragedata)
		COMMAND_HANDLER(IDC_GETSTREAMERDATA, BN_CLICKED, OnClickedGetstreamerdata)
	END_MSG_MAP()

public:
	
	BOOL ListCtrlAddData();
	void InitStreamerLinkType();
	void InitStorageLinkType();
	void UpLinkTab();
	void UpdateListCtrlData();
	void ShowEdit(BOOL bShow,int nItem,int nIndex, RECT rcCtrl);
	void ShowSiteControl(BOOL b1);
	
	LRESULT OnSelchangeTab(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnClickedlinkstoragebtn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnClickedLinkstreamerbtn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnClickedUpsgbtn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnClickedUpstoragebtn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnClickedUpstreamerbtn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSelchangeStoragelinkcombo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSelchangeStreamerlinkcombo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL& bHandled);
	LRESULT OnClose(UINT, WPARAM, LPARAM, BOOL& bHandled);
	LRESULT OnClickAppExitbtn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL& bHandled);
	LRESULT OnDblclkList(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnEditEnd(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClickedUpstoragelinkbtn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnClickedUpstreamerlinkbtn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnClickedGetstoragedata(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnClickedGetstreamerdata(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

private:
	

	HWND m_hTabAction;
	HWND m_hListCtrl;
	CListEdit m_listEdit;
	FILE *m_fp;
	UINT m_nfilesize;
	SData m_sData;
	int m_hflag;
	BOOL m_bStreamLinkType;

	LinkInfo m_linkinfo;
	Ice::CommunicatorPtr	    m_ic;
	
	TianShanIce::ValueMap m_Pvals;
	TianShanIce::PDSchema m_schema;
	TianShanIce::Transport::PathAdminPrx m_client;
	TianShanIce::Storage::ContentStorePrx     m_ContentClient;

	TianShanIce::Site::SiteAdminPrx  m_businessClient;
	TianShanIce::Streamer::StreamServicePrx   m_StreamerClient;
};

#endif // !defined(AFX_MYDIALOG_H__A7B811F0_4437_4BCB_B8F6_F44DC9B63EC1__INCLUDED_)
