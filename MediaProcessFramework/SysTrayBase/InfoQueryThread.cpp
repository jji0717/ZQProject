#include "stdafx.h"
#include "InfoQueryThread.h"
#include "RpcWpClient.h"

InfoQueryThread::InfoQueryThread(const char* host, int port/* =12000 */, bool autoStop/* =false */)
	: m_strHost(host), m_nPort(port),
	m_isRun(true), m_isAutoStop(autoStop), m_nTimeout(DEAULT_REFRESH_FREQ),
	m_hStart(::CreateEvent(NULL, TRUE, FALSE, NULL) ),
	m_hStop(::CreateEvent(NULL, FALSE, FALSE, NULL) ), 
	m_nCurrentPos(0), m_nDisconnectCount(0), m_pNotifee(NULL)
{
	
}

InfoQueryThread::~InfoQueryThread()
{
	::CloseHandle(m_hStart);
	::CloseHandle(m_hStop);
}

void InfoQueryThread::stopThread()
{
	::SetEvent(m_hStop);
}

void InfoQueryThread::startQuery()
{
	::SetEvent(m_hStart);
}

void InfoQueryThread::AddUpdateDlg(CInfoDlg* pdlg)
{
	m_pUpdateDlg[m_nCurrentPos] = pdlg;
	m_nCurrentPos++;
}

int InfoQueryThread::run()
{
	HANDLE events[] = {m_hStop, m_hStart};
	
	while (m_isRun)
	{
		try
		{
			for(int i=0; i<m_nCurrentPos; i++) 
			{
				RpcValue result;
				RpcClient client(m_strHost.c_str(),m_nPort);
				
				//setResponseTimeout(5);
				
				if (client.execute(GETINFO_METHOD, m_pUpdateDlg[i]->GenRpcValue(), result))
				{
					m_pUpdateDlg[i]->UpdateDlgList(result); // Refresh the new result to the Dialog
					//CWnd::FromHandle(m_pUpdateDlg[i]->m_hWnd)->UpdateData(false);
				}
				else
					m_nDisconnectCount++;
			}
			
			// Can't get new information from the node, show message.
			if (0<m_nDisconnectCount && m_nDisconnectCount<=m_nCurrentPos)
			{
				if (m_nCurrentPos == 1) // let the popup window show its own message
					;
				else if (m_nDisconnectCount == m_nCurrentPos) 
					// can't get all information, suggest the service or network has been down.
					AfxMessageBox("Attention: Service Failure! Please check your service or network status.");
				else
					AfxMessageBox("Attention: Some of your information could not be updated, please check the service status.");
			}
			
			if(m_pNotifee)
				m_pNotifee->startWindowUpdate();
			
			::ResetEvent(m_hStart);
			
			if(m_isAutoStop)
			{
				m_isRun = false;	// autostop mode, do once and die
			}
			else switch ( WaitForMultipleObjects(2,events,false,m_nTimeout) )
			{
			case WAIT_OBJECT_0:
				m_isRun = false;	// non-autostop mode, signaled to die
				break;
			case WAIT_OBJECT_0+1:
				
				break;
			case WAIT_TIMEOUT:
				
				break;
			default:
				AfxMessageBox("Error happens when query a value, please check the Service!");
				break;
			}
		} 
		catch(...)  // Must catch the thread exception in order to prevent halting.
		{
			if(m_pNotifee)	
				m_pNotifee->startWindowUpdate();
			WaitForSingleObject(m_hStop,5000); // wait 5s.
		}
	} // end while
	
	return 0;
}

void InfoQueryThread::final()
{
	if(m_isAutoStop)
		delete this;
}

void InfoQueryThread::SetNotifer(CInfoDlg* notifee)
{
	m_pNotifee = notifee;
}
