// ZQEventsCtrl.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "ZQEventsCtrl.h"
#include "TianShanEventImpl.h"
#include "StreamEventSinkImpl.h"
#include "StreamProgressSinkImpl.h"
#include "PlaylistEventSinkImpl.h"
#include "ProvisionProgressImpl.h"
#include "ProvisionStateChangeImpl.h"
#include "SessionEventSinkImpl.h"

string m_strServiceName;
string m_strServiceEndPoint;
TianShanIce::Streamer::StreamSmithAdminPrx   m_StreamSmithAdminClient = NULL;
Ice::CommunicatorPtr	m_Communicator = NULL;

RegPlayListStateProc m_StateProc = NULL; // for the playliststate proc
void ReadConfigFromXml();
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	string strIceStormEndPoint ;
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			ReadConfigFromXml();
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			if ( m_StateProc)
			{
				m_StateProc = NULL;
			}
			if ( m_Communicator)
			{
			//	m_Communicator->destroy(); // if call it,then the process is not delete when exit,why?,so remark the code 20070424
				m_Communicator = NULL;
			}
			
			break;
    }
    return TRUE;
}

void  InitIceStorm()
{
	try
	{
	
		string strIceStormPoint = std::string(SERVICE_NAME_TopicManager ":") ;
		strIceStormPoint += "default -h 10.50.12.4 -p 10000";
		
		TianShanIce::Events::EventChannelImpl::Ptr eventChannel = NULL;
		Ice::CommunicatorPtr	ic;
		int i =0;
		ic = Ice::initialize(i,NULL);
		
		Ice::ObjectAdapterPtr adapter= ic->createObjectAdapterWithEndpoints("Events.Subscriber","default -h 10.50.12.4 -p 10000");
		eventChannel = new TianShanIce::Events::EventChannelImpl(adapter,strIceStormPoint.c_str(),true);
		if ( !eventChannel)
		{
			printf("StreamOnProgressEvent failed\n");
			return ;
		}
				
		 // StreamProgressEvent
	//	TianShanIce::Streamer::StreamProgressSinkPtr sink = new StreamProgressSinkImpl();
	//	eventChannel->sink(sink);
		

		// StreamPlayListEvent
		StreamEventSinkImpl  *sink1 ;
		sink1 = new StreamEventSinkImpl();

	//	TianShanIce::Streamer::StreamEventSinkPtr sink = new StreamEventSinkImpl();
		TianShanIce::Streamer::StreamEventSinkPtr sink ;
		sink = sink1;

		::TianShanIce::Properties qos;
		bool bRet = eventChannel->sink(sink, qos);
		adapter->activate();
	}
	catch( const ::Ice::Exception &ex)
	{
	}
	catch(...)
	{
	}
	return ;
}

// 返回服务的类别值
int  GetCateGoryDatas(ITEMDATA **pCategorys,int *iCount)
{
	// Categorys's Data
	int i ;
	if ( _stricmp(m_strServiceName.c_str(),"StreamSmith") == 0 )
	{
		*iCount =  7;
		*pCategorys = new ITEMDATA[*iCount];
		for (  i = 0; i < (*iCount); i ++ )
		{
			(*pCategorys)[i].szItemData = (char*)malloc(ITEMLEN);
		}
		sprintf((*pCategorys)[0].szItemData,"%s","OnEndOfStream");
		sprintf((*pCategorys)[1].szItemData,"%s","OnBeginningOfStream");
		sprintf((*pCategorys)[2].szItemData,"%s","OnSpeedChanged");
		sprintf((*pCategorys)[3].szItemData,"%s","OnStateChanged");
		sprintf((*pCategorys)[4].szItemData,"%s","OnExit");
		sprintf((*pCategorys)[5].szItemData,"%s","OnProgress");
		sprintf((*pCategorys)[6].szItemData,"%s","OnItemStepped");
	}
	else if (  (_stricmp(m_strServiceName.c_str(),"ClusterContentStore") == 0 ) ||  (_stricmp(m_strServiceName.c_str(),"NodeContentStore") == 0 )  ) // ContentStore
	{
		*iCount =  2;
		*pCategorys = new ITEMDATA[*iCount];
		for (  i = 0; i < (*iCount); i ++ )
		{
			(*pCategorys)[i].szItemData = (char*)malloc(ITEMLEN);
		}
		sprintf((*pCategorys)[0].szItemData,"%s","OnProgress");
		sprintf((*pCategorys)[1].szItemData,"%s","OnStateChanged");
	}
	else if ( ( _stricmp(m_strServiceName.c_str(),"Weiwoo") == 0 )  ||  ( _stricmp(m_strServiceName.c_str(),"SiteAdminSvc") == 0 ) )
	{
		*iCount =  3;
		*pCategorys = new ITEMDATA[*iCount];
		for (  i = 0; i < (*iCount); i ++ )
		{
			(*pCategorys)[i].szItemData = (char*)malloc(ITEMLEN);
		}
		sprintf((*pCategorys)[0].szItemData,"%s","OnNewSession");
		sprintf((*pCategorys)[1].szItemData,"%s","OnDestroySession");
		sprintf((*pCategorys)[2].szItemData,"%s","OnStateChanged");
	}
	else //无Event Message
	{
		*iCount = 0;
		pCategorys = NULL;
	}
	return 1;
}

#ifdef PTRMODE

int  GetPlayListData_Proc(const char * cUid,const char * cStateValue,ITEMDATA **pColumnNames,int *iCol)
{
	string strPlayListId;
	strPlayListId =   cUid;
	string strColumnName;
	int iTemp;
	int i;
	char sTemp[50]={0};

	TianShanIce::Streamer::SpigotBoards SpigotBoardsData;
	SpigotBoardsData.clear();
	
	try
	{
		TianShanIce::Streamer::PlaylistPrx playlist = m_StreamSmithAdminClient->openPlaylist(strPlayListId,SpigotBoardsData,false);
		if ( playlist )
		{
			TianShanIce::Streamer::PlaylistExPrx playlistEx = 	TianShanIce::Streamer::PlaylistExPrx::checkedCast(playlist);
			if ( playlistEx)
			{
				*iCol = 8;

				// one playlist's Data
				*pColumnNames = new ITEMDATA[*iCol];
				for (   i = 0; i < (*iCol); i ++ )
				{
					(*pColumnNames)[i].szItemData = (char*)malloc(ITEMLEN);
				}
				
				TianShanIce::Streamer::PlaylistAttr   playlistAttrisData = playlistEx->getAttr();
				
				sprintf((*pColumnNames)[0].szItemData,"%s",strPlayListId.c_str());

				strColumnName = playlistAttrisData.destIP;
				sprintf((*pColumnNames)[1].szItemData,"%s",strColumnName.c_str());

				
				iTemp = playlistAttrisData.destPort;
				memset(sTemp,0,sizeof(sTemp));
				sprintf(sTemp,"%d",iTemp);
				strColumnName = sTemp;
				sprintf((*pColumnNames)[2].szItemData,"%s",strColumnName.c_str());

				
				strColumnName = playlistAttrisData.destMac;
				sprintf((*pColumnNames)[3].szItemData,"%s",strColumnName.c_str());


				iTemp = playlistAttrisData.vstrmPort;
				memset(sTemp,0,sizeof(sTemp));
				sprintf(sTemp,"%d",iTemp);
				strColumnName = sTemp;
				sprintf((*pColumnNames)[4].szItemData,"%s",strColumnName.c_str());


				iTemp = playlistAttrisData.programNumber;
				memset(sTemp,0,sizeof(sTemp));
				sprintf(sTemp,"%d",iTemp);
				strColumnName = sTemp;
				sprintf((*pColumnNames)[5].szItemData,"%s",strColumnName.c_str());


				strColumnName = cStateValue;
				sprintf((*pColumnNames)[6].szItemData,"%s",strColumnName.c_str());

				iTemp = playlistAttrisData.currentCtrlNum;
				memset(sTemp,0,sizeof(sTemp));
				sprintf(sTemp,"%d",iTemp);
				strColumnName = sTemp;
				sprintf((*pColumnNames)[7].szItemData,"%s",strColumnName.c_str());
			}
		}
	}
	catch( const ::Ice::Exception &ex)
	{
		*iCol = 8;
		*pColumnNames = new ITEMDATA[*iCol];
		for (  i = 0; i < (*iCol); i ++ )
		{
			(*pColumnNames)[i].szItemData = (char*)malloc(ITEMLEN);
		}
		sprintf((*pColumnNames)[0].szItemData,"%s",cUid);
		for (  i = 1; i < 8 ; i ++ )
		{
			sprintf((*pColumnNames)[i].szItemData,"%s","");
		}
	}
	catch(...)
	{
		*iCol = 8;
		*pColumnNames = new ITEMDATA[*iCol];
		for (  i = 0; i < (*iCol); i ++ )
		{
			(*pColumnNames)[i].szItemData = (char*)malloc(ITEMLEN);
		}

		sprintf((*pColumnNames)[0].szItemData,"%s",cUid);
		for ( int i = 1; i < 8 ; i ++ )
		{
			sprintf((*pColumnNames)[i].szItemData,"%s","");
		}
	}
	return 0;
}


//ITEMDATA **  GetData_Proc(ATTRIBSDATA **pAttribsData, int *iAttCount,ITEMDATA **pColumnNames,int *iRow, int *iCol,ITEMDATA **pCellsData)
//ITEMDATA **  GetData_Proc(GRIDDATAARRAY2 & AttribsData,ITEMDATA **pColumnNames,int *iRow, int *iCol,ITEMDATA **pCellsData);
//ITEMDATA **  GetData_Proc(char * cCondition,char * cTabname,GRIDDATAARRAY2 & AttribsData,ITEMDATA **pColumnNames,int *iRow, int *iCol,ITEMDATA **pCellsData)
//ITEMDATA **  GetData_Proc(char * cCondition,char * cTabname,GRIDDATAARRAY2 & AttribsData,ITEMDATA **pColumnNames,int *iRow, int *iCol,ITEMDATA **pCellsData,RegProgrssProc proc)
ITEMDATA   **  GetData_Proc(char * cCondition,char * cTabname,GRIDDATAARRAY2 & AttribsData,ITEMDATA **pColumnNames,int *iRow, int *iCol,ITEMDATA **pCellsData,RegProgrssProc proc,RegPlayListStateProc stateProc )
{
	if ( stateProc )
	{
		m_StateProc = stateProc;
	}
	int i =0;
	
	try
	{
		string strEndPoint;
		string strData;
		int iSize;
		int j,m;
		STRVECTOR OneRowData;
		GRIDDATAARRAY RowDataArray;
		string strTemp;
		string strColumnName;
		char sTemp[50]={0};

		if ( _stricmp(m_strServiceName.c_str(),"StreamSmith") == 0 )
		{
			int iStreamerNum,iPlaylistNum;
//			TianShanIce::Streamer::StreamSmithAdminPrx   m_StreamSmithAdminClient = NULL;
			
			
			strEndPoint = "StreamSmith:";
   		    strEndPoint +=m_strServiceEndPoint;
			if ( m_Communicator)
			{
				m_StreamSmithAdminClient= TianShanIce::Streamer::StreamSmithAdminPrx::checkedCast(m_Communicator->stringToProxy(strEndPoint));
				if ( m_StreamSmithAdminClient)
				{
					if ( _stricmp(cTabname,"Streamer") == 0 ) // Streamer Tab
					{
						TianShanIce::Streamer::StreamerDescriptors  StreamerDes = m_StreamSmithAdminClient->listStreamers();
						iStreamerNum = StreamerDes.size();
						if ( iStreamerNum > 0 )
						{
							*iCol = 3;
							*iRow = iStreamerNum;
							
							// ColumnNames's Data
							*pColumnNames = new ITEMDATA[*iCol];
							for (  i = 0; i < (*iCol); i ++ )
							{
								(*pColumnNames)[i].szItemData = (char*)malloc(ITEMLEN);
							}
							sprintf((*pColumnNames)[0].szItemData,"%s","deviceid");
							sprintf((*pColumnNames)[1].szItemData,"%s","type");
							sprintf((*pColumnNames)[2].szItemData,"%s","netid");

							// CellsData,二维数组
							pCellsData = new ITEMDATA* [*iRow];
							for ( j = 0; j < (*iRow); j ++)
							{
									pCellsData[j] = new ITEMDATA[(*iCol)];
							}

							for( i=0;i<(*iRow);i++)
							{
								for( j=0;j<(*iCol);j++)
								{
									(pCellsData[i][j]).szItemData = (char*)malloc(ITEMLEN);
								}
								sprintf((pCellsData[i][0]).szItemData,"%s", StreamerDes[i].deviceId.c_str());
								sprintf((pCellsData[i][1]).szItemData,"%s", StreamerDes[i].type.c_str());
								strColumnName = m_StreamSmithAdminClient->getNetId();
								sprintf((pCellsData[i][2]).szItemData,"%s", strColumnName.c_str());
							}

							
							// Attribes's Datas
							AttribsData.clear();
							

							/* below is the old operation mode
							OneRowData.clear();
							RowDataArray.clear();
						
							// for the streamer's column names
							strColumnName="NetId";
							OneRowData.push_back(strColumnName);
							RowDataArray[0]=OneRowData;	

							// for the streamer's cells data
							for ( i = 0; i < iStreamerNum; i ++)
							{

								strTemp = StreamerDes[i].deviceId;
								strColumnName = m_StreamSmithAdminClient->getNetId();
								
								OneRowData.clear();
								OneRowData.push_back(strColumnName);
								RowDataArray[1]=OneRowData;		
								AttribsData[strTemp]=RowDataArray;
							}
							*/
						}
						else
						{
							AttribsData.clear();
							pColumnNames = NULL;
							pCellsData   = NULL;
							*iRow = 0;
							*iCol = 0;
						}
					}
					// 这里有一个问题，即PlayList是动态变动的,所以会遇到属性不能拿到的现象。add by  dony 20070417
					else if ( _stricmp(cTabname,"PlayList") == 0 ) // PlayList Tab
					{
						string strPlayListId;
						int iTemp;
						TianShanIce::Streamer::PlaylistIDs playlistDes = m_StreamSmithAdminClient->listPlaylists();
						iPlaylistNum = playlistDes.size();
						if ( iPlaylistNum > 0 )
						{
							*iCol = 8;
							*iRow = iPlaylistNum;
							
							// ColumnNames's Data
							*pColumnNames = new ITEMDATA[*iCol];
							for (  i = 0; i < (*iCol); i ++ )
							{
								(*pColumnNames)[i].szItemData = (char*)malloc(ITEMLEN);
							}
							sprintf((*pColumnNames)[0].szItemData,"%s","PlayListID");
							sprintf((*pColumnNames)[1].szItemData,"%s","destIP");
							sprintf((*pColumnNames)[2].szItemData,"%s","destPort");
							sprintf((*pColumnNames)[3].szItemData,"%s","destMac");
							sprintf((*pColumnNames)[4].szItemData,"%s","vstrmPort");
							sprintf((*pColumnNames)[5].szItemData,"%s","programNumber");
							sprintf((*pColumnNames)[6].szItemData,"%s","State");
							sprintf((*pColumnNames)[7].szItemData,"%s","currentCtrlNum");

							

							// CellsData,二维数组
							pCellsData = new ITEMDATA* [*iRow];
							for ( j = 0; j < (*iRow); j ++)
							{
								pCellsData[j] = new ITEMDATA[(*iCol)];
							}

							for( i=0;i<(*iRow);i++)
							{
								for( j=0;j<(*iCol);j++)
								{
									(pCellsData[i][j]).szItemData = (char*)malloc(ITEMLEN);
								}

								strPlayListId =   playlistDes[i];

								TianShanIce::Streamer::SpigotBoards SpigotBoardsData;
								SpigotBoardsData.clear();
								
								try
								{
									TianShanIce::Streamer::PlaylistPrx playlist = m_StreamSmithAdminClient->openPlaylist(strPlayListId,SpigotBoardsData,false);
									if ( playlist )
									{
										TianShanIce::Streamer::PlaylistExPrx playlistEx = 	TianShanIce::Streamer::PlaylistExPrx::checkedCast(playlist);
										if ( playlistEx)
										{

											TianShanIce::Streamer::PlaylistAttr   playlistAttrisData = playlistEx->getAttr();
											
											sprintf((pCellsData[i][0]).szItemData,"%s",strPlayListId.c_str());

											strColumnName = playlistAttrisData.destIP;
											sprintf((pCellsData[i][1]).szItemData,"%s",strColumnName.c_str());


											iTemp = playlistAttrisData.destPort;
											memset(sTemp,0,sizeof(sTemp));
											sprintf(sTemp,"%d",iTemp);
											sprintf((pCellsData[i][2]).szItemData,"%s",strColumnName.c_str());

											
											strColumnName = playlistAttrisData.destMac;
											sprintf((pCellsData[i][3]).szItemData,"%s",strColumnName.c_str());


											iTemp = playlistAttrisData.vstrmPort;
											memset(sTemp,0,sizeof(sTemp));
											sprintf(sTemp,"%d",iTemp);
											strColumnName = sTemp;
											sprintf((pCellsData[i][4]).szItemData,"%s",strColumnName.c_str());


											iTemp = playlistAttrisData.programNumber;
											memset(sTemp,0,sizeof(sTemp));
											sprintf(sTemp,"%d",iTemp);
											strColumnName = sTemp;
											sprintf((pCellsData[i][5]).szItemData,"%s",strColumnName.c_str());


											// 注意以下两个值要结合Event，动态获取 20070423
											iTemp = playlistAttrisData.playlistState;
											memset(sTemp,0,sizeof(sTemp));
											sprintf(sTemp,"%d",iTemp);
											strColumnName = sTemp;
											sprintf((pCellsData[i][6]).szItemData,"%s",strColumnName.c_str());

											iTemp = playlistAttrisData.currentCtrlNum;
											memset(sTemp,0,sizeof(sTemp));
											sprintf(sTemp,"%d",iTemp);
											strColumnName = sTemp;
											sprintf((pCellsData[i][7]).szItemData,"%s",strColumnName.c_str());

										}
									}
								}
								catch( const ::Ice::Exception &ex)
								{
									for (  int p = 0; p < (*iCol); p ++)
									{
										free((*pColumnNames)[p].szItemData);
									}
									delete  pColumnNames;
									pColumnNames = NULL;

									// free the cells's memory
									for( int q=0;q<(*iRow);q++)
									{
										delete [] pCellsData[q]; //释放列
									}
									delete pCellsData; //释放行
									pCellsData = NULL;
									AttribsData.clear();
									*iRow = 0;
									*iCol = 0;
									break;
								}
								catch(...)
								{
									for (  int p = 0; p < (*iCol); p ++)
									{
										free((*pColumnNames)[p].szItemData);
									}
									delete  pColumnNames;
									pColumnNames = NULL;

									// free the cells's memory
									for( int q=0;q<(*iRow);q++)
									{
										delete [] pCellsData[q]; //释放列
									}
									delete pCellsData; //释放行
									pCellsData = NULL;
									AttribsData.clear();
									*iRow = 0;
									*iCol = 0;
									break;
								}
							}

							// Attribes's Data
							AttribsData.clear();

							/* below is the old operation mode
							OneRowData.clear();
							RowDataArray.clear();
						
							// for the playlist's column names
//							strColumnName="Guid";
//							OneRowData.push_back(strColumnName);

							strColumnName="SiteName";
							OneRowData.push_back(strColumnName);

							strColumnName="ResourceGuid";
							OneRowData.push_back(strColumnName);

							strColumnName="ClientSessID";//ClientSessionID";
							OneRowData.push_back(strColumnName);


							strColumnName="endPoint";
							OneRowData.push_back(strColumnName);

							strColumnName="MaxRate";
							OneRowData.push_back(strColumnName);

							strColumnName="MinRate";
							OneRowData.push_back(strColumnName);

							strColumnName="NowRate";
							OneRowData.push_back(strColumnName);

							strColumnName="destIP";
							OneRowData.push_back(strColumnName);

							strColumnName="destPort";
							OneRowData.push_back(strColumnName);

							strColumnName="destMac";
							OneRowData.push_back(strColumnName);

							strColumnName="vstrmPort";
							OneRowData.push_back(strColumnName);

							strColumnName="programNumber";
							OneRowData.push_back(strColumnName);

							strColumnName="playlistState";
							OneRowData.push_back(strColumnName);

							strColumnName="curCtrlNum"; // currentCtrlNum
							OneRowData.push_back(strColumnName);

							strColumnName="vstrmSessID";
							OneRowData.push_back(strColumnName);
							RowDataArray[0]=OneRowData;	

							// for the streamer's cells data
							for ( i = 0; i < iPlaylistNum; i ++)
							{
								strTemp = playlistDes[i];
								
								TianShanIce::Streamer::SpigotBoards SpigotBoardsData;
								SpigotBoardsData.clear();

								try
								{
									TianShanIce::Streamer::PlaylistPrx playlist = m_StreamSmithAdminClient->openPlaylist(strTemp,SpigotBoardsData,false);
									if ( playlist )
									{
										TianShanIce::Streamer::PlaylistExPrx playlistEx = 	TianShanIce::Streamer::PlaylistExPrx::checkedCast(playlist);
										if ( playlistEx)
										{

											TianShanIce::Streamer::PlaylistAttr   playlistAttrisData = playlistEx->getAttr();

											OneRowData.clear();
	//										strColumnName = playlistAttrisData.Guid;
	//										OneRowData.push_back(strColumnName);

											strColumnName = playlistAttrisData.StreamSmithSiteName;
											OneRowData.push_back(strColumnName);


											strColumnName = playlistAttrisData.ResourceGuid;
											OneRowData.push_back(strColumnName);


											strColumnName = playlistAttrisData.endPoint;
											OneRowData.push_back(strColumnName);

											iTemp = playlistAttrisData.MaxRate;
											memset(sTemp,0,sizeof(sTemp));
											sprintf(sTemp,"%d",iTemp);
											strColumnName = sTemp;
											OneRowData.push_back(strColumnName);

											iTemp = playlistAttrisData.MinRate;
											memset(sTemp,0,sizeof(sTemp));
											sprintf(sTemp,"%d",iTemp);
											strColumnName = sTemp;
											OneRowData.push_back(strColumnName);


											iTemp = playlistAttrisData.NowRate;
											memset(sTemp,0,sizeof(sTemp));
											sprintf(sTemp,"%d",iTemp);
											strColumnName = sTemp;
											OneRowData.push_back(strColumnName);

											strColumnName = playlistAttrisData.destIP;
											OneRowData.push_back(strColumnName);


											iTemp = playlistAttrisData.destPort;
											memset(sTemp,0,sizeof(sTemp));
											sprintf(sTemp,"%d",iTemp);
											strColumnName = sTemp;
											OneRowData.push_back(strColumnName);

											strColumnName = playlistAttrisData.destMac;
											OneRowData.push_back(strColumnName);


											iTemp = playlistAttrisData.vstrmPort;
											memset(sTemp,0,sizeof(sTemp));
											sprintf(sTemp,"%d",iTemp);
											strColumnName = sTemp;
											OneRowData.push_back(strColumnName);


											iTemp = playlistAttrisData.programNumber;
											memset(sTemp,0,sizeof(sTemp));
											sprintf(sTemp,"%d",iTemp);
											strColumnName = sTemp;
											OneRowData.push_back(strColumnName);

											iTemp = playlistAttrisData.playlistState;
											memset(sTemp,0,sizeof(sTemp));
											sprintf(sTemp,"%d",iTemp);
											strColumnName = sTemp;
											OneRowData.push_back(strColumnName);

											iTemp = playlistAttrisData.currentCtrlNum;
											memset(sTemp,0,sizeof(sTemp));
											sprintf(sTemp,"%d",iTemp);
											strColumnName = sTemp;
											OneRowData.push_back(strColumnName);

											iTemp = playlistAttrisData.vstrmSessID;
											memset(sTemp,0,sizeof(sTemp));
											sprintf(sTemp,"%d",iTemp);
											strColumnName = sTemp;
											OneRowData.push_back(strColumnName);

											RowDataArray[1]=OneRowData;		
											AttribsData[strTemp]=RowDataArray;
											iSize = AttribsData.size();
										}
										else
										{
											// Attribes's Data,if there are not datas
											AttribsData.clear();
											break;
										}
									}
									else
									{
										// Attribes's Data,if there are not datas
										AttribsData.clear();
										break;
									}
								}
								catch( const ::Ice::Exception &ex)
								{
									// Attribes's Data
									AttribsData.clear();
								}
								catch(...)
								{
									// Attribes's Data
									AttribsData.clear();
								}
							}
							*/
						}
						else
						{
							*iCol = 8;
							*iRow = 0;
							
							// ColumnNames's Data
							*pColumnNames = new ITEMDATA[*iCol];
							for (  i = 0; i < (*iCol); i ++ )
							{
								(*pColumnNames)[i].szItemData = (char*)malloc(ITEMLEN);
							}
							sprintf((*pColumnNames)[0].szItemData,"%s","PlayListID");
							sprintf((*pColumnNames)[1].szItemData,"%s","destIP");
							sprintf((*pColumnNames)[2].szItemData,"%s","destPort");
							sprintf((*pColumnNames)[3].szItemData,"%s","destMac");
							sprintf((*pColumnNames)[4].szItemData,"%s","vstrmPort");
							sprintf((*pColumnNames)[5].szItemData,"%s","programNumber");
							sprintf((*pColumnNames)[6].szItemData,"%s","State");
							sprintf((*pColumnNames)[7].szItemData,"%s","currentCtrlNum");

							
							// CellsData,二维数组
							pCellsData = new ITEMDATA* [*iRow];
							for ( j = 0; j < (*iRow); j ++)
							{
								pCellsData[j] = new ITEMDATA[(*iCol)];
							}

							for( i=0;i<(*iRow);i++)
							{
								for( j=0;j<(*iCol);j++)
								{
									(pCellsData[i][j]).szItemData = (char*)malloc(ITEMLEN);
									sprintf((pCellsData[i][j]).szItemData,"%s","");
								}
							}


							AttribsData.clear();
							/*
							pColumnNames = NULL;
							pCellsData   = NULL;
							*iRow = 0;
							*iCol = 0;
							*/
						}
					}
				}
				else
				{
					AttribsData.clear();
					pColumnNames = NULL;
					pCellsData   = NULL;
					*iRow = 0;
					*iCol = 0;
				}
			}
		}
		/* below is the old code
		
		if ( iSize > 0 )
		{
			
			*iCol = 2;
			*iRow = iSize;
			
			// ColumnNames's Data
			*pColumnNames = new ITEMDATA[*iCol];
			for (  i = 0; i < (*iCol); i ++ )
			{
				(*pColumnNames)[i].szItemData = (char*)malloc(ITEMLEN);
				if ( i == 0)
				{
					sprintf((*pColumnNames)[i].szItemData,"%s","deviceid");
				}
				else
				{
					sprintf((*pColumnNames)[i].szItemData,"%s","type");
				}
			}

			// CellsData,二维数组
			pCellsData = new ITEMDATA* [*iRow];
			for ( j = 0; j < (*iRow); j ++)
			{
					pCellsData[j] = new ITEMDATA[(*iCol)];
			}

			for( i=0;i<(*iRow);i++)
			{
				for( j=0;j<(*iCol);j++)
				{
					(pCellsData[i][j]).szItemData = (char*)malloc(ITEMLEN);
				}
				sprintf((pCellsData[i][0]).szItemData,"%s", StreamerDes[i].deviceId.c_str());
				sprintf((pCellsData[i][1]).szItemData,"%s", StreamerDes[i].type.c_str());
			}
			

			TianShanIce::Streamer::PlaylistIDs playlistDes = m_StreamSmithAdminClient->listPlaylists();
			iSize = playlistDes.size();
			
			/*
			// Attribes's Data
			*iAttCount = ( iSize + 2);
			*pAttribsData = new ATTRIBSDATA[*iAttCount];

			for ( i = 0; i <  iSize ; i ++)
			{
				(*pAttribsData)[i].szAttribsName  = (char*)malloc(NAEMELEN);
				(*pAttribsData)[i].szAttisbsValue = (char*)malloc(ITEMLEN);
				strData = playlistDes[i];
				if ( i == 0)
				{
					sprintf((*pAttribsData)[i].szAttribsName,"%s","PlayListID");
				}
				else
				{
					sprintf((*pAttribsData)[i].szAttribsName,"%s","");
				}
				sprintf((*pAttribsData)[i].szAttisbsValue,"%s",strData.c_str());
			}
		   (*pAttribsData)[iSize].szAttribsName  = (char*)malloc(NAEMELEN);
		   (*pAttribsData)[iSize].szAttisbsValue = (char*)malloc(ITEMLEN);
		   
		   
		   char sTotal[15]={0};
		   itoa(iSize,sTotal,10);
		   strData = sTotal;
		   sprintf((*pAttribsData)[iSize].szAttribsName,"%s","TotalIdNums");
		   sprintf((*pAttribsData)[iSize].szAttisbsValue,"%s",strData.c_str());
		   
		   (*pAttribsData)[iSize+1].szAttribsName  = (char*)malloc(NAEMELEN);
		   (*pAttribsData)[iSize+1].szAttisbsValue = (char*)malloc(ITEMLEN);
		   
		   strData = m_StreamSmithAdminClient->getNetId();
		   sprintf((*pAttribsData)[iSize+1].szAttribsName,"%s","NetId");
		   sprintf((*pAttribsData)[iSize+1].szAttisbsValue,"%s",strData.c_str());
		   */
		else if ( _stricmp(m_strServiceName.c_str(),"ChodSvc") == 0 ) // ChodSvc
		{
			
			::ChannelOnDemand::ChannelPublisherExPrx  chanelDememandClient = NULL;
			strEndPoint = "ChannelPublisherEx:";
			strEndPoint +=m_strServiceEndPoint;
			string strTemp;
			int iValue;
			int iChannelsNum;
			string strChannelName;
			string strItemName;
			bool bTemp;
			long lTemp;

			if ( m_Communicator)
			{
				chanelDememandClient= ::ChannelOnDemand::ChannelPublisherExPrx::checkedCast(m_Communicator->stringToProxy(strEndPoint));
				::ChannelOnDemand::ChannelPublishPointPrx chanelPublishClient ;
				::ChannelOnDemand::ChannelItem  ItemClient;
				TianShanIce::StrValues ItemValues ;
					
				if ( chanelDememandClient)
				{
					TianShanIce::StrValues channels = chanelDememandClient->list();
					iChannelsNum = channels.size();
							
					if ( iChannelsNum > 0 )
					{
						*iRow = iChannelsNum;
						*iCol = 3;
					
						// ColumnNames's Data
						*pColumnNames = new ITEMDATA[*iCol];
						for (  i = 0; i < (*iCol); i ++ )
						{
							(*pColumnNames)[i].szItemData = (char*)malloc(ITEMLEN);
						}
						sprintf((*pColumnNames)[0].szItemData,"%s","Name");
						sprintf((*pColumnNames)[1].szItemData,"%s","Description");
						sprintf((*pColumnNames)[2].szItemData,"%s","MaxBitrate");
						
						

						// CellsData,二维数组
						pCellsData = new ITEMDATA* [*iRow];
						for ( j = 0; j < (*iRow); j ++)
						{
							pCellsData[j] = new ITEMDATA[(*iCol)];
						}
						
						for( i=0;i<(*iRow);i++)
						{
							for( j=0;j<(*iCol);j++)
							{
								(pCellsData[i][j]).szItemData = (char*)malloc(ITEMLEN);
							}
							strChannelName =channels[i];
							
							chanelPublishClient = chanelDememandClient->open(strChannelName);
							if ( chanelPublishClient)
							{
								sprintf((pCellsData[i][0]).szItemData,"%s",strChannelName.c_str());

								strTemp = chanelPublishClient->getDesc();
								sprintf((pCellsData[i][1]).szItemData,"%s",strTemp.c_str());

								iValue = chanelPublishClient->getMaxBitrate();
								memset(sTemp,0,sizeof(sTemp));
								sprintf(sTemp,"%d",iValue);
								strTemp = sTemp;
								sprintf((pCellsData[i][2]).szItemData,"%s",strTemp.c_str());

								// Attribes's Datas's column name
								OneRowData.clear();
								RowDataArray.clear();
														
								// for the chodsvc's attribes column names
								strColumnName="contentName";
								OneRowData.push_back(strColumnName);

								strColumnName="broadcastStart";
								OneRowData.push_back(strColumnName);

								strColumnName="expiration";
								OneRowData.push_back(strColumnName);

								strColumnName="playable";
								OneRowData.push_back(strColumnName);

								strColumnName="forceNormalSpeed";
								OneRowData.push_back(strColumnName);

								strColumnName="inTimeOffset";
								OneRowData.push_back(strColumnName);

								strColumnName="outTimeOffset";
								OneRowData.push_back(strColumnName);

								strColumnName="spliceIn";
								OneRowData.push_back(strColumnName);

								strColumnName="spliceOut";
								OneRowData.push_back(strColumnName);
								RowDataArray[0]=OneRowData;	

								// for the chodsvc's attribes column names

								// get the chanel's Items
								TianShanIce::StrValues ItemValues=chanelPublishClient->getItemSequence();
								iSize = ItemValues.size();
								

								if ( iSize > 0)
								{
									for ( m = 0; m < iSize; m ++)
									{
										strItemName = ItemValues[m];
										ItemClient = chanelPublishClient->findItem(strItemName);
																		
										OneRowData.clear();
										
										strColumnName =ItemClient.contentName;
										OneRowData.push_back(strColumnName);

										strColumnName =ItemClient.broadcastStart;
										OneRowData.push_back(strColumnName);

										strColumnName =ItemClient.expiration;
										OneRowData.push_back(strColumnName);

										bTemp = ItemClient.playable;
										if ( bTemp)
										{
											strColumnName ="true";
										}
										else
										{
											strColumnName ="false";
										}
										OneRowData.push_back(strColumnName);

										bTemp = ItemClient.forceNormalSpeed;
										if ( bTemp)
										{
											strColumnName ="true";
										}
										else
										{
											strColumnName ="false";
										}
										OneRowData.push_back(strColumnName);

										lTemp = (long)ItemClient.inTimeOffset;
										memset(sTemp,0,sizeof(sTemp));
										ltoa(lTemp,sTemp,10);
										strColumnName =sTemp;
										OneRowData.push_back(strColumnName);


										lTemp = (long)ItemClient.outTimeOffset;
										memset(sTemp,0,sizeof(sTemp));
										ltoa(lTemp,sTemp,10);
										strColumnName =sTemp;
										OneRowData.push_back(strColumnName);


										bTemp = ItemClient.spliceIn;
										if ( bTemp)
										{
											strColumnName ="true";
										}
										else
										{
											strColumnName ="false";
										}
										OneRowData.push_back(strColumnName);

										bTemp = ItemClient.spliceOut;
										if ( bTemp)
										{
											strColumnName ="true";
										}
										else
										{
											strColumnName ="false";
										}
										OneRowData.push_back(strColumnName);
										RowDataArray[m+1]=OneRowData;		
										
									}
									AttribsData[strChannelName]=RowDataArray;
								}
							}
						}
					}
					else
					{
						AttribsData.clear();
						pColumnNames = NULL;
						pCellsData   = NULL;
						*iRow = 0;
						*iCol = 0;
					}
				}
			}
		}
		else if (  (_stricmp(m_strServiceName.c_str(),"ClusterContentStore") == 0 ) ||  (_stricmp(m_strServiceName.c_str(),"NodeContentStore") == 0 )  ) // ContentStore
		{
			long lFileSize;
			bool b1;
			float fFrameRate;
			int iContentStoreNum;
			TianShanIce::Storage::ContentStorePrx   contentstoreClient = NULL;
			
			strEndPoint = "ContentStore:";
			strEndPoint +=m_strServiceEndPoint;
			
			if ( m_Communicator)
			{
				contentstoreClient= TianShanIce::Storage::ContentStorePrx::checkedCast(m_Communicator->stringToProxy(strEndPoint));
				if ( contentstoreClient)
				{
					
					string strcondition;
//					strcondition ="*";
					strcondition =cCondition;
					TianShanIce::StrValues  contentDes = contentstoreClient->listContent(strcondition);
					iContentStoreNum = contentDes.size();
					string strStoreType = contentstoreClient->type();
					string strContentType ="MPEG2TS";
					string strContentName;
											
					
					if ( iContentStoreNum > 0 )
					{
						*iCol = 11;
						*iRow = iContentStoreNum;

						// ColumnNames's Data
						*pColumnNames = new ITEMDATA[*iCol];
						for (  i = 0; i < (*iCol); i ++ )
						{
							(*pColumnNames)[i].szItemData = (char*)malloc(ITEMLEN);
						}
						sprintf((*pColumnNames)[0].szItemData,"%s","ContentName");
						sprintf((*pColumnNames)[1].szItemData,"%s","isProvisioned");
						sprintf((*pColumnNames)[2].szItemData,"%s","ProvisionTime");
						sprintf((*pColumnNames)[3].szItemData,"%s","ContentType");
						sprintf((*pColumnNames)[4].szItemData,"%s","Framerate(fps)");
						sprintf((*pColumnNames)[5].szItemData,"%s","Filesize(bytes)");
						sprintf((*pColumnNames)[6].szItemData,"%s","PlayTime(ms)");
						sprintf((*pColumnNames)[7].szItemData,"%s","BitRate(bps)");
						sprintf((*pColumnNames)[8].szItemData,"%s","MD5Checksum");
						sprintf((*pColumnNames)[9].szItemData,"%s","ExportURL");
						sprintf((*pColumnNames)[10].szItemData,"%s","SourceUrl");
						
						// CellsData,二维数组
						pCellsData = new ITEMDATA* [*iRow];
						for ( j = 0; j < (*iRow); j ++)
						{
							pCellsData[j] = new ITEMDATA[(*iCol)];
						}
		
						for( i=0;i<(*iRow);i++)
						{
							for( j=0;j<(*iCol);j++)
							{
								(pCellsData[i][j]).szItemData = (char*)malloc(ITEMLEN);
							}
							
							strContentName = contentDes[i];
							TianShanIce::Storage::ContentPrx contentClient  = contentstoreClient->openContent(strContentName,strContentType,TRUE);
							if ( contentClient)
							{


	
								sprintf((pCellsData[i][0]).szItemData,"%s",strContentName.c_str());

								try
								{
									b1 = contentClient->isProvisioned();
								}
								catch( const ::Ice::Exception &ex)
								{
									b1 =  false;
								}
								catch(...)
								{
									b1 =  false;
								}
								if ( b1 )
								{
									sprintf((pCellsData[i][1]).szItemData,"%s","true");
								}
								else
								{
									sprintf((pCellsData[i][1]).szItemData,"%s","false");
								}
								
								try
								{
									strTemp = contentClient->getProvisionTime();
									
								}
								catch( const ::Ice::Exception &ex)
								{
									strTemp ="";
								}
								catch(...)
								{
									strTemp ="";
								}
								sprintf((pCellsData[i][2]).szItemData,"%s",strTemp.c_str());

								try
								{
									strTemp = contentClient->getLocaltype();
								}
								catch( const ::Ice::Exception &ex)
								{
									strTemp ="";
								}
								catch(...)
								{
									strTemp ="";
								}
								sprintf((pCellsData[i][3]).szItemData,"%s",strTemp.c_str());

								
								try
								{
									fFrameRate = contentClient->getFramerate();
								}
								catch( const ::Ice::Exception &ex)
								{
									fFrameRate = 0.00;
								}
								catch(...)
								{
									fFrameRate = 0.00;
								}
								sprintf((pCellsData[i][4]).szItemData,"%f",fFrameRate);

								
								try
								{
									lFileSize = (long)contentClient->getFilesize();
								}
								catch( const ::Ice::Exception &ex)
								{
									lFileSize = 0;
								}
								catch(...)
								{
									lFileSize = 0;
								}
								sprintf((pCellsData[i][5]).szItemData,"%d",lFileSize);

								try
								{
									lFileSize = (long)contentClient->getPlayTime();
								}
								catch( const ::Ice::Exception &ex)
								{
									lFileSize = 0;
								}
								catch(...)
								{
									lFileSize = 0;
								}
								sprintf((pCellsData[i][6]).szItemData,"%d",lFileSize);

								int iBitRate;
								try
								{
									iBitRate = contentClient->getBitRate();
								}
								catch( const ::Ice::Exception &ex)
								{
									iBitRate = 0;
								}
								catch(...)
								{
									iBitRate = 0;
								}
								sprintf((pCellsData[i][7]).szItemData,"%d",iBitRate);

								try
								{
									strTemp = contentClient->getMD5Checksum();
								}
								catch( const ::Ice::Exception &ex)
								{
									strTemp ="";
								}
								
								catch(...)
								{
									strTemp ="";
								}
								sprintf((pCellsData[i][8]).szItemData,"%s",strTemp.c_str());

								try
								{
									strTemp = contentClient->getExportURL();
								}
								catch( const ::Ice::Exception &ex)
								{
									strTemp ="";
								}
								
								catch(...)
								{
									strTemp ="";
								}
								sprintf((pCellsData[i][9]).szItemData,"%s",strTemp.c_str());

								try
								{
									strTemp = contentClient->getSourceUrl();
								}
								catch( const ::Ice::Exception &ex)
								{
									strTemp ="";
								}
								catch(...)
								{
									strTemp ="";
								}
								sprintf((pCellsData[i][10]).szItemData,"%s",strTemp.c_str());

								if ( i != (*iRow -1 ) )
								{
									Sleep(1000); // 为了不影响ContentStore服务的效率，这里必须要有1秒钟的延迟时间。
								}
								// add 20070423 for callback proc
								if ( proc)
								{
									(*proc)((*iRow),(i+1));
								}
								// add 20070423 for callback proc

							}
						}

						// Attribes's Data,the Attribe's Data is empty
						AttribsData.clear();

				       /* below is the old code for the old mode
						
						OneRowData.clear();
						RowDataArray.clear();
						
						// for the contentstore's column names
						strColumnName="Name";
						OneRowData.push_back(strColumnName);

						strColumnName="IsProvisioned";
						OneRowData.push_back(strColumnName);

						strColumnName="ProvisionTime";
						OneRowData.push_back(strColumnName);

						strColumnName="ContentType";
						OneRowData.push_back(strColumnName);

						strColumnName="Framerate";
						OneRowData.push_back(strColumnName);

						strColumnName="FileSize";
						OneRowData.push_back(strColumnName);

						strColumnName="PlayTime";
						OneRowData.push_back(strColumnName);

						strColumnName="BitRate";
						OneRowData.push_back(strColumnName);

						strColumnName="DestinationURL";
						OneRowData.push_back(strColumnName);

						strColumnName="SourceURL";
						OneRowData.push_back(strColumnName);

						RowDataArray[0]=OneRowData;	
//						iSize = RowDataArray.size();

						// for the content's cells data
						for ( i = 0; i < iContentStoreNum; i ++)
						{
							strTemp = contentDes[i];
							TianShanIce::Storage::ContentPrx contentClient  = contentstoreClient->openContent(strTemp,strContentType,TRUE);

							if ( contentClient )
							{

								OneRowData.clear();
								strColumnName = contentClient->getName();
								OneRowData.push_back(strColumnName);
																							
								bool b1;
								try
								{
									b1 = contentClient->isProvisioned();
								}
								catch( const ::Ice::Exception &ex)
								{
									b1 =  false;
								}
								catch(...)
								{
									b1 =  false;
								}
								if ( b1 )
								{
									strColumnName = "true";
								}
								else
								{
									strColumnName = "false";
								}
								OneRowData.push_back(strColumnName);

								try
								{
									strColumnName = contentClient->getProvisionTime();
									
								}
								catch( const ::Ice::Exception &ex)
								{
									strColumnName ="";
								}
								catch(...)
								{
									strColumnName ="";
								}
								OneRowData.push_back(strColumnName);

								try
								{
									strColumnName = contentClient->getLocaltype();
								}
								catch( const ::Ice::Exception &ex)
								{
									strColumnName ="";
								}
								catch(...)
								{
									strColumnName ="";
								}
								OneRowData.push_back(strColumnName);

								float fFrameRate;
								try
								{
									fFrameRate = contentClient->getFramerate();
								}
								catch( const ::Ice::Exception &ex)
								{
									fFrameRate = 0.00;
								}
								catch(...)
								{
									fFrameRate = 0.00;
								}
								memset(sTemp,0,sizeof(sTemp));
								sprintf(sTemp,"%f",fFrameRate);
								strColumnName = sTemp;
								OneRowData.push_back(strColumnName);

								long lFileSize;
								try
								{
									lFileSize = (long)contentClient->getFilesize();
								}
								catch( const ::Ice::Exception &ex)
								{
									lFileSize = 0;
								}
								catch(...)
								{
									lFileSize = 0;
								}
								memset(sTemp,0,sizeof(sTemp));
								sprintf(sTemp,"%d",lFileSize);
								strColumnName = sTemp;
								OneRowData.push_back(strColumnName);


								try
								{
									lFileSize = (long)contentClient->getPlayTime();
								}
								catch( const ::Ice::Exception &ex)
								{
									lFileSize = 0;
								}
								catch(...)
								{
									lFileSize = 0;
								}
								memset(sTemp,0,sizeof(sTemp));
								sprintf(sTemp,"%d",lFileSize);
								strColumnName = sTemp;
								OneRowData.push_back(strColumnName);

								int iBitRate;
								try
								{
									iBitRate = contentClient->getBitRate();
								}
								catch( const ::Ice::Exception &ex)
								{
									iBitRate = 0;
								}
								catch(...)
								{
									iBitRate = 0;
								}
								memset(sTemp,0,sizeof(sTemp));
								sprintf(sTemp,"%d",iBitRate);
								strColumnName = sTemp;
								OneRowData.push_back(strColumnName);

							
								try
								{
									strColumnName = contentClient->getExportURL();
								}
								catch( const ::Ice::Exception &ex)
								{
									strColumnName ="";
								}
								catch(...)
								{
									strColumnName ="";
								}
								OneRowData.push_back(strColumnName);
								

								try
								{
									strColumnName = contentClient->getSourceUrl();
								}
								catch( const ::Ice::Exception &ex)
								{
									strColumnName ="";
								}
								catch(...)
								{
									strColumnName ="";
								}
								OneRowData.push_back(strColumnName);
								RowDataArray[1]=OneRowData;		
								//	iSize = OneRowData.size();
	//							iSize = RowDataArray.size();
								AttribsData[strTemp]=RowDataArray;
	//							iSize = AttribsData.size();

								Sleep(1000); // 为了不影响ContentStore服务的效率，这里必须要有1秒钟的延迟时间。
							}
							else
							{
								// Attribes's Data,if there are not datas
								AttribsData.clear();
								break;
							}
						}
						*/
					}
					else
					{
						AttribsData.clear();
						pColumnNames = NULL;
						pCellsData   = NULL;
						*iRow = 0;
						*iCol = 0;
					}
				}
			}
		}
		
		else if ( _stricmp(m_strServiceName.c_str(),"SiteAdminSvc") == 0) // SiteAdminSvc
		{
			
			long lMaxBW;
			int iMaxSession;
			int iSiteNum,iAppNum;
			TianShanIce::Site::SiteAdminPrx siteAdminClient = NULL;
			
			

			strEndPoint = "BusinessRouter:";
			strEndPoint +=m_strServiceEndPoint;
			iSiteNum = iAppNum = 0;
			if (m_Communicator)
			{
				siteAdminClient = TianShanIce::Site::SiteAdminPrx::checkedCast(m_Communicator->stringToProxy(strEndPoint));
				if ( siteAdminClient)
				{
					if ( _stricmp(cTabname,"Site") == 0 ) // Site Tab
					{
						TianShanIce::Site::VirtualSites SiteData = siteAdminClient->listSites();
						iSiteNum = SiteData.size();
						if ( iSiteNum > 0 )
						{
							*iRow = iSiteNum;				
							*iCol = 5;
							
							// ColumnNames's Data
							*pColumnNames = new ITEMDATA[*iCol];
							for (  i = 0; i < (*iCol); i ++ )
							{
								(*pColumnNames)[i].szItemData = (char*)malloc(ITEMLEN);
							}
							sprintf((*pColumnNames)[0].szItemData,"%s","name");
							sprintf((*pColumnNames)[1].szItemData,"%s","des");
							sprintf((*pColumnNames)[2].szItemData,"%s","maxDownstreamBwKbps");
							sprintf((*pColumnNames)[3].szItemData,"%s","maxSessions");
							sprintf((*pColumnNames)[4].szItemData,"%s","pubKey");
												
							// CellsData,二维数组
							pCellsData = new ITEMDATA* [*iRow];
							for ( j = 0; j < (*iRow); j ++)
							{
								pCellsData[j] = new ITEMDATA[(*iCol)];
							}
							
							for( i=0;i<(*iRow);i++)
							{
								for( j=0;j<(*iCol);j++)
								{
									(pCellsData[i][j]).szItemData = (char*)malloc(ITEMLEN);
								}
								
								strTemp = SiteData[i].name;
								sprintf((pCellsData[i][0]).szItemData,"%s",strTemp.c_str());

								strTemp = SiteData[i].desc;
								sprintf((pCellsData[i][1]).szItemData,"%s",strTemp.c_str());

								lMaxBW = (long)SiteData[i].maxDownstreamBwKbps;
								memset(sTemp,0,sizeof(sTemp));
								ltoa(lMaxBW,sTemp,10);
								strTemp = sTemp;
								sprintf((pCellsData[i][2]).szItemData,"%s",strTemp.c_str());

								iMaxSession = SiteData[i].maxSessions;
								memset(sTemp,0,sizeof(sTemp));
								itoa(iMaxSession,sTemp,10);
								strTemp = sTemp;
								sprintf((pCellsData[i][3]).szItemData,"%s",strTemp.c_str());

								strTemp = SiteData[i].pubKey;
								sprintf((pCellsData[i][4]).szItemData,"%s",strTemp.c_str());
							}

							// Attribes's Datas
							OneRowData.clear();
							RowDataArray.clear();
							
							// for the storeage's column names
							strColumnName="siteName";
							OneRowData.push_back(strColumnName);

							strColumnName="mountedPath";
							OneRowData.push_back(strColumnName);

							strColumnName="appName";
							OneRowData.push_back(strColumnName);

							RowDataArray[0]=OneRowData;	
//							iSize = RowDataArray.size();

							// for the site's cells data
							for ( i = 0; i < iSiteNum; i ++)
							{
								strTemp = SiteData[i].name;
								TianShanIce::Site::AppMounts mounts =  siteAdminClient->listMounts(strTemp);
								iSize = mounts.size();
								if ( iSize == 0 )
								{
									AttribsData.clear();
									break;
								}

								m = 1;
								for (TianShanIce::Site::AppMounts::iterator it = mounts.begin(); it != mounts.end(); it++)
								{
																	
									OneRowData.clear();
//									strColumnName =(*it)->siteName;
									strColumnName = strTemp;
									OneRowData.push_back(strColumnName);
									
																
									strColumnName =(*it)->getMountedPath();
									OneRowData.push_back(strColumnName);

									strColumnName = (*it)->getAppName();
									OneRowData.push_back(strColumnName);

									RowDataArray[m]=OneRowData;		
									m++;
								}
//								iSize = OneRowData.size();
//								iSize = RowDataArray.size();
								AttribsData[strTemp]=RowDataArray;
//								iSize = AttribsData.size();
							}
						}
						else
						{
							AttribsData.clear();
							pColumnNames = NULL;
							pCellsData   = NULL;
							*iRow = 0;
							*iCol = 0;
						}
					}
					else if ( _stricmp(cTabname,"App") == 0 ) // App Tab
					{
						TianShanIce::Site::AppInfos AppData = siteAdminClient->listApplications();
						iAppNum = AppData.size();
						if ( iAppNum > 0 )
						{
							*iRow = iAppNum;				
							*iCol = 3;
							
							// ColumnNames's Data
							*pColumnNames = new ITEMDATA[*iCol];
							for (  i = 0; i < (*iCol); i ++ )
							{
								(*pColumnNames)[i].szItemData = (char*)malloc(ITEMLEN);
							}
							sprintf((*pColumnNames)[0].szItemData,"%s","name");
							sprintf((*pColumnNames)[1].szItemData,"%s","endpoint");
							sprintf((*pColumnNames)[2].szItemData,"%s","description");
							sprintf((*pColumnNames)[3].szItemData,"%s","maxSessions");
							
							// CellsData,二维数组
							pCellsData = new ITEMDATA* [*iRow];
							for ( j = 0; j < (*iRow); j ++)
							{
								pCellsData[j] = new ITEMDATA[(*iCol)];
							}
							
							for( i=0;i<(*iRow);i++)
							{
								for( j=0;j<(*iCol);j++)
								{
									(pCellsData[i][j]).szItemData = (char*)malloc(ITEMLEN);
								}
								
								strTemp = AppData[i].name;
								sprintf((pCellsData[i][0]).szItemData,"%s",strTemp.c_str());

								strTemp = AppData[i].endpoint;
								sprintf((pCellsData[i][1]).szItemData,"%s",strTemp.c_str());

								strTemp = AppData[i].desc;
								sprintf((pCellsData[i][2]).szItemData,"%s",strTemp.c_str());
							}

							// Attribes's Data
							AttribsData.clear();
						}
						else
						{
							AttribsData.clear();
							pColumnNames = NULL;
							pCellsData   = NULL;
							*iRow = 0;
							*iCol = 0;
						}
					}
					else if ( _stricmp(cTabname,"LiveTxn") == 0 ) // LiveTxn Tab
					{
						AttribsData.clear();
						pColumnNames = NULL;
						pCellsData   = NULL;
						*iRow = 0;
						*iCol = 0;
					}
				}
				else
				{
					AttribsData.clear();
					pColumnNames = NULL;
					pCellsData   = NULL;
					*iRow = 0;
					*iCol = 0;
				}
			}
		}
		else if ( _stricmp(m_strServiceName.c_str(),"Weiwoo") == 0 ) // Weiwoo
		{
			int k;
			int iStorageNum,iStreamerNum,iSGroupNum;
			TianShanIce::Transport::PathAdminPrx pathAdminClient = NULL;
			string strType;
			char strText[150]={0};
			string strKey;
			TianShanIce::ValueMap mPvals;
			TianShanIce::ValueMap::iterator mPvalsItor;
			TianShanIce::PDSchema mschema;
			STRVECTOR OneRowData1;
			

			strncpy(strText,cTabname,8);
			strEndPoint = "PathManager:";
			strEndPoint +=m_strServiceEndPoint;
			iStorageNum = iStreamerNum = iSGroupNum = 0;
									
			if (m_Communicator)
			{
				pathAdminClient = TianShanIce::Transport::PathAdminPrx::checkedCast(m_Communicator->stringToProxy(strEndPoint));

				if ( pathAdminClient)
				{
					if ( _stricmp(cTabname,"Storage") == 0 ) // Storage Tab
					{
						TianShanIce::Transport::Storages storages = pathAdminClient->listStorages();
						iStorageNum = storages.size();
						
						if ( iStorageNum > 0 )
						{
							*iRow = iStorageNum;				
							*iCol = 4;
							
							// ColumnNames's Data
							*pColumnNames = new ITEMDATA[*iCol];
							for (  i = 0; i < (*iCol); i ++ )
							{
								(*pColumnNames)[i].szItemData = (char*)malloc(ITEMLEN);
							}
							sprintf((*pColumnNames)[0].szItemData,"%s","NetId");
							sprintf((*pColumnNames)[1].szItemData,"%s","Type");
							sprintf((*pColumnNames)[2].szItemData,"%s","Description");
							sprintf((*pColumnNames)[3].szItemData,"%s","Ifep");
												
							// CellsData,二维数组
							pCellsData = new ITEMDATA* [*iRow];
							for ( j = 0; j < (*iRow); j ++)
							{
								pCellsData[j] = new ITEMDATA[(*iCol)];
							}
							
							for( i=0;i<(*iRow);i++)
							{
								for( j=0;j<(*iCol);j++)
								{
									(pCellsData[i][j]).szItemData = (char*)malloc(ITEMLEN);
								}
								
								strTemp = storages[i].netId;
								sprintf((pCellsData[i][0]).szItemData,"%s",strTemp.c_str());
								strTemp = storages[i].type;
								sprintf((pCellsData[i][1]).szItemData,"%s",strTemp.c_str());
								strTemp = storages[i].desc;
								sprintf((pCellsData[i][2]).szItemData,"%s",strTemp.c_str());
								strTemp = storages[i].ifep;
								sprintf((pCellsData[i][3]).szItemData,"%s",strTemp.c_str());
							}

							
							// Attribs's Data
							// for the storage's storagelink  data
							for ( i = 0; i < iStorageNum; i ++)
							{
								// Attribes data
								OneRowData.clear();
								OneRowData1.clear();
								RowDataArray.clear();
								
								// for the storeage's column names
								strColumnName="Ident";
								OneRowData1.push_back(strColumnName);

								strColumnName="Type";
								OneRowData1.push_back(strColumnName);

								strColumnName="StorageId";
								OneRowData1.push_back(strColumnName);

								strColumnName="StreamerId";
								OneRowData1.push_back(strColumnName);

								strTemp = storages[i].netId;
								TianShanIce::Transport::StorageLinks storagelinks = pathAdminClient->listStorageLinksByStorage(strTemp);
								
								iSize = storagelinks.size();
								if ( iSize == 0)
								{
									// Attribes's Data,if there are not datas
									AttribsData.clear();
									break;
								}

								m = 1;
								for (TianShanIce::Transport::StorageLinks::iterator it = storagelinks.begin(); it < storagelinks.end(); it++)
								{
									strType = (*it)->getType();
									
								    if ( it == storagelinks.begin() )
									{
										mPvals = (*it)->getPrivateData();
										mschema = pathAdminClient->getStorageLinkSchema(strType);
										
//										iSize = mPvals.size();
										for( k = 0; k < (int)mPvals.size(); k++)
										{
											mPvalsItor =  mPvals.find(mschema[k].keyname);
											memset(strText,0,sizeof(strText));
											if(!GetPrivateDataStr(strText,&(mPvalsItor->second)))
											{
												sprintf(strText,"%s","");										 
											} 
											strKey =mPvalsItor->first;
											strColumnName=strKey;
											OneRowData1.push_back(strColumnName);
								 		}
										RowDataArray[0]=OneRowData1;	
									}

									OneRowData.clear();
																
									strColumnName =(*it)->getIdent().name;
									OneRowData.push_back(strColumnName);

									OneRowData.push_back(strType);

									strColumnName = (*it)->getStorageId();
									OneRowData.push_back(strColumnName);

									strColumnName = (*it)->getStreamerId();
									OneRowData.push_back(strColumnName);

									mPvals = (*it)->getPrivateData();
									mschema = pathAdminClient->getStorageLinkSchema(strType);

									for( k = 0; k < (int)mPvals.size(); k++)
									{
										mPvalsItor =  mPvals.find(mschema[k].keyname);
										memset(strText,0,sizeof(strText));
										if(!GetPrivateDataStr(strText,&(mPvalsItor->second)))
										{
											sprintf(strText,"%s","");										 
										} 
										strColumnName=strText;
										OneRowData.push_back(strColumnName);
								 	}
									RowDataArray[m]=OneRowData;		
									m++;
								}
								iSize = OneRowData.size();
//								iSize = RowDataArray.size();

								AttribsData[strTemp]=RowDataArray;
//								iSize = AttribsData.size();
							}
						}
						else
						{
							AttribsData.clear();
							pColumnNames = NULL;
							pCellsData   = NULL;
							*iRow = 0;
							*iCol = 0;
						}
					}
					
					else if ( _stricmp(strText,"Streamer") == 0 ) // Streamer Tab for StreamLinks and StorageLinks
					{
						TianShanIce::Transport::Streamers streamers = pathAdminClient->listStreamers();
						iStreamerNum = streamers.size();


						if ( iStreamerNum > 0 )
						{
							*iRow = iStreamerNum;				
							*iCol = 4;
							
							// ColumnNames's Data
							*pColumnNames = new ITEMDATA[*iCol];
							for (  i = 0; i < (*iCol); i ++ )
							{
								(*pColumnNames)[i].szItemData = (char*)malloc(ITEMLEN);
							}
							sprintf((*pColumnNames)[0].szItemData,"%s","NetId");
							sprintf((*pColumnNames)[1].szItemData,"%s","Type");
							sprintf((*pColumnNames)[2].szItemData,"%s","Description");
							sprintf((*pColumnNames)[3].szItemData,"%s","Ifep");
												
							// CellsData,二维数组
							pCellsData = new ITEMDATA* [*iRow];
							for ( j = 0; j < (*iRow); j ++)
							{
								pCellsData[j] = new ITEMDATA[(*iCol)];
							}
							
							for( i=0;i<(*iRow);i++)
							{
								for( j=0;j<(*iCol);j++)
								{
									(pCellsData[i][j]).szItemData = (char*)malloc(ITEMLEN);
								}
								
								strTemp = streamers[i].netId;
								sprintf((pCellsData[i][0]).szItemData,"%s",strTemp.c_str());

								strTemp = streamers[i].type;
								sprintf((pCellsData[i][1]).szItemData,"%s",strTemp.c_str());

								strTemp = streamers[i].desc;
								sprintf((pCellsData[i][2]).szItemData,"%s",strTemp.c_str());

								strTemp = streamers[i].ifep;
								sprintf((pCellsData[i][3]).szItemData,"%s",strTemp.c_str());
							}

							// Attribes data for streamlink
							if (cTabname[8] =='1' ) // for the streamlinks
							{
								// for the streamlink's cells data
								for ( i = 0; i < iStreamerNum; i ++)
								{
									OneRowData.clear();
									OneRowData1.clear();
									RowDataArray.clear();

									strColumnName="Ident";
									OneRowData1.push_back(strColumnName);

									strColumnName="Type";
									OneRowData1.push_back(strColumnName);

									strColumnName="StreamerId";
									OneRowData1.push_back(strColumnName);

									strColumnName="ServiceGroupId";
									OneRowData1.push_back(strColumnName);

									strTemp = streamers[i].netId;
									TianShanIce::Transport::StreamLinks streamlinks = pathAdminClient->listStreamLinksByStreamer(strTemp);

									iSize = streamlinks.size();
									if ( iSize == 0)
									{
										// Attribes's Data,if there are not datas
										AttribsData.clear();
										break;
									}
								
									m = 1;
									for (TianShanIce::Transport::StreamLinks::iterator it = streamlinks.begin(); it < streamlinks.end(); it++)
									{
										strType = (*it)->getType();
										
										if ( it == streamlinks.begin() )
										{
											
											mPvals = (*it)->getPrivateData();
											mschema = pathAdminClient->getStreamLinkSchema(strType);
											
//											iSize = mPvals.size();
											for( k = 0; k < (int)mPvals.size(); k++)
											{
												mPvalsItor =  mPvals.find(mschema[k].keyname);
												memset(strText,0,sizeof(strText));
												if(!GetPrivateDataStr(strText,&(mPvalsItor->second)))
												{
													sprintf(strText,"%s","");										 
												} 
												strKey =mPvalsItor->first;
												strColumnName=strKey;
												OneRowData1.push_back(strColumnName);
								 			}
											RowDataArray[0]=OneRowData1;	
										}

										OneRowData.clear();
																	
										strColumnName =(*it)->getIdent().name;
										OneRowData.push_back(strColumnName);

										OneRowData.push_back(strType);
										
										strColumnName = (*it)->getStreamerId();
										OneRowData.push_back(strColumnName);

										memset(sTemp,0,sizeof(sTemp));
										itoa((*it)->getServiceGroupId(),sTemp,10);
										strColumnName = sTemp;
										OneRowData.push_back(strColumnName);

										mPvals = (*it)->getPrivateData();
										mschema = pathAdminClient->getStreamLinkSchema(strType);

										for( k = 0; k < (int)mPvals.size(); k++)
										{
											mPvalsItor =  mPvals.find(mschema[k].keyname);
											memset(strText,0,sizeof(strText));
											if(!GetPrivateDataStr(strText,&(mPvalsItor->second)))
											{
												sprintf(strText,"%s","");										 
											} 
											strColumnName=strText;
											OneRowData.push_back(strColumnName);
								 		}
										RowDataArray[m]=OneRowData;		
										m++;
									}
//									iSize = OneRowData.size();
//									iSize = RowDataArray.size();

									AttribsData[strTemp]=RowDataArray;
//									iSize = AttribsData.size();
								}
							}
							else // for the storagelinks
							{
								// for the storagelink's cells data
								for ( i = 0; i < iStreamerNum; i ++)
								{
									OneRowData.clear();
									OneRowData1.clear();
									RowDataArray.clear();

									strColumnName="Ident";
									OneRowData1.push_back(strColumnName);

									strColumnName="Type";
									OneRowData1.push_back(strColumnName);

									strColumnName="StorageId";
									OneRowData1.push_back(strColumnName);

									strColumnName="StreamerId";
									OneRowData1.push_back(strColumnName);

									strTemp = streamers[i].netId;
									TianShanIce::Transport::StorageLinks streamstoragelinks = pathAdminClient->listStorageLinksByStreamer(strTemp);

									iSize = streamstoragelinks.size();
									if ( iSize == 0)
									{
										// Attribes's Data,if there are not datas
										AttribsData.clear();
										break;
									}


									m = 1;
									for (TianShanIce::Transport::StorageLinks::iterator it = streamstoragelinks.begin(); it < streamstoragelinks.end(); it++)
									{
										strType = (*it)->getType();

										if ( it == streamstoragelinks.begin() )
										{
											mPvals = (*it)->getPrivateData();
											mschema = pathAdminClient->getStorageLinkSchema(strType);
										
//											iSize = mPvals.size();
											for( k = 0; k < (int)mPvals.size(); k++)
											{
												mPvalsItor =  mPvals.find(mschema[k].keyname);
												memset(strText,0,sizeof(strText));
												if(!GetPrivateDataStr(strText,&(mPvalsItor->second)))
												{
													sprintf(strText,"%s","");										 
												} 
												strKey =mPvalsItor->first;
												strColumnName=strKey;
												OneRowData1.push_back(strColumnName);
								 			}
											RowDataArray[0]=OneRowData1;	
											
										}

										OneRowData.clear();
																	
										strColumnName =(*it)->getIdent().name;
										OneRowData.push_back(strColumnName);

										OneRowData.push_back(strType);

										strColumnName =(*it)->getStorageId();
										OneRowData.push_back(strColumnName);
										
										strColumnName = (*it)->getStreamerId();
										OneRowData.push_back(strColumnName);

										
										mPvals = (*it)->getPrivateData();
										mschema = pathAdminClient->getStorageLinkSchema(strType);

										for( k = 0; k < (int)mPvals.size(); k++)
										{
											mPvalsItor =  mPvals.find(mschema[k].keyname);
											memset(strText,0,sizeof(strText));
											if(!GetPrivateDataStr(strText,&(mPvalsItor->second)))
											{
												sprintf(strText,"%s","");										 
											} 
											strColumnName=strText;
											OneRowData.push_back(strColumnName);
								 		}
										RowDataArray[m]=OneRowData;		
										m++;
									}
//									iSize = OneRowData.size();
//									iSize = RowDataArray.size();

									AttribsData[strTemp]=RowDataArray;
//									iSize = AttribsData.size();
								}

							}
						}
						else
						{
							AttribsData.clear();
							pColumnNames = NULL;
							pCellsData   = NULL;
							*iRow = 0;
							*iCol = 0;
						}
					}
					else if ( _stricmp(cTabname,"ServiceGroup") == 0 ) // ServiceGroup Tab
					{
						TianShanIce::Transport::ServiceGroups serviestore = pathAdminClient->listServiceGroups();
						iSGroupNum = serviestore.size();

						if ( iSGroupNum > 0 )
						{
							*iRow = iSGroupNum;				
							*iCol = 3;
							
							// ColumnNames's Data
							*pColumnNames = new ITEMDATA[*iCol];
							for (  i = 0; i < (*iCol); i ++ )
							{
								(*pColumnNames)[i].szItemData = (char*)malloc(ITEMLEN);
							}
							sprintf((*pColumnNames)[0].szItemData,"%s","Id");
							sprintf((*pColumnNames)[1].szItemData,"%s","Type");
							sprintf((*pColumnNames)[2].szItemData,"%s","Description");
																			
							// CellsData,二维数组
							pCellsData = new ITEMDATA* [*iRow];
							for ( j = 0; j < (*iRow); j ++)
							{
								pCellsData[j] = new ITEMDATA[(*iCol)];
							}
							
							for( i=0;i<(*iRow);i++)
							{
								for( j=0;j<(*iCol);j++)
								{
									(pCellsData[i][j]).szItemData = (char*)malloc(ITEMLEN);
								}
								memset(sTemp,0,sizeof(sTemp));
								itoa(serviestore[i].id,sTemp,10);
								strTemp = sTemp;
								sprintf((pCellsData[i][0]).szItemData,"%s",strTemp.c_str());

								strTemp = serviestore[i].type;
								sprintf((pCellsData[i][1]).szItemData,"%s",strTemp.c_str());

								strTemp = serviestore[i].desc;
								sprintf((pCellsData[i][2]).szItemData,"%s",strTemp.c_str());
							}

													

							// for the servicegroup's cells data
							for ( i = 0; i < iSGroupNum; i ++)
							{

								// Attribes data for streamlink
								OneRowData.clear();
								OneRowData1.clear();
								RowDataArray.clear();
								
								// for the storeage's column names
								strColumnName="Ident";
								OneRowData1.push_back(strColumnName);

								strColumnName="Type";
								OneRowData1.push_back(strColumnName);

								strColumnName="StreamerId";
								OneRowData1.push_back(strColumnName);

								strColumnName="ServiceGroupId";
								OneRowData1.push_back(strColumnName);

								memset(sTemp,0,sizeof(sTemp));
								itoa(serviestore[i].id,sTemp,10);
								strTemp = sTemp;

								TianShanIce::Transport::StreamLinks servicegrouplinks = pathAdminClient->listStreamLinksByServiceGroup(serviestore[i].id);

								iSize = servicegrouplinks.size();
								if ( iSize == 0)
								{
									// Attribes's Data,if there are not datas
									AttribsData.clear();
									break;
								}
							
								m = 1;
								for (TianShanIce::Transport::StreamLinks::iterator it = servicegrouplinks.begin(); it < servicegrouplinks.end(); it++)
								{
									strType = (*it)->getType();

								    if ( it == servicegrouplinks.begin() )
									{
										
										mPvals = (*it)->getPrivateData();
										mschema = pathAdminClient->getStreamLinkSchema(strType);
										
//										iSize = mPvals.size();
										for( k = 0; k < (int)mPvals.size(); k++)
										{
											mPvalsItor =  mPvals.find(mschema[k].keyname);
											memset(strText,0,sizeof(strText));
											if(!GetPrivateDataStr(strText,&(mPvalsItor->second)))
											{
												sprintf(strText,"%s","");										 
											} 
											strKey =mPvalsItor->first;
											strColumnName=strKey;
											OneRowData1.push_back(strColumnName);
								 		}
										RowDataArray[0]=OneRowData1;	
									}

									OneRowData.clear();
																
									strColumnName =(*it)->getIdent().name;
									OneRowData.push_back(strColumnName);

									OneRowData.push_back(strType);
									
									strColumnName = (*it)->getStreamerId();
									OneRowData.push_back(strColumnName);


									memset(sTemp,0,sizeof(sTemp));
									itoa((*it)->getServiceGroupId(),sTemp,10);
									strColumnName = sTemp;
									OneRowData.push_back(strColumnName);

									mPvals = (*it)->getPrivateData();
									mschema = pathAdminClient->getStreamLinkSchema(strType);

									for( k = 0; k < (int)mPvals.size(); k++)
									{
										mPvalsItor =  mPvals.find(mschema[k].keyname);
										memset(strText,0,sizeof(strText));
										if(!GetPrivateDataStr(strText,&(mPvalsItor->second)))
										{
											sprintf(strText,"%s","");										 
										} 
										strColumnName=strText;
										OneRowData.push_back(strColumnName);
								 	}
									RowDataArray[m]=OneRowData;		
									m++;
								}
//								iSize = OneRowData.size();
//								iSize = RowDataArray.size();

								AttribsData[strTemp]=RowDataArray;
//								iSize = AttribsData.size();
							}
						}
						else
						{
							AttribsData.clear();
							pColumnNames = NULL;
							pCellsData   = NULL;
							*iRow = 0;
							*iCol = 0;
						}
					}

					/*
					
					// for the application 's column names
					OneRowData.clear();
					RowDataArray.clear();
					
					strColumnName="AppName";
					OneRowData.push_back(strColumnName);
					strColumnName="EndPoint";
					OneRowData.push_back(strColumnName);
					
					strColumnName="Description";
					OneRowData.push_back(strColumnName);
					
					
					RowDataArray[0]=OneRowData;	
					iSize = RowDataArray.size();

					for ( i = 0; i < iAppNum; i ++)
					{
						OneRowData.clear();
						strTemp = AppData[i].name;
						strColumnName =AppData[i].name;
						OneRowData.push_back(strColumnName);

						strColumnName =AppData[i].endpoint;
						OneRowData.push_back(strColumnName);

						strColumnName =AppData[i].desc;
						OneRowData.push_back(strColumnName);

						RowDataArray[1]=OneRowData;		
											
						iSize = OneRowData.size();
						iSize = RowDataArray.size();
						AttribsData[strTemp]=RowDataArray;
						iSize = AttribsData.size();
					}
					*/
				}
				else
				{
					AttribsData.clear();
					pColumnNames = NULL;
					pCellsData   = NULL;
					*iRow = 0;
					*iCol = 0;
				}
			}
			else
			{
				AttribsData.clear();
				pColumnNames = NULL;
				pCellsData   = NULL;
				*iRow = 0;
				*iCol = 0;
			}
		}
	}
	catch( const ::Ice::Exception &ex)
	{
//		pAttribsData = NULL;
		AttribsData.clear();
		pColumnNames = NULL;
		pCellsData   = NULL;
//		*iAttCount =0;
		*iRow = 0;
		*iCol = 0;
	}
	catch(...)
	{
//		pAttribsData = NULL;
		AttribsData.clear();
		pColumnNames = NULL;
		pCellsData   = NULL;
//		*iAttCount =0;
		*iRow = 0;
		*iCol = 0;
	}
	return pCellsData;
}

#else

int  GetData_Proc(ATTRISVECTOR & attribsData,int *ColumnCount,STRVECTOR &ColumnNames, int *RowCount, GRIDDATAARRAY & CellsData)
{
	int iReturn = 0;
	*ColumnCount = 3;
	string strColunName ;

	ATTRISDATA attribesDataTmp;
	attribesDataTmp.strAttrName="AttirbsName1";
	attribesDataTmp.strAttrValue="Value1";
	attribsData.push_back(attribesDataTmp);

	attribesDataTmp.strAttrName="AttirbsName2";
	attribesDataTmp.strAttrValue="Value2";
	attribsData.push_back(attribesDataTmp);

	attribesDataTmp.strAttrName="AttirbsName3";
	attribesDataTmp.strAttrValue="Value3";
	attribsData.push_back(attribesDataTmp);

	attribesDataTmp.strAttrName="AttirbsName4";
	attribesDataTmp.strAttrValue="Value4";
	attribsData.push_back(attribesDataTmp);

	attribesDataTmp.strAttrName="AttirbsName5";
	attribesDataTmp.strAttrValue="Value5";
	attribsData.push_back(attribesDataTmp);


	attribesDataTmp.strAttrName="AttirbsName6";
	attribesDataTmp.strAttrValue="Value6";
	attribsData.push_back(attribesDataTmp);

	attribesDataTmp.strAttrName="AttirbsName7";
	attribesDataTmp.strAttrValue="Value7";
	attribsData.push_back(attribesDataTmp);

	attribesDataTmp.strAttrName="AttirbsName8";
	attribesDataTmp.strAttrValue="Value8";
	attribsData.push_back(attribesDataTmp);

	attribesDataTmp.strAttrName="AttirbsName9";
	attribesDataTmp.strAttrValue="Value9";
	attribsData.push_back(attribesDataTmp);

	attribesDataTmp.strAttrName="AttirbsName10";
	attribesDataTmp.strAttrValue="Value10";
	attribsData.push_back(attribesDataTmp);

	attribesDataTmp.strAttrName="AttirbsName11";
	attribesDataTmp.strAttrValue="Value11";
	attribsData.push_back(attribesDataTmp);


	strColunName ="No";
	ColumnNames.push_back(strColunName);

	strColunName ="ChannelName";
	ColumnNames.push_back(strColunName);


	strColunName ="ChannelSize";
	ColumnNames.push_back(strColunName);
	int isize = ColumnNames.size();
	
	*RowCount = 5;	
	string strRowData;
	char sRowNum[10]={0};
	
	for ( int i =0; i < (*RowCount); i++)
	{
		STRVECTOR OneRowData;
		for ( int j = 0; j < (*ColumnCount); j ++ )
		{
			if ( j == 0)
			{
				memset(sRowNum,0,sizeof(sRowNum));
				itoa(i,sRowNum,10);
				strRowData =sRowNum;
			}
			else
			{
				memset(sRowNum,0,sizeof(sRowNum));
				itoa(i,sRowNum,10);
				strRowData ="ColumnData";
				strRowData +=sRowNum;
				memset(sRowNum,0,sizeof(sRowNum));
				itoa(j,sRowNum,10);
				strRowData +=sRowNum;
			}
			OneRowData.push_back(strRowData);
		}
		CellsData[i]=OneRowData;		
	}
	if ( (ColumnNames.empty()) && (CellsData.empty()) )
	{
		iReturn = -1;
	}
	return iReturn;
}
#endif
	
int  OnEvent_Proc( const EVENTATTRISDATA & attribeData,RegEvent_Proc proc)
{

#ifndef DEMOVER
	try
	{
	
		string strIceStormPoint = std::string(SERVICE_NAME_TopicManager ":") ;
		strIceStormPoint += attribeData.strIceStormEndPoint.c_str();
			
		TianShanIce::Events::EventChannelImpl::Ptr eventChannel = NULL;
		
		int i =0;

		Ice::ObjectAdapterPtr adapter= m_Communicator->createObjectAdapterWithEndpoints("Events.Subscriber","tcp -p 10001");
		eventChannel = new TianShanIce::Events::EventChannelImpl(adapter,strIceStormPoint.c_str(),true);
		if ( !eventChannel)
		{
			printf("StreamOnProgressEvent failed\n");
			return -1;
		}
		bool bRet = false;
		// 蒱获所有的Event Source
		if ( _stricmp(m_strServiceName.c_str(),"StreamSmith") == 0 )
		{
			// StreamProgressEventSink
			TianShanIce::Streamer::StreamProgressSinkPtr progressSink = new StreamProgressSinkImpl(proc);
			bRet =eventChannel->sink(progressSink);

			// StreamEventSink
			TianShanIce::Streamer::StreamEventSinkPtr streamEventsink = new StreamEventSinkImpl(proc);
			TianShanIce::Properties qos;
			bRet = eventChannel->sink(streamEventsink, qos);

			// PlaylistEventSink
			TianShanIce::Streamer::PlaylistEventSinkPtr playlistEventsink = new PlaylistEventSinkImpl(proc);
//			TianShanIce::Properties qos1;
			bRet = eventChannel->sink(playlistEventsink, qos);
		}
		else if (  (_stricmp(m_strServiceName.c_str(),"ClusterContentStore") == 0 ) ||  (_stricmp(m_strServiceName.c_str(),"NodeContentStore") == 0 )  ) // ContentStore
		{
			// ProvisionProgressEventSink
			TianShanIce::Storage::ProvisionProgressSinkPtr ProvisionprogressSink = new ProvisionProgressImpl(proc);
			TianShanIce::Properties qos;
			bRet = eventChannel->sink(ProvisionprogressSink,qos);


			// ProvisonStateEventSink
			TianShanIce::Storage::ProvisionStateChangeSinkPtr ProvisionStateSink = new ProvisionStateChangeImpl(proc);
			TianShanIce::Properties qos1;
			bRet = eventChannel->sink(ProvisionStateSink,qos1);
		}
		else if ( _stricmp(m_strServiceName.c_str(),"Weiwoo") == 0 )
		{
			// SessionEventSink
			TianShanIce::SRM::SessionEventSinkPtr WeiwooSessionSink = new SessionEventSinkImpl(proc);
			TianShanIce::Properties qos;
			bRet = eventChannel->sink(WeiwooSessionSink,qos);
		}
		
		adapter->activate();
		eventChannel->start();
		m_Communicator->waitForShutdown();
	}
	catch( const ::Ice::Exception &ex)
	{
		return -1;
	}
	catch(...)
	{
		return -1;
	}
#else
	// for the demo test
	
	SYSTEMTIME sysTime;
	memset(&sysTime,0,sizeof(sysTime));

	string strCategory ="StreamEvent";
	int iLevel = 10;
	string strCurTime="2007-01-12";
	string strMessage="new message";
	if ( proc)
	{
		int i=0;
		while(1)
		{
			string strNewCategory;
			string strNewCurTime;
			string strNewMessage;
			char s1[20]={0};
			int j =( i % 3 );
			itoa(j,s1,10);
			strNewCategory = strCategory + s1;
			memset(&sysTime,0,sizeof(sysTime));
			GetLocalTime(&sysTime);
			char s2[100]={0};
			sprintf(s2,"%04d%s%02d%s%02d%s%02d%s%02d%s%02d",sysTime.wYear,"-",sysTime.wMonth,"-",sysTime.wDay," ",sysTime.wHour,":",sysTime.wMinute,":",sysTime.wSecond);
			strNewCurTime = s2;

			memset(s1,0,sizeof(s1));
			itoa(i,s1,10);
			strNewMessage = strMessage + s1;
			(*proc)(strNewCategory,j,strNewCurTime,strNewMessage);
			i++;
			Sleep(2000);
		}
	}
	else
	{
		return -1;
	}
#endif
	return 0;
}

void ReadConfigFromXml()
{
	ZQ::common::MutexGuard gd(m_Mutex);
	try
	{
		int i =0;
		m_Communicator	 = Ice::initialize(i,NULL);

		ZQ::common::XMLPrefDoc* pXMLDoc;
		ZQ::common::ComInitializer* pcomInit;
			
		pcomInit = new ZQ::common::ComInitializer();
		pXMLDoc = new ZQ::common::XMLPrefDoc(*pcomInit);
		char strFile[MAX_PATH] ={0};
		TCHAR theFileName[ MAX_PATH ] ={0};
		GetModuleFileName(NULL,theFileName,sizeof(theFileName)/sizeof(theFileName[0]));
		
		int iSzLen=_tcsclen(theFileName);
		if (  iSzLen > 0 )
		{
				int iTemp=iSzLen-1;
				while (theFileName[iTemp]!='\\' && iTemp >=0 )
					iTemp--;
				iTemp--;//skip '\'
				while (theFileName[iTemp]!='\\' && iTemp >=0 )
					iTemp--;
				if(iTemp>0)
				{
					theFileName[iTemp]='\0';
				}
				_tcsncat(theFileName,_T("\\etc"),_tcsclen(_T("\\etc")));
		}
		_tcscat(theFileName,_T("\\InfoClientConfig.xml"));
		
	#if defined _UNICODE || defined UNICODE
		WideCharToMultiByte(CP_ACP,NULL,theFileName,-1,strFile,sizeof(strFile),NULL,NULL);
	#else
		sprintf(strFile,"%s",theFileName);
	#endif
		if ( pXMLDoc )
		{
			bool bRes;
			bRes = pXMLDoc->open(strFile);
			if ( !bRes )
			{
				return ;
			}
		}
	
		char szNodeName[XMLNAME_LEN]={0};
		char szNodeValue[XMLDATA_LEN]={0};
		
		ZQ::common::IPreference* rootIpref = pXMLDoc->root();
		ZQ::common::IPreference* itemIpref = NULL;
		
		itemIpref = rootIpref->firstChild(); 
		while(itemIpref != NULL)
		{
			memset(szNodeName,0,sizeof(szNodeName));
			itemIpref->name(szNodeName);
			// Service name
			if ( _stricmp(szNodeName,"ServiceSNMP") == 0 )
			{
				memset(szNodeValue,0,sizeof(szNodeValue));
				itemIpref->get("Name",szNodeValue);
				m_strServiceName = szNodeValue;
			}

			// service endpoint 
			memset(szNodeName,0,sizeof(szNodeName));
			itemIpref->name(szNodeName);
			if ( _stricmp(szNodeName,"Service") == 0 )
			{
				memset(szNodeValue,0,sizeof(szNodeValue));
				itemIpref->get("EndPoint",szNodeValue);
				m_strServiceEndPoint = szNodeValue;
			}
			itemIpref = rootIpref->nextChild();
		}
		if ( itemIpref )
		{
			itemIpref->free();
		}
		rootIpref->free();
		
		if(pcomInit != NULL)
			delete pcomInit;
		pcomInit = NULL;
		pXMLDoc = NULL;
	}
	catch(...)
	{
		return ;
	}
}
