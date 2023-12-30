
#ifndef __COLORDOC_H_
#define __COLORDOC_H_

#pragma warning(disable:4786 )
#include <vector>
#include <map>
using std::vector;
using std::map;

static const unsigned int iControlHigth   = 50;  //Bar的固定高度
static const unsigned int iControlWidth   = 300; //Bar的固定宽度
static const unsigned int iMaxUserNum     = 150; //最大的用户数
static const unsigned int iFilmNameMaxLen = 512; //节目名称的最大长度 

typedef struct  DATAStruct
{
	COLORREF  crColor;    //颜色
	double    dStartPos;  //开始的位置，以小时为单位
	double    dEndPos;    //结束的位置，以小时为单位
//  TCHAR     sFilmName[iFilmNameMaxLen];//节目的名称
	CComBSTR  bstrFilmName;//节目的名称
}DATASTRUCT,*PDATASTRUCT;

typedef struct ShowRegionStruct
{
	double    dStartPos; //显示区域的开始的位置，以小时为单位
	double    dEndPos;   //显示区域的结束的位置，以小时为单位
}SHOWREGIONSTRUCT,*PSHOWREGIONSTRUCT;

typedef struct  USERDATAStruct
{
	COLORREF  crColor; //颜色
	double    dCurPos; //当前的位置(以小时为单位)
//	int       iID;     //用户ID号,它作为MAP表的KEY值
}USERDATASTRUCT,*PUSERDATASTRUCT;

typedef vector<DATASTRUCT>     DATAVECTOR;
typedef DATAVECTOR::value_type VECTORTYPE;
typedef DATAVECTOR::iterator   VECTORITERTMP;

typedef map<int,USERDATASTRUCT>  USERDATAMAP;
typedef USERDATAMAP::value_type  MAPTYPE;
typedef USERDATAMAP::iterator    MAPITERTMP;
// 红色的COLORREF值:255,绿色的COLORREF值:65280,蓝色的COLORREF值：16711680;

class CColorBarControl;
class CColorBarDoc
{
public:
	CColorBarDoc(CColorBarControl *pTestATLControl);
	virtual ~CColorBarDoc();

	double     GetShowBarLength()   const;            // 得到显示区域的长度
	double     GetBarLength()       const;            // 得到全部区域的长度
	double     GetLenTimeRatio()    const;            // 得到长度/时间的比率
	double     GetFillBarStartPos() const;            // 得到全部区域的开始位置
	double     GetFillBarEndPos()   const;            // 得到全部区域的结束位置
	double     GetdLinePos()        const;            // 得到当前画线点位置
	int        GetBarWidthPixes()   const;            // 得到Bar的像素宽度
	int        GetBarHightPixes()   const;            // 得到Bar的像素高度
	int        GetMapStartValue();                    // 得到最小的用户ID值
	int        GetMapEndValue();                      // 得到最大的用户ID值
	int        GetiCurUserNum();                      // 得到当前用户数
	int        HourPosConvertToLPos(double dHourPos); // 把小时位置转换成坐标位置
	bool       bIsExistInMap( COLORREF crColorVal,int *pID);// 判断该用户颜色是否在Map表中已存在
		
	SHOWREGIONSTRUCT &  GetShowRegionData();       // 得到ShowReagion的区域大小值
	DATAVECTOR       &  GetShowRegionDataVector(); // 得到ShowReagion的区域容器
	DATAVECTOR       &  GetFillRegionDataVector(); // 得到FillColor的区域容器
	USERDATAMAP      &  GetUserLineDataMap();      // 得到用户画线数据的容器   

	void     SetFillRegionDataVector(VECTORTYPE & DataVectorTmp);    //设置FillColor的区域容器
	void     SetShowRegionDataVector(VECTORTYPE & DataVectorTmp);    //设置ShowReagion的区域容器
	void     SetUserLineDataMap(int iIDTemp,USERDATASTRUCT & DataMapTmp);//设置用户画线数据的容器
	void     SetShowRegionData(SHOWREGIONSTRUCT & ShowRegionTmp);    //设置ShowReagion的区域大小值
	void     SetShowBarLength(double dLength);                       //设置ShowRegion的长度,ShowRegion时变动
	void     SetBarLength(double dLength);                           //设置Bar的长度,FillColor时变动
	void     SetBarWidthPixes(int   iBarLength);                     //设置Bar的象素宽度,随用户移动改变
	void     SetBarHightPixes(int   iBarHigth);                      //设置Bar的象素高度,随用户移动改变
	void     SetLenTimeRatio(double dRatioTmp);                      //设置长度/时间的比率值
	void     SetdLinePos(double dPos);                               //设置当前的点位置  
	void     SetiCurUserNum(int iNum);                               //设置当前的用户数
	void     ReCacluteLenTimeRatio();                                //重新计算长度/时间的比率值
	void     ResetSetUserLineDataMap(int iID);                       //重新设置用户画线数据的容器

	// for the test's produce 
	void     TestResult();                                           //代码调试中用于测试的过程

private:
	void     CreateShowRegionVector(SHOWREGIONSTRUCT & ShowRegionTmp);//建立ShowRange的容器
private:
	int                      m_iCurUserNum;                           //当前的用户数
	int                      m_iBarWidthPixes;                        //Bar的像素宽度，随用户改变而改变
	int                      m_iBarHightPixes;                        //Bar的像素高度，随用户改变而改变
	double                   m_dLinePos;                              //当前的画线的坐标值（以小时为单位)
	double                   m_dShowRegionLength;                     //显示的区域大小(以小时为单位)
	double                   m_dBarLength;                            //Bar的长度，由FillColor来变动,(以小时为单位，初始值为3小时)
	double                   m_dLenTimeRatio;                         //长度/时间比率
	SHOWREGIONSTRUCT         m_dShowRegionData;                       //显示区域的数据
	DATAVECTOR               m_FillRegionVector;                      //FillColor的容器
	DATAVECTOR               m_ShowRegionVector;                      //ShowRange的容器
    USERDATAMAP              m_UserLineMap;                           //用户画线数据的容器
	CColorBarControl *       m_pColorBarControl;                      //控件的指针
};
#endif
