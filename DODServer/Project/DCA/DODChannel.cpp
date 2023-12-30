// DODChannel.cpp: implementation of the CDODChannel class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DODChannel.h"
#include "clog.h"
#include "FileFindExt.h"
#include "messagemacro.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define WRITECHANNELLOGTIME 180


CDODChannel::CDODChannel()
{
	Init();
}

CDODChannel::~CDODChannel()
{	
	SetEvent(m_hWaitEvent);
	m_bIsStop = TRUE;	
	Sleep(1);
	if( m_hSendThread )	
	{
		WaitForSingleObject( m_hSendThread, INFINITE );
		CloseHandle(m_hSendThread);
		m_hSendThread = NULL;
	}
	CloseHandle(m_hWaitEvent);
	DeleteCriticalSection(&m_channelCriticalSection);

	m_MessageList.clear();
}
void CDODChannel::Init()
{	
	m_nMsgNumber = 0;
	m_strCachingDir = "NoInitial";
	m_strTag[0] = '\0';
	m_bDetected = FALSE;
	m_nType = 0;  //m_nType=1 one has lifttime:   m_nType=0 other has not lifttime;
	m_nStreamID = 0;
	m_nRepeateTime = 0;
	m_bIsStop = FALSE;
	m_bEncrypted = 0;
	m_nSessionID = 0;
	m_nRate = 0;
	nStreamType = 0;
	m_channelID = 0;
	m_DetectInterVal = 5000;
	m_nPortID = 0;
	m_sDataType = "";
	m_sChannelName = "";
	m_sPortName = "";
	m_nMessageType = 0;
	m_bNeedUpdateChannel = FALSE;
	m_QueueName = "";
	m_MessageList.clear();
	InitializeCriticalSection(&m_channelCriticalSection);
	m_nMultiplestream = 0;
	m_nStreamCount = 0;
	m_nSendWithDestination = 0;
	m_sSendMsgDataType = "";
	m_nSendMsgExpiredTime = 0;
	m_hSendThread = NULL;
	m_hWaitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
}
void CDODChannel::DataUpdated()
{

}
void CDODChannel::ReleaseAll()
{
	
}

int CDODChannel::FullPathToNew(CString remotePath,int UpdateMode,CString subPath)
{
	
	return 0;
}

int CDODChannel::FullPathModified(CString remotePath,int UpdateMode,CString subPath)
{
//	Clog( LOG_DEBUG,_T("[%s-%s]CDODChannel::FullPathModified"),m_sPortName,m_sChannelName);
	return MoveAllFile(remotePath,1,subPath);
}

int CDODChannel::DeleteFullPath(CString remotePath,int UpdateMode,CString subPath)
{
	Clog( LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d]CDODChannel::DeleteFullPath remotePath = %s") ,m_sPortName,m_nPortID,m_sChannelName,m_channelID,remotePath);
	int ret;
	CString str;
	str = m_strCachingDir;
	ret = DeleteDirectory(str.GetBuffer(0),false);
	if (!ret)
	{
		Clog(LOG_DEBUG,_T(" MoveAllFile: delete remote directory error."),m_sChannelName );
		Clog( LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d]CDODChannel::DeleteFullPath error") ,m_sPortName,m_nPortID,m_sChannelName,m_channelID);
		return 1;
	}
	Clog( LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d]CDODChannel::DeleteFullPath [CachingDir = %s]success") ,m_sPortName,m_nPortID,m_sChannelName,m_channelID,str);

/*	Clog(LOG_DETAIL,_T(" Create CachingDir  directory :[CachingDir = %s]"),m_strCachingDir);
	if (CreateDirectory((LPCTSTR)m_strCachingDir, NULL) == FALSE)		
	{
		int nError = GetLastError();
		char strError[500];
		GetErrorDescription(nError, strError);
		Clog(LOG_DEBUG,_T("CreateDirectory is error.GetLastError = %d, ErrorDescription = %s"),nError, strError);
		return 1;
	}
	Clog(LOG_DETAIL,_T(" Create CachingDir  directory success!"));*/
	return 0;
}

int CDODChannel::DeleteSubFile(CString remotePath,int UpdateMode,CString subPath)
{
	Clog( LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d]CDODChannel::DeleteDIRorFile remotePath=%s"),m_sPortName,m_nPortID,m_sChannelName,m_channelID,remotePath);
	int ret;
	CString str;
	str = m_strCachingDir + subPath;
	ret = DeleteFile(str.GetBuffer(0));
	if (!ret)
	{
		int nError = GetLastError();
		char strError[500];

		GetErrorDescription(nError, strError);

		Clog(LOG_DEBUG,_T(" DeleteDIRorFile:%s error.GetLastError() = %d,ErrorDescription = %s"),str,nError,strError);
		Clog( LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d]CDODChannel::DeleteDIRorFile error"),m_sPortName,m_nPortID,m_sChannelName,m_channelID);
		return 1;
	}
	Clog( LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d]CDODChannel::DeleteDIRorFile success"),m_sPortName,m_nPortID,m_sChannelName,m_channelID);
	return 0;
}

int CDODChannel::CreateOrReplaceFile(CString remotePath,int UpdateMode,CString subPath)
{
	//CString strSource,strCacheDir;
	Clog( LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d]CDODChannel::CreateOrReplaceFile remotePath = %s,subPath = %s"),m_sPortName,m_nPortID,m_sChannelName,m_channelID,remotePath,subPath);

	CFileFindExt ProcessDir;
	BOOL ret = ProcessDir.DoProcess(remotePath,FileCopy,m_strCachingDir); 
	if (!ret)
	{		
		int nError = GetLastError();
		char strError[500];

		GetErrorDescription(nError, strError);

		Clog(LOG_DEBUG,_T("CDODChannel.Copy allfile to cacheFolder error! GetLastError() = %d,ErrorDescription = %s "),nError,strError );
		Clog( LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d]CDODChannel::CreateOrReplaceFile error"),m_sPortName,m_nPortID,m_sChannelName,m_channelID);
		return 1;
	}

	ret = DeleteDirectory(remotePath.GetBuffer(0),true);

	if (!ret)
	{
		int nError = GetLastError();
		char strError[500];

		GetErrorDescription(nError, strError);

		Clog( LOG_DEBUG,_T("Del remote directory error(%s)::GetLastError=%d ,ErrorDescription =  %s "),remotePath,nError,strError);
		Clog( LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d]CDODChannel::CreateOrReplaceFile error"),m_sPortName,m_nPortID,m_sChannelName,m_channelID);
		return 1;
	}

	Clog( LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d]CDODChannel::CreateOrReplaceFile success"),m_sPortName,m_nPortID,m_sChannelName,m_channelID);

	return 0;
}
int CDODChannel::UpdateSubfolder(CString remotePath,int UpdateMode,CString subPath)
{
	 CString strtemp,str1;
	 Clog( LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d]CDODChannel::UpdateSubfolder remotePath = %s"),m_sPortName,m_nPortID,m_sChannelName,m_channelID,remotePath);

	 strtemp = m_strCachingDir + subPath;
	 str1 = remotePath;

	 BOOL ret = DeleteDirectory(strtemp.GetBuffer(0),true);
	 if (!ret)
	 {
		 int nError = GetLastError();
		 char strError[500];

		 GetErrorDescription(nError, strError);

		 Clog(LOG_DEBUG,_T(" MoveAllFile: delete local directory (%s) error:GetLastError=%d, ErrorDescription = %s"),strtemp, nError, strError);
		// return 1;
	 }
	strtemp = m_strCachingDir;
	CFileFindExt ProcessDir;
	ret = ProcessDir.DoProcess(str1,FileCopy,strtemp); 

	if (!ret)
	{ 
		int nError = GetLastError();
		char strError[500];

		GetErrorDescription(nError, strError);

		Clog(LOG_DEBUG,_T("CDODChannel.Copy allfile to cacheFolder error! GetLastError=%d, ErrorDescription = %s"),nError, strError);
		Clog( LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d]CDODChannel::UpdateSubfolder error"),m_sPortName,m_nPortID,m_sChannelName,m_channelID);

		return 1;
	}

	ret = DeleteDirectory(remotePath.GetBuffer(0),true);
	if (!ret)
	{
		Clog(LOG_DEBUG,_T(" MoveAllFile: delete remote directory (%s) error"),m_sChannelName );
		Clog(LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d]CDODChannel::UpdateSubfolder error"),m_sPortName,m_nPortID,m_sChannelName,m_channelID);
		return 1;
	}
	Clog( LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d]CDODChannel::UpdateSubfolder success"),m_sPortName,m_nPortID,m_sChannelName,m_channelID);

	return 0;
}
int CDODChannel::MoveAllFile(CString remotePath,int UpdateMode,CString subPath)
{
	Clog( LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d]CDODChannel::MoveAllFile for remotePath=(%s),CachingDir=(%s)"),m_sPortName,m_nPortID,m_sChannelName,m_channelID,remotePath,m_strCachingDir);
	BOOL ret = TRUE;
	if (remotePath.GetLength()>0)
	{
		CFileFindExt ProcessDir;
		ret = ProcessDir.DoProcess(remotePath,FileCopy,m_strCachingDir); 
		if (!ret)
		{
			Clog(LOG_DEBUG,_T("CDODChannel.Copy allfile to cacheFolder  (%s) error!  "),m_sChannelName);
			return 1;
		}
	}

	Clog( LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d]CDODChannel::MoveAllFile Copy data to Local_path OK(%s)"),m_sPortName,m_nPortID,m_sChannelName,m_channelID,m_strCachingDir);
	ret = DeleteDirectory(remotePath.GetBuffer(0),true);
	if (!ret) 
	{
	    int nError = GetLastError();
		char strError[500];

		GetErrorDescription(nError, strError);
		Clog( LOG_DEBUG,_T("Del remote directory error(%s)::GetLastError=%d ,ErrorDescription =  %s "),remotePath,nError,strError);
		return 1;
	}
	Clog( LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d]CDODChannel::Delete remote Directory OK(%s)"),m_sPortName,m_nPortID,m_sChannelName,m_channelID,remotePath);

	return 0;
}
int CDODChannel::StopFileSend(CString sMessageID)
{
	zqMessageList::iterator itrMessage;
	ZQCMessageInfoTINF zqMsgInfo;

	EnterCriticalSection(&(m_channelCriticalSection));	
	if(m_MessageList.size()<1)
	{
		LeaveCriticalSection(&(m_channelCriticalSection));	
		return 0;
	}
	try
	{
		for(itrMessage= m_MessageList.begin();itrMessage != m_MessageList.end();)
		{		
			if (sMessageID.CompareNoCase(itrMessage->sMessageID)!= 0)
			{	
				++itrMessage;	
				continue;
			}

			itrMessage->m_nWorkDuration = 0;
			itrMessage->m_deleteTime = COleDateTime::GetCurrentTime();
			itrMessage->bForever = FALSE;
			
			zqMsgInfo = *itrMessage;
			itrMessage = m_MessageList.erase(itrMessage);
			m_MessageList.push_front(zqMsgInfo);
			SetEvent(m_hWaitEvent);
			Clog( LOG_DEBUG,_T("CDODChannel::StopFileSend find nMessageID = %s (filename = %s)"),sMessageID,zqMsgInfo.m_fileName);
		}
	}
	catch (...) 
	{
		LeaveCriticalSection(&(m_channelCriticalSection));

		int nError = GetLastError();
		char strError[500];

		GetErrorDescription(nError, strError);
		Clog( LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d]DODChannel StopFileSend filename:error,GetLastError()=%d, ErrorDescription = %s "),m_sPortName,m_nPortID,m_sChannelName,m_channelID,nError,strError);
		return 0;
	}
	LeaveCriticalSection(&(m_channelCriticalSection));	

	return 0;
}
int CDODChannel::CreateMsgFile(CString &content,CString Filename,int nLeafTime,CString sMessageID,int ndatatype)
{
	if(m_bIsStop)
		return 0;
	//Clog( LOG_DEBUG,_T("CDODChannel::CreateMsgFile Filename=%s") );

	int strsize = content.GetLength();

	if (strsize < 1)
	{
		Clog(LOG_DEBUG,_T(" CreateMsgFile: message 's length error.") );
		return 1;
	}

	CString sFileName = "Msg";
	SYSTEMTIME time;
	GetLocalTime(&time);
	CString sTime,sDletime;
// Add %3d for time.wMilliseconds;
// ------------------------------------------------------ Modified by zhenan_ji at 2006年1月11日 14:17:00

	sTime.Format("%04d%02d%02d%02d%02d%02d%03d",time.wYear,time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds );
	sFileName = m_strCachingDir+"\\" +sTime ;
	
	ZQCMessageInfoTINF info;
	COleDateTimeSpan olets;
	try
	{	
		info.m_createTime = COleDateTime::GetCurrentTime();
		info.m_nWorkDuration = nLeafTime;
		info.sMessageID = sMessageID;
	     
		if (nLeafTime == 0)
		{
			info.bForever = TRUE;
			info.m_fileName = sFileName +"DEL00000000000000" + Filename ;
		}
		else
		{
			info.bForever = FALSE;
			olets.SetDateTimeSpan(0, 0, 0, info.m_nWorkDuration); 
			info.m_deleteTime = info.m_createTime + olets;
			sDletime = info.m_deleteTime.Format("DEL%Y%m%d%H%M%S");
			info.m_fileName = sFileName +sDletime + Filename;
		}	

		HANDLE hFile;
		hFile = CreateFile(info.m_fileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, NULL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) 
		{
			// ------------------------------------------------------ Modified by zhenan_ji at 2005年10月17日 13:54:59
			int nError = GetLastError();
			char strError[500];

			GetErrorDescription(nError, strError);
			Clog(LOG_DEBUG,_T(" CreateMsgFile error:CreateFile.filename = %s ,GetLastError() = %d ,ErrorDescription = %s"),info.m_fileName,nError,strError);
			return DODcreatetempfileERROR;
		}
		int kk = content.GetLength();
		DWORD kks = 0;
		WriteFile(hFile,content.GetBuffer(0),kk,&kks,NULL);
		CloseHandle(hFile); 
		hFile = NULL;
	}
	catch (...) 
	{
		int nError = GetLastError();
		char strError[500];

		GetErrorDescription(nError, strError);
		Clog(LOG_DEBUG,_T(" CreateMsgFile:filename=%s,write data to file error,GetlastError()=%d, ErrorDescription = %s"),info.m_fileName,nError,strError);
		return 0;
	}
	EnterCriticalSection(&m_channelCriticalSection);
	try
	{
		if(m_MessageList.size() < 1 )
		{
			m_MessageList.push_front(info);
			SetEvent(m_hWaitEvent);
		}
		else
			AddMsgList(&info);
	}
	catch (...) 
	{
		LeaveCriticalSection(&m_channelCriticalSection);
		int nError = GetLastError();
		char strError[500];

		GetErrorDescription(nError, strError);
		Clog(LOG_DEBUG,_T(" CreateMsgFile:filename = %s,push error,GetLastError()= %d, ErrorDescription = %s"),sFileName,nError, strError);
		return 0;
	}
	LeaveCriticalSection(&m_channelCriticalSection);

	Clog( LOG_DEBUG,_T("[%s,PortId:%d-%s,ChannelId:%d]CDODChannel::CreateMsgFile filename=%s"),m_sPortName,m_nPortID,m_sChannelName,m_channelID,info.m_fileName);

	return 0;
}

/*DWORD WINAPI DODFileListhread(LPVOID lpParam) 
{
	CDODChannel *ch = (CDODChannel *)lpParam;
	double  second = 0;
	int i = 0;
	long count = 0;

	zqMessageVector::iterator itrMessage;
	zqMessageVector *mm = &(ch->m_MessageVector);
	COleDateTime oleWriteLogTime = COleDateTime::GetCurrentTime();
	while(!ch->m_bIsStop)
	{
		i = 0;
		while(!ch->m_bIsStop && i < 125)
		{
			Sleep(2);
			i++;
		}

		if(ch->m_bIsStop)		
		{
			Clog( LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d]::DODFileListhread stop !"),ch->m_sPortName,ch->m_nPortID,ch->m_sChannelName ,ch->m_channelID);
			return 0;
		}
		Clog( LOG_INFO,_T("[%s,PortId:%d - %s,ChannelId:%d]It will EnterCriticalSection  "),ch->m_sPortName,ch->m_nPortID,ch->m_sChannelName ,ch->m_channelID );

		EnterCriticalSection(&(ch->m_channelCriticalSection));	

		COleDateTimeSpan olets = COleDateTime::GetCurrentTime() - oleWriteLogTime;

		second = (int) (olets.GetTotalSeconds()); 
		Clog( LOG_INFO,_T("[%s,PortId:%d - %s,ChannelId:%d]get current time span "),ch->m_sPortName,ch->m_nPortID,ch->m_sChannelName ,ch->m_channelID );
		if (second > WRITECHANNELLOGTIME)
		{
			Clog( LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d] Vect size:%d"),ch->m_sPortName,ch->m_nPortID,ch->m_sChannelName ,ch->m_channelID,mm->size());
			oleWriteLogTime = COleDateTime::GetCurrentTime();		
		}
		if(mm->size() < 1)
		{
			LeaveCriticalSection(&(ch->m_channelCriticalSection));	
			Clog( LOG_INFO,_T("[%s,PortId:%d - %s,ChannelId:%d]Current msg totalNumber is zero "),ch->m_sPortName,ch->m_nPortID,ch->m_sChannelName ,ch->m_channelID );
			continue;
		}
		try
		{
			for(itrMessage = mm->begin();itrMessage != mm->end();)
			{		
				if(ch->m_bIsStop)		
				{
					Clog( LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d]CDODChannel::DODFileListhread stop !"),ch->m_sPortName,ch->m_nPortID,ch->m_sChannelName ,ch->m_channelID );
					break;
				}
				if (itrMessage->bForever)
				{
					++itrMessage;
					Clog( LOG_DETAIL,_T("[%s,PortId:%d - %s,ChannelId:%d]find current msg is Forever "),ch->m_sPortName,ch->m_nPortID,ch->m_sChannelName ,ch->m_channelID );
					continue;
				}

				COleDateTimeSpan olets = COleDateTime::GetCurrentTime() - itrMessage->m_createTime;
				second = olets.GetTotalSeconds(); 
				if(int(second) < itrMessage->m_nWorkDuration)
				{
					++itrMessage;
					Clog( LOG_DETAIL,_T("[%s,PortId:%d - %s,ChannelId:%d]find current msg is not exceed time "),ch->m_sPortName,ch->m_nPortID,ch->m_sChannelName ,ch->m_channelID );
					continue;
				}
				Clog( LOG_DETAIL,_T("[%s,PortId:%d - %s,ChannelId:%d]Current message is expired !"),ch->m_sPortName,ch->m_nPortID,ch->m_sChannelName ,ch->m_channelID );

				if(DeleteFile(itrMessage->m_fileName) == FALSE)
				{
					int nError = GetLastError();
					char strError[500];

					GetErrorDescription(nError, strError);
					Clog( LOG_DEBUG,_T("delete nMessageID = %s filename = %s GetLastError() = %d, ErrorDescription = %s"),itrMessage->sMessageID,itrMessage->m_fileName,nError, strError);
				}
				else
					Clog( LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d]delete nMessageID=%s filename=%s OK"),ch->m_sPortName,ch->m_nPortID,ch->m_sChannelName ,ch->m_channelID,itrMessage->sMessageID,itrMessage->m_fileName);
				mm->erase(itrMessage);

				Sleep(1);
				ch->m_bNeedUpdateChannel = TRUE;
				Sleep(1);
				Clog( LOG_DETAIL,_T("[%s,PortId:%d - %s,ChannelId:%d]Setting current channel update flag !"),ch->m_sPortName,ch->m_nPortID,ch->m_sChannelName ,ch->m_channelID);
			}		
		}
		catch (...) 
		{
			LeaveCriticalSection(&(ch->m_channelCriticalSection));

			int nError = GetLastError();
			char strError[500];

			GetErrorDescription(nError, strError);
			Clog( LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d] check or erase error,GetLastError = %d, ErrorDescription = %s !"),ch->m_sPortName,ch->m_nPortID,ch->m_sChannelName ,ch->m_channelID,nError,strError);
			return 0;
		}
		LeaveCriticalSection(&(ch->m_channelCriticalSection));	

//		Clog( LOG_DEBUG,_T("CDODChannel::DODFileListhread:leaving CriticalSection!"));

	}

	Clog( LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d]CDODChannel::DODFileListhread exit !"),ch->m_sPortName,ch->m_nPortID,ch->m_sChannelName ,ch->m_channelID);
	return 1;
}*/

DWORD WINAPI DODFileListhread(LPVOID lpParam) 
{
    CDODChannel *ch = (CDODChannel *)lpParam;

	zqMessageList::iterator itrMessage;
	double  second = 0;
	CString strtime;
	DWORD timeout = INFINITE; 

	while(!ch->m_bIsStop)
	{   
		if(ch->m_MessageList.size() > 0 )
		{
			itrMessage = ch->m_MessageList.begin();
			if(itrMessage->bForever)
			{
				timeout = INFINITE;
			}
			else
			{
				COleDateTimeSpan olets =  itrMessage->m_deleteTime - COleDateTime::GetCurrentTime() ;
			
				second = (int) (olets.GetTotalSeconds());
				if(second > 0)
				  timeout = second * 1000;
				else
				  timeout = 0;
			}
		}
		else
		{
			timeout = INFINITE;
		}

		Clog( LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d] Vect size:%d"),ch->m_sPortName,ch->m_nPortID,ch->m_sChannelName ,ch->m_channelID,ch->m_MessageList.size());

		if( WaitForSingleObject(ch->m_hWaitEvent, timeout) == WAIT_OBJECT_0)
		{
			ResetEvent(ch->m_hWaitEvent);
			continue;
		}	
		EnterCriticalSection(&ch->m_channelCriticalSection);	

		if(DeleteFile(itrMessage->m_fileName) == FALSE)
		{
			int nError = GetLastError();
			char strError[500];

			GetErrorDescription(nError, strError);
			Clog( LOG_DEBUG,_T("delete nMessageID = %s filename = %s GetLastError() = %d, ErrorDescription = %s"),itrMessage->sMessageID,itrMessage->m_fileName,nError, strError);
		}
		else
		{
			strtime = itrMessage->m_deleteTime.Format("%Y-%m-%d %H:%M:%S");
			Clog( LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d]delete nMessageID=%s filename=%s,deletetime = %s OK"),ch->m_sPortName,ch->m_nPortID,ch->m_sChannelName ,ch->m_channelID,itrMessage->sMessageID,itrMessage->m_fileName,strtime);
		}
	
		ch->m_MessageList.erase(itrMessage);
		ch->m_bNeedUpdateChannel = TRUE;
	    CPortManager::AddUpdateChannel(ch);  
		LeaveCriticalSection(&ch->m_channelCriticalSection);
	}
	Clog( LOG_DEBUG,_T("[%s,PortId:%d - %s,ChannelId:%d] Exit DODFileListhread  OK"),ch->m_sPortName,ch->m_nPortID,ch->m_sChannelName ,ch->m_channelID);
	return 1;
}
int CDODChannel::Create()
{
	DWORD IDThread;
	m_bIsStop = FALSE;
	m_bNeedUpdateChannel = FALSE;
// ------------------------------------------------------ Modified by zhenan_ji at 2005年12月7日 18:55:50
// 0:sholeFold,	1:msg:2:local file
	if (m_nType == DATAEXCHANGETYPE_MESSAGE_FORMAT)
	{	
		m_LastUpdateTime = COleDateTime::GetCurrentTime();

		m_hSendThread = CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)DODFileListhread,this,0 , &IDThread); 

		if(m_hSendThread)
		{ 
			Clog(LOG_DEBUG,_T("ChannelID = channel%d, m_nType = DATAEXCHANGETYPE_MESSAGE_FORMAT Create DODFileListhread success! "),m_channelID);
		}
	}	
	return 0;
}
void CDODChannel::Enable(BOOL flag)
{
	m_bIsStop = flag;
}

CString CDODChannel::GetCurrDateTime()
{
	SYSTEMTIME time;
	GetLocalTime(&time);
	CString sTime;
	sTime.Format("%04d%02d%02d%02d:%02d:%02d", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds );
	return sTime;
}
void CDODChannel::Stop()
{
	m_bIsStop = TRUE;
	SetEvent(m_hWaitEvent);
}

void  CDODChannel::AddMsgList(ZQCMessageInfoTINF *pMsginfo)
{
	COleDateTimeSpan olets;
	double second ; 	
	
	EnterCriticalSection(&m_channelCriticalSection);
	
	zqMessageList::iterator itrMessage = m_MessageList.begin();

	if(pMsginfo->bForever)
	{
		Clog(LOG_DEBUG,_T("This Message is a forever Msg! MsgID = %s"),pMsginfo->sMessageID);

		m_MessageList.push_back(*pMsginfo);
	    LeaveCriticalSection(&m_channelCriticalSection);
		return;
	}

	while(itrMessage != m_MessageList.end())
	{
		if(pMsginfo->bForever)
			break;
		olets = pMsginfo->m_deleteTime - itrMessage->m_deleteTime;
		second = olets.GetTotalSeconds();

		if(second > 0 )	
			itrMessage++;
		else
			break;
	}

	if(itrMessage == m_MessageList.begin())
	{
		Clog(LOG_DEBUG,_T("This Message is Push_front! MsgID = %s"),pMsginfo->sMessageID);

		m_MessageList.push_front(*pMsginfo);
		SetEvent(m_hWaitEvent);
	}
	else
		if(itrMessage != m_MessageList.end())
		{
			Clog(LOG_DEBUG,_T("This Message is Push_back! MsgID = %s"),pMsginfo->sMessageID);
			m_MessageList.insert(itrMessage, *pMsginfo);
		}
		else
		{
			Clog(LOG_DEBUG,_T("This Message is Push_middle! MsgID = %s"),pMsginfo->sMessageID);
			m_MessageList.push_back(*pMsginfo);
		}
	LeaveCriticalSection(&m_channelCriticalSection);
}