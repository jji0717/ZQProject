// AttribeDialog.cpp: implementation of the CAttribeDialog class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AttribeDialog.h"
//GRIDDATAARRAY2  m_AttribesDatas;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CAttribeDialog::CAttribeDialog(BOOL bListCtrlMode,LPCTSTR szTabName,LPCTSTR szParentTabName)
{
	m_iAttrsNum = -1;
	m_bListCtrlMode = bListCtrlMode;
	m_pListCtrl = NULL;
	m_pEdit = NULL;
	m_pStaic = NULL;
	m_strWindowName = szTabName;
	memset(m_strParentTabName,0,sizeof(m_strParentTabName));
	sprintf(m_strParentTabName,"%s",szParentTabName);
}

CAttribeDialog::~CAttribeDialog()
{
	if ( m_iAttrsNum >  0)
	{
		if ( m_bListCtrlMode)
		{
			if ( m_pListCtrl->IsWindow())
			{
				m_pListCtrl->DestroyWindow();
			}
			delete m_pListCtrl;
			m_pListCtrl = NULL;
		}
		else
		{
			for ( int i =0; i < m_iAttrsNum; i ++ )
			{
				if ( m_pStaic[i].IsWindow())
				{
					m_pStaic[i].DestroyWindow();
				}
				if ( m_pEdit[i].IsWindow())
				{
					m_pEdit[i].DestroyWindow();
				}
			}
			delete [] m_pStaic;
			m_pStaic = NULL;
			delete [] m_pEdit;
			m_pEdit = NULL;
		}
	}
}


LRESULT CAttribeDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	InitControl();
	bHandled = FALSE;
	return S_OK;
}

LRESULT CAttribeDialog::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	if ( m_iAttrsNum > 0 )
	{

		RECT rcClientRect;
		GetClientRect(&rcClientRect);

		if ( m_bListCtrlMode)
		{
			/*
			int icx,icx1;
			icx = 80;
//			icx1 = ( rcClientRect.right - rcClientRect.left ) /2;
			icx1 = ( rcClientRect.right - rcClientRect.left - icx);

			m_pListCtrl->SetColumnWidth(0,icx);
			m_pListCtrl->SetColumnWidth(1,icx1);
			m_pListCtrl->MoveWindow(&rcClientRect);
			*/
			int icx;
			icx = ( rcClientRect.right - rcClientRect.left) / m_iAttrsNum;
			for ( int i =0; i < m_iAttrsNum; i ++)
			{
				m_pListCtrl->SetColumnWidth(i,icx);
			}
			m_pListCtrl->MoveWindow(&rcClientRect);
		}
		else
		{
			CDCHandle hdc =GetDC();
			int iWidth =  ( rcClientRect.right - rcClientRect.left ) / 4;

			int iRowNum ;

			if ( m_iAttrsNum % 2 == 0 )
			{
				iRowNum = m_iAttrsNum / 2;
			}
			else
			{
				iRowNum = ( m_iAttrsNum + 1) / 2;
			}
			int iHight =  ( ( rcClientRect.bottom - rcClientRect.top ) - ( iRowNum + 1 )* 8 ) / iRowNum ;


			SIZE sizeValue;
			TCHAR szTemp[] = _T("attribs  Names");
			hdc.GetTextExtent(szTemp,lstrlen(szTemp),&sizeValue);
			int  iStaticWidth = (int)sizeValue.cx;
			int  iStaticHigth = (int)sizeValue.cy;
			TEXTMETRIC  tm ;
			::GetTextMetrics (hdc, &tm) ;

			RECT *prc1,*prc2;
			prc1 = new RECT[m_iAttrsNum];
			prc2 = new RECT[m_iAttrsNum];
			
			for ( int i = 0; i < m_iAttrsNum; i ++ )
			{
				if ( i %2 == 0 ) // 前面的两个Control
				{
					prc1[i].left  = rcClientRect.left + 10 ;
					prc1[i].right = prc1[i].left + iStaticWidth;
					if ( i == 0 )
					{
						prc1[i].top  = rcClientRect.top + 5 ;
					}
					else 
					{
						prc1[i].top  = prc1[i-1].bottom + 8 ; 
					}
					prc1[i].bottom = prc1[i].top + iStaticHigth;

					prc2[i].left   = prc1[i].right + 10;
					prc2[i].top    = prc1[i].top;
					prc2[i].bottom = prc1[i].bottom + 6;
					prc2[i].right  = prc2[i].left + iWidth;
				}
				else // 后面的两个Control
				{
					prc1[i].left   = prc2[i-1].right + 20;
					prc1[i].right  = prc1[i].left + iStaticWidth;
					prc1[i].top    = prc1[i-1].top;
					prc1[i].bottom = prc1[i-1].bottom;

					prc2[i].left = prc1[i].right + 10;
					prc2[i].top = prc1[i].top;
					prc2[i].bottom = prc1[i].bottom + 6;
					prc2[i].right = prc2[i].left + iWidth;
				}
				m_pStaic[i].MoveWindow(&prc1[i]);
				m_pEdit[i].MoveWindow(&prc2[i]);
			}
			if ( prc1)
			{
				delete [] prc1;
			}
			prc1 = NULL;

			if ( prc2)
			{
				delete [] prc2;
			}
			prc2 = NULL;
			ReleaseDC(hdc);
		}
		
	}
	bHandled = FALSE;
	return S_OK;
}

void CAttribeDialog::InitControl()
{
	RECT rcClientRect;
	GetClientRect(&rcClientRect);
	
	DWORD dwStyle,dwExStyle;
	int icx,iRowNum;
	UINT nMask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	UINT nFmt = LVCFMT_LEFT | LVCFMT_BITMAP_ON_RIGHT;
	int i = 0,j;
	int iSize;
	string strTempData;


//	if ( (  _stricmp(m_strParentTabName,"Streamer1") == 0 )  || (  _stricmp(m_strParentTabName,"Streamer2") == 0  )) // Streamer's StorageLink or StreamLink Tab
	{
	
		TCHAR szTabName[XMLDATA_LEN]={0};
		TCHAR szDllName[XMLDATA_LEN] ={0};
		
		// Get Attris Fun's name 
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
	#ifdef PTRMODE
	//		typedef ITEMDATA ** (CALLBACK *GetData_Proc)(ATTRIBSDATA **pAttribsData, int *iAttCount,ITEMDATA **pColumnNames,int *iRow, int *iCol,ITEMDATA **pCellsData);
	//		typedef ITEMDATA ** (CALLBACK *GetData_Proc)(GRIDDATAARRAY2 & AttribsData,ITEMDATA **pColumnNames,int *iRow, int *iCol,ITEMDATA **pCellsData);
	//		typedef	ITEMDATA ** (CALLBACK *GetData_Proc)(char * cCondition,char * cTabname,GRIDDATAARRAY2 & AttribsData,ITEMDATA **pColumnNames,int *iRow, int *iCol,ITEMDATA **pCellsData);
	//		typedef	ITEMDATA ** (CALLBACK *GetData_Proc)(char * cCondition,char * cTabname,GRIDDATAARRAY2 & AttribsData,ITEMDATA **pColumnNames,int *iRow, int *iCol,ITEMDATA **pCellsData,RegProgrssProc proc);
			typedef ITEMDATA ** (CALLBACK *GetData_Proc)(char * cCondition,char * cTabname,GRIDDATAARRAY2 & AttribsData,ITEMDATA **pColumnNames,int *iRow, int *iCol,ITEMDATA **pCellsData,RegProgrssProc proc,RegPlayListStateProc stateProc);
	#else
			typedef int (CALLBACK *GetData_Proc)(ATTRISVECTOR & attribsData,int *ColumnCount,STRVECTOR &ColumnNames, int *RowCount, GRIDDATAARRAY & CellsData);
	#endif
			GetData_Proc hGetData = NULL;
			// modify hLib to global var
//			HINSTANCE hLib;
//			hLib = LoadLibrary(szDllName);
//			if ( hLib)
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
				#ifdef PTRMODE
					int iRow,iCol; 
					ITEMDATA **pCellsData = NULL;
					ITEMDATA * pColumnsName = NULL;
	//				int iAttribsCount;ATTRIBSDATA *pAttribsData = NULL; // This is the old operation style
					GRIDDATAARRAY2 AttribesDatas;
					AttribesDatas.clear();
					
	//				pCellsData = (*hGetData)(&pAttribsData,&iAttribsCount,&pColumnsName,&iRow,&iCol,pCellsData);
	//				pCellsData = (*hGetData)(AttribesDatas,&pColumnsName,&iRow,&iCol,pCellsData);
					pCellsData = (*hGetData)("*",m_strParentTabName,AttribesDatas,&pColumnsName,&iRow,&iCol,pCellsData,NULL,NULL);
					
					iSize = AttribesDatas.size();

					
				//	m_iAttrsNum = iAttribsCount;

					if (  m_bListCtrlMode )
					{
						dwStyle   =  WS_CHILD|WS_VSCROLL|LVS_REPORT|WS_HSCROLL|LVS_SHOWSELALWAYS ;
						dwExStyle =  LVS_EX_FULLROWSELECT| LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP|WS_EX_STATICEDGE;
							
						m_pListCtrl = new CListViewCtrl();
						m_pListCtrl->Create(*this,&rcClientRect,NULL,dwStyle,dwExStyle);
						//	m_pListCtrl->Create(*this);
						m_pListCtrl->SetExtendedListViewStyle(dwExStyle);
						m_pListCtrl->SetParent(*this);
						m_pListCtrl->MoveWindow(&rcClientRect);
						m_pListCtrl->ShowWindow(SW_SHOWNORMAL);
						icx = ( rcClientRect.right - rcClientRect.left ) /2;

						if ( !AttribesDatas.empty() )
						{

							GRIDDATAARRAYITOR2  array2Itor;
							array2Itor    = AttribesDatas.find(m_strWindowName);
							if ( array2Itor != AttribesDatas.end())
							{
								GRIDDATAARRAY  AllData;
								GRIDDATAARRAYITOR itorTmp;
								int iIndex =0;
								AllData.clear();
								AllData =  (*array2Itor).second;
								iSize = AllData.size();
								
								for ( itorTmp = AllData.begin(); itorTmp != AllData.end(); itorTmp ++ )
								{
									STRVECTOR OneRowData;
									iRowNum = (*itorTmp).first;
									OneRowData =(*itorTmp).second;
									if ( iRowNum == 0 ) // 取得列名
									{
										iSize = OneRowData.size();
										m_iAttrsNum = iSize;
										for (  j = 0; j < iSize; j ++ )
										{
											strTempData = OneRowData[j];
											m_pListCtrl->AddColumn((LPCTSTR)strTempData.c_str(),j);
											m_pListCtrl->SetColumnWidth(j,icx);
										}
									}
									else //取得列表数据
									{
										iSize = OneRowData.size();
										for (  j = 0; j < iSize; j ++ )
										{
											strTempData = OneRowData[j];
											m_pListCtrl->AddItem(iRowNum-1,j,(LPCTSTR)strTempData.c_str());
										}
									}
								}
								AllData.clear();
							}
						}
					}
					else
					{
					
						string strStaticText,strEditText;
						CDCHandle hdc =GetDC();
						int iWidth =  ( rcClientRect.right - rcClientRect.left ) / 4;
						int iRowNum ;
						if ( m_iAttrsNum % 2 == 0 )
						{
							iRowNum = m_iAttrsNum / 2;
						}
						else
						{
							iRowNum = ( m_iAttrsNum + 1) / 2;
						}
						int iHight =  ( ( rcClientRect.bottom - rcClientRect.top ) - ( iRowNum + 1 )* 8 ) / iRowNum ;
					
						SIZE sizeValue;
						TCHAR szTemp[] = _T("attribs  Names");
						TCHAR szTemp1[] =_T("Value");
						hdc.GetTextExtent(szTemp,lstrlen(szTemp),&sizeValue);
						LONG iStaticWidth = sizeValue.cx;
						LONG iStaticHigth = sizeValue.cy;
					
						
						RECT *prc1,*prc2;
						m_pStaic = new CStatic[m_iAttrsNum];
						m_pEdit  = new CEdit[m_iAttrsNum];
						prc1 = new RECT[m_iAttrsNum];
						prc2 = new RECT[m_iAttrsNum];

						
						UINT nEdit ;

						for (  i =0; i < m_iAttrsNum; i ++)
						{
						
							if ( i %2 == 0 ) // 前面的两个Control
							{
								prc1[i].left  = rcClientRect.left + 10 ;
								prc1[i].right = prc1[i].left + iStaticWidth;
								if ( i == 0 )
								{
									prc1[i].top  = rcClientRect.top + 5 ;
								}
								else 
								{
									prc1[i].top  = prc1[i-1].bottom + 8 ; 
								}
								prc1[i].bottom = prc1[i].top + iStaticHigth;

								prc2[i].left   = prc1[i].right + 10;
								prc2[i].top    = prc1[i].top;
								prc2[i].bottom = prc1[i].bottom + 6;
								prc2[i].right  = prc2[i].left + iWidth;
							}
							else // 后面的两个Control
							{
								prc1[i].left   = prc2[i-1].right + 20;
								prc1[i].right  = prc1[i].left + iStaticWidth;
								prc1[i].top    = prc1[i-1].top;
								prc1[i].bottom = prc1[i-1].bottom;

								prc2[i].left = prc1[i].right + 10;
								prc2[i].top = prc1[i].top;
								prc2[i].bottom = prc1[i].bottom + 6;
								prc2[i].right = prc2[i].left + iWidth;
							}
							
							
	//						strStaticText = pAttribsData[i].szAttribsName;
	//						strEditText = pAttribsData[i].szAttisbsValue;
							
							dwStyle = WS_CHILD | WS_VISIBLE;
							dwExStyle = 0;// WS_EX_STATICEDGE;//WS_EX_CLIENTEDGE;//WS_EX_OVERLAPPEDWINDOW ;
							m_pStaic[i].Create(*this,&prc1[i],_T(""),dwStyle,dwExStyle);
							m_pStaic[i].SetWindowText(strStaticText.c_str());

							dwStyle = WS_CHILD | WS_VISIBLE | ES_LEFT | ES_WANTRETURN  | WS_BORDER |WS_TABSTOP | ES_AUTOHSCROLL    ;
							dwExStyle = WS_EX_LEFT | WS_EX_OVERLAPPEDWINDOW;

							nEdit = IDC_ATTRIEDIT + i;
							_U_MENUorID menuEdit(nEdit);

							m_pEdit[i].Create(*this,&prc2[i],strEditText.c_str(),dwStyle,dwExStyle,menuEdit);
							m_pEdit[i].SetWindowText(strEditText.c_str());
							
							m_pEdit[i].SetLimitText(150);
							m_pStaic[i].SetParent(*this);
							m_pEdit[i].SetParent(*this);
							m_pStaic[i].MoveWindow(&prc1[i]);
							m_pStaic[i].ShowWindow(SW_SHOWNORMAL);
							m_pEdit[i].MoveWindow(&prc2[i]);
							m_pEdit[i].ShowWindow(SW_SHOWNORMAL);
						}
						if ( prc1)
						{
							delete [] prc1;
						}
						prc1 = NULL;

						if ( prc2)
						{
							delete [] prc2;
						}
						prc2 = NULL;
						ReleaseDC(hdc);
					}
					
					/*
					// free the Attibes's memory
					for (  i = 0; i < iAttribsCount; i ++)
					{
						free(pAttribsData[i].szAttribsName);
						free(pAttribsData[i].szAttisbsValue);
					}
					delete pAttribsData;
					*/
					
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
				#else
					int columnCount =0;
					int rowCount =0;
					STRVECTOR  ColumnData;
					GRIDDATAARRAY AllData;
					GRIDDATAARRAYITOR itorTmp;

					ATTRISVECTOR  attribesVector;
					
					AllData.clear();
					int iReturn;
					iReturn=(*hGetData)(attribesVector,&columnCount,ColumnData,&rowCount,AllData);
					string strTempData;
					
					m_iAttrsNum = attribesVector.size();

					CDCHandle hdc =GetDC();
					int iWidth =  ( rcClientRect.right - rcClientRect.left ) / 4;
					int iRowNum ;
					if ( m_iAttrsNum % 2 == 0 )
					{
						iRowNum = m_iAttrsNum / 2;
					}
					else
					{
						iRowNum = ( m_iAttrsNum + 1) / 2;
					}
					int iHight =  ( ( rcClientRect.bottom - rcClientRect.top ) - ( iRowNum + 1 )* 8 ) / iRowNum ;
				
					SIZE sizeValue;
					TCHAR szTemp[] = _T("attribs  Names");
					TCHAR szTemp1[] =_T("Value");
					hdc.GetTextExtent(szTemp,lstrlen(szTemp),&sizeValue);
					LONG iStaticWidth = sizeValue.cx;
					LONG iStaticHigth = sizeValue.cy;
				
					string strStaticText,strEditText;
					RECT *prc1,*prc2;
					m_pStaic = new CStatic[m_iAttrsNum];
					m_pEdit  = new CEdit[m_iAttrsNum];
					prc1 = new RECT[m_iAttrsNum];
					prc2 = new RECT[m_iAttrsNum];

					DWORD dwStyle,dwExStyle;
					UINT nEdit ;

					for ( int i =0; i < m_iAttrsNum; i ++)
					{
					
						if ( i %2 == 0 ) // 前面的两个Control
						{
							prc1[i].left  = rcClientRect.left + 10 ;
							prc1[i].right = prc1[i].left + iStaticWidth;
							if ( i == 0 )
							{
								prc1[i].top  = rcClientRect.top + 5 ;
							}
							else 
							{
								prc1[i].top  = prc1[i-1].bottom + 8 ; 
							}
							prc1[i].bottom = prc1[i].top + iStaticHigth;

							prc2[i].left   = prc1[i].right + 10;
							prc2[i].top    = prc1[i].top;
							prc2[i].bottom = prc1[i].bottom + 6;
							prc2[i].right  = prc2[i].left + iWidth;
						}
						else // 后面的两个Control
						{
							prc1[i].left   = prc2[i-1].right + 20;
							prc1[i].right  = prc1[i].left + iStaticWidth;
							prc1[i].top    = prc1[i-1].top;
							prc1[i].bottom = prc1[i-1].bottom;

							prc2[i].left = prc1[i].right + 10;
							prc2[i].top = prc1[i].top;
							prc2[i].bottom = prc1[i].bottom + 6;
							prc2[i].right = prc2[i].left + iWidth;
						}
						
						strStaticText = attribesVector[i].strAttrName.c_str();
						strEditText = attribesVector[i].strAttrValue.c_str();
						
						dwStyle = WS_CHILD | WS_VISIBLE;
						dwExStyle = 0;// WS_EX_STATICEDGE;//WS_EX_CLIENTEDGE;//WS_EX_OVERLAPPEDWINDOW ;
						m_pStaic[i].Create(*this,&prc1[i],_T(""),dwStyle,dwExStyle);
						m_pStaic[i].SetWindowText(strStaticText.c_str());

						dwStyle = WS_CHILD | WS_VISIBLE | ES_LEFT | ES_WANTRETURN  | WS_BORDER |WS_TABSTOP | ES_AUTOHSCROLL    ;
						dwExStyle = WS_EX_LEFT | WS_EX_OVERLAPPEDWINDOW;

						nEdit = IDC_ATTRIEDIT + i;
						_U_MENUorID menuEdit(nEdit);

						m_pEdit[i].Create(*this,&prc2[i],strEditText.c_str(),dwStyle,dwExStyle,menuEdit);
						m_pEdit[i].SetWindowText(strEditText);
						
						m_pEdit[i].SetLimitText(150);
						m_pStaic[i].SetParent(*this);
						m_pEdit[i].SetParent(*this);
						m_pStaic[i].MoveWindow(&prc1[i]);
						m_pStaic[i].ShowWindow(SW_SHOWNORMAL);
						m_pEdit[i].MoveWindow(&prc2[i]);
						m_pEdit[i].ShowWindow(SW_SHOWNORMAL);
					}
					if ( prc1)
					{
						delete [] prc1;
					}
					prc1 = NULL;

					if ( prc2)
					{
						delete [] prc2;
					}
					prc2 = NULL;
					ReleaseDC(hdc);
					
					attribesVector.clear();
					ColumnData.clear();
					AllData.clear();
				#endif
				}
			//	FreeLibrary(hLib); // for the test
			}
		}
		/*
		else
		{
			if (  m_bListCtrlMode )
			{
				dwStyle   =  WS_CHILD|WS_VSCROLL|LVS_REPORT|WS_HSCROLL|LVS_SHOWSELALWAYS ;
				dwExStyle =  LVS_EX_FULLROWSELECT| LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP|WS_EX_STATICEDGE;
					
				m_pListCtrl = new CListViewCtrl();
				m_pListCtrl->Create(*this,&rcClientRect,NULL,dwStyle,dwExStyle);
				//	m_pListCtrl->Create(*this);
				m_pListCtrl->SetExtendedListViewStyle(dwExStyle);
				m_pListCtrl->SetParent(*this);
				m_pListCtrl->MoveWindow(&rcClientRect);
				m_pListCtrl->ShowWindow(SW_SHOWNORMAL);
				icx = ( rcClientRect.right - rcClientRect.left ) /2;

				if ( !m_AttribesDatas.empty() )
				{

					GRIDDATAARRAYITOR2  array2Itor;
					array2Itor    = m_AttribesDatas.find(m_strWindowName);
					if ( array2Itor != m_AttribesDatas.end())
					{
						GRIDDATAARRAY  AllData;
						GRIDDATAARRAYITOR itorTmp;
						int iIndex =0;
						AllData.clear();
						AllData =  (*array2Itor).second;
						iSize = AllData.size();
						
						for ( itorTmp = AllData.begin(); itorTmp != AllData.end(); itorTmp ++ )
						{
							STRVECTOR OneRowData;
							iRowNum = (*itorTmp).first;
							OneRowData =(*itorTmp).second;
							if ( iRowNum == 0 ) // 取得列名
							{
								iSize = OneRowData.size();
								m_iAttrsNum = iSize;
								for (  j = 0; j < iSize; j ++ )
								{
									strTempData = OneRowData[j];
									m_pListCtrl->AddColumn((LPCTSTR)strTempData.c_str(),j);
									m_pListCtrl->SetColumnWidth(j,icx);
								}
							}
							else //取得列表数据
							{
								iSize = OneRowData.size();
								for (  j = 0; j < iSize; j ++ )
								{
									strTempData = OneRowData[j];
									m_pListCtrl->AddItem(iRowNum-1,j,(LPCTSTR)strTempData.c_str());
								}
							}
						}
						AllData.clear();
					}
				}
			}
		}
		*/
}