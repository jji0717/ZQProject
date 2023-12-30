#ifndef __DATASTRUCE_H_
#define __DATASTRUCE_H_

#pragma warning(disable:4786 )
#include <vector>
#include <map>
#include <string>
#include "ColorBar.h"

using std::vector;
using std::map;
using std::string;


#define BEGIN_USER_COLOR                      RGB(0,0,0)
#define	CHANNELITEM_KEY_SEPARATOR	"#"

#define DEF_SUBSCRIBER_ENDPOINT_STREAM         "tcp -p 10000"
//#define DEF_TOPIC_MANAGER_ENDPOINT_STREAM      "tcp -h 192.168.80.49  -p 10000"
#define DEF_TOPIC_MANAGER_ENDPOINT_STREAM      "TianShanEvents/TopicManager:tcp -h 192.168.80.49  -p 10000"

#define DEF_SUBSCRIBER_ENDPOINT_CHANEL	   	   "tcp -p 9832"
#define DEF_TOPIC_MANAGER_ENDPOINT_CHANEL    "ChannelPublisherEx:tcp -h 10.15.10.250  -p 9832" // for the ChannelPublisherEx Interface

typedef struct  DATAStruct
{
	COLORREF       crColor;    //��ɫ
	double         dStartPos;  //��ʼ��λ�ã���СʱΪ��λ
	double         dEndPos;    //������λ�ã���СʱΪ��λ
	CComBSTR       bstrFilmName;//��Ŀ������
}DATASTRUCT,*PDATASTRUCT;

// �������û�ID����Ϣ
typedef struct CurSorData
{
	int           iCurSorID; //�û���ID��ֵ
	CComBSTR      bstrStreamId;//��ʵ���ֵ����StreamIdֵ������OnProgress������Comment=CtrlNum=0&ItemName=\vod\00080009�е�00080009ֵ
//	string        strStreamId;//StreamId
}CURSORDATA,*PCURSORDATA;

//��������Ľṹ
typedef struct ContainData
{
	IColorBarControl * pContainControl;//��Ŀ���������ָ��
	double             dChanelWidth; //��Ŀ���ܳ��ȣ����ӽ�Ŀ�ĳ���
	CComBSTR           bstrChanelId;//ChanelId
//	string             strChanelId; //ChanelId
}CONTAINDATA,*PCONTAINDATA;


// generate data from channel
typedef vector<DATASTRUCT>          DATAVECTOR;
typedef DATAVECTOR::value_type      VECTORTYPE;
typedef DATAVECTOR::iterator        VECTORITERTMP;

// generate data from channel, the data contains the channel and  items data of a channel
typedef map<CComBSTR,DATAVECTOR>    CHANELDATAMAP;//key=channel name
typedef CHANELDATAMAP::value_type   MAPTYPE;
typedef CHANELDATAMAP::iterator     MAPITERTMP;

// generate data from StreamEventSink the data contains the containwnd's control
typedef map<CComBSTR,CONTAINDATA>   CONTAINERMAP; //key=StreamId
typedef CONTAINERMAP::value_type    CONTAINERMAPTYPE;
typedef CONTAINERMAP::iterator      CONTAINERMAPITER;

// generate data from StreamProgressSink the data contains the usercursor'ids of the channel and  drawline's position
typedef vector<CURSORDATA>          CURSORVECTOR; 
typedef CURSORVECTOR::value_type    CURSORVECTORTYPE;
typedef CURSORVECTOR::iterator      CURSORVECTORITER;

//generate data from StreamProgressSink the data contains the usercursor'ids of the channel and  drawline's position
typedef map<CComBSTR,CURSORVECTOR>  USERDRAWLINEMAP; //key=ChanelId
typedef USERDRAWLINEMAP::value_type USERDRAWLINEMAPTYPE;
typedef USERDRAWLINEMAP::iterator   USERDRAWLINEMAPITER;

#endif

