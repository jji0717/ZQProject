#ifndef __HSNTree_ConnectService_H__
#define __HSNTree_ConnectService_H__

#include <NativeThread.h>

namespace HSNTree
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
		HANDLE _event;
		bool _bExit;

	}; // class ConnectService

} // namespace HSNTree

#endif // #define __HSNTree_ConnectService_H__

