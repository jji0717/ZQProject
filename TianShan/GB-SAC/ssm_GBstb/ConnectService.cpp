#include "./Environment.h"
#include "./ConnectService.h"

extern ZQ::common::Log* s1log;

namespace TianShanS1
{
	ConnectService::ConnectService(Environment& env) : _env(env), _bExit(false)
	{
	}

	ConnectService::~ConnectService()
	{
		stop();
	}

	void ConnectService::stop()
	{
		_bExit = true;
        _event.signal();
	}

	bool ConnectService::init(void)
	{
		SSMLOG(DebugLevel, CLOGFMT(ConnectService, "ConnectService thread init()"));
		return true;
	}

	int ConnectService::run()
	{
		bool _bIceStorm(false);
		while (false == _bExit)
		{
			if (false == _bIceStorm) {
				_bIceStorm = _env.connectIceStorm();
            }
			if (true == _bIceStorm) {
				_bExit = true;
            }
			else {
                _event.wait(5000);
            }
		}

		return 0;
	}

	void ConnectService::final(void)
	{
		SSMLOG(DebugLevel, CLOGFMT(ConnectService, "ConnectService thread final()"));
	}

} // namespace TianShanS1

