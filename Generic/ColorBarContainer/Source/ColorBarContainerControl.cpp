// ColorBarContainerControl.cpp : Implementation of CColorBarContainerControl

#include "stdafx.h"
#include "CBarContainer.h"
#include "ColorBarContainerControl.h"

typedef ::IceInternal::Handle< ::TianShanIce::Streamer::StreamEventSink> StreamEventSinkPtr;
typedef ::IceInternal::Handle< ::TianShanIce::Streamer::StreamProgressSink> StreamProgressSinkPtr;
typedef ::IceInternal::Handle< ::TianShanIce::Streamer::PlaylistEventSink> PlaylistEventSinkPtr;

/////////////////////////////////////////////////////////////////////////////
// CColorBarContainerControl

CColorBarContainerControl::CColorBarContainerControl()
{
	m_bWindowOnly          = TRUE;
	m_pContainWin          = NULL;
	m_Communicator         = NULL;
//	m_ChanelAdapter        = NULL;
	m_StreamAdapter        = NULL;
	m_eventChannel         = NULL;
	m_publisherExPrx       = NULL;
	m_channelPrx           = NULL;
	m_pColorBarInterface   = NULL;
	m_pContainControlWidth = NULL;

	m_tsSubscribeEndpoint_Stream    = DEF_SUBSCRIBER_ENDPOINT_STREAM;
	m_tsTopicManagerEndpoint_Stream = DEF_TOPIC_MANAGER_ENDPOINT_STREAM;
	
	m_tsSubscribeEndpoint_Chanel    = DEF_SUBSCRIBER_ENDPOINT_CHANEL;
	m_tsTopicManagerEndpoint_Chanel = DEF_TOPIC_MANAGER_ENDPOINT_CHANEL;
	m_CursorColor  = BEGIN_USER_COLOR;
	m_ChanelMap.clear();
	m_ContainerMap.clear();
	m_UserCursorDataMap.clear();
	
	m_WindowNum = 0;
	m_dStartPos = 0.0;
	m_dEndPos   = 0.0;
}

CColorBarContainerControl::~CColorBarContainerControl()
{
	if ( !m_ChanelMap.empty())
	{
		m_ChanelMap.clear();
	}
	if (!m_ContainerMap.empty())
	{
		m_ContainerMap.clear();
	}
	if (!m_UserCursorDataMap.empty())
	{
		m_UserCursorDataMap.clear();
	}
	if ( m_pContainWin )
	{
		for ( size_t i =0; i < m_WindowNum; i ++ )
		{
		    if ( m_pContainWin[i].IsWindow())
			{
				m_pContainWin[i].DestroyWindow();
				
			}
		}
		delete [] m_pContainWin;
		m_pContainWin = NULL;
	}
	if ( m_pColorBarInterface)
	{
		for ( size_t i =0; i < m_WindowNum; i ++ )
		{
			if ( m_pColorBarInterface[i])
			{
				m_pColorBarInterface[i].Release();
			}
		}
		delete [] m_pColorBarInterface;
		m_pColorBarInterface = NULL;
	}
	OnChanelUnInit();
	OnStreamEventUnInit();
}

LRESULT CColorBarContainerControl::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	bHandled = FALSE;
	return S_OK;
}

LRESULT CColorBarContainerControl::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	RECT rc;
	GetClientRect(&rc);
	
	size_t size ;
	size_t i;
	
	RECT rcTemp;
	int iWdith;
	
	size = m_ChanelMap.size();
	i = 0;
	if ( size > 0 )
	{
		iWdith = ( ( rc.bottom - rc.top ) - ( size + 1 )* 8 ) / size ;
			
		for ( i = 0; i < size; i ++ )
		{
			rcTemp.left   = rc.left  + 10;
			rcTemp.right  = rc.right - 10;
			rcTemp.top    = rc.top   +  i * iWdith + i*8  ; // 每个Bar之间的空隔是8pixs
			rcTemp.bottom = rcTemp.top + iWdith  ;
			
			m_pContainWin[i].SetWindowPos(HWND_TOP,rcTemp.left,rcTemp.top,rcTemp.right-rcTemp.left,iWdith,SWP_SHOWWINDOW|SWP_NOZORDER);
		}
	}
	return S_OK;
}



// Create the Contain Window
HRESULT CColorBarContainerControl::CreateContainWinow()
{
	try
	{

		AtlAxWinInit(); 
		size_t size ;
		size_t i;
		size_t j;
		
		RECT rc;
		GetClientRect(&rc);
		RECT rcTemp;
		int iWdith;
		
		HWND handle;
		MAPITERTMP  itorTmp;
		DATAVECTOR  VectorTmp;
		
		CComPtr<IUnknown>   pContainer ;
		CComPtr<IUnknown>   pControl ;
		CComPtr<IOleObject> spObj;
		CComPtr<IColorBarControl> pColorBarInterface;
		SIZEL sz;
		COLORREF crColor;
		double dStartPos = 0.0;
		double dEndPos = 0.0;
		double dShowStartPos = 0.0;
		double dShowEndPos = 0.0;

		CComBSTR bstrName;

		CComBSTR bstr(L"ColorBar.ColorBarControl");

		CLSID   theclsid;
		HRESULT hr = ::CLSIDFromProgID(bstr.m_str,&theclsid);
		USES_CONVERSION;
		
		LPOLESTR lpolestr;
		StringFromCLSID(theclsid,&lpolestr);

		size = m_ChanelMap.size();
		m_pContainControlWidth = new double[size];
		i = 0;
		j = 0;
		if ( size > 0 )
		{
			iWdith = ( ( rc.bottom - rc.top ) - ( size + 1 )*8 ) / size ;
			m_WindowNum = size;
			m_pContainWin = new CIBMSBaseWnd[size];
			m_pColorBarInterface = new CComPtr<IColorBarControl>[size];
			
		
			for ( itorTmp = m_ChanelMap.begin(); itorTmp != m_ChanelMap.end(); itorTmp ++ )
			{
				rcTemp.left   = rc.left  + 10;
				rcTemp.right  = rc.right - 10;
				rcTemp.top    = rc.top   +  i * iWdith + i*8  ; // 每个Bar之间的空隔是8pixs
				rcTemp.bottom = rcTemp.top + iWdith  ;

				handle = m_pContainWin[i].Create(this->m_hWnd,rcTemp, _T(""),
												 WS_CHILD | WS_VISIBLE | 
												 WS_CLIPSIBLINGS | 
												 WS_CLIPCHILDREN |
												 WS_TABSTOP,
       											 0);

				
				if ( !handle)
				{
					
     			}
				if ( i > 0 )
				{
					pContainer.Release();
					pControl.Release();
					spObj.Release();
					pColorBarInterface.Release();
				}
				m_pContainWin[i].IBMSCreateControlEx2(lpolestr,m_pContainWin[i].m_hWnd,NULL,&pContainer,&pControl);
				
				if(SUCCEEDED( m_pContainWin[i].QueryControl(&spObj) ))
				{
					spObj->GetExtent(DVASPECT_CONTENT,&sz);    
					AtlHiMetricToPixel(&sz,&sz);
					m_pContainWin[i].SetWindowPos(0,rcTemp.left,rcTemp.top,sz.cx,sz.cy,SWP_NOZORDER);
					spObj->SetExtent(DVASPECT_CONTENT,&sz);
				}
				m_pContainWin[i].SetWindowPos(HWND_TOP,rcTemp.left,rcTemp.top,rcTemp.right-rcTemp.left,iWdith,SWP_SHOWWINDOW|SWP_NOZORDER);

				VectorTmp = (*itorTmp).second;
				size = VectorTmp.size();
							
				pControl->QueryInterface(&m_pColorBarInterface[i]);
				if ( size == 0 ) // 如果没有子节目
				{
					crColor       = VectorTmp[0].crColor;
					dStartPos     = VectorTmp[0].dStartPos;
					dShowStartPos = dStartPos;
					dEndPos       = VectorTmp[0].dEndPos;
					dShowEndPos   = dEndPos;
					bstrName      = VectorTmp[0].bstrFilmName;
					
					m_pColorBarInterface[i]->FillColor(crColor,dStartPos,dEndPos,bstrName);
				}
				else //有子节目
				{
					// 遍历全部子节目
					for ( j = 0 ; j < size ; j ++ )
					{
						crColor    = VectorTmp[j].crColor;
						dStartPos  = VectorTmp[j].dStartPos;
						dEndPos    = VectorTmp[j].dEndPos;
						bstrName   = VectorTmp[j].bstrFilmName;
						m_pColorBarInterface[i]->FillColor(crColor,dStartPos,dEndPos,bstrName);
					}
					dShowStartPos = VectorTmp[0].dStartPos;
					dShowEndPos   = VectorTmp[size-1].dEndPos;
				}
				m_pColorBarInterface[i]->ShowRange(dShowStartPos,dShowEndPos);
				m_pContainControlWidth[i]=dShowEndPos - dShowStartPos;
				i ++;
			}
		}
		CoTaskMemFree(lpolestr);
	}
	catch(...)
	{
		return E_HANDLE;
	}
	return S_OK;
}

bool CColorBarContainerControl::IsExistByStreamId(string strStreamId)
{
	bool bExist = false;
	if ( !m_ContainerMap.empty())
	{
		CComBSTR bstrStreamId;
		bstrStreamId = strStreamId.c_str();

		CONTAINERMAPITER  containItor;
		containItor    = m_ContainerMap.find(bstrStreamId);
		if ( containItor == m_ContainerMap.end() )
		{
			bExist = false;
		}
		else
		{
			bExist = true;
		}
	}
	return bExist;
}

// create the contain map by the StreamId
HRESULT  CColorBarContainerControl::CreateContainMap(string  strStreamId) 
{
	try
	{
		if ((m_publisherExPrx)  && ( !strStreamId.empty()))
		{
			string strChanelIdName;
			CComBSTR bstrChanelIdName;
			CComBSTR bstrStreamId;
			MAPITERTMP   itorTmp;
			CONTAINDATA  ContainDataTmp;

			memset((void*)&ContainDataTmp,0,sizeof(ContainDataTmp));
			ContainDataTmp.pContainControl = NULL;
			
			int i =0;
			strChanelIdName  = m_publisherExPrx->getChannelByStream(strStreamId); // the code is must
			
			// for the test,正式时需要删掉以下代码
			if ( strChanelIdName.empty() )
			{
				strChanelIdName  = "CCTV1";
			}
			// for the test 

			bstrChanelIdName = strChanelIdName.c_str();
			bstrStreamId     = strStreamId.c_str();

			// for the test 11
			for ( itorTmp = m_ChanelMap.begin(); itorTmp != m_ChanelMap.end(); itorTmp ++ )
			{			
				if ((*itorTmp).first == bstrChanelIdName ) // find the chanelId
				{
					ContainDataTmp.dChanelWidth     = m_pContainControlWidth[i]; // 节目的总长度
					ContainDataTmp.pContainControl  = m_pColorBarInterface[i].p;
					ContainDataTmp.bstrChanelId     = strChanelIdName.c_str() ;
					m_ContainerMap[bstrStreamId]    = ContainDataTmp;
					break;
				}
				i++;
			}
		}
		else
		{
			return S_FALSE;
		}
	}
	catch(...)
	{
		return E_HANDLE;
	}
	return S_OK;
}

// create cursor by streamId
HRESULT CColorBarContainerControl::CreateUserCursorMap(string strStreamId)// create the user cursor map by the StreamId
{
	HRESULT hr = S_FALSE;
	try
	{
		if( !strStreamId.empty())
		{
			if ( !m_ContainerMap.empty())
			{
				string strChanelId;
				CComBSTR bstrChanelId;
				CComBSTR bstrStreamId;


				bstrStreamId = strStreamId.c_str();
					

				//根据SteamId 查找到ChanceId,并根据该ChanceId找到组件的指针
				CONTAINDATA       containDataTmp;
				memset((void*)&containDataTmp,0,sizeof(containDataTmp));

				CONTAINERMAPITER  containItor;
				containItor    = m_ContainerMap.find(bstrStreamId);
				
				containDataTmp = (*containItor).second;
				bstrChanelId   = containDataTmp.bstrChanelId; //节目频道名称

	//			USES_CONVERSION;
	//			strChanelId    = OLE2T(bstrChanelId.m_str());

				//创建用户的CursorID号值，并保存该到Map列表中。
				CURSORVECTOR  cursorVectTmp;
				cursorVectTmp.clear();

				CURSORVECTORTYPE cursorDataTmp;
				memset((void*)&cursorDataTmp,0,sizeof(cursorDataTmp));
									
				int iCursorId;
				iCursorId = 0;
				hr = containDataTmp.pContainControl->CreateCursor(m_CursorColor,&iCursorId);

				if ( iCursorId < 151 )
				{
					cursorDataTmp.iCurSorID    = iCursorId;
					cursorDataTmp.bstrStreamId = bstrStreamId;
					cursorVectTmp.push_back(cursorDataTmp);
					m_UserCursorDataMap[bstrChanelId] = cursorVectTmp;
					m_CursorColor += 500; // 颜色递增500,先暂时这么定
				}
			}
		}	
	}
	catch(...)
	{
		return E_HANDLE;
	}
	return hr;
}

HRESULT  CColorBarContainerControl::UserDrawLine(string strStreamId,double dRatio,string strComment, int iCursorId)//draw the line by the params 
{
	HRESULT hr = S_FALSE;
	try
	{
		if( !strStreamId.empty())
		{
				
			//根据SteamId 查找到ChanceId,并根据该ChanceId找到组件的指针
			CONTAINDATA       containDataTmp;
			memset((void*)&containDataTmp,0,sizeof(containDataTmp));
			string   strChanelId;
			int iCursorId;
			iCursorId = -1;

			if ( !m_ContainerMap.empty() )
			{
				CONTAINERMAPITER  containItor;
				CComBSTR bstrChanelId;
				CComBSTR bstrStreamId;

				bstrStreamId = strStreamId.c_str();
				
				containItor    = m_ContainerMap.find(bstrStreamId);
				
				containDataTmp = (*containItor).second;
				bstrChanelId   = containDataTmp.bstrChanelId; // 节目的名称
	//			strChanelId    = bstrChanelId;

				//获取CursorID值
				USERDRAWLINEMAPITER  usermapitorTmp;
				CURSORVECTOR         uservectitorTmp;
				uservectitorTmp.clear();

				usermapitorTmp = m_UserCursorDataMap.find(bstrChanelId);

				uservectitorTmp = (*usermapitorTmp).second;
				size_t size,i;
				size = uservectitorTmp.size();
				if ( size > 0 )
				{
					for ( i = 0 ; i < size ; i ++ )
					{
						if ( uservectitorTmp[i].bstrStreamId  == bstrStreamId )
						{
							iCursorId = uservectitorTmp[i].iCurSorID ;
							break;
						}
					}
				}
				
				// 获取长度值
				string strStreamItem;
				int ipos;
				ipos = strComment.find_last_of("\\");
				strStreamItem = MidString(strComment,ipos+1,strComment.length());
				GetRegionTimeData(strStreamId,strStreamItem);
				
				if  ( iCursorId != -1 ) 
				{
					double dPos;
					dPos = m_dStartPos + ( dRatio * (m_dEndPos- m_dStartPos) );
					hr =containDataTmp.pContainControl->DrawLine(iCursorId,dPos);
				}
			}
		}
	}
	catch(...)
	{
		return E_HANDLE;
	}
	return hr;
}


HRESULT  CColorBarContainerControl::CreateCursorAndDrawLine(string strStreamId,string strComment,double dRatio)
{
	HRESULT hr = S_FALSE;
	try
	{
		if( !strStreamId.empty())
		{
			if ( !m_ContainerMap.empty())
			{
				string strChanelId;
				string strStreamItem; //OnProgress参数中最后的Item名称值

				CComBSTR bstrChanelId;
				CComBSTR bstrStreamId;
				CComBSTR bstrStreamItem;

				int ipos;
				ipos = strComment.find_last_of("\\");
				
				strStreamItem  = MidString(strComment,ipos+1,strComment.length());
				bstrStreamItem = strStreamItem.c_str();
				bstrStreamId = strStreamId.c_str();
					
				//根据SteamId 查找到ChanceId,并根据该ChanceId找到组件的指针
				CONTAINDATA       containDataTmp;
				memset((void*)&containDataTmp,0,sizeof(containDataTmp));

				CONTAINERMAPITER  containItor;
				containItor    = m_ContainerMap.find(bstrStreamId);
				containDataTmp = (*containItor).second;
				bstrChanelId   = containDataTmp.bstrChanelId; //节目频道名称
				size_t size,i;


				

				CURSORVECTOR  cursorVectTmp;
				cursorVectTmp.clear();

				CURSORVECTORTYPE cursorDataTmp;
				memset((void*)&cursorDataTmp,0,sizeof(cursorDataTmp));

				int iCursorId;
				double dPos;

				iCursorId = -1;
				if ( m_UserCursorDataMap.empty()) // 还没有创建CursorMap
				{
					//创建用户的CursorID号值，并保存该到Map列表中。
					hr = containDataTmp.pContainControl->CreateCursor(m_CursorColor,&iCursorId);
					if ( iCursorId < 151 )
					{
						cursorDataTmp.iCurSorID    = iCursorId;
						cursorDataTmp.bstrStreamId = bstrStreamItem; //保存StreamItem值
						cursorVectTmp.push_back(cursorDataTmp);
						m_UserCursorDataMap[bstrChanelId] = cursorVectTmp;
						m_CursorColor += 500; // 颜色递增500,先暂时这么定

						GetRegionTimeData(strStreamId,strStreamItem);
						if  ( iCursorId != -1 ) 
						{
						
							dPos = m_dStartPos + ( dRatio * (m_dEndPos- m_dStartPos) );
							hr   = containDataTmp.pContainControl->DrawLine(iCursorId,dPos);
						}
					}
					else
					{
						hr = E_HANDLE;
					}
				}
				else // 如果map表中已经存在数据
				{
					//获取CursorID值
					USERDRAWLINEMAPITER  usermapitorTmp;
					CURSORVECTOR         uservectitorTmp;
					uservectitorTmp.clear();

					usermapitorTmp = m_UserCursorDataMap.find(bstrChanelId);
					uservectitorTmp = (*usermapitorTmp).second;
					size = uservectitorTmp.size();

					bool bIsExist;
					bIsExist = false;
					if ( size > 0 )
					{
						for ( i = 0 ; i < size ; i ++ )
						{
							if ( uservectitorTmp[i].bstrStreamId == bstrStreamItem )
							{
								bIsExist = true;
								iCursorId = uservectitorTmp[i].iCurSorID ;
								break;
							}
						}
						if ( bIsExist ) // 如果找到CurSorId
						{
							GetRegionTimeData(strStreamId,strStreamItem);
							if  ( iCursorId != -1 ) 
							{
								dPos = m_dStartPos + ( dRatio * (m_dEndPos- m_dStartPos) );
								hr   = containDataTmp.pContainControl->DrawLine(iCursorId,dPos);
							}
						}
						else // 如果没有CurSorId，则重新建立CurSorId
						{
							//创建用户的CursorID号值，并保存该到Map列表中。
							hr = containDataTmp.pContainControl->CreateCursor(m_CursorColor,&iCursorId);
							if ( iCursorId < 151 )
							{
								cursorDataTmp.iCurSorID    = iCursorId;
								cursorDataTmp.bstrStreamId = bstrStreamItem; //保存StreamItem值
								cursorVectTmp.push_back(cursorDataTmp);
								m_UserCursorDataMap[bstrChanelId] = cursorVectTmp;
								m_CursorColor += 500; // 颜色递增500,先暂时这么定

								GetRegionTimeData(strStreamId,strStreamItem);
								if  ( iCursorId != -1 ) 
								{
								
									dPos = m_dStartPos + ( dRatio * (m_dEndPos- m_dStartPos) );
									hr   = containDataTmp.pContainControl->DrawLine(iCursorId,dPos);
								}
							}
							else
							{
								hr = E_HANDLE;
							}
						}
					}
					else
					{
						hr = E_HANDLE;
					}
				}
			}
		}
	}
	catch(...)
	{
		return E_HANDLE;
	}
	return hr;
}

LRESULT CColorBarContainerControl::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	bHandled = TRUE;
	return S_OK;
}

HRESULT CColorBarContainerControl::OnStreamEventInit()
{
	
	DWORD dwSize = 0;

	// get topic manager endpoint
	
//	char szTopicManagerEndpoint[256];
//	wcstombs(szTopicManagerEndpoint, DEF_TOPIC_MANAGER_ENDPOINT_STREAM, 256);
//	m_tsTopicManagerEndpoint_Stream = szTopicManagerEndpoint;

	// get subscribe side endpoint to receive the message from IceStorm
		
//	char szSubscribeEndpoint[256];
//	wcstombs(szSubscribeEndpoint, DEF_SUBSCRIBER_ENDPOINT_STREAM, 256);
//	m_tsSubscribeEndpoint_Stream = szSubscribeEndpoint;
	
	return S_OK;
}



// Subscribe Stream Event Sink
bool CColorBarContainerControl::SubscribeStreamEventSink()
{
		
	bool bRet = true;
	
	// Subscribe Stream Event Sink
	
	StreamEventSinkPtr sink = new StreamEventSinkImpl(this);
	::TianShanIce::Properties qos;

	
	bRet = m_eventChannel->sink(sink, qos);
	if(!bRet)
	{
		ATLTRACE(_T("Failed to subscribe topic %s to Eventchannel"),m_tsSubscribeEndpoint_Stream.c_str());
		return false;
	}
	
	return bRet;
}


// Subscribe PlayList Event Sink
bool CColorBarContainerControl::SubscribePlaylistEventSink()
{
		
	bool bRet = true;
	
	// Subscribe Stream Event Sink
	
	
	
	PlaylistEventSinkPtr sink = new PlaylistEventSinkImpl(this);
	::TianShanIce::Properties qos;

	
	bRet = m_eventChannel->sink(sink, qos);
	if(!bRet)
	{
		ATLTRACE(_T("Failed to subscribe topic %s to Eventchannel"),m_tsSubscribeEndpoint_Stream.c_str());
		return false;
	}
	
	return bRet;
}


// Subscribe Progress Event Sink
bool CColorBarContainerControl::SubscribeProgressSink()
{
	bool bRet = true;
	
	// Subscribe Progress Event Sink
	StreamProgressSinkPtr  progsink = new StreamProgressSinkImpl(this);
	
	::TianShanIce::Properties qos;

	bRet = m_eventChannel->sink(progsink);
	if(!bRet)
	{
		ATLTRACE(_T("Failed to subscribe topic %s to Eventchannel"),m_tsSubscribeEndpoint_Stream.c_str());
		return false;
	}
	return bRet;
}

// Start the Chanel Ice
HRESULT CColorBarContainerControl::OnChanelStart()
{
	int  i = 0;
	try
	{
		if ( m_Communicator  == NULL )
		{
			m_Communicator = Ice::initialize(i, NULL);
		}
//		m_ChanelAdapter = m_Communicator->createObjectAdapterWithEndpoints("ColorBarContainer.Subscriber", m_tsSubscribeEndpoint_Chanel.c_str());
//		m_ChanelAdapter->activate();
	}
	catch(const Ice::Exception& ex)
	{
		ATLTRACE(_T("Initialize Ice runtime met exception with error: %s"), 
						               ex.ice_name().c_str());
		return E_HANDLE;
	}
	catch(...)
	{
		ATLTRACE(_T("Initialize Ice runtime met unknow exception"));
		return E_HANDLE;
	}

	// Initialize Channel
	try
	{
//		m_publisherExPrx = ::ChannelOnDemand::ChannelPublisherExPrx::checkedCast(m_ChanelAdapter->getCommunicator()->stringToProxy(m_tsTopicManagerEndpoint_Chanel.c_str()));
		m_publisherExPrx = ::ChannelOnDemand::ChannelPublisherExPrx::checkedCast(m_Communicator->stringToProxy(m_tsTopicManagerEndpoint_Chanel.c_str()));
	}
	catch(::Ice::ProxyParseException e)
	{
		ATLTRACE(_T("There was an error while parsing the stringified proxy, ChannelPublisher:%s"),m_tsTopicManagerEndpoint_Chanel.c_str());
		return E_HANDLE;
	}
	catch(::Ice::NoEndpointException e)
	{
		ATLTRACE(_T("Endpoint no found!"));
		return E_HANDLE;
	}
	catch(::Ice::ObjectNotFoundException e)
	{
		ATLTRACE(_T("Can not found object!"));
		return E_HANDLE;
	}
	catch(::Ice::ObjectNotExistException e)
	{
		ATLTRACE(_T("Can not found object!"));
		return E_HANDLE;
	}
	catch(...)
	{
		ATLTRACE(_T("Unknown exception got when parsing ,ChannelPublisher: %s!"),m_tsTopicManagerEndpoint_Chanel.c_str());
		return E_HANDLE;
	}

	if(!m_publisherExPrx)
	{
		ATLTRACE(_T("Failed to connect to ChannelPublisher,ChannelPublisher:%s!"),m_tsTopicManagerEndpoint_Chanel.c_str());
		return E_HANDLE;
	}
	return S_OK;
}

string  CColorBarContainerControl::MidString(string strSource,int iStartPos, int iEndPos)
{
	string strReturn ;
	strReturn.empty();
	strReturn="";
	int i;
	for ( i = iStartPos; i < iEndPos; i ++ )
	{
		strReturn +=strSource.at(i);
	}
	return strReturn;
}

double  CColorBarContainerControl::ConvertToHour(string strTime)
{
	double dReturn = 0.0;
	double dHour,dMinute,dSecond;
	string strHour,strMinute,strSecond;
	int pos,pos1;
	pos  = strTime.find_first_of(":");
	pos1 = strTime.find_last_of(":");
	strHour   = MidString(strTime,0,pos);
	strMinute = MidString(strTime,pos+1,pos1);
	strSecond = MidString(strTime,pos1+1,strTime.length());
	dHour   = (double)atoi(strHour.c_str());
	dMinute = (double)((double)(atoi(strMinute.c_str())/(double)60));
	dSecond = (double)((double)(atoi(strSecond.c_str())/(double)3600));
	dReturn = dHour + dMinute + dSecond;
	return dReturn;
}

double CColorBarContainerControl::CalcTimeDiff(string strStartTime,string strEndTime)
{
	double dReturn;
	
	string strStartFront,strStartEnd;
	string strEndFront,strEndEnd;
	string strReturn;
	strReturn.empty();
	strReturn ="";
	dReturn = 0.0;

	int ipos;
	ipos = strStartTime.find("T");
	

	strStartFront = MidString(strStartTime,0,ipos);
	strEndFront   = MidString(strEndTime,0,ipos);

	strStartEnd   = MidString(strStartTime,ipos+1,strStartTime.length());
	strEndEnd     = MidString(strEndTime,ipos+1,strEndTime.length());

	if ( strStartFront == strEndFront )
	{
		dReturn = ConvertToHour(strEndEnd) - ConvertToHour(strStartEnd);
	}
	else
	{
		dReturn = ConvertToHour(strEndEnd) - ConvertToHour(strStartEnd) + 24.00; 
	}
	return dReturn;
}

//根据StreamId得到该子节目的开始位置与终止位置
HRESULT CColorBarContainerControl::GetRegionTimeData(string strStreamId,string strComment)
{
	try
	{
		m_dStartPos = 0.0;
	    m_dEndPos   = 0.0;

		CComBSTR bstrStreamId;
		CComBSTR bstrComment;
		CComBSTR bstrTemp;
		CString  cstrChanelName;
		

		bstrStreamId = strStreamId.c_str();
		bstrComment  = strComment.c_str();
		
		
		// 先找出节目的名称

		if ( !m_ContainerMap.empty() )
		{
			CONTAINERMAPITER  containItor;
			CONTAINDATA       containDataTmp;
			memset((void*)&containDataTmp,0,sizeof(containDataTmp));
		
			CComBSTR bstrChanelId;
			containItor    = m_ContainerMap.find(bstrStreamId);

			containDataTmp = (*containItor).second;
			bstrChanelId   = containDataTmp.bstrChanelId; //节目频道名称

			
			if ( !m_ChanelMap.empty() )
			{
				MAPITERTMP  itorTmp;
				DATAVECTOR  VectorTmp;
				size_t  size,j;

				VectorTmp.clear();
				itorTmp = m_ChanelMap.find(bstrChanelId);
				if ( itorTmp == m_ChanelMap.end() )
				{
					throw ;
				}
				VectorTmp = (*itorTmp).second;
				size = VectorTmp.size();

				USES_CONVERSION;

				if ( size > 0 )
				{
					for ( j = 0 ; j < size ; j ++ )
					{
						bstrTemp       = VectorTmp[j].bstrFilmName;
						cstrChanelName = OLE2T(bstrTemp.m_str);
						
						if ( cstrChanelName.Find(strComment.c_str()) != - 1 )  // 找到子节目
						{
							m_dStartPos = VectorTmp[j].dStartPos;
	                        m_dEndPos   = VectorTmp[j].dEndPos ;
							break;
						}
					}
				}
			}
		}
	}
	catch(...)
	{
		return E_HANDLE;
	}
	return S_OK;
}

HRESULT CColorBarContainerControl::GetChanelName()
{
	try
	{
		if (m_publisherExPrx)
		{
			string strChannelName;
			CComBSTR bstrChannelName;
			string strName;
			string strDesc;
			string strContentName;
			size_t iSize;
			::ChannelOnDemand::ChannelItem tmpItem,tmpItem1;
			TianShanIce::StrValues channels;
			TianShanIce::StrValues items;

			DATAVECTOR  VectorTmp;
			
			VECTORTYPE  DataTmp;
			COLORREF    crColor; 
					
			channels= m_publisherExPrx->list();
			iSize   = channels.size();

			for(size_t i=0; i<channels.size(); i++)
			{
				strChannelName.empty();
				strName.empty();
				strDesc.empty();
				strContentName.empty();

				VectorTmp.clear();
				strChannelName  = channels[i];
				bstrChannelName =strChannelName.c_str();
				m_channelPrx = m_publisherExPrx->open(strChannelName);

				if ( m_channelPrx )
				{
					// re-query all children items
					items = m_channelPrx->getItemSequence();
					iSize = items.size();

					strName = m_channelPrx->getName();
					strDesc = m_channelPrx->getDesc();
					
	//				size_t keyPrefixLen = strName.length() + strlen(CHANNELITEM_KEY_SEPARATOR);
					size_t keyPrefixLen = strName.length() ;

					if ( iSize == 0 ) // 没有子节目
					{
						memset((void*)&DataTmp,0,sizeof(DataTmp));
						crColor = RGB(255,0,0);
						DataTmp.crColor      = crColor;
						DataTmp.bstrFilmName = strName.c_str();
						DataTmp.dStartPos    = 0.0; //默认为0
						DataTmp.dEndPos      = 2.0; //默认为3小时
						VectorTmp.push_back(DataTmp);

	//					m_ChanelMap.insert(MAPTYPE(strChannelName,VectorTmp));
						m_ChanelMap[bstrChannelName] = VectorTmp;
					}
					else //有子节目
					{
						for(size_t j = 0; j<items.size(); j++)
						{
	//						strContentName = items[j].substr(keyPrefixLen);
							strContentName = items[j];
							tmpItem = m_channelPrx->findItem(strContentName);

							if(!tmpItem.contentName.empty())
							{
								//颜色的设置还有待讨论,暂时设为以下几个值
								if ( j % 3  ==0 )
								{
									crColor = RGB(255,0,0);
								}
								else if ( j % 3 == 1 )
								{
									crColor = RGB(0,255,0);
								}
								else if (  j % 3 == 2 )
								{
									crColor = RGB(0,0,255);
								}
								memset((void*)&DataTmp,0,sizeof(DataTmp));

								DataTmp.crColor       = crColor;
								DataTmp.bstrFilmName  = strName.c_str();
								DataTmp.bstrFilmName.Append(".");
								DataTmp.bstrFilmName.Append(tmpItem.contentName.c_str());
								if ( items.size() == 1 ) // if only a item
								{
									DataTmp.dStartPos    = 0.0; //默认为0
									DataTmp.dEndPos      = 2.0; //默认为2小时,有待讨论
								}
								else
								{
									if ( j == 0 ) // the first item
									{
										DataTmp.dStartPos    = 0.0; //默认为0
										
	//									strContentName = items[j+1].substr(keyPrefixLen);
										strContentName = items[j+1];
										tmpItem1 = m_channelPrx->findItem(strContentName);
										DataTmp.dEndPos      = DataTmp.dStartPos + CalcTimeDiff(tmpItem.broadcastStart,tmpItem1.broadcastStart);
									}
									else if ( j == (items.size() - 1 ) ) // the last item
									{
										DataTmp.dStartPos    = VectorTmp[j-1].dEndPos; //默认为0
										DataTmp.dEndPos      = DataTmp.dStartPos + 2.0; //默认为2小时,有待讨论
									}
									else // the middle items
									{
	//									strContentName = items[j+1].substr(keyPrefixLen);
										strContentName = items[j+1];
										tmpItem1 = m_channelPrx->findItem(strContentName);

										DataTmp.dStartPos    = VectorTmp[j-1].dEndPos; //默认为0
										DataTmp.dEndPos      = DataTmp.dStartPos + CalcTimeDiff(tmpItem.broadcastStart,tmpItem1.broadcastStart);
									}
								}
								VectorTmp.push_back(DataTmp);
							//strTemp +=tmpItem.broadcastStart; broadcastStart是开始的时间值
							}
						}
	//					m_ChanelMap.insert(MAPTYPE(strChannelName,VectorTmp));
						m_ChanelMap[bstrChannelName] = VectorTmp;
					}
				}
			}

			// for the test
/*	
			string strChaneId;
			string strStreamId;
			strStreamId ="d7e83825-00df-4792-8422-335af6adceb7";
			strChaneId = m_publisherExPrx->getChannelByStream(strStreamId);
*/
		}
	}
	catch(...)
	{
		return E_HANDLE;
	}
	return S_OK;
}

HRESULT CColorBarContainerControlCreateContainWinow()
{

	return S_OK;
}

HRESULT CColorBarContainerControl::OnChanelUnInit()
{
	if ( m_channelPrx)
	{
		m_channelPrx = NULL;
	}

	m_publisherExPrx = NULL;
//	m_ChanelAdapter = NULL;
	if(m_Communicator != NULL)
	{
		try
		{
//			m_Communicator->destroy();
//			m_Communicator = NULL;
		}
		catch(...)
		{
			return S_OK;
		}
	}
	return S_OK;
}


HRESULT CColorBarContainerControl::OnStreamEventStart()
{	
	// Initialize Ice environment
	int  i = 0;
	try
	{
		if ( m_Communicator  == NULL )
		{
			m_Communicator = Ice::initialize(i, NULL);
		}
		m_StreamAdapter = m_Communicator->createObjectAdapterWithEndpoints("ColorBarContainer.Subscriber", m_tsSubscribeEndpoint_Stream.c_str());
		m_StreamAdapter->activate();
	}
	catch(const Ice::Exception& ex)
	{
		ATLTRACE(_T("Initialize Ice runtime met exception with error: %s"), 
						               ex.ice_name().c_str());
		return E_HANDLE;
	}
	catch(...)
	{
		ATLTRACE(_T("Initialize Ice runtime met unknow exception"));
		return E_HANDLE;
	}
	
	// 
	//
	// Initialize EventChannel
	//
	try
	{
	//  m_eventChannel = new TianShanIce::Events::EventChannelImpl(m_StreamAdapter,DEF_TOPIC_MANAGER_ENDPOINT_STREAM,true);
		m_eventChannel = new TianShanIce::Events::EventChannelImpl(m_StreamAdapter,m_tsTopicManagerEndpoint_Stream.c_str(),true);
	}
	catch(const TianShanIce::BaseException& ex)
	{
		ATLTRACE(_T("Initialize Event Channel on endpointer %s failed with error %s"), 
					m_tsSubscribeEndpoint_Stream.c_str(), ex.message.c_str());
		return E_HANDLE;
	}
	catch(...)
	{
		ATLTRACE(_T("Initialize Event Channel on endpointer %s failed with unknown exception"), 
					m_tsTopicManagerEndpoint_Stream.c_str());
		return E_HANDLE;
	}
	bool bRet = true;
	
	//
	// Subscribe Stream Event StateChange topic
	// 
	bRet = SubscribeStreamEventSink();
	if(!bRet)
	{
		ATLTRACE(_T("Subscribe Stream Event event failed"));
		return E_HANDLE;
	}

	//
	// Subscribe Stream  Progress topic
	// 
	bRet = SubscribeProgressSink();
	if(!bRet)
	{
		ATLTRACE(_T("Subscribe Progress  event failed"));
		return E_HANDLE;
	}

	// 可以不需要监听PlaylistEvent Sink 事件
	/*
	bRet = SubscribePlaylistEventSink();
	if ( !bRet )
	{
		ATLTRACE(_T("Subscribe Playlist  event failed"));
		return E_HANDLE;
	}
	*/
	m_eventChannel->start();
	return S_OK;
}

HRESULT CColorBarContainerControl::OnStreamEventUnInit(void)
{
	m_eventChannel = NULL;
	m_StreamAdapter = NULL;
	if(m_Communicator != NULL)
	{
		try
		{
			m_Communicator->destroy();
			m_Communicator = NULL;
		}
		catch(...)
		{
			return E_HANDLE;
		}
	}
	return S_OK;
}

STDMETHODIMP CColorBarContainerControl::SetStreamIPAddress(BSTR bstrIpAddress,BSTR bstrPort)
{
	try
	{
		TCHAR strText[256];
		USES_CONVERSION;
		wsprintf(strText,OLE2T(bstrIpAddress));
		m_tsTopicManagerEndpoint_Stream  ="TianShanEvents/TopicManager:tcp -h ";
		m_tsTopicManagerEndpoint_Stream +=strText;
		m_tsTopicManagerEndpoint_Stream +=" -p ";
		memset((void*)strText,0,sizeof(strText));
		wsprintf(strText,OLE2T(bstrPort));
		m_tsTopicManagerEndpoint_Stream +=strText;

	//	m_tsTopicManagerEndpoint_Stream +=" -p 10000";
	//	MessageBox(m_tsTopicManagerEndpoint_Stream.c_str());
		OnStreamEventStart();
	
		// for the test
		
//		string strStreamId ="TestStream1";
//		CreateContainMap(strStreamId); // for the test
		// for the test
	}
	catch(...)
	{
		return E_HANDLE;

	}
	return S_OK;
}

STDMETHODIMP CColorBarContainerControl::SetChanelIpAddress(BSTR bstrIpAddress,BSTR bstrPort)
{
	try
	{
		TCHAR strText[256];
		USES_CONVERSION;
		wsprintf(strText,OLE2T(bstrIpAddress));

		m_tsTopicManagerEndpoint_Chanel  ="ChannelPublisherEx:tcp -h ";
		m_tsTopicManagerEndpoint_Chanel +=strText;

		m_tsTopicManagerEndpoint_Chanel +=" -p ";
		memset((void*)strText,0,sizeof(strText));
		wsprintf(strText,OLE2T(bstrPort));
		m_tsTopicManagerEndpoint_Chanel +=strText;

	//	MessageBox(m_tsTopicManagerEndpoint_Chanel.c_str());
		OnChanelStart();
		GetChanelName();

	//	OnTest(); // for the test
		CreateContainWinow();
		BOOL b = FALSE;
		OnSize(0,0,0,b);

	}
	catch(...)
	{
		return E_HANDLE;
	}
	return S_OK;
}
