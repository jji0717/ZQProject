// ORBAEventReceiver.h: interface for the CORBAEventReceiver class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ORBAEVENTRECEIVER_H__E59D9FED_A8CF_45A5_982D_AB6C0ACE45CF__INCLUDED_)
#define AFX_ORBAEVENTRECEIVER_H__E59D9FED_A8CF_45A5_982D_AB6C0ACE45CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <map>
#include <deque>
#include "NativeThread.h"
#include "BaseMessageReceiver.h"

namespace CosNotification{
	struct StructuredEvent;
};

class CORBAEventReceiver : public BaseMessageReceiver, ZQ::common::NativeThread
{
public:
	CORBAEventReceiver(int channelID);
	virtual ~CORBAEventReceiver();

	static const char* getTypeInfo()
	{
		return KD_KV_RECEIVERTYPE_ISAORBEVENTCHANNEL;
	}

protected:
	virtual bool init(InitInfo& initInfo, const char* szSessionName);
	virtual void close();

	virtual void requireFields(std::vector<std::string>& fields);

	int run();

	enum 
	{
		TYPE_STRING,
		TYPE_INT
	};

	struct ORB_FIELD_DATA
	{		
		const char* szOuter;
		const char* szInter;
		int			type;		//TYPE_STRING for string  TYPE_INT for int
	};

	virtual void OnMessage(int nMessageID, MessageFields* pMessage);

	static	ORB_FIELD_DATA		_header_fields[];
	static  int					_header_field_count;

	static	ORB_FIELD_DATA		_body_fields[];
	static  int					_body_field_count;

	std::string					_strDomain;
	std::string					_strNameServer;
	std::string					_strNameServiceInitRef;
	std::string					_strNotifycationServiceInitRef;
	std::string					_strEndPoint;
	std::string					_strEventChannelName;	

	int							_nMaxEventQueueDeep;
	
	std::map<std::string, ORB_FIELD_DATA*>		_headerMap;
	std::map<std::string, ORB_FIELD_DATA*>		_bodyMap;

	struct _EVENT_DATA
	{
		int nMessageID;
		CosNotification::StructuredEvent* pEvent;
	};
	std::deque<_EVENT_DATA>	_eventQueue;

	CRITICAL_SECTION							_lock;
	HANDLE						_hSem;
	HANDLE						_hExit;
};

#endif // !defined(AFX_ORBAEVENTRECEIVER_H__E59D9FED_A8CF_45A5_982D_AB6C0ACE45CF__INCLUDED_)
