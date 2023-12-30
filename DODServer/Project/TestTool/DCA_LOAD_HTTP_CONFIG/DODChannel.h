// DODChannel.h: interface for the CDODChannel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DODCHANNEL_H__A992F1B6_B3A3_438C_9BD6_DF9ACAAC6407__INCLUDED_)
#define AFX_DODCHANNEL_H__A992F1B6_B3A3_438C_9BD6_DF9ACAAC6407__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afx.h>
#include <vector>
#include "DODDevKit.h"

typedef struct Message_Info
{
	// The message body was save into the file.
	CString m_fileName;

	//each message create time,when it come from JMS server by AxtiveJMS 	
	COleDateTime m_createTime;

	//It denote a order number in the queue.
	//bForever=TRUE,means= forever;else has duration
	BOOL bForever;
	
	//every message left time.
	int m_nWorkDuration;

	// messageid and nDataType identify message
	CString sMessageID;	
	//int nDataType;
	
}ZQCMessageInfoTINF, *PZQCMessageInfoTINF;

typedef std::vector<ZQCMessageInfoTINF > zqMessageVector;

/*
class CMessageInfo
{
public:
	CMessageInfo();
	~CMessageInfo();
	CString m_fileName;
	COleDateTime m_createTime;
	BOOL flag;
	int index;
	int m_nWorkDuration;//every message left time.
};
*/
class CDODChannel  
{
public:
	CDODChannel();
	virtual ~CDODChannel();

//It is scout path,in a word, it equal to  catalog name.
	CString m_strCachingDir;

//It used to channel tag
	char m_strTag[4];
	
//The channel auto detected flag
	BOOL m_bDetected;

//  for subchannel. control show frequency of the message file.
	int m_nRepeateTime;

//m_nType=1 one has lifttime:   m_nType=0 other has not lifttime;
	int m_nType;  

//a byname of channel_ID
	int m_nStreamID;

// encrypt flag
	int m_bEncrypted;
	int m_nRate;

//If auto detected flag is true, the program will scout the path every m_nRepeateTime +1 seconds 
	int m_DetectInterVal;

// ------------------------------------------------------ Modified by zhenan_ji at 2006年2月28日 17:25:43
// create ilp file for nav
	CString m_sSendMsgDataType;
	int m_nSendMsgExpiredTime;

//It used to create cache path,it's value come from Port Configuration
	int m_channelID;

// ------------------------------------------------------ Modified by zhenan_ji at 2005年11月2日 17:06:10
	//	single stream; 1, multiple stream;
	int m_nMultiplestream;

	// if multiple stream =1 ,m_nstream count must nozero.
	int m_nStreamCount;

//a message number about current channel 
	int m_nMsgNumber;

// delete messagefile thread flag 
	BOOL m_bIsStop;

// current port 's mission ID
	int m_nSessionID;

//It's value come from JMS server
	int m_nPortID;

//updateChannel of interval
	BOOL m_bNeedUpdateChannel;

// channel property, it's value will be used in PMT.
	int nStreamType;

// ------------------------------------------------------ Modified by zhenan_ji at 2006年6月1日 14:03:50
	//indicate current channel attrib;
	CString m_sDataType;
// create index table or not by destionation name of message's content.
	int m_nSendWithDestination;

	CDODDevKit *m_kit;

	// process mutex flag for message_vector
	CRITICAL_SECTION m_channelCriticalSection;

	void DataUpdated();
	void ReleaseAll();
	int Create();
	
	//For message fomrat 
	COleDateTime m_LastUpdateTime;

//Message type come from PORTCONTROL
	int m_nMessageType;

	void Enable(BOOL flag);

	void Init();

//queue name ,in sending message, it will be used.
	CString m_QueueName;
	CString m_sChannelName,m_sPortName;
	int MoveAllFile(CString remotePath,int UpdateMode,CString subPath);
	int UpdateSubfolder(CString remotePath,int UpdateMode,CString subPath);
	int DeleteSubFile(CString remotePath,int UpdateMode,CString subPath);
	int DeleteFullPath(CString remotePath,int UpdateMode,CString subPath);
	int FullPathModofied(CString remotePath,int UpdateMode,CString subPath);
	int FullPathToNew(CString remotePath,int UpdateMode,CString subPath);
	int CreateOrReplaceFile(CString remotePath,int UpdateMode,CString subPath);
	int StopFileSend(CString sMessageID);
	//messagelist.The list was not used, because the list can not defined sort.
	zqMessageVector m_MessgeVector;

	int CreateMsgFile(CString &content,CString Filename,int nLeafTime,CString sMessageID,int ndatatype);
private:
	CString GetCurrDateTime();

private:
	
	// for command  to write file mode 
	HANDLE m_hSendThread;
};

#endif // !defined(AFX_DODCHANNEL_H__A992F1B6_B3A3_438C_9BD6_DF9ACAAC6407__INCLUDED_)
