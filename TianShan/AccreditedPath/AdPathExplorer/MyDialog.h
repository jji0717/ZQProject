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
class CListEdit : public CWindowImpl<CListEdit>
{
public:
	CListEdit();
	virtual ~CListEdit();

	BEGIN_MSG_MAP(CListEdit)
    MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
	END_MSG_MAP()
private:
	DWORD m_dwData;
	BOOL m_bExchange;
public:
	void  SetCtrlData(DWORD dwData)
	{
		m_dwData = dwData;
	}
	DWORD GetCtrlData()
	{
        return m_dwData;
	}

	LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		// TODO : Add Code for message handler. Call DefWindowProc if necessary.
	     HWND pParent = ::GetParent(m_hWnd);
		::PostMessage(pParent,WM_USER_EDIT_END,m_bExchange,0);
		
	 	DefWindowProc(uMsg,wParam,lParam);
		return 0;	
	}
};


class CAddDataDlg : public CDialogImpl<CAddDataDlg>  
{
public:	
	///Init TabCtrl
	void InitTabAction();

	///Init ServiceGrout Tab
	void TabServiceGroup();

	///Init Streamer Tab
	void TabStreamer();

	///Init Storage Tab
	void TabStorage();
	
	/// init StreamerLink Tab
	void TabStreamerLink();

	///Inti StorageLink Tab
	void TabStorageLink();

	///Init ListCtrl
	///@param[in] pFindString ListCtrl flag
	void LCAddPrivateData(char * pFindString);

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
	void VariantType(const char * pstr,::TianShanIce::Variant *pvar);
 
	CAddDataDlg();
	virtual ~CAddDataDlg();
    CAddDataDlg(TianShanIce::AccreditedPath::PathAdminPrx client, int nflag,SData data);
    CAddDataDlg(TianShanIce::AccreditedPath::PathAdminPrx client, int nflag, LinkInfo linkinfo);
	HWND m_hTabAction;
	HWND m_hListCtrl;
	CListEdit m_listEdit;
	FILE *m_fp;
	UINT m_nfilesize;
	SData m_sData;
	LinkInfo m_linkinfo;
	TianShanIce::AccreditedPath::PathAdminPrx m_client;
	int m_hflag;
	enum {PPG_UPSTORAGE, PPG_UPSTREAMER, PPG_UPSG,PPG_LINKSTORAGE,PPG_LINKSTREAMER};
	enum { IDD = IDD_ADDDATA };

	::TianShanIce::ValueMap m_Pvals;
	::TianShanIce::AccreditedPath::PDSchema m_schema;

	BEGIN_MSG_MAP(CAddDataDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	    MESSAGE_HANDLER(WM_USER_EDIT_END,OnEditEnd)
        NOTIFY_HANDLER(IDC_TAB, TCN_SELCHANGE, OnSelchangeTab)
		
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
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL& bHandled)
	{
		// when the dialog box is created, subclass its edit control:
		bHandled = FALSE;
		m_hTabAction = GetDlgItem(IDC_TAB);
        m_hListCtrl = GetDlgItem(IDC_LIST);

       m_listEdit.SubclassWindow(GetDlgItem(IDC_EDIT));
	   ::ShowWindow(GetDlgItem(IDC_EDIT),SW_HIDE);

/*		DWORD dwStyle = ::GetWindowLong(m_listEdit.m_hWnd,GWL_STYLE);
       dwStyle |= ES_AUTOHSCROLL | ES_NOHIDESEL | WS_CHILD | WS_BORDER ;
       ::SetWindowLong(m_listEdit.m_hWnd,GWL_STYLE,dwStyle);*/

		::SetWindowText(GetDlgItem(IDC_EP_EDIT), " ContentStore:default -p 10000 ");
		::SetWindowText(GetDlgItem(IDC_DESC_EDIT), "this is a test!");

       if(!OpenConfigfile())
	   {
		   ::SendMessage(m_hWnd,WM_CLOSE,NULL,NULL);
		   return 0;
	   }
        InitTabAction();
        InitCombox(IDC_STORAGE_COMBO, "StorageType");
        InitCombox(IDC_STREAMER_COMBO, "StreamerType");
        InitStorageLinkType();
		InitStreamerLinkType();
		InitListCtrl();
       
		switch (m_hflag)
		{
		case PPG_UPSTORAGE:
			TabStorage();
			if(m_sData.flag)
			{
			 ::SetWindowText(GetDlgItem(IDC_EP_EDIT), (LPCSTR)m_sData.ifep.c_str());
		     ::SetWindowText(GetDlgItem(IDC_DESC_EDIT),(LPCSTR)m_sData.desc.c_str());
			 ::SetWindowText(GetDlgItem(IDC_UK_EDIT),(LPCSTR)m_sData.netId.c_str());
			 ::EnableWindow(GetDlgItem(IDC_UK_EDIT), FALSE);
			 int npos = SendMessage( GetDlgItem(IDC_STORAGE_COMBO) ,CB_FINDSTRING, 0,(LPARAM)(LPCSTR)m_sData.type.c_str());
			 if(npos >= 0)
				 ::SendMessage( GetDlgItem(IDC_STORAGE_COMBO),CB_SETCURSEL,(WPARAM) npos, 0);
			 else
			 {
				 ::SendMessage( GetDlgItem(IDC_STORAGE_COMBO),CB_SETCURSEL,(WPARAM) npos, 0);
				 MessageBox("No Type Match!");
			 }
			 
			 try
			 {	 	
				 m_Pvals = m_client->getStoragePrivateData(m_sData.netId);
				 
			 }
			 catch (const Ice::Exception& ex)
			 {
				 MessageBox(_T(ex.ice_name().c_str()),"connect error");
				 return 0;
			 }
		

			 UpdateListCtrlData();
			}
			break;
		case PPG_UPSTREAMER:
			TabStreamer();
			if(m_sData.flag)
			{
			 ::SetWindowText(GetDlgItem(IDC_EP_EDIT), (LPCSTR)m_sData.ifep.c_str());
		     ::SetWindowText(GetDlgItem(IDC_DESC_EDIT),(LPCSTR)m_sData.desc.c_str());
			 ::SetWindowText(GetDlgItem(IDC_UK_EDIT),(LPCSTR)m_sData.netId.c_str());
			 ::EnableWindow(GetDlgItem(IDC_UK_EDIT), FALSE);
			  int npos = SendMessage( GetDlgItem(IDC_STREAMER_COMBO) ,CB_FINDSTRING, 0,(LPARAM)(LPCSTR)m_sData.type.c_str());
			 if(npos >= 0)
				 ::SendMessage( GetDlgItem(IDC_STREAMER_COMBO),CB_SETCURSEL,(WPARAM) npos, 0);
			 else
			 {
				::SendMessage( GetDlgItem(IDC_STREAMER_COMBO),CB_SETCURSEL,(WPARAM) npos, 0);
				MessageBox("No Type Match!");
			 }

			 m_Pvals.clear();

			 try
			 {	 
				 m_Pvals = m_client->getStreamerPrivateData(m_sData.netId);
				 
			 }
			 catch (const Ice::Exception& ex)
			 {
				 MessageBox(_T(ex.ice_name().c_str()),"connect error");
				 return 0;
			 }
			 
			 UpdateListCtrlData();
			}
			break;
		case PPG_UPSG:
			TabServiceGroup();
			if(m_sData.flag)
			{
			 char temp[10];
			 ::SetWindowText(GetDlgItem(IDC_SGID_EDIT), (LPCSTR)itoa(m_sData.id, temp, 10));
		     ::SetWindowText(GetDlgItem(IDC_SGDESC_EDIT),(LPCSTR)m_sData.desc.c_str());
			 ::EnableWindow(GetDlgItem(IDC_SGID_EDIT), FALSE);
			}
			break;
		case PPG_LINKSTORAGE:
			TabStorageLink();
			if(m_linkinfo.bStorage == 1)
			{
			  UpLinkTab();
			}
			break;
		case PPG_LINKSTREAMER:
			TabStreamerLink();
			if(m_linkinfo.bStorage == 0)
			{
               UpLinkTab();
			}
			
		default:
			break;
		}
		if(m_hflag >= 0)
		 TabCtrl_SetCurSel(m_hTabAction, m_hflag);

/*		HMENU hMenu = LoadMenu(_Module.GetResourceInstance(),
		MAKEINTRESOURCE(IDR_MENU));
	    ::SetMenu (m_hWnd, hMenu) ;
		DestroyMenu(hMenu);*/
		return 0;
	}

	LRESULT OnClose(UINT, WPARAM, LPARAM, BOOL& bHandled)
	{
		// when the dialog box is created, subclass its edit control:
		EndDialog(0);
		return 0;
	}
		LRESULT OnClickAppExitbtn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
		{ 
		   ::SendMessage(m_hWnd,WM_CLOSE,NULL,NULL);
		   return 0;
		}

	LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL& bHandled)
	{
		// when the dialog box is created, subclass its edit control:
		if(m_fp)
			fclose(m_fp);
		return 0;
	}

public:
	void InitStorageType();
	void InitStreamerType();
	BOOL ListCtrlAddData();
	void InitStreamerLinkType();
	void InitStorageLinkType();

	void UpLinkTab();
	void UpdateListCtrlData();

	LRESULT OnSelchangeTab(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT OnClickedlinkstoragebtn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnClickedLinkstreamerbtn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnClickedUpsgbtn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnClickedUpstoragebtn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnClickedUpstreamerbtn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnSelchangeStoragelinkcombo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnSelchangeStreamerlinkcombo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnDblclkList(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
	{
		// TODO : Add Code for control notification handler.
		RECT rcCtrl;
		LVHITTESTINFO lvhti;
		POINT pos;
		::GetCursorPos(&pos);
		// Retreive the coordinates of the treecrtl viwndow
		RECT cRect ;
		::GetWindowRect(m_hListCtrl, &cRect);
		
		pos.x += -cRect.left;
		pos.y += -cRect.top;

		lvhti.pt = pos;
		int nItem = ListView_SubItemHitTest(m_hListCtrl, &lvhti);
		if(nItem == -1)
			return 0;
		int nSubItem = lvhti.iSubItem;
		if(nSubItem != 1)
			return 0;
		ListView_GetSubItemRect(m_hListCtrl, nItem, nSubItem, LVIR_LABEL, &rcCtrl);
		ShowEdit(TRUE,nItem,nSubItem,rcCtrl);
		
		return 0;
	}
	void ShowEdit(BOOL bShow,int nItem,int nIndex, RECT rcCtrl)
	{
		RECT cRect, cDlgRect;
		HWND test;
		RECT rect = {0};
//		m_listEdit.Create(m_hWnd,rect,NULL,WS_VISIBLE|ES_AUTOHSCROLL|WS_CHILD|ES_LEFT|ES_WANTRETURN|WS_BORDER,0,IDC_EDIT1,NULL);
//		m_listEdit.Create(m_hWnd,rect,NULL,WS_VISIBLE|ES_AUTOHSCROLL|ES_LEFT|ES_WANTRETURN|WS_BORDER,0,IDC_EDIT1,NULL);
		if(bShow == TRUE)
		{
			char temp[50];
			ListView_GetItemText(m_hListCtrl,nItem,nIndex, temp, 50);
          
			::GetWindowRect(m_hWnd, &cDlgRect);
			::GetWindowRect(m_hListCtrl, &cRect);
			::MoveWindow(m_listEdit.m_hWnd,rcCtrl.left +cRect.left - cDlgRect.left, rcCtrl.top + cRect.top - cDlgRect.top - 40 ,rcCtrl.right - rcCtrl.left + 1,rcCtrl.bottom - rcCtrl.top + 3, TRUE);
			::ShowWindow(m_listEdit.m_hWnd,SW_SHOW);
			
			::SetWindowText(m_listEdit.m_hWnd,temp);
		    test = ::SetFocus(m_listEdit.m_hWnd);
			
			::SendMessage(m_listEdit.m_hWnd,EM_SETSEL,0, -1);

			m_listEdit.SetCtrlData(MAKEWPARAM(nIndex,nItem)); 
		}
		
		else
			::ShowWindow(m_listEdit.m_hWnd,SW_HIDE);

	}
	
	LRESULT OnEditEnd(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if(wParam == TRUE)
		{
			char temp[50];
			::GetWindowText(m_listEdit.m_hWnd,temp, 50);
			DWORD dwData = m_listEdit.GetCtrlData();
			int nItem= dwData>>16;
			int nIndex = dwData&0x0000ffff;
			ListView_SetItemText(m_hListCtrl, nItem,nIndex,temp);
		}
		else
		{
			
		}	
		if(lParam == FALSE)
		{
		::ShowWindow(m_listEdit.m_hWnd,SW_HIDE);
		}
		return 0;
	}
	LRESULT OnClickedUpstoragelinkbtn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		// TODO : Add Code for control notification handler.
      AtlTrace("%d", m_schema.size());
	  if(!SetPrivateData())
			return 0;
		try
		{
		   ::TianShanIce::AccreditedPath::StorageLinkExPrx storagelinkexprx = ::TianShanIce::AccreditedPath::StorageLinkExPrx::checkedCast(m_linkinfo.storagelinkPrx);
			storagelinkexprx->updatePrivateData(m_Pvals);
		}
		catch(::TianShanIce::InvalidParameter& ex)
		{
			MessageBox(ex.message.c_str());
			return 0;
		}	
		catch (const Ice::Exception& ex)
		{
			MessageBox(_T(ex.ice_name().c_str()),"connect error");
			return 0;
		}
		MessageBox("Update linkStorage success!","Update Link");
		m_Pvals = m_linkinfo.storagelinkPrx ->getPrivateData();
		UpdateListCtrlData();
		return 0;
	}
	LRESULT OnClickedUpstreamerlinkbtn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		// TODO : Add Code for control notification handler.
 //   AtlTrace("%d", m_schema.size());
	if(!SetPrivateData())
			return 0;
	try
	 {
	 ::TianShanIce::AccreditedPath::StreamLinkExPrx streamlinkexprx = ::TianShanIce::AccreditedPath::StreamLinkExPrx::checkedCast(m_linkinfo.streamlinkprx);
	  streamlinkexprx->updatePrivateData(m_Pvals);
	 }
	 catch(::TianShanIce::InvalidParameter& ex)
	 {
		 MessageBox(ex.message.c_str());
		 return 0;
	 }	
	 catch (const Ice::Exception& ex)
	 {
		 MessageBox(_T(ex.ice_name().c_str()),"connect error");
		 return 0;
	 }
	 MessageBox("Update linkStreamer success!","Update Link");
	 m_Pvals = m_linkinfo.streamlinkprx->getPrivateData();
	 UpdateListCtrlData();
	 return 0;
	}
};

#endif // !defined(AFX_MYDIALOG_H__A7B811F0_4437_4BCB_B8F6_F44DC9B63EC1__INCLUDED_)
