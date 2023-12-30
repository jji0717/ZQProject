#include "stdafx.h"
#include "ColorBarControl.h"
#include "ColorBarDoc.h"

CColorBarDoc::CColorBarDoc(CColorBarControl *pTestATLControl)
{
	this->m_pColorBarControl  = pTestATLControl;
	this->m_dLinePos          = 0.00;
	this->m_dBarLength        = 3.00; // ��ʼֵΪ3��Сʱ�ĳ��ȣ����ֵ���Ա䶯��ͨ��FillColor���䶯��ȡ
	this->m_iBarWidthPixes    = iControlWidth; //��ʼֵ����Ϊ300;
	this->m_iBarHightPixes    = iControlHigth; //��ʼֵ����Ϊ50;
	this->m_dShowRegionLength = 0.00; // ��ʾ��Bar���ȳ�ʼֵΪ0.0Сʱ
	this->m_iCurUserNum       = 0;    //��ʼֵΪ0
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

void  CColorBarDoc::SetiCurUserNum(int iNum)                             //���õ�ǰ���û���
{
	this->m_iCurUserNum = iNum;
}

void  CColorBarDoc::SetFillRegionDataVector(VECTORTYPE & DataVectorTmp)
{
	m_FillRegionVector.push_back(DataVectorTmp);
	ReCacluteLenTimeRatio();
}

void   CColorBarDoc::SetShowRegionDataVector(VECTORTYPE & DataVectorTmp) //����ShowReagion����������
{
	this->m_ShowRegionVector.push_back(DataVectorTmp);
}

void   CColorBarDoc::SetUserLineDataMap(int iIDTemp,USERDATASTRUCT & DataMapTmp)  //�����û��������ݵ�����
{
	this->m_UserLineMap[iIDTemp] = DataMapTmp;
}

void   CColorBarDoc::ResetSetUserLineDataMap(int iID)                       //���������û��������ݵ�����
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

int   CColorBarDoc::GetBarHightPixes()   const            // �õ�Bar�����ظ߶�
{
	return this->m_iBarHightPixes;
}

void   CColorBarDoc::SetBarLength(double dLength)
{
	this->m_dBarLength = dLength;
}
void   CColorBarDoc::SetBarWidthPixes(int  iBarLength)                    //����Bar�����س���,���û��ƶ��ı�
{
	this->m_iBarWidthPixes = iBarLength;
}

void   CColorBarDoc::SetBarHightPixes(int iBarHight)                 //����Bar�����ظ߶�,���û��ƶ��ı�
{
	this->m_iBarHightPixes = iBarHight;
}

DATAVECTOR & CColorBarDoc::GetFillRegionDataVector() 
{
	return this->m_FillRegionVector;
}
	
USERDATAMAP & CColorBarDoc::GetUserLineDataMap() // �õ��û��������ݵ�����   
{
	return this->m_UserLineMap;
}

DATAVECTOR & CColorBarDoc::GetShowRegionDataVector() // �õ�ShowReagion����������
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

void  CColorBarDoc::ReCacluteLenTimeRatio() // ���¼��㳤��/ʱ��ı���ֵ
{
	int iSize = this->m_FillRegionVector.size();
	/* // ����1
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
	// ����2
	VECTORITERTMP  itorTemp,itorTemp1;
	if ( !m_FillRegionVector.empty()) // ����ǿ�
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

//����ShowRange������
void   CColorBarDoc::CreateShowRegionVector(SHOWREGIONSTRUCT & ShowRegionTmp)//����ShowRange������
{
	int iSize = this->m_FillRegionVector.size();
	// ����1
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

			if ((  dStartPos >= dVectorStartPos ) && ( dStartPos < dVectorEndPos ) ) // �����ʼ���ڵ�һ��������
			{
				if ( dEndPos <= dVectorEndPos ) // ����յ�Ҳ��������
				{
					DataTmp.crColor      = m_FillRegionVector[i].crColor;
					DataTmp.dStartPos    = dStartPos;
					DataTmp.dEndPos      = dEndPos;
					DataTmp.bstrFilmName = m_FillRegionVector[i].bstrFilmName;
//					lstrcpy(DataTmp.sFilmName,m_FillRegionVector[i].sFilmName);
					m_ShowRegionVector.push_back(DataTmp);
					break; // �˳�ѭ��
				}
				else // �յ㲻��������
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
			if ((  dEndPos >= dVectorStartPos  ) && ( dEndPos < dVectorEndPos ) ) // �����ʼ���ڵ�һ��������
			{
				DataTmp.crColor       = m_FillRegionVector[i].crColor;
				DataTmp.dStartPos     = dVectorStartPos;
				DataTmp.dEndPos       = dEndPos;
				DataTmp.bstrFilmName  = m_FillRegionVector[i].bstrFilmName;
//				lstrcpy(DataTmp.sFilmName,m_FillRegionVector[i].sFilmName);
				m_ShowRegionVector.push_back(DataTmp);
				break; // �˳�ѭ��
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

int  CColorBarDoc::GetMapStartValue()        // �õ���С���û�IDֵ
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

int  CColorBarDoc::GetMapEndValue()          // �õ������û�IDֵ
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

int   CColorBarDoc::HourPosConvertToLPos(double dHourPos) // ��Сʱλ��ת��������λ��
{
	return (int)(dHourPos * m_dLenTimeRatio);
}

bool   CColorBarDoc::bIsExistInMap( COLORREF crColorVal,int *pID) // �жϸ��û���ɫ�Ƿ���Map�����Ѵ���
{
	bool bReturn = false;
	if ( m_UserLineMap.empty()) //map��Ϊ��
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


// ���¹����Ǳ�������еĵ��Բ�����
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
