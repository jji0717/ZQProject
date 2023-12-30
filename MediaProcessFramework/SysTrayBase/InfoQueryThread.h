#include <string>
#include <iostream>
#include <RpcWpValue.h>
#include <NativeThread.h>
#include "InfoDlg.h"

#define DEAULT_REFRESH_FREQ		30000

using namespace ZQ::rpc;
 
class InfoQueryThread : public ZQ::common::NativeThread
{
public:
	void stopThread();
	void startQuery();
	virtual int run();
	virtual void final();
	InfoQueryThread(const char* host, int port=12000, bool autoStop=false);
	~InfoQueryThread();

	void AddUpdateDlg(CInfoDlg* pdlg);
	void SetNotifer(CInfoDlg* notifee);

	/// set refresh frequence for non-autostop query thread
	///@param[in]	timeout		-the refresh time interval, in milli-seconds
	void SetFreq(DWORD timeout){ m_nTimeout = timeout; }

private:
	std::string		m_strHost;
	int				m_nPort;

	bool			m_isRun;
	bool			m_isAutoStop;
	HANDLE			m_hStart, m_hStop;	
	DWORD			m_nTimeout;
	
	CInfoDlg*		m_pUpdateDlg[10];
	int				m_nCurrentPos;
	int				m_nDisconnectCount;

	CInfoDlg*		m_pNotifee;
};