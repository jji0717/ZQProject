// MyDialog.cpp: implementation of the CAddDataDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MyDialog.h"
#include "resource.h"
#include <sys/types.h>
#include <sys/stat.h>
//////////////////////////////////////////////////////////////////////////
// class CListEdit

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CListEdit::CListEdit()
{
	m_bExchange = TRUE;
}

CListEdit::~CListEdit()
{
	
}

//////////////////////////////////////////////////////////////////////////
// class CAddDataDlg
CAddDataDlg::CAddDataDlg() 
{
	
}
CAddDataDlg::CAddDataDlg(TianShanIce::AccreditedPath::PathAdminPrx client, int nflag, SData data)
{
	m_client = client;
    m_hflag = nflag;
	m_sData = data;
	m_linkinfo.bStorage = -1;

}
CAddDataDlg::CAddDataDlg(TianShanIce::AccreditedPath::PathAdminPrx client, int nflag, LinkInfo linkinfo)
{
	m_linkinfo = linkinfo;
	m_client = client;
    m_hflag = nflag;
}
CAddDataDlg::~CAddDataDlg()
{
	
}
void CAddDataDlg::InitStreamerType()
{

}

void CAddDataDlg::InitStorageType()
{

}

void CAddDataDlg::InitTabAction()
{   
	TCITEM tcitem = {0};
	tcitem.mask = TCIF_TEXT | TCIF_STATE | TCIF_PARAM ;
	
	tcitem.pszText = "Storage";
	TabCtrl_InsertItem(m_hTabAction, PPG_UPSTORAGE, &tcitem);
	
	tcitem.pszText = "Streamer";
	TabCtrl_InsertItem(m_hTabAction, PPG_UPSTREAMER, &tcitem );
	
	tcitem.pszText = "ServiceGroup";
	TabCtrl_InsertItem(m_hTabAction, PPG_UPSG, &tcitem);
	
	tcitem.pszText = "Storage Link";
	TabCtrl_InsertItem(m_hTabAction, PPG_LINKSTORAGE, &tcitem);
	
	tcitem.pszText = "Streamer Link";
	TabCtrl_InsertItem(m_hTabAction, PPG_LINKSTREAMER, &tcitem);
	TabStorage();
}
void CAddDataDlg::TabStorage()
{
	::ShowWindow(GetDlgItem(IDC_UK_EDIT),SW_SHOW);
    ::ShowWindow(GetDlgItem(IDC_EP_EDIT),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_DESC_EDIT),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_LIST),SW_SHOW);
    ::ShowWindow(GetDlgItem(IDC_STORAGE_COMBO),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_STATIC2),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_STATIC3),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_STATIC4),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_STATIC5),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_STATIC15),SW_HIDE);

    ::ShowWindow(GetDlgItem(IDC_GETSTORAGEDATA),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_GETSTREAMERDATA),SW_HIDE);

	::ShowWindow(GetDlgItem(IDC_UPSTORAGEBTN),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_UPSTREAMERBTN),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_UPSGBTN),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_LINKSTORAGEBTN),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_LINKSTREAMERBTN),SW_HIDE);
	
	
    ::ShowWindow(GetDlgItem(IDC_STREAMER_COMBO), SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC6),SW_HIDE);
	
	::ShowWindow(GetDlgItem(IDC_SGID_EDIT), SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_SGDESC_EDIT), SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC7), SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC8), SW_HIDE);
	
	
	::ShowWindow(GetDlgItem(IDC_STATIC9),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC10),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC11),SW_HIDE);
	
    ::ShowWindow(GetDlgItem(IDC_COMBO11), SW_HIDE);	
    ::ShowWindow(GetDlgItem(IDC_COMBO22), SW_HIDE);	
    ::ShowWindow(GetDlgItem(IDC_STORAGELINKCOMBO), SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STREAMERLINKCOMBO), SW_HIDE);	

	
	::ShowWindow(GetDlgItem(IDC_STATIC13),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC14),SW_HIDE);

	::ShowWindow(GetDlgItem(IDC_UPSTORAGELINKBTN),SW_HIDE);
    ::ShowWindow(GetDlgItem(IDC_UPSTREAMERLINKBTN),SW_HIDE);
    LCAddPrivateData("StoragePrivateKeys");	
	
	::SetWindowText(GetDlgItem(IDC_EP_EDIT), "");
	::SetWindowText(GetDlgItem(IDC_DESC_EDIT),"");
	::SetWindowText(GetDlgItem(IDC_UK_EDIT),"");
	::EnableWindow(GetDlgItem(IDC_UK_EDIT), TRUE);
}

void CAddDataDlg::TabStreamer()
{
	::ShowWindow(GetDlgItem(IDC_UK_EDIT),SW_SHOW);
    ::ShowWindow(GetDlgItem(IDC_EP_EDIT),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_DESC_EDIT),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_LIST),SW_SHOW);
    ::ShowWindow(GetDlgItem(IDC_STORAGE_COMBO),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC2),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_STATIC3),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC4),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_STATIC5),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_STATIC15),SW_SHOW);

	::ShowWindow(GetDlgItem(IDC_GETSTORAGEDATA),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_GETSTREAMERDATA),SW_SHOW);
	
	::ShowWindow(GetDlgItem(IDC_UPSTORAGEBTN),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_UPSTREAMERBTN),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_UPSGBTN),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_LINKSTORAGEBTN),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_LINKSTREAMERBTN),SW_HIDE);
	
    ::ShowWindow(GetDlgItem(IDC_STREAMER_COMBO), SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_STATIC6),SW_SHOW);
	
	::ShowWindow(GetDlgItem(IDC_SGID_EDIT), SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_SGDESC_EDIT), SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC7), SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC8), SW_HIDE);

	::ShowWindow(GetDlgItem(IDC_UPSTORAGELINKBTN),SW_HIDE);
    ::ShowWindow(GetDlgItem(IDC_UPSTREAMERLINKBTN),SW_HIDE);
	
	::ShowWindow(GetDlgItem(IDC_STATIC9),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC10),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC11),SW_HIDE);
	
    ::ShowWindow(GetDlgItem(IDC_COMBO11), SW_HIDE);	
    ::ShowWindow(GetDlgItem(IDC_COMBO22), SW_HIDE);	
    ::ShowWindow(GetDlgItem(IDC_STORAGELINKCOMBO), SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STREAMERLINKCOMBO), SW_HIDE);	

	
	::ShowWindow(GetDlgItem(IDC_STATIC13),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC14),SW_HIDE);
	LCAddPrivateData("StreamerPrivateKeys");
	
	::SetWindowText(GetDlgItem(IDC_EP_EDIT), "");
	::SetWindowText(GetDlgItem(IDC_DESC_EDIT),"");
	::SetWindowText(GetDlgItem(IDC_UK_EDIT),"");
	::EnableWindow(GetDlgItem(IDC_UK_EDIT), TRUE);
}

void CAddDataDlg::TabServiceGroup()
{
	::ShowWindow(GetDlgItem(IDC_UK_EDIT),SW_HIDE);
    ::ShowWindow(GetDlgItem(IDC_EP_EDIT),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_DESC_EDIT),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_LIST),SW_HIDE);
    ::ShowWindow(GetDlgItem(IDC_STORAGE_COMBO),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC2),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC3),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC4),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC5),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC15),SW_HIDE);

	::ShowWindow(GetDlgItem(IDC_GETSTORAGEDATA),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_GETSTREAMERDATA),SW_HIDE);
	
	::ShowWindow(GetDlgItem(IDC_UPSTORAGEBTN),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_UPSTREAMERBTN),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_UPSGBTN),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_LINKSTORAGEBTN),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_LINKSTREAMERBTN),SW_HIDE);
	
    ::ShowWindow(GetDlgItem(IDC_STREAMER_COMBO), SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC6),SW_HIDE);
	
	::ShowWindow(GetDlgItem(IDC_SGID_EDIT), SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_SGDESC_EDIT), SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_STATIC7), SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_STATIC8), SW_SHOW);
	
	::ShowWindow(GetDlgItem(IDC_STATIC9),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC10),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC11),SW_HIDE);
	
    ::ShowWindow(GetDlgItem(IDC_COMBO11), SW_HIDE);	
    ::ShowWindow(GetDlgItem(IDC_COMBO22), SW_HIDE);	
    ::ShowWindow(GetDlgItem(IDC_STORAGELINKCOMBO), SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STREAMERLINKCOMBO), SW_HIDE);
 
	
	::ShowWindow(GetDlgItem(IDC_STATIC13),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC14),SW_HIDE);

	::ShowWindow(GetDlgItem(IDC_UPSTORAGELINKBTN),SW_HIDE);
    ::ShowWindow(GetDlgItem(IDC_UPSTREAMERLINKBTN),SW_HIDE);

	::SetWindowText(GetDlgItem(IDC_SGID_EDIT), "");
	::SetWindowText(GetDlgItem(IDC_SGDESC_EDIT),"");
	::EnableWindow(GetDlgItem(IDC_SGID_EDIT), TRUE);


}

void CAddDataDlg::TabStorageLink()
{
	::EnableWindow(GetDlgItem(IDC_COMBO11), TRUE);
	::EnableWindow(GetDlgItem(IDC_COMBO22), TRUE);
	::EnableWindow(GetDlgItem(IDC_STORAGELINKCOMBO), TRUE);


	::ShowWindow(GetDlgItem(IDC_UK_EDIT),SW_HIDE);
    ::ShowWindow(GetDlgItem(IDC_EP_EDIT),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_DESC_EDIT),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_LIST),SW_SHOW);
    ::ShowWindow(GetDlgItem(IDC_STORAGE_COMBO),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC2),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC3),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC4),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC5),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC15),SW_HIDE);

	::ShowWindow(GetDlgItem(IDC_GETSTORAGEDATA),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_GETSTREAMERDATA),SW_HIDE);
	
	::ShowWindow(GetDlgItem(IDC_UPSTORAGEBTN),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_UPSTREAMERBTN),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_UPSGBTN),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_LINKSTORAGEBTN),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_LINKSTREAMERBTN),SW_HIDE);
	
    ::ShowWindow(GetDlgItem(IDC_STREAMER_COMBO), SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC6),SW_HIDE);

	::ShowWindow(GetDlgItem(IDC_UPSTORAGELINKBTN),SW_HIDE);
    ::ShowWindow(GetDlgItem(IDC_UPSTREAMERLINKBTN),SW_HIDE);
	
	::ShowWindow(GetDlgItem(IDC_SGID_EDIT), SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_SGDESC_EDIT), SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC7), SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC8), SW_HIDE);
	
	::ShowWindow(GetDlgItem(IDC_STATIC9),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_STATIC10),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_STATIC11),SW_SHOW);
	
	::ShowWindow(GetDlgItem(IDC_COMBO11), SW_SHOW);	
    ::ShowWindow(GetDlgItem(IDC_COMBO22), SW_SHOW);	
    ::ShowWindow(GetDlgItem(IDC_STORAGELINKCOMBO), SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_STREAMERLINKCOMBO), SW_HIDE);

    
	::ShowWindow(GetDlgItem(IDC_STATIC13),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC14),SW_HIDE);
	
	DeleteComboItem(IDC_COMBO11);
    DeleteComboItem(IDC_COMBO22);
	::TianShanIce::AccreditedPath::Storages storages;
	try
	{	 
    storages  = m_client->listStorages();
	}
	catch (const Ice::Exception& ex)
	{
		MessageBox(_T(ex.ice_name().c_str()),"connect error");
		return;
	}

	int size = storages.size();
	
	for(int j = 0; j < size; j ++)
	{ 		   
		::SendMessage(GetDlgItem(IDC_COMBO11), CB_ADDSTRING,0, (LPARAM)TEXT(storages[j].netId.c_str()));
	}
	storages.clear();
   ::TianShanIce::AccreditedPath::Streamers streamers;
	try
	{	 
	  streamers = m_client->listStreamers();
	}
	catch (const Ice::Exception& ex)
	{
		MessageBox(_T(ex.ice_name().c_str()),"connect error");
		return;
	}
	 
	size = streamers.size();
	   
	for( j = 0; j < size; j ++)
	{ 
		
		::SendMessage(GetDlgItem(IDC_COMBO22), CB_ADDSTRING,0, (LPARAM)TEXT(streamers[j].netId.c_str()));
	}
	streamers.clear();
	
	m_schema.clear();
	SendMessage( GetDlgItem(IDC_STORAGELINKCOMBO),CB_SETCURSEL,(WPARAM) 0, 0);
	char strCombo[100];	
	std::string strType;	
	SendMessage( GetDlgItem(IDC_STORAGELINKCOMBO) ,CB_GETLBTEXT, 0,(LPARAM)TEXT(strCombo));
	strType = strCombo;
	
	try
	{	 
      m_schema = m_client->getStorageLinkSchema(strType);
	}
	catch (const Ice::Exception& ex)
	{
		MessageBox(_T(ex.ice_name().c_str()),"connect error");
		return;
	}
    
	ListCtrlAddData();

	SendMessage( GetDlgItem(IDC_COMBO11),CB_SETCURSEL,(WPARAM) 0, 0);
	SendMessage( GetDlgItem(IDC_COMBO22),CB_SETCURSEL,(WPARAM) 0, 0);
	m_Pvals.clear();
}
void CAddDataDlg::TabStreamerLink()
{
    ::EnableWindow(GetDlgItem(IDC_COMBO11), TRUE);
	::EnableWindow(GetDlgItem(IDC_COMBO22), TRUE);
	::EnableWindow(GetDlgItem(IDC_STREAMERLINKCOMBO), TRUE);
	

	::ShowWindow(GetDlgItem(IDC_UK_EDIT),SW_HIDE);
    ::ShowWindow(GetDlgItem(IDC_EP_EDIT),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_DESC_EDIT),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_LIST),SW_SHOW);
    ::ShowWindow(GetDlgItem(IDC_STORAGE_COMBO),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC2),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC3),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC4),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC5),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC15),SW_HIDE);

	::ShowWindow(GetDlgItem(IDC_GETSTORAGEDATA),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_GETSTREAMERDATA),SW_HIDE);
	
	::ShowWindow(GetDlgItem(IDC_UPSTORAGEBTN),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_UPSTREAMERBTN),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_UPSGBTN),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_LINKSTORAGEBTN),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_LINKSTREAMERBTN),SW_SHOW);
	
    ::ShowWindow(GetDlgItem(IDC_STREAMER_COMBO), SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC6),SW_HIDE);
	
	::ShowWindow(GetDlgItem(IDC_SGID_EDIT), SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_SGDESC_EDIT), SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC7), SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC8), SW_HIDE);
	
	::ShowWindow(GetDlgItem(IDC_STATIC9),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC10),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC11),SW_SHOW);
	
	::ShowWindow(GetDlgItem(IDC_COMBO11), SW_SHOW);	
    ::ShowWindow(GetDlgItem(IDC_COMBO22), SW_SHOW);	
    ::ShowWindow(GetDlgItem(IDC_STORAGELINKCOMBO), SW_HIDE);	
	::ShowWindow(GetDlgItem(IDC_STREAMERLINKCOMBO), SW_SHOW);	
	
	::ShowWindow(GetDlgItem(IDC_STATIC13),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_STATIC14),SW_SHOW);

	::ShowWindow(GetDlgItem(IDC_UPSTORAGELINKBTN),SW_HIDE);
    ::ShowWindow(GetDlgItem(IDC_UPSTREAMERLINKBTN),SW_HIDE);
	
	
    DeleteComboItem(IDC_COMBO11);
    DeleteComboItem(IDC_COMBO22);	
	::TianShanIce::AccreditedPath::Streamers streamers;
	try
	{	 
      streamers  = m_client->listStreamers();
	}
	catch (const Ice::Exception& ex)
	{
		MessageBox(_T(ex.ice_name().c_str()),"connect error");
		return;
	}
 
	int size = streamers.size();
	char temp[10];
	
	for( int j = 0; j < size; j ++)
	{ 		
		::SendMessage(GetDlgItem(IDC_COMBO11), CB_ADDSTRING,0, (LPARAM)TEXT(streamers[j].netId.c_str()));
	}
	streamers.clear();

	::TianShanIce::AccreditedPath::ServiceGroups servicegroups;
	try
	{	 
	  servicegroups = m_client->listServiceGroups();
	}
	catch (const Ice::Exception& ex)
	{
		MessageBox(_T(ex.ice_name().c_str()),"connect error");
		return;
	}
 
	size = servicegroups.size();
	   
	for(j = 0; j < size; j ++)
	{  		
		::SendMessage(GetDlgItem(IDC_COMBO22), CB_ADDSTRING,0, (LPARAM)TEXT(itoa(servicegroups[j].id, temp, 10)));
	}
	servicegroups.clear();

	m_schema.clear();
	SendMessage( GetDlgItem(IDC_STREAMERLINKCOMBO),CB_SETCURSEL,(WPARAM) 0, 0);

	char strCombo[100];	
	std::string strType;
	
	SendMessage( GetDlgItem(IDC_STREAMERLINKCOMBO) ,CB_GETLBTEXT, 0,(LPARAM)TEXT(strCombo));
	strType = strCombo;

	try
	{	 
    	m_schema = m_client->getStreamLinkSchema(strType);
	}
	catch (const Ice::Exception& ex)
	{
		MessageBox(_T(ex.ice_name().c_str()),"connect error");
		return;
	}
    
	ListCtrlAddData();
    
    SendMessage( GetDlgItem(IDC_COMBO11),CB_SETCURSEL,(WPARAM) 0, 0);
	SendMessage( GetDlgItem(IDC_COMBO22),CB_SETCURSEL,(WPARAM) 0, 0);
	m_Pvals.clear();
}
LRESULT CAddDataDlg::OnSelchangeTab(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	// TODO : Add Code for control notification handler.
	::ShowWindow(m_listEdit.m_hWnd,SW_HIDE);
	
	DWORD dwState = TabCtrl_GetCurSel(m_hTabAction);
	
	switch (dwState)
	{
	case PPG_UPSTORAGE:
		TabStorage();
		break;
	case PPG_UPSTREAMER:
		TabStreamer();
		break;
	case PPG_UPSG:
		TabServiceGroup();
		break;
	case PPG_LINKSTORAGE:
		TabStorageLink();
		break;
	case PPG_LINKSTREAMER:
		TabStreamerLink();
	default:
		break;
	}
	return 0;
}
void CAddDataDlg::InitListCtrl()
{
	ListView_SetExtendedListViewStyle(m_hListCtrl,LVS_EX_FULLROWSELECT| LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);
	::GetWindowLong(m_hListCtrl,GWL_EXSTYLE | GWL_EXSTYLE & LVS_EX_FULLROWSELECT);
	
	LV_COLUMN lvCol;
	lvCol.mask = LVCF_TEXT | LVCF_WIDTH |LVIF_STATE;
	lvCol.fmt = LVCFMT_LEFT;
	lvCol.iSubItem = 0;
	
	ListView_DeleteColumn(m_hListCtrl,0);
	lvCol.cx = 200;
	lvCol.pszText = "PrivateData";
	ListView_InsertColumn(m_hListCtrl,0,&lvCol);
	
	lvCol.cx = 150;
	lvCol.pszText = "Value";
	ListView_InsertColumn(m_hListCtrl,1,&lvCol);	
}
void CAddDataDlg::DeleteComboItem(UINT nComboID)
{
	UINT ncount = ::SendMessage(GetDlgItem(nComboID),  CB_GETCOUNT, 0, 0);
	for(UINT i = 0; i < ncount; i++)
	{
		int j = ::SendMessage(GetDlgItem(nComboID),  CB_DELETESTRING, ncount - i -1, 0); 
	}
}

LRESULT CAddDataDlg::OnClickedlinkstoragebtn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	// TODO : Add Code for control notification handler.
	std::string  StoragenetId = " ", StreamernetId = " ";
	std::string strType;
	char temp[50] = " ";
	
	int nPos = ::SendMessage(GetDlgItem(IDC_COMBO11), CB_GETCURSEL,0, 0);
	if(nPos < 0 )
	{
     MessageBox("StorageNetid is not empty!");
	  return 0;
	}
    ::SendMessage(GetDlgItem(IDC_COMBO11), CB_GETLBTEXT,nPos,(LPARAM)TEXT(temp));
	if(temp[0] == '\0')
	{
	  MessageBox("StorageNetid is not empty!");
	  return 0;
	}
	StoragenetId = temp;

	if(!Delblank(temp))
	{
	  MessageBox("StorageNetid is not empty!");
	  return 0;
	}
    nPos = ::SendMessage(GetDlgItem(IDC_COMBO22), CB_GETCURSEL,0, 0);
	if(nPos < 0 )
	{
      MessageBox("StreamerNetid is not empty!");
	  return 0;
	}
    ::SendMessage(GetDlgItem(IDC_COMBO22), CB_GETLBTEXT,nPos,(LPARAM)TEXT(temp));

	if(temp[0] == '\0')
	{
	  MessageBox("StreamerNetid is not empty!");
	  return 0;
	}

	StreamernetId = temp;

	if(!Delblank(temp))
	{
	  MessageBox("StreamerNetid is not empty!");
	  return 0;
	}
    nPos = ::SendMessage(GetDlgItem(IDC_STORAGELINKCOMBO), CB_GETCURSEL,0, 0);
    ::SendMessage(GetDlgItem(IDC_STORAGELINKCOMBO), CB_GETLBTEXT,nPos,(LPARAM)TEXT(temp));
	strType = temp;

	if(!SetPrivateData())
		return 0;
	try
	{
       m_client->linkStorage(StoragenetId,StreamernetId, strType, m_Pvals);
	}
	catch(::TianShanIce::InvalidParameter& ex)
	{
		MessageBox(ex.message.c_str());
		return 0;
	}	
	catch (const Ice::Exception& ex)
	{
		MessageBox(_T(ex.ice_name().c_str()),"linkStorage error");
		return 0;
	}

    MessageBox("linkStorage success!","Link");
	TabStorageLink();
	return 0;
}

LRESULT CAddDataDlg::OnClickedLinkstreamerbtn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	// TODO : Add Code for control notification handler.
	int nSGID;
	std::string StreamernetId = " ";
	std::string strType;
	char temp[50]= " ";
	
	int nPos = ::SendMessage(GetDlgItem(IDC_COMBO11), CB_GETCURSEL,0, 0);
	if(nPos < 0)
	{
      MessageBox("StreamerNetid is not empty!");
	  return 0;
	}
    ::SendMessage(GetDlgItem(IDC_COMBO11), CB_GETLBTEXT,nPos,(LPARAM)TEXT(temp));

	if(temp[0] == '\0')
	{
	  MessageBox("StreamerNetid is not empty!");
	  return 0;
	}

	StreamernetId = temp;
	if(!Delblank(temp))
	{
	  MessageBox("StreamerNetid is not empty!");
	  return 0;
	}
    nPos = ::SendMessage(GetDlgItem(IDC_COMBO22), CB_GETCURSEL,0, 0);
	if(nPos < 0)
	{
      MessageBox("SGID is not empty!");
	  return 0;
	}
    ::SendMessage(GetDlgItem(IDC_COMBO22), CB_GETLBTEXT,nPos,(LPARAM)TEXT(temp));

	if(temp[0] == '\0')
	{
	  MessageBox("SGID is not empty!");
	  return 0;
	}

	nSGID = atoi(temp);

	if(!Delblank(temp))
	{
	  MessageBox("SGID is not empty!");
	  return 0;
	}
    nPos = ::SendMessage(GetDlgItem(IDC_STREAMERLINKCOMBO), CB_GETCURSEL,0, 0);
    ::SendMessage(GetDlgItem(IDC_STREAMERLINKCOMBO), CB_GETLBTEXT,nPos,(LPARAM)TEXT(temp));
	strType = temp;
		
	if(!SetPrivateData())
		return 0;
	 try
	 {
      ::TianShanIce::AccreditedPath::StreamLinkPrx stre = m_client->linkStreamer(nSGID, StreamernetId, strType, m_Pvals);
	 }
	 catch(::TianShanIce::InvalidParameter& ex)
	 {
		 MessageBox(ex.message.c_str());
		 return 0;
	 }	
	 catch (const Ice::Exception& ex)
	 {
		 MessageBox(_T(ex.ice_name().c_str()),"linkStreamer error");
		 return 0;
	 }
    MessageBox("linkStreamer success!","Link");

    TabStreamerLink();
	return 0;
}

LRESULT CAddDataDlg::OnClickedUpsgbtn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	// TODO : Add Code for control notification handler.
	int nId;
	char temp[100];
	std::string strDesc;
	
	::GetWindowText(GetDlgItem(IDC_SGID_EDIT),temp,20 );
	nId = atoi(temp);
	if(!Delblank(temp))
	{
	  MessageBox("ServiceGroupID is not empty!");
	  return 0;
	}
	
    ::GetWindowText(GetDlgItem(IDC_SGDESC_EDIT),temp,100 );
	strDesc = temp;
	try
	{	 
	 m_client->updateServiceGroup(nId, strDesc);
	}
	catch (const Ice::Exception& ex)
	{
		MessageBox(_T(ex.ice_name().c_str()),"connect error");
		return 0;
	}

	
	MessageBox("updateServiceGroup success!","Update");
	
	::SetWindowText(GetDlgItem(IDC_SGID_EDIT), "");
	::SetWindowText(GetDlgItem(IDC_SGDESC_EDIT),"");
	::EnableWindow(GetDlgItem(IDC_SGID_EDIT), TRUE);
	return 0;
}

LRESULT CAddDataDlg::OnClickedUpstoragebtn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	// TODO : Add Code for control notification handler.
	char temp[100];
	std::string strType, strUK, strEP, strDesc;
	
	int nPos = ::SendMessage(GetDlgItem(IDC_STORAGE_COMBO), CB_GETCURSEL,0, 0);
    ::SendMessage(GetDlgItem(IDC_STORAGE_COMBO), CB_GETLBTEXT,nPos,(LPARAM)TEXT(temp));
	strType = temp;

	::GetWindowText(GetDlgItem(IDC_EP_EDIT),temp,100 );
    strEP = temp;

	if(!Delblank(temp))
	{
	  MessageBox("EndPoint is not empty!");
	  return 0;
	}
	
	::GetWindowText(GetDlgItem(IDC_UK_EDIT),temp,100 );
	strUK = temp;

	if(!Delblank(temp))
	{
	  MessageBox("NetId is not empty!");
	  return 0;
	}
	

	::GetWindowText(GetDlgItem(IDC_DESC_EDIT),temp,100 );
    strDesc = temp;
	try
	{	 
    	m_client->updateStorage(strUK, strType,strEP, strDesc);
	}
	catch (const Ice::Exception& ex)
	{
		MessageBox(_T(ex.ice_name().c_str()),"connect error");
		return 0;
	}


/*	if(!SetPrivateData())
		return 0;*/

	 ::TianShanIce::ValueMap::iterator pos = m_Pvals.begin();

   
	 try
	 {	 
		 while(pos != m_Pvals.end())
		 { 
			 m_client->setStreamerPrivateData(strUK, (*pos).first,(*pos).second);	 
			 pos++;
		 }
	 }
	 catch (const Ice::Exception& ex)
	 {
		 MessageBox(_T(ex.ice_name().c_str()),"connect error");
		 return 0;
	 }

	MessageBox("updateStorage success!","Update");
	::SetWindowText(GetDlgItem(IDC_EP_EDIT), "");
	::SetWindowText(GetDlgItem(IDC_DESC_EDIT),"");
	::SetWindowText(GetDlgItem(IDC_UK_EDIT),"");
	::EnableWindow(GetDlgItem(IDC_UK_EDIT), TRUE);

	LCAddPrivateData("StoragePrivateKeys");	
	return 0;
}
LRESULT CAddDataDlg::OnClickedUpstreamerbtn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	// TODO : Add Code for control notification handler.
	char temp[100];
	std::string strType, strUK, strEP, strDesc;
	
	int nPos = ::SendMessage(GetDlgItem(IDC_STREAMER_COMBO), CB_GETCURSEL,0, 0);
    ::SendMessage(GetDlgItem(IDC_STREAMER_COMBO), CB_GETLBTEXT,nPos,(LPARAM)TEXT(temp));
	strType = temp;
	
	::GetWindowText(GetDlgItem(IDC_EP_EDIT),temp,100 );
    strEP = temp;

	if(!Delblank(temp))
	{
	  MessageBox("EndPoint is not empty!");
	  return 0;
	}

	::GetWindowText(GetDlgItem(IDC_UK_EDIT),temp,100 );
	strUK = temp;

	if(!Delblank(temp))
	{
	  MessageBox("NetId is not empty!");
	  return 0;
	}

	::GetWindowText(GetDlgItem(IDC_DESC_EDIT),temp,100 );
    strDesc = temp;
	try
	{	 
     	m_client->updateStreamer(strUK, strType, strEP, strDesc);
	}
	catch (const Ice::Exception& ex)
	{
		MessageBox(_T(ex.ice_name().c_str()),"connect error");
		return 0;
	}


/*	if(!SetPrivateData())
		return 0;*/

   ::TianShanIce::ValueMap::iterator pos = m_Pvals.begin();

    
   try
   {	 
	   while(pos != m_Pvals.end())
	   {
		   m_client->setStreamerPrivateData(strUK, (*pos).first,(*pos).second);
		   pos++;
	   }
   }
	catch (const Ice::Exception& ex)
	{
		MessageBox(_T(ex.ice_name().c_str()),"connect error");
		return 0;
	}

	MessageBox("updateStreamer success!","Update");
	
	::SetWindowText(GetDlgItem(IDC_EP_EDIT), "");
	::SetWindowText(GetDlgItem(IDC_DESC_EDIT),"");
	::SetWindowText(GetDlgItem(IDC_UK_EDIT),"");
	::EnableWindow(GetDlgItem(IDC_UK_EDIT), TRUE);

	LCAddPrivateData("StreamerPrivateKeys");
	return 0;
}
BOOL CAddDataDlg::OpenConfigfile()
{
	struct _stat buf;
	int  result; 
	
/*	result = _stat("D:\\ZQPROJS\\TianShan\\bin\\TianShanDefines.cpp", &buf);
	
	m_fp = fopen("D:\\ZQPROJS\\TianShan\\bin\\TianShanDefines.cpp", "rb");*/


	result = _stat("TianShanDefines.cpp", &buf);
	
	m_fp = fopen("TianShanDefines.cpp", "rb");
	
	if(m_fp == NULL || result == -1)
	{
		MessageBox("open configfile  failure!");
		return FALSE;
	}
	
	m_nfilesize = buf.st_size;

	return TRUE;
}

void CAddDataDlg::InitCombox(UINT nID, char *pFindString)
{
    char seps[]   = ",\t\r\n\"[]={}";
	char *p, *presult;
	char *strtemp ;
	
	strtemp = new char[m_nfilesize];
	
	if(!strtemp)
		return;
	fseek( m_fp, 0, SEEK_SET );
	fread(strtemp, m_nfilesize, 1, m_fp);
	
    p = strstr(strtemp, pFindString);
	
	if(p == NULL)
	{
		return;
	}
	presult = strtok(p,seps);	
	
	for(presult = strtok(NULL,seps); strcmp(presult, " NULL") != 0; presult = strtok(NULL,seps))
	{
		if( *presult == ' ')
			continue;
		::SendMessage(GetDlgItem(nID), CB_ADDSTRING,0, (LPARAM)TEXT(presult));
	}
	SendMessage( GetDlgItem(nID),CB_SETCURSEL,(WPARAM) 0, 0);	
	
	delete []strtemp;
}

void CAddDataDlg::LCAddPrivateData(char *pFindString)
{
	char seps[]   = ",\t\r\n\"[]={}";
	char *p, *presult;
	char *strtemp ;
	int i = -1;
	
	LVITEM lvItem;
	lvItem.mask = LVIF_TEXT | LVIF_IMAGE; 
	lvItem.state = 0;
	lvItem.iSubItem = 0;
	ListView_DeleteAllItems(m_hListCtrl);
	
	strtemp = new char[m_nfilesize];
	
	if(!strtemp)
		return;
	
	fseek( m_fp, 0, SEEK_SET);
	fread(strtemp, m_nfilesize, 1, m_fp);
	
    p = strstr(strtemp, pFindString);
	
	if(p == NULL)
	{
		return;
	}
	
	presult = strtok(p,seps);
	m_Pvals.clear();
	::TianShanIce::Variant var;
	std::string strkey;
	for(presult = strtok(NULL,seps); strcmp(presult, " NULL") != 0; presult = strtok(NULL,seps))
	{
		if( *presult == ' ')
			continue;
		//key:string
		lvItem.iItem = ++i;
		lvItem.pszText =(LPSTR)presult; 
		ListView_InsertItem(m_hListCtrl, &lvItem);
        strkey = presult;
		// variant: type
		presult = strtok(NULL,seps);
        VariantType(presult, &var);
		//default string
		presult = strtok(NULL,seps);
		if(*presult == ' ')
			presult = strtok(NULL,seps);
		ListView_SetItemText(m_hListCtrl,i,1,(LPSTR)presult);
		m_Pvals.insert(::TianShanIce::ValueMap::value_type (strkey, var));
	}
	delete []strtemp;
}

void CAddDataDlg::VariantType(const char *pstr ,::TianShanIce::Variant *pvar)
{
	while(*pstr == ' ')
		pstr++;
	if(!strcmp(pstr,"vtBin"))
		pvar->type  = ::TianShanIce::vtBin;
	
    if(!strcmp(pstr,"vtInts"))
		pvar->type = ::TianShanIce::vtInts;
	
	if(!strcmp(pstr,"vtLongs"))
		pvar->type = ::TianShanIce::vtLongs;
	
	if(!strcmp(pstr,"vtStrings"))
		pvar->type = ::TianShanIce::vtStrings;
	
}

BOOL CAddDataDlg::SetPrivateData()
{
	char temp[150], strcheck[150],*pstr;
	std::string strKey;
	::TianShanIce::Variant var;
	int nItemCount = ListView_GetItemCount(m_hListCtrl);

	for(int i = 0; i < nItemCount; i++)
	{
		ListView_GetItemText(m_hListCtrl,i,0, temp, 150);
		strKey = temp;
		ListView_GetItemText(m_hListCtrl,i,1, temp, 150);
        strcpy(strcheck, temp);
	    pstr = Delblank(strcheck);
		if(m_schema[i].optional == TRUE)
		{  
			if(pstr == NULL)
			{	
				strKey = strKey + " is not empty!!";
				MessageBox(strKey.c_str());
				return FALSE;
			}
		}
		else
		{
			if(!CheckPrivateData(strKey, m_schema[i].defaultvalue.type, temp,&var))
			{
				strKey = strKey + " : data format is error!!";
				MessageBox(strKey.c_str());
				return FALSE;
			}	
			m_Pvals[strKey] = var;
		}
		var.strs.clear();
		var.ints.clear();
		var.lints.clear();
		var.bin.clear();
	}
	return TRUE;
}
void CAddDataDlg::UpdateListCtrlData()
{
	LVITEM lvItem;
	lvItem.mask = LVIF_TEXT | LVIF_IMAGE; 
	lvItem.state = 0;
	lvItem.iSubItem = 0;
    ListView_DeleteAllItems(m_hListCtrl);
	char strText[150];
	char *key;
    ::TianShanIce::ValueMap::iterator pos;

    for(UINT i = 0; i < m_Pvals.size(); i++)
	{
     pos =  m_Pvals.find(m_schema[i].keyname);
	 if(!GetPrivateDataStr(strText,&(pos->second)))
	 {
		 MessageBox("GetPrivateDataStr error!!");
			 return;	
	 } 
	 lvItem.iItem = i;
	 key = (LPSTR) pos->first.c_str();	  
	 lvItem.pszText =(LPSTR)key; 
	 ListView_InsertItem(m_hListCtrl, &lvItem);	 
	 ListView_SetItemText(m_hListCtrl,i,1,(LPSTR)strText);
	}
}

void CAddDataDlg::UpLinkTab()
{
  	char temp[20];
	int npos;
	m_Pvals.clear();
	if(m_linkinfo.bStorage)//update storagelink
	{	
	    ::ShowWindow(GetDlgItem(IDC_LINKSTORAGEBTN),SW_HIDE);
	    ::ShowWindow(GetDlgItem(IDC_UPSTORAGELINKBTN),SW_SHOW);
       
		 npos = SendMessage( GetDlgItem(IDC_COMBO11) ,CB_FINDSTRING, 0,(LPARAM)(LPCSTR)m_linkinfo.storageNetId.c_str());
		 if(npos >= 0)
		    ::SendMessage( GetDlgItem(IDC_COMBO11),CB_SETCURSEL,(WPARAM) npos, 0);

		 npos = SendMessage( GetDlgItem(IDC_COMBO22) ,CB_FINDSTRING, 0,(LPARAM)(LPCSTR)m_linkinfo.streamerNetId.c_str());
		 if(npos >= 0)
		    ::SendMessage( GetDlgItem(IDC_COMBO22),CB_SETCURSEL,(WPARAM) npos, 0);

         npos = SendMessage( GetDlgItem(IDC_STORAGELINKCOMBO) ,CB_FINDSTRING, 0,(LPARAM)(LPCSTR)m_linkinfo.linktype.c_str());
		 if(npos >= 0)
			 ::SendMessage( GetDlgItem(IDC_STORAGELINKCOMBO),CB_SETCURSEL,(WPARAM) npos, 0);
		 
		 ::EnableWindow(GetDlgItem(IDC_STORAGELINKCOMBO), FALSE);
		 
		 m_Pvals = m_linkinfo.storagelinkPrx ->getPrivateData();
		 
		 m_schema.clear();
		 int npos = SendMessage( GetDlgItem(IDC_STREAMERLINKCOMBO) ,CB_GETCURSEL, 0,0);
		 if(npos >=0 )
		 {
			 char strCombo[100];	
			 std::string strType;
			 SendMessage( GetDlgItem(IDC_STORAGELINKCOMBO) ,CB_GETLBTEXT, npos,(LPARAM)TEXT(strCombo));
			 strType = strCombo;
			 try
			 {	 
				 m_schema = m_client->getStorageLinkSchema(strType);
			 }
			 catch (const Ice::Exception& ex)
			 {
				 MessageBox(_T(ex.ice_name().c_str()),"connect error");
				 return;
			 }		
		 }
	}
	else
	{
        ::ShowWindow(GetDlgItem(IDC_LINKSTREAMERBTN),SW_HIDE);
	    ::ShowWindow(GetDlgItem(IDC_UPSTREAMERLINKBTN),SW_SHOW);
       
		 npos = SendMessage( GetDlgItem(IDC_COMBO11) ,CB_FINDSTRING, 0,(LPARAM)(LPCSTR)m_linkinfo.streamerNetId.c_str());
		 if(npos >= 0)
		    ::SendMessage( GetDlgItem(IDC_COMBO11),CB_SETCURSEL,(WPARAM) npos, 0);

		 npos = SendMessage( GetDlgItem(IDC_COMBO22) ,CB_FINDSTRING, 0,(LPARAM)(LPCSTR)itoa(m_linkinfo.SGId, temp, 10));
		 if(npos >= 0)
		    ::SendMessage( GetDlgItem(IDC_COMBO22),CB_SETCURSEL,(WPARAM) npos, 0);

         npos = SendMessage( GetDlgItem(IDC_STREAMERLINKCOMBO) ,CB_FINDSTRING, 0,(LPARAM)(LPCSTR)m_linkinfo.linktype.c_str());
		 if(npos >= 0)
		    ::SendMessage( GetDlgItem(IDC_STREAMERLINKCOMBO),CB_SETCURSEL,(WPARAM) npos, 0);

        ::EnableWindow(GetDlgItem(IDC_STREAMERLINKCOMBO), FALSE);

		m_Pvals = m_linkinfo.streamlinkprx->getPrivateData();
		
		m_schema.clear();
		int npos = SendMessage( GetDlgItem(IDC_STREAMERLINKCOMBO) ,CB_GETCURSEL, 0,0);
		if(npos >=0 )
		{
			char strCombo[100];	
			std::string strType;
			SendMessage( GetDlgItem(IDC_STREAMERLINKCOMBO) ,CB_GETLBTEXT, npos,(LPARAM)TEXT(strCombo));
			strType = strCombo;
				try
				{	 
					m_schema = m_client->getStreamLinkSchema(strType);
				}
				catch (const Ice::Exception& ex)
				{
					MessageBox(_T(ex.ice_name().c_str()),"connect error");
					return;
				
				}
		}

	}
      
	::EnableWindow(GetDlgItem(IDC_COMBO11), FALSE);
	::EnableWindow(GetDlgItem(IDC_COMBO22), FALSE);
    
	UpdateListCtrlData();
}

void CAddDataDlg::InitStorageLinkType()
{
	 ::TianShanIce::StrValues strvalues;
	try
	{	 
      strvalues = m_client->listSupportedStorageLinkTypes();
	}
	catch (const Ice::Exception& ex)
	{
		MessageBox(_T(ex.ice_name().c_str()),"connect error");
		return;
	}

   for(int i = 0; i < strvalues.size(); i++)
   {
   	::SendMessage(GetDlgItem(IDC_STORAGELINKCOMBO), CB_ADDSTRING,i, (LPARAM)TEXT(strvalues[i].c_str()));
   }
   SendMessage( GetDlgItem(IDC_STORAGELINKCOMBO),CB_SETCURSEL,(WPARAM) 0, 0);
}

LRESULT CAddDataDlg::OnSelchangeStoragelinkcombo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	// TODO : Add Code for control notification handler.
	int npos = SendMessage( GetDlgItem(IDC_STORAGELINKCOMBO) ,CB_GETCURSEL, 0,0);
	if(npos >= 0 )
	{
		char strCombo[100]; 
		std::string strType;
		
		SendMessage( GetDlgItem(IDC_STORAGELINKCOMBO) ,CB_GETLBTEXT, npos,(LPARAM)TEXT(strCombo));
		strType = strCombo;
		
		try
		{	 
	    	m_schema = m_client->getStorageLinkSchema(strType);
		}
		catch (const Ice::Exception& ex)
		{
			MessageBox(_T(ex.ice_name().c_str()),"connect error");
			return 0;
		}	
		ListCtrlAddData();
	}
	return 0;
}

void CAddDataDlg::InitStreamerLinkType()
{
   m_schema.clear();
   ::TianShanIce::StrValues strvalues;
   	try
	{	
	 strvalues	= m_client->listSupportedStreamLinkTypes(); 
	}
	catch (const Ice::Exception& ex)
	{
		MessageBox(_T(ex.ice_name().c_str()),"connect error");
		return;
	}
 
   for(int i = 0; i < strvalues.size(); i++)
   {
    ::SendMessage(GetDlgItem(IDC_STREAMERLINKCOMBO), CB_ADDSTRING,i, (LPARAM)TEXT(strvalues[i].c_str()));
   }
   SendMessage( GetDlgItem(IDC_STREAMERLINKCOMBO),CB_SETCURSEL,(WPARAM) 0, 0);
}

LRESULT CAddDataDlg::OnSelchangeStreamerlinkcombo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	// TODO : Add Code for control notification handler.
	m_schema.clear();
	int npos = SendMessage( GetDlgItem(IDC_STREAMERLINKCOMBO) ,CB_GETCURSEL, 0,0);
	if(npos >=0 )
	{
		char strCombo[100];	
		std::string strType;

		SendMessage( GetDlgItem(IDC_STREAMERLINKCOMBO) ,CB_GETLBTEXT, npos,(LPARAM)TEXT(strCombo));
		strType = strCombo;
	try
	{	 
		m_schema = m_client->getStreamLinkSchema(strType);
	}
	catch (const Ice::Exception& ex)
	{
		MessageBox(_T(ex.ice_name().c_str()),"connect error");
		return 0;
	}
	
	ListCtrlAddData();
	}
	return 0;
}

BOOL CAddDataDlg::ListCtrlAddData()
{
   char strValue[100];
	LVITEM lvItem;
	lvItem.mask = LVIF_TEXT | LVIF_IMAGE; 
	lvItem.state = 0;
	lvItem.iSubItem = 0;
	ListView_DeleteAllItems(m_hListCtrl);

	for(int i = 0; i < m_schema.size(); i ++)
	{
		if(!GetPrivateDataStr(strValue, &m_schema[i].defaultvalue))
		{
		  return FALSE;
		}
		lvItem.iItem = i;
		lvItem.pszText =(LPSTR)m_schema[i].keyname.c_str(); 
		ListView_InsertItem(m_hListCtrl, &lvItem);
		ListView_SetItemText(m_hListCtrl,i,1,(LPSTR)strValue);
	} 
	return TRUE;
}

