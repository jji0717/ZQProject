// CdmiFuseDlg.cpp : implementation file
//
#include "stdafx.h"
#include "urlstr.h"
#include "CdmiFuseTray.h"
#include "CdmiFuseDlg.h"
// CCdmiFuseDlg dialog
IMPLEMENT_DYNAMIC(CCdmiFuseDlg, CDialog)

CCdmiFuseDlg::CCdmiFuseDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCdmiFuseDlg::IDD, pParent)
	, m_strCdmiServer(_T(""))
	, m_strUserName(_T(""))
	, m_strPassWord(_T(""))
	,dokanOptions(0)
	,hSingleMutex(NULL)
{
}

CCdmiFuseDlg::~CCdmiFuseDlg()
{
}

void CCdmiFuseDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_CONTAINER, m_cbContainer);
	DDX_Control(pDX, IDC_COMBO_DRIVE, m_cbDrive);
	DDX_Control(pDX, IDC_LIST1, m_listContainer);
	DDX_Text(pDX, IDC_EDIT_USER, m_strUserName);
	DDX_Text(pDX, IDC_EDIT_PASSWORD, m_strPassWord);
	DDX_Text(pDX, IDC_EDIT_CDMI_SERVER, m_strCdmiServer);
	DDX_Control(pDX, IDC_CHECK_SAVE, m_buttonSave);
	DDX_Control(pDX, IDC_CHECK_AUTO, m_buttonAutomount);
}


BEGIN_MESSAGE_MAP(CCdmiFuseDlg, CDialog)
	ON_BN_CLICKED(IDC_LOGIN, &CCdmiFuseDlg::OnBnClickedLogin)
	ON_BN_CLICKED(IDC_APPLY, &CCdmiFuseDlg::OnBnClickedApply)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CCdmiFuseDlg::OnNMDblclkList1)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_MESSAGE(WM_USER_CLOSE,OnExit) //tray exit message
	ON_MESSAGE(DM_GETDEFID,OnGetDefID)   
	ON_WM_NCPAINT()
	ON_NOTIFY(NM_RCLICK, IDC_LIST1, &CCdmiFuseDlg::OnNMRclickList1)
	ON_EN_CHANGE(IDC_EDIT_USER, &CCdmiFuseDlg::OnEnChangeEditUser)
	ON_COMMAND(ID_MENU_DELETE, &CCdmiFuseDlg::OnMenuDelete)
	ON_COMMAND(ID_MENU_CONNECT, &CCdmiFuseDlg::OnMenuConnect)
	ON_COMMAND(ID_MENU_AUTOMOUNT, &CCdmiFuseDlg::OnMenuAutomount)
END_MESSAGE_MAP()


// CCdmiFuseDlg message handlers
BOOL CCdmiFuseDlg::OnInitDialog()
{
	CdmiFuseOps::startCURLenv();
	hSingleMutex = CreateMutex(NULL,FALSE,"CdmiFuseDlg");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		CloseHandle(hSingleMutex);
		hSingleMutex=NULL;
		::AfxGetApp()->m_pMainWnd->SendMessage(WM_USER_EXIT);
		HWND hDlg=::FindWindow(NULL,"CdmiFuse");
		CWnd *pDlg=FindWindowExA(NULL,hDlg,NULL,"CdmiFuse");
		if (hDlg != NULL)
		{
			pDlg->ShowWindow(SW_SHOW);
			pDlg->SetForegroundWindow();
		}
		return FALSE;
	}
	CDialog::OnInitDialog();
	dokanOptions |= DOKAN_OPTION_DEBUG |DOKAN_OPTION_STDERR;
	initListCtrl();
	m_log.open(getPath(_T("log")).c_str(),7);
	addFirstUser();
	//m_strCdmiServer ="http://172.16.20.131:8080/aqua/rest/cdmi/";
	//m_strPassWord = "cstest";
	//m_strUserName = "cstest";
	UpdateData(FALSE);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CCdmiFuseDlg::initListCtrl()
{
	//format list
	m_listContainer.DeleteAllItems();
	DWORD dwExStyle = m_listContainer.GetExtendedStyle();
	dwExStyle |= LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_FLATSB ;
	m_listContainer.SetExtendedStyle( dwExStyle );

	//format column
	m_listContainer.InsertColumn(0,"Container",LVCFMT_LEFT,150);
	m_listContainer.InsertColumn(1,"Drive",LVCFMT_LEFT,60);
	m_listContainer.InsertColumn(2,"Automount",LVCFMT_LEFT,80);
	m_listContainer.InsertColumn(3,"Status",LVCFMT_LEFT,80);
}
void CCdmiFuseDlg::refreshContainerList(std::vector <std::string> &Containers)
{
	m_listContainer.DeleteAllItems();
	//Add list to container
	m_cbContainer.ResetContent();
	while(!Containers.empty())
	{
		std::string theContainer=Containers.front();
		if (theContainer.rfind('/') == theContainer.length()-1)
		{
			theContainer.erase(theContainer.rfind('/'));
		}
		if (theContainer != _T(""))
		{
			m_cbContainer.AddString(theContainer.c_str());
		}
		Containers.erase(Containers.begin());
	}
	m_cbContainer.SetCurSel(0);
	//Add list to drive
	m_cbDrive.ResetContent();
	DWORD  drive=GetLogicalDrives();
	int i;
	char ch;
	for(i=0,ch='A';i<26;i++,ch++)
	{
		if (ch=='A' || ch=='B')
		{
			drive=drive>>1;
			continue;
		}
		char ptr[2]={0};
		sprintf_s(ptr,_T("%c\0"),ch);
		if (drive%2 == 0)
		{
			m_cbDrive.AddString(ptr);
		}
		drive=drive>>1;
	}
	m_cbDrive.SetCurSel(0);
	//add the save data to containerList 
	Json::Reader reader;
	Json::Value userValue;
	reader.parse(m_strUserInfor,userValue);
	if (userValue.isMember("containers"))
	{
		Json::Value containers=userValue["containers"];
		Json::Value::UInt m;
		for(m=0;m<containers.size();m++)
		{
			Json::Value  containerValue=containers[m];
			if (!containerValue.isMember("Container") || !containerValue.isMember("Drive") ||!containerValue.isMember("Automount") ||!containerValue.isMember("Status"))
			{
				continue;
			}
			std::string containerName=containerValue["Container"].asString();
			std::string driveName=containerValue["Drive"].asString();
			int containerIndex=m_cbContainer.FindString(-1,containerName.c_str());
			int drievIndex=m_cbDrive.FindString(-1,driveName.c_str());
			if (containerIndex == -1 || drievIndex == -1)
			{
				continue;
			}
			m_cbContainer.DeleteString(containerIndex);
			m_cbDrive.DeleteString(drievIndex);
			std::string automount=containerValue["Automount"].asString();
			std::string  status=containerValue["Status"].asString();
			if(automount == "Y")
			{
				if( driveJudge(driveName))
				{
					CString driveInfor;
					driveInfor.Format("the drive[ %s ] is already exist ",driveName.c_str());
					inforBox(driveInfor);
					return;
				}
				addDokanFuse(containerName,driveName);
				status=_T("Active");
			}
			else
			{
				if (driveJudge(driveName))
				{
					CString driveInfor;
					driveInfor.Format("the drive[%s] is already exist ",driveName.c_str());
					inforBox(driveInfor);
					return;
				}
				status=_T("Inactive");
			}
			int row=m_listContainer.InsertItem(m_listContainer.GetItemCount(),containerName.c_str());
			m_listContainer.SetItemText(row,0,containerName.c_str());
			m_listContainer.SetItemText(row,1,driveName.c_str());
			m_listContainer.SetItemText(row,2,automount.c_str());
			m_listContainer.SetItemText(row,3,status.c_str());
			m_cbContainer.SetCurSel(0);
			m_cbDrive.SetCurSel(0);
		}//for
	}
}

void CCdmiFuseDlg::OnBnClickedLogin()
{
	UpdateData(TRUE);
	m_strCdmiServer.Trim();
	m_strPassWord.Trim();
	m_strUserName.Trim();
	if(m_strCdmiServer.GetLength() < 1 || m_strUserName.GetLength() < 1 || m_strPassWord.GetLength() < 1)
	{
		m_buttonSave.SetCheck(0);
		m_strPassWord=_T("");
		UpdateData(FALSE);
		inforBox("Invalid CdmiServer URL or UserName or PassWord","Error");
	   	//AfxMessageBox("Invalid CdmiServer URL or UserName or PassWord");
		return;
	}
	if (! CdmiFuseOps::getServerSideConfig(m_log,m_thpool,m_strCdmiServer.GetString()))
	{
		m_buttonSave.SetCheck(0);
		m_cbContainer.ResetContent();
		m_cbDrive.ResetContent();
		m_cbContainer.SetCurSel(0);
		m_cbDrive.SetCurSel(0);
		m_listContainer.DeleteAllItems();
		UpdateData(FALSE);
		inforBox("failed to getServerSideConfig\nplease check up your network and your CdmiServer URL","Error");
		return;
	}
	Json::Value result;
	int cdmiRetCode = CdmiFuseOps::loginIn(result, m_log, m_thpool, m_strCdmiServer.GetString(), m_strUserName.GetString(), m_strPassWord.GetString());
	if(!CdmiRet_SUCC(cdmiRetCode) || !result.isMember("secretAccessKey") || !result.isMember("objectID"))
	{
		m_strPassWord=_T("");
		m_buttonSave.SetCheck(0);
		m_cbContainer.ResetContent();
		m_cbDrive.ResetContent();
		m_cbContainer.SetCurSel(0);
		m_cbDrive.SetCurSel(0);
		m_listContainer.DeleteAllItems();
		UpdateData(FALSE);
	      CString strError;
	    //strError.Format("failed to login with error[%d==>%s]", cdmiRetCode, CdmiFuseOps::cdmiRetStr(cdmiRetCode));
	      if (701 == cdmiRetCode)
	      {
			strError.Format("failed to login\nplease check out your CdmiServer URL");
	      }else if (401 == cdmiRetCode)
		{
			strError.Format("failed to login\nplease check out your username and password");
		}else
		{
			strError.Format("failed to login with error[%d==>%s]", cdmiRetCode, CdmiFuseOps::cdmiRetStr(cdmiRetCode));
		}
	      inforBox(strError,"Error");	
	      return;
	}
	//clear the CdmiDokanMap
	clearDokanFuse();
	// TODO: Add your control notification handler code here
	std::vector <std::string> returnContainers;
     if (result.isMember("containers"))
	{
		Json::Value containers=result["containers"];
		Json::Value::UInt cIndex;
		if (containers.size() <= 0)
		{
			inforBox("there are no containers return");
		}
		for (cIndex = 0;cIndex < containers.size();cIndex++)
		{
			std::string containerId=containers[cIndex].asString();
			Json::Value containerName;
			//std::string containerUri="http://cstest:cstest@172.16.20.131:8080/aqua/rest/cdmi/cdmi_objectid/";
			std::string  cdmiServer=m_strCdmiServer.GetString();
			ZQ::common::URLStr strUrl(cdmiServer.c_str());
			strUrl.setUserName(m_strUserName.GetString());
			strUrl.setPwd(m_strPassWord.GetString());
			std::string containerUri= (char*)strUrl.generate();
			containerUri+="cdmi_objectid/";
			containerUri+=containerId;
			CdmiFuseOps::getContainer(containerName,m_log, m_thpool,containerUri);
			if (!containerName.isMember("parentURI") || !containerName.isMember("objectName"))
			{
				m_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseDlg,"LogIn failed to get the ContainerName with ID [%s]"),containerId.c_str());
				continue;;
			}
			Json::Value theContainer=containerName["parentURI"];
			std::string name=theContainer.asString();
			theContainer=containerName["objectName"];
			name += theContainer.asString();
			if (name == "")
			{
				m_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseDlg,"LogIn failed to get the ContainerName with ID [%s]"),containerId.c_str());
				continue;;
			}
			returnContainers.push_back(name);
		}//for
     }// if (result.isMember("containers")) 
	refreshContainerList(returnContainers);
	UpdateData(FALSE);
}
void CCdmiFuseDlg::OnBnClickedApply()
{
	//add the containerComboBox and the driveComboBox  to the containerList
	CString  containerName;
	m_cbContainer.GetWindowText(containerName);
	CString driveName;
	m_cbDrive.GetWindowText(driveName);
	if (containerName.IsEmpty() || driveName.IsEmpty())
	{
		//no data to add ,then do nothing
		inforBox("sorry , there no container or drive to mount");
		return;
	}
	for (int i=0;i<m_listContainer.GetItemCount();i++)
	{
		if (m_listContainer.GetItemText(i,0) == containerName || m_listContainer.GetItemText(i,1) == driveName)
		{
			//if the containerName or the driveName already exist then do nothing
			return;
		}
	}
	std::string theName=driveName.GetString();
	if(driveJudge(theName))
	{
		m_cbDrive.DeleteString(m_cbDrive.FindString(-1,driveName));
		m_cbDrive.SetCurSel(0);
		m_buttonAutomount.SetCheck(0);
		CString driveInfor;
		driveInfor.Format("the drive[ %s ] is already exist ",driveName.GetString());
		inforBox(driveInfor);
		return;
	}
	addDokanFuse(containerName.GetString(),driveName.GetString());
	//insert  element
	int row=m_listContainer.InsertItem(m_listContainer.GetItemCount(),containerName);
	m_listContainer.SetItemText(row,0,containerName);
	m_listContainer.SetItemText(row,1,driveName);
	if (m_buttonAutomount.GetCheck())
	{
		m_listContainer.SetItemText(row,2,"Y");
	}else{
		m_listContainer.SetItemText(row,2,"N");
	}
	m_listContainer.SetItemText(row,3,"Active");
	int c=m_cbContainer.FindString(-1,containerName);
	m_cbContainer.DeleteString(c);
	m_cbContainer.SetCurSel(0);
	m_cbDrive.DeleteString(m_cbDrive.FindString(-1,driveName));
	m_cbDrive.SetCurSel(0);
	m_buttonAutomount.SetCheck(0);
	return ;
	// TODO: Add your control notification handler code here
}
void CCdmiFuseDlg::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	POSITION pos = m_listContainer.GetFirstSelectedItemPosition();
	INT nLine = m_listContainer.GetNextSelectedItem(pos);
	if (nLine == -1) {
		return;
	}
	//you can get line number and know what you want to do
	/*CString sBuf;
	sBuf.Format("You have selected Line %d",nLine);
	inforBox(sBuf);*/
	*pResult = 0;
}
void CCdmiFuseDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	ShowWindow(SW_HIDE);
}

LRESULT CCdmiFuseDlg::OnExit(WPARAM wParam,LPARAM lParam)
{
	//use the message WM_CLOSE_CHILD to close all child window
	while(!m_handleChild.empty()){
		HWND hCloseHandle=m_handleChild.back();
		m_handleChild.pop_back();
		::PostMessage(hCloseHandle,WM_CLOSE_CHILD,NULL,NULL);
	}
	//close mutex
	if(hSingleMutex != NULL)
	{
		CloseHandle(hSingleMutex);
	}
    //save the user's Information
	saveUserInfor();
    //delete  all elements of CdmiDokanMap and free all the ptr
	clearDokanFuse();
	CdmiFuseOps::stopCURLenv();

	// close dialog
	CDialog::OnOK();
	return 0;
}
LRESULT CCdmiFuseDlg::OnGetDefID(WPARAM wParam,LPARAM lParam)
{
	 return   MAKELONG(0,DC_HASDEFID);   
}
void CCdmiFuseDlg::OnNcPaint()
{
	// TODO: Add your message handler code here
	// Do not call CDialog::OnNcPaint() for painting messages
        static int i = 1; // 如果将i定义为1，则必须把窗口的Visible属性去掉, else set i=2
        if(i > 0)
        {
            i --;
            ShowWindow(SW_HIDE);
        }
        else
      CDialog::OnNcPaint();
}
void CCdmiFuseDlg::saveUserInfor()
{
	UpdateData(TRUE);
	if (m_strUserName.GetString() == "")
	{
		return;
	}
	int Ret=m_buttonSave.GetCheck();
	Json::Value userValue;
	Json::FastWriter writer;
	userValue["userName"] = std::string(m_strUserName.GetString());
	if (Ret == 1)
	{
		userValue["passWord"] = std::string(m_strPassWord.GetString());
	}
	else
	{
		userValue["passWord"] = std::string("");
	}
	userValue["cdmiServer"]=std::string(m_strCdmiServer.GetString());
	Json::Value containers;
	Json::Value containerValue;
	for(int i=0;i<m_listContainer.GetItemCount();i++)
	{
		containerValue["Container"] = std::string(m_listContainer.GetItemText(i,0).GetString());
		containerValue["Drive"] = std::string(m_listContainer.GetItemText(i,1).GetString());
		containerValue["Automount"] = std::string(m_listContainer.GetItemText(i,2).GetString());
		containerValue["Status"] = std::string(m_listContainer.GetItemText(i,3).GetString());
		containers.append(containerValue);
		containerValue.clear();
	}
	userValue["containers"] =  containers;
	std::string userBody = writer.write(userValue);
	if (strcmp(userBody.c_str(),m_strUserInfor.c_str()) == 0)
	{
		return ;
	}
	deleteUser();
	int length=userBody.length() ;
	HANDLE hFile=::CreateFile(
		getPath(m_strUserName.GetString()).c_str(),
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD writeSize;
		length=length*sizeof(char);
		//write the user's information
		if(!::WriteFile(hFile,userBody.c_str(),userBody.length(),&writeSize,NULL))
		{
			inforBox("Write UserInformation Fail","ERROR");
			::CloseHandle(hFile);
			return;
		}
	}
	else{
		inforBox("Write UserInformation Fail","ERROR");
		::CloseHandle(hFile);
		return;
	}
	::CloseHandle(hFile);
}// 
std::string CCdmiFuseDlg::findUser(std::string &userName)
{
	HANDLE hFile=::CreateFile(
		getPath(userName).c_str(),
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_READONLY,
		NULL
		);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return "";
	}
	std::string buffer;
	char thebuffer[64]={0};
	DWORD readSize,hopeRead=sizeof(thebuffer);
	DWORD fileSize=GetFileSize(hFile,NULL);
	while(fileSize != 0)
	{
		::ReadFile(hFile,thebuffer,hopeRead,&readSize,NULL);
		for (DWORD i=0;i< readSize;i++)
			buffer.push_back(thebuffer[i]);

		fileSize -=readSize;
	}
	CloseHandle(hFile);
	//parse the string buffer to the Json value
	Json::Reader reader;
	Json::Value theValue;
	reader.parse(buffer,theValue);
	if (!theValue.isMember("userName") || !theValue.isMember("passWord") || !theValue.isMember("cdmiServer"))
		return "";

	std::string theName=theValue["userName"].asString();
	if (userName != theName)
		return "";

	std::string passWord=theValue["passWord"].asString();
	std::string cdmiServer=theValue["cdmiServer"].asString();
	m_strPassWord.Format("%s",passWord.c_str());
	m_strCdmiServer.Format("%s",cdmiServer.c_str());
	if (passWord  != "")
		m_buttonSave.SetCheck(1);

	return buffer;
}

void  CCdmiFuseDlg::addFirstUser()
{
	std::string userName=_T("");
	CTime accessTime;
	CFileFind findFile;
	bool ret=findFile.FindFile(".\\users\\*.bin");
	findFile.GetCreationTime(accessTime);
	int num=2;
	do{
		CTime theTime;
		findFile.GetLastAccessTime(theTime);
		if (theTime > accessTime)
		{
			accessTime=theTime;
			userName=findFile.GetFileTitle();
		}
		if(!findFile.FindNextFile())
			num--;
	}while(num);
	findFile.Close();
	if (userName == _T(""))
	{
		return;
	}
	HANDLE hFile=::CreateFile(
		getPath(userName).c_str(),
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_READONLY,
		NULL
		);
	std::string buffer;
	char thebuffer[64]={0};
	DWORD readSize,hopeRead=sizeof(thebuffer);
	DWORD fileSize=GetFileSize(hFile,NULL);
	while(fileSize != 0)
	{
		::ReadFile(hFile,thebuffer,hopeRead,&readSize,NULL);
		for (DWORD i=0; i<readSize; i++)
			buffer.push_back(thebuffer[i]);

		fileSize -=readSize;
	}

	if (buffer == "")
	{

		CloseHandle(hFile);
		m_strUserName.Format("%s",userName.c_str());
		UpdateData(FALSE);
		deleteUser();
		return;
	}

	m_strUserInfor=buffer;
	//parse the string buffer to the Json value
	Json::Reader reader;
	Json::Value theValue;
	reader.parse(buffer,theValue);
	if (!theValue.isMember("userName") || !theValue.isMember("passWord") || !theValue.isMember("cdmiServer"))
		return;

	std::string theName=theValue["userName"].asString();
	std::string passWord=theValue["passWord"].asString();
	std::string cdmiServer=theValue["cdmiServer"].asString();
	m_strUserName.Format("%s",theName.c_str());
	m_strPassWord.Format("%s",passWord.c_str());
	m_strCdmiServer.Format("%s",cdmiServer.c_str());
	UpdateData(false);
	CloseHandle(hFile);
	//m_buttonSave.SetCheck(0);
	if (passWord != "")
	{
		m_buttonSave.SetCheck(1);
		OnBnClickedLogin();
	}

	return;
}
void CCdmiFuseDlg::deleteUser()
{
	//delete the user's information
	UpdateData(TRUE);
	DeleteFile(getPath(m_strUserName.GetString()).c_str());
}
void CCdmiFuseDlg::inforBox(CString inforData,CString titleData)
{
	//use the CDialogInfo Dialog to show the information 
	CDialogInfo InfoDialog(inforData,titleData,this);
	InfoDialog.DoModal();
}
std::string CCdmiFuseDlg::getPath(std::string pathType,std::string driveName/*=""*/)
{
	CFileFind pathFind;
	std::string filePath;
	if ("log" == pathType)
	{
		filePath=_T(".\\logs");
	}
	else
	{
		filePath=_T(".\\users");
	}
	while(!pathFind.FindFile(filePath.c_str()))
	{
		::CreateDirectory(filePath.c_str(),NULL);
	}
	pathFind.Close();
	 if (pathType == "log")
	{
		if (driveName == "")
		{
			filePath=_T(".\\logs\\CdmiFuseTray.log");
		}
		else
		{
			filePath=_T(".\\logs\\CdmiFuseTray_");
			filePath+=driveName;
			filePath+=_T(".log");
		}
	}
	else
	{
		filePath+=_T("\\");
		filePath+=pathType;
		filePath+=_T(".bin");
	}
	return filePath;
}
void CCdmiFuseDlg::OnNMRclickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	POSITION pos = m_listContainer.GetFirstSelectedItemPosition();
	INT nLine = m_listContainer.GetNextSelectedItem(pos);
	if (nLine == -1) {
		return;
	}
	CMenu popMenu;
	popMenu.LoadMenu(IDR_CONTAINER);
	CMenu* pPopup = popMenu.GetSubMenu(0);
	std::string strConnect,strAuto;
	if("Y" == m_listContainer.GetItemText(nLine,2))
		strAuto=_T("Manual");
	else
		strAuto=_T("Automount");
	if ("Active" == m_listContainer.GetItemText(nLine,3))
		strConnect=_T("Inactive");
	else
		strConnect=_T("Active");
	pPopup->ModifyMenu(1,MF_STRING | MF_BYPOSITION,ID_MENU_AUTOMOUNT,strAuto.c_str());
	pPopup->ModifyMenu(2,MF_STRING | MF_BYPOSITION,ID_MENU_CONNECT,strConnect.c_str());
	CPoint point;
	GetCursorPos(&point);
	point.y+=10;
	pPopup->TrackPopupMenu(LVCFMT_LEFT,point.x,point.y,this);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}
void CCdmiFuseDlg::OnEnChangeEditUser()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	UpdateData(TRUE);
	m_strUserInfor=findUser(std::string(m_strUserName.GetString()));
	if (m_strUserInfor == "")
	{
		m_strPassWord=_T("");
		m_buttonSave.SetCheck(0);
	}
	m_cbContainer.ResetContent();
	m_cbDrive.ResetContent();
	m_cbContainer.SetCurSel(0);
	m_cbDrive.SetCurSel(0);
	m_listContainer.DeleteAllItems();
	UpdateData(FALSE);
	if (m_strPassWord.IsEmpty())
	{
		return;
	}
	OnBnClickedLogin();
	// TODO:  Add your control notification handler code here
}
void CCdmiFuseDlg::OnMenuDelete()
{
    POSITION pos = m_listContainer.GetFirstSelectedItemPosition();
    INT nLine = m_listContainer.GetNextSelectedItem(pos);
    CString container=m_listContainer.GetItemText(nLine,0);
    CString drive=m_listContainer.GetItemText(nLine,1);
    deleteDokanFuse(container.GetString());
    m_cbContainer.AddString(container.GetString());
    m_cbContainer.SetCurSel(0);
    m_cbDrive.AddString(drive.GetString());
    m_cbDrive.SetCurSel(0);
    m_listContainer.DeleteItem(nLine);
	// TODO: Add your command handler code here
}
void CCdmiFuseDlg::OnMenuConnect()
{
	POSITION pos = m_listContainer.GetFirstSelectedItemPosition();
	INT nLine = m_listContainer.GetNextSelectedItem(pos);
	CString status=m_listContainer.GetItemText(nLine,3);
	if ("Inactive" == status)
	{
		std::string theName=m_listContainer.GetItemText(nLine,1).GetString();
		if(driveJudge(theName))
		{
			std::string drive=m_listContainer.GetItemText(nLine,1).GetString();
			m_cbContainer.AddString(m_listContainer.GetItemText(nLine,0).GetString());
			m_cbContainer.SetCurSel(0);
			m_listContainer.DeleteItem(nLine);
			CString driveInfor;
			driveInfor.Format("the drive[ %s ] is already exist ",drive.c_str());
			inforBox(driveInfor);
			return;
		}
		addDokanFuse(m_listContainer.GetItemText(nLine,0).GetString(),m_listContainer.GetItemText(nLine,1).GetString());
		m_listContainer.SetItemText(nLine,3,"Active");
		//add the connected code
	}else{
		m_listContainer.SetItemText(nLine,3,"Inactive");
		deleteDokanFuse(m_listContainer.GetItemText(nLine,0).GetString());
		//add the disconnected code
	}
	// TODO: Add your command handler code here
}
void CCdmiFuseDlg::OnMenuAutomount()
{
	POSITION pos = m_listContainer.GetFirstSelectedItemPosition();
	INT nLine = m_listContainer.GetNextSelectedItem(pos);
	CString automount=m_listContainer.GetItemText(nLine,2);
	if ("N" == automount)
	{
		m_listContainer.SetItemText(nLine,2,"Y");
	}else{
		m_listContainer.SetItemText(nLine,2,"N");
	}
	// TODO: Add your command handler code here
}
std::string CCdmiFuseDlg::getRootURL()
{
	CString rootUrl=m_strCdmiServer;
	int pos=rootUrl.Find("//"); 
	pos+=2;
	CString userInfor;
	userInfor.Format("%s:%s@",m_strUserName,m_strPassWord);
	rootUrl.Insert(pos,userInfor);
	return rootUrl.GetString();
}
void CCdmiFuseDlg::clearDokanFuse()
{
	while (! m_ptrDokanFuse.empty())
	{
		CdmiDokan* pfuse=m_ptrDokanFuse.begin()->second;
		if (pfuse)
		{
			Sleep(100);
			pfuse->stop();
			delete pfuse;
			pfuse=NULL;
		}
		m_ptrDokanFuse.erase(m_ptrDokanFuse.begin());
	}
	while (! m_ptrMountLog.empty())
	{
		ZQ::common::FileLog* pLog=m_ptrMountLog.begin()->second;
		if (pLog)
		{
			delete pLog;
			pLog=NULL;
		}
		m_ptrMountLog.erase(m_ptrMountLog.begin());
	}
}
void CCdmiFuseDlg::addDokanFuse(std::string container,std::string drive)
{
	std::string rootURL=getRootURL();
	CdmiDokan* pfuse;
	/*if (driveJudge(drive))
	{
		return false;
	}*/
	ZQ::common::FileLog* mountLog=new ZQ::common::FileLog(getPath(_T("log"),drive).c_str(),7);
	pfuse=new CdmiDokan(*mountLog,m_thpool,drive,rootURL,container,dokanOptions);
	bool rest=pfuse->start();
	m_ptrDokanFuse.insert(std::pair<std::string,CdmiDokan*> (container,pfuse));
	m_ptrMountLog.insert(std::pair<std::string,ZQ::common::FileLog*> (container,mountLog));
}
void CCdmiFuseDlg::deleteDokanFuse(std::string container)
{
	std::map<std::string,CdmiDokan*>::iterator pos;
	pos=m_ptrDokanFuse.find(container);
	if (pos==m_ptrDokanFuse.end())
	{
		return;
	}
	CdmiDokan* pfuse=pos->second;
	if (pfuse!=NULL)
	{
		Sleep(100);
		pfuse->stop();
		delete pfuse;
		pfuse=NULL;
	}
	m_ptrDokanFuse.erase(pos);
	std::map<std::string,ZQ::common::FileLog*>::iterator lPos;
	lPos=m_ptrMountLog.find(container);
	ZQ::common::FileLog *pLog=lPos->second;
	if(pLog != NULL)
	{
		delete pLog;
		pLog=NULL;
	}
	m_ptrMountLog.erase(lPos);
}
bool CCdmiFuseDlg::driveJudge(std::string &drive)
{
	char ch=*(drive.begin());
	DWORD  thedrive=GetLogicalDrives();
	for (char c='A';c<ch;c++)
	{
		thedrive=thedrive>>1;
	}
	return thedrive%2;
}
/////////////////////////////////////////////////////////////////
// CDialogInfo dialog

IMPLEMENT_DYNAMIC(CDialogInfo, CDialog)

CDialogInfo::CDialogInfo(CString &infor,CString title/*="INFO"*/,CWnd* pParent /*=NULL*/)
	: inforData(infor),titleData(title),CDialog(CDialogInfo::IDD, pParent)
{
	
}

CDialogInfo::~CDialogInfo()
{
}

void CDialogInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BOOL CDialogInfo::OnInitDialog()
{
	CDialog::OnInitDialog();
	int len=inforData.GetLength();
	((CCdmiFuseDlg*)this->GetParent())->m_handleChild.push_back(this->m_hWnd);
	/*CRect theRect;
	this->GetWindowRect(&theRect);*/
	int t=inforData.Find("\n",0);
	if (t>40)
	{
		t=0;
	}
	for (int i=40+t;i<len;i+=40)
	{
	     t=inforData.Find("\n",i);
		if (t!=-1 && t<i)
		{
			i=t+40;
			continue;
		}
		t=inforData.Find(" ",i-10);
		if (t>i-10 && t<i+10)
		{
			i=t;
		}
		inforData.Insert(i,"\n");
		i++;
	}
	this->SetWindowText(titleData);
	OnPaint();
	return TRUE;
}

BEGIN_MESSAGE_MAP(CDialogInfo, CDialog)
	ON_BN_CLICKED(IDOK, &CDialogInfo::OnBnClickedOk)
	ON_WM_PAINT()
	ON_MESSAGE(WM_CLOSE_CHILD,OnCloseChild)
END_MESSAGE_MAP()


// CDialogInfo message handlers


void CDialogInfo::OnBnClickedOk()
{
	OnClose();
	OnOK();
}


void CDialogInfo::OnClose(void)
{
	this->GetParent()->SetForegroundWindow();
	this->DestroyWindow();
}

void CDialogInfo::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	dc.SetBkMode(TRANSPARENT);
	CRect oldRect;
	this->GetClientRect(&oldRect);
	oldRect.top=oldRect.top+20;
	dc.DrawText(inforData,oldRect,DT_CENTER);
}

LRESULT CDialogInfo::OnCloseChild(WPARAM waram,LPARAM lparam)
{
	OnBnClickedOk();
	return TRUE;
	// TODO: Add your message handler code here
}
