// DODChannel.cpp: implementation of the CDODChannel class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DODChannel.h"
#include "clog.h"
#include "FileFindExt.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

extern BOOL g_bNotInternalTest;

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
	m_strCachingDir="";
	m_strTag[0]='\0';
	m_bDetected=FALSE;
	m_nType=0;  //m_nType=1 one has lifttime:   m_nType=0 other has not lifttime;
	m_nStreamID=0;
	m_nRepeateTime=5000;
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
	m_nDataType=0;
	m_strName="";
	m_nMessageType=0;
	m_QueueName="";
	m_MessgeVector.clear();
	InitializeCriticalSection(&m_channelCriticalSection);
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
	Clog( LOG_DEBUG,_T("CDODChannel::FullPathModofied") );

	int ret;
	CString str;
	str=m_strCachingDir;
	ret=DeleteDirectory(str.GetBuffer(0));
	if (!ret)
	{
		Clog( 1,_T(" MoveAllFile: delete remote directory error.") );
		return 1;
	}
	MoveAllFile(remotePath,1,subPath);
	return 0;
}

int CDODChannel::DeleteFullPath(CString remotePath,int UpdateMode,CString subPath)
{
	Clog( LOG_DEBUG,_T("CDODChannel::DeleteFullPath") );

	int ret;
	CString str;
	str=m_strCachingDir;
	ret=DeleteDirectory(str.GetBuffer(0));
	if (!ret)
	{
		Clog( 1,_T(" MoveAllFile: delete remote directory error.") );
		return 1;
	}
	return 0;
}

int CDODChannel::DeleteSubFile(CString remotePath,int UpdateMode,CString subPath)
{
	Clog( LOG_DEBUG,_T("CDODChannel::DeleteSubFile") );

	int ret;
	CString str;
	str=m_strCachingDir+subPath;
	ret=DeleteDirectory(str.GetBuffer(0));
	if (!ret)
	{
		Clog( 1,_T(" MoveAllFile: delete remote directory error.") );
		return 1;
	}
	return 0;
}

int CDODChannel::UpdateSubFile(CString remotePath,int UpdateMode,CString subPath)
{
	 CString strtemp,str1;

	 str1=subPath;
	// str1=subPath-remotePath;
	strtemp=m_strCachingDir+str1;

	CFileFindExt ProcessDir;
	BOOL ret=ProcessDir.DoProcess(remotePath,FileCopy,strtemp); 

	if (!ret)
	{
		Clog( 1,_T("CDODChannel.Copy allfile to cacheFolder error!  ") );
		return 1;
	}

	ret=DeleteDirectory(subPath.GetBuffer(0));
	if (!ret)
	{
		Clog( 1,_T(" MoveAllFile: delete remote directory error.") );
		return 1;
	}


	return 0;
}
int CDODChannel::MoveAllFile(CString remotePath,int UpdateMode,CString subPath)
{
	Clog( LOG_DEBUG,_T("CDODChannel::MoveAllFile") );

	CFileFindExt ProcessDir;
	BOOL ret=ProcessDir.DoProcess(remotePath,FileCopy,m_strCachingDir); 
	
	if (!ret)
	{
		Clog( 1,_T("CDODChannel.Copy allfile to cacheFolder error!  ") );
		return 1;
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
*/
	ret=DeleteDirectory(remotePath.GetBuffer(0));
	if (!ret)
	{
		Clog( 1,_T(" remotePath: delete remote directory error.") );
		return 1;
	}

	return 0;
}
int CDODChannel::CreateMsgFile(CString &content,CString Filename,int nLeafTime,int nMessageID,int delOrAdd,int ndatatype)
{
	if(m_bIsStop)
		return 0;
	Clog( LOG_DEBUG,_T("CDODChannel::CreateMsgFile") );

// ------------------------------------------------------ Modified by zhenan_ji at 2005Äê7ÔÂ19ÈÕ 14:44:04

// del message file by messageid and datatype.
	if (delOrAdd==2)
	{
		if(m_MessgeVector.size()<1)
			return 0;

		zqMessageVector::iterator itrMessage;

		EnterCriticalSection(&m_channelCriticalSection);

		for(itrMessage= m_MessgeVector.begin();itrMessage != m_MessgeVector.end();itrMessage++)
		{
			if ((itrMessage->nDataType == ndatatype) && (itrMessage->nMessageID==nMessageID))
			{
				DeleteFile(itrMessage->m_fileName);

				m_MessgeVector.erase(itrMessage);

				Sleep(1);
				m_kit->UpdateCatalog(m_nSessionID,m_nPortID,m_nStreamID);
				Sleep(1);
				LeaveCriticalSection(&m_channelCriticalSection);	
				Clog( LOG_DEBUG,_T("CDODChannel::CreateMsgFile delOrAdd=2") );
				return 0;
			}	
		}		
		LeaveCriticalSection(&m_channelCriticalSection);	
		Clog( LOG_DEBUG,_T("CDODChannel::CreateMsgFile delOrAdd=2 no found") );
		return 0;
	}
	if (delOrAdd !=1)
	{
		Clog( 1,_T(" CreateMsgFile: delOrAdd operationCode error.") );
		return 1;
	}
	int strsize=content.GetLength();

	if (strsize<1)
	{
		Clog( 1,_T(" CreateMsgFile: message 's len error.") );
		return 1;
	}

	CString sFileName="Msg";
	SYSTEMTIME time;
	GetLocalTime(&time);
	CString sTime;
	sTime.Format("%04d%02d%02d%02d%02d%02d", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds );
	sFileName=m_strCachingDir+"\\"+sTime+Filename;

	HANDLE hFile;
	hFile = CreateFile(sFileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, NULL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) 
	{
		Clog( 1,_T(" CreateMsgFile:CreateFile error.") );
		return DODcreatetempfileERROR;
	}
	int kk=content.GetLength();
	//BYTE *aaa=new BYTE[kk];
//	aaa=(BYTE *)(content.GetBuffer(0));
	DWORD kks=0;
	WriteFile(hFile,content.GetBuffer(0),kk,&kks,NULL);
	CloseHandle(hFile); 
	hFile=NULL;
	//delete[] aaa;
	//aaa=NULL;
	ZQCMessageInfoTINF info;

	info.m_fileName=sFileName;
	info.m_createTime=COleDateTime::GetCurrentTime();
	info.m_nWorkDuration=nLeafTime;
	info.nMessageID=nMessageID;
	info.nDataType=ndatatype;
	EnterCriticalSection(&m_channelCriticalSection);
	m_MessgeVector.push_back(info);
	LeaveCriticalSection(&m_channelCriticalSection);	

	Clog( LOG_DEBUG,_T("CDODChannel::CreateMsgFile end") );

	return 0;
}

DWORD WINAPI DODFileListhread(LPVOID lpParam) 
{
	CDODChannel *ch=(CDODChannel *)lpParam;
	double  second=0;
	int i=0;
	long count=0;
	zqMessageVector::iterator itrMessage;
	zqMessageVector *mm=&(ch->m_MessgeVector);
	while(!ch->m_bIsStop)
	{
		i=0;
		while(!ch->m_bIsStop && i<125)
		{
			Sleep(2);
			i++;
		}
		if(ch->m_bIsStop)
			return 0;
	//	flag=FALSE;
		EnterCriticalSection(&(ch->m_channelCriticalSection));	
		 if(mm->size()<1)
			 continue;
		
		for(itrMessage= mm->begin();itrMessage != mm->end();)
		{		
			COleDateTimeSpan olets=COleDateTime::GetCurrentTime() - itrMessage->m_createTime;
			second = olets.GetTotalSeconds(); 
			if(int(second) < itrMessage->m_nWorkDuration)
			{
				++itrMessage;
				continue;
			}
		
			DeleteFile(itrMessage->m_fileName);

			mm->erase(itrMessage);

			Sleep(1);
			ch->m_kit->UpdateCatalog(ch->m_nSessionID,ch->m_nPortID,ch->m_nStreamID);
			Sleep(1);
			
			Clog( LOG_DEBUG,_T("CDODChannel::DODFileListhread DeleteFile and UpdateCatalog") );
		}		
		LeaveCriticalSection(&(ch->m_channelCriticalSection));	
	}
	return 1;
}
int CDODChannel::Create()
{
	if(m_kit==NULL)
		return 1;

	DWORD IDThread;
	m_bIsStop =FALSE;
	if (m_nType)
	{	
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