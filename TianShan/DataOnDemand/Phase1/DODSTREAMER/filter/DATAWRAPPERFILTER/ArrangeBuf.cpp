#include "StdAfx.h"
#include ".\arrangebuf.h"

CArrangeBuf::CArrangeBuf(void)
{
	m_strFilename[0]='\0';
	m_hFile=INVALID_HANDLE_VALUE;
	m_nTotalPacketNumber=0;
	m_nUsedPacketNumber=0;
	m_nFirstInsetPad=1;
	m_bFileOver=FALSE;
	m_bIsFirstPadding=TRUE;
	m_nPID=0;
}

CArrangeBuf::~CArrangeBuf(void)
{
	char strLog[MAX_PATH];	
	if (m_hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFile);
		m_hFile=INVALID_HANDLE_VALUE;
		DeleteFile(m_strFilename);
		wsprintf(strLog,"PID=%d:DeleteFile ok filename=(%s) ",m_nPID,m_strFilename);		LogMyEvent(1,0,strLog);	
	}
}
int CArrangeBuf::ReNewOpenFile()
{	
	if (m_hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFile);
		m_hFile=INVALID_HANDLE_VALUE;
	}
	m_nTotalPacketNumber=0;
	m_nUsedPacketNumber=0;
	m_nFirstInsetPad=1;
	m_bFileOver=FALSE;
	m_bIsFirstPadding=TRUE;
	char strLog[MAX_PATH];	

	//wsprintf(strLog,"PID=%d:CreateFile init strfilename=(%s)",m_nPID,m_strFilename);		LogMyEvent(1,0,strLog);	

	m_hFile = CreateFile(m_strFilename, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, NULL, NULL);
	if (m_hFile == INVALID_HANDLE_VALUE) 
	{			
		DWORD dwErr =  ::GetLastError();
		wsprintf(strLog,"PID=%d:CreateFile error strfilename=(%s) errorcode=%d",m_nPID,m_strFilename,dwErr);		LogMyEvent(1,0,strLog);	
		m_hFile = INVALID_HANDLE_VALUE;
		return 0;
	}
	//wsprintf(strLog,"PID=%d:CreateFile ok strfilename=(%s)",m_nPID,m_strFilename);		LogMyEvent(1,0,strLog);	
	return 0;
}

