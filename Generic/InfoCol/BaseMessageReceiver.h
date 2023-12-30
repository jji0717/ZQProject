// BaseMessageReceiver.h: interface for the BaseMessageReceiver class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BASEMESSAGERECEIVER_H__DB63DF8C_377C_42F5_8D6D_238D3059A2F0__INCLUDED_)
#define AFX_BASEMESSAGERECEIVER_H__DB63DF8C_377C_42F5_8D6D_238D3059A2F0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include <vector>
#include "InitInfo.h"

struct MESSAGE_FIELD;
typedef std::vector<struct MESSAGE_FIELD> MessageFields;

class BaseMessageReceiver  
{
public:
	BaseMessageReceiver(int channelID);
	virtual ~BaseMessageReceiver();

	virtual bool init(InitInfo& initInfo, const char* szSessionName)=0;
	virtual void close()=0;

	virtual void OnMessage(int nMessageID, MessageFields* pMessage)=0;

	virtual void requireFields(std::vector<std::string>& fields)=0;

	void setParam(const char*sKey, const char* sValue);

	int  getChannelID() { return _channelID;}

protected:
	int _channelID;
};

#endif // !defined(AFX_BASEMESSAGERECEIVER_H__DB63DF8C_377C_42F5_8D6D_238D3059A2F0__INCLUDED_)
