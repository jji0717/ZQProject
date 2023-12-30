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
#include "PortManager.h"
#include "queuemanagement.h"

#define LOGFILENAME "C:\\log\\DODClientAgent.log"
#define LOGMAXSIZE  (64*1024*1024)     //64M
#define LOGMAXLEVEL 100


class CDODClientAgent  
{
public:
	CDODClientAgent();
	~CDODClientAgent();

	//now , them will be execute for service manager.
	//when service restar call it,too.
	//when stop servce,call destroy
	BOOL create();
	BOOL Destroy();

	//It's no used.
	BOOL Pause();
	BOOL Resume();

	//for interior simulation,send configuration xml_file
	//In the old version ,It will be used interior
	void SetMessage();

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

private:

	//SRM service ,IP_address,IP_port.etc
	PPsortInfo m_portInfoSRM;
 
// in local config.ini , it used to verify equal local 's IPPORT and inputconfig's ones.
	PPsortInfo m_comeIP;

	CPortManager *m_pPortManager;
    CJMS   *m_jms;

	//interior test for JMS,it's no used
	CString m_TopicName;

	//major message parse
	CJMSParser * m_Parse;

	//DOD client queuename for send portconfig message
	CString m_Configqueue;	

	CString m_CurrentEXEPath;
	//Management all queue for the same queuename;
	CQueueManageMent m_QueueManagement;
};

#endif // !defined(AFX_DODCLINEAGENT_H__04F73375_861C_42BC_B514_ECCF8781539E__INCLUDED_)
