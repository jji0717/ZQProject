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
	COLORREF       crColor;    //颜色
	double         dStartPos;  //开始的位置，以小时为单位
	double         dEndPos;    //结束的位置，以小时为单位
	CComBSTR       bstrFilmName;//节目的名称
}DATASTRUCT,*PDATASTRUCT;

// 创建的用户ID号信息
typedef struct CurSorData
{
	int           iCurSorID; //用户的ID号值
	CComBSTR      bstrStreamId;//其实这个值不是StreamId值，而是OnProgress过程中Comment=CtrlNum=0&ItemName=\vod\00080009中的00080009值
//	string        strStreamId;//StreamId
}CURSORDATA,*PCURSORDATA;

//组件容器的结构
typedef struct ContainData
{
	IColorBarControl * pContainControl;//节目的组件对象指针
	double             dChanelWidth; //节目的总长度，如子节目的长度
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

