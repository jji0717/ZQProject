// SMXmlProc.cpp: implementation of the CSMXmlProc class.
//
//////////////////////////////////////////////////////////////////////

#include "..\STVMainHeaders.h"
#include "..\MainCtrl\ScheduleTV.h"
#include "SMXmlProc.h"
#include "xmlwritestream.h"
#include "SMConnector.h"




//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace ZQ::common;


const char* MSG_VERSION_STRING			= "0099";	///<message version string


//////////////////////////////////////////////////////////////////////////
///  this class is for splitering a playlist's multi 'I' type asset to multi playlist
///  and make every playlist to main control is begin with 'I' type asset
class PlayListISpliter
{
public:
	PlayListISpliter(XMLPrefDoc* pDoc, ZQ::common::IPreference* pPlayList):_pPlayListParent(pPlayList)
	{
		_pDoc = pDoc;
		_pIAsset = _pPlayListParent->firstChild();
	}

	~PlayListISpliter()
	{
	}

	ZQ::common::IPreference*GetNextPlayList()
	{
		if (!_pIAsset)
			return NULL;

		ZQ::common::IPreference* pPlayListRet;
		

		pPlayListRet = _pDoc->newElement(XML_PLAYLIST);
		pPlayListRet->set(XML_PLAYLIST_LISTNUMBER, "0");
		pPlayListRet->set(XML_PLAYLIST_ELEMENTNUMBER, "0");			

		char sTmp[256];

		while(_pIAsset) 
		{
			pPlayListRet->addNextChild(_pIAsset);

			_pIAsset->free();

			_pIAsset = _pPlayListParent->nextChild();

			if (_pIAsset)
			{
				_pIAsset->get(XML_PLAYLIST_ASSET_TYPE, sTmp);
				if (sTmp[0] == 'I' || sTmp[0] == 'i')
				{
					// it is a new I type asset, current playlist is ok
					break;				
				}			
			}
		};

		return pPlayListRet;
	}


private:
	
	ZQ::common::IPreference* _pPlayListParent;
	XMLPrefDoc* _pDoc;
	ZQ::common::IPreference* _pIAsset;
};


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

	ZQ::common::IPreference* pRoot, *pTemp,  *pMessage, * pPort, * pSchedule, *pScInfo, *pPlayList, *pAsset;


	pRoot = NULL;
	pMessage = NULL;


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
			bSuccess = false;
			nRet = STVXMLPARSEERR;
		}
	}
	catch (...) 
	{
		bSuccess = false;
		nRet = STVXMLPARSEERR;
	}

	if (bSuccess)
	{
		pRoot = doc.root();


		// get message content node
		// first Child is Agent information
		pTemp = pRoot->firstChild();		
		if (!pTemp)
		{
			bSuccess = false;
			nRet = STVERRFORMAT;
		}
		else
			pTemp->free();


		pMessage = pRoot->nextChild();   // pPref --> <Message> body
		if (!pMessage)
		{
			bSuccess = false;
			nRet = STVERRFORMAT;
		}
		else
		{
			//////////////////////////////////////////////////////////////////////////
			//  get message information
			pMessage->get("MessageCode", sTmp);
			
			nMessageCode = atoi(sTmp);	

			pMessage->get("Time", sTime);

			pMessage->get("Flag", sTmp);
			nFlag = atoi(sTmp);

			pMessage->get("No", sTmp);
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
				pScInfo = pMessage->firstChild();    // <ScheduleInformation>
				if (!pScInfo)
				{
					bSuccess = false;
					nRet = STVERRFORMAT;
					break;
				}


				pSchedule = pScInfo->firstChild();
				if (!pSchedule)
				{
					bSuccess = false;
					nRet = STVERRFORMAT;
					pScInfo->free();

					break;
				}


				pSchedule->get("Type", sTmp);
/*				if (sTmp[0] != 'P')
				{
				}
*/

				// channel information
				pPort = pSchedule->firstChild();	//<Port>	
				if (!pPort)
				{
					bSuccess = false;
					nRet = STVERRFORMAT;
					pScInfo->free();
					pSchedule->free();

					break;
				}

				
				pPlayList = pSchedule->nextChild();
				while(pPlayList)
				{
					PlayListISpliter plisp(&doc, pPlayList);					
					ZQ::common::IPreference* pPL;

					while(pPL = plisp.GetNextPlayList())
					{
						nRet = _pSITV->OnNewPlayList(pPort, pPL, LISTTYPE_PLAYLIST);
						if (nRet)
						{
							bSuccess = false;
							pPL->free();
							break;
						}

						pPL->free();
					}

					pPlayList->free();	// release current node

					if (!bSuccess)
					{
						break;
					}

					pPlayList = pSchedule->nextChild();
				}
				

				pPort->free();
				pSchedule->free();
				pScInfo->free();
			}
			break;

		case MC_PLAY_FILLER_IMMEDIATE:
			{
				//////////////////////////////////////////////////////////////////////////
				// play Filler Immediately
				pScInfo = pMessage->firstChild();    // <ScheduleInformation>
				if (!pScInfo)
				{
					bSuccess = false;
					nRet = STVERRFORMAT;
					break;
				}


				
				pSchedule = pScInfo->firstChild();		// <Schedule> ' sub node
				if (!pSchedule)
				{
					bSuccess = false;
					nRet = STVERRFORMAT;
					pScInfo->free();

					break;
				}

				
				
				pPort = pSchedule->firstChild();
				while(pPort)
				{
					nRet = _pSITV->OnPlayFillerImmediately(pPort);

					if (nRet)
					{
						bSuccess = false;
						pPort->free();

						break;
					}

					pPort->free();

					pPort = pSchedule->nextChild();
				}


				pSchedule->free();
				pScInfo->free();
			}
			break;

		case MC_ENQUIRY_STATUS:
//			//////////////////////////////////////////////////////////////////////////
//			//SM Enquiry the Status
//			{
//				
//				pScInfo = pMessage->firstChild();    // <ScheduleInformation>
//				if (!pScInfo)
//				{
//					bSuccess = false;
//					nRet = STVERRFORMAT;					
//					break;
//				}
//
//				
//
//				pPort = pScInfo->firstChild();
//				while(pPort)
//				{
//					// let the main control to call SendStatusFeedback send the content	
//					// send all channel so can send all the request status within a time
//					nRet = _pSITV->OnQueryStatus(pPort);
//
//					if (nRet)
//					{
//						bSuccess = false;
//						pPort->free();
//
//						break;
//					}
//
//					pPort->free();
//
//					pPort = pScInfo->nextChild();
//				}
//				
//				
//				pScInfo->free();
//			}
			break;

		case MC_PLAYLIST_BARKER:
			//////////////////////////////////////////////////////////////////////////		
			//Send PlayList to BARKER System
			{
				//////////////////////////////////////////////////////////////////////////
				// call SITV subsystem to deal
				pScInfo = pMessage->firstChild();    // <ScheduleInformation>
				if (!pScInfo)
				{
					bSuccess = false;
					nRet = STVERRFORMAT;
					break;
				}


				pSchedule = pScInfo->firstChild();
				if (!pSchedule)
				{
					bSuccess = false;
					nRet = STVERRFORMAT;
					pScInfo->free();

					break;
				}


				pSchedule->get("Type", sTmp);
/*				if (sTmp[0] != 'P')
				{
				}
*/

				// channel information
				pPort = pSchedule->firstChild();	//<Port>	
				if (!pPort)
				{
					bSuccess = false;
					nRet = STVERRFORMAT;
					pScInfo->free();
					pSchedule->free();

					break;
				}

				
				pPlayList = pSchedule->nextChild();
				while(pPlayList)
				{
					nRet = _pSITV->OnNewPlayList(pPort, pPlayList, LISTTYPE_BARKER);
					if (nRet)
					{
						bSuccess = false;
						pPlayList->free();	// release current node

						break;
					}


					pPlayList->free();	// release current node


					pPlayList = pSchedule->nextChild();
				}
				

				pPort->free();
				pSchedule->free();
				pScInfo->free();
			}
			break;

		case MC_FILLERLIST:
			//////////////////////////////////////////////////////////////////////////
			//Send FillerList to PlayBack
			{
				//////////////////////////////////////////////////////////////////////////
				// call SITV subsystem to deal
				pScInfo = pMessage->firstChild();    // <ScheduleInformation>
				if (!pScInfo)
				{
					bSuccess = false;
					nRet = STVERRFORMAT;
					break;
				}


				pSchedule = pScInfo->firstChild();
				if (!pSchedule)
				{
					bSuccess = false;
					nRet = STVERRFORMAT;
					pScInfo->free();

					break;
				}


				pSchedule->get("Type", sTmp);
/*				if (sTmp[0] != 'P')
				{
				}
*/

				// channel information
				pPort = pSchedule->firstChild("Port");	//<Port>

//				!! Notice, if no <Port> exist, it is a global filler  
//				!! Hence it's normal						--Bernie				

				if (!pPort)
				{
//					bSuccess = false;
//					nRet = STVERRFORMAT;
//					pScInfo->free();
//					pSchedule->free();
//
//					break;
				}

				
				pPlayList = pSchedule->firstChild("FillerList");
				while(pPlayList)
				{
					nRet = _pSITV->OnNewPlayList(pPort, pPlayList, LISTTYPE_FILLER);
					if (nRet)
					{
						bSuccess = false;
						pPlayList->free();	// release current node

						break;
					}


					pPlayList->free();	// release current node


					pPlayList = pSchedule->nextChild();
				}
				
				if(pPort)
					pPort->free();
				pSchedule->free();
				pScInfo->free();
			}
			break;

		case MC_PLAY_SPECIFIC_ASSET:
			//////////////////////////////////////////////////////////////////////////
			//	Play Specific Asset
			{
				pScInfo = pMessage->firstChild();    // <ScheduleInformation>
				if (!pScInfo)
				{
					bSuccess = false;
					nRet = STVERRFORMAT;
					break;
				}


				
				pSchedule = pScInfo->firstChild();		//<Schedule>
				if(!pSchedule)
				{
					bSuccess = false;
					nRet = STVERRFORMAT;
					pScInfo->free();

					break;
				}



				// channel information
				pPort = pSchedule->firstChild();	//<Port>
				if (!pPort)
				{
					bSuccess = false;
					nRet = STVERRFORMAT;
					pScInfo->free();
					pSchedule->free();

					break;
				}

				
				pAsset = pSchedule->nextChild();	//<Asset>
				if (!pAsset)
				{
					bSuccess = false;
					nRet = STVERRFORMAT;
					pScInfo->free();
					pSchedule->free();
					pPort->free();

					break;
				}



				nRet = _pSITV->OnPlayAssetImmediately(pPort, pAsset);
				if (nRet)
				{
					bSuccess = false;
				}
				

				pAsset->free();
				pSchedule->free();	// release current node
				pPort->free();
				pScInfo->free();
			}
			break;

		case MC_SKIPTO_SPECIFIC_ASSET:
			//////////////////////////////////////////////////////////////////////////
			//	Skip to Specific Asset
			{
				pScInfo = pMessage->firstChild();    // <ScheduleInformation>
				if (!pScInfo)
				{
					bSuccess = false;
					nRet = STVERRFORMAT;
					break;
				}


				
				pSchedule = pScInfo->firstChild();		//<Schedule>
				if(!pSchedule)
				{
					bSuccess = false;
					nRet = STVERRFORMAT;
					pScInfo->free();

					break;
				}



				// channel information
				pPort = pSchedule->firstChild();	//<Port>
				if (!pPort)
				{
					bSuccess = false;
					nRet = STVERRFORMAT;
					pScInfo->free();
					pSchedule->free();

					break;
				}

				
				pAsset = pSchedule->nextChild();	//<Asset>
				if (!pAsset)
				{
					bSuccess = false;
					nRet = STVERRFORMAT;
					pScInfo->free();
					pSchedule->free();
					pPort->free();

					break;
				}



				nRet = _pSITV->OnPlaySkipToAsset(pPort, pAsset);
				if (nRet)
				{
					bSuccess = false;
				}
				

				pAsset->free();
				pSchedule->free();	// release current node
				pPort->free();
				pScInfo->free();
			}
			break;

		case MC_HOLD_NEXT_ASSET:
			{
				//////////////////////////////////////////////////////////////////////////
				// Hold Next Asset
				pScInfo = pMessage->firstChild();    // <ScheduleInformation>
				if (!pScInfo)
				{
					bSuccess = false;
					nRet = STVERRFORMAT;
					break;
				}


				
				pSchedule = pScInfo->firstChild();		//<Schedule>
				if(!pSchedule)
				{
					bSuccess = false;
					nRet = STVERRFORMAT;
					pScInfo->free();

					break;
				}



				// channel information
				pPort = pSchedule->firstChild();	//<Port>
				if (!pPort)
				{
					bSuccess = false;
					nRet = STVERRFORMAT;
					pScInfo->free();
					pSchedule->free();

					break;
				}

				
				pAsset = pSchedule->nextChild();	//<Asset>
				if (!pAsset)
				{
					bSuccess = false;
					nRet = STVERRFORMAT;
					pScInfo->free();
					pSchedule->free();
					pPort->free();

					break;
				}



				nRet = _pSITV->OnHoldAsset(pPort, pAsset);
				if (nRet)
				{
					bSuccess = false;
				}
				

				pAsset->free();
				pSchedule->free();	// release current node
				pPort->free();
				pScInfo->free();
			}
			break;

		case MC_STARTUP_CHANNEL:
			{
				pPort = pMessage->firstChild();    // <Port>

				while(pPort)
				{
					nRet = _pSITV->OnChannelStartup(pPort);

					if (nRet)
					{

					}

					pPort->free();


					pPort = pMessage->nextChild();
				}

			}
			break;

		case MC_SHUTDOWN_CHANNEL:
			{

				pPort = pMessage->firstChild();    // <Port>

				while(pPort)
				{
					nRet = _pSITV->OnChannelShutdown(pPort);

					if (nRet)
					{

					}

					pPort->free();


					pPort = pMessage->nextChild();
				}
			}
			break;

		case MC_CONFIGURATION:
			{
				pTemp = pMessage->firstChild();    // <Configration>

				while(pTemp)
				{
					pTemp->get("Type", sTmp);
					int nType = atoi(sTmp);

					// is itv's configuration
					if (nType == 2)
					{
						nRet = _pSITV->OnConfigration(pTemp);

						if (nRet)
						{
							bSuccess = false;
						}

						pTemp->free();
						break;
					}


					pTemp->free();

					
					//next configuration
					pTemp = pMessage->nextChild();
				}

			}
			break;


		case MC_HANDSHAKE:
			{
				/// meaning current client id is error, log it
				
				bNeedSendResponse = false;
			}
		default:
			//////////////////////////////////////////////////////////////////////////
			// 
			bSuccess = false;
			nRet = STVERRFORMAT;//unknown
		}
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
	}		


	//release
	if (pMessage)
	{
		pMessage->free();
	}

	if (pRoot)
	{
		pRoot->free();
	}

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