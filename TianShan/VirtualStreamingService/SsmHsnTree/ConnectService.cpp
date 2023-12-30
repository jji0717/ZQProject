#include "Environment.h"
#include "ConnectService.h"

namespace HSNTree
{
	ConnectService::ConnectService(Environment& env) : _env(env), _event(NULL), _bExit(false)
	{
	}

	ConnectService::~ConnectService()
	{
		stop();
		if (NULL != _event)
			::CloseHandle(_event);
		_event = NULL;
	}

	void ConnectService::stop()
	{
		_bExit = true;
		if (NULL != _event)
			::SetEvent(_event);
		::Sleep(1);
	}

	bool ConnectService::init(void)
	{
		glog(DebugLevel, CLOGFMT(ConnectService, "ConnectService thread init()"));
		_event = ::CreateEvent(NULL, FALSE, FALSE, NULL);
		return NULL != _event ? true : false;
	}

	int ConnectService::run()
	{
		bool _bIceStorm(false);
		while (false == _bExit)
		{
			if (false == _bIceStorm)
				_bIceStorm = _env.connectIceStorm();
			if (true == _bIceStorm)
				_bExit = true;
			else 
				WaitForSingleObject(_event, 5000);
		}

		return 0;
	}

	void ConnectService::final(void)
	{
		glog(DebugLevel, CLOGFMT(ConnectService, "ConnectService thread final()"));
	}

} // namespace HSNTree

