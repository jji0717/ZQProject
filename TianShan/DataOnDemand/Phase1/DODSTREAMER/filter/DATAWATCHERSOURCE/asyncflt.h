//------------------------------------------------------------------------------
// File: AsyncFlt.h
//
// Desc: DirectShow sample code - header file for async filter.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

//
//  Define an internal filter that wraps the base CBaseReader stuff
//
//#include "MXFGuids.h"
#include "log_lxg.h"
#include "interfaceDefination.h"
#include "resource1.h"
#include "common.h"
#include "tchar.h"

#ifndef _NO_FIX_LOG
#include "fltinit.h"
#endif

/////////////////////////////////////////////////////////////////////////////////////////
//class CCatalogSourceFilter
class CCatalogSourceFilter : 
	public CAsyncReader, 
	public IFileSourceFilter, 
	public ISpecifyPropertyPages, 
	public ICatalogConfigure, 
#ifndef _NO_FIX_LOG
	public IFilterInit, 
#endif
	public ICatalogState
{
public:
    CCatalogSourceFilter(LPUNKNOWN pUnk, HRESULT *phr) :
        CAsyncReader(NAME("Mem Reader"), pUnk, &m_Stream, phr),
        m_pFileName(NULL),
        m_pbData(NULL),
		m_pszCatalog(0),
		m_hEvtMonitor(NULL),
		m_hEvtStop(NULL),
		m_hEvtSubChannelStateEvent(NULL),
		m_hMonitor(NULL),
		m_bDetectedFlag(FALSE),
		m_lDetectedInterval(5000),
		m_catalogStateEvent(NULL),
		m_nErrorCode(0),
		m_currentStatus(STATE_READY),		
		m_iSetCatalogTime(0),
		m_nCanUpdateCatalog(1)
    {
		m_IsStop = FALSE;
		m_dwID = g_dwInstanceID++;
		SetID();
		//smglog(SMGLOG_DETAIL, m_dwID, " CCatalogSourceFilter::CCatalogSourceFilter.start");

		InitializeCriticalSection(&m_lock);
    }

    ~CCatalogSourceFilter()
    {
        if( NULL != m_pbData )
		{
			delete [] m_pbData;
			m_pbData=NULL;
		}
		if( NULL != m_pFileName )
		{
			delete [] m_pFileName;
			m_pFileName=NULL;
		}
		if( NULL != m_pszCatalog)
		{
			delete[] m_pszCatalog;
			m_pszCatalog=NULL;
		}		
		if( NULL != m_hMonitor )
		{
			CloseThread();
			m_hMonitor=NULL;
		}

// ------------------------------------------------------ Modified by zhenan_ji at 2005年7月16日 11:19:11
		Catalogs.clear();
		SubChannellists.clear();
		TempSubChannellists.clear();
		DeleteCriticalSection(&m_lock);

		smglog(SMGLOG_DETAIL, m_dwID, "CCatalogSourceFilter::~CCatalogSourceFilter ok.");
    }

    static CUnknown * WINAPI CreateInstance(LPUNKNOWN, HRESULT *);
public:

	// added by Cary
#ifndef _NO_FIX_LOG
	
	virtual void initLog(ISvcLog* log)
	{
		extern ISvcLog* service_log;
		service_log = log;
	}
#endif

	//zhenan add it for createThread 
	STDMETHODIMP Run(REFERENCE_TIME tStart);
//	void Stop();
	STDMETHODIMP Stop(){
		CAsyncReader::Stop(); 
		smglog(SMGLOG_DETAIL, m_dwID, "CCatalogSourceFilter() stop end .");
		return S_OK;}
    DECLARE_IUNKNOWN
	

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv)
    {
/*        if (riid == IID_IFileSourceFilter) 
		{
            return GetInterface((IFileSourceFilter *)this, ppv);
        } 
		else */if(riid == IID_ISpecifyPropertyPages )
		{
			return GetInterface( (ISpecifyPropertyPages *)this, ppv );
		}
		else if(riid == IID_ICatalogConfigure )
		{
			return GetInterface( (ICatalogConfigure *)this, ppv );
		}
		else if(riid == IID_ICatalogState )
		{
			return GetInterface( (ICatalogState *)this, ppv );
		}

#ifndef _NO_FIX_LOG
		else if (riid == IID_IFilterInit) {
			return GetInterface( (IFilterInit *)this, ppv );
		}
#endif
		else 
		{
            return CAsyncReader::NonDelegatingQueryInterface(riid, ppv);
        }
    }

    /*  IFileSourceFilter methods */

    //  Load a (new) file
    STDMETHODIMP Load(LPCOLESTR lpwszFileName, const AM_MEDIA_TYPE *pmt)
    {
		CheckPointer(lpwszFileName, E_POINTER);	
        // lstrlenW is one of the few Unicode functions that works on win95
        int cch = lstrlenW(lpwszFileName) + 1;
#ifndef UNICODE
        TCHAR *lpszFileName=0;
        lpszFileName = new char[cch * 2];
        if (!lpszFileName) 
		{
			m_nErrorCode=OUTOFMEMORY;
			smglog(SMGLOG_ERROR, m_dwID, "Load filename,len(%d) out of memory .",cch);
      	    return E_OUTOFMEMORY;
        }
        WideCharToMultiByte(GetACP(), 0, lpwszFileName, -1,lpszFileName, cch, NULL, NULL);
#else
        TCHAR lpszFileName[FILENAMELEN]={0};
        lstrcpy(lpszFileName, lpwszFileName);
#endif
        CAutoLock lck(&m_csFilter);

        /*  Check the file type */
        CMediaType cmt;
        if (NULL == pmt) 
		{
            cmt.SetType(&MEDIATYPE_Stream);
        } 
		else 
		{
            cmt = *pmt;
        }
        if (!ReadTheFile(lpszFileName)) 
		{
#ifndef UNICODE
            delete [] lpszFileName;lpszFileName=NULL;
#endif
            return E_FAIL;
        }
        m_Stream.Init(m_pbData, m_llSize);//it's very important! 
        m_pFileName = new WCHAR[cch];

        if (m_pFileName!=NULL)
    	    CopyMemory(m_pFileName, lpwszFileName, cch*sizeof(WCHAR));

    	m_mt = cmt;

        cmt.bTemporalCompression = TRUE;
        cmt.lSampleSize = 1;
        return S_OK;
    }


    STDMETHODIMP GetCurFile(LPOLESTR * ppszFileName, AM_MEDIA_TYPE *pmt)
    {
        CheckPointer(ppszFileName, E_POINTER);
        *ppszFileName = NULL;

        if (m_pFileName!=NULL) 
		{
        	DWORD n = sizeof(WCHAR)*(1+lstrlenW(m_pFileName));

            *ppszFileName = (LPOLESTR) CoTaskMemAlloc( n );
            if (*ppszFileName!=NULL)
			{
                  CopyMemory(*ppszFileName, m_pFileName, n);
            }
        }

        if (pmt!=NULL) 
		{
            CopyMediaType(pmt, &m_mt);
        }

        return NOERROR;
    }
	
	/*  ISpecifyPropertyPages methods */
	STDMETHODIMP GetPages(CAUUID* pPages);

	/*  ICatalogConfigure methods */
	STDMETHODIMP 	SetCatalogName(LPCOLESTR pszCatalog);
	STDMETHODIMP	GetCatalogName(LPOLESTR* ppszCatalog);
	STDMETHODIMP	UpdateCatalog();
	STDMETHODIMP	SetDetectedFlag(BOOL bDetected);
	STDMETHODIMP	GetDetectedFlag (BOOL* bDetected);
	STDMETHODIMP	SetDetectedInterval(long lInterval) ;
	STDMETHODIMP	GetDetectedInterval (long* lInterval);
	STDMETHODIMP	GetCatalogs(vector<DWS_SubChannelList > *pSubChannellists) ;
	STDMETHODIMP	SetSubChannelRate(vector<DWS_SubChannelList > pSubChannellists, long nSubChannel);
	STDMETHODIMP    SetChannelEvent(HANDLE hStateEvent);

	/*  ICatalogState methods */
	STDMETHODIMP SetCatalogStateEvent(HANDLE hStateEvent);	 
	STDMETHODIMP GetLastErrorCode(int* nError);
	STDMETHODIMP GetCurrentStatus(int* pnStatus);

public:

	//Search path
	BOOL SearchPath( TCHAR * CurrentPath );
	//Deal path
	BOOL DealPath( TCHAR * CurrentPath );
	//Clear file vector
	void ClearFileVector();
	//Monitor catalog thread
	BOOL MonitorCatalogThread();
	////Monitor catalog
	static DWORD WINAPI MonitorCatalog(LPVOID lParam); 
	// stop the thread and close the handle
    HRESULT CloseThread(void);
	//Catalog status process
	void OnCatalogStatus(vector<DWS_SubChannelList > pSubChannellist);
	//Compare subchanel list
	BOOL CompareSubchanelList(vector<DWS_SubChannelList > NewSubChannellists,vector<DWS_SubChannelList > OldSubChannellists);
	//Delele Catalog
	void DelCatalog(  CCatalog * pCatalog );
	//Search File
	void SearchFile( std::vector<CCatalog>::iterator it );
	//Load SubChannel list
	BOOL LoadSubChannelList( char * FileName, WORD id  );
	//Delele SubChannel list
	void DelSubChannellists( DWS_SubChannelList * pSubChannelList );

private:
    BOOL CCatalogSourceFilter::ReadTheFile(LPCTSTR lpszFileName);
    LPWSTR				 m_pFileName;			//load file name
    LONGLONG			 m_llSize;				//file size
	HANDLE				 m_hMonitor;			//monitor catalog thread handle
	HANDLE				 m_hEvtMonitor;			//whether read data with client or not.
	HANDLE				 m_hEvtStop;			//stop exchange thread.
	PBYTE				 m_pbData;				//file contents
	LPOLESTR			 m_pszCatalog;			//catalog name
	HANDLE				 m_hEvtSubChannelStateEvent;// subChannel state event handle

	vector<CCatalog > Catalogs;
	vector<DWS_SubChannelList > SubChannellists;
	vector<DWS_SubChannelList > TempSubChannellists;
	//SubChannelVector SubChannellists;
	//SubChannelVector TempSubChannellists;
protected:
	BOOL				 m_bDetectedFlag;		//detected flag
	long				 m_lDetectedInterval;	//detected interval
	HANDLE				 m_catalogStateEvent;	//filter state event
	int					 m_nErrorCode;			//error code
	int					 m_currentStatus;		//current status
	CRITICAL_SECTION	 m_lock;				//lock
	int					 m_iSetCatalogTime;		//Set catalog times
// ------------------------------------------------------ Modified by zhenan_ji at 2005年12月28日 13:38:53
// Add this for prevent search file was called at the same time ,in thread or update.
	int m_nCanUpdateCatalog;
};


/////////////////////////////////////////////////////////////////////////////////////////
//class CBMLConfigPage

class CCatalogConfigPage : public CBasePropertyPage
{
public:
	static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpUnknown,HRESULT *pHr);

public:
	CCatalogConfigPage(LPUNKNOWN lpUnknown,HRESULT *pHr);
	~CCatalogConfigPage();

public:
	virtual HRESULT OnConnect(IUnknown *pUnknown);
	virtual HRESULT OnDisconnect();
	virtual HRESULT OnActivate();
	virtual HRESULT OnDeactivate();
	virtual HRESULT OnApplyChanges();
	virtual INT_PTR OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

public:
	HRESULT ReflectCatalog();
	HRESULT EnterCatalog();

protected:
	ICatalogConfigure* m_pCatalogConfig;
	HWND m_hWndCatalog;
	HWND m_hWndDetectedFlag;
};