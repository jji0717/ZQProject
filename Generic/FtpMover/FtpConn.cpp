
// ===========================================================================
// Copyright (c) 1997, 1998 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Ident : $Id: FtpConn.cpp,v 1.5 2004/07/29 05:13:27 wli Exp $
// Branch: $Name:  $
// Author: Kaliven lee
// Desc  : impl the Connection  class FtpConn
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/FtpMover/FtpConn.cpp $ 
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 8     06-08-04 13:30 Ken.qian
// move from comextra to common
// 
// 7     05-04-04 16:56 Kaliven.lee
// modify some log 
// 
// 6     05-04-04 13:33 Kaliven.lee
// 
// 5     05-03-28 17:50 Kaliven.lee
// 
// 4     05-03-24 17:02 Kaliven.lee
// 
// 2     05-02-04 11:35 Kaliven.lee
// add head

// 
//
//////////////////////////////////////////////////////////////////////

#include "FtpConn.h"
#include "FtpMoverConf.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
extern ZQ::common::Log *pGlog;
extern FILELOCKLIST gFileLocks;

using ZQ::common::Log;
using ZQ::common::NativeThread;
DWORD FtpConn::m_UnitID = 0;
FtpConn::FtpConn(bool isPassive ):
ZQ::common::NativeThread(),
m_bQuit(false),
m_ftpConn(NULL),
m_pSession(NULL),
m_isPassive(isPassive),
m_dwState(FTPCONN_STATE_NOMORAL),
m_isBusy(false)
{
	
	m_StartEvent = ::CreateEvent(NULL,FALSE,FALSE,NULL);
	m_StopEvent = ::CreateEvent(NULL,FALSE,FALSE,NULL);
	m_id = ++m_UnitID;
}

FtpConn::~FtpConn()
{
	if(m_isBusy)
		DisConnect();	
	if(m_StartEvent != INVALID_HANDLE_VALUE)
		::CloseHandle(m_StartEvent);
	if(m_StopEvent != INVALID_HANDLE_VALUE)
		::CloseHandle(m_StopEvent);	
}
bool FtpConn::IsBusy(void)
{
	return m_isBusy;
}
bool FtpConn::Connect(wchar_t* server,wchar_t* username,wchar_t* password,DWORD dwPort  ,bool isPassive )
{
	CString csStr ;
	csStr.Format(_T("FtpMoverConn%d"),m_id);
	m_pSession = new CInternetSession(csStr);	
	m_pTask->state = Processing;
	glog(Log::L_INFO,_T("FtpConn %d:try to connect the FtpServer %s:%d"),m_id,server,dwPort);
	TRY{	
		m_ftpConn = m_pSession->GetFtpConnection(server,username,password,dwPort,isPassive);
//		if(!m_FtpClient.connect(server,dwPort,username,password,isPassive))
		if(!m_ftpConn)
		{
			glog(Log::L_ERROR,_T("FtpConn %d:can not connect to ftp server %s"),m_id,server);
			m_pTask->state = Failed;
			return false;
		}
		glog(Log::L_DEBUG,_T("FtpConn %d:Success connect to the Ftp Server %s"),m_id,server);
		m_dwState = FTPCONN_STATE_READY;
	
	}CATCH(CInternetException, e)
	{
		wchar_t buf[512] = _T("");		
		e->GetErrorMessage(buf, 512);
		CString str = buf;
		str.Replace(_T("\r\n"),_T(""));
		glog(Log::L_ERROR,_T("FtpConn %d:Connect error %s"),m_id,str);
		m_dwState = FTPCONN_STATE_CONNECTFAILED;
		m_pTask->state = Failed;
		return false;
	}
	END_CATCH
	return true;
}
bool FtpConn::PutFile(wchar_t* fileName,wchar_t* remoteFileName)
{
	CInternetFile* pTarget;
//	FtpClient::FtpFile * pRemoteFile = NULL;
	glog(Log::L_INFO,_T("FtpConn %d:Try to transfer local file %s as %s"),m_id,fileName,remoteFileName);
	HANDLE hSrc;
	TRY{	

		pTarget = m_ftpConn->OpenFile(remoteFileName,GENERIC_WRITE);
		pTarget->SetWriteBufferSize(0);
		if(!pTarget)
			return FALSE;
		glog(Log::L_INFO,_T("FtpConn %d:Open remote file %s prepare to write"),m_id,remoteFileName);

		hSrc = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if(hSrc == INVALID_HANDLE_VALUE)
		{
			m_dwState = FTPCONN_STATE_READFAILED;
			glog(Log::L_ERROR,_T("FtpConn %d:Can not open local the source file %s"),m_id,fileName);
			return false;
		}
		glog(Log::L_INFO,_T("FtpConn %d:Open local File %s ready to read."),m_id,fileName);
		
	}CATCH(CInternetException, e)
	{
		m_dwState = FTPCONN_STATE_WRITEFAILED;
		wchar_t err[1024];
		memset(err, 0, sizeof(err));
		e->GetErrorMessage(err, sizeof(err));
		m_pTask->state = Failed;
		CString str = err;
		str.Replace(_T("\r\n"),_T(""));
		glog(Log::L_ERROR, _T("FtpConn %d:ftp Connection error: %s"), m_id,str);				
		return false;
	}
	END_CATCH	
	DWORD size = GetFileSize(hSrc, NULL);
	// TODO watch such buf size
	DWORD dwRead = 0;
	DWORD szSend = 0;
	char buf[WRITEBUFFSIZE];
	do {
		m_dwState = FTPCONN_STATE_TRANSFERRING;
		if(!ReadFile(hSrc, buf, sizeof(buf), &dwRead, NULL))
		{	
			m_dwState = FTPCONN_STATE_READFAILED;
			m_pTask->state = Failed;
			glog(Log::L_ERROR,_T("FtpConn %d:Can not read the source file %s"),m_id,fileName);			
			break;
		}
		if(dwRead)
		{
			TRY
			{
		          pTarget->Write(buf, dwRead);	  
				  szSend += dwRead;				 
				  m_pTask->percentage = szSend*100.0/size;		
			}
			CATCH(CInternetException ,e)
			{
				m_dwState = FTPCONN_STATE_WRITEFAILED;
				wchar_t err[512];
				memset(err, 0, sizeof(err));
				e->GetErrorMessage(err, 512);
				m_pTask->state = Failed;
				CString str = err;
				str.Replace(_T("\r\n"),_T(""));
				glog(Log::L_ERROR, _T("FtpConn %d: ftp write error(%s)."), m_id,str);				
				break;
			}
			END_CATCH
		}
	} while(dwRead!=0);
	// if no error happened 
	if(m_dwState == FTPCONN_STATE_TRANSFERRING)
	{	
		if(size == szSend)
		{
			glog(Log::L_INFO, _T("FtpConn %d:transfer file %s Ok!"),m_id,fileName);
			m_dwState = FTPCONN_STATE_TRANSFERRED;
			
		}
		else
		{
			glog(Log::L_ERROR, _T("FtpConn %d:transfer file process do not finished."),m_id,fileName);
			m_dwState = FTPCONN_STATE_WRITEFAILED;
		}
	}
	CloseHandle(hSrc);
	TRY{
		pTarget->Close();
	}CATCH(CInternetException,e)
	{
		glog(Log::L_INFO, _T("FtpConn %d:Exception catch when close remote file %s."),m_id,fileName);
	}
	END_CATCH
	if(m_dwState != FTPCONN_STATE_TRANSFERRED)
		return false;
	else 
		return true;
}
DWORD FtpConn::GetCurState(void)
{
	return m_dwState;
}
bool FtpConn::DisConnect(void)
{	
	if(!m_isBusy)
		return true;
	glog(Log::L_INFO,_T("FtpConn %d: Try to close connection to %s."),m_id,m_pTask->wcsDestineServer);

	if(m_pSession)
	{
	
		TRY
		{
			if(m_ftpConn)
			{
				glog(Log::L_INFO,_T("FtpConn %d: Try to end international session to %s."),m_id,m_pTask->wcsDestineServer);
				m_pSession->Close();
			}
			
		}CATCH(CInternetException ,e)
		{
			wchar_t err[512];
			memset(err, 0, sizeof(err));
			e->GetErrorMessage(err, 512);
			CString str = err;
			str.Replace(_T("\r\n"),_T(""));
			glog(Log::L_ERROR, "FtpConn %d: Close session failed(%s).", m_id,str);	
		}
		END_CATCH
	
		delete m_pSession;
	
		
	}
	m_isBusy = false;	
	return true;
}
int FtpConn::run()
{
	DWORD rtn = 0;
	glog(Log::L_INFO,_T("FtpConn %d: ready to process task."),m_id);
	HANDLE events[2];
	events[0] = m_StopEvent;
	events[1] = m_StartEvent;
	while(!m_bQuit)
	{
		rtn = ::WaitForMultipleObjectsEx(2,events,FALSE,FTPCONN_TIMEOUT,FALSE);
		if(m_bQuit)
				return 0;
		switch(rtn)
		{
			case	WAIT_OBJECT_0 :
				return 0;
				break;
			case	WAIT_OBJECT_0 +1:
				ExecuteTask();
				break;
			case WAIT_TIMEOUT:
				if(m_bQuit)
					return 0;
				break;
			
			default:
				break;
		}
	
	}
	return 0;
}
void FtpConn::SetTask(TRANSFERTASK* TransferTask)
{
	m_pTask = TransferTask;
	m_pTask->dwFtpConnID = m_id;
	m_dwState = FTPCONN_STATE_NOMORAL;
}
void FtpConn::ExecuteTask(void)
{
	bool bIsTranferSuccess = false;
	if(Connect(m_pTask->wcsDestineServer,m_pTask->wcsUserName,m_pTask->wcsPassword,m_pTask->dwPort,m_isPassive))	
		bIsTranferSuccess = PutFile(m_pTask->wcsFileName,m_pTask->wcsRemoteFileName);
	DisConnect();
	if(bIsTranferSuccess)
		m_pTask->state = Finished;
	else
		m_pTask->state = Failed;
	m_pTask->dwFtpConnID = 0;
	glog(Log::L_DEBUG,_T("FtpConn %d is ready to processing new Task."),m_id);
}
void FtpConn::StartTransfer(TRANSFERTASK* TransferTask)
{
	m_isBusy = true;
	SetTask(TransferTask);

	
	glog(Log::L_INFO,_T("FtpConn %d: start to process task %s."),m_id,m_pTask->wcsFileName);
	::SetEvent(m_StartEvent);
}
void FtpConn::Stop(void)
{
	glog(Log::L_INFO,_T("FtpConn %d: receive the stop command from Connection manager."),m_id);
	if(m_isBusy)	
		DisConnect();
	::SetEvent(m_StopEvent);
	m_bQuit = true;
}
DWORD FtpConn::GetID(void)
{
	return m_id;
}
