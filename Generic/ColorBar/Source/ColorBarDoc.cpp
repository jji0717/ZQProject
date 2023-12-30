#include "stdafx.h"
#include "ColorBarControl.h"
#include "ColorBarDoc.h"

CColorBarDoc::CColorBarDoc(CColorBarControl *pTestATLControl)
{
	this->m_pColorBarControl  = pTestATLControl;
	this->m_dLinePos          = 0.00;
	this->m_dBarLength        = 3.00; // 初始值为3个小时的长度，这个值可以变动，通过FillColor来变动获取
	this->m_iBarWidthPixes    = iControlWidth; //初始值设置为300;
	this->m_iBarHightPixes    = iControlHigth; //初始值设置为50;
	this->m_dShowRegionLength = 0.00; // 显示的Bar长度初始值为0.0小时
	this->m_iCurUserNum       = 0;    //初始值为0
//	this->m_dLenTimeRatio = (double)(iControlWidth/m_dBarLength);
	this->m_dLenTimeRatio = (double)(m_iBarWidthPixes/m_dBarLength);
	this->m_FillRegionVector.clear();
	this->m_ShowRegionVector.clear();
	this->m_UserLineMap.clear();
	memset((void*)&m_dShowRegionData,0,sizeof(m_dShowRegionData));
}

CColorBarDoc::~CColorBarDoc()
{
	if (!m_FillRegionVector.empty())
	{
		m_FillRegionVector.clear();
	}
	if (!m_ShowRegionVector.empty())
	{
		m_ShowRegionVector.clear();
	}
	if (!m_UserLineMap.empty())
	{
		m_UserLineMap.clear();
	}
	if ( m_pColorBarControl )
	{
		m_pColorBarControl = NULL;
	}
}

double  CColorBarDoc::GetdLinePos() const
{
	return this->m_dLinePos;
}

int   CColorBarDoc::GetiCurUserNum()   
{
	int iSize = m_UserLineMap.size();
	m_iCurUserNum = iSize;
	return this->m_iCurUserNum;
}

void  CColorBarDoc::SetdLinePos(double dPos)
{
	this->m_dLinePos = dPos;
}

void  CColorBarDoc::SetiCurUserNum(int iNum)                             //设置当前的用户数
{
	this->m_iCurUserNum = iNum;
}

void  CColorBarDoc::SetFillRegionDataVector(VECTORTYPE & DataVectorTmp)
{
	m_FillRegionVector.push_back(DataVectorTmp);
	ReCacluteLenTimeRatio();
}

void   CColorBarDoc::SetShowRegionDataVector(VECTORTYPE & DataVectorTmp) //设置ShowReagion的区域容器
{
	this->m_ShowRegionVector.push_back(DataVectorTmp);
}

void   CColorBarDoc::SetUserLineDataMap(int iIDTemp,USERDATASTRUCT & DataMapTmp)  //设置用户画线数据的容器
{
	this->m_UserLineMap[iIDTemp] = DataMapTmp;
}

void   CColorBarDoc::ResetSetUserLineDataMap(int iID)                       //重新设置用户画线数据的容器
{
	if ( !m_UserLineMap.empty())
	{
		MAPITERTMP itorTmp;
		itorTmp    = m_UserLineMap.find(iID);
		m_UserLineMap.erase(itorTmp);
	}
}

double  CColorBarDoc::GetShowBarLength() const
{
	return this->m_dShowRegionLength ;
}

void   CColorBarDoc::SetShowBarLength(double dLength)
{
	this->m_dShowRegionLength = dLength;
}

double  CColorBarDoc::GetBarLength()     const
{
	return this->m_dBarLength;
}

int  CColorBarDoc::GetBarWidthPixes()  const
{
	return this->m_iBarWidthPixes ;
}

int   CColorBarDoc::GetBarHightPixes()   const            // 得到Bar的像素高度
{
	return this->m_iBarHightPixes;
}

void   CColorBarDoc::SetBarLength(double dLength)
{
	this->m_dBarLength = dLength;
}
void   CColorBarDoc::SetBarWidthPixes(int  iBarLength)                    //设置Bar的象素长度,随用户移动改变
{
	this->m_iBarWidthPixes = iBarLength;
}

void   CColorBarDoc::SetBarHightPixes(int iBarHight)                 //设置Bar的象素高度,随用户移动改变
{
	this->m_iBarHightPixes = iBarHight;
}

DATAVECTOR & CColorBarDoc::GetFillRegionDataVector() 
{
	return this->m_FillRegionVector;
}
	
USERDATAMAP & CColorBarDoc::GetUserLineDataMap() // 得到用户画线数据的容器   
{
	return this->m_UserLineMap;
}

DATAVECTOR & CColorBarDoc::GetShowRegionDataVector() // 得到ShowReagion的区域容器
{
	return this->m_ShowRegionVector;

}

SHOWREGIONSTRUCT & CColorBarDoc::GetShowRegionData()
{
	return this->m_dShowRegionData;
}

void  CColorBarDoc::SetShowRegionData(SHOWREGIONSTRUCT & ShowRegionTmp)
{
    this->m_dShowRegionData = ShowRegionTmp;
	double dTemp = m_dShowRegionData.dEndPos - m_dShowRegionData.dStartPos ;
	this->m_dShowRegionLength = dTemp;
//	this->SetShowBarLength(dTemp);
	if ( !m_ShowRegionVector.empty() )
	{
		m_ShowRegionVector.clear();
	}
	this->CreateShowRegionVector(ShowRegionTmp);
	// for the test
//	this->TestResult();
}

double  CColorBarDoc::GetLenTimeRatio() const
{
	return this->m_dLenTimeRatio;
}

void    CColorBarDoc::SetLenTimeRatio(double dRatioTmp)
{
	this->m_dLenTimeRatio = dRatioTmp;
}

void  CColorBarDoc::ReCacluteLenTimeRatio() // 重新计算长度/时间的比率值
{
	int iSize = this->m_FillRegionVector.size();
	/* // 方法1
	if ( iSize > 0 )
	{
		double dStartPos = m_FillRegionVector[0].dStartPos;
		double dEndPos   = m_FillRegionVector[iSize-1].dEndPos;
		double dTemp = dEndPos - dStartPos;
		SetBarLength(dTemp);
		dTemp = (double)(iControlWidth/m_dBarLength);
		SetLenTimeRatio(dTemp);
	}
	*/
	// 方法2
	VECTORITERTMP  itorTemp,itorTemp1;
	if ( !m_FillRegionVector.empty()) // 如果非空
	{
		itorTemp = m_FillRegionVector.begin();
		double dStartPos = (*itorTemp).dStartPos;
		double dEndPos;
		if ( iSize != 1 )
		{
			itorTemp1 = m_FillRegionVector.end();
			--itorTemp1;
			dEndPos   = (*itorTemp1).dEndPos;
		}
		else
		{
			dEndPos = (*itorTemp).dEndPos;
		}
		double dTemp = dEndPos - dStartPos;
		m_dBarLength = dTemp;
//		SetBarLength(dTemp);
//		dTemp = (double)(iControlWidth/m_dBarLength);
		dTemp = (double)(m_iBarWidthPixes/m_dBarLength);
		SetLenTimeRatio(dTemp);
	}
}

//建立ShowRange的容器
void   CColorBarDoc::CreateShowRegionVector(SHOWREGIONSTRUCT & ShowRegionTmp)//建立ShowRange的容器
{
	int iSize = this->m_FillRegionVector.size();
	// 方法1
	int i = 0;
	VECTORTYPE  DataTmp;
	memset((void*)&DataTmp,0,sizeof(DataTmp));
	double dStartPos = ShowRegionTmp.dStartPos;
	double dEndPos   = ShowRegionTmp.dEndPos;
	double dVectorStartPos,dVectorEndPos;

	if ( iSize > 0 )
	{
		for ( i =0; i < iSize ; i ++ )
		{
			dVectorStartPos  = m_FillRegionVector[i].dStartPos;
			dVectorEndPos    = m_FillRegionVector[i].dEndPos;

			if ((  dStartPos >= dVectorStartPos ) && ( dStartPos < dVectorEndPos ) ) // 如果起始点在第一个区域内
			{
				if ( dEndPos <= dVectorEndPos ) // 如果终点也在区域内
				{
					DataTmp.crColor      = m_FillRegionVector[i].crColor;
					DataTmp.dStartPos    = dStartPos;
					DataTmp.dEndPos      = dEndPos;
					DataTmp.bstrFilmName = m_FillRegionVector[i].bstrFilmName;
//					lstrcpy(DataTmp.sFilmName,m_FillRegionVector[i].sFilmName);
					m_ShowRegionVector.push_back(DataTmp);
					break; // 退出循环
				}
				else // 终点不在区域内
				{
					DataTmp.crColor      = m_FillRegionVector[i].crColor;
					DataTmp.dStartPos    = dStartPos;
					DataTmp.dEndPos      = dVectorEndPos;
					DataTmp.bstrFilmName = m_FillRegionVector[i].bstrFilmName; 
//					lstrcpy(DataTmp.sFilmName,m_FillRegionVector[i].sFilmName);
					m_ShowRegionVector.push_back(DataTmp);	
					i ++;
					dVectorStartPos  = m_FillRegionVector[i].dStartPos;
					dVectorEndPos    = m_FillRegionVector[i].dEndPos;
					if ( i >= iSize )
					{
						break;
					}
				}
			}
			if ((  dEndPos >= dVectorStartPos  ) && ( dEndPos < dVectorEndPos ) ) // 如果起始点在第一个区域内
			{
				DataTmp.crColor       = m_FillRegionVector[i].crColor;
				DataTmp.dStartPos     = dVectorStartPos;
				DataTmp.dEndPos       = dEndPos;
				DataTmp.bstrFilmName  = m_FillRegionVector[i].bstrFilmName;
//				lstrcpy(DataTmp.sFilmName,m_FillRegionVector[i].sFilmName);
				m_ShowRegionVector.push_back(DataTmp);
				break; // 退出循环
			}
			else
			{
				DataTmp.crColor       = m_FillRegionVector[i].crColor;
				DataTmp.dStartPos     = dVectorStartPos;
				DataTmp.dEndPos       = dVectorEndPos;
				DataTmp.bstrFilmName  = m_FillRegionVector[i].bstrFilmName;
//				lstrcpy(DataTmp.sFilmName,m_FillRegionVector[i].sFilmName);
				m_ShowRegionVector.push_back(DataTmp);			
			}
		}
	}
}


double  CColorBarDoc::GetFillBarStartPos() const
{
	int iSize = this->m_FillRegionVector.size();
	double bTemp =0.0;
	if ( iSize > 0 )
	{
		bTemp =  m_FillRegionVector[0].dStartPos;
	}
	return bTemp;
}

double  CColorBarDoc::GetFillBarEndPos() const
{
	int iSize = this->m_FillRegionVector.size();
	double bTemp =0.0;
	if ( iSize > 0 )
	{
		bTemp =m_FillRegionVector[iSize-1].dEndPos;
	}
	return bTemp;
}

int  CColorBarDoc::GetMapStartValue()        // 得到最小的用户ID值
{
	int  iSize   = this->m_UserLineMap.size();
	int  iIDTemp = 1;
	if ( iSize > 0 )
	{
		MAPITERTMP itorTmp;
		itorTmp = m_UserLineMap.begin();
		iIDTemp = (*itorTmp).first;
	}
	return iIDTemp;
}

int  CColorBarDoc::GetMapEndValue()          // 得到最大的用户ID值
{
	int iSize   = this->m_UserLineMap.size();
	int iIDTemp = iMaxUserNum;
	if ( iSize > 0 )
	{
		MAPITERTMP itorTmp;
		itorTmp = m_UserLineMap.end();
		--itorTmp;
		iIDTemp = (*itorTmp).first;
	}
	return iIDTemp;
}

int   CColorBarDoc::HourPosConvertToLPos(double dHourPos) // 把小时位置转换成坐标位置
{
	return (int)(dHourPos * m_dLenTimeRatio);
}

bool   CColorBarDoc::bIsExistInMap( COLORREF crColorVal,int *pID) // 判断该用户颜色是否在Map表中已存在
{
	bool bReturn = false;
	if ( m_UserLineMap.empty()) //map表为空
	{
		bReturn = false;
	}
	else
	{
		MAPITERTMP itorTmp;
		COLORREF   crColorTmp;
		for ( itorTmp = m_UserLineMap.begin(); itorTmp != m_UserLineMap.end();  itorTmp++)
		{
			crColorTmp = (itorTmp->second).crColor;
			if ( crColorTmp == crColorVal )
			{
				*pID    = itorTmp->first;
				bReturn = true;
				break;
			}
			else
			{
				bReturn = false;
			}
		}
	}
	if ( !bReturn )
	{
		*pID = 0;
	}
	return bReturn;
}


// 以下过程是编码过程中的调试测试用
void  CColorBarDoc::TestResult()
{
	int iSize;
	/*
	iSize = this->m_ShowRegionVector.size();
	if ( iSize > 0 )
	{
		TCHAR  *s;
		s = new TCHAR[20];
		TCHAR s1[100];
		memset((void*)s1,0,sizeof(s1));
		double dTemp;
		for ( int i =0; i < iSize ; i ++ )
		{
			dTemp = m_ShowRegionVector[i].crColor;
			dTemp = m_ShowRegionVector[i].dStartPos;
			dTemp = m_ShowRegionVector[i].dEndPos;
		}
		delete s;
	}
	*/
	iSize = this->m_UserLineMap.size();
	MAPITERTMP itorTmp;
	COLORREF   crColorTmp;
	double     dTemp;
	int iID;

	for ( itorTmp = m_UserLineMap.begin(); itorTmp != m_UserLineMap.end();  itorTmp++)
	{
		iID = itorTmp->first;
		crColorTmp = (itorTmp->second).crColor;
		dTemp      = (itorTmp->second).dCurPos;
	}
}
