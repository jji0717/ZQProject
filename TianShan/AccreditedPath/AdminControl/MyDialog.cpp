// MyDialog.cpp: implementation of the CAddDataDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MyDialog.h"
#include "resource.h"
#include <sys/types.h>
#include <sys/stat.h>

CString m_strConstoreEndpoint;
CString m_strStreamSmithEndPoint;

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

void  CListEdit::SetCtrlData(DWORD dwData)
{
	m_dwData = dwData;
}

DWORD CListEdit::GetCtrlData()
{
    return m_dwData;
}

LRESULT CListEdit::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// TODO : Add Code for message handler. Call DefWindowProc if necessary.
	 HWND pParent = ::GetParent(m_hWnd);
	::PostMessage(pParent,WM_USER_EDIT_END,m_bExchange,0);
	DefWindowProc(uMsg,wParam,lParam);
	return 0;	
}

LRESULT CListEdit::OnKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	int iVirkeyCode = (int)wParam;
	if ( iVirkeyCode == VK_RETURN)
	{	
		HWND pParent = ::GetParent(m_hWnd);
		::PostMessage(pParent,WM_USER_EDIT_END,m_bExchange,0);
		DefWindowProc(uMsg,wParam,lParam);
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// class CAddDataDlg
CAddDataDlg::CAddDataDlg() 
{
	m_StreamerClient = NULL;
	m_ContentClient  = NULL;
	m_client = NULL;
	m_ic = NULL;
	m_businessClient = NULL;
}

CAddDataDlg::CAddDataDlg(Ice::CommunicatorPtr ic,TianShanIce::Transport::PathAdminPrx client, TianShanIce::Site::SiteAdminPrx businessClient,int nflag, SData data)
{
	m_client = client;
	m_businessClient = businessClient;
    m_hflag = nflag;
	m_sData = data;
	m_linkinfo.bStorage = -1;
	m_ic = ic;
	m_StreamerClient = NULL;
	m_ContentClient  = NULL;
}

CAddDataDlg::CAddDataDlg(Ice::CommunicatorPtr ic,TianShanIce::Transport::PathAdminPrx client, TianShanIce::Site::SiteAdminPrx businessClient,int nflag, LinkInfo linkinfo)
{
	m_client = client;
	m_businessClient = businessClient;
	m_hflag = nflag;
	m_linkinfo = linkinfo;
	m_ic = ic;
	m_StreamerClient = NULL;
	m_ContentClient  = NULL;
}

CAddDataDlg::~CAddDataDlg()
{
	m_StreamerClient = NULL;
	m_ContentClient  = NULL;
	m_client = NULL;
	m_ic = NULL;
}

void CAddDataDlg::InitTabAction(int iFlag)
{   
	TCITEM tcitem = {0};
	tcitem.mask = TCIF_TEXT | TCIF_STATE | TCIF_PARAM ;
	
	switch ( iFlag)
	{
	case PPG_UPSTORAGE:
		tcitem.pszText = "Storage";
		TabCtrl_InsertItem(m_hTabAction, PPG_UPSTORAGE, &tcitem);
		break;
	case PPG_UPSTREAMER:
		tcitem.pszText = "Streamer";
		TabCtrl_InsertItem(m_hTabAction, PPG_UPSTREAMER, &tcitem );
		break;
	case PPG_UPSG:
		tcitem.pszText = "ServiceGroup";
		TabCtrl_InsertItem(m_hTabAction, PPG_UPSG, &tcitem);	
		break;
	case PPG_LINKSTORAGE:
		tcitem.pszText = "Storage Link";
		TabCtrl_InsertItem(m_hTabAction, PPG_LINKSTORAGE, &tcitem);
		break;
	case PPG_LINKSTREAMER:
		tcitem.pszText = "Streamer Link";
		TabCtrl_InsertItem(m_hTabAction, PPG_LINKSTREAMER, &tcitem);
		break;
	case PPG_UPDATESITE:
		tcitem.pszText = "Site";
		TabCtrl_InsertItem(m_hTabAction, PPG_UPDATESITE, &tcitem);
		break;
	case PPG_UPDATEAPP:
		tcitem.pszText = "App";
		TabCtrl_InsertItem(m_hTabAction, PPG_UPDATEAPP, &tcitem);
		break;
	case PPG_MOUNTAPP:
		tcitem.pszText = "M/UountApp";
		TabCtrl_InsertItem(m_hTabAction, PPG_MOUNTAPP, &tcitem);
		break;
	default:
		break;
	}
}

void CAddDataDlg::TabStorage(std::string strName)
{
	::ShowWindow(GetDlgItem(IDC_PATH_EDIT),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STREAMERID_COMBO),SW_HIDE);
	
	if ( strName =="Storage")
	{
		::ShowWindow(GetDlgItem(IDC_LIST),SW_SHOW);
		::ShowWindow(GetDlgItem(IDC_GETSTORAGEDATA),SW_SHOW);
		::ShowWindow(GetDlgItem(IDC_STORAGE_COMBO),SW_SHOW);
		::SetWindowText(GetDlgItem(IDC_STATIC4),_T("EndPoint:"));
		::SetWindowText(GetDlgItem(IDC_STATIC3),_T("StorageNetId:"));
		::ShowWindow(GetDlgItem(IDC_STATIC2),SW_SHOW);
		::SetWindowText(GetDlgItem(IDC_UPSTORAGEBTN),_T("Update Storage"));
	}
	else
	{
		::ShowWindow(GetDlgItem(IDC_LIST),SW_HIDE);
		::ShowWindow(GetDlgItem(IDC_GETSTORAGEDATA),SW_HIDE);
		::ShowWindow(GetDlgItem(IDC_STORAGE_COMBO),SW_HIDE);
		::SetWindowText(GetDlgItem(IDC_STATIC4),_T("Name:"));
		::SetWindowText(GetDlgItem(IDC_STATIC3),_T("EndPoint:"));
		::ShowWindow(GetDlgItem(IDC_STATIC2),SW_HIDE);
		::SetWindowText(GetDlgItem(IDC_UPSTORAGEBTN),_T("Update App"));
	}

	::ShowWindow(GetDlgItem(IDC_UK_EDIT),SW_SHOW);
    ::ShowWindow(GetDlgItem(IDC_EP_EDIT),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_DESC_EDIT),SW_SHOW);
	
	::ShowWindow(GetDlgItem(IDC_STATIC3),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_STATIC4),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_STATIC5),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_STATIC15),SW_HIDE);

    
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
		
	::SetWindowText(GetDlgItem(IDC_DESC_EDIT),"");
	::SetWindowText(GetDlgItem(IDC_UK_EDIT),"");
	::EnableWindow(GetDlgItem(IDC_UK_EDIT), TRUE);
}

void CAddDataDlg::TabStreamer()
{
	
	::ShowWindow(GetDlgItem(IDC_PATH_EDIT),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_UK_EDIT),SW_HIDE);

	::ShowWindow(GetDlgItem(IDC_STREAMERID_COMBO),SW_SHOW);
    ::ShowWindow(GetDlgItem(IDC_EP_EDIT),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_DESC_EDIT),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_LIST),SW_SHOW);
    ::ShowWindow(GetDlgItem(IDC_STORAGE_COMBO),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STATIC2),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_STATIC3),SW_SHOW);
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
	
//	::SetWindowText(GetDlgItem(IDC_EP_EDIT), "");
	::SetWindowText(GetDlgItem(IDC_DESC_EDIT),"");
	::SetWindowText(GetDlgItem(IDC_UK_EDIT),"");
	::EnableWindow(GetDlgItem(IDC_UK_EDIT), TRUE);
	::SetWindowText(GetDlgItem(IDC_STATIC3),_T("StreamerNetId:"));
}

void CAddDataDlg::TabServiceGroup(std::string strName)
{
	::ShowWindow(GetDlgItem(IDC_PATH_EDIT),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_STREAMERID_COMBO),SW_HIDE);
	if ( strName =="ServiceGroup" )
	{
		ShowSiteControl(FALSE);
		::SetWindowText(GetDlgItem(IDC_STATIC7),_T("ServiceGroup ID:"));
		::SetWindowText(GetDlgItem(IDC_STATIC8),_T("Desc:"));
		::SetWindowText(GetDlgItem(IDC_UPSGBTN),_T("Update SerGroup"));
		::ShowWindow(GetDlgItem(IDC_UPSTREAMERBTN),SW_HIDE);
	}
	else
	{
		if ( m_sData.id == 1)
		{
			ShowSiteControl(FALSE);
			::SetWindowText(GetDlgItem(IDC_STATIC7),_T("Property:"));
			::SetWindowText(GetDlgItem(IDC_STATIC8),_T("Value:"));
			::SetWindowText(GetDlgItem(IDC_UPSGBTN),_T("Set Properties"));
			::SetWindowText(m_hWnd,_T("Set Properties"));
			m_sData.id = 0;
			::ShowWindow(GetDlgItem(IDC_UPSTREAMERBTN),SW_HIDE);
		}
		else
		{
						
			::SetWindowText(GetDlgItem(IDC_STATIC7),_T("Name:"));
			::SetWindowText(GetDlgItem(IDC_STATIC8),_T("Desc:"));
			if ( m_sData.flag)
			{
				::ShowWindow(GetDlgItem(IDC_UPSTREAMERBTN),SW_SHOW);
				::SetWindowText(m_hWnd,_T("Update Site"));
				::SetWindowText(GetDlgItem(IDC_UPSGBTN),_T("Update Site"));
				ShowSiteControl(TRUE);
				::SetWindowText(GetDlgItem(IDC_UPSTREAMERBTN),_T("UpdateSiteResLimit"));
			}
			else
			{
				::ShowWindow(GetDlgItem(IDC_UPSTREAMERBTN),SW_SHOW);
				ShowSiteControl(TRUE);
				::SetWindowText(m_hWnd,_T("Add Site"));
				::SetWindowText(GetDlgItem(IDC_UPSGBTN),_T("Add Site"));
				::SetWindowText(GetDlgItem(IDC_UPSTREAMERBTN),_T("AddSiteResLimited"));
			}
			
		}
	}

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
	::ShowWindow(GetDlgItem(IDC_STREAMERID_COMBO),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_PATH_EDIT),SW_HIDE);
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

	::SetWindowText(m_hWnd,_T("Add StorageLink"));
	::SetWindowText(GetDlgItem(IDC_LINKSTORAGEBTN),_T("Add StorageLink"));
	
	try
	{
	
		TianShanIce::Transport::Storages stores = m_client->listStorages();
		int size = stores.size();
		
		for(int j = 0; j < size; j ++)
		{ 		   
			::SendMessage(GetDlgItem(IDC_COMBO11), CB_ADDSTRING,0, (LPARAM)TEXT(stores[j].netId.c_str()));
		}
		stores.clear();
		
		TianShanIce::Transport::Streamers streamers = m_client->listStreamers();
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
		m_schema = m_client->getStorageLinkSchema(strType);
		ListCtrlAddData();

		SendMessage( GetDlgItem(IDC_COMBO11),CB_SETCURSEL,(WPARAM) 0, 0);
		SendMessage( GetDlgItem(IDC_COMBO22),CB_SETCURSEL,(WPARAM) 0, 0);
		m_Pvals.clear();
	}
	catch(Ice::Exception &e1)
   {
	   ATLTRACE(_T("Ice Exception %s\n"),e1.ice_name().c_str());
	   return ;
   }
   catch (...)
   {
	   return ;
   }
}
void CAddDataDlg::TabStreamerLink(std::string strName)
{
	
	//="StreamerLink"

	::ShowWindow(GetDlgItem(IDC_STREAMERID_COMBO),SW_HIDE);
	::EnableWindow(GetDlgItem(IDC_COMBO11), TRUE);
	
	::ShowWindow(GetDlgItem(IDC_UK_EDIT),SW_HIDE);
    ::ShowWindow(GetDlgItem(IDC_EP_EDIT),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_DESC_EDIT),SW_HIDE);
	
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
		
	::ShowWindow(GetDlgItem(IDC_COMBO11), SW_SHOW);	
    
    ::ShowWindow(GetDlgItem(IDC_STORAGELINKCOMBO), SW_HIDE);	
		
	::ShowWindow(GetDlgItem(IDC_STATIC13),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_STATIC14),SW_SHOW);

	::ShowWindow(GetDlgItem(IDC_UPSTORAGELINKBTN),SW_HIDE);
    ::ShowWindow(GetDlgItem(IDC_UPSTREAMERLINKBTN),SW_HIDE);
	DeleteComboItem(IDC_COMBO11);

	
    
	if ( strName =="StreamerLink")
	{
		::EnableWindow(GetDlgItem(IDC_STREAMERLINKCOMBO), TRUE);
		::ShowWindow(GetDlgItem(IDC_STREAMERLINKCOMBO), SW_SHOW);	
		DeleteComboItem(IDC_STREAMERLINKCOMBO);
		::ShowWindow(GetDlgItem(IDC_STATIC11),SW_SHOW);	

		::ShowWindow(GetDlgItem(IDC_PATH_EDIT),SW_HIDE);
		::SetWindowText(GetDlgItem(IDC_STATIC13),_T("Streamer NetID:"));
		::SetWindowText(GetDlgItem(IDC_STATIC14),_T("ServiceGroup ID:"));
		::SetWindowText(GetDlgItem(IDC_STATIC11),_T("Type:"));
		::SetWindowText(m_hWnd,_T("Add StreamerLink"));
		::SetWindowText(GetDlgItem(IDC_LINKSTREAMERBTN),_T("Add StreamerLink"));
		

		InitStreamerLinkType();
		::EnableWindow(GetDlgItem(IDC_COMBO22), TRUE);
		::ShowWindow(GetDlgItem(IDC_COMBO22), SW_SHOW);	
		DeleteComboItem(IDC_COMBO22);
		::ShowWindow(GetDlgItem(IDC_LIST),SW_SHOW);
		try
		{
			::TianShanIce::Transport::Streamers streamers = m_client->listStreamers();
			int size = streamers.size();
			char temp[20];
			
			for( int j = 0; j < size; j ++)
			{ 		
				::SendMessage(GetDlgItem(IDC_COMBO11), CB_ADDSTRING,0, (LPARAM)TEXT(streamers[j].netId.c_str()));
			}
			streamers.clear();
			
			TianShanIce::Transport::ServiceGroups servicegroups = m_client->listServiceGroups();
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
			m_schema = m_client->getStreamLinkSchema(strType);
			ListCtrlAddData();
    
			SendMessage( GetDlgItem(IDC_COMBO11),CB_SETCURSEL,(WPARAM) 0, 0);
			SendMessage( GetDlgItem(IDC_COMBO22),CB_SETCURSEL,(WPARAM) 0, 0);
			m_Pvals.clear();
		}
		catch(::TianShanIce::InvalidParameter& ex)
		{
			MessageBox(ex.message.c_str());
			return ;
		}	
		catch (const Ice::Exception& ex)
		{
			MessageBox(_T(ex.ice_name().c_str()),"linkStorage error");
			return ;
		}
		catch(...)
		{
			MessageBox(_T("System Exception\n"));
			return;
		}
	}
	else if ( strName =="MountApp") // Mount Application
	{
		::EnableWindow(GetDlgItem(IDC_STREAMERLINKCOMBO), TRUE);
		::ShowWindow(GetDlgItem(IDC_STREAMERLINKCOMBO), SW_SHOW);	
		DeleteComboItem(IDC_STREAMERLINKCOMBO);
		::ShowWindow(GetDlgItem(IDC_STATIC11),SW_SHOW);	

		::ShowWindow(GetDlgItem(IDC_PATH_EDIT),SW_SHOW);
		::SetWindowText(GetDlgItem(IDC_STATIC13),_T("Site:"));
		::SetWindowText(GetDlgItem(IDC_STATIC14),_T("Path:"));
		::SetWindowText(GetDlgItem(IDC_STATIC11),_T("App:"));
		::SetWindowText(GetDlgItem(IDC_LINKSTREAMERBTN),_T("Mount App"));

		::ShowWindow(GetDlgItem(IDC_COMBO22),SW_HIDE);
		::ShowWindow(GetDlgItem(IDC_LIST),SW_HIDE);
		try
		{
			TianShanIce::Site::VirtualSites stores = m_businessClient->listSites();
			int size = stores.size();
			int j;
			for( j = 0; j < size; j ++)
			{ 		
				::SendMessage(GetDlgItem(IDC_COMBO11), CB_ADDSTRING,0, (LPARAM)TEXT(stores[j].name.c_str()));
			}
			SendMessage( GetDlgItem(IDC_COMBO11),CB_SETCURSEL,(WPARAM) 0, 0);
			stores.clear();


			TianShanIce::Site::AppInfos apps = m_businessClient->listApplications();
     		size = apps.size();

		    for( j = 0; j < size; j ++)
			{ 
				::SendMessage(GetDlgItem(IDC_STREAMERLINKCOMBO), CB_ADDSTRING,j, (LPARAM)TEXT(apps[j].name.c_str()));
			}
			SendMessage( GetDlgItem(IDC_STREAMERLINKCOMBO),CB_SETCURSEL,(WPARAM) 0, 0);
			apps.clear();
		}
		catch(::Ice::Exception e)
		{
			ATLTRACE(_T("Call  listSites/application method  failed Ice Exception with error %s"),e.ice_name().c_str());
			return ;
		}
		catch(const TianShanIce::BaseException& ex)
		{
			ATLTRACE(_T("Call  listSites/application method  failed Ice Exception with error %s"),ex.message.c_str());
			return ;
		}
		catch(...)
		{
			ATLTRACE(_T("Call  listSites method  failed  System Exception"));
			return ;
		}
	}
	else // UMount Application
	{
		::ShowWindow(GetDlgItem(IDC_STREAMERLINKCOMBO),SW_HIDE);	
		::ShowWindow(GetDlgItem(IDC_STATIC11),SW_HIDE);	

		::ShowWindow(GetDlgItem(IDC_PATH_EDIT),SW_SHOW);
		::SetWindowText(GetDlgItem(IDC_STATIC13),_T("Site:"));
		::SetWindowText(GetDlgItem(IDC_STATIC14),_T("Path:"));
		::SetWindowText(GetDlgItem(IDC_LINKSTREAMERBTN),_T("UMount App"));

		::ShowWindow(GetDlgItem(IDC_COMBO22),SW_HIDE);
		::ShowWindow(GetDlgItem(IDC_LIST),SW_HIDE);
		try
		{
			
			TianShanIce::Site::VirtualSites stores = m_businessClient->listSites();
		
			int size = stores.size();
			int j;
			for( j = 0; j < size; j ++)
			{ 		
				::SendMessage(GetDlgItem(IDC_COMBO11), CB_ADDSTRING,0, (LPARAM)TEXT(stores[j].name.c_str()));
			}
			SendMessage( GetDlgItem(IDC_COMBO11),CB_SETCURSEL,(WPARAM) 0, 0);
			stores.clear();
		}
		catch(::Ice::Exception e)
		{
			ATLTRACE(_T("Call  listSites/application method  failed Ice Exception with error %s"),e.ice_name().c_str());
			return ;
		}
		catch(const TianShanIce::BaseException& ex)
		{
			ATLTRACE(_T("Call  listSites/application method  failed Ice Exception with error %s"),ex.message.c_str());
			return ;
		}
		catch(...)
		{
			ATLTRACE(_T("Call  listSites method  failed  System Exception"));
			return ;
		}
	}
}

LRESULT CAddDataDlg::OnSelchangeTab(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	// TODO : Add Code for control notification handler.
	::ShowWindow(m_listEdit.m_hWnd,SW_HIDE);
	std::string strName;
	
	DWORD dwState = TabCtrl_GetCurSel(m_hTabAction);
	
	switch (dwState)
	{
	case PPG_UPSTORAGE:
		::SetWindowText(GetDlgItem(IDC_EP_EDIT),"");
		strName ="Storage";
		TabStorage(strName);
		break;
	case PPG_UPSTREAMER:
		::SetWindowText(GetDlgItem(IDC_EP_EDIT),"");
		TabStreamer();
		break;
	case PPG_UPSG:
		strName = "ServiceGroup";
		TabServiceGroup(strName);
		break;
	case PPG_LINKSTORAGE:
		TabStorageLink();
		break;
	case PPG_LINKSTREAMER:
		strName ="StreamerLink";
		TabStreamerLink(strName);
		break;
	case PPG_UPDATESITE:
		strName = "Site";
		TabServiceGroup(strName);
		break;
	case PPG_UPDATEAPP:
		strName ="Storage";
		TabStorage("other");
		break;
    case PPG_MOUNTAPP:
		if ( m_sData.id == 1)
		{
			strName ="MountApp";
		}
		else
		{
			strName ="UMountApp";
		}
		TabStreamerLink(strName);
		break;
	default:
		break;
	}
	return 0;
}

void CAddDataDlg::InitListCtrl()
{
	ListView_SetExtendedListViewStyle(m_hListCtrl,LVS_EX_FULLROWSELECT| LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);
	::GetWindowLong(m_hListCtrl,GWL_EXSTYLE | GWL_EXSTYLE & LVS_EX_FULLROWSELECT);
	
	CRect rc;
	::GetClientRect(m_hListCtrl,&rc);

	LV_COLUMN lvCol;
	lvCol.mask = LVCF_TEXT | LVCF_WIDTH |LVIF_STATE;
	lvCol.fmt = LVCFMT_LEFT;
	lvCol.iSubItem = 0;
	
	ListView_DeleteColumn(m_hListCtrl,0);
	lvCol.cx = rc.Width()/2;
	lvCol.pszText = "PrivateData";
	ListView_InsertColumn(m_hListCtrl,0,&lvCol);
	
	lvCol.cx = rc.Width()/2;
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
	char temp[100] = " ";
	
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

       TianShanIce::Transport::StorageLinkPrx link =m_client->linkStorage(StoragenetId,StreamernetId, strType, m_Pvals);
	   if ( link )
	   {
		   char szTemp[100];
		   sprintf(szTemp,"StorageLink[%s] added",link->getIdent().name.c_str());
	       MessageBox(szTemp,"Link successful");

		   // add by dony 
	//	   TianShanIce::AccreditedPath::StorageLinkExPrx storagelinkexprx = ::TianShanIce::AccreditedPath::StorageLinkExPrx::checkedCast(link);
	//	   storagelinkexprx->updatePrivateData(m_Pvals);
		   // add by dony

	       TabStorageLink();

	   }
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
	catch(...)
	{
		return 0;
	}
    return 0;
}

LRESULT CAddDataDlg::OnClickedLinkstreamerbtn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	// TODO : Add Code for control notification handler. ::SetWindowText(GetDlgItem(IDC_LINKSTREAMERBTN),_T("link Streamer"));
	int nSGID;
	std::string StreamernetId = " ";
	std::string strType;
	char temp[100]= " ";
	std::string appname="";
	
	
	int nPos = ::SendMessage(GetDlgItem(IDC_COMBO11), CB_GETCURSEL,0, 0);
	if(nPos < 0)
	{
      MessageBox("StreamerNetid is not empty!");
	  return 0;
	}
    ::SendMessage(GetDlgItem(IDC_COMBO11), CB_GETLBTEXT,nPos,(LPARAM)TEXT(temp));
	StreamernetId = temp;



	std::string strName;
	::GetWindowText(GetDlgItem(IDC_LINKSTREAMERBTN),temp,100);
	strName = temp;
	if ( strName =="Add StreamerLink")
	{
		if(temp[0] == '\0')
		{
		  MessageBox("StreamerNetid is not empty!");
		  return 0;
		}

		
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
			TianShanIce::Transport::StreamLinkPrx stre = m_client->linkStreamer(nSGID, StreamernetId, strType, m_Pvals);
			if ( stre)
			{
				char szTemp[100];
				sprintf(szTemp,"StreamerLink[%s] added",stre->getIdent().name.c_str());
				MessageBox(szTemp,"Link successful");
				strType ="StreamerLink";
				TabStreamerLink(strType);

				// add by dony 

			//	TianShanIce::AccreditedPath::StreamLinkExPrx streamlinkexprx = ::TianShanIce::AccreditedPath::StreamLinkExPrx::checkedCast(stre);
			//	streamlinkexprx->updatePrivateData(m_Pvals);
				// add by dony

			}
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
		 catch(...)
		 {
			 return 0;
		 }

	}
	else if ( strName =="Mount App")
	{
		if(temp[0] == '\0')
		{
			 MessageBox("Site is not empty!");
			return 0;
		}
		if(!Delblank(temp))
		{
			MessageBox("Site is not empty!");
			return 0;
		}

		memset(temp,0,sizeof(temp));
		::GetWindowText(GetDlgItem(IDC_PATH_EDIT),temp,100 );
		if(!Delblank(temp))
		{
			MessageBox("Path is not empty!");
			return 0;
		}
		strType = temp;


		memset(temp,0,sizeof(temp));
		nPos = ::SendMessage(GetDlgItem(IDC_STREAMERLINKCOMBO), CB_GETCURSEL,0, 0);
		if(nPos < 0)
		{
			MessageBox("StreamerNetid is not empty!");
			return 0;
		}
		::SendMessage(GetDlgItem(IDC_STREAMERLINKCOMBO), CB_GETLBTEXT,nPos,(LPARAM)TEXT(temp));
		if(!Delblank(temp))
		{
			MessageBox("App is not empty!");
			return 0;
		}
		appname=temp;
		try
		{
			if (m_businessClient->mountApplication(StreamernetId, strType, appname))
			{
				MessageBox(_T("mount application successful"));
			}
		}
		catch(::TianShanIce::InvalidParameter& ex)
		{
			 MessageBox(ex.message.c_str());
			 return 0;
		}	
		catch (const Ice::Exception& ex)
		{
			 MessageBox(_T(ex.ice_name().c_str()),"mountapplication error");
			 return 0;
		}
		catch(...)
		{
			 return 0;
		}
	}
	else
	{
		if(temp[0] == '\0')
		{
			MessageBox("Site is not empty!");
			return 0;
		}
		if(!Delblank(temp))
		{
			MessageBox("Site is not empty!");
			return 0;
		}

		memset(temp,0,sizeof(temp));
		::GetWindowText(GetDlgItem(IDC_PATH_EDIT),temp,100 );
		if(!Delblank(temp))
		{
			MessageBox("Path is not empty!");
			return 0;
		}
		strType = temp;

		try
		{
			if (m_businessClient->unmountApplication(StreamernetId, strType))
			{
				MessageBox(_T("umount application successful"));
			}
		}
		catch(::TianShanIce::InvalidParameter& ex)
		{
			 MessageBox(ex.message.c_str());
			 return 0;
		}	
		catch (const Ice::Exception& ex)
		{
			 MessageBox(_T(ex.ice_name().c_str()),"umountapplication error");
			 return 0;
		}
		catch(...)
		{
			 return 0;
		}
	}
    return 0;
}

LRESULT CAddDataDlg::OnClickedUpsgbtn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	// TODO : Add Code for control notification handler.

	int nId;
	char temp[100];
	std::string strDesc;
	std::string strName;
	
	::GetWindowText(GetDlgItem(IDC_UPSGBTN),temp,100);
	strDesc = temp;
	if ( (  strDesc =="Update SerGroup")  || (  strDesc =="Add SerGroup") )
	{
		memset(temp,0,sizeof(temp));
		::GetWindowText(GetDlgItem(IDC_SGID_EDIT),temp,100 );
		nId = atoi(temp);
		if(!Delblank(temp))
		{
			MessageBox("ServiceGroupID is not empty!");
			return 0;
		}
		
		memset(temp,0,sizeof(temp));
		::GetWindowText(GetDlgItem(IDC_SGDESC_EDIT),temp,100 );
		strDesc = temp;
		try
		{
			bool b1 =m_client->updateServiceGroup(nId, strDesc);
			if ( b1 )
			{
				MessageBox("updateServiceGroup success!","Update");
			
				::SetWindowText(GetDlgItem(IDC_SGID_EDIT), "");
				::SetWindowText(GetDlgItem(IDC_SGDESC_EDIT),"");
				::EnableWindow(GetDlgItem(IDC_SGID_EDIT), TRUE);
			}
		}
		catch(::TianShanIce::InvalidParameter& ex)
		{
			 MessageBox(ex.message.c_str());
			 return 0;
		}	
		catch (const Ice::Exception& ex)
		{
			 MessageBox(_T(ex.ice_name().c_str()),"updateServiceGroup error");
			 return 0;
		}
	}
	else if ( ( strDesc =="Update Site") || ( strDesc =="Add Site") )
	{
		memset(temp,0,sizeof(temp));
		::GetWindowText(GetDlgItem(IDC_SGID_EDIT),temp,100 );
		strName = temp;

		if(!Delblank(temp))
		{
		  MessageBox("Name is not empty!");
		  return 0;
		}
		
		memset(temp,0,sizeof(temp));
		::GetWindowText(GetDlgItem(IDC_SGDESC_EDIT),temp,100 );
		strDesc = temp;
		try
		{
			bool b1 =m_businessClient->updateSite(strName,strDesc);
			if ( b1 )
			{
				MessageBox("updateSite success!","Update");
				::SetWindowText(GetDlgItem(IDC_SGDESC_EDIT),"");
				::EnableWindow(GetDlgItem(IDC_SGID_EDIT), TRUE);
			}
		}
		catch(::TianShanIce::InvalidParameter& ex)
		{
			 MessageBox(ex.message.c_str());
			 return 0;
		}	
		catch (const Ice::Exception& ex)
		{
			 MessageBox(_T(ex.ice_name().c_str()),"update site error");
			 return 0;
		}
		catch(...)
		{
			MessageBox(_T("system Exception\n"));
			return 0;
		}

		
		
	}
	else // Set SiteProperties
	{
		memset(temp,0,sizeof(temp));
		::GetWindowText(GetDlgItem(IDC_SGID_EDIT),temp,100 );
		strName = temp;

		if(!Delblank(temp))
		{
			MessageBox("Property is not empty!");
			return 0;
		}
		
		memset(temp,0,sizeof(temp));
		::GetWindowText(GetDlgItem(IDC_SGDESC_EDIT),temp,100 );
		if(!Delblank(temp))
		{
			MessageBox("Value is not empty!");
			return 0;
		}
		strDesc = temp;

		try
		{
			TianShanIce::Properties props = m_businessClient->getSiteProperties(m_sData.netId);
			props[strName] = strDesc;
			bool b1 = m_businessClient->setSiteProperties(m_sData.netId,props);
			if ( b1 )
			{
				strDesc  =" set sitename=";
				strDesc += m_sData.netId;
				strDesc +="'s [";
				strDesc +=strName;
				strDesc +="] = ";
				strDesc +=temp;
				strDesc +=" successful";
				MessageBox(strDesc.c_str(),"Set Site");
			}
		}
		catch(::TianShanIce::InvalidParameter& ex)
		{
			 MessageBox(ex.message.c_str());
			 return 0;
		}	
		catch (const Ice::Exception& ex)
		{
			 MessageBox(_T(ex.ice_name().c_str()),"update site error");
			 return 0;
		}
		catch(...)
		{
			MessageBox(_T("System Exception\n"));
			return 0;
		}
	}
	return 0;
}

LRESULT CAddDataDlg::OnClickedUpstoragebtn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	// TODO : Add Code for control notification handler.

	char temp[100];
	std::string strType, strUK, strEP, strDesc;
	int nPos;

	::GetWindowText(GetDlgItem(IDC_UPSTORAGEBTN),temp,100 );
	strType = temp;
	if ( ( strType =="Update Storage")  || ( strType =="Add Storage") )
	{
		memset(temp,0,sizeof(temp));
		nPos = ::SendMessage(GetDlgItem(IDC_STORAGE_COMBO), CB_GETCURSEL,0, 0);
		::SendMessage(GetDlgItem(IDC_STORAGE_COMBO), CB_GETLBTEXT,nPos,(LPARAM)TEXT(temp));
		strType = temp;

		::GetWindowText(GetDlgItem(IDC_EP_EDIT),temp,100 );
		strEP = temp;
		m_strConstoreEndpoint = temp;

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
			bool b1 =m_client->updateStorage(strUK, strType,strEP, strDesc);

			/*
			if(!SetPrivateData())
				return 0;
				*/
			
			if ( b1)
			{
				TianShanIce::ValueMap::iterator pos = m_Pvals.begin();
				while(pos != m_Pvals.end())
				{
					//m_client->setStreamerPrivateData(strUK, (*pos).first,(*pos).second);
					m_client->setStoragePrivateData(strUK,(*pos).first,(*pos).second);
					pos++;
				}
				MessageBox("updateStorage success!","Update");
				
				::SetWindowText(GetDlgItem(IDC_DESC_EDIT),"");
				::SetWindowText(GetDlgItem(IDC_UK_EDIT),"");
				::EnableWindow(GetDlgItem(IDC_UK_EDIT), TRUE);
			}
		}
		catch(const TianShanIce::BaseException& ex)
		{
			ATLTRACE(_T("Call  updateStorage method  failed Ice Exception with error %s"),ex.message.c_str());
			return S_FALSE;
		}
		catch(...)
		{
			ATLTRACE(_T("Call  updateStorage method  failed  System Exception"));
			return S_FALSE;
		}
	}
	else
	{
		memset(temp,0,sizeof(temp));
			
		::GetWindowText(GetDlgItem(IDC_EP_EDIT),temp,100 );
		strEP = temp;

		if(!Delblank(temp))
		{
		  MessageBox("Name is not empty!");
		  return 0;
		}
		
		::GetWindowText(GetDlgItem(IDC_UK_EDIT),temp,100 );
		strUK = temp;

		if(!Delblank(temp))
		{
		  MessageBox("EndPoint is not empty!");
		  return 0;
		}
		
		::GetWindowText(GetDlgItem(IDC_DESC_EDIT),temp,100 );
		strDesc = temp;
		
		try
		{
			bool b1 =m_businessClient->updateApplication(strEP,strUK,strDesc);
						
			if ( b1)
			{
				MessageBox("update Application success!","Update");
				::SetWindowText(GetDlgItem(IDC_EP_EDIT), "");
				::SetWindowText(GetDlgItem(IDC_DESC_EDIT),"");
				::SetWindowText(GetDlgItem(IDC_UK_EDIT),"");
				::EnableWindow(GetDlgItem(IDC_UK_EDIT), TRUE);
			}
		}
		catch(const TianShanIce::BaseException& ex)
		{
			ATLTRACE(_T("Call  update application method  failed Ice Exception with error %s"),ex.message.c_str());
			return S_FALSE;
		}
		catch(...)
		{
			ATLTRACE(_T("Call  update application method  failed  System Exception"));
			return S_FALSE;
		}
	}
	return S_OK;
}

LRESULT CAddDataDlg::OnClickedUpstreamerbtn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	// TODO : Add Code for control notification handler.
	
	char temp[100];
	std::string strType, strUK, strEP, strDesc,strName;
	::GetWindowText(GetDlgItem(IDC_UPSTREAMERBTN),temp,100);
	strDesc = temp;
	if ( (  strDesc =="AddSiteResLimited") || (  strDesc =="UpdateSiteResLimit"))
	{
		memset(temp,0,sizeof(temp));
		::GetWindowText(GetDlgItem(IDC_SGID_EDIT),temp,100 );
		strName = temp;

		if(!Delblank(temp))
		{
		  MessageBox("Name is not empty!");
		  return 0;
		}

		// add by dony for set siteresourcemaxmlimit
		memset(temp,0,sizeof(temp));
		::GetWindowText(GetDlgItem(IDC_BWEDIT),temp,100 );
		int iMaxBW = atoi(temp);
		if(!Delblank(temp))
		{
			MessageBox("MaxBandWidth is not empty!");
			return 0;
		}

		memset(temp,0,sizeof(temp));
		::GetWindowText(GetDlgItem(IDC_SESSIONEDIT),temp,100 );
		if(!Delblank(temp))
		{
			MessageBox("MaxSessions is not empty!");
			return 0;
		}
		int iMaxSessions = atoi(temp);

		try
		{
			m_businessClient->updateSiteResourceLimited(strName,iMaxBW,iMaxSessions);
			MessageBox("updateSiteResourceLimited success!","Update");
			::SetWindowText(GetDlgItem(IDC_SGID_EDIT), "");
			::SetWindowText(GetDlgItem(IDC_BWEDIT), "");
			::SetWindowText(GetDlgItem(IDC_SESSIONEDIT), "");
			::EnableWindow(GetDlgItem(IDC_SGID_EDIT), TRUE);
		}
		catch(::TianShanIce::InvalidParameter& ex)
		{
			 MessageBox(ex.message.c_str());
			 return 0;
		}	
		catch (const Ice::Exception& ex)
		{
			 MessageBox(_T(ex.ice_name().c_str()),"update site error");
			 return 0;
		}
		catch(...)
		{
			MessageBox(_T("system Exception\n"));
			return 0;
		}
	}
	else
	{
	
		int nPos = ::SendMessage(GetDlgItem(IDC_STREAMER_COMBO), CB_GETCURSEL,0, 0);
		::SendMessage(GetDlgItem(IDC_STREAMER_COMBO), CB_GETLBTEXT,nPos,(LPARAM)TEXT(temp));
		strType = temp;
		
		::GetWindowText(GetDlgItem(IDC_EP_EDIT),temp,100 );
		m_strStreamSmithEndPoint = temp;
		strEP = temp;

		if(!Delblank(temp))
		{
		  MessageBox("EndPoint is not empty!");
		  return 0;
		}


		memset(temp,0,sizeof(temp));
		nPos = ::SendMessage(GetDlgItem(IDC_STREAMERID_COMBO), CB_GETCURSEL,0, 0);
		::SendMessage(GetDlgItem(IDC_STREAMERID_COMBO), CB_GETLBTEXT,nPos,(LPARAM)TEXT(temp));
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
			bool b1 =m_client->updateStreamer(strUK, strType, strEP, strDesc);
			
			/*
			if(!SetPrivateData())
				return 0;
				*/
			

			if ( b1 )
			{
				TianShanIce::ValueMap::iterator pos = m_Pvals.begin();

				while(pos != m_Pvals.end())
				{
				 m_client->setStreamerPrivateData(strUK, (*pos).first,(*pos).second);
				 pos++;
				}

				MessageBox("updateStreamer success!","Update");
							
				::SetWindowText(GetDlgItem(IDC_DESC_EDIT),"");
				::SetWindowText(GetDlgItem(IDC_UK_EDIT),"");
				::EnableWindow(GetDlgItem(IDC_UK_EDIT), TRUE);
			}
		}
		catch(const TianShanIce::BaseException& ex)
		{
			ATLTRACE(_T("Call  updateStreamer method  failed Ice Exception with error %s"),ex.message.c_str());
			return S_FALSE;
		}
		catch(...)
		{
			ATLTRACE(_T("Call  updateStreamer method  failed  System Exception"));
			return S_FALSE;
		}
	}
	return 0;
}

LRESULT CAddDataDlg::OnInitDialog(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
	// when the dialog box is created, subclass its edit control:

	
	std::string strName;
	bHandled = FALSE;
	m_hTabAction = GetDlgItem(IDC_TAB);
    m_hListCtrl = GetDlgItem(IDC_LIST);

    m_listEdit.SubclassWindow(GetDlgItem(IDC_EDIT));
    ::ShowWindow(GetDlgItem(IDC_EDIT),SW_HIDE);
	
    InitTabAction(m_hflag);
    InitCombox(IDC_STORAGE_COMBO, "StorageType");
    InitCombox(IDC_STREAMER_COMBO, "StreamerType");
    InitStorageLinkType();
	InitListCtrl();
   
	switch (m_hflag)
	{
	case PPG_UPSTORAGE:
		ShowSiteControl(FALSE);
		TabStorage();
		::SetWindowText(m_hWnd,_T("Add Storage"));
		if ( m_strConstoreEndpoint.IsEmpty())
		{
			::SetWindowText(GetDlgItem(IDC_EP_EDIT),_T("ContentStore: tcp -h 192.168.80.49 -p 55588"));
		}
		else
		{
			::SetWindowText(GetDlgItem(IDC_EP_EDIT),m_strConstoreEndpoint);
		}
		::SetWindowText(GetDlgItem(IDC_UPSTORAGEBTN),_T("Add Storage"));
		if(m_sData.flag)
		{
			 ::SetWindowText(GetDlgItem(IDC_EP_EDIT), (LPCSTR)m_sData.ifep.c_str());
			 ::SetWindowText(GetDlgItem(IDC_DESC_EDIT),(LPCSTR)m_sData.desc.c_str());
			 ::SetWindowText(GetDlgItem(IDC_UK_EDIT),(LPCSTR)m_sData.netId.c_str());
			 ::EnableWindow(GetDlgItem(IDC_UK_EDIT), FALSE);


			 ::SendMessage(GetDlgItem(IDC_STORAGE_COMBO), CB_ADDSTRING,0,(LPARAM)(LPCSTR)m_sData.type.c_str());
			 ::SendMessage( GetDlgItem(IDC_STORAGE_COMBO),CB_SETCURSEL,(WPARAM) 0, 0);	
			 m_Pvals = m_client->getStoragePrivateData(m_sData.netId);
			 UpdateListCtrlData();
			 ::SetWindowText(m_hWnd,_T("Update Storage"));
			 ::SetWindowText(GetDlgItem(IDC_UPSTORAGEBTN),_T("Update Storage"));
		}
		break;
	case PPG_UPSTREAMER:
		ShowSiteControl(FALSE);
		TabStreamer();
		::SetWindowText(m_hWnd,_T("Add Streamer"));
		if ( m_strStreamSmithEndPoint.IsEmpty())
		{
			::SetWindowText(GetDlgItem(IDC_EP_EDIT),_T("StreamSmith: tcp -h 10.15.10.251 -p 10000"));
		}
		else
		{
			::SetWindowText(GetDlgItem(IDC_EP_EDIT),m_strStreamSmithEndPoint);
		}
		
		::SetWindowText(GetDlgItem(IDC_UPSTREAMERBTN),_T("Add Streamer"));
		if(m_sData.flag)
		{
			 ::SetWindowText(GetDlgItem(IDC_EP_EDIT), (LPCSTR)m_sData.ifep.c_str());
			 ::SetWindowText(GetDlgItem(IDC_DESC_EDIT),(LPCSTR)m_sData.desc.c_str());
			 ::EnableWindow(GetDlgItem(IDC_UK_EDIT), FALSE);

			 ::SendMessage(GetDlgItem(IDC_STREAMERID_COMBO), CB_ADDSTRING,0,(LPARAM)(LPCSTR)m_sData.netId.c_str());
			 ::SendMessage( GetDlgItem(IDC_STREAMERID_COMBO),CB_SETCURSEL,(WPARAM) 0, 0);	


			 ::SendMessage(GetDlgItem(IDC_STREAMER_COMBO), CB_ADDSTRING,0,(LPARAM)(LPCSTR)m_sData.type.c_str());
			 ::SendMessage( GetDlgItem(IDC_STREAMER_COMBO),CB_SETCURSEL,(WPARAM) 0, 0);	
			 m_Pvals.clear();
			 m_Pvals = m_client->getStreamerPrivateData(m_sData.netId);
			 UpdateListCtrlData();
			 ::SetWindowText(m_hWnd,_T("Update Streamer"));
			 ::SetWindowText(GetDlgItem(IDC_UPSTREAMERBTN),_T("Update Streamer"));
		}
		break;
	case PPG_UPSG:
		ShowSiteControl(FALSE);
		strName = "ServiceGroup";
		TabServiceGroup(strName);
		::SetWindowText(m_hWnd,_T("Add ServiceGroup"));
		::SetWindowText(GetDlgItem(IDC_UPSGBTN),_T("Add SerGroup"));
		if(m_sData.flag)
		{
			 char temp[10];
			 ::SetWindowText(GetDlgItem(IDC_SGID_EDIT), (LPCSTR)itoa(m_sData.id, temp, 10));
			 ::SetWindowText(GetDlgItem(IDC_SGDESC_EDIT),(LPCSTR)m_sData.desc.c_str());
			 ::EnableWindow(GetDlgItem(IDC_SGID_EDIT), FALSE);
			 ::SetWindowText(m_hWnd,_T("Update ServiceGroup"));
			 ::SetWindowText(GetDlgItem(IDC_UPSGBTN),_T("Update SerGroup"));
		}
		break;
	case PPG_LINKSTORAGE:
		ShowSiteControl(FALSE);
		m_bStreamLinkType = FALSE;
		TabStorageLink();
		if(m_linkinfo.bStorage == 1)
		{
			UpLinkTab();
		}
		
		break;
	case PPG_LINKSTREAMER:
		ShowSiteControl(FALSE);
		m_bStreamLinkType = TRUE;
		InitStreamerLinkType();
		strName ="StreamerLink";
		TabStreamerLink(strName);
		if(m_linkinfo.bStorage == 0)
		{
           UpLinkTab();
		}
		
		break;
	case PPG_UPDATESITE:

		strName ="Site";
		TabServiceGroup(strName);
		if(m_sData.flag)
		{
			 ::SetWindowText(GetDlgItem(IDC_SGID_EDIT), (LPCSTR)m_sData.netId.c_str());
			 ::SetWindowText(GetDlgItem(IDC_SGDESC_EDIT),(LPCSTR)m_sData.desc.c_str());
			 char szTemp[20]={0};
			 itoa(m_sData.iMaxSession,szTemp,10);
			 ::SetWindowText(GetDlgItem(IDC_SESSIONEDIT),(LPCSTR)szTemp);
			 memset(szTemp,0,sizeof(szTemp));
			 ltoa(m_sData.lMaxBW,szTemp,10);
			 ::SetWindowText(GetDlgItem(IDC_BWEDIT),(LPCSTR)szTemp);
			 ::EnableWindow(GetDlgItem(IDC_SGID_EDIT), FALSE);
		}
		break;
	case PPG_UPDATEAPP:
		ShowSiteControl(FALSE);
		TabStorage("other");
		::SetWindowText(m_hWnd,_T("Add App"));
		::SetWindowText(GetDlgItem(IDC_UPSTORAGEBTN),_T("Add App"));
		
		if(m_sData.flag)
		{
			 ::SetWindowText(GetDlgItem(IDC_EP_EDIT), (LPCSTR)m_sData.netId.c_str());
			 ::SetWindowText(GetDlgItem(IDC_DESC_EDIT),(LPCSTR)m_sData.desc.c_str());
			 ::SetWindowText(GetDlgItem(IDC_UK_EDIT),(LPCSTR)m_sData.ifep.c_str());
			 ::EnableWindow(GetDlgItem(IDC_EP_EDIT), FALSE);
			 ::SetWindowText(m_hWnd,_T("Update App"));
			 ::SetWindowText(GetDlgItem(IDC_UPSTORAGEBTN),_T("Update App"));
		}
		break;
	case PPG_MOUNTAPP:
		ShowSiteControl(FALSE);
		if ( m_sData.id == 1)
		{
			strName ="MountApp";
			::SetWindowText(m_hWnd,_T("Mount App"));
		}
		else
		{
			strName ="UMountApp";
			::SetWindowText(m_hWnd,_T("UMount App"));
		}
		TabStreamerLink(strName);
		break;
	default:
		break;
	}
	if(m_hflag >= 0)
	{
		TabCtrl_SetCurSel(m_hTabAction, m_hflag);
	}
    CenterWindow();
	return 0;
}

LRESULT CAddDataDlg::OnClose(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
	// when the dialog box is created, subclass its edit control:
	EndDialog(0);
	return 0;
}

LRESULT CAddDataDlg::OnClickAppExitbtn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{ 
   ::SendMessage(m_hWnd,WM_CLOSE,NULL,NULL);
   return 0;
}

LRESULT CAddDataDlg::OnClickedGetstoragedata(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
#ifdef  _FORTEST
		::SendMessage(GetDlgItem(IDC_STORAGE_COMBO), CB_ADDSTRING,0, (LPARAM)(LPSTR)("TestStorageType1"));
		::SendMessage(GetDlgItem(IDC_STORAGE_COMBO), CB_ADDSTRING,0, (LPARAM)(LPSTR)("TestStorageType2"));
		::SendMessage(GetDlgItem(IDC_STORAGE_COMBO), CB_ADDSTRING,0, (LPARAM)(LPSTR)("TestStorageType3"));
#endif
	::SendMessage(GetDlgItem(IDC_STORAGE_COMBO), CB_RESETCONTENT ,0,0);
	char temp[250];
	std::string strEndPoint;
	::GetWindowText(GetDlgItem(IDC_EP_EDIT),temp,250 );
	strEndPoint = temp;
//	strEndPoint="ContentStore: tcp -h 10.11.0.250 -p 55588";
	// for the test strEndPoint="ContentStore: tcp -h 10.11.0.250 -p 55588"
//	::SetWindowText(GetDlgItem(IDC_EP_EDIT),_T("ContentStore: tcp -h 192.168.80.49 -p 55588");
	try
	{
		m_ContentClient= TianShanIce::Storage::ContentStorePrx::checkedCast(m_ic->stringToProxy(strEndPoint));
	}

	catch( const ::Ice::Exception &ex)
	{
		memset(temp,0,sizeof(temp));
		sprintf(temp,_T("%s%s%s"),_T("ContentStorePrx::CheckedCast  Ice exception with error:"), 
					   ex.ice_name().c_str(),strEndPoint.c_str());
		ATLTRACE(temp);
		MessageBox(temp);
		return FALSE;
	}
	catch(...)
	{
		sprintf(temp,_T("%s%s"),_T("Unknown exception got when parsing %s!"),strEndPoint.c_str());
		ATLTRACE(temp);
		MessageBox(temp);
		return FALSE;
	}
	
	if(!m_ContentClient)
	{
		ATLTRACE(_T("Failed to connect to ContentStore,ContentStore:%s!"),strEndPoint.c_str());
		return S_FALSE;
	}
	
	if ( m_ContentClient )
	{
		try
		{
		
			std::string strStoreType = m_ContentClient->type();
			std::string strSotreID  = m_ContentClient->getNetId();
	//		TianShanIce::Storage::StoreType  StoreTypeValue = m_ContentClient->type();
	//		int iNum = (int)StoreTypeValue;
	//		char sTemp[10];
	//		itoa(iNum,sTemp,10);
	//		::SendMessage(GetDlgItem(IDC_STORAGE_COMBO), CB_ADDSTRING,0, (LPARAM)(LPSTR)(sTemp));
			::SendMessage(GetDlgItem(IDC_STORAGE_COMBO), CB_ADDSTRING,0, (LPARAM)(LPSTR)(strStoreType.c_str()));
			::SetWindowText(GetDlgItem(IDC_UK_EDIT),strSotreID.c_str());
		}
		catch( const ::Ice::Exception &ex)
		{
			MessageBox("ContentStore's Type error");
			return 0;
		}
		catch(...)
		{
			MessageBox("ContentStore's Type error");
			return 0;
		}
	}
	SendMessage( GetDlgItem(IDC_STORAGE_COMBO),CB_SETCURSEL,(WPARAM) 0, 0);	
	return 0;
}
LRESULT CAddDataDlg::OnClickedGetstreamerdata(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
#ifdef  _FORTEST
		::SendMessage(GetDlgItem(IDC_STREAMER_COMBO), CB_ADDSTRING,0, (LPARAM)(LPSTR)("TestStreamType1"));
		::SendMessage(GetDlgItem(IDC_STREAMER_COMBO), CB_ADDSTRING,0, (LPARAM)(LPSTR)("TestStreamType2"));
		::SendMessage(GetDlgItem(IDC_STREAMER_COMBO), CB_ADDSTRING,0, (LPARAM)(LPSTR)("TestStreamType3"));
#endif

	char temp[250];
	std::string strEndPoint;
	::GetWindowText(GetDlgItem(IDC_EP_EDIT),temp,250 );
	strEndPoint = temp;
	::SendMessage(GetDlgItem(IDC_STREAMER_COMBO), CB_RESETCONTENT ,0, 0);
	::SendMessage(GetDlgItem(IDC_STREAMERID_COMBO), CB_RESETCONTENT ,0, 0);
	
//	 
	// for the test strEndPoint="StreamSmith: tcp -h 10.15.10.251 -p 10000"
	try
	{
		m_StreamerClient= TianShanIce::Streamer::StreamServicePrx::checkedCast(m_ic->stringToProxy(strEndPoint));
	}
	catch( const ::Ice::Exception &ex)
	{
		memset(temp,0,sizeof(temp));
		sprintf(temp,_T("%s%s%s"),_T("StreamServicePrx::CheckedCast  Ice exception with error:"), 
					   ex.ice_name().c_str(),strEndPoint.c_str());
		ATLTRACE(temp);
		MessageBox(temp);
		return FALSE;
	}
	catch(...)
	{
		sprintf(temp,_T("%s%s"),_T("Unknown exception got when parsing %s!"),strEndPoint.c_str());
		ATLTRACE(temp);
		MessageBox(temp);
		return FALSE;
	}
	
	if(!m_StreamerClient)
	{
		ATLTRACE(_T("Failed to connect to StreamSmith:%s!"),strEndPoint.c_str());
		return S_FALSE;
	}
	
	if ( m_StreamerClient )
	{
		try
		{
		
			TianShanIce::Streamer::StreamerDescriptors  StreamerDes = m_StreamerClient->listStreamers();

			int size = StreamerDes.size();
	   		for( int j = 0; j < size; j ++)
			{
				::SendMessage(GetDlgItem(IDC_STREAMER_COMBO), CB_ADDSTRING,0, (LPARAM)(LPSTR)(StreamerDes[j].type.c_str()));
				strEndPoint  =m_StreamerClient->getNetId();
				strEndPoint +="/";
				strEndPoint += StreamerDes[j].deviceId;
				::SendMessage(GetDlgItem(IDC_STREAMERID_COMBO), CB_ADDSTRING,0, (LPARAM)(LPSTR)(strEndPoint.c_str()));

			}
		}
		catch( const ::Ice::Exception &ex)
		{
			MessageBox("ListStreamers error");
			return 0;
		}
		catch(...)
		{
			MessageBox("ListStreamers error");
			return 0;
		}
	}
	SendMessage( GetDlgItem(IDC_STREAMER_COMBO),CB_SETCURSEL,(WPARAM) 0, 0);	
	SendMessage( GetDlgItem(IDC_STREAMERID_COMBO),CB_SETCURSEL,(WPARAM) 0, 0);	
	return 0;
}
LRESULT CAddDataDlg::OnDestroy(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
	// when the dialog box is created, subclass its edit control:
	return 0;
}
void CAddDataDlg::InitCombox(UINT nID, char *pFindString)
{
	::SetWindowText(GetDlgItem(IDC_EP_EDIT), "");
	/*
	std::string strEndPoint;
	char temp[150];
	
	if( strcmp(pFindString,"StorageType") == 0 ) // StorageType
	{
		::GetWindowText(GetDlgItem(IDC_EP_EDIT),temp,150 );
		strEndPoint = temp;
		strEndPoint="ContentStore: tcp -h 10.11.0.250 -p 55588";
		// for the test strEndPoint="ContentStore: tcp -h 10.11.0.250 -p 55588"
		try
		{
			m_ContentClient= TianShanIce::Storage::ContentStorePrx::checkedCast(m_ic->stringToProxy(strEndPoint));
		}
		catch(::Ice::ProxyParseException e)
		{
			ATLTRACE(_T("There was an error while parsing the stringified proxy, ContentStore:%s"),strEndPoint.c_str());
			return ;
		}
		catch(::Ice::NoEndpointException e)
		{
			ATLTRACE(_T("Endpoint no found!"));
			return ;
		}
		catch(::Ice::ObjectNotFoundException e)
		{
			ATLTRACE(_T("Can not found object!"));
			return ;
		}
		catch(::Ice::ObjectNotExistException e)
		{
			ATLTRACE(_T("Can not found object!"));
			return ;
		}
		catch(...)
		{
			ATLTRACE(_T("Unknown exception got when parsing ,ContentStore: %s!"),strEndPoint.c_str());
			return;
		}

		if(!m_ContentClient)
		{
			ATLTRACE(_T("Failed to connect to ContentStore,ContentStore:%s!"),strEndPoint.c_str());
			return;
		}
		if ( m_ContentClient )
		{
			TianShanIce::Storage::StoreType  StoreTypeValue = m_ContentClient->type();
			::SendMessage(GetDlgItem(nID), CB_ADDSTRING,0, (LPARAM)(LPSTR)(StoreTypeValue));
		}
	}
	else //StreamerType
	{
		::GetWindowText(GetDlgItem(IDC_EP_EDIT),temp,150 );
		strEndPoint = temp;
		strEndPoint="StreamSmith: tcp -h 10.15.10.251 -p 10000";
		// for the test strEndPoint="StreamSmith: tcp -h 10.15.10.251 -p 10000"
		try
		{
			m_StreamerClient= TianShanIce::Streamer::StreamServicePrx::checkedCast(m_ic->stringToProxy(strEndPoint));
		}
		catch(::Ice::ProxyParseException e)
		{
			ATLTRACE(_T("There was an error while parsing the stringified proxy, StreamSmith:%s"),strEndPoint.c_str());
			return ;
		}
		catch(::Ice::NoEndpointException e)
		{
			ATLTRACE(_T("Endpoint no found!"));
			return ;
		}
		catch(::Ice::ObjectNotFoundException e)
		{
			ATLTRACE(_T("Can not found object!"));
			return ;
		}
		catch(::Ice::ObjectNotExistException e)
		{
			ATLTRACE(_T("Can not found object!"));
			return ;
		}
		catch(...)
		{
			ATLTRACE(_T("Unknown exception got when parsing ,StreamSmith: %s!"),strEndPoint.c_str());
			return;
		}
		if(!m_StreamerClient)
		{
			ATLTRACE(_T("Failed to connect to StreamSmith:%s!"),strEndPoint.c_str());
			return;
		}
		if ( m_StreamerClient )
		{
			TianShanIce::Streamer::StreamerDescriptors  StreamerDes = m_StreamerClient->listStreamers();

			int size = StreamerDes.size();
	   		for( int j = 0; j < size; j ++)
			{
				::SendMessage(GetDlgItem(nID), CB_ADDSTRING,0, (LPARAM)(LPSTR)(StreamerDes[j].type.c_str()));
			}
		}
	}
	SendMessage( GetDlgItem(nID),CB_SETCURSEL,(WPARAM) 0, 0);	
	*/
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
	TianShanIce::Variant var;
	int nItemCount = ListView_GetItemCount(m_hListCtrl);

	for(int i = 0; i < nItemCount; i++)
	{
/*		ListView_GetItemText(m_hListCtrl,i,0, temp, 150);
		strKey = temp;
		ListView_GetItemText(m_hListCtrl,i,1, temp, 150);
		if ( strlen(temp) == 0 )
		{
			strKey = strKey + "的值不能为空，请设置";
			MessageBox(strKey.c_str());
			return FALSE;
		}
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
		}*/
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
			
			// add by dony 
			var.bRange = false;
			var.type   = m_schema[i].defaultvalue.type;
		
			if ( !m_bStreamLinkType )
			{
				switch(var.type)
				{
					case ::TianShanIce::vtInts:		var.ints.push_back(atoi(temp)); break;
					case ::TianShanIce::vtLongs:	var.lints.push_back(atol(temp)); break;
					case ::TianShanIce::vtStrings:	var.strs.push_back(temp); break;
				}
			}
			else
			{
				/*
				if(!CheckPrivateData(strKey, m_schema[i].defaultvalue.type, temp,&var))
				{
					strKey = strKey + " : data format is error!!";
					MessageBox(strKey.c_str());
					return FALSE;
				}
				*/
				switch(var.type)
				{
					case TianShanIce::vtInts:		CheckPrivateData("",var.type,temp,&var); break;
					case TianShanIce::vtLongs:	    CheckPrivateData("",var.type,temp,&var); break;
					case TianShanIce::vtStrings:	var.strs.push_back(temp); break;
				}
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
    TianShanIce::ValueMap::iterator pos;

	int isize = m_Pvals.size();
	isize = isize > m_schema.size() ?isize : m_schema.size();

    for(UINT i = 0; i < isize; i++)
	{
		std::string& str = m_schema[i].keyname;
		pos =  m_Pvals.find(m_schema[i].keyname);
		if (pos == m_Pvals.end()) 
		{
			if(!GetPrivateDataStr(strText,&m_schema[i].defaultvalue))
			{
				MessageBox("GetPrivateDataStr error!!");
				strText[0] ='\0';
			}			
			key = (LPSTR)m_schema[i].keyname.c_str();
		}
		else
		{		
			
			if(!GetPrivateDataStr(strText,&(pos->second)))
			{
				MessageBox("GetPrivateDataStr error!!");
				strText[0] ='\0';
				//		 return;	
			} 
			key = (LPSTR) pos->first.c_str();	  
		}

		lvItem.iItem = i;		
		lvItem.pszText =(LPSTR)key; 
		ListView_InsertItem(m_hListCtrl, &lvItem);	 
		ListView_SetItemText(m_hListCtrl,i,1,(LPSTR)strText);
	}
}
void DumpSchema(	TianShanIce::PDSchema& schema)
{
	TianShanIce::PDSchema::iterator it = schema.begin();
	char	szBuf[1024];
	for ( ; it != schema.end(); it++ ) 
	{
		TianShanIce::PDElement& ele = *it; 
		switch(ele.defaultvalue.type) 
		{		
		case TianShanIce::vtStrings:
			{
				sprintf(szBuf,"VtStrings:Key [%s] Value[%s]\n",ele.keyname.c_str(),ele.defaultvalue.strs[0].c_str());
			}
			break;
		case TianShanIce::vtInts:
			{
				sprintf(szBuf,"vtInts:Key [%s] Value[%d]\n",ele.keyname.c_str(),ele.defaultvalue.ints[0] );
			}
			break;
		case TianShanIce::vtLongs:
			{
				sprintf(szBuf,"vtLongs:Key [%s] Value[%d]\n",ele.keyname.c_str(),ele.defaultvalue.lints[0] );
			}
			break;
		case TianShanIce::vtFloats:
			{
				sprintf(szBuf,"vtFloats:Key [%s] Value[%f]\n",ele.keyname.c_str(),ele.defaultvalue.floats[0] );
			}
			break;
		default:
			{
				sprintf(szBuf,"Invalid TianShan type with KEY [%s]\n",ele.keyname.c_str());
			}
			break;
		}
		OutputDebugStringA(szBuf);
	}
}

void CAddDataDlg::UpLinkTab()
{
	try
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
			
			if ( m_linkinfo.storagelinkPrx)
			{
				m_Pvals = m_linkinfo.storagelinkPrx ->getPrivateData();
				
				m_schema.clear();
				int npos = SendMessage( GetDlgItem(IDC_STORAGELINKCOMBO) ,CB_GETCURSEL, 0,0);
				if(npos >=0 )
				{
					char strCombo[100];	
					std::string strType;
					SendMessage( GetDlgItem(IDC_STORAGELINKCOMBO) ,CB_GETLBTEXT, npos,(LPARAM)TEXT(strCombo));
					strType = strCombo;
					m_schema = m_client->getStorageLinkSchema(strType);
				}
				
			}
			::SetWindowText(m_hWnd,_T("Update StorageLink"));
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
			else
			{
				char szBuf[1024];
				sprintf(szBuf,"Can't find the link type %s",m_linkinfo.linktype.c_str());
				MessageBox(szBuf,"ERROR");
				::EnableWindow(GetDlgItem(IDC_STREAMERLINKCOMBO), FALSE);
				::EnableWindow(m_hListCtrl,FALSE);
				::EnableWindow(GetDlgItem(IDC_COMBO11), FALSE);
				::EnableWindow(GetDlgItem(IDC_COMBO22), FALSE);
				::EnableWindow(GetDlgItem(IDC_UPSTREAMERLINKBTN), FALSE);
				return ;
			}
			
			::EnableWindow(GetDlgItem(IDC_STREAMERLINKCOMBO), FALSE);
			
			if ( m_linkinfo.streamlinkprx)
			{
				m_Pvals = m_linkinfo.streamlinkprx->getPrivateData();
				
				m_schema.clear();
				int npos = SendMessage( GetDlgItem(IDC_STREAMERLINKCOMBO) ,CB_GETCURSEL, 0,0);
				if(npos >=0 )
				{
					char strCombo[100];	
					std::string strType;
					SendMessage( GetDlgItem(IDC_STREAMERLINKCOMBO) ,CB_GETLBTEXT, npos,(LPARAM)TEXT(strCombo));
					strType = strCombo;	
					m_schema = m_client->getStreamLinkSchema(strType);
				}
			}
			::SetWindowText(m_hWnd,_T("Update StreamerLink"));
		}
		
		::EnableWindow(GetDlgItem(IDC_COMBO11), FALSE);
		::EnableWindow(GetDlgItem(IDC_COMBO22), FALSE);
		
		UpdateListCtrlData();
	}
	catch(Ice::Exception &e1)
	{
		ATLTRACE(_T("Ice Exception %s\n"),e1.ice_name().c_str());
		return ;
	}
	catch (...)
	{
		return  ;
	}
}

void CAddDataDlg::InitStorageLinkType()
{
	try
	{
	   TianShanIce::StrValues strvalues = m_client->listSupportedStorageLinkTypes();
	   for(size_t i = 0; i < strvalues.size(); i++)
	   {
   			::SendMessage(GetDlgItem(IDC_STORAGELINKCOMBO), CB_ADDSTRING,i, (LPARAM)TEXT(strvalues[i].c_str()));
	   }
	   SendMessage( GetDlgItem(IDC_STORAGELINKCOMBO),CB_SETCURSEL,(WPARAM) 0, 0);
	}
	catch(Ice::Exception &e1)
	{
	   ATLTRACE(_T("Ice Exception %s\n"),e1.ice_name().c_str());
	   return ;
	}
    catch (...)
	{
	   return  ;
	}
}

LRESULT CAddDataDlg::OnSelchangeStoragelinkcombo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	// TODO : Add Code for control notification handler.
	try
	{
		int npos = SendMessage( GetDlgItem(IDC_STORAGELINKCOMBO) ,CB_GETCURSEL, 0,0);
		if(npos >= 0 )
		{
			char strCombo[100]; 
			std::string strType;
			
			SendMessage( GetDlgItem(IDC_STORAGELINKCOMBO) ,CB_GETLBTEXT, npos,(LPARAM)TEXT(strCombo));
			strType = strCombo;
			
			m_schema = m_client->getStorageLinkSchema(strType);
			
			ListCtrlAddData();
		}

	}
	catch(Ice::Exception &e1)
   {
	   ATLTRACE(_T("Ice Exception %s\n"),e1.ice_name().c_str());
	   return 0;
   }
   catch (...)
   {
	   return 0 ;
   }
	return 0;
}

void CAddDataDlg::InitStreamerLinkType()
{
   try
   {
   
	   m_schema.clear();
	   TianShanIce::StrValues strvalues= m_client->listSupportedStreamLinkTypes();

	   for(size_t i = 0; i < strvalues.size(); i++)
	   {
			::SendMessage(GetDlgItem(IDC_STREAMERLINKCOMBO), CB_ADDSTRING,i, (LPARAM)TEXT(strvalues[i].c_str()));
	   }
	   SendMessage( GetDlgItem(IDC_STREAMERLINKCOMBO),CB_SETCURSEL,(WPARAM) 0, 0);
   }
   catch(Ice::Exception &e1)
	{
	   ATLTRACE(_T("Ice Exception %s\n"),e1.ice_name().c_str());
	   return ;
	}
	catch (...)
	{
	   return  ;
	}
}

LRESULT CAddDataDlg::OnSelchangeStreamerlinkcombo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	// TODO : Add Code for control notification handler.
	try
	{
		m_schema.clear();
		int npos = SendMessage( GetDlgItem(IDC_STREAMERLINKCOMBO) ,CB_GETCURSEL, 0,0);
		if(npos >=0 )
		{
			char strCombo[100];	
			std::string strType;

			SendMessage( GetDlgItem(IDC_STREAMERLINKCOMBO) ,CB_GETLBTEXT, npos,(LPARAM)TEXT(strCombo));
			strType = strCombo;

			m_schema = m_client->getStreamLinkSchema(strType);
			ListCtrlAddData();
		}
	}
	catch(Ice::Exception &e1)
   {
	   ATLTRACE(_T("Ice Exception %s\n"),e1.ice_name().c_str());
	   return 0;
   }
   catch (...)
   {
	   return 0 ;
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

	for(int i = 0; i < (int)m_schema.size(); i ++)
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

LRESULT CAddDataDlg::OnDblclkList(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	// TODO : Add Code for control notification handler.
	RECT rcCtrl;
	LVHITTESTINFO lvhti;
	POINT pos;
	::GetCursorPos(&pos);
	RECT cRect ;
	::GetWindowRect(m_hListCtrl, &cRect);
//	::GetClientRect(m_hListCtrl,&cRect);
//	::ScreenToClient(m_hListCtrl,&pos);
	
	// Retreive the coordinates of the treecrtl viwndow
	
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

void CAddDataDlg::ShowSiteControl(BOOL b1)
{
	if ( b1)
	{
		::ShowWindow(GetDlgItem(IDC_STATICBW),SW_SHOW);
		::ShowWindow(GetDlgItem(IDC_BWEDIT),SW_SHOW);
		::ShowWindow(GetDlgItem(IDC_SESSIONSTATIC),SW_SHOW);
		::ShowWindow(GetDlgItem(IDC_SESSIONEDIT),SW_SHOW);
	}
	else
	{
		::ShowWindow(GetDlgItem(IDC_STATICBW),SW_HIDE);
		::ShowWindow(GetDlgItem(IDC_BWEDIT),SW_HIDE);
		::ShowWindow(GetDlgItem(IDC_SESSIONSTATIC),SW_HIDE);
		::ShowWindow(GetDlgItem(IDC_SESSIONEDIT),SW_HIDE);
	}
}
void CAddDataDlg::ShowEdit(BOOL bShow,int nItem,int nIndex, RECT rcCtrl)
{
	RECT cRect,cEdit;
	RECT rect = {0};
	if(bShow == TRUE)
	{
		char temp[100];
		ListView_GetItemText(m_hListCtrl,nItem,nIndex, temp, 100);
      
	//	::GetWindowRect(m_hWnd, &cDlgRect);
	//	::GetClientRect(m_hWnd,&cDlgRect);
		::GetWindowRect(m_hListCtrl, &cRect);
//		::GetClientRect(m_hListCtrl,&cRect);
		::GetWindowRect(m_listEdit,&cEdit);
//		::GetClientRect(m_listEdit,&cEdit);
		SetRect(&rect,cRect.left+ rcCtrl.left + 2, cRect.top + rcCtrl.top, cRect.left+ rcCtrl.left + ( rcCtrl.right - rcCtrl.left),cRect.top + rcCtrl.top + ( cEdit.bottom - cEdit.top) );
		ScreenToClient(&rect);
//		::MoveWindow(m_listEdit.m_hWnd,rcCtrl.left +cRect.left - cDlgRect.left, rcCtrl.top + cRect.top - cDlgRect.top - 30 ,rcCtrl.right - rcCtrl.left + 1,cEdit.bottom - cEdit.top, TRUE); // The old Style

		::MoveWindow(m_listEdit.m_hWnd,rect.left,rect.top,rcCtrl.right - rcCtrl.left, cEdit.bottom - cEdit.top, TRUE);
		::SetWindowPos(m_listEdit.m_hWnd,HWND_TOP,rect.left ,rect.top,rcCtrl.right - rcCtrl.left,cEdit.bottom-cEdit.top,SWP_SHOWWINDOW|SWP_DRAWFRAME);

		::SetWindowText(m_listEdit.m_hWnd,temp);
		::SetFocus(m_listEdit.m_hWnd);
		::ShowWindow(m_listEdit.m_hWnd,SW_SHOW);

//		::GetWindowRect(m_listEdit,&cEdit);
//		::GetClientRect(m_listEdit,&cEdit);

		::SendMessage(m_listEdit.m_hWnd,EM_SETSEL,0, -1);
		m_listEdit.SetCtrlData(MAKEWPARAM(nIndex,nItem)); 
	}
	
	else
		::ShowWindow(m_listEdit.m_hWnd,SW_HIDE);

}
	
LRESULT CAddDataDlg::OnEditEnd(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if(wParam == TRUE)
	{
		char temp[100];
		::GetWindowText(m_listEdit.m_hWnd,temp,100);
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

LRESULT CAddDataDlg::OnClickedUpstoragelinkbtn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	// TODO : Add Code for control notification handler.
   AtlTrace("%d", m_schema.size());
   if(!SetPrivateData())
		return 0;
	
	TianShanIce::Transport::StorageLinkExPrx storagelinkexprx = ::TianShanIce::Transport::StorageLinkExPrx::checkedCast(m_linkinfo.storagelinkPrx);

	try
	{
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
	catch(...)
	{
		MessageBox(_T("System Exception\n"));
		return 0;
	}
	MessageBox("Update linkStorage success!","Update Link");
	m_Pvals = m_linkinfo.storagelinkPrx ->getPrivateData();
	UpdateListCtrlData();
	return 0;
}

LRESULT CAddDataDlg::OnClickedUpstreamerlinkbtn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	// TODO : Add Code for control notification handler.
//   AtlTrace("%d", m_schema.size());
	if(!SetPrivateData())
		return 0;
	 TianShanIce::Transport::StreamLinkExPrx streamlinkexprx = ::TianShanIce::Transport::StreamLinkExPrx::checkedCast(m_linkinfo.streamlinkprx);
	 try
	 {
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
	 catch(...)
	 {
		 MessageBox(_T("System Excetption\n"));
		 return 0;
	 }
	 MessageBox("Update linkStreamer success!","Update Link");
	 m_Pvals = m_linkinfo.streamlinkprx->getPrivateData();
	 UpdateListCtrlData();
	 return 0;
}