// ShareBufferMan.h: interface for the CShareBufferMan class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SHAREBUFFERMAN_H__3554C045_0D72_4721_8ECE_52C5C275B31D__INCLUDED_)
#define AFX_SHAREBUFFERMAN_H__3554C045_0D72_4721_8ECE_52C5C275B31D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "trickmodule/BufferPool.h"
#include <queue>

#define QUE_END -100

class CShareBufferMan  
{
public:
	void FreeBuffer(SDataBuffer*pBuf);

	void InitBufferQue(int nSize);
	int GetSize();
	SDataBuffer* GetData();
	BOOL SetData(LPVOID lpBuf,int nDataLen);
	CShareBufferMan();
	virtual ~CShareBufferMan();

private:
	SDataBuffer* GetBuffer();

private:
	std::queue<SDataBuffer*>	m_BufQue;

	HANDLE						m_hNotify;

//	int m_nBufferSize;
	CRITICAL_SECTION			m_ShareBufCri;

	//free buf
	std::queue<PVOID>			m_FreeBufQue;
	std::queue<PVOID>			m_BufQueBak;

	int							m_nFreeBufQueSize;
	CRITICAL_SECTION			m_FreeBufCri;
};

#endif // !defined(AFX_SHAREBUFFERMAN_H__3554C045_0D72_4721_8ECE_52C5C275B31D__INCLUDED_)
