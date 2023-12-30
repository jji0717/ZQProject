#ifndef __LiveChannel_ConnectService_H__
#define __LiveChannel_ConnectService_H__

#include <NativeThread.h>
#include "SystemUtils.h"

namespace LiveChannel
{
	class Environment;
	class ConnectService : public ZQ::common::NativeThread
	{
	public: 
		ConnectService(Environment& env);
		virtual ~ConnectService();
		void stop();

	protected:
		virtual bool init(void);
		virtual int run();
		virtual void final(void);

	protected: 
		Environment& _env;
        SYS::SingleObject _event;
		bool _bExit;

	}; // class ConnectService

} // namespace LiveChannel

#endif // #define __LiveChannel_ConnectService_H__

