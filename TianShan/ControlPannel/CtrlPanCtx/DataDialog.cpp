// DataDialog.cpp: implementation of the CDataDialog class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DataDialog.h"

CSortListViewCtrl* CDataDialog::m_pListCtrl = NULL;
bool               CDataDialog::IsExist(const string & strUid, int * iIndex);
int CALLBACK       CDataDialog::GetProgressProc( const int & iTotalNum, const int  & iCurNum);
int CALLBACK       CDataDialog::GetPlayListStateProc(const string & strUid, const string & strMsg, const string & strStateValue, const int & iCurCtrlNum );
CProgressBarCtrl   CDataDialog::m_ProgressCtrl;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CDataDialog::CDataDialog(LPCTSTR szTabName,BOOL bShowAboveCtrl)
{
	memset(m_strTabName,0,sizeof(m_strTabName));
	sprintf(m_strTabName,"%s",szTabName);
	m_pListCtrl = NULL; 
	m_bShowAboveCtrl = bShowAboveCtrl;
	m_bShowAllData = TRUE;
	m_iNumColumns = -1;
	m_pCellsData = NULL; 
	m_iNumRows = -1;
	m_iBtnSearchNum = 0;
}

CDataDialog::~CDataDialog()
{
	if ( m_pListCtrl)
	{
		m_pListCtrl->DestroyWindow();
	}
	m_pListCtrl = NULL;
	// free the cells's memory
	FreeMemoryData();
}

void CDataDialog::FreeMemoryData()
{
	if ( m_bShowAboveCtrl  && m_pCellsData)
	{			
		for( int i=0;i<m_iNumRows;i++)
		{
			for( int j=0;j<m_iNumColumns;j++)
			{
				free((m_pCellsData[i][j]).szItemData);
			}
		}
		for( i=0;i<m_iNumRows;i++)
		{
			delete [] m_pCellsData[i]; //释放列
		}
		delete m_pCellsData; //释放行
	}
}

LRESULT CDataDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int iMode = 0;
	DWORD dwStyle =  dwStyle   =  WS_CHILD|WS_VSCROLL|LVS_REPORT|WS_HSCROLL|LVS_SHOWSELALWAYS ;
	DWORD dwExStyle =  LVS_EX_FULLROWSELECT| LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP;

	m_pListCtrl = new CSortListViewCtrl(iMode,m_strTabName);
	m_pListCtrl->Create(*this,rcDefault,NULL,dwStyle,dwExStyle);
//	m_pListCtrl->Attach(GetDlgItem(IDC_DATALIST));
	m_pListCtrl->SetExtendedListViewStyle(dwExStyle);
	m_pListCtrl->SetParent(*this);
	m_pListCtrl->ShowWindow(SW_SHOWNORMAL);
	m_pListCtrl->AttachHeader();

	if ( m_bShowAboveCtrl)
	{
		
		m_StaticCtrl.Attach(GetDlgItem(IDC_DATASTATIC));
		m_CheckBtn.Attach(GetDlgItem(IDC_DATACHECK));
		m_ProgressCtrl.Attach(GetDlgItem(IDC_DATAPROGRESS));
		m_ConditionEdit.SubclassWindow(GetDlgItem(IDC_CONDITIONEDIT));
		m_SearchBtn.SubclassWindow(GetDlgItem(IDC_SEARCHBTN));
		m_CheckBtn.SetCheck(m_bShowAllData);
	}
	else
	{
		::ShowWindow(GetDlgItem(IDC_DATASTATIC),SW_HIDE);
		::ShowWindow(GetDlgItem(IDC_DATACHECK),SW_HIDE);
		::ShowWindow(GetDlgItem(IDC_DATAPROGRESS),SW_HIDE);
		::ShowWindow(GetDlgItem(IDC_CONDITIONEDIT),SW_HIDE);
		::ShowWindow(GetDlgItem(IDC_SEARCHBTN),SW_HIDE);

		if ( _stricmp(m_strTabName,"PlayList") == 0 ) // playlist tab
		{
			InitPlayListListCtrl();
		}
		else
		{
			InitOtherListCtrl();
		}
	}
	bHandled = FALSE;
	return S_OK;
}

void CDataDialog::InitPlayListListCtrl()
{
	TCHAR szTabName[XMLDATA_LEN]={0};
	TCHAR szDllName[XMLDATA_LEN] ={0};
		
	// Get Grid Fun's name 
#if defined _UNICODE || defined UNICODE
	MultiByteToWideChar(
				 CP_ACP,         // code page
				 0,              // character-type options
				 gXMLFileData.strFuncDso.c_str(),  // address of string to map
				 strlen(gXMLFileData.strFuncDso.c_str()),      // number of bytes in string
				 szTabName,       // address of wide-character buffer
				 XMLDATA_LEN);             // size of buffer);

#else
	sprintf(szTabName,"%s",gXMLFileData.strFuncDso.c_str());
#endif
	if  ( _tcsstr(szTabName,_T("%") )!= NULL )
	{
			TCHAR szTemp[XMLDATA_LEN]={0};
			ExpandEnvironmentStrings(szTabName, szTemp, XMLDATA_LEN);
			memset(szTabName,0,sizeof(szTabName));
			_stprintf(szTabName,_T("%s"),szTemp);
	}
	_stprintf(szDllName,_T("%s"),szTabName);


	// Get Data from dll,then add datas to UI Desktop
//	typedef ITEMDATA ** (CALLBACK *GetData_Proc)(ATTRIBSDATA **pAttribsData, int *iAttCount,ITEMDATA **pColumnNames,int *iRow, int *iCol,ITEMDATA **pCellsData);
//	typedef ITEMDATA ** (CALLBACK *GetData_Proc)(GRIDDATAARRAY2 & AttribsData,ITEMDATA **pColumnNames,int *iRow, int *iCol,ITEMDATA **pCellsData);
//	typedef	ITEMDATA ** (CALLBACK *GetData_Proc)(char * cCondition,char * cTabname,GRIDDATAARRAY2 & AttribsData,ITEMDATA **pColumnNames,int *iRow, int *iCol,ITEMDATA **pCellsData);
//	typedef	ITEMDATA ** (CALLBACK *GetData_Proc)(char * cCondition,char * cTabname,GRIDDATAARRAY2 & AttribsData,ITEMDATA **pColumnNames,int *iRow, int *iCol,ITEMDATA **pCellsData,RegProgrssProc proc);
	typedef ITEMDATA ** (CALLBACK *GetData_Proc)(char * cCondition,char * cTabname,GRIDDATAARRAY2 & AttribsData,ITEMDATA **pColumnNames,int *iRow, int *iCol,ITEMDATA **pCellsData,RegProgrssProc proc,RegPlayListStateProc stateProc);

	GetData_Proc hGetData = NULL;
	// modify hLib to global var
//	HINSTANCE hLib;
//	hLib = LoadLibrary(szDllName);
//  if ( hLib)
	if ( !m_hLib )
	{
		m_hLib = LoadLibrary(szDllName);
	}

	if ( m_hLib)
	{
		//hGetData = (GetData_Proc)GetProcAddress(hLib,gXMLFileData.strGridFunc.c_str());
		hGetData = (GetData_Proc)GetProcAddress(m_hLib,gXMLFileData.strGridFunc.c_str());
		if ( hGetData)
		{
		
			int iRow,iCol;
			ITEMDATA **pCellsData = NULL;
			ITEMDATA * pColumnsName = NULL;
//				int iAttribsCount; ATTRIBSDATA *pAttribsData = NULL; // this is the old operation

			GRIDDATAARRAY2 m_AttribesDatas; 
			m_AttribesDatas.clear();
			
//				pCellsData = (*hGetData)(&pAttribsData,&iAttribsCount,&pColumnsName,&iRow,&iCol,pCellsData);
			pCellsData = (*hGetData)("*",m_strTabName,m_AttribesDatas,&pColumnsName,&iRow,&iCol,pCellsData,NULL,GetPlayListStateProc);
		//	if ( iRow > 0 && iCol > 0 )
			if ( iCol > 0 )
			{
				m_iNumColumns = iCol ;
			
				int i = 0;
				for ( i = 0; i < iCol; i ++)
				{
					m_pListCtrl->AddColumn((LPCTSTR)(pColumnsName[i].szItemData),i);
				}

				for(i=0;i<iRow;i++)
				{
					for(int j=0;j<iCol;j++)
					{
						m_pListCtrl->AddItem(i,j,(LPCTSTR)((pCellsData[i][j]).szItemData));
					}
				}

				// free the columns's memory
				for (  i = 0; i < iCol; i ++)
				{
					free(pColumnsName[i].szItemData);
				}
				delete  pColumnsName;

				// free the cells's memory
				for( i=0;i<iRow;i++)
				{
					for( int j=0;j<iCol;j++)
					{
						free((pCellsData[i][j]).szItemData);
					}
				}
				for( i=0;i<iRow;i++)
				{
					delete [] pCellsData[i]; //释放列
				}
				delete pCellsData; //释放行
			}
		
		}
	//	FreeLibrary(hLib); // modify hLib to global var
	}
}

void CDataDialog::InitOtherListCtrl()
{
	TCHAR szTabName[XMLDATA_LEN]={0};
	TCHAR szDllName[XMLDATA_LEN] ={0};
		
	// Get Grid Fun's name 
#if defined _UNICODE || defined UNICODE
	MultiByteToWideChar(
				 CP_ACP,         // code page
				 0,              // character-type options
				 gXMLFileData.strFuncDso.c_str(),  // address of string to map
				 strlen(gXMLFileData.strFuncDso.c_str()),      // number of bytes in string
				 szTabName,       // address of wide-character buffer
				 XMLDATA_LEN);             // size of buffer);

#else
	sprintf(szTabName,"%s",gXMLFileData.strFuncDso.c_str());
#endif
	if  ( _tcsstr(szTabName,_T("%") )!= NULL )
	{
			TCHAR szTemp[XMLDATA_LEN]={0};
			ExpandEnvironmentStrings(szTabName, szTemp, XMLDATA_LEN);
			memset(szTabName,0,sizeof(szTabName));
			_stprintf(szTabName,_T("%s"),szTemp);
	}
	_stprintf(szDllName,_T("%s"),szTabName);


	// Get Data from dll,then add datas to UI Desktop
//	typedef ITEMDATA ** (CALLBACK *GetData_Proc)(ATTRIBSDATA **pAttribsData, int *iAttCount,ITEMDATA **pColumnNames,int *iRow, int *iCol,ITEMDATA **pCellsData);
//	typedef ITEMDATA ** (CALLBACK *GetData_Proc)(GRIDDATAARRAY2 & AttribsData,ITEMDATA **pColumnNames,int *iRow, int *iCol,ITEMDATA **pCellsData);
//	typedef	ITEMDATA ** (CALLBACK *GetData_Proc)(char * cCondition,char * cTabname,GRIDDATAARRAY2 & AttribsData,ITEMDATA **pColumnNames,int *iRow, int *iCol,ITEMDATA **pCellsData);
//  typedef	ITEMDATA ** (CALLBACK *GetData_Proc)(char * cCondition,char * cTabname,GRIDDATAARRAY2 & AttribsData,ITEMDATA **pColumnNames,int *iRow, int *iCol,ITEMDATA **pCellsData,RegProgrssProc proc);
	typedef ITEMDATA ** (CALLBACK *GetData_Proc)(char * cCondition,char * cTabname,GRIDDATAARRAY2 & AttribsData,ITEMDATA **pColumnNames,int *iRow, int *iCol,ITEMDATA **pCellsData,RegProgrssProc proc,RegPlayListStateProc stateProc);
	
	GetData_Proc hGetData = NULL;
	// modify hLib to global var
//	HINSTANCE hLib;
//	hLib = LoadLibrary(szDllName);
//  if ( hLib)
	if ( !m_hLib )
	{
		m_hLib = LoadLibrary(szDllName);
	}

	if ( m_hLib)
	{
		//hGetData = (GetData_Proc)GetProcAddress(hLib,gXMLFileData.strGridFunc.c_str());
		hGetData = (GetData_Proc)GetProcAddress(m_hLib,gXMLFileData.strGridFunc.c_str());
		if ( hGetData)
		{
		
			int iRow,iCol;
			ITEMDATA **pCellsData = NULL;
			ITEMDATA * pColumnsName = NULL;
//				int iAttribsCount; ATTRIBSDATA *pAttribsData = NULL; // this is the old operation

			GRIDDATAARRAY2 m_AttribesDatas; 
			m_AttribesDatas.clear();
			
//				pCellsData = (*hGetData)(&pAttribsData,&iAttribsCount,&pColumnsName,&iRow,&iCol,pCellsData);
			pCellsData = (*hGetData)("*",m_strTabName,m_AttribesDatas,&pColumnsName,&iRow,&iCol,pCellsData,NULL,NULL);
			m_pListCtrl->SetAttribeCount(m_AttribesDatas.size());
			if ( iCol > 0 )
			{
				m_iNumColumns = iCol ;
			
				int i = 0;
				for ( i = 0; i < iCol; i ++)
				{
					m_pListCtrl->AddColumn((LPCTSTR)(pColumnsName[i].szItemData),i);
				}

				for(i=0;i<iRow;i++)
				{
					for(int j=0;j<iCol;j++)
					{
						m_pListCtrl->AddItem(i,j,(LPCTSTR)((pCellsData[i][j]).szItemData));
					}
				}

				// free the columns's memory
				for (  i = 0; i < iCol; i ++)
				{
					free(pColumnsName[i].szItemData);
				}
				delete  pColumnsName;

				// free the cells's memory
				for( i=0;i<iRow;i++)
				{
					for( int j=0;j<iCol;j++)
					{
						free((pCellsData[i][j]).szItemData);
					}
				}
				for( i=0;i<iRow;i++)
				{
					delete [] pCellsData[i]; //释放列
				}
				delete pCellsData; //释放行
			}
		
		}
	//	FreeLibrary(hLib); // modify hLib to global var
	}
}

void CDataDialog::InitContentListCtrl()
{
	int iCount = m_pListCtrl->GetItemCount();
	if ( iCount > 0 )
	{
		m_pListCtrl->DeleteAllItems();
	}
	
	TCHAR szTabName[XMLDATA_LEN] ={0};
	TCHAR szDllName[XMLDATA_LEN] ={0};

// Get Grid Fun's name 
#if defined _UNICODE || defined UNICODE
		MultiByteToWideChar(
					 CP_ACP,         // code page
					 0,              // character-type options
					 gXMLFileData.strFuncDso.c_str(),  // address of string to map
					 strlen(gXMLFileData.strFuncDso.c_str()),      // number of bytes in string
					 szTabName,       // address of wide-character buffer
					 XMLDATA_LEN);             // size of buffer);

#else
		sprintf(szTabName,"%s",gXMLFileData.strFuncDso.c_str());
#endif
		if  ( _tcsstr(szTabName,_T("%") )!= NULL )
		{
				TCHAR szTemp[XMLDATA_LEN]={0};
				ExpandEnvironmentStrings(szTabName, szTemp, XMLDATA_LEN);
				memset(szTabName,0,sizeof(szTabName));
				_stprintf(szTabName,_T("%s"),szTemp);
		}
		_stprintf(szDllName,_T("%s"),szTabName);


		// Get Data from dll,then add datas to UI Desktop
		typedef ITEMDATA ** (CALLBACK *GetData_Proc)(char * cCondition,char * cTabname,GRIDDATAARRAY2 & AttribsData,ITEMDATA **pColumnNames,int *iRow, int *iCol,ITEMDATA **pCellsData,RegProgrssProc proc,RegPlayListStateProc stateProc);

		GetData_Proc hGetData = NULL;
		// modify hLib to global var
//		HINSTANCE hLib;
//		hLib = LoadLibrary(szDllName);
//		if ( hLib)
		if ( !m_hLib )
		{
			m_hLib = LoadLibrary(szDllName);
		}
		if ( m_hLib )
		{
		//	hGetData = (GetData_Proc)GetProcAddress(hLib,gXMLFileData.strGridFunc.c_str()); // for the test
			hGetData = (GetData_Proc)GetProcAddress(m_hLib,gXMLFileData.strGridFunc.c_str());
			if ( hGetData)
			{
			
				int iRow,iCol,i,j;
				ITEMDATA **pCellsData = NULL;
				ITEMDATA * pColumnsName = NULL;

				GRIDDATAARRAY2 m_AttribesDatas; 
				m_AttribesDatas.clear();
				char sText[NAEMELEN]={0};
				m_ConditionEdit.GetWindowText(sText,NAEMELEN);
					
				pCellsData = (*hGetData)(sText,m_strTabName,m_AttribesDatas,&pColumnsName,&iRow,&iCol,pCellsData,GetProgressProc,NULL);
				if ( iCol > 0 )
				{
					m_iNumColumns = iCol ;

					// add by 20070423 for save the data
					FreeMemoryData();
					m_iNumRows = iRow;
					m_pCellsData = new ITEMDATA* [m_iNumRows];
					for ( j = 0; j < (m_iNumRows); j ++)
					{
						m_pCellsData[j] = new ITEMDATA[m_iNumColumns];
					}
	
					for( i=0;i<(m_iNumRows);i++)
					{
						for( j=0;j<(m_iNumColumns);j++)
						{
							(m_pCellsData[i][j]).szItemData = (char*)malloc(ITEMLEN);
							sprintf((m_pCellsData[i][j]).szItemData,"%s",(pCellsData[i][j]).szItemData);
						}
					}
					// add by 20070423 for save the data
					if ( m_iBtnSearchNum ==1 )
					{
						for ( i = 0; i < iCol; i ++)
						{
							m_pListCtrl->AddColumn((LPCTSTR)(pColumnsName[i].szItemData),i);
						}
					}
					for(i=0;i<iRow;i++)
					{
						for( j=0;j<iCol;j++)
						{
							if ( m_bShowAllData )
							{
								m_pListCtrl->AddItem(i,j,(LPCTSTR)((pCellsData[i][j]).szItemData));
							}
							else
							{
								if ( j == 0)
								{
									m_pListCtrl->AddItem(i,j,(LPCTSTR)((pCellsData[i][j]).szItemData));
								}
							}
						}
					}
	
					// free the columns's memory
					for (  i = 0; i < iCol; i ++)
					{
						free(pColumnsName[i].szItemData);
					}
					delete  pColumnsName;

					// free the cells's memory
					for( i=0;i<iRow;i++)
					{
						for( int j=0;j<iCol;j++)
						{
							free((pCellsData[i][j]).szItemData);
						}
					}
					for( i=0;i<iRow;i++)
					{
						delete [] pCellsData[i]; //释放列
					}
					delete pCellsData; //释放行
				}
			
//			FreeLibrary(hLib);// modify hLib to global var
		}
	}
}

void CDataDialog::RefreshListCtrl()
{
	int iCount = m_pListCtrl->GetItemCount();
	if ( iCount > 0 )
	{
		m_pListCtrl->DeleteAllItems();
		for(int i=0;i<m_iNumRows;i++)
		{
			for(int j=0;j<m_iNumColumns;j++)
			{
				
				if ( m_bShowAllData )
				{
					m_pListCtrl->AddItem(i,j,(LPCTSTR)((m_pCellsData[i][j]).szItemData));
				}
				else
				{
					if ( j == 0)
					{
						m_pListCtrl->AddItem(i,j,(LPCTSTR)((m_pCellsData[i][j]).szItemData));
					}
				}
			}
		}
	}
}

LRESULT CDataDialog::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	RECT rcClientRect;
	GetClientRect(&rcClientRect);
	if ( m_bShowAboveCtrl )
	{
		RECT rc1,rc2;
		m_SearchBtn.GetWindowRect(&rc1);
		ScreenToClient(&rc1);

		rc2.left = rcClientRect.left;
		rc2.bottom = rcClientRect.bottom;
		rc2.top = rc1.bottom + 5;
		rc2.right = rcClientRect.right;
		m_pListCtrl->MoveWindow(&rc2);
	}
	else
	{
		m_pListCtrl->MoveWindow(&rcClientRect);
	}
	if ( m_iNumColumns > 0 )
	{
		int icx;
		icx =  ( rcClientRect.right - rcClientRect.left ) / m_iNumColumns;
		for ( int i =0; i < m_iNumColumns; i ++ )
		{		
			m_pListCtrl->SetColumnWidth(i,icx);
		}
	}
	bHandled = FALSE;
	return S_OK;
}

LRESULT CDataDialog::OnSearch(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_ProgressCtrl.SetRange(0,0);
	m_ProgressCtrl.SetPos(0);
	m_iBtnSearchNum ++;
	InitContentListCtrl();
	bHandled = FALSE;
	return S_OK;
}

LRESULT CDataDialog::OnDataBtn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int iCheckValue;
	iCheckValue = m_CheckBtn.GetCheck();
	
	if ( iCheckValue == 1)
	{
		m_CheckBtn.SetCheck(1);
		m_bShowAllData = TRUE;
	}
	else
	{
		m_CheckBtn.SetCheck(0);
		m_bShowAllData = FALSE;
	}
	RefreshListCtrl();
	bHandled = FALSE;
	return S_OK;
}

int CALLBACK  CDataDialog::GetProgressProc( const int &  iTotalNum, const int  & iCurNum)
{
	if ( iCurNum == 1)
	{
		m_ProgressCtrl.SetRange(0,iTotalNum);
	}
	m_ProgressCtrl.SetPos(iCurNum);
	return 0;
}

bool CDataDialog::IsExist(const string & strUid,int * iIndex)
{
	bool bExist = false;
	int iCount = m_pListCtrl->GetItemCount();
	*iIndex = -1;
	if ( iCount > 0 )
	{
		char szText[ITEMLEN]={0};
		for ( int i = 0; i < iCount; i ++ )
		{
			m_pListCtrl->GetItemText(i,0,szText,ITEMLEN);
			if ( _stricmp(szText,strUid.c_str()) == 0)
			{
				bExist = true;
				*iIndex = i;
				break;
			}
		}
	}
	return bExist;
}

int CALLBACK  CDataDialog::GetPlayListStateProc(const string & strUid, const string & strMsg, const string & strStateValue, const int & iCurCtrlNum )
{
	if ( _stricmp(strMsg.c_str(),"OnExit") == 0 ) // playlist onExit msg to destroy item
	{
		int iIndex;
		if ( IsExist(strUid,&iIndex) )
		{
			m_pListCtrl->DeleteItem(iIndex);
		}
	}
	else if ( _stricmp(strMsg.c_str(),"OnStateChanged" ) == 0) // playlist statechagne to add item
	{
		int iCount = m_pListCtrl->GetItemCount();
		int iIndex ;
		if ( !IsExist(strUid,&iIndex) )
		{
			TCHAR szTabName[XMLDATA_LEN]={0};
			TCHAR szDllName[XMLDATA_LEN] ={0};
		
			// Get Grid Fun's name 
		#if defined _UNICODE || defined UNICODE
			MultiByteToWideChar(
						 CP_ACP,         // code page
						 0,              // character-type options
						 gXMLFileData.strFuncDso.c_str(),  // address of string to map
						 strlen(gXMLFileData.strFuncDso.c_str()),      // number of bytes in string
						 szTabName,       // address of wide-character buffer
						 XMLDATA_LEN);             // size of buffer);

		#else
			sprintf(szTabName,"%s",gXMLFileData.strFuncDso.c_str());
		#endif
			if  ( _tcsstr(szTabName,_T("%") )!= NULL )
			{
					TCHAR szTemp[XMLDATA_LEN]={0};
					ExpandEnvironmentStrings(szTabName, szTemp, XMLDATA_LEN);
					memset(szTabName,0,sizeof(szTabName));
					_stprintf(szTabName,_T("%s"),szTemp);
			}
			_stprintf(szDllName,_T("%s"),szTabName);


			// Get Data from dll,then add datas to UI Desktop
			typedef int  (CALLBACK *GetPlayListData_Proc)(const char * cUid,const char * cStateValue,ITEMDATA **pColumnNames,int *iCol);
			GetPlayListData_Proc hGetData = NULL;

			// modify hLib to global var
		//	HINSTANCE hLib;
		//	hLib = LoadLibrary(szDllName);
		//  if ( hLib)
			if ( !m_hLib )
			{
				m_hLib = LoadLibrary(szDllName);
			}

			if ( m_hLib)
			{
				hGetData = (GetPlayListData_Proc)GetProcAddress(m_hLib,"GetPlayListData_Proc");
				if ( hGetData)
				{
					int iCol,iReturn ;
					ITEMDATA * pColumnsName = NULL;
					iReturn = (*hGetData)(strUid.c_str(),strStateValue.c_str(),&pColumnsName,&iCol);
			
					if (  iCol > 0 )
					{
						
						int i = 0;
						for ( i = 0; i < iCol; i ++)
						{
							m_pListCtrl->AddItem(iCount,i,(LPCTSTR)(pColumnsName[i].szItemData)); // for the test
						}

						// free the columns's memory
						for (  i = 0; i < iCol; i ++)
						{
							free(pColumnsName[i].szItemData);
						}
						delete  pColumnsName;
					}
				
				}
			//	FreeLibrary(hLib); // modify hLib to global var
			}
		}
	}
	else if ( _stricmp(strMsg.c_str(),"OnProgress") == 0 ) // playlist onprogress to update item
	{
		char szTemp[10]={0};
		itoa(iCurCtrlNum,szTemp,10);

		int iIndex = 0;
		if ( IsExist(strUid,&iIndex) )
		{
			m_pListCtrl->SetItemText(iIndex,7,(LPCTSTR)szTemp);
//			m_pListCtrl->SetItemText(iIndex,2,(LPCTSTR)"2222");
//			m_pListCtrl->SetItemText(iIndex,7,(LPCTSTR)"7777");
		}
	}
	return 0;
}
