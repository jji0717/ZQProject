//------------------------------------------------------------------------------
// File: PullPin.cpp
//
// Desc: DirectShow base classes - implements CPullPin class that pulls data
//       from IAsyncReader.
//
// Copyright (c) 1992-2002 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#include "stdafx.h"
#include "pullpin3.h"

void CPullPin::CatalogChang(SubChannelVector pSubChannel, LPVOID lParam)
{
	char    chLogMsg[MAX_PATH];

	if( !lParam )
	{
		wsprintf(chLogMsg,"When enter CPullPin::CatalogChang() - lParam is NULL. No workn");LogMyEvent(1,0,chLogMsg);
		return;
	}

	CPullPin * pPullPin = (CPullPin*) lParam;
	
	if (pPullPin ->m_IsActive == FALSE )
	{
		wsprintf(chLogMsg,"PID=%d:enter CPullPin::CatalogChang() -current staic is inActive,size=%d",pPullPin->m_lPID,pSubChannel.size());LogMyEvent(1,0,chLogMsg);
		return;
	}

	wsprintf(chLogMsg,"PID=%d:Enter - CatalogChang(): have file change notification",pPullPin->m_lPID);LogMyEvent(1,0,chLogMsg);

	pPullPin->m_isNewUpdate = TRUE;

	if (pPullPin->m_IsProcessing)
	{
		while ( pPullPin->m_IsProcessing)
		{
			if (pPullPin->m_IsStop)
			{
				wsprintf(chLogMsg,"PID=%d:CatalogChang - receiver stop_command",pPullPin->m_lPID);LogMyEvent(1,0,chLogMsg);
				return ;
			}
			Sleep(2);
		}
		Sleep(2);
	}
	wsprintf(chLogMsg,"PID=%d:Enter - PreprocessSubchannelList",pPullPin->m_lPID);LogMyEvent(1,0,chLogMsg);

	//if( !pPullPin->m_byMode )
	pPullPin->PreprocessSubchannelList( pSubChannel );

	pPullPin->m_byVersion++;
	if( pPullPin->m_byVersion > 31 )
		pPullPin->m_byVersion = 0;
}

CPullPin::CPullPin()
  : m_pReader(NULL),
    m_pAlloc(NULL),
	m_byVersion(0),
    m_State(TM_Exit),
	m_byMode(0)
{
	m_IsStop = FALSE;
	m_IsActive = FALSE;
	m_isNewUpdate = FALSE;
	m_IsProcessing = FALSE;
	hUpdateEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

CPullPin::~CPullPin()
{
    Disconnect();
	m_SubChannelList.clear();
	if( hUpdateEvent )
	{
		CloseHandle( hUpdateEvent );
	}
}

// returns S_OK if successfully connected to an IAsyncReader interface
// from this object
// Optional allocator should be proposed as a preferred allocator if
// necessary
HRESULT
CPullPin::Connect(IUnknown* pUnk, IMemAllocator* pAlloc, BOOL bSync)
{
	CAutoLock lock(&m_AccessLock);

	if (m_pReader) 
	{
		return VFW_E_ALREADY_CONNECTED;
	}

	HRESULT hr = pUnk->QueryInterface(IID_IAsyncReader, (void**)&m_pReader);
	if (FAILED(hr)) 
	{
		return(hr);
	}

	m_pCatalog = pUnk;
	//zhenan 20060817 discard it
	m_pCatalog->SetDetectedRoutine( &CatalogChang, this );

	hr = DecideAllocator(pAlloc, NULL);
	if (FAILED(hr)) 
	{
		Disconnect();
		return hr;
	}

	LONGLONG llTotal, llAvail;
	hr = m_pReader->Length(&llTotal, &llAvail);
	if (FAILED(hr)) 
	{
		Disconnect();
		return hr;
	}

    // convert from file position to reference time
    m_tDuration = llTotal * UNITS;	// UNITS: 10 ^ 7
    m_tStop = m_tDuration;
    m_tStart = 0;

    m_bSync = bSync;

    return S_OK;
}

// disconnect any connection made in Connect
HRESULT
CPullPin::Disconnect()
{
	CAutoLock lock(&m_AccessLock);

//	StopThread();

	if (m_pReader) 
	{
		m_pReader->Release();
		m_pReader = NULL;
	}

	if (m_pAlloc) 
	{
		m_pAlloc->Release();
		m_pAlloc = NULL;
	}
	//;
	//m_pCatalog = NULL;

	return S_OK;
}

// agree an allocator using RequestAllocator - optional
// props param specifies your requirements (non-zero fields).
// returns an error code if fail to match requirements.
// optional IMemAllocator interface is offered as a preferred allocator
// but no error occurs if it can't be met.
HRESULT
CPullPin::DecideAllocator(
						  IMemAllocator * pAlloc,
						  ALLOCATOR_PROPERTIES * pProps)
{
	ALLOCATOR_PROPERTIES *pRequest;
	ALLOCATOR_PROPERTIES Request;
	if (pProps == NULL) 
	{
		Request.cBuffers = 1; //3;
		Request.cbBuffer = OBJECTPAYLOADMAXLEN;		// a sample pass the data volume of one object-in-a-table per time
		Request.cbAlign = 0;
		Request.cbPrefix = 0;
		pRequest = &Request;
	} else 
	{
		pRequest = pProps;
	}
	HRESULT hr = m_pReader->RequestAllocator(
		pAlloc,
		pRequest,
		&m_pAlloc);
	return hr;
}

// start pulling data
HRESULT
CPullPin::Active(void)
{
    ASSERT(!ThreadExists());
    return StartThread();
}

// stop pulling data
HRESULT
CPullPin::Inactive(void)
{
	char chLogMsg[MAX_PATH];
	wsprintf(chLogMsg,"PID=%d:CPullPin:Inactive.", m_lPID);
	LogMyEvent(1,0,chLogMsg);
    StopThread();	
    return S_OK;
}
void CPullPin::SetStreamCount( int nCount,int nPID )
{
	m_nStreamCount = nCount; 
};


HRESULT
CPullPin::Seek(REFERENCE_TIME tStart, REFERENCE_TIME tStop)
{
	CAutoLock lock(&m_AccessLock);

	ThreadMsg AtStart = m_State;

	if (AtStart == TM_Start) 
	{
		BeginFlush();
		PauseThread();
		EndFlush();
	}

	m_tStart = tStart;
	m_tStop = tStop;

	char chLogMsg[MAX_PATH];
	wsprintf(chLogMsg,"PID=%d:CPullPin::Seek.", m_lPID);
	LogMyEvent(1,0,chLogMsg);


	//HRESULT hr = S_OK;
	//if (AtStart == TM_Start) 
	//{
	//	hr = StartThread();
	//}

	return S_OK;
}

HRESULT
CPullPin::Duration(REFERENCE_TIME* ptDuration)
{
    *ptDuration = m_tDuration;
    return S_OK;
}


HRESULT
CPullPin::StartThread()
{
    CAutoLock lock(&m_AccessLock);

	if (!m_pAlloc || !m_pReader)
	{
		return E_UNEXPECTED;
	}

	HRESULT hr;
	if (!ThreadExists()) 
	{

		// commit allocator
		hr = m_pAlloc->Commit();
		if (FAILED(hr))
		{
			return hr;
		}

		// start thread
		if (!Create()) 
		{
			return E_FAIL;
		}
	}
	m_IsActive = TRUE;
	//m_State = TM_Start;
	//hr = (HRESULT) CallWorker(m_State);
	return hr;
}

HRESULT
CPullPin::PauseThread()
{
    CAutoLock lock(&m_AccessLock);

    if (!ThreadExists()) {
	return E_UNEXPECTED;
    }

    // need to flush to ensure the thread is not blocked
    // in WaitForNext
    HRESULT hr = m_pReader->BeginFlush();
    if (FAILED(hr)) {
	return hr;
    }

    m_State = TM_Pause;
    hr = CallWorker(TM_Pause);

    m_pReader->EndFlush();
    return hr;
}

HRESULT
CPullPin::StopThread()
{
	char chLogMsg[MAX_PATH];
	wsprintf(chLogMsg,"PID=%d:CPullPin:StopThread.", m_lPID);
	LogMyEvent(1,0,chLogMsg);

	m_IsStop = TRUE;

	CAutoLock lock(&m_AccessLock);

	m_IsProcessing = FALSE;

	if (!ThreadExists()) 
	{
		return S_FALSE;
	}

	// need to flush to ensure the thread is not blocked
	// in WaitForNext
	HRESULT hr = m_pReader->BeginFlush();
	if (FAILED(hr)) 
	{
		return hr;
	}

	m_State = TM_Exit;
	hr = CallWorker(TM_Exit);

	m_pReader->EndFlush();

	// wait for thread to completely exit
	Close();

	// decommit allocator
	if (m_pAlloc) 
	{
		m_pAlloc->Decommit();
	}
	wsprintf(chLogMsg,"PID=%d:CPullPin:StopThread.end", m_lPID);
	LogMyEvent(1,0,chLogMsg);

	return S_OK;
}

HRESULT CPullPin::SetSubChannelList( SubChannelVector &pSubChannellist )
{
	return S_OK;
}

HRESULT CPullPin::SendFlag( BYTE byFlag )
{
	return S_OK;
}

DWORD
CPullPin::ThreadProc(void)
{
	TCHAR chLogMsg[MAX_PATH];

	while(1) 
	{
//		wsprintf(chLogMsg,"PID=%d:Enter - ThreadProc:circel.",m_lPID);LogMyEvent(1,0,chLogMsg);

		if (m_isNewUpdate)
		{
			m_IsProcessing = FALSE;
			wsprintf(chLogMsg,"PID=%d:ThreadProc new update",m_lPID);LogMyEvent(1,0,chLogMsg);
		}
		DWORD cmd;

		cmd = GetRequest();
		switch(cmd) 
		{
		case TM_Exit:
			Reply(S_OK);
			return 0;

		case TM_Pause:
			// we are paused already
			Reply(S_OK);
			break;

		case TM_Start:
			Reply(S_OK);

			if (m_isNewUpdate)
			{
				m_IsProcessing = FALSE;
				wsprintf(chLogMsg,"PID=%d:ThreadProc find new update",m_lPID);LogMyEvent(1,0,chLogMsg);
				break ;
			}

			// wait for the updating notification from the DataWatchSource.	
			wsprintf(chLogMsg,"PID=%d:Enter - WaitForSingleObject: Wait the file change notification.",m_lPID);LogMyEvent(1,0,chLogMsg);

			//while( WaitForSingleObject( hUpdateEvent, 10 ) != WAIT_OBJECT_0 ) // INFINITE  1000
			//{
			//	if( CheckRequest( &cmd ) )
			//		if( cmd == TM_Exit )
			//		{	
			//			m_pReader->BeginFlush();
			//			CleanupCancelled();
			//			m_pReader->EndFlush();

			//			cmd = GetRequest();
			//			Reply(S_OK);
			//			m_IsProcessing = FALSE;
			//			wsprintf(chLogMsg,"PID=%d:Enter - WaitForSingleObject:get update event.",m_lPID);LogMyEvent(1,0,chLogMsg);
			//			return 0; 
			//		}
			//		//continue;
			//}

			if (m_isNewUpdate)
			{
				m_IsProcessing = FALSE;
				wsprintf(chLogMsg,"PID=%d:ThreadProc GetUpdateEvent  find new update",m_lPID);LogMyEvent(1,0,chLogMsg);
				return 0;
			}
			wsprintf(chLogMsg,"PID=%d:Leave - WaitForSingleObject: a file change notification happen.",m_lPID);LogMyEvent(1,0,chLogMsg);
			///////////////////////////////////////////////
			m_iCurrSubChannel = -1;
			InitSortBuffer();			
			//Receive( NULL );
			glog(ISvcLog::L_DEBUG, "++++++++++ Start wrap (PID = %d) ++++++++++", m_lPID);
			SendFlag( 0 );	// start
			Reply(S_OK);
			wsprintf(chLogMsg,"PID=%d:File process begin.File Count: %d.",m_lPID,m_SubChannelList.size());LogMyEvent(1,0,chLogMsg);
		
			for( WORD i=0;i< m_SubChannelList.size(); i++ )
			{
				if (m_isNewUpdate)
				{
					m_IsProcessing = FALSE;
					wsprintf(chLogMsg,"PID=%d:ThreadProc :start: find new update",m_lPID);LogMyEvent(1,0,chLogMsg);
					break;
				}
				if(m_IsStop)
				{
					wsprintf(chLogMsg,"PID=%d:File process .File respond stop_command",m_lPID);LogMyEvent(1,0,chLogMsg);
					break;
				}
				Process();
			}
			if (m_isNewUpdate)
			{
				m_IsProcessing = FALSE;
				wsprintf(chLogMsg,"PID=%d:ThreadProc :process sendbuffer before: find new update",m_lPID);LogMyEvent(1,0,chLogMsg);
				break;
			}

			SortBufferAndDelive();
			SendFlag( 1 );	// end
			glog(ISvcLog::L_DEBUG, "++++++++++ Finish wrap (PID = %d) ++++++++++", m_lPID);
			//Receive( NULL );
			wsprintf(chLogMsg,"PID=%d:File process end,setting IsProcessing is false",m_lPID);LogMyEvent(1,0,chLogMsg);
			m_IsProcessing = FALSE;
			break;
		}

		// at this point, there should be no outstanding requests on the
		// upstream filter.
		// We should force begin/endflush to ensure that this is true.
		// !!!Note that we may currently be inside a BeginFlush/EndFlush pair
		// on another thread, but the premature EndFlush will do no harm now
		// that we are idle.
		m_pReader->BeginFlush();
		CleanupCancelled();
		m_pReader->EndFlush();
	}
}

HRESULT
CPullPin::QueueSample(
					  REFERENCE_TIME& tCurrent,
					  REFERENCE_TIME tAlignStop,
					  BOOL bDiscontinuity
					  )
{
	IMediaSample* pSample;

	HRESULT hr = m_pAlloc->GetBuffer(&pSample, NULL, NULL, 0);
	if (FAILED(hr)) 
	{
		return hr;
	}

	LONGLONG tStopThis = tCurrent + (pSample->GetSize() * UNITS);
	if (tStopThis > tAlignStop) 
	{
		tStopThis = tAlignStop;
	}
	pSample->SetTime(&tCurrent, &tStopThis);
	tCurrent = tStopThis;

	pSample->SetDiscontinuity(bDiscontinuity);

	hr = m_pReader->Request(
		pSample,
		0);
	if (FAILED(hr))
	{
		pSample->Release();

	CleanupCancelled();
	OnError(hr);
    }
    return hr;
}

HRESULT
CPullPin::CollectAndDeliver(
    REFERENCE_TIME tStart,
    REFERENCE_TIME tStop)
{
    IMediaSample* pSample = NULL;   // better be sure pSample is set
    DWORD_PTR dwUnused;
    HRESULT hr = m_pReader->WaitForNext(
			INFINITE,
			&pSample,
			&dwUnused);
    if (FAILED(hr)) {
	if (pSample) {
	    pSample->Release();
	}
    } else {
	hr = DeliverSample(pSample, tStart, tStop);
    }
    if (FAILED(hr)) {
	CleanupCancelled();
	OnError(hr);
    }
    return hr;

}

HRESULT
CPullPin::DeliverSample(
    IMediaSample* pSample,
    REFERENCE_TIME tStart,
    REFERENCE_TIME tStop
    )
{
    // fix up sample if past actual stop (for sector alignment)
    REFERENCE_TIME t1, t2;
    pSample->GetTime(&t1, &t2);
    if (t2 > tStop) {
	t2 = tStop;
    }

    // adjust times to be relative to (aligned) start time
    t1 -= tStart;
    t2 -= tStart;
    pSample->SetTime(&t1, &t2);


    HRESULT hr = Receive(pSample);
    pSample->Release();
	//TCHAR szMsg[128];
	//sprintf( szMsg, "PID=%d:Exit CPullPin::DeliverSample()",m_lPID);		LogMyEvent(1,0,szMsg);

    return hr;
}

void
CPullPin::Process(void)
{
	TCHAR szMsg[1024];
	m_iCurrSubChannel++;
	//////////assign the name of file that need to be pulled.///////////////////////////////
	if( m_SubChannelList.size() < 1 )
	{
		sprintf( szMsg, "PID=%d:m_SubChannelList.size(): %d < 1, return.",m_lPID, m_SubChannelList.size());		LogMyEvent(0,0,szMsg);
		return;
	}
	if( m_iCurrSubChannel >= (int)(m_SubChannelList.size()) )
	{
		sprintf( szMsg, "PID=%d:m_iCurrSubChannel >= m_SubChannelList.size():",m_lPID);		LogMyEvent(0,0,szMsg);
		return; 
	}

	if (m_isNewUpdate)
	{
		wsprintf(szMsg,"PID=%d:CPullPin :process find new update",m_lPID);LogMyEvent(1,0,szMsg);
		return;
	}

	USES_CONVERSION;
	WCHAR wFile[MAX_PATH];
	wcsncpy(wFile, T2W(m_SubChannelList[m_iCurrSubChannel].szFileName), NUMELMS(wFile)-1);
	wFile[MAX_PATH-1] = 0;

	//zhenan add 20060727
	if(m_pCatalog->SetFileName( wFile ) == S_FALSE)
	{
		sprintf( szMsg, "PID=%d------No.%d file: %s. set filename error",m_lPID, m_iCurrSubChannel, m_SubChannelList[m_iCurrSubChannel].szFileName );
		LogMyEvent(0,0,szMsg);
		return ;
	}

	if (m_isNewUpdate)
	{
		wsprintf(szMsg,"PID=%d:CPullPin :process_setfilenaem find new update",m_lPID);LogMyEvent(2,0,szMsg);
		return;
	}
	sprintf( szMsg, "PID=%d--No.%d file: %s.",m_lPID, m_iCurrSubChannel, m_SubChannelList[m_iCurrSubChannel].szFileName );
	LogMyEvent(2,0,szMsg);	

	// convert from file position to reference time
	m_tDuration = (m_SubChannelList[m_iCurrSubChannel].dwFileSize) * UNITS;	// UNITS: 10 ^ 7
	m_tStop = m_tDuration;
	m_tStart = 0;

// ------------------------------------------------------ Modified by zhenan_ji at 2005Äê3ÔÂ25ÈÕ 15:51:20

	//m_iCurrSubChannel++;
	///////////////////////////////////////////////

	// is there anything to do?
	if (m_tStop <= m_tStart)
	{
		EndOfStream();
		return;
	}

	BOOL bDiscontinuity = TRUE;

    // if there is more than one sample at the allocator,
    // then try to queue 2 at once in order to overlap.
    // -- get buffer count and required alignment
    ALLOCATOR_PROPERTIES Actual;
    HRESULT hr = m_pAlloc->GetProperties(&Actual);

    // align the start position downwards
    REFERENCE_TIME tStart = AlignDown(m_tStart / UNITS, Actual.cbAlign) * UNITS;
    REFERENCE_TIME tCurrent = tStart;

	REFERENCE_TIME tStop = m_tStop;
	if (tStop > m_tDuration) 
	{
		tStop = m_tDuration;
	}

    // align the stop position - may be past stop, but that
    // doesn't matter
    REFERENCE_TIME tAlignStop = AlignUp(tStop / UNITS, Actual.cbAlign) * UNITS;

	// add a level loop here, for check out per file in list.	-leon-

    DWORD dwRequest;

	if (!m_bSync)
	{

		//  Break out of the loop either if we get to the end or we're asked
		//  to do something else
		while (tCurrent < tAlignStop) 
		{
			if (m_isNewUpdate)
			{
				wsprintf(szMsg,"PID=%d:CPullPin :m_bSync is false find new update",m_lPID);LogMyEvent(1,0,szMsg);
				return;
			}
			// Break out without calling EndOfStream if we're asked to
			// do something different
			if (CheckRequest(&dwRequest))
			{
				return;
			}

			// queue a first sample
			if (Actual.cBuffers > 1) 
			{

				hr = QueueSample(tCurrent, tAlignStop, TRUE);
				bDiscontinuity = FALSE;

				if (FAILED(hr))
				{
					return;
				}
			}
			// loop queueing second and waiting for first..
			while (tCurrent < tAlignStop) 
			{

				if (m_isNewUpdate)
				{
					wsprintf(szMsg,"PID=%d:CPullPin :loop queueing second  find new update",m_lPID);LogMyEvent(1,0,szMsg);
					return;
				}

				hr = QueueSample(tCurrent, tAlignStop, bDiscontinuity);
				bDiscontinuity = FALSE;

				if (FAILED(hr)) 
				{
					return;
				}

				hr = CollectAndDeliver(tStart, tStop);
				if (S_OK != hr)
				{

					// stop if error, or if downstream filter said
					// to stop.
					return;
				}
			}

			if (Actual.cBuffers > 1) 
			{
				hr = CollectAndDeliver(tStart, tStop);
				if (FAILED(hr)) 
				{
					return;
				}
			}
		}
	} 
	else 
	{
		// sync version of above loop
		while (tCurrent < tAlignStop) 
		{

			if (m_isNewUpdate)
			{
				wsprintf(szMsg,"PID=%d:CPullPin :sync version of above loop  find new update",m_lPID);LogMyEvent(1,0,szMsg);
				return;
			}

			// Break out without calling EndOfStream if we're asked to
			// do something different
			if (CheckRequest(&dwRequest))
			{
				return;
			}

			IMediaSample* pSample;

			hr = m_pAlloc->GetBuffer(&pSample, NULL, NULL, 0);
			if (FAILED(hr)) 
			{
				OnError(hr);
				return;
			}

			LONGLONG tStopThis = tCurrent + (pSample->GetSize() * UNITS);
			if (tStopThis > tAlignStop)
			{
				tStopThis = tAlignStop;
			}

			pSample->SetTime(&tCurrent, &tStopThis);
			tCurrent = tStopThis;

			if (bDiscontinuity) 
			{
				pSample->SetDiscontinuity(TRUE);
				bDiscontinuity = FALSE;
			}

			hr = m_pReader->SyncReadAligned(pSample);

			if (FAILED(hr)) 
			{
				pSample->Release();
				OnError(hr);
				return;
			}

			hr = DeliverSample(pSample, tStart, tStop);
			if (hr != S_OK) 
			{
				if (FAILED(hr)) 
				{
					OnError(hr);
				}
				return;
			}
		}
	}

	//TCHAR szMsg[1024];
	//sprintf( szMsg, "File process end: No. %d filename: %s.", m_iCurrSubChannel, m_SubChannelList[m_iCurrSubChannel].szFileName );
	//DbgLog((LOG_TRACE, 0, szMsg));

	EndOfStream();
	sprintf( szMsg, "PID=%d Exit CPullPin::Process()",m_lPID);		LogMyEvent(1,0,szMsg);

}

// after a flush, cancelled i/o will be waiting for collection
// and release
void
CPullPin::CleanupCancelled(void)
{
    while (1) {
	IMediaSample * pSample;
	DWORD_PTR dwUnused;

	HRESULT hr = m_pReader->WaitForNext(
			    0,          // no wait
			    &pSample,
			    &dwUnused);
	if(pSample) {
	    pSample->Release();
	} else {
	    // no more samples
	    return;
	}
    }
}

// assign the initialize value to per item in the m_SubChannelList and arrange the items in the list again.
// @pSubChannel: file list.
void CPullPin::PreprocessSubchannelList( SubChannelVector pSubChannel )
{
	int   iLength;
	TCHAR szFileName[MAX_PATH];

	// mainly assign the table_id and table_id_extension for per item, generate objectkey based the two value.
	// meantime, calculate the table count based on the size of per file.
	//ObjectDescriptor objDescriptor;
	TCHAR szTmp[32];

	BYTE byTableID = 0x10;		// 0x10 ~ 0xFF
	WORD wTableIDExt = 0x0100;	// 0x0100 ~ 0xFF00
	DWORD dwCount = 0;
	TCHAR chLogMsg[1024];

	// mainly assign the table_id and table_id_extension for per item, generate objectkey based the two value.
	// meantime, calculate the table count based on the size of per file.
	//ObjectDescriptor objDescriptor;
	WORD wTmp;
	SubChannelVector::iterator itrSubChannel;
	
	wsprintf(chLogMsg,"PID=%d:PreprocessSubchannelList.fileListsize(): %d",m_lPID,pSubChannel.size());LogMyEvent(1,0,chLogMsg);

	int nIndex = 0;
	for( itrSubChannel = pSubChannel.begin(); 
		itrSubChannel != pSubChannel.end(); ++itrSubChannel )
	{
		nIndex ++;
		if(m_IsStop)
		{
			wsprintf(chLogMsg,"PID=%d:PreprocessSubchannelList respond stop_command",m_lPID);LogMyEvent(1,0,chLogMsg);
			return;
		}
		wsprintf(chLogMsg,"PID=%d:ProcessList(index %d)-File:%s",m_lPID,nIndex,(*itrSubChannel).szFileName );LogMyEvent(1,0,chLogMsg);
		if( (*itrSubChannel).dwFileSize < 1 )
		{
			wsprintf(chLogMsg,"PID=%d: FileSize < 1, ignore it.",m_lPID);LogMyEvent(1,0,chLogMsg);
			continue;
		}

		if( m_byMode & WRAP_MODE_MULTISTREAM )
		{
			_tcscpy( szFileName, (*itrSubChannel).szFileName );
			DiscardPrefixOfFilename( szFileName );
			DiscardFileExtName( szFileName );
			iLength = (int)_tcslen( szFileName );
			if( iLength < 2 )
			{
				wsprintf(chLogMsg,"PID=%d:WRAP_MODE_MULTISTREAM: FileName length < 2, ignore it.",m_lPID);LogMyEvent(1,0,chLogMsg);
				continue;
			}

			wTmp = szFileName[iLength-1]+szFileName[iLength-2];
			(*itrSubChannel).lChannelPID = m_lPID + (wTmp%m_nStreamCount);
		}

		//////////////////////////////////////////////////
		if( m_byMode & WRAP_MODE_TABLEIDFROMFILENAME )
		{
			_tcscpy( szFileName, (*itrSubChannel).szFileName );
			DiscardPrefixOfFilename( szFileName );
			DiscardFileExtName( szFileName );
			iLength = (int)_tcslen( szFileName );
			if( iLength <= 0 )
			{
				wsprintf(chLogMsg,"PID=%d:WRAP_MODE_MULTISTREAM: FileName length <= 0, ignore it..",m_lPID);LogMyEvent(1,0,chLogMsg);
				continue;
			}
			if( iLength < 6 )
			{			
				szTmp[0] = 0;
				for( int i=0; i<(6-iLength); i++ )
					szTmp[i] = '0';
				szTmp[i] = 0;
				_tcscat( szTmp, szFileName );

				_tcscpy( szFileName, szTmp );
			}
			iLength = (int)_tcslen( szFileName );


			DWORD wFilenameASSIC[6];

			///////////////////////////////////////////////////
			wFilenameASSIC[5] = wTmp = ConvertHexChar( szFileName[iLength-6] );
			//wsprintf(chLogMsg,"szFileName[iLength-6]: %d", wTmp);LogMyEvent(1,0,chLogMsg);

			byTableID = (BYTE)(wTmp*16);
			wFilenameASSIC[4] = wTmp = ConvertHexChar( szFileName[iLength-5] );
			//wsprintf(chLogMsg,"szFileName[iLength-5]: %d", wTmp);LogMyEvent(1,0,chLogMsg);

			byTableID += (BYTE)wTmp;
			byTableID = byTableID | 0x80;

			wFilenameASSIC[3] = wTmp = ConvertHexChar( szFileName[iLength-4] );
			//wsprintf(chLogMsg,"szFileName[iLength-4]: %d", wTmp);LogMyEvent(1,0,chLogMsg);

			wTableIDExt = wTmp*16*16*16;
			wFilenameASSIC[2] = wTmp = ConvertHexChar( szFileName[iLength-3] );
			//wsprintf(chLogMsg,"szFileName[iLength-3]: %d", wTmp);LogMyEvent(1,0,chLogMsg);

			wTableIDExt += wTmp*16*16;
			wFilenameASSIC[1] = wTmp = ConvertHexChar( szFileName[iLength-2] );
			//wsprintf(chLogMsg,"szFileName[iLength-2]: %d", wTmp);LogMyEvent(1,0,chLogMsg);

			wTableIDExt += wTmp*16;
			wFilenameASSIC[0] = wTmp = ConvertHexChar( szFileName[iLength-1] );
			//wsprintf(chLogMsg,"szFileName[iLength-1]: %d", wTmp);LogMyEvent(1,0,chLogMsg);

			wTableIDExt += wTmp;
			wsprintf(chLogMsg,"PID=%d: Filename to tableID[%d%d]_tableIDExt[%d%d%d%d]",m_lPID, wFilenameASSIC[5],wFilenameASSIC[4],wFilenameASSIC[3],wFilenameASSIC[2],wFilenameASSIC[1],wFilenameASSIC[0]);LogMyEvent(1,0,chLogMsg);

			//////////////////////////////////////////////////
		}

		(*itrSubChannel).byTableID = byTableID;
		(*itrSubChannel).wTableIDExtension = wTableIDExt;

		wsprintf(chLogMsg,"PID=%d:File Information: table_id: 0x%X	table_id_ext: 0x%X	filename: %s.", m_lPID,byTableID, wTableIDExt, (*itrSubChannel).szFileName ); LogMyEvent(1,0,chLogMsg);
		
		//(*itrSubChannel)->byObjectKeyLength = DEFAULTOBJECTKEYLENGTH;
		memset( (*itrSubChannel).sObjectKey, 0, MAXOBJECTKEYLENGTH );
		(*itrSubChannel).sObjectKey[1] = byTableID; // (0x) 0010 -> 0010  
		wTmp = htons( wTableIDExt );	// (0x) 0100 -> 0001  
		memcpy( (*itrSubChannel).sObjectKey+2, &wTmp, sizeof(wTmp) );


		(*itrSubChannel).wReserve = m_byVersion;
		(*itrSubChannel).byControlFlag = 1;			// ?
		(*itrSubChannel).byTableCount =  (*itrSubChannel).dwFileSize/OBJECTPAYLOADMAXLEN + (*itrSubChannel).dwFileSize%OBJECTPAYLOADMAXLEN?1:0;

		dwCount += (*itrSubChannel).dwRepeat;

		if( !( m_byMode & WRAP_MODE_TABLEIDFROMFILENAME ))
		{
			if( wTableIDExt >= 0xFF00 )
			{
				wTableIDExt = 0x0100;

				if( byTableID >= 0xFF )
				{
					byTableID = 0x10;
				}
				else
				{
					byTableID ++;
					if( byTableID == 0x80 )
					{
						byTableID ++;
					}
				}
			}
			else
			{
				wTableIDExt = wTableIDExt+0x0100;
			}
		}
	}
	//m_SubChannelList = pSubChannel;

	wsprintf(chLogMsg,"PID=%d:Set m_isNewUpdate is false ,m_IsProcessing is true", m_lPID); LogMyEvent(1,0,chLogMsg);
	m_isNewUpdate = FALSE;
	m_IsProcessing = TRUE;
	m_SubChannelList.clear();
	m_SubChannelList.resize( dwCount );


	wsprintf(chLogMsg,"PID=%d:Clear SubChannelList", m_lPID); LogMyEvent(1,0,chLogMsg);

	// get the file that has max repeat times in the list. sort
	//sort( pSubChannel.begin(), pSubChannel.end(), IsLessRepeat() );

	//for_each( pSubChannel.begin(), pSubChannel.end(), PushSubChannelToList );

	m_SubChannelList = pSubChannel;
	//for( itrSubChannel = pSubChannel.begin(); 
	//	itrSubChannel != pSubChannel.end(); ++itrSubChannel )
	//{

	//	if (m_isNewUpdate)
	//	{
	//		m_IsProcessing = FALSE;
	//		wsprintf(chLogMsg,"PID=%d:PreprocessSubchannelList pushSubchannel new update",m_lPID);LogMyEvent(1,0,chLogMsg);
	//		return;
	//	}
	//	if(m_IsStop)
	//	{
	//		wsprintf(chLogMsg,"PID=%d:PreprocessSubchannelList pushSubchannel respond stop_command",m_lPID);LogMyEvent(1,0,chLogMsg);
	//		return;
	//	}

	//	if( (*itrSubChannel).dwFileSize < 1 )
	//		continue;
	//	PushSubChannelToList( (*itrSubChannel) );
	//}
	wsprintf(chLogMsg,"PID=%d:GetTableCount about subchannel file", m_lPID); LogMyEvent(1,0,chLogMsg);

	SetSubChannelList( pSubChannel );

	// notify the pull thread to start processing.
	SetEvent(  hUpdateEvent );

	if (m_isNewUpdate)
	{
		m_IsProcessing = FALSE;
		wsprintf(chLogMsg,"PID=%d:SetEvent(  hUpdateEvent isNewUpdate",m_lPID);LogMyEvent(1,0,chLogMsg);
		return ;
	}
	wsprintf(chLogMsg,"PID=%d:SetEvent for processing.",m_lPID); LogMyEvent(1,0,chLogMsg);

	m_State = TM_Start;
	CallWorker(m_State);
}

char CPullPin::ConvertHexChar(char ch) 
{
	if((ch>='0')&&(ch<='9'))
		return ch-0x30;
	else if((ch>='A')&&(ch<='Z'))
		return ch-'A'+10;
	else if((ch>='a')&&(ch<='z'))
		return ch-'a'+10;
	else 
		return (-1);
}
// change the extension name of certain file to .dv
// e.g. "temp.dv.pd" -> "temp.dv"
bool CPullPin::DiscardFileExtName( LPTSTR lpszFileName )
{
	int iLen = (int)_tcslen( lpszFileName );

	//bool bHaveExt = false;
	int i=iLen-1;
	for( ; i>=0; i-- )
	{
		if( lpszFileName[i] == '.' )
		{
			lpszFileName[i] = 0;
			//bHaveExt = true;
			break;
		}
	}

	return true;
}


// discard prefix of filename, i.e. "\DVL\050025" => "050025"
bool CPullPin::DiscardPrefixOfFilename( LPTSTR lpszFileName )
{
	int iLen = (int)_tcslen( lpszFileName );

	TCHAR szTmpName[MAX_PATH] = { 0 };

	int i=iLen-2;
	for( ; i>=0; i-- )
	{
		if( lpszFileName[i] == '\\' )
		{
			_tcscpy( szTmpName, &lpszFileName[i+1] );
			_tcscpy( lpszFileName, szTmpName );
			break;
		}
	}

	return true;
}

// re-arrange the order of items in the subchannel list based on the repeat time.
void CPullPin::PushSubChannelToList( DWS_SubChannelList pSubChannel )
{
	// get the offset based on the count of empty node in the list.
	DWORD dwCount = 0;
	//count( m_SubChannelList.begin(), m_SubChannelList.end(), NULL, dwCount );
	char chLogMsg[MAX_PATH];
	// amount the empty node count.
	SubChannelVector::iterator itrSubChannel;
	for( itrSubChannel = m_SubChannelList.begin(); 
		itrSubChannel != m_SubChannelList.end(); ++itrSubChannel )
	{
		if (m_isNewUpdate)
		{
			m_IsProcessing = FALSE;
			wsprintf(chLogMsg,"PID=%d:PushSubChannelToList check tablecount new update",m_lPID);LogMyEvent(1,0,chLogMsg);
			return;
		}
		if( (*itrSubChannel).byTableCount == 0 )
			dwCount++;
	}

	DWORD dwOffset = dwCount/pSubChannel.dwRepeat;

	// put the file in the empty node of the list based on the offset.
	DWORD dwNum = 0;
	dwCount = dwOffset;
	for( itrSubChannel = m_SubChannelList.begin(); 
		itrSubChannel != m_SubChannelList.end(); ++itrSubChannel )
	{

		if (m_isNewUpdate)
		{
			m_IsProcessing = FALSE;
			wsprintf(chLogMsg,"PID=%d:PushSubChannelToList new update",m_lPID);LogMyEvent(1,0,chLogMsg);
			return;
		}

		if(m_IsStop)
		{
			wsprintf(chLogMsg,"PID=%d:PushSubChannelToList respond stop_command",m_lPID);LogMyEvent(1,0,chLogMsg);
			return;
		}

		if( (*itrSubChannel).byTableCount == 0 )
		{
			if( dwCount == dwOffset )
			{
				(*itrSubChannel) = pSubChannel;
				dwCount = 0;
				
				dwNum++;
				if( dwNum == pSubChannel.dwRepeat )
					break;
			}
			else
			{
				dwCount++;
			}
		}
	}	
}

