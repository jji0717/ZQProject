// TrickJob.cpp: implementation of the CTrickJob class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TrickJob.h"
#include "trickpub.h"
#include "common.h"
#include "FNString.h"
#include "VSTRMSDK\inc\TrickFilesLibrary.h"
//#include "..\MCPPublish\MCPPubComHeader.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMpegBufferPool::CMpegBufferPool() :
 m_bufferDescriptors(sizeof(SDataBuffer))
, m_bufferSize(MAX_DATA_BUF_SIZE)
{
	
}

CMpegBufferPool::~CMpegBufferPool()
{
	PVOID buf;
	
	m_ioMutex.enter();
	
	while(!m_ioBuffers.empty())
	{
		buf = m_ioBuffers.front();
		
		if (buf)
		{
			delete buf;
		}
		
		m_ioBuffers.pop();
	}
	
	m_ioMutex.leave();
}

SDataBuffer* CMpegBufferPool::Alloc()
{
	SDataBuffer *buf = 0;
	PVOID ioBuf = 0;
	
	try
	{
		buf = (SDataBuffer *)m_bufferDescriptors.Get();
		if (buf)
		{
			// get buffer from queue if have
			m_ioMutex.enter();
			if (!m_ioBuffers.empty())
			{
				ioBuf = m_ioBuffers.front();
				m_ioBuffers.pop();
			}
			m_ioMutex.leave();
			
			
			// buffer queue don't have
			if (ioBuf == 0)
			{
				//				ioBuf = VirtualAlloc(0, m_bufferSize, MEM_COMMIT,PAGE_READWRITE);
				ioBuf = new BYTE[m_bufferSize];
				if (!ioBuf)
				{
					//					glog(Log::L_ERROR, "CMpegBufferPool: cann't alloc memory: %d.", m_bufferSize);
				}
			}
			
			if (ioBuf)
			{
				buf->mpegBuffer.pointer = (PUCHAR)ioBuf;
				buf->mpegBuffer.length = m_bufferSize;
			}
			else
			{
				m_bufferDescriptors.Free(buf);
				buf = 0;
			}
		}
	}
	catch(...)
	{
		//noted by salien
		//glog(Log::L_ERROR, "CMpegBufferPool: unknown exception.");
	}
	
	return buf;
}

void CMpegBufferPool::Free(SDataBuffer *buf)
{ 
	m_ioMutex.enter();
	m_ioBuffers.push(buf->mpegBuffer.pointer);
	m_ioMutex.leave();
	
	buf->mpegBuffer.pointer = 0;
	m_bufferDescriptors.Free(buf); 
}

void CMpegBufferPool::SetBufferSize(DWORD size)
{
	m_bufferSize = size;
}

CTrickJob::CTrickJob()
{
	m_bJobRun=FALSE;

	m_nBufferSize=0;
	
	ZeroMemory(m_szPathName,MAX_FILENAME_SIZE);
	ZeroMemory(m_szMulticastAddress,MAX_IP_SIZE);
	m_uSourcePort=-1;
	m_uDestPort=-1;
	ZeroMemory(m_szSubscribeFilename,MAX_FILENAME_SIZE);
	ZeroMemory(m_szPublisherIP,MAX_IP_SIZE);
	ZeroMemory(m_szPublisherInterfaceIP,MAX_IP_SIZE);
}

CTrickJob::~CTrickJob()
{
	if(m_SubscriberIpList.size()>0)
		m_SubscriberIpList.clear();

	if(m_SubscriberInterIpList.size()>0)
		m_SubscriberInterIpList.clear();
}

//#define MCP_PUBLISH

BOOL CTrickJob::CreateJob()
{

	ImportThreadParams genParams;
	
	if(m_szPathName[0]==0)return FALSE;
	
	char sss[MAX_FILENAME_SIZE];
	ZeroMemory(sss,MAX_FILENAME_SIZE);
	WideCharToMultiByte(CP_ACP, NULL, m_szPathName, -1, sss, sizeof(sss), NULL, NULL);
	
	strcpy(genParams.targetFilename ,sss);
	
	_bufferPool.SetBufferSize(m_nBufferSize);
	
	genParams.bufferSize = m_nBufferSize;//NT_MPEG_BUFFER_LENGTH;
	
	genParams.outBufferPool	= &_bufferPool;
	genParams.srcBufferPool	= this;
	genParams.context			= (PVOID) this;
	genParams.maxMpegCodingErrors = 100;
	
	// first import is to get some info of the media file
	
	CTrickImportUser  *pTrickImport=new CTrickImportUser(&genParams, 0);
	
	if(pTrickImport==NULL)return FALSE;
	
	HRESULT hRet = pTrickImport->Initialize();
	
	if (hRet != CTrickImportUser::E_OK)
	{
		pTrickImport->UnInitialize();	
		delete pTrickImport;
		pTrickImport=NULL;
		return FALSE;
	}
	

#ifndef MCP_PUBLISH
	DWORD mpegBitrate		= pTrickImport->m_trickCharacteristics.bitRate;
	DWORD TransportBitrate = (DWORD)mpegBitrate * (1 + FTP_MAXSPEED_MPEGFILE_RATE); 
	DWORD mcpBitRate = (ULONG)(TransportBitrate * (1 + BitRateGreaterPecent));
	
	m_Session.SetTransportRate(mcpBitRate);
	
	if(m_szMulticastAddress[0]!=0)
		m_Session.SetMulticastAddress(m_szMulticastAddress);
	
	m_Session.SetVideoHSize(pTrickImport->m_trickCharacteristics.horizontalSize);
	m_Session.SetVideoVSize(pTrickImport->m_trickCharacteristics.verticalSize);
	m_Session.SetVideoBitRate(pTrickImport->m_trickCharacteristics.videoBitRate);
	
	if(m_uSourcePort>0)
		m_Session.SetSourcePort(m_uSourcePort);
	
	if(m_uDestPort>0)
		m_Session.SetDestport(m_uDestPort);
		
	m_Session.SetPublishFilename(m_szPathName);
	
	if(m_szSubscribeFilename[0]!=0)
		m_Session.SetSubscribeFilename(m_szSubscribeFilename);
	else
	{
		CFNString file(sss);
		string strSName=file.GetFileName();
		if(strSName.empty())
		{
			pTrickImport->UnInitialize();	
			delete pTrickImport;
			pTrickImport=NULL;
			return FALSE;
		}
		
		wchar_t wFileName[MAX_FILENAME_SIZE];
		ZeroMemory(wFileName,MAX_FILENAME_SIZE);
		MultiByteToWideChar(CP_ACP, 0, strSName.c_str(), -1, wFileName, sizeof(wFileName));
		
		m_Session.SetSubscribeFilename(wFileName);
	}
	
	if(m_szPublisherIP[0]!=0)
		m_Session.SetPublisherIP(m_szPublisherIP);
	
	if(m_szPublisherInterfaceIP[0]!=0)
		m_Session.SetPublisherInterfaceIP(m_szPublisherInterfaceIP);
	
	while(m_SubscriberIpList.size()!=0)
	{
		wstring ip1=m_SubscriberIpList.front();
		m_SubscriberIpList.pop_front();
		
		wstring ip2=m_SubscriberInterIpList.front();
		m_SubscriberInterIpList.pop_front();
		
		if(FAILED(m_Session.AddSubscriber((BSTR)ip1.c_str(),(BSTR)ip2.c_str())))
		{
			pTrickImport->UnInitialize();	
			delete pTrickImport;
			pTrickImport=NULL;
			return FALSE;
		}
	}
	
	long nRet;
	nRet = m_Session.Start();
	
	// if fail
	if(nRet!=1)
	{
		char szbuf[100];
		sprintf(szbuf,"In TrickPublisher.dll CreateJob, Start() Error Occur  error and number=%d",nRet);
		OutputDebugString(szbuf);
		
		pTrickImport->UnInitialize();
		delete pTrickImport;
		pTrickImport=NULL;

		return FALSE;
	}
	
#endif
	
	//Set job run flag
	m_bJobRun=TRUE;

	hRet = pTrickImport->Import(); // will block
	pTrickImport->UnInitialize();	
	
#ifndef MCP_PUBLISH
	// if mcp fail
	if(m_Session.IsDone())
	{
		//log
		OutputDebugString("In TrickPublisher.dll CreateJob, Publish Failed\n");
		
		if(pTrickImport)
		{
			delete pTrickImport;
			pTrickImport=NULL;
		}
		
		DeleteTrickFile();
		
		return FALSE;
	}
	
#endif
	
	if (FAILED(hRet))
	{		
		//log
		OutputDebugString("In TrickPublisher.dll CreateJob, Publish Failed\n");
		
#ifndef MCP_PUBLISH
		
		if(pTrickImport)
		{
			delete pTrickImport;
			pTrickImport=NULL;
		}
		
		m_Session.Stop();
			
		m_Session.WaitForFinish(1000);
	
#endif
		
		DeleteTrickFile();
		return FALSE;
	}
	
	int n=sizeof(*pTrickImport);

	if(pTrickImport)
	{
		delete pTrickImport;
		pTrickImport=NULL;
	}
	
#ifndef MCP_PUBLISH
	// timeout or finish
	
	if(FAILED(m_Session.WaitForFinish(1000)))
	{
		DeleteTrickFile();
		return FALSE;
	}
	
	/*	if (SUCCEEDED(m_pMcpPublish->IsDone()))
	{
	//log and message to filter graph
	}
	*/
	
#endif
	
	return TRUE;
}

SDataBuffer *CTrickJob::Alloc()
{
	return m_SBM.GetData();
}

void CTrickJob::Free(SDataBuffer *buf)
{
	if(buf)
		m_SBM.FreeBuffer(buf);
}

void CTrickJob::SetFilePathName(wchar_t *pPath)
{
	if(pPath==NULL || *pPath==0)
		return;
	
	ZeroMemory(m_szPathName,MAX_FILENAME_SIZE);
	memcpy(m_szPathName,pPath,MAX_FILENAME_SIZE);
}

void CTrickJob::SetMaxMpegCodingErrors(int nCode)
{
	m_nCodingError=nCode;
}

void CTrickJob::SetMulticastAddress(BSTR strAddr)
{
	ZeroMemory(m_szMulticastAddress,MAX_IP_SIZE);
	wcscpy(m_szMulticastAddress,strAddr);
}

void CTrickJob::SetSourcePort(UINT uPort)
{
	m_uSourcePort=uPort;
}

void CTrickJob::SetDestport(UINT uPort)
{
	m_uDestPort=uPort;
}


void CTrickJob::SetSubscribeFilename(BSTR strFileName)
{
	ZeroMemory(m_szSubscribeFilename,MAX_FILENAME_SIZE);
	wcscpy(m_szSubscribeFilename,strFileName);
}

void CTrickJob::SetPublisherIP(BSTR strAddr)
{
	ZeroMemory(m_szPublisherIP,MAX_IP_SIZE);
	wcscpy(m_szPublisherIP,strAddr);
}

void CTrickJob::SetPublisherInterfaceIP(BSTR strAddr)
{
	ZeroMemory(m_szPublisherInterfaceIP,MAX_IP_SIZE);
	wcscpy(m_szPublisherInterfaceIP,strAddr);
}

HRESULT CTrickJob::AddSubscriber(BSTR ip1,BSTR ip2)
{
	if(ip1==NULL || *ip1==0)return S_FALSE;
	
	m_SubscriberIpList.push_back(ip1);
	m_SubscriberInterIpList.push_back(ip1);
	return S_OK;
}

UINT ThreadProc(LPVOID lpParam)
{
	CTrickJob* pJob=(CTrickJob*)lpParam;
	
	if(pJob==NULL)return 0;

	pJob->CreateJob();
/*	SDataBuffer *buf=pJob->Alloc();
	if(buf==NULL)return 0;
	pJob->SetJobRunFlag();
	while(buf!=NULL)
	{
		pJob->Free(buf);
		buf=pJob->Alloc();
*/
	return 0;
}


void CTrickJob::SetBufferSize(int nLen)
{
	m_SBM.InitBufferQue(nLen);
	
	m_nBufferSize=nLen;
}

void CTrickJob::SetData(LPVOID lpBuf,int nDataLen)
{
	m_SBM.SetData(lpBuf,nDataLen);
}

int CTrickJob::GetJobCount()
{
	return  m_SBM.GetSize();
}


void CTrickJob::DeleteTrickFile()
{
	if(m_szPathName[0]!=0)
	{
		char sss[MAX_FILENAME_SIZE];
		ZeroMemory(sss,MAX_FILENAME_SIZE);
		WideCharToMultiByte(CP_ACP, NULL, m_szPathName, -1, sss, sizeof(sss), NULL, NULL);
		
		char temp[MAX_FILENAME_SIZE];
		ZeroMemory(temp,MAX_FILENAME_SIZE);
		
		//delete file
		if(!DeleteFile(sss))
		{
			//log 
			OutputDebugString("In TrickPublisher.dll DeleteTrickFile,Not Find the File to be Deleted !\n");
		}
		
		strcpy(temp,sss);
		strcat(temp,".ff");
		if(!DeleteFile(temp))
		{
			//log 
			OutputDebugString("In TrickPublisher.dll DeleteTrickFile,Not Find the File to be Deleted !\n");
		}
		
		ZeroMemory(temp,MAX_FILENAME_SIZE);
		strcpy(temp,sss);
		strcat(temp,".fr");
		if(!DeleteFile(temp))
		{
			//log 
			OutputDebugString("In TrickPublisher.dll DeleteTrickFile,Not Find the File to be Deleted !\n");
		}
		
		ZeroMemory(temp,MAX_FILENAME_SIZE);
		strcpy(temp,sss);
		strcat(temp,".vvx");
		if(!DeleteFile(temp))
		{
			//log 
			OutputDebugString("In TrickPublisher.dll DeleteTrickFile,Not Find the File to be Deleted !\n");
		}
	}
}

void CTrickJob::StopJob()
{
	//stop
	m_SBM.SetData(NULL,0);
	//m_Session.Stop();
}

BOOL CTrickJob::IsRunning()
{
	return m_bJobRun;
}

void CTrickJob::SetJobStopFlag()
{
	m_bJobRun=FALSE;
}



void CTrickJob::SetJobRunFlag()
{
	m_bJobRun=TRUE;
}
