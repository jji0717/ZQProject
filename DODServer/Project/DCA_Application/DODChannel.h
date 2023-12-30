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

// define maxmount message numbers

typedef struct Message_Info
{
	// The message body was save into the file.
	CString m_fileName;

	//each message create time,when it come from JMS server by AxtiveJMS 	
	COleDateTime m_createTime;

	//if flag == false ,The message is invalite
	BOOL flag;

	//It denote a order number in the queue.
	int index;
	
	//every message left time.
	int m_nWorkDuration;

	// messageid and nDataType identify message
	int nMessageID;	
	int nDataType;
	
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

//
//	int m_pDataChaching; 

//It used to create cache path,it's value come from Port Configuration
	int m_channelID;


//	CMessageInfo *m_msgArray[MAXMESSAGETOTAL];

//a message number about current channel 
	int m_nMsgNumber;

// delete messagefile thread flag 
	BOOL m_bIsStop;

// current port 's mission ID
	int m_nSessionID;

//It's value come from JMS server
	int m_nPortID;

// channel property, it's value will be used in PMT.
	int nStreamType;
	int m_nDataType;

	CDODDevKit *m_kit;

	// process mutex flag for message_vector
	CRITICAL_SECTION m_channelCriticalSection;

	void DataUpdated();
	void ReleaseAll();
	int Create();
	
//Message type come from PORTCONTROL
	int m_nMessageType;

	void Enable(BOOL flag);

	void Init();

//queue name ,in sending message, it will be used.
	CString m_QueueName;
	CString m_strName;
	int MoveAllFile(CString remotePath,int UpdateMode,CString subPath);
	int UpdateSubFile(CString remotePath,int UpdateMode,CString subPath);
	int DeleteSubFile(CString remotePath,int UpdateMode,CString subPath);
	int DeleteFullPath(CString remotePath,int UpdateMode,CString subPath);
	int FullPathModofied(CString remotePath,int UpdateMode,CString subPath);
	int FullPathToNew(CString remotePath,int UpdateMode,CString subPath);

	//messagelist.The list was not used, because the list can not defined sort.
	zqMessageVector m_MessgeVector;

	int CreateMsgFile(CString &content,CString Filename,int nLeafTime,int nMessageID,int delOrAdd,int ndatatype);
private:
	CString GetCurrDateTime();

private:

	// for command  to write file mode 
	HANDLE m_hSendThread;
};

#endif // !defined(AFX_DODCHANNEL_H__A992F1B6_B3A3_438C_9BD6_DF9ACAAC6407__INCLUDED_)
