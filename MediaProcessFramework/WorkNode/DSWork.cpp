// DSWork.cpp: implementation of the DSWork class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DSWork.h"

#include <dshow.h>
#pragma comment(lib,"strmiids.lib")

#include "MPFLogHandler.h"
using namespace ZQ::common;

#include "TaskAcceptor.h"

MPF_WORKNODE_NAMESPACE_BEGIN

DSWork::DSWork(WorkFactory* factory, const char* TaskTypename/* = "DSWork" */, const char* sessionURL/* =NULL */)
:BaseWork(factory,TaskTypename,sessionURL),
m_pMediaSeeking(NULL),
m_pGraphBuilder(NULL),
m_pMediaEvent(NULL),
m_pMediaControl(NULL)
{
	m_evThreadExit=CreateEvent(NULL,FALSE,FALSE,NULL);
	CoInitialize(NULL);

	try
	{
		CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, 
			IID_IGraphBuilder, (void **)&m_pGraphBuilder);
		
		if(!m_pGraphBuilder)
		{
			MPFLog(MPFLogHandler::L_ERROR,"DSWork::DSWork() Create GraphBuilder Object Ptr Failed\n");
			return;
		}
		
		IGraphBuilder*pGraphBuilder=(IGraphBuilder*)m_pGraphBuilder;
		pGraphBuilder->QueryInterface(IID_IMediaEventEx,(void**)&m_pMediaEvent);
		
		if(m_pMediaEvent==NULL)
		{
			
			MPFLog(MPFLogHandler::L_ERROR,"DSWork::DSWork() Create MediaEventEx Object Failed\n");
			
		}
		
		pGraphBuilder->QueryInterface(IID_IMediaSeeking,(void**)&m_pMediaSeeking);
		if(m_pMediaSeeking==NULL)
		{
			MPFLog(MPFLogHandler::L_ERROR,"DSWork::DSWork() Create MediaSeeking Object Failed\n");
			
		}

		pGraphBuilder->QueryInterface(IID_IMediaControl,(void**)&m_pMediaControl);
		if(m_pMediaControl==NULL)
		{
			MPFLog(MPFLogHandler::L_ERROR,"DSWork::DSWork() Create MediaControl Object Failed\n");
			
		}
	}
	catch(...)
	{
		MPFLog(MPFLogHandler::L_ERROR,"DSWork::DSWork() Got exception!\n");
			
	}
}


DSWork::~DSWork()
{
	if(m_pMediaSeeking)
	{
		IMediaSeeking* pMediaSeeking=(IMediaSeeking*)m_pMediaSeeking;
		pMediaSeeking->Release();
		m_pMediaSeeking = NULL;
	}

	if(m_pMediaEvent)
	{
		IMediaEvent* pMediaEvent=(IMediaEvent*)m_pMediaEvent;
		pMediaEvent->Release();
		m_pMediaEvent = NULL;
	}

	if(m_pMediaControl)
	{
		IMediaControl* pMediaControl = (IMediaControl*)m_pMediaControl;
		pMediaControl->Release();
		m_pMediaControl = NULL;
	}

	IGraphBuilder* pGraphBuilder=(IGraphBuilder*)m_pGraphBuilder;
	if(pGraphBuilder)
	{
		pGraphBuilder->Release();
		pGraphBuilder = NULL;
	}
	
	CoUninitialize();
}


void DSWork::ReleaseGraphBuilder()
{
	IGraphBuilder*pGraphBuilder=(IGraphBuilder*)m_pGraphBuilder;
	if(pGraphBuilder)
	{
		pGraphBuilder->Release();
		pGraphBuilder=NULL;
	}
}


void* DSWork::GetGraphBuilder()
{
	return (void*)m_pGraphBuilder;
}


int DSWork::run()
{
	HANDLE  hEvent=NULL; 
	long    evCode, param1, param2;
	
	IMediaEventEx* pEvent = (IMediaEventEx *)m_pMediaEvent;
	IMediaControl* pControl = (IMediaControl*)m_pMediaControl;
	try
	{
		if(pEvent==NULL)
		{
			MPFLog(MPFLogHandler::L_ERROR,"DSWork::Run() IMediaEventEx * pEvent =NULL\n");
			return 0;
		}
		if(pControl==NULL)
		{
			MPFLog(MPFLogHandler::L_ERROR,"DSWork::Run() IMediaControl * pControl =NULL\n");
			return 0;
		}

		// start graph flow
		HRESULT hr = pControl->Run();
		if(FAILED(hr))
		{
			MPFLog(MPFLogHandler::L_ERROR,"DSWork::Run() pControl->Run() failed\n");
			return 0;
		}


		hr = pEvent->GetEventHandle((OAEVENT*)&hEvent);
		if(FAILED(hr) || hEvent==NULL)
		{
			MPFLog(MPFLogHandler::L_ERROR,"DSWork::Run() pEvent->GetEventHandle() failed\n");
			return 0;
		}
		
		while(1) 
		{
			//manual exit thread by set exit event
			if(WaitForSingleObject(m_evThreadExit,10)==WAIT_OBJECT_0)
			{
				MPFLog(MPFLogHandler::L_DEBUG,"DSWork::Run() Thread is closed by force\n");
				break;
			}
			
			if (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, DEFAULT_DS_WAITTIME))
			{ 
				while (SUCCEEDED(hr = pEvent->GetEvent(&evCode, &param1, &param2, 0)))
				{
					MPFLog(MPFLogHandler::L_DEBUG,"DSWork::Run() Media Event got\n");
					
					//call WorkTask virtual process
					OnNotifyEvent(evCode,param1,param2);
					

					// release parameters
					pEvent->FreeEventParams(evCode, param1, param2);
				}
			}
		}
		
		// stop graph flow
		hr = pControl->Stop();
		if(FAILED(hr))
		{
			MPFLog(MPFLogHandler::L_ERROR,"DSWork::Run() pControl->Stop() Failed\n");
			return 0;
		}
	}
	catch(...)
	{
		MPFLog(MPFLogHandler::L_ERROR,"DSWork::Run() Got exception\n");
	}
	return 0;
}

void DSWork::Stop()
{
	SetEvent(m_evThreadExit);
}

bool DSWork::OnGetProgress(ZQ::rpc::RpcValue& attrs)
{
	if(!m_pMediaSeeking)
		return FALSE;
	
	IMediaSeeking*pMediaSeeking=(IMediaSeeking*)m_pMediaSeeking;

	const GUID format= TIME_FORMAT_MEDIA_TIME;
	// by default, set time format to Reference time (100-nanosecond units).
	if(FAILED(pMediaSeeking->SetTimeFormat(&format)))
		return FALSE;
	

	LONGLONG duration =0, current =0;
	ZQ::rpc::RpcValue Begin, Curr, End;

	// get duration
	if (FAILED(pMediaSeeking->GetDuration(&duration)))
		return FALSE;
	
	// get current postion
	if (FAILED(pMediaSeeking->GetCurrentPosition(&current)))
		return FALSE;
	
	// fill the result
	char buff[22];
	
	Begin = RpcValue(0);

	buff[0]='\0';
	_i64toa(current, buff, 10);
	Curr = RpcValue(buff);

	buff[0]='\0';
	_i64toa(duration, buff, 10);
	End = RpcValue(buff);

	attrs[BEGIN_POS_KEY]	= Begin;
	attrs[CURRENT_POS_KEY]	= Curr;
	attrs[END_POS_KEY]		= End;
	
	return TRUE;
}



MPF_WORKNODE_NAMESPACE_END

