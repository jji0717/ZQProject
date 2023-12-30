// InfoCollector.h: interface for the InfoCollector class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INFOCOLLECTOR_H__228F62FA_6021_4CB0_9075_263338597FDA__INCLUDED_)
#define AFX_INFOCOLLECTOR_H__228F62FA_6021_4CB0_9075_263338597FDA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "KeyDefine.h"
#include <vector>

class HandlerGroup;
class BaseMessageReceiver;
class BaseInfoCol;
class BaseMessageHandler;
class ChannelMessageQueue;

class InfoCollector  
{
public:
	InfoCollector();
	virtual ~InfoCollector();

	bool init(const char* szCfgFile);

	bool start();

	void close();

protected:

	BaseMessageReceiver* createMessageReceiver(int channelID, const char* szType = KD_KV_RECEIVERTYPE_TEXTFILE);
	BaseInfoCol* createInfoCol(const char* szType = KD_KV_SOURCETYPE_SCLOG);
	BaseMessageHandler* createMessageHandler(const char* szType = KD_KV_HANDLERTYPE_BOOSTREG);

private:
	std::vector<BaseMessageReceiver*>	_receiverList;
	std::vector<BaseInfoCol*>			_sourceList;
	std::vector<HandlerGroup*>			_handlerGroups;
	ChannelMessageQueue*				_pChannelQueue;
	std::string							_strCfgFile;
};

#endif // !defined(AFX_INFOCOLLECTOR_H__228F62FA_6021_4CB0_9075_263338597FDA__INCLUDED_)
