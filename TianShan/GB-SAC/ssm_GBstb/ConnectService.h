#ifndef __TianShanS1_ConnectService_H__
#define __TianShanS1_ConnectService_H__

#include <NativeThread.h>
#include "SystemUtils.h"

namespace TianShanS1
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

} // namespace TianShanS1

#endif // #define __TianShanS1_ConnectService_H__

