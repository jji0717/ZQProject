
#ifndef __COLORDOC_H_
#define __COLORDOC_H_

#pragma warning(disable:4786 )
#include <vector>
#include <map>
using std::vector;
using std::map;

static const unsigned int iControlHigth   = 50;  //Bar�Ĺ̶��߶�
static const unsigned int iControlWidth   = 300; //Bar�Ĺ̶����
static const unsigned int iMaxUserNum     = 150; //�����û���
static const unsigned int iFilmNameMaxLen = 512; //��Ŀ���Ƶ���󳤶� 

typedef struct  DATAStruct
{
	COLORREF  crColor;    //��ɫ
	double    dStartPos;  //��ʼ��λ�ã���СʱΪ��λ
	double    dEndPos;    //������λ�ã���СʱΪ��λ
//  TCHAR     sFilmName[iFilmNameMaxLen];//��Ŀ������
	CComBSTR  bstrFilmName;//��Ŀ������
}DATASTRUCT,*PDATASTRUCT;

typedef struct ShowRegionStruct
{
	double    dStartPos; //��ʾ����Ŀ�ʼ��λ�ã���СʱΪ��λ
	double    dEndPos;   //��ʾ����Ľ�����λ�ã���СʱΪ��λ
}SHOWREGIONSTRUCT,*PSHOWREGIONSTRUCT;

typedef struct  USERDATAStruct
{
	COLORREF  crColor; //��ɫ
	double    dCurPos; //��ǰ��λ��(��СʱΪ��λ)
//	int       iID;     //�û�ID��,����ΪMAP���KEYֵ
}USERDATASTRUCT,*PUSERDATASTRUCT;

typedef vector<DATASTRUCT>     DATAVECTOR;
typedef DATAVECTOR::value_type VECTORTYPE;
typedef DATAVECTOR::iterator   VECTORITERTMP;

typedef map<int,USERDATASTRUCT>  USERDATAMAP;
typedef USERDATAMAP::value_type  MAPTYPE;
typedef USERDATAMAP::iterator    MAPITERTMP;
// ��ɫ��COLORREFֵ:255,��ɫ��COLORREFֵ:65280,��ɫ��COLORREFֵ��16711680;

class CColorBarControl;
class CColorBarDoc
{
public:
	CColorBarDoc(CColorBarControl *pTestATLControl);
	virtual ~CColorBarDoc();

	double     GetShowBarLength()   const;            // �õ���ʾ����ĳ���
	double     GetBarLength()       const;            // �õ�ȫ������ĳ���
	double     GetLenTimeRatio()    const;            // �õ�����/ʱ��ı���
	double     GetFillBarStartPos() const;            // �õ�ȫ������Ŀ�ʼλ��
	double     GetFillBarEndPos()   const;            // �õ�ȫ������Ľ���λ��
	double     GetdLinePos()        const;            // �õ���ǰ���ߵ�λ��
	int        GetBarWidthPixes()   const;            // �õ�Bar�����ؿ��
	int        GetBarHightPixes()   const;            // �õ�Bar�����ظ߶�
	int        GetMapStartValue();                    // �õ���С���û�IDֵ
	int        GetMapEndValue();                      // �õ������û�IDֵ
	int        GetiCurUserNum();                      // �õ���ǰ�û���
	int        HourPosConvertToLPos(double dHourPos); // ��Сʱλ��ת��������λ��
	bool       bIsExistInMap( COLORREF crColorVal,int *pID);// �жϸ��û���ɫ�Ƿ���Map�����Ѵ���
		
	SHOWREGIONSTRUCT &  GetShowRegionData();       // �õ�ShowReagion�������Сֵ
	DATAVECTOR       &  GetShowRegionDataVector(); // �õ�ShowReagion����������
	DATAVECTOR       &  GetFillRegionDataVector(); // �õ�FillColor����������
	USERDATAMAP      &  GetUserLineDataMap();      // �õ��û��������ݵ�����   

	void     SetFillRegionDataVector(VECTORTYPE & DataVectorTmp);    //����FillColor����������
	void     SetShowRegionDataVector(VECTORTYPE & DataVectorTmp);    //����ShowReagion����������
	void     SetUserLineDataMap(int iIDTemp,USERDATASTRUCT & DataMapTmp);//�����û��������ݵ�����
	void     SetShowRegionData(SHOWREGIONSTRUCT & ShowRegionTmp);    //����ShowReagion�������Сֵ
	void     SetShowBarLength(double dLength);                       //����ShowRegion�ĳ���,ShowRegionʱ�䶯
	void     SetBarLength(double dLength);                           //����Bar�ĳ���,FillColorʱ�䶯
	void     SetBarWidthPixes(int   iBarLength);                     //����Bar�����ؿ��,���û��ƶ��ı�
	void     SetBarHightPixes(int   iBarHigth);                      //����Bar�����ظ߶�,���û��ƶ��ı�
	void     SetLenTimeRatio(double dRatioTmp);                      //���ó���/ʱ��ı���ֵ
	void     SetdLinePos(double dPos);                               //���õ�ǰ�ĵ�λ��  
	void     SetiCurUserNum(int iNum);                               //���õ�ǰ���û���
	void     ReCacluteLenTimeRatio();                                //���¼��㳤��/ʱ��ı���ֵ
	void     ResetSetUserLineDataMap(int iID);                       //���������û��������ݵ�����

	// for the test's produce 
	void     TestResult();                                           //������������ڲ��ԵĹ���

private:
	void     CreateShowRegionVector(SHOWREGIONSTRUCT & ShowRegionTmp);//����ShowRange������
private:
	int                      m_iCurUserNum;                           //��ǰ���û���
	int                      m_iBarWidthPixes;                        //Bar�����ؿ�ȣ����û��ı���ı�
	int                      m_iBarHightPixes;                        //Bar�����ظ߶ȣ����û��ı���ı�
	double                   m_dLinePos;                              //��ǰ�Ļ��ߵ�����ֵ����СʱΪ��λ)
	double                   m_dShowRegionLength;                     //��ʾ�������С(��СʱΪ��λ)
	double                   m_dBarLength;                            //Bar�ĳ��ȣ���FillColor���䶯,(��СʱΪ��λ����ʼֵΪ3Сʱ)
	double                   m_dLenTimeRatio;                         //����/ʱ�����
	SHOWREGIONSTRUCT         m_dShowRegionData;                       //��ʾ���������
	DATAVECTOR               m_FillRegionVector;                      //FillColor������
	DATAVECTOR               m_ShowRegionVector;                      //ShowRange������
    USERDATAMAP              m_UserLineMap;                           //�û��������ݵ�����
	CColorBarControl *       m_pColorBarControl;                      //�ؼ���ָ��
};
#endif
