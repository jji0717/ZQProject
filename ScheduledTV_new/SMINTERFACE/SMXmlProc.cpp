// SMXmlProc.cpp: implementation of the CSMXmlProc class.
//
//////////////////////////////////////////////////////////////////////

#include "..\STVMainHeaders.h"
#include "..\MainCtrl\ScheduleTV.h"
#include "..\PlaylistMod\STVList_def.h"
#include "SMXmlProc.h"
#include "xmlwritestream.h"
#include "SMConnector.h"
#include "log.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace ZQ::common;


const char* MSG_VERSION_STRING			= "0099";	///<message version string


//////////////////////////////////////////////////////////////////////////
///  this class is for splitering a playlist's multi 'I' type asset to multi playlist
///  and make every playlist to main control is begin with 'I' type asset
//class PlayListISpliter
//{
//public:
//	PlayListISpliter(XMLPrefDoc* pDoc, ZQ::common::IPreference* pPlayList):_pPlayListParent(pPlayList)
//	{
//		_pDoc = pDoc;
//		_pIAsset = _pPlayListParent.pref()->firstChild();
//	}
//
//	~PlayListISpliter()
//	{
//	}
//
//	ZQ::common::IPreference*GetNextPlayList()
//	{
//		if (!_pIAsset)
//			return NULL;
//
//		ZQ::common::IPreference* pPlayListRet;
//		
//
//		pPlayListRet = _pDoc->newElement(TAG_PLAYLIST);
//		pPlayListRet->set(KEY_PLAYLIST_LISTNUMBER, "0");
//		pPlayListRet->set(XML_PLAYLIST_ELEMENTNUMBER, "0");			
//
//		char sTmp[256];
//
//		while(_pIAsset) 
//		{
//			pPlayListRet->addNextChild(_pIAsset);
//
//
//			_pIAsset = _pPlayListParent.pref()->nextChild();
//
//			if (_pIAsset)
//			{
//				_pIAsset.pref()->get(XML_PLAYLIST_ASSET_TYPE, sTmp);
//				if (sTmp[0] == 'I' || sTmp[0] == 'i')
//				{
//					// it is a new I type asset, current playlist is ok
//					break;				
//				}			
//			}
//		};
//
//		return pPlayListRet;
//	}
//
//
//private:
//	
//	ZQ::common::IPreference* _pPlayListParent;
//	XMLPrefDoc* _pDoc;
//	ZQ::common::IPreference* _pIAsset;
// };


/*
/// the main control 
class SITV
{
public:
	
	int OnNewPlayList(IPreference* pPortPref, IPreference* pPlayListPref, int nType)
	{
		//printf("OnNewPlayList Type: %d\n", nType);
		char ss[200000];
		unsigned long nLen = sizeof(ss);
		pPortPref->toStream(ss, &nLen);
		printf("OnNewPlayList type %d: \n 1st:\n%s\n", nType, ss);
		nLen = sizeof(ss);
		pPlayListPref->toStream(ss, &nLen);
		printf("2nd:\n%s\n", ss);

		return 0;
	}
	
	int OnPlayAssetImmediately(IPreference* pPortPref, IPreference* pPlayListPref)
	{
		char ss[200000];
		unsigned long nLen = sizeof(ss);
		pPortPref->toStream(ss, &nLen);
		printf("OnPlayAssetImmediately: \n 1st:\n%s\n", ss);
		nLen = sizeof(ss);
		pPlayListPref->toStream(ss, &nLen);
		printf("2nd:\n%s\n", ss);

		return 0;
	}
	
	int OnHoldAsset(IPreference* pPortPref, IPreference* pAssetPref)
	{
		char ss[200000];
		unsigned long nLen = sizeof(ss);
		pPortPref->toStream(ss, &nLen);
		printf("OnHoldAsset: \n 1st:\n%s\n", ss);
		nLen = sizeof(ss);
		pAssetPref->toStream(ss, &nLen);
		printf("2nd:\n%s\n", ss);

		return 0;
	}
	
	int OnPlaySkipToAsset(IPreference* pPortPref, IPreference* pPlayListPref)
	{
		char ss[200000];
		unsigned long nLen = sizeof(ss);
		pPortPref->toStream(ss, &nLen);
		printf("OnPlaySkipToAsset: \n 1st:\n%s\n", ss);
		nLen = sizeof(ss);
		pPlayListPref->toStream(ss, &nLen);
		printf("2nd:\n%s\n", ss);

		return 0;
	}

	int OnPlayFillerImmediately(IPreference* pPortPref)
	{
		char ss[200000];
		unsigned long nLen = sizeof(ss);
		pPortPref->toStream(ss, &nLen);
		printf("OnPlayFillerImmediately : \n 1st:\n%s\n", ss);

		return 0;
	}

	int OnQueryStatus(IPreference* pPortPref)
	{
		char ss[200000];
		unsigned long nLen = sizeof(ss);
		pPortPref->toStream(ss, &nLen);
		printf("OnQueryStatus: \n 1st:\n%s\n", ss);

		return 0;
	}

	int OnChannelStartup(IPreference* pPortPref)
	{
		char ss[200000];
		unsigned long nLen = sizeof(ss);
		pPortPref->toStream(ss, &nLen);
		printf("OnChannelStartup: \n 1st:\n%s\n", ss);

		return 0;
	}

	int OnChannelShutdown(IPreference* pPortPref)
	{
		char ss[200000];
		unsigned long nLen = sizeof(ss);
		pPortPref->toStream(ss, &nLen);
		printf("OnChannelShutdown: \n 1st:\n%s\n", ss);

		return 0;
	}

	int OnConfigration(IPreference* pConfigPref)
	{
		char ss[200000];
		unsigned long nLen = sizeof(ss);
		pConfigPref->toStream(ss, &nLen);
		printf("OnConfigration: \n 1st:\n%s\n", ss);

		return 0;
	}
};

SITV sitv;

*/

CSMXmlProc::CSMXmlProc()
{
	_dwNo = GEN_ID_DOWNBOUND;
	_pSITV = NULL;
	init = NULL;
}

CSMXmlProc::~CSMXmlProc()
{

}

void CSMXmlProc::XmlProc(const char* sXml)
{
	char sTime[128];	//parsed message time
	
	int  nFlag = 0;			//parsed message flag
	
	DWORD dwRNO = 0;		//parsed message No
	
	int nMessageCode = 0;	//parsed message code

	int nRet;				//errorid, 0 means right

	std::string  sErrorStr;		//error descripton

	ZQ::common::PrefGuard pRoot, pTemp,  pMessage, pPort, pSchedule, pScInfo, pPlayList, pAsset;

	bool bSuccess = true;


	bool bNeedSendResponse = true;		//flag for if need send response
	

#ifdef _DEBUG
	printf("%s\n", sXml);

	//glog(Log::L_DEBUG, sXml);
#endif



	char sTmp[256];
	ZQ::common::XMLPrefDoc doc(*init);

	try
	{
		if (!doc.read(sXml, -1))
		{
			GTRACEERR;
			glog(Log::L_ERROR, "CSMXmlProc::XmlProc() failed to read xml: \n%s",sXml);
			bSuccess = false;
			nRet = STVXMLPARSEERR;
		}
	}
	catch (ZQ::common::Exception e)
	{
		glog(Log::L_ERROR, "CSMXmlProc::XmlProc() got exception when reading xml: %s\n%s",e.getString(), sXml);
		bSuccess = false;
		nRet = STVXMLPARSEERR;
	}
	catch (...) 
	{
		glog(Log::L_ERROR, "CSMXmlProc::XmlProc() got unknown exception when reading xml: \n%s",sXml);
		bSuccess = false;
		nRet = STVXMLPARSEERR;
	}
	
	try
	{
	
		if (bSuccess)
		{
			pRoot.pref(doc.root());
			if(!pRoot.valid())
			{
				GTRACEERR;
				bSuccess = false;
				nRet = STVERRFORMAT;
			}

			// get message content node
			// first Child is Agent information
			pTemp.pref(pRoot.pref()->firstChild());		
			if (!pTemp.valid())
			{
				GTRACEERR;
				bSuccess = false;
				nRet = STVERRFORMAT;
			}


			pMessage.pref(pRoot.pref()->nextChild());   // pPref --> <Message> body
			if (!pMessage.valid())
			{
				GTRACEERR;
				bSuccess = false;
				nRet = STVERRFORMAT;
			}
			else
			{
				//////////////////////////////////////////////////////////////////////////
				//  get message information
				pMessage.pref()->get("MessageCode", sTmp);
				
				nMessageCode = atoi(sTmp);	

				pMessage.pref()->get("Time", sTime);

				pMessage.pref()->get("Flag", sTmp);
				nFlag = atoi(sTmp);

				pMessage.pref()->get("No", sTmp);
				dwRNO = atoi(sTmp);
			}
		}

		
		if (bSuccess)
		{
			switch(nMessageCode) {
			case MC_RESPONSE:

				// is response message
				bNeedSendResponse = false;
				break;

			case MC_SCHEDULE_LIST:
				{
					//////////////////////////////////////////////////////////////////////////
					// call SITV subsystem to deal
					pScInfo.pref(pMessage.pref()->firstChild());    // <ScheduleInformation>
					if (!pScInfo.valid())
					{
						GTRACEERR;
						bSuccess = false;
						nRet = STVERRFORMAT;
						break;
					}


					pSchedule.pref(pScInfo.pref()->firstChild());
					if (!pSchedule.valid())
					{
						GTRACEERR;
						bSuccess = false;
						nRet = STVERRFORMAT;

						break;
					}


					pSchedule.pref()->get("Type", sTmp);
	/*				if (sTmp[0] != 'P')
					{
					}
	*/
					if(!pSchedule.pref()->has(KEY_SCHEDULE_ID))
					{
						GTRACEERR;
						bSuccess = false;
						nRet = STVERRFORMAT;
						break;
					}
					pSchedule.pref()->get(KEY_SCHEDULE_ID, sTmp);	//schedule ID
					std::string		schId = sTmp;
					schId += "#";

					if(!pSchedule.pref()->has(KEY_SCHEDULE_SEQ))
					{
						schId += "0";
					}
					else
					{
						pSchedule.pref()->get(KEY_SCHEDULE_SEQ, sTmp);
						schId += sTmp;
					}

					// channel information
					pPort.pref(pSchedule.pref()->firstChild());	//<Port>	
					if (!pPort.valid())
					{
						GTRACEERR;
						bSuccess = false;
						nRet = STVERRFORMAT;

						break;
					}

					
					pPlayList.pref(pSchedule.pref()->nextChild());
					while(pPlayList.valid())
					{
						nRet = _pSITV->OnNewPlayList(schId.c_str(), pPort.pref(), pPlayList.pref(), LISTTYPE_NORMAL);
						if (nRet)
						{
							bSuccess = false;
							break;
						}

						pPlayList.pref(pSchedule.pref()->nextChild());
					}

				}
				break;

			case MC_PLAY_FILLER_IMMEDIATE:
				{
					//////////////////////////////////////////////////////////////////////////
					// play Filler Immediately
					pScInfo.pref( pMessage.pref()->firstChild() );    // <ScheduleInformation>
					if (!pScInfo.valid())
					{
						GTRACEERR;
						bSuccess = false;
						nRet = STVERRFORMAT;
						break;
					}


					
					pSchedule.pref( pScInfo.pref()->firstChild() );		// <Schedule> ' sub node
					if (!pSchedule.valid())
					{
						GTRACEERR;
						bSuccess = false;
						nRet = STVERRFORMAT;
						break;
					}

					
					
					pPort.pref( pSchedule.pref()->firstChild() );
					while(pPort.valid())
					{
						nRet = _pSITV->OnPlayFillerImmediately(pPort.pref());

						if (nRet)
						{
							bSuccess = false;

							break;
						}


						pPort.pref( pSchedule.pref()->nextChild() );
					}


				}
				break;

			case MC_ENQUIRY_STATUS:
	//			//////////////////////////////////////////////////////////////////////////
	//			//SM Enquiry the Status
	//			{
	//				
	//				pScInfo = pMessage.pref()->firstChild();    // <ScheduleInformation>
	//				if (!pScInfo)
	//				{
	//					bSuccess = false;
	//					nRet = STVERRFORMAT;					
	//					break;
	//				}
	//
	//				
	//
	//				pPort = pScInfo.pref()->firstChild();
	//				while(pPort)
	//				{
	//					// let the main control to call SendStatusFeedback send the content	
	//					// send all channel so can send all the request status within a time
	//					nRet = _pSITV->OnQueryStatus(pPort);
	//
	//					if (nRet)
	//					{
	//						bSuccess = false;
	//
	//						break;
	//					}
	//
	//
	//					pPort = pScInfo.pref()->nextChild();
	//				}
	//				
	//				
	//			}
				break;

			case MC_PLAYLIST_BARKER:
				//////////////////////////////////////////////////////////////////////////		
				//Send PlayList to BARKER System
				{
					//////////////////////////////////////////////////////////////////////////
					// call SITV subsystem to deal
					pScInfo.pref( pMessage.pref()->firstChild() );    // <ScheduleInformation>
					if (!pScInfo.valid())
					{
						GTRACEERR;
						bSuccess = false;
						nRet = STVERRFORMAT;
						break;
					}


					pSchedule.pref( pScInfo.pref()->firstChild() );
					if (!pSchedule.valid())
					{
						GTRACEERR;
						bSuccess = false;
						nRet = STVERRFORMAT;

						break;
					}


					pSchedule.pref()->get("Type", sTmp);
	/*				if (sTmp[0] != 'P')
					{
					}
	*/
					if(!pSchedule.pref()->has(KEY_SCHEDULE_ID))
					{
						GTRACEERR;
						bSuccess = false;
						nRet = STVERRFORMAT;
						break;
					}
					pSchedule.pref()->get(KEY_SCHEDULE_ID, sTmp);	//schedule ID
					std::string		schId = sTmp;
					schId += "#";

					if(!pSchedule.pref()->has(KEY_SCHEDULE_SEQ))
					{
						schId += "0";
					}
					else
					{
						pSchedule.pref()->get(KEY_SCHEDULE_SEQ, sTmp);
						schId += sTmp;
					}

					// channel information
					pPort.pref( pSchedule.pref()->firstChild() );	//<Port>	
					if (!pPort.valid())
					{
						GTRACEERR;
						bSuccess = false;
						nRet = STVERRFORMAT;

						break;
					}

					
					pPlayList.pref( pSchedule.pref()->nextChild() );
					while(pPlayList.valid())
					{
						nRet = _pSITV->OnNewPlayList(schId.c_str(), pPort.pref(), pPlayList.pref(), LISTTYPE_BARKER);
						if (nRet)
						{
							bSuccess = false;

							break;
						}


						pPlayList.pref( pSchedule.pref()->nextChild() );
					}
					
				}
				break;

			case MC_FILLERLIST:
				//////////////////////////////////////////////////////////////////////////
				//Send FillerList to PlayBack
				{
					//////////////////////////////////////////////////////////////////////////
					// call SITV subsystem to deal
					pScInfo.pref( pMessage.pref()->firstChild() );    // <ScheduleInformation>
					if (!pScInfo.valid())
					{
						GTRACEERR;
						bSuccess = false;
						nRet = STVERRFORMAT;
						break;
					}


					pSchedule.pref( pScInfo.pref()->firstChild() );
					if (!pSchedule.valid())
					{
						GTRACEERR;
						bSuccess = false;
						nRet = STVERRFORMAT;

						break;
					}


					pSchedule.pref()->get("Type", sTmp);
	/*				if (sTmp[0] != 'P')
					{
					}
	*/
					if(!pSchedule.pref()->has(KEY_SCHEDULE_ID))
					{
						GTRACEERR;
						bSuccess = false;
						nRet = STVERRFORMAT;
						break;
					}
					pSchedule.pref()->get(KEY_SCHEDULE_ID, sTmp);	//schedule ID
					std::string		schId = sTmp;
					schId += "#";

					if(!pSchedule.pref()->has(KEY_SCHEDULE_SEQ))
					{
						schId += "0";
					}
					else
					{
						pSchedule.pref()->get(KEY_SCHEDULE_SEQ, sTmp);
						schId += sTmp;
					}

					// channel information
					pPort.pref( pSchedule.pref()->firstChild("Port") );	//<Port>

	//				!! Notice, if no <Port> exist, it is a global filler  
	//				!! Hence it's normal						--Bernie				
	//
	//				if (!pPort)
	//				{
	//					bSuccess = false;
	//					nRet = STVERRFORMAT;
	//
	//					break;
	//				}
					
					pPlayList.pref( pSchedule.pref()->firstChild("FillerList") );
					while(pPlayList.valid())
					{
						nRet = _pSITV->OnNewPlayList(schId.c_str(), pPort.pref(), pPlayList.pref(), LISTTYPE_FILLER);
						if (nRet)
						{
							bSuccess = false;

							break;
						}


						pPlayList.pref( pSchedule.pref()->nextChild() );
					}
					
				}
				break;

			case MC_PLAY_SPECIFIC_ASSET:
				//////////////////////////////////////////////////////////////////////////
				//	Play Specific Asset
				{
					pScInfo.pref( pMessage.pref()->firstChild() );    // <ScheduleInformation>
					if (!pScInfo.valid())
					{
						GTRACEERR;
						bSuccess = false;
						nRet = STVERRFORMAT;
						break;
					}


					
					pSchedule.pref( pScInfo.pref()->firstChild() );		//<Schedule>
					if(!pSchedule.valid())
					{
						GTRACEERR;
						bSuccess = false;
						nRet = STVERRFORMAT;

						break;
					}



					// channel information
					pPort.pref( pSchedule.pref()->firstChild() );	//<Port>
					if (!pPort.valid())
					{
						GTRACEERR;
						bSuccess = false;
						nRet = STVERRFORMAT;

						break;
					}

					
					pAsset.pref( pSchedule.pref()->nextChild() );	//<Asset>
					if (!pAsset.valid())
					{
						GTRACEERR;
						bSuccess = false;
						nRet = STVERRFORMAT;

						break;
					}


					PrefGuard	pTimeshift(pSchedule.pref()->nextChild());	//<TimeShift>

					nRet = _pSITV->OnPlayAssetImmediately(pPort.pref(), pAsset.pref(), pTimeshift.pref());
					if (nRet)
					{
						bSuccess = false;

						break;
					}
					
				}
				break;

			case MC_SKIPTO_SPECIFIC_ASSET:
				//////////////////////////////////////////////////////////////////////////
				//	Skip to Specific Asset
				{
					pScInfo.pref( pMessage.pref()->firstChild() );    // <ScheduleInformation>
					if (!pScInfo.valid())
					{
						GTRACEERR;
						bSuccess = false;
						nRet = STVERRFORMAT;
						break;
					}


					
					pSchedule.pref( pScInfo.pref()->firstChild() );		//<Schedule>
					if(!pSchedule.valid())
					{
						GTRACEERR;
						bSuccess = false;
						nRet = STVERRFORMAT;

						break;
					}



					// channel information
					pPort.pref( pSchedule.pref()->firstChild() );	//<Port>
					if (!pPort.valid())
					{
						GTRACEERR;
						bSuccess = false;
						nRet = STVERRFORMAT;

						break;
					}

					
					pAsset.pref( pSchedule.pref()->nextChild() );	//<Asset>
					if (!pAsset.valid())
					{
						GTRACEERR;
						bSuccess = false;
						nRet = STVERRFORMAT;

						break;
					}



					nRet = _pSITV->OnPlaySkipToAsset(pPort.pref(), pAsset.pref());
					if (nRet)
					{
						bSuccess = false;
						break;
					}
					
				}
				break;

			case MC_HOLD_NEXT_ASSET:
				{
					//////////////////////////////////////////////////////////////////////////
					// Hold Next Asset
					pScInfo.pref( pMessage.pref()->firstChild() );    // <ScheduleInformation>
					if (!pScInfo.valid())
					{
						GTRACEERR;
						bSuccess = false;
						nRet = STVERRFORMAT;
						break;
					}


					
					pSchedule.pref( pScInfo.pref()->firstChild() );		//<Schedule>
					if(!pSchedule.valid())
					{
						GTRACEERR;
						bSuccess = false;
						nRet = STVERRFORMAT;

						break;
					}



					// channel information
					pPort.pref( pSchedule.pref()->firstChild() );	//<Port>
					if (!pPort.valid())
					{
						GTRACEERR;
						bSuccess = false;
						nRet = STVERRFORMAT;

						break;
					}

					
					pAsset.pref( pSchedule.pref()->nextChild() );	//<Asset>
					if (!pAsset.valid())
					{
						GTRACEERR;
						bSuccess = false;
						nRet = STVERRFORMAT;

						break;
					}



					nRet = _pSITV->OnHoldAsset(pPort.pref(), pAsset.pref());
					if (nRet)
					{
						bSuccess = false;
						break;
					}
					

				}
				break;

			case MC_STARTUP_CHANNEL:
				{
					pPort.pref( pMessage.pref()->firstChild() );    // <Port>

					while(pPort.valid())
					{
						nRet = _pSITV->OnChannelStartup(pPort.pref());

						if (nRet)
						{
							bSuccess = false;
							break;
						}

						pPort.pref( pMessage.pref()->nextChild() );
					}

				}
				break;

			case MC_SHUTDOWN_CHANNEL:
				{

					pPort.pref( pMessage.pref()->firstChild() );    // <Port>

					while(pPort.valid())
					{
						nRet = _pSITV->OnChannelShutdown(pPort.pref());

						if (nRet)
						{
							bSuccess = false;
							break;
						}

						pPort.pref( pMessage.pref()->nextChild() );
					}
				}
				break;

			case MC_CONFIGURATION:
				{
					pTemp.pref( pMessage.pref()->firstChild() );    // <Configration>

					while(pTemp.valid())
					{
						pTemp.pref()->get("Type", sTmp);
						int nType = atoi(sTmp);

						// is itv's configuration
						if (nType == 2)
						{
							nRet = _pSITV->OnConfigration(pTemp.pref());

							if (nRet)
							{
								bSuccess = false;
								break;
							}

							break;
						}

						
						//next configuration
						pTemp.pref( pMessage.pref()->nextChild() );
					}

				}
				break;


			case MC_HANDSHAKE:
				{
					/// meaning current client id is error, log it
					
					bNeedSendResponse = false;
				}
				break;
			default:
				//////////////////////////////////////////////////////////////////////////
				//
				GTRACEERR;
				bSuccess = false;
				nRet = STVERRFORMAT;//unknown
			}
		}

	}
	catch(ZQ::common::Exception excp)
	{
		glog(Log::L_ERROR, "CSMXmlProc::XmlProc()  got exception : %s!", excp.getString());
		bNeedSendResponse = true;
		bSuccess = false;
		nRet = STVERRFORMAT;
	}
	catch (...) 
	{
		glog(Log::L_ERROR, "CSMXmlProc::XmlProc()  got unknown exception!");
		bNeedSendResponse = true;
		bSuccess = false;
		nRet = STVERRFORMAT;
	}

	//get error string
	if (!bSuccess)
	{
		char ss[256];
		sprintf(ss, "messagecode=%d", nMessageCode);
		const char* sErrDesc = GetSTVErrorDesc(nRet);
		if (sErrDesc)
			sErrorStr = sErrDesc;
		else
			sErrorStr = ss;

		glog(Log::L_DEBUG, "CSMXmlProc::XmlProc()  error during paring XML: %s!", sErrorStr.c_str());
	}		

	//release
	doc.close();

	// return the response information
	if (bNeedSendResponse)
	{
		_pConnector->SendResponse(dwRNO, bSuccess, nRet, sErrorStr.c_str());
	}	
}


void CSMXmlProc::XmlStatusFeedback(
		const char* sContent, 
		char* sOutputXml,
		int* nOutput)
{
	CXmlWriteStream stream(sOutputXml);

	char sTime[32], sTime1[32];
	SYSTEMTIME tm;
	GetLocalTime(&tm);
	sprintf(sTime, "%04d%02d%02d %02d:%02d:%02d", tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond);
	sprintf(sTime1, "%04d-%02d-%02d %02d:%02d:%02d", tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond);




	//head
	stream.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n\
<AgentCommand>\r\n\
\t<NoIInfoHeader commandType=\"%d\" sequence=\"0\" command=\"\" systemTime=\"%s\" operationMode=\"\" version=\"%s\">\r\n\
\t\t<Sender computerName=\"Playback\" ipAddress=\"%s\" tcpPort=\"%d\" status=\"ACTIVE\"/>\r\n\
\t\t<Receiver computerName=\"SM\" ipAddress=\"%s\" tcpPort=\"%d\" status=\"ACTIVE\"/>\r\n\
\t</NoIInfoHeader>\r\n",  MC_STATUS_FEEDBACK,  sTime1,  MSG_VERSION_STRING, \
	_pConnector->_sBindIP,  _pConnector->_nLocalPort,  _pConnector->_sSMIP, _pConnector->_nPort);

	
	
	stream.Write("\t<Message Time=\"%s\" Flag=\"0\" No=\"%d\" Source=\"1\" MessageCode=\"%d\">\r\n", sTime, _dwNo, MC_STATUS_FEEDBACK);
	


	//content && tail
	stream.Write("%s\r\n\t</Message>\r\n</AgentCommand>", sContent);
	

	//adjust ID
	++_dwNo;
	if (_dwNo == GEN_ID_UPBOUND)
	{
		_dwNo = GEN_ID_DOWNBOUND;
	}


	//return
	*nOutput = stream.GetStreamLength();
}

int CSMXmlProc::XmlResponse(char* sXml, int nMsgID, bool bSuccess, int nErrorCode /* = 0 */, const char* sErrorString /* = NULL */)
{
	CXmlWriteStream wstream(sXml);
	


	char sTime[32], sTime1[32];
	SYSTEMTIME tm;
	GetLocalTime(&tm);
	sprintf(sTime, "%04d%02d%02d %02d:%02d:%02d", tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond);
	sprintf(sTime1, "%04d-%02d-%02d %02d:%02d:%02d", tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond);




	//head
	wstream.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n\
<AgentCommand>\r\n\
\t<NoIInfoHeader commandType=\"%d\" sequence=\"0\" command=\"\" systemTime=\"%s\" operationMode=\"\" version=\"%s\">\r\n\
\t\t<Sender computerName=\"Playback\" ipAddress=\"%s\" tcpPort=\"%d\" status=\"ACTIVE\"/>\r\n\
\t\t<Receiver computerName=\"SM\" ipAddress=\"%s\" tcpPort=\"%d\" status=\"ACTIVE\"/>\r\n\
\t</NoIInfoHeader>\r\n",  MC_RESPONSE,  sTime1,  MSG_VERSION_STRING, \
	_pConnector->_sBindIP,  _pConnector->_nLocalPort,  _pConnector->_sSMIP, _pConnector->_nPort);

	
	

	wstream.Write("\t<Message Time=\"%s\" Flag=\"1\" No=\"%d\" Source=\"1\" MessageCode=\"%d\">\r\n", sTime, _dwNo, MC_RESPONSE);


	wstream.Write("\t\t<Message No=\"%d\" Status=\"%d\" ", nMsgID, bSuccess?1:0);

	if (bSuccess)
	{
		wstream.Write(" />\r\n\t</Message>\r\n</AgentCommand>");
	}
	else
	{
		wstream.Write("ErrorCode=\"%d\" ErrorMessage=\"%s\" />\r\n\t</Message>\r\n</AgentCommand>", nErrorCode, sErrorString);
	}
	


	//adjust ID
	++_dwNo;
	if (_dwNo == GEN_ID_UPBOUND)
	{
		_dwNo = GEN_ID_DOWNBOUND;
	}

	
	// return the msg length
	return wstream.GetStreamLength();
}


void CSMXmlProc::XmlEnquireSchedule(
		const char* sContent, 
		char* sOutputXml,
		int* nOutput)
{
	CXmlWriteStream stream(sOutputXml);


	char sTime[32], sTime1[32];
	SYSTEMTIME tm;
	GetLocalTime(&tm);
	sprintf(sTime, "%04d%02d%02d %02d:%02d:%02d", tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond);
	sprintf(sTime1, "%04d-%02d-%02d %02d:%02d:%02d", tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond);



	//head
	stream.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n\
<AgentCommand>\r\n\
\t<NoIInfoHeader commandType=\"%d\" sequence=\"0\" command=\"\" systemTime=\"%s\" operationMode=\"\" version=\"%s\">\r\n\
\t\t<Sender computerName=\"Playback\" ipAddress=\"%s\" tcpPort=\"%d\" status=\"ACTIVE\"/>\r\n\
\t\t<Receiver computerName=\"SM\" ipAddress=\"%s\" tcpPort=\"%d\" status=\"ACTIVE\"/>\r\n\
\t</NoIInfoHeader>\r\n",  MC_ENQUIRY_SCHDULE,  sTime1,  MSG_VERSION_STRING, \
	_pConnector->_sBindIP,  _pConnector->_nLocalPort,  _pConnector->_sSMIP, _pConnector->_nPort);

	
	stream.Write("\t<Message Time=\"%s\" Flag=\"0\" No=\"%d\" Source=\"1\" MessageCode=\"%d\">\r\n\t\t<ScheduleInformation>\r\n", sTime, _dwNo, MC_ENQUIRY_SCHDULE);
		
	
	//content & tail
	stream.Write("%s\r\n\t\t</ScheduleInformation>\r\n\t</Message>\r\n</AgentCommand>", sContent);	



	//adjust ID
	++_dwNo;
	if (_dwNo == GEN_ID_UPBOUND)
	{
		_dwNo = GEN_ID_DOWNBOUND;
	}


	//return
	*nOutput = stream.GetStreamLength();
}



void CSMXmlProc::XmlQueryFillerList(
		const char* sContent, 
		char* sOutputXml,
		int* nOutput)
{
	CXmlWriteStream stream(sOutputXml);


	char sTime[32], sTime1[32];
	SYSTEMTIME tm;
	GetLocalTime(&tm);
	sprintf(sTime, "%04d%02d%02d %02d:%02d:%02d", tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond);
	sprintf(sTime1, "%04d-%02d-%02d %02d:%02d:%02d", tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond);



	//head
	stream.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n\
<AgentCommand>\r\n\
\t<NoIInfoHeader commandType=\"%d\" sequence=\"0\" command=\"\" systemTime=\"%s\" operationMode=\"\" version=\"%s\">\r\n\
\t\t<Sender computerName=\"Playback\" ipAddress=\"%s\" tcpPort=\"%d\" status=\"ACTIVE\"/>\r\n\
\t\t<Receiver computerName=\"SM\" ipAddress=\"%s\" tcpPort=\"%d\" status=\"ACTIVE\"/>\r\n\
\t</NoIInfoHeader>\r\n",  MC_ENQUIRY_SCHDULE,  sTime1,  MSG_VERSION_STRING, \
	_pConnector->_sBindIP,  _pConnector->_nLocalPort,  _pConnector->_sSMIP, _pConnector->_nPort);

	
	stream.Write("\t<Message Time=\"%s\" Flag=\"0\" No=\"%d\" Source=\"1\" MessageCode=\"%d\">\r\n", sTime, _dwNo, MC_QUERY_FILLER);
		
	
	//content & tail
	stream.Write("%s\r\n\t</Message>\r\n</AgentCommand>", sContent);	



	//adjust ID
	++_dwNo;
	if (_dwNo == GEN_ID_UPBOUND)
	{
		_dwNo = GEN_ID_DOWNBOUND;
	}


	//return
	*nOutput = stream.GetStreamLength();
}

int CSMXmlProc::XmlHandShakeMsg(char* sXml, int nClientID)
{
	CXmlWriteStream stream(sXml);


	char sTime[32], sTime1[32];
	SYSTEMTIME tm;
	GetLocalTime(&tm);
	sprintf(sTime, "%04d%02d%02d %02d:%02d:%02d", tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond);
	sprintf(sTime1, "%04d-%02d-%02d %02d:%02d:%02d", tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond);



	//head
	stream.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n\
<AgentCommand>\r\n\
\t<NoIInfoHeader commandType=\"%d\" sequence=\"0\" command=\"\" systemTime=\"%s\" operationMode=\"\" version=\"%s\">\r\n\
\t\t<Sender computerName=\"Playback\" ipAddress=\"%s\" tcpPort=\"%d\" status=\"ACTIVE\"/>\r\n\
\t\t<Receiver computerName=\"SM\" ipAddress=\"%s\" tcpPort=\"%d\" status=\"ACTIVE\"/>\r\n\
\t</NoIInfoHeader>\r\n", MC_HANDSHAKE,  sTime1,  MSG_VERSION_STRING, \
	_pConnector->_sBindIP,  _pConnector->_nLocalPort,  _pConnector->_sSMIP, _pConnector->_nPort);


	stream.Write("\t<Message Time=\"%s\" Flag=\"0\" No=\"%d\" Source=\"1\" MessageCode=\"%d\">\r\n", sTime, _dwNo, MC_HANDSHAKE);
		
	
	//content & tail
	stream.Write("\t\t<ClientID value=\"%d\"/>\r\n\t</Message>\r\n</AgentCommand>", nClientID);	



	//adjust ID
	++_dwNo;
	if (_dwNo == GEN_ID_UPBOUND)
	{
		_dwNo = GEN_ID_DOWNBOUND;
	}



	//return
	return stream.GetStreamLength();
}

int CSMXmlProc::XmlEnquireConfig(char* sXml, int nClientID)
{
	CXmlWriteStream stream(sXml);


	char sTime[32], sTime1[32];
	SYSTEMTIME tm;
	GetLocalTime(&tm);
	sprintf(sTime, "%04d%02d%02d %02d:%02d:%02d", tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond);
	sprintf(sTime1, "%04d-%02d-%02d %02d:%02d:%02d", tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond);



	//head
	stream.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n\
<AgentCommand>\r\n\
\t<NoIInfoHeader commandType=\"%d\" sequence=\"0\" command=\"\" systemTime=\"%s\" operationMode=\"\" version=\"%s\">\r\n\
\t\t<Sender computerName=\"Playback\" ipAddress=\"%s\" tcpPort=\"%d\" status=\"ACTIVE\"/>\r\n\
\t\t<Receiver computerName=\"SM\" ipAddress=\"%s\" tcpPort=\"%d\" status=\"ACTIVE\"/>\r\n\
\t</NoIInfoHeader>\r\n", MC_HANDSHAKE,  sTime1,  MSG_VERSION_STRING, \
	_pConnector->_sBindIP,  _pConnector->_nLocalPort,  _pConnector->_sSMIP, _pConnector->_nPort);


	stream.Write("\t<Message Time=\"%s\" Flag=\"0\" No=\"%d\" Source=\"1\" MessageCode=\"%d\">\r\n", sTime, _dwNo, MC_QUERY_CONFIGURATION);
		

	//content & tail
	stream.Write("\t\t<Configuration type=\"2\">\r\n\t\t\t<ClientID value=\"%d\"/>\r\n\t\t</Configuration>\r\n\t</Message>\r\n</AgentCommand>", nClientID);	



	//adjust ID
	++_dwNo;
	if (_dwNo == GEN_ID_UPBOUND)
	{
		_dwNo = GEN_ID_DOWNBOUND;
	}



	//return
	return stream.GetStreamLength();

}