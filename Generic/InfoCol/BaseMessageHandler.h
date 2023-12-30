// BaseMessageHandler.h: interface for the BaseMessageHandler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BASEMESSAGEHANDLER_H__264C1FEC_1C72_4628_B022_9BA0822B4710__INCLUDED_)
#define AFX_BASEMESSAGEHANDLER_H__264C1FEC_1C72_4628_B022_9BA0822B4710__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include <vector>

class ChannelMessageQueue;
class BaseMessageHandler  
{
public:
	BaseMessageHandler();
	virtual ~BaseMessageHandler();

	virtual bool init(int channelID, const char* syntax, ChannelMessageQueue* channelQueue);
	
	void addOutputField(const char* key, const char* value);

	virtual bool handleMessage(const char* msg)=0;

	virtual void close();

protected:
	ChannelMessageQueue*	_channelQueue;

	int						_channelID;

	std::string				_syntax;

	struct OUTPUT_FIELD
	{
		std::string	strKey;
		std::string	strValue;
	};

	std::vector<struct OUTPUT_FIELD>		_outputs;
};

#endif // !defined(AFX_BASEMESSAGEHANDLER_H__264C1FEC_1C72_4628_B022_9BA0822B4710__INCLUDED_)
