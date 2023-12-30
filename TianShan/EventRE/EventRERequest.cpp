#include "EventRERequest.h"

ConnectEventChannelRequest::ConnectEventChannelRequest(EventRuleEngine& ruleEng, NativeThreadPool& pool)
: ZQ::common::ThreadRequest(pool), _ruleEng(ruleEng), _bExit(false)
{
}

ConnectEventChannelRequest::~ConnectEventChannelRequest()
{
}

bool ConnectEventChannelRequest::init()
{
	return true;
}

int ConnectEventChannelRequest::run(void)
{
	while (!_bExit)
	{
		if (_ruleEng.ConnectEventChannel())
			_bExit = true;
		else 
			_event.wait(2000);
	}

	return 0;
}

void ConnectEventChannelRequest::final(int retcode, bool bCancelled)
{
	delete this;
}
