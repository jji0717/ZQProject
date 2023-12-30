#pragma once
#include "SmsControl.h"
#include <EventGwHelper.h>


class EventHandle : public EventGateway::IGenericEventHelper
{
public:
	EventHandle(void);
	~EventHandle(void);

	bool Init(void);
	void UnInit(void);
	void SetLog(ZQ::common::Log* pLog);

public:
	virtual void onEvent(
        const ::std::string& category,
        ::Ice::Int eventId,
        const ::std::string& eventName,
        const ::std::string& stampUTC,
        const ::std::string& sourceNetId,
		const EventGateway::Properties& params);

	//name define
	static char nameDef[3][10];

	struct EventGroup
	{
		std::string strEvent;
		std::string strFormat;
		std::vector<std::string> telV;
	};
	typedef std::vector<struct EventGroup> GROUPV;
private:
	ZQ::common::Log*	_pLog;
	SmsControl			_SmsCtrl;
	GROUPV				_eventGroupV;
};
