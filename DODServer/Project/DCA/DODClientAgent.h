// DODClineAgent.h: interface for the CDODClientAgent class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DODCLINEAGENT_H__04F73375_861C_42BC_B514_ECCF8781539E__INCLUDED_)
#define AFX_DODCLINEAGENT_H__04F73375_861C_42BC_B514_ECCF8781539E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//

#include"JMS.h"
#include "Parser.h"
//
#include "receivejmsmsg.h"
#include "PortManager.h"
#include "queuemanagement.h"
typedef struct ZQMsgParser
{
   CString QueueName;
   CReceiveJmsMsg *MsgReceive;
}ZQMSGPARSER, *PZQMSGPARSER;

class CDODClientAgent  
{
public:
	CDODClientAgent();
	~CDODClientAgent();

	//now , them will be execute for service manager.
	//when service restar call it,too.
	//when stop service,call destroy
	BOOL create();
	BOOL Destroy();

	//It's no used.
	BOOL Pause();
	BOOL Resume();

	//for interior simulation,send configuration xml_file
	//In the old version ,It will be used interior
	BOOL SetMessage();

	//read *.ini content. connect JBoss.
	void Initialize();

	// Uninitialize, destroy object ,delete all variable
	void UnInitialize();


	//It is detailed in CPortManager
	void Updatecatelog();

	//for interior testting
	void Stop();
	void ClosePort();
	CString GetPort();
	int  EnableChannel(BOOL bEnable);

	//get all port's state
	int GetState(int index);

	// add some queues of channels
	BOOL AddChannelQueue();

    //ReSet MsgListener flag 
	BOOL m_bReSetMsgListener;
    //major message parse Vector.one parse object match one receive Message queue
	std::vector<ZQMSGPARSER> m_VecParser;

	//major message parse,parse PortConfigration
	CReceiveJmsMsg * m_ParsePortConfig;
private:

    CJmsProcThread *m_Pjmsprocthread;

	//SRM service ,IP_address,IP_port.etc
	// in local config.ini , it used to verify equal local 's IPPORT and inputconfig's ones.
	//PPsortInfo m_portInfoSRM;

	CPortManager *m_pPortManager;


	//interior test for JMS,it's no used
	CString m_TopicName;

	//DOD client queuename for send portconfig message
	CString m_Configqueue;	
   
	CString m_CurrentEXEPath;
	//Management all queue for the same queuename;

    CString m_ProviderValue;

    //Get PortConfigration interval
	int m_nConfigMsgTimeOut;

	void ReLoadMsgChannelFile(CDODChannel * pChannel);

public:
	//When DCA restart,reload Msg data and notify DSA rewrapping
	BOOL GetAllChannelData();
 
	CString m_strPortConfigFileBak;
	BOOL ConnectionJBoss(void);	
	CQueueManageMent m_QueueManagement;
	CJMS   *m_jms;
};

#endif // !defined(AFX_DODCLINEAGENT_H__04F73375_861C_42BC_B514_ECCF8781539E__INCLUDED_)
