// TrickJob.h: interface for the CTrickJob class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TRICKJOB_H__97F6A54D_2D0A_4AA1_8EFF_1A413F9E721E__INCLUDED_)
#define AFX_TRICKJOB_H__97F6A54D_2D0A_4AA1_8EFF_1A413F9E721E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning(disable:4786)

#include <list>
#include <string>
using namespace std;

#include "TrickModule/TrickImportUser.h"
#include "ShareBufferMan.h"

#include "activeSession.h"

#define MAX_DATA_BUF_SIZE               (((4*1024*1024)/8)*3)  // the default buffer size if the
#define NT_MPEG_BUFFER_LENGTH			(1024 * MPEG2_TRANSPORT_PACKET_SIZE)


class Mutex
{
public:
	///initializes the critical section object
	Mutex()
		{ ::InitializeCriticalSection(&_mutex); }

	///releases all resources used by an unowned critical section object.
	~Mutex()
		{ ::DeleteCriticalSection(&_mutex); }

	///waits for ownership of the specified critical section object,The function returns when the calling thread is granted ownership.
	void enter()
		{::EnterCriticalSection(&_mutex);}

	///releases ownership of the specified critical section object.
	void leave()
		{::LeaveCriticalSection(&_mutex);}
		
#if _WIN32_WINNT >=0x0400
	bool tryEnter()
		{return (::TryEnterCriticalSection(&_mutex) == TRUE);}
#endif //_WIN32_WINNT 

private:

	CRITICAL_SECTION  _mutex;
	///can't be copied!!!
	Mutex(const Mutex &);
	Mutex &operator=(const Mutex &);
};


class CMpegBufferPool : public CBufferPool
{
public:
	CMpegBufferPool();
	~CMpegBufferPool();
	//
	// CBufferPool methods
	//
	SDataBuffer *Alloc();
	void		Free(SDataBuffer *buf);
	void		SetBufferSize(DWORD size);

	CLibBuffer	m_bufferDescriptors;
	
	queue<void*>m_ioBuffers;	
	Mutex		m_ioMutex;

	int			m_bufferSize;
};


class CTrickJob : public CBufferPool
{
public:
	int GetJobCount();
	void SetData(LPVOID lpBuf,int nDataLen);
	void SetBufferSize(int nLen);
	BOOL CreateJob();
	
	CTrickJob();
	virtual ~CTrickJob();

	virtual SDataBuffer *Alloc();
	virtual void Free(SDataBuffer *buf);

public:
	void SetJobRunFlag();
	void SetJobStopFlag();
	BOOL IsRunning();
	void StopJob();
	void SetFilePathName(wchar_t* pPath );
	void SetMaxMpegCodingErrors(int nCode);
    void STDMETHODCALLTYPE SetMulticastAddress(BSTR strAddr);
	void STDMETHODCALLTYPE SetSourcePort(UINT uPort);
	void STDMETHODCALLTYPE SetDestport(UINT uPort);
	void STDMETHODCALLTYPE SetSubscribeFilename(BSTR strFileName);
	void STDMETHODCALLTYPE SetPublisherIP(BSTR strAddr);
	void STDMETHODCALLTYPE SetPublisherInterfaceIP(BSTR strAddr);
	HRESULT AddSubscriber(BSTR ip1,BSTR ip2);

private:
	void DeleteTrickFile();

	CMpegBufferPool _bufferPool;
	wchar_t			m_szPathName[MAX_FILENAME_SIZE];
	CShareBufferMan m_SBM;
	int				m_nBufferSize;

	//for publish
	wchar_t			m_szMulticastAddress[MAX_IP_SIZE];
	int				m_uSourcePort;
	int				m_uDestPort;
	wchar_t			m_szSubscribeFilename[MAX_FILENAME_SIZE];
	wchar_t			m_szPublisherIP[MAX_IP_SIZE];
	wchar_t			m_szPublisherInterfaceIP[MAX_IP_SIZE];
	int				m_nCodingError;

	list<wstring>	m_SubscriberIpList;
	list<wstring>	m_SubscriberInterIpList;

	//job run
	BOOL			m_bJobRun;
	
	//publish
	ActiveSession	m_Session;
};
#pragma warning(default :4786)
#endif // !defined(AFX_TRICKJOB_H__97F6A54D_2D0A_4AA1_8EFF_1A413F9E721E__INCLUDED_)
