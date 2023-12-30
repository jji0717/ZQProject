// PortManager.h: interface for the CPortManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PORTMANAGER_H__2D7DE0CB_6A86_4BE7_A296_7839ED5E3ED8__INCLUDED_)
#define AFX_PORTMANAGER_H__2D7DE0CB_6A86_4BE7_A296_7839ED5E3ED8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "DODPort.h"
typedef std::vector<CDODPort *> CPORTVECTOR;
typedef std::deque<CDODChannel*> CDODCHANNLEQUEUE;
class CJMSParser;
class CPortManager  
{
public:
	CPortManager(CString logpath,int InterVal);
	virtual ~CPortManager();

	//port number ,I would not like to use it.
	int m_PortNumber;

	// port vector
	CPORTVECTOR m_PortVector;

	//get all parameters from all xml_content,then call this founction
	int ApplyParameter();

	//modify this port 's all channels.
	int UpdateCatalog();

	//stop all port.
	int Stop();
	
	//close all port ,irase all elements
	int ClosePort();

	//update channel interval
	static int m_nUpdateInterVal;

	//return one of portinfo by cstring.format
	CString GetPort();

	//enable or forbidden one channel of a port
	int  EnableChannel(BOOL bEnable);

	void RunUpdateThread();
	//current implement file path.for monitor filter
	CString m_path;

	//It's the developKit dll
	CDODDevKit *m_kit;
	int GetState(int nID);

	// received configuration queuename from JBoss
	CString m_Localqueuename;

	CString m_sDataTypeInitial;

    //Need update channel Queue
	static CDODCHANNLEQUEUE m_DelChannelQueue;

   //Add MsgChannel which need to Update
	static void AddUpdateChannel(CDODChannel* pChannelInfo);
	static HANDLE m_hUpdateEvent;

	static CRITICAL_SECTION m_UpdateCriticalSection;
private:
	// for command  to write file mode 
	HANDLE m_hUpdateThread;

};

#endif // !defined(AFX_PORTMANAGER_H__2D7DE0CB_6A86_4BE7_A296_7839ED5E3ED8__INCLUDED_)
