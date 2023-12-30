#include "SMSXmlProc.h"

using namespace ZQ::common;

/**************  SMSConfigInfo  ****************/

SMSConfigInfo::SMSConfigInfo()
{
	strnset(_configFilePath, 0, MAX_PATH+1);
	strnset(_TMIp,	   0, 20);
	strnset(_ACFlag,   0, 10);
	strnset(_CTFlag,   0, 10);
	strnset(_RGFlag,   0, 10);
	strnset(_NCFlag,   0, 10);
	
	strcpy (_TMIp, SMSSRV_DEFAULT_IP_ADR);

	_TMPort        = SMSSRV_DEFAULT_PORT;
	_response	   = RESPONSE;
	_errorResponse = RESPONSE;
	_replyHistory  = RESPONSE;

	memset(_success, 0x00, MAX_LENGTH*sizeof(int));
}

/**************  SMSXmlProc  ****************/
SMSXmlProc::SMSXmlProc()
{
	init = NULL;
}

SMSXmlProc::~SMSXmlProc()
{
}

void SMSXmlProc::XmlProc(char* sXml, 
						 char* sequenceId, 
						 char* actionCode, 
						 char* parameter1, 
						 char* parameter2)
{
	ZQ::common::XMLPrefDoc doc(*init);

	ZQ::common::PrefGuard pRoot, pGuard;

	char prSequenceID[10];//store parameter sequence
	memset(prSequenceID, 0x00, 10*sizeof(char));

	char nodeName[20];
	memset(nodeName, 0x00, 20*sizeof(char));

	bool bSuccess = true;

	try
	{
		if (!doc.read(sXml, -1))
		{
			bSuccess = false;
		}
	}
	catch (ZQ::common::Exception e)
	{
		bSuccess = false;
	}
	catch (...) 
	{		
		bSuccess = false;
	}
	
	if (bSuccess)
	{
		pRoot.pref(doc.root());
		if (!pRoot.valid())
		{
			doc.close();
			return;
		}
		else
		{
			pRoot.pref()->name(nodeName);
			if (strcmp(nodeName, "Message") != 0)
			{
				doc.close();
				return;
			}
		}

		pGuard.pref(pRoot.pref()->firstChild());
		while (1)
		{
			if (!pGuard.valid())
			{
				doc.close();
				return;
			}
			memset(nodeName, 0x00, 20*sizeof(char));
			pGuard.pref()->name(nodeName);
			if (strcmp(nodeName, "Head") == 0)
			{
				if (strlen(sequenceId) == 0)
				{
					pGuard.pref()->get("sequence", sequenceId);
				}
			}
			else if(strcmp(nodeName, "Action") == 0)
			{
				if (strlen(actionCode) == 0)
				{
					pGuard.pref()->get("code", actionCode);
				}
			}
			else if(strcmp(nodeName, "Parameter") == 0)
			{
				pGuard.pref()->get("sequence", prSequenceID);
				if (strcmp(prSequenceID, "1") == 0)
				{
					if (strlen(parameter1) == 0)
					{
						pGuard.pref()->gettext(parameter1);
					}
				}
				else if (strcmp(prSequenceID, "2") == 0)
				{
					if (strlen(parameter2) == 0)
					{
						pGuard.pref()->gettext(parameter2);
					}
				}
			}
			pGuard.pref(pRoot.pref()->nextChild());
		}
	}
	doc.close();
}

bool SMSXmlProc::XmlGetConfig(SMSConfigInfo* info)
{
	ZQ::common::XMLPrefDoc doc(*init);

	ZQ::common::PrefGuard pRoot, pGuard, pCon;

	char nodeName[20];
	memset(nodeName, 0x00, 20*sizeof(char));

	char content[30];

	char SuccessFlag[10];

	wchar_t text[200];

	bool bSuccess = true;

	try
	{
		//if (!doc.open(filename, -1))
		if (!doc.open(info->_configFilePath, -1))
		{
			bSuccess = false;
		}
	}
	catch (ZQ::common::Exception e)
	{
		bSuccess = false;
	}
	catch (...) 
	{		
		bSuccess = false;
	}

	if (bSuccess)
	{
		pRoot.pref(doc.root());
		if (!pRoot.valid())
		{
			return bSuccess;
		}
		else
		{
			pRoot.pref()->name(nodeName);
			if (strcmp(nodeName, "Config") != 0)
			{
				return false;
			}
		}
		
		pGuard.pref(pRoot.pref()->firstChild());
		while (1)
		{
			if (!pGuard.valid())
			{
				return bSuccess;
			}
			
			memset(nodeName, 0x00, 20*sizeof(char));
			pGuard.pref()->name(nodeName);

			if (strcmp(nodeName, "TICP") == 0)
			{
				pCon.pref(pGuard.pref()->firstChild());
				
				while (1)
				{
					if (!pCon.valid())
					{
						break;
					}
					memset(nodeName, 0x00, 20*sizeof(char));
					pCon.pref()->name(nodeName);

					if (strcmp(nodeName, "Ip") == 0)
					{
						memset(content, 0x00, 30*sizeof(char));
						pCon.pref()->get("address", content);
						//Iplist.AddHead(content);
						info->_TicpIplist.AddHead(content);

						memset(content, 0x00, 30*sizeof(char));
						pCon.pref()->get("port", content);
						//Portlist.AddHead(content);
						info->_TicpPortlist.AddHead(content);
					}
					pCon.pref(pGuard.pref()->nextChild());
				}				
			}
			else if (strcmp(nodeName, "SMServer") == 0)
			{
				pCon.pref(pGuard.pref()->firstChild());
				
				while (1)
				{
					if (!pCon.valid())
					{
						break;
					}
					memset(nodeName, 0x00, 20*sizeof(char));
					pCon.pref()->name(nodeName);

					if (strcmp(nodeName, "Ip") == 0)
					{
						memset(content, 0x00, 30*sizeof(char));
						pCon.pref()->get("address", content);
						//strcpy(TMIp, content);
						strcpy(info->_TMIp, content);

						memset(content, 0x00, 30*sizeof(char));
						pCon.pref()->get("port", content);
						//strcpy(TMPort, content);
						info->_TMPort = atoi(content);
					}
					pCon.pref(pGuard.pref()->nextChild());
				}								
			}
			else if (strcmp(nodeName, "StreamControl") == 0)
			{
				pCon.pref(pGuard.pref()->firstChild());
				while (1)
				{
					if (!pCon.valid())
					{
						break;
					}
					memset(nodeName, 0x00, 20*sizeof(char));
					pCon.pref()->name(nodeName);

					if (!strcmp(nodeName, "PlayFlag")		 || 
						!strcmp(nodeName, "StopFlag")		 ||
						!strcmp(nodeName, "FastForwardFlag") ||
						!strcmp(nodeName, "FastRewindFlag")  ||
						!strcmp(nodeName, "PauseFlag")		 ||
						!strcmp(nodeName, "ResumeFlag"))
					{
						memset(content, 0x00, 30*sizeof(char));
						pCon.pref()->get("SMS", content);
						//TMFlagList.AddHead(content);
						info->_TMFlagList.AddHead(content);

						memset(content, 0x00, 30*sizeof(char));
						pCon.pref()->get("Ticp", content);
						//TicpFlagList.AddHead(content);
						info->_TicpFlagList.AddHead(content);
					}
					pCon.pref(pGuard.pref()->nextChild());
				}			
			}
			else if (strcmp(nodeName, "ReturnText") == 0)
			{
				pCon.pref(pGuard.pref()->firstChild());
				
				while (1)
				{
					if (!pCon.valid())
					{
						break;
					}
					memset(nodeName, 0x00, 20*sizeof(char));
					pCon.pref()->name(nodeName);

					if (strcmp(nodeName, "TICP") == 0)
					{
						memset(content, 0x00, 30*sizeof(char));
						pCon.pref()->get("ActionCode", content);
						//ActionCode.AddHead(content);
						info->_ActionCodeList.AddHead(content);

						memset(text, 0x00, 200*sizeof(wchar_t));
						pCon.pref()->getUnicode(L"text", text);
						//ReturnText.AddHead(text);
						info->_ReturnTextList.AddHead(text);

						memset(SuccessFlag, 0x00, 10*sizeof(char));
						pCon.pref()->get("SuccessFlag", SuccessFlag);
						if (strlen(SuccessFlag) > 0)
						{
							int flag  = atoi(SuccessFlag);
							if (flag == 1)
							{
								int index = atoi(content);
								/*if (success)
								{
									success[index] = 1;
								}*/
								info->_success[index] = 1;
							}
						}
						/*else
						{
							int index = atoi(content);
							if (success)
							{
								success[index] = 0;
							}
						}
						*/
					}
					pCon.pref(pGuard.pref()->nextChild());
				}				
			}
			else if (strcmp(nodeName, "ACTION") == 0)
			{
				pCon.pref(pGuard.pref()->firstChild());
				
				while (1)
				{
					if (!pCon.valid())
					{
						break;
					}
					memset(nodeName, 0x00, 20*sizeof(char));
					pCon.pref()->name(nodeName);

					if (strcmp(nodeName, "StreamControl") == 0)
					{
						memset(text, 0x00, 200*sizeof(wchar_t));
						pCon.pref()->getUnicode(L"Flag", text);
						int len = wcslen(text) * 2 + 1;
						//WideCharToMultiByte(CP_ACP, 0, text, -1, ACFlag, len, NULL, NULL);
						WideCharToMultiByte(CP_ACP, 0, text, -1, info->_ACFlag, len, NULL, NULL);
					}
					else if (strcmp(nodeName, "ChatControl") == 0)
					{
						memset(text, 0x00, 200*sizeof(wchar_t));
						pCon.pref()->getUnicode(L"Flag", text);
						int len = wcslen(text) * 2 + 1;
						//WideCharToMultiByte(CP_ACP, 0, text, -1, CTFlag, len, NULL, NULL);
						WideCharToMultiByte(CP_ACP, 0, text, -1, info->_CTFlag, len, NULL, NULL);
					}
					else if (strcmp(nodeName, "RegisterControl") == 0)
					{
						memset(text, 0x00, 200*sizeof(wchar_t));
						pCon.pref()->getUnicode(L"Flag", text);
						int len = wcslen(text) * 2 + 1;
						//WideCharToMultiByte(CP_ACP, 0, text, -1, RGFlag, len, NULL, NULL);
						WideCharToMultiByte(CP_ACP, 0, text, -1, info->_RGFlag, len, NULL, NULL);
					}
					else if (strcmp(nodeName, "NickNameControl") == 0)
					{
						memset(text, 0x00, 200*sizeof(wchar_t));
						pCon.pref()->getUnicode(L"Flag", text);
						int len = wcslen(text) * 2 + 1;
						//WideCharToMultiByte(CP_ACP, 0, text, -1, NCFlag, len, NULL, NULL);
						WideCharToMultiByte(CP_ACP, 0, text, -1, info->_NCFlag, len, NULL, NULL);
					}
					pCon.pref(pGuard.pref()->nextChild());
				}	
			}
			else if (strcmp(nodeName, "ON_OFF") == 0)
			{
				pCon.pref(pGuard.pref()->firstChild());
				
				while (1)
				{
					if (!pCon.valid())
					{
						break;
					}
					memset(nodeName, 0x00, 20*sizeof(char));
					pCon.pref()->name(nodeName);

					if (strcmp(nodeName, "Response") == 0)
					{
						memset(content, 0x00, 30*sizeof(char));
						pCon.pref()->get("Flag", content);
						//response = atoi(content);
						info->_response = atoi(content);
					}
					else if (strcmp(nodeName, "ErrorResponse") == 0)
					{
						memset(content, 0x00, 30*sizeof(char));
						pCon.pref()->get("Flag", content);
						//errorResponse = atoi(content);
						info->_errorResponse = atoi(content);
					}
					else if (strcmp(nodeName, "ReplyHistory") == 0)
					{
						memset(content, 0x00, 30*sizeof(char));
						pCon.pref()->get("Flag", content);
						//replyHistory = atoi(content);
						info->_replyHistory = atoi(content);
					}
					pCon.pref(pGuard.pref()->nextChild());
				}				
			}
			else if (strcmp(nodeName, "SMSProvider") == 0)
			{
				pCon.pref(pGuard.pref()->firstChild());
				
				while (1)
				{
					if (!pCon.valid())
					{
						break;
					}
					memset(nodeName, 0x00, 20*sizeof(char));
					pCon.pref()->name(nodeName);

					if ((strcmp(nodeName, "PHS")	== 0) || 
						(strcmp(nodeName, "MOBILE") == 0) ||
						(strcmp(nodeName, "UNICOM") == 0))
					{
						memset(content, 0x00, 30*sizeof(char));
						pCon.pref()->get("SP", content);
						//SP.AddHead(content);
						info->_SPList.AddHead(content);

						memset(content, 0x00, 30*sizeof(char));
						pCon.pref()->get("ZipCode", content);
						//SPNumber.AddHead(content);
						info->_SPNumberList.AddHead(content);
					}
					pCon.pref(pGuard.pref()->nextChild());
				}	
			}
			
			pGuard.pref(pRoot.pref()->nextChild());
		}
	}
	return bSuccess;
}
