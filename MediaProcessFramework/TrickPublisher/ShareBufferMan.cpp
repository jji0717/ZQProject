// ShareBufferMan.cpp: implementation of the CShareBufferMan class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ShareBufferMan.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#include "BufPoolLock.h"
CShareBufferMan::CShareBufferMan()
{
	m_hNotify=CreateEvent(NULL,FALSE,FALSE,NULL);

	InitializeCriticalSection(&m_ShareBufCri);
	InitializeCriticalSection(&m_FreeBufCri);
}

CShareBufferMan::~CShareBufferMan()
{
	while(m_BufQueBak.size()>0)
	{
		SDataBuffer*buf=(SDataBuffer*)m_BufQueBak.front();
		m_BufQueBak.pop();
		if(buf)
		{
			if(buf->mpegBuffer.pointer)
			{
				delete  buf->mpegBuffer.pointer;
				buf->mpegBuffer.pointer=NULL;
			}
			
			delete buf;
			buf=NULL;
		}

	}

	ResetEvent(m_hNotify);
	CloseHandle(m_hNotify);
	m_hNotify=NULL;
	
	DeleteCriticalSection(&m_ShareBufCri);
	DeleteCriticalSection(&m_FreeBufCri);
}

BOOL CShareBufferMan::SetData(LPVOID lpBuf,int nDataLen)
{
	CBufPoolLock lock(m_ShareBufCri);

	if(lpBuf==NULL)
	{
		m_BufQue.push(NULL);
	}
	else
	{
		SDataBuffer *buf=GetBuffer();
	
		if(!buf)return FALSE;

		if(nDataLen<m_nFreeBufQueSize)
			ZeroMemory(buf->mpegBuffer.pointer,m_nFreeBufQueSize);

		memcpy(buf->mpegBuffer.pointer,lpBuf,nDataLen);
		buf->mpegBuffer.length=nDataLen;
		buf->len=nDataLen;
		
		m_BufQue.push(buf);
	}

	SetEvent(m_hNotify);
	return TRUE;
}

SDataBuffer* CShareBufferMan::GetData()
{	
	WaitForSingleObject(m_hNotify,-1);

	CBufPoolLock lock(m_ShareBufCri);

	SDataBuffer *buf=m_BufQue.front();
	m_BufQue.pop();

	if(buf==NULL)
	{
		return NULL;
	}

	int nSize=m_BufQue.size();

	if(nSize!=0)
		SetEvent(m_hNotify);

	return buf;
}

int CShareBufferMan::GetSize()
{
	CBufPoolLock lock(m_ShareBufCri);
	
	return m_BufQue.size();
}

void CShareBufferMan::InitBufferQue(int nSize)
{
	m_nFreeBufQueSize=nSize;
}

SDataBuffer* CShareBufferMan::GetBuffer()
{
	CBufPoolLock lock(m_FreeBufCri);

	if(m_FreeBufQue.size()==0)
	{

		SDataBuffer*buf=new SDataBuffer;
		buf->mpegBuffer.pointer=new UCHAR[m_nFreeBufQueSize];
		ZeroMemory(buf->mpegBuffer.pointer,m_nFreeBufQueSize);

		m_BufQueBak.push(buf);

		return buf;
	}

	SDataBuffer*buf=(SDataBuffer*)m_FreeBufQue.front();

	m_FreeBufQue.pop();

	return buf;
}

void CShareBufferMan::FreeBuffer(SDataBuffer *pBuf)
{
	CBufPoolLock lock(m_FreeBufCri);
	m_FreeBufQue.push(pBuf);
}

