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

//extern BOOL g_bNotInternalTest;

#define WRITECHANNELLOGTIME 180


CDODChannel::CDODChannel()
{
	Init();
}

CDODChannel::~CDODChannel()
{
	DeleteCriticalSection(&m_channelCriticalSection);
	m_bIsStop=TRUE;	
	Sleep(1);

	m_MessgeVector.clear();
}
void CDODChannel::Init()
{	
	m_nMsgNumber=0;
	m_kit=NULL;
	m_strCachingDir="NoInitial";
	m_strTag[0]='\0';
	m_bDetected=FALSE;
	m_nType=0;  //m_nType=1 one has lifttime:   m_nType=0 other has not lifttime;
	m_nStreamID=0;
	m_nRepeateTime=0;
	m_bIsStop=FALSE;
	m_bEncrypted=0;
	m_nSessionID=0;
	m_nRate=0;
//	m_pDataChaching=0;
//	m_nWorkDuration=0;
	nStreamType=0;
	m_channelID=0;
	m_DetectInterVal=5000;
	m_nPortID=0;
	m_sDataType="";
//	m_nUpdateInterval=0;
	m_sChannelName="";
	m_sPortName="";
	m_nMessageType=0;
	m_bNeedUpdateChannel=FALSE;
	m_QueueName="";
	m_MessgeVector.clear();
	InitializeCriticalSection(&m_channelCriticalSection);
	m_nMultiplestream=0;
	m_nStreamCount=0;
	m_nSendWithDestination=0;
	m_sSendMsgDataType="";
	m_nSendMsgExpiredTime=0;
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

int CDODChannel::FullPathModofied(CString remotePath,int UpdateMode,CString subPath)
{
//	Clog( LOG_DEBUG,_T("[%s-%s]CDODChannel::FullPathModofied"),m_sPortName,m_sChannelName);
	return MoveAllFile(remotePath,1,subPath);
}

int CDODChannel::DeleteFullPath(CString remotePath,int UpdateMode,CString subPath)
{
	Clog( LOG_DEBUG,_T("[%s-%s]CDODChannel::DeleteFullPath remotePath=%s") ,m_sPortName,m_sChannelName,remotePath);
	int ret;
	CString str;
	str=m_strCachingDir;
	ret=DeleteDirectory(str.GetBuffer(0));
	if (!ret)
	{
		Clog(LOG_DEBUG,_T(" MoveAllFile: delete remote directory error."),m_sChannelName );
		return 1;
	}
	return 0;
}

int CDODChannel::DeleteSubFile(CString remotePath,int UpdateMode,CString subPath)
{
	Clog( LOG_DEBUG,_T("[%s-%s]CDODChannel::DeleteDIRorFile remotePath=%s"),m_sPortName,m_sChannelName,remotePath);
	int ret;
	CString str;
	str=m_strCachingDir+"\\"+remotePath;
	ret=DeleteFile(str.GetBuffer(0));
	if (!ret)
	{
		int ddd=GetLastError();
		Clog(LOG_DEBUG,_T(" DeleteDIRorFile:%s error.GetLastError()=%d (%s)"),str,ddd,m_sChannelName);
		return 1;
	}
	return 0;
}

int CDODChannel::CreateOrReplaceFile(CString remotePath,int UpdateMode,CString subPath)
{
	//CString strSource,strCacheDir;
	Clog( LOG_DEBUG,_T("[%s-%s]CDODChannel::CreateOrReplaceFile remotePath=%s,subPath=%s"),m_sPortName,m_sChannelName,remotePath,subPath);

	CFileFindExt ProcessDir;
	BOOL ret=ProcessDir.DoProcess(remotePath,FileCopy,m_strCachingDir); 
	if (!ret)
	{		
		int nError=GetLastError();
		Clog(LOG_DEBUG,_T("CDODChannel.Copy allfile to cacheFolder error! GetLastError()=%d"),nError );
		return 1;
	}

	ret=DeleteDirectory(remotePath.GetBuffer(0));
	if (!ret)
	{
		int nError=GetLastError();
		Clog(LOG_DEBUG,_T(" MoveAllFile: delete remote directory (%s) error GetLastError()=%d"),remotePath,nError);
		return 1;
	}
	return 0;
}
int CDODChannel::UpdateSubfolder(CString remotePath,int UpdateMode,CString subPath)
{
	 CString strtemp,str1;
	 Clog( LOG_DEBUG,_T("[%s-%s]CDODChannel::UpdateSubfolder remotePath=%s"),m_sPortName,m_sChannelName,remotePath);

	 strtemp=m_strCachingDir+subPath;
	 str1=remotePath;

	 BOOL ret=DeleteDirectory(strtemp.GetBuffer(0));
	 if (!ret)
	 {
		 int nError=GetLastError();
		 Clog(LOG_DEBUG,_T(" MoveAllFile: delete local directory (%s) error:GetLastError=%d"),strtemp,nError );
		// return 1;
	 }
	strtemp=m_strCachingDir;
	CFileFindExt ProcessDir;
	ret=ProcessDir.DoProcess(str1,FileCopy,strtemp); 

	if (!ret)
	{ 
		int nError=GetLastError();
		Clog(LOG_DEBUG,_T("CDODChannel.Copy allfile to cacheFolder error! GetLastError=%d"),nError );
		return 1;
	}

	ret=DeleteDirectory(remotePath.GetBuffer(0));
	if (!ret)
	{
		Clog(LOG_DEBUG,_T(" MoveAllFile: delete remote directory (%s) error"),m_sChannelName );
		return 1;
	}
	return 0;
}
int CDODChannel::MoveAllFile(CString remotePath,int UpdateMode,CString subPath)
{
	Clog( LOG_DEBUG,_T("[%s-%s]CDODChannel::MoveAllFile for remotePath=(%s)"),m_sPortName,m_sChannelName,remotePath);
	BOOL ret=TRUE;
	if (remotePath.GetLength()>0)
	{
		CFileFindExt ProcessDir;
		ret=ProcessDir.DoProcess(remotePath,FileCopy,m_strCachingDir); 
	//	Clog( LOG_DEBUG,_T("CDODChannel::MoveAllFileProcessDir.DoProcess") );

		if (!ret)
		{
			Clog(LOG_DEBUG,_T("CDODChannel.Copy allfile to cacheFolder  (%s) error!  "),m_sChannelName);
			return 1;
		}
	}
	/*
	CString	DstDir="d:\\DstDir";
	
	CFileFindExt ProcessDir;
	BOOL ret=ProcessDir.DoProcess(CurDir,FileCopy,DstDir); 
	
	if (!ret)
	{

	}


	CString CurDir = "\\\\192.168.80.5\\User Data\\Leon.li\\ToRose"; 
	CString CurDir = "\\\\192.168.80.21\\data\\share"; 
	CString CurDir = "\\\\192.168.80.95\\forDOD\\1125653318750"; 
	
	//remotePath+="\\H0\\H0.txt";		ret=DeleteFile(remotePath); 
	Sleep(10000);*/
	ret=DeleteDirectory(remotePath.GetBuffer(0));
	if (!ret) 
	{
		int nError=GetLastError();
		Clog( LOG_DEBUG,_T("Del remote directory error(%s)::GetLastError=%d error "),remotePath,nError);
		return 1;
	}

	return 0;
}
int CDODChannel::StopFileSend(CString sMessageID)
{
	zqMessageVector::iterator itrMessage;

	EnterCriticalSection(&(m_channelCriticalSection));	
	if(m_MessgeVector.size()<1)
	{
		LeaveCriticalSection(&(m_channelCriticalSection));	
		return 0;
	}
	try
	{
		for(itrMessage= m_MessgeVector.begin();itrMessage != m_MessgeVector.end();)
		{		
			if (sMessageID.CompareNoCase(itrMessage->sMessageID)!=0)
			{
				++itrMessage;
				continue;
			}

			itrMessage->m_nWorkDuration=0;
			itrMessage->bForever=FALSE;
			Clog( LOG_DEBUG,_T("CDODChannel::StopFileSend find nMessageID=%s (filename=%s)"),sMessageID,itrMessage->m_fileName);
			break;
		}
	}
	catch (...) 
	{
		LeaveCriticalSection(&(m_channelCriticalSection));	
		Clog( LOG_DEBUG,_T("[%s-%s]DODChannel StopFileSend filename:error,lasterror=%d "),m_sPortName,m_sChannelName,GetLastError());
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

	int strsize=content.GetLength();

	if (strsize<1)
	{
		Clog(LOG_DEBUG,_T(" CreateMsgFile: message 's len error.") );
		return 1;
	}

	CString sFileName="Msg";
	SYSTEMTIME time;
	GetLocalTime(&time);
	CString sTime;
// Add %3d for time.wMilliseconds;
// ------------------------------------------------------ Modified by zhenan_ji at 2006年1月11日 14:17:00

	sTime.Format("%04d%02d%02d%02d%02d%02d%03d",time.wYear,time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds );
	sFileName=m_strCachingDir+"\\"+sTime+Filename;

	try
	{
		HANDLE hFile;
		hFile = CreateFile(sFileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, NULL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) 
		{
			// ------------------------------------------------------ Modified by zhenan_ji at 2005年10月17日 13:54:59
			Clog(LOG_DEBUG,_T(" CreateMsgFile:CreateFile.filename=%s,lasterror=%d error"),sFileName,GetLastError());
			return DODcreatetempfileERROR;
		}
		int kk=content.GetLength();
		DWORD kks=0;
		WriteFile(hFile,content.GetBuffer(0),kk,&kks,NULL);
		CloseHandle(hFile); 
		hFile=NULL;
	}
	catch (...) 
	{
		Clog(LOG_DEBUG,_T(" CreateMsgFile:filename=%s,write data to file error,lasterror=%d"),sFileName,GetLastError());
		return 0;
	}
	EnterCriticalSection(&m_channelCriticalSection);
	try
	{
		ZQCMessageInfoTINF info;

		info.m_fileName=sFileName;
		info.m_createTime=COleDateTime::GetCurrentTime();
		info.m_nWorkDuration=nLeafTime;
		info.sMessageID=sMessageID;
		info.bForever=FALSE;
		if (nLeafTime==0)
			info.bForever=TRUE;
		m_MessgeVector.push_back(info);
	}
	catch (...) 
	{
		LeaveCriticalSection(&m_channelCriticalSection);	
		Clog(LOG_DEBUG,_T(" CreateMsgFile:filename=%s,push error,lasterror=%d"),sFileName,GetLastError());
		return 0;
	}
	LeaveCriticalSection(&m_channelCriticalSection);	
	Clog( LOG_DEBUG,_T("[%s-%s]CDODChannel::CreateMsgFile filename=%s"),m_sPortName,m_sChannelName,sFileName);

	return 0;
}

DWORD WINAPI DODFileListhread(LPVOID lpParam) 
{
	CDODChannel *ch=(CDODChannel *)lpParam;
	double  second=0;
	int i=0;
	long count=0;

//	int sss=0;
//	sss=ch->m_MessgeVector.size();

	zqMessageVector::iterator itrMessage;
	zqMessageVector *mm=&(ch->m_MessgeVector);
	COleDateTime oleWriteLogTime=COleDateTime::GetCurrentTime();
	while(!ch->m_bIsStop)
	{
		i=0;
		while(!ch->m_bIsStop && i<125)
		{
			Sleep(2);
			i++;
		}
		if(ch->m_bIsStop)		
		{
			Clog( LOG_DEBUG,_T("CDODChannel::DODFileListhread stop !") );
			return 0;
		}
		EnterCriticalSection(&(ch->m_channelCriticalSection));	

		COleDateTimeSpan olets=COleDateTime::GetCurrentTime() - oleWriteLogTime;
		second =(int) (olets.GetTotalSeconds()); 
		if (second > WRITECHANNELLOGTIME)
		{
			Clog( LOG_DETAIL,_T("[%s-%s] Vect size:%d"),ch->m_sPortName,ch->m_sChannelName,mm->size());
			oleWriteLogTime=COleDateTime::GetCurrentTime();		
		}
		if(mm->size()<1)
		{
			LeaveCriticalSection(&(ch->m_channelCriticalSection));	
			continue;
		}
		try
		{
			for(itrMessage= mm->begin();itrMessage != mm->end();)
			{		
				if (itrMessage->bForever)
				{
					++itrMessage;
					continue;
				}

				COleDateTimeSpan olets=COleDateTime::GetCurrentTime() - itrMessage->m_createTime;
				second = olets.GetTotalSeconds(); 
				if(int(second) < itrMessage->m_nWorkDuration)
				{
					++itrMessage;
					continue;
				}

				if(DeleteFile(itrMessage->m_fileName)==FALSE)
				{
					DWORD  oERR=GetLastError();
					Clog( LOG_DEBUG,_T("delete nMessageID=%s filename=%s LastError=%d"),itrMessage->sMessageID,itrMessage->m_fileName,oERR);
				}
				else
					Clog( LOG_DEBUG,_T("delete nMessageID=%s filename=%s OK"),itrMessage->sMessageID,itrMessage->m_fileName);
				mm->erase(itrMessage);

				Sleep(1);
				//ch->m_kit->UpdateCatalog(ch->m_nSessionID,ch->m_nPortID,ch->m_nStreamID);
				ch->m_bNeedUpdateChannel=TRUE;
				Sleep(1);
				// Clog( LOG_DEBUG,_T("CDODChannel::DODFileListhread DeleteFile and UpdateCatalog") );
			}		
		}
		catch (...) 
		{
			LeaveCriticalSection(&(ch->m_channelCriticalSection));	
			Clog( LOG_DEBUG,_T("[%s-%s] check or erase error,lasterror=%d:: !"),ch->m_sPortName,ch->m_sChannelName,GetLastError());
			return 0;
		}
		LeaveCriticalSection(&(ch->m_channelCriticalSection));	
	}

	Clog( LOG_DEBUG,_T("[%s-%s]CDODChannel::DODFileListhread exit !"),ch->m_sPortName,ch->m_sChannelName);
	return 1;
}
int CDODChannel::Create()
{
	if(m_kit==NULL)
		return 1;

	DWORD IDThread;
	m_bIsStop =FALSE;
	m_bNeedUpdateChannel=FALSE;
// ------------------------------------------------------ Modified by zhenan_ji at 2005年12月7日 18:55:50
// 0:sholeFold,	1:msg:2:local file
	if (m_nType==DATAEXCHANGETYPE_MESSAGE_FORMAT)
	{	
		m_LastUpdateTime=COleDateTime::GetCurrentTime();
		m_hSendThread=CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)DODFileListhread,this,0 , &IDThread); 
	}
	
	return 0;
}
void CDODChannel::Enable(BOOL flag)
{
	m_bIsStop=flag;
}

CString CDODChannel::GetCurrDateTime()
{
	SYSTEMTIME time;
	GetLocalTime(&time);
	CString sTime;
	sTime.Format("%04d%02d%02d%02d:%02d:%02d", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds );
	return sTime;
}