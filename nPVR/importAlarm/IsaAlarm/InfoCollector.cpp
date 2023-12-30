// InfoCollector.cpp: implementation of the InfoCollector class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable:4786)

#include <map>
#include "InfoCollector.h"
#include "BaseInfoCol.h"
#include "HandlerGroup.h"
#include "BaseMessageHandler.h"
#include "ChannelMessageQueue.h"
#include "BaseMessageReceiver.h"
#include "InitInfo.h"
#include "Log.h"
#include "StringFuncton.h"

using namespace ZQ::common;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

InfoCollector::InfoCollector()
{

}

InfoCollector::~InfoCollector()
{
	close();
}

bool InfoCollector::init(const char* szCfgFile)
{
	_strCfgFile = szCfgFile;

	InitInfo	initInfo;
	std::map<int, HandlerGroup*> handlerGroups;

	if (!initInfo.init(szCfgFile))
	{
		glog(Log::L_ERROR, "Configure file %s is error", szCfgFile);
		return false;
	}

	initInfo.setCurrent(KD_SN_GLOBAL);

	//first create the receiver
	int nChannel=0;
	int nSource=0;
	if (!initInfo.getValue(KD_KN_CHANNELCOUNT, nChannel))
	{
		return false;
	}

	if (!initInfo.getValue(KD_KN_SOURCECOUNT, nSource))
	{
		return false;
	}

	//
	// load all receiver
	//
	char  szSessionName[128];
	for(int i=1;i<=nChannel;i++)
	{
		sprintf(szSessionName, "%s%03d", KD_SN_CHANNEL, i);
		initInfo.setCurrent(szSessionName);
	
		int nReceiver = 0;
		if (!initInfo.getValue(KD_KN_RECEIVERCOUNT, nReceiver))
		{
			continue;
		}

		for(int j=1;j<=nReceiver;j++)
		{
			char szSNR[128];
			sprintf(szSNR, "%s%03d%s%02d", KD_SN_SH_CHANNEL, 
				i, KD_SN_SH_RECEIVER, j);
			initInfo.setCurrent(szSNR);
			
			//if KD_KN_ENABLE_ITEM not found, set to enable
			int nEnable=1;
			initInfo.getValue(KD_KN_ENABLE_ITEM, nEnable);
			if (!nEnable)
			{
				continue;
			}

			std::string strType;
			initInfo.getValue(KD_KN_TYPE, strType);
			BaseMessageReceiver* pReceiver = createMessageReceiver(i, strType.c_str());
			if (!pReceiver)
			{
				glog(Log::L_ERROR, KD_KEY_ERROR_FORMAT, KD_KN_TYPE, szSNR, strType.c_str());
				continue;
			}

			if (!pReceiver->init(initInfo, szSNR))
			{
				delete pReceiver;
				glog(Log::L_ERROR, "Receiver ");
				continue;
			}

			_receiverList.push_back(pReceiver);
		}
	}

	//if no receiver was loaded, exist
	if (_receiverList.empty())
	{
		glog(Log::L_ERROR, "No message receiver loaded");
		return false;
	}

	//
	// create channel queue
	//
	_pChannelQueue = new ChannelMessageQueue();
	if (!_pChannelQueue->init())
	{
		glog(Log::L_ERROR, "ChannelMessageQueue init fail");
		return false;
	}
	
	for(std::vector<BaseMessageReceiver*>::iterator it=_receiverList.begin();
		it!=_receiverList.end();it++)
	{
		_pChannelQueue->addMessageReceiver(*it);
	}



	//
	// load all message handler
	//
	for(i=1;i<=nSource;i++)
	{
		sprintf(szSessionName, "%s%02d", KD_SN_SOURCE, i);
		initInfo.setCurrent(szSessionName);
	
		//if KD_KN_ENABLE_ITEM not found, set to enable
		int nEnable=1;
		initInfo.getValue(KD_KN_ENABLE_ITEM, nEnable);
		if (!nEnable)
		{
			continue;
		}

		std::string strType;
		initInfo.getValue(KD_KN_TYPE, strType);
		
		BaseInfoCol* pInfoCol = createInfoCol(strType.c_str());
		if (!pInfoCol)
		{
			glog(Log::L_ERROR, KD_KEY_ERROR_FORMAT, KD_KN_TYPE, szSessionName, strType.c_str());
			continue;
		}

		if(!pInfoCol->init(initInfo, szSessionName))
		{
			glog(Log::L_ERROR, "Source %d init failure", i);
			delete pInfoCol;
			continue;
		}

		int nGroupID = 0;
		if (!initInfo.getValue(KD_KN_HANDLERGROUPID, nGroupID))
		{
			delete pInfoCol;
			continue;
		}

		HandlerGroup* pHandlerGroup = NULL;

		//
		// see if the group already created
		//
		std::map<int, HandlerGroup*>::iterator it;
		it = handlerGroups.find(nGroupID);
		if (it!=handlerGroups.end())
		{
			pHandlerGroup = it->second;
			pInfoCol->setHandlerGroup(pHandlerGroup);
			continue;
		}

		// not found, create it
		pHandlerGroup = new HandlerGroup();		

		char szHGN[128];
		sprintf(szHGN, "%s%02d", KD_SN_HANDLERGROUP, nGroupID);
		initInfo.setCurrent(szHGN);

		int nHandlerCount = 0;
		if (!initInfo.getValue(KD_KN_HANDLERCOUNT, nHandlerCount))
		{
			delete pInfoCol;
			continue;
		}

		for(int j=1;j<=nHandlerCount;j++)
		{
			char szHandler[128];
			sprintf(szHandler, "%s%02d%s%03d", KD_SN_SH_HANDLERGROUP, 
				nGroupID, KD_SN_SH_HANDLER, j);
			initInfo.setCurrent(szHandler);

			//if KD_KN_ENABLE_ITEM not found, set to enable
			int nEnable=1;
			initInfo.getValue(KD_KN_ENABLE_ITEM, nEnable);
			if (!nEnable)
			{
				continue;
			}

			std::string strSyntax;
			if(!initInfo.getValue(KD_KN_SYNTAX, strSyntax, true, true))
			{
				continue;
			}

			int nChannelID=0;
			if(!initInfo.getValue(KD_KN_CHANNELID, nChannelID))
			{
				continue;
			}

			if (nChannelID<=0||nChannelID>nChannel)
			{
				glog(Log::L_ERROR, KD_KEY_ERROR_FORMAT_INT, KD_KN_CHANNELID, szHandler, nChannelID);
				continue;
			}

			std::vector<std::string> strFields;
			_pChannelQueue->getChannelRequireFields(nChannelID, strFields);

			BaseMessageHandler* pHandler = createMessageHandler();
			if (!pHandler)
			{
				glog(Log::L_ERROR, "Create message handler failure");
				continue;
			}

			if(!pHandler->init(nChannelID, strSyntax.c_str(), _pChannelQueue))
			{
				glog(Log::L_ERROR, KD_KEY_ERROR_FORMAT, KD_KN_SYNTAX, szHandler, strSyntax.c_str());
				continue;
			}

			for(int k=0;k<strFields.size();k++)
			{
				char szValue[512];
				initInfo.getValue(strFields[k].c_str(), szValue, sizeof(szValue));

				// skip field whose value is empty
				if (szValue[0])
				{
					pHandler->addOutputField(strFields[k].c_str(), szValue);
				}
			}

			pHandlerGroup->addHandler(pHandler);
		}

		if (pHandlerGroup->getHanderCount()==0)
		{
			glog(Log::L_ERROR, "No message handler in handler group %d", nGroupID);
			delete pHandlerGroup;
			delete pInfoCol;

			continue;
		}
		
		pInfoCol->setHandlerGroup(pHandlerGroup);
		handlerGroups.insert(std::pair<int,HandlerGroup*>(nGroupID, pHandlerGroup));

		_sourceList.push_back(pInfoCol);
	}

	//
	// see if all source failed
	//
	if (_sourceList.empty())
	{
		glog(Log::L_ERROR, "No source collector loaded");
		return false;		
	}

	//
	// see if all handler failed
	//
	if (handlerGroups.empty())
	{
		glog(Log::L_ERROR, "No handler group loaded");
		return false;		
	}

	std::map<int, HandlerGroup*>::iterator itm=handlerGroups.begin();
	for(;itm!=handlerGroups.end();itm++)
	{
		_handlerGroups.push_back(itm->second);
	}	

	StrFuncInit();

	return true;
}

bool InfoCollector::start()
{
	int nSucc = 0;
	int i=0;
	std::vector<BaseInfoCol*>::iterator its;
	for(its=_sourceList.begin();its!=_sourceList.end();its++,i++)
	{
		if((*its)->start())
		{
			nSucc++;			
		}
		else
		{
			glog(Log::L_ERROR, "Source %d start failed", i);
		}
	}	

	return (nSucc>0);
}

void InfoCollector::close()
{
	if (_sourceList.size())
	{
		std::vector<BaseInfoCol*>::iterator its=_sourceList.begin();
		for(;its!=_sourceList.end();its++)
		{
			if (*its)
			{
				(*its)->close();
				delete (*its);
			}		
		}
		_sourceList.clear();
		glog(Log::L_DEBUG, L"All source closed");
	}

	StrFuncClose();

	if (_pChannelQueue)
	{
		_pChannelQueue->close();
		delete _pChannelQueue;
		_pChannelQueue = NULL;
	}

	if (_handlerGroups.size())
	{
		std::vector<HandlerGroup*>::iterator ith=_handlerGroups.begin();
		for(;ith!=_handlerGroups.end();ith++)
		{
			if (*ith)
			{
				delete (*ith);
			}
		}
		_handlerGroups.clear();
		glog(Log::L_DEBUG, L"All handler groups closed");
	}

	if (_receiverList.size())
	{
		std::vector<BaseMessageReceiver*>::iterator itr=_receiverList.begin();
		for(;itr!=_receiverList.end();itr++)
		{
			if (*itr)
			{
				delete (*itr);
			}
		}
		_receiverList.clear();
		glog(Log::L_DEBUG, L"All receivers closed");
	}
}
